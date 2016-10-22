#include "util.h"
#include <algorithm>
#include <QColor>

static double cross(const Vec2d &O, const Vec2d &A, const Vec2d &B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

void convex_hull(const std::vector<Vec2d> &points, std::vector<Vec2d> &res) {
    res.clear();

    int n = points.size(), k = 0;
    std::vector<Vec2d> H(2*n);
    std::vector<Vec2d> p = points;

    // Sort points lexicographically
    auto cmp = [](const Vec2d &a, const Vec2d &b){
        return (a.x < b.x ) || (a.x == b.x && a.y < b.y);
    };
    std::sort(p.begin(), p.end(), cmp);

    // Build lower hull
    for (int i = 0; i < n; ++i) {
        while (k >= 2 && cross(H[k-2], H[k-1], p[i]) <= 0) k--;
        H[k++] = p[i];
    }

    // Build upper hull
    for (int i = n-2, t = k+1; i >= 0; i--) {
        while (k >= t && cross(H[k-2], H[k-1], p[i]) <= 0) k--;
        H[k++] = p[i];
    }

    H.resize(k-1);
    H.swap(res);
    return;
}

inline double compute_Cmn_plus(double w, double h, double f, double u1, double v1, double u2, double v2) {
    return sqr(w/f/2)*u1*u2 + sqr(h/f/2)*v1*v2 + 1;
}

inline Vec3d compute_XYZ_from_zm_plus(double zm, double um, double vm, double w, double h, double f) {
    double Xm = -um*w*zm/2/f;
    double Ym = -vm*h*zm/2/f;
    double Zm = zm;
    return Vec3d(Xm, Ym, Zm);
}

bool compute_zm_plus(const std::vector<Vec2d> &points, const int order[], double w, double h, double f, double z0, double &zm1, double &zm2) {
    int r_index = order[0], m_index = order[1], n_index = order[2], l_index = order[3];
    double x1 = points[m_index].x, y1 = points[m_index].y;
    double x2 = points[n_index].x, y2 = points[n_index].y;
    double x3 = points[l_index].x, y3 = points[l_index].y;
    double x0 = points[r_index].x, y0 = points[r_index].y;

    double C01 = compute_Cmn_plus(w, h, f, x0, y0, x1, y1), C02 = compute_Cmn_plus(w, h, f, x0, y0, x2, y2), C03 = compute_Cmn_plus(w, h, f, x0, y0, x3, y3);
    double C00 = compute_Cmn_plus(w, h, f, x0, y0, x0, y0);
    double C12 = compute_Cmn_plus(w, h, f, x1, y1, x2, y2), C23 = compute_Cmn_plus(w, h, f, x2, y2, x3, y3), C31 = compute_Cmn_plus(w, h, f, x3, y3, x1, y1);

    double a = C01*C01*C23 + C00*C12*C31 - C01*C03*C12 - C01*C02*C31;
    double b = 2*(C01*C02*C03 - C00*C01*C23);
    double c = C00*C00*C23 - C00*C02*C03;
    double delta = b*b-4*a*c;
    if (delta < 0) return false;
    double sqrt_delta = sqrt(delta);
    zm1 = (-b - sqrt_delta)/(2*a)*z0;
    zm2 = (-b + sqrt_delta)/(2*a)*z0;
    return true;
}

bool two2three_plus(
    const std::vector<Vec2d> &points, int w, int h, // input
    Mat4d &perspective_mat, std::vector<Vec3d> &anchors, double &near_, double &far_, double &focal_   // output
    )
{
    double us[4], vs[4];
    std::vector<Vec2d> uvs(4);
    for (int i = 0; i < 4; i++) {
        us[i] = points[i].x/double(w-1)*2-1;
        vs[i] = -(points[i].y/double(h-1)*2-1);
        uvs[i] = Vec2d(us[i], vs[i]);
    }
    double u1 = us[1], v1 = vs[1], u2 = us[2], v2 = vs[2], u3 = us[3], v3 = vs[3], u0 = us[0], v0 = vs[0];
    double near_clip = 1e-2, far_clip = 1e4;
    double a = -(far_clip + near_clip)/(far_clip-near_clip);
    double b = -2*far_clip*near_clip/(far_clip-near_clip);
    near_ = near_clip;
    far_ = far_clip;
    anchors.clear();
    double focal = std::max(w, h)*1.2;
    bool isfind = false;
    // note: z0 = 1;
    for (; focal < 1e8; focal *= 1.2) {
        int order[] = {0,1,2,3};
        double zm1 = 0, zm2 = 0;
        const double z0 = -1;
        double is_real = compute_zm_plus(uvs, order, w, h, focal, z0, zm1, zm2);
        if (is_real) {
            double zn1, zn2, zl1, zl2;
            Vec3d P0 = compute_XYZ_from_zm_plus(z0, u0, v0, w, h, focal);
            double f = focal;
            double C01 = compute_Cmn_plus(w, h, f, u0, v0, u1, v1), C02 = compute_Cmn_plus(w, h, f, u0, v0, u2, v2), C03 = compute_Cmn_plus(w, h, f, u0, v0, u3, v3);
            double C00 = compute_Cmn_plus(w, h, f, u0, v0, u0, v0);
            double C12 = compute_Cmn_plus(w, h, f, u1, v1, u2, v2), C23 = compute_Cmn_plus(w, h, f, u2, v2, u3, v3), C31 = compute_Cmn_plus(w, h, f, u3, v3, u1, v1);

            zl1 = (C01*zm1 - C00*z0)*z0/(C31*zm1 - C03*z0);
            zl2 = (C01*zm2 - C00*z0)*z0/(C31*zm2 - C03*z0);
            zn1 = (C03*zl1 - C00*z0)*z0/(C23*zl1 - C02*z0);
            zn2 = (C03*zl2 - C00*z0)*z0/(C23*zl2 - C02*z0);

            Vec3d P1 = compute_XYZ_from_zm_plus(zm2, u1, v1, w, h, focal);
            Vec3d P2 = compute_XYZ_from_zm_plus(zn2, u2, v2, w, h, focal);
            Vec3d P3 = compute_XYZ_from_zm_plus(zl2, u3, v3, w, h, focal);
            perspective_mat.setTo(0);
            perspective_mat.at(0,0) = 2*focal/w;
            perspective_mat.at(1,1) = 2*focal/h;
            perspective_mat.at(2,2) = a;
            perspective_mat.at(2,3) = b;
            perspective_mat.at(3,2) = -1;
            /* end debug */
            anchors.push_back(P0);
            anchors.push_back(P1);
            anchors.push_back(P2);
            anchors.push_back(P3);
            isfind = true;
            break;
        }
    }
    if (!isfind) {
        printf("No solution\n");
    }
    focal_ = focal;
    return isfind;
}

void triangle_interpolate(const Vec2d &A, const Vec2d &B, const Vec2d &C, const Vec2d &P, double &a, double &b, double &c) {
    // P = a*A + b*B + c*C
    double s = triangle_area(A,B,C);
    double s_a = triangle_area(P,B,C);
    double s_b = triangle_area(P,C,A);
    double s_c = triangle_area(P,A,B);
    a = s_a / s;
    b = s_b / s;
    c = s_c / s;
}

void interpolate_image(const QImage &qimg, double i, double j, QRgb &color) {
    color = QRgb(0);
    // f(0,0)(1-x)(1-y)+f(0,1)(1-x)y+f(1,1)xy+f(1,0)x(1-y)
    int int_i = floor(i), int_j = floor(j);
    double x = i - int_i;
    double y = j - int_j;
    int w = qimg.width(), h = qimg.height();
    /*cv::Vec3b f00 = im.at<cv::Vec3b>(int_i, int_j);
    cv::Vec3b f01 = im.at<cv::Vec3b>(int_i+1, int_j);
    cv::Vec3b f10 = im.at<cv::Vec3b>(int_i, int_j+1);
    cv::Vec3b f11 = im.at<cv::Vec3b>(int_i+1, int_j+1);*/
    const uchar* ptr = qimg.bits();
    Vec3f f00, f01, f10, f11;

    if (int_i >= 0 && int_i < h && int_j >= 0 && int_j < w) {
        QRgb c = qimg.pixel(int_j, int_i);
        f00 = Vec3f(qRed(c), qGreen(c), qBlue(c));
    }
    if (int_i+1 >= 0 && int_i+1 < h && int_j >= 0 && int_j < w) {
        QRgb c = qimg.pixel(int_j, int_i + 1);
        f01 = Vec3f(qRed(c), qGreen(c), qBlue(c));
    }
    if (int_i >= 0 && int_i < h && int_j+1 >= 0 && int_j+1 < w) {
        QRgb c = qimg.pixel(int_j + 1, int_i);
        f10 = Vec3f(qRed(c), qGreen(c), qBlue(c));
    }
    if (int_i+1 >= 0 && int_i+1 < h && int_j+1 >= 0 && int_j+1 < w) {
        QRgb c = qimg.pixel(int_j + 1, int_i + 1);
        f11 = Vec3f(qRed(c), qGreen(c), qBlue(c));
    }
    int r = ( f00.x*(1-x)*(1-y) + f01.x*(1-x)*y + f11.x*x*y + f10.x*x*(1-y));
    int g = ( f00.y*(1-x)*(1-y) + f01.y*(1-x)*y + f11.y*x*y + f10.y*x*(1-y));
    int b = ( f00.z*(1-x)*(1-y) + f01.z*(1-x)*y + f11.z*x*y + f10.z*x*(1-y));
    color = qRgb(r, g, b);
}

Vec2d ray_rect_intersection(const Vec2d& o, const Vec2d &d, int rx, int ry, int rw, int rh) {
    double eps = 1e-10;
    double t1 = -1, t2 = -1, t3 = -1, t4 = -1;
    double x0 = o.x, y0 = o.y;
    double x1 = rx, x2 = rx + rw;
    double y1 = ry, y2 = ry + rh;
    double dx = d.x, dy = d.y;
    if (dx == 0 && dy == 0) {
        return Vec2d(-1, -1);
    } else if (dx == 0 && dy != 0) {
        t3 = (y1-y0)/dy;
        t4 = (y2-y0)/dy;
        double xx3 = x0 + t3*dx;
        double xx4 = x0 + t4*dx;
        if (xx3 < x1 || xx3 > x2) t3 = -1;
        if (xx4 < x1 || xx4 > x2) t4 = -1;
    } else if (dx != 0 && dy == 0) {
        t1 = (x1-x0)/dx;
        t2 = (x2-x0)/dx;
        double yy1 = y0 + t1*dy;
        double yy2 = y0 + t2*dy;
        if (yy1 < y1 || yy1 > y2) t1 = -1;
        if (yy2 < y1 || yy2 > y2) t2 = -1;
    } else {
        // dx != 0 && dy != 0
        t1 = (x1-x0)/dx;
        t2 = (x2-x0)/dx;
        t3 = (y1-y0)/dy;
        t4 = (y2-y0)/dy;
        double xx3 = x0 + t3*dx;
        double xx4 = x0 + t4*dx;
        if (xx3 < x1 - eps || xx3 > x2 + eps) t3 = -1;
        if (xx4 < x1 - eps || xx4 > x2 + eps) t4 = -1;
        double yy1 = y0 + t1*dy;
        double yy2 = y0 + t2*dy;
        if (yy1 < y1 - eps || yy1 > y2 + eps) t1 = -1;
        if (yy2 < y1 - eps || yy2 > y2 + eps) t2 = -1;
    }
    if (t1 < 0 && t2 < 0 && t3 < 0 && t4 < 0) {
        return Vec2d(-1,-1);
    }
    double ts[] = {t1, t2, t3, t4};
    int min_id = -1;
    double min_t = std::numeric_limits<double>::infinity();
    for (int i = 0; i <4; i++) {
        double val = (ts[i] < 0 ? std::numeric_limits<double>::infinity() : ts[i]);
        if (val < min_t) {
            min_t = val;
            min_id = i;
        }
    }
    return Vec2d(x0 + min_t*dx, y0 + min_t*dy);
}
