#ifndef UTIL_H
#define UTIL_H

#include "Vec3.h"
#include "Vec2.h"
#include "Mat.h"
#include "common.h"
#include "GLMesh.h"
#include <QImage>

template<typename _Tp>
bool plane_normal(const Vec3<_Tp> &P0, const Vec3<_Tp> &P1, const Vec3<_Tp> &P2, Vec3<_Tp> &normal) {
    _Tp x0 = P0.x, y0 = P0.y, z0 = P0.z;
    _Tp x1 = P1.x, y1 = P1.y, z1 = P1.z;
    _Tp x2 = P2.x, y2 = P2.y, z2 = P2.z;
    _Tp nx = (y1 - y0)*(z2 - z0) - (y2 - y0)*(z1 - z0);
    _Tp ny = (z1 - z0)*(x2 - x0) - (z2 - z0)*(x1 - x0);
    _Tp nz = (x1 - x0)*(y2 - y0) - (x2 - x0)*(y1 - y0);
    _Tp len = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
    if (fabs(len) < std::numeric_limits<_Tp>::min()) return false;
    _Tp inv_len = 1./len;
    normal.x = nx*inv_len;
    normal.y = ny*inv_len;
    normal.z = nz*inv_len;
    return true;
}

template<typename _Tp>
_Tp random_float() {
    return (_Tp)rand() / (_Tp)RAND_MAX;
}

template<typename _Tp>
Vec3<_Tp> CosWeightedHemisphereSample(const Vec3<_Tp> &n) {
    _Tp Xi1 = (_Tp)rand()/(_Tp)RAND_MAX;
    _Tp Xi2 = (_Tp)rand()/(_Tp)RAND_MAX;

    _Tp  theta = acos(sqrt(1.0-Xi1));
    _Tp  phi = 2.0 * 3.1415926535897932384626433832795 * Xi2;

    _Tp xs = sin(theta) * cos(phi);
    _Tp ys = cos(theta);
    _Tp zs = sin(theta) * sin(phi);

    Vec3<_Tp> y(n.x, n.y, n.z);
    Vec3<_Tp> h = y;
    if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
        h.x= 1.0;
    else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
        h.y = 1.0;
    else
        h.z = 1.0;


    Vec3<_Tp> x = (h.cross(y)).unit();
    Vec3<_Tp> z = (x.cross(y)).unit();

    Vec3<_Tp> direction = x*xs + y*ys + z*zs;
    return direction.unit();
}

void convex_hull(const std::vector<Vec2d> &p, std::vector<Vec2d> &res);

bool two2three_plus(
    const std::vector<Vec2d> &points, int w, int h, // input
    Mat4d &perspective_mat, std::vector<Vec3d> &anchors, double &near, double &far, double &focal   // output
);

void triangle_interpolate(const Vec2d &A, const Vec2d &B, const Vec2d &C, const Vec2d &P, double &a, double &b, double &c);

void interpolate_image(const QImage &qimg, double i, double j, QRgb &color);

Vec2d ray_rect_intersection(const Vec2d& o, const Vec2d &d, int rx, int ry, int rw, int rh);

template<typename T>
inline T triangle_area(const Vec2<T> &A, const Vec2<T> &B, const Vec2<T> &C) {
    // CCW
    T cp = (B.x*C.y + C.x*A.y + A.x*B.y) - (B.x*A.y + C.x*B.y + A.x*C.y);
    return static_cast<T>(0.5) * cp;
}

template<typename T>
inline T determinant_3(T a, T b, T c, T d, T e, T f, T g, T h, T i) {
    // | a b c |
    // | d e f |
    // | g h i |
    return a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
}

template<typename T>
inline T triangle_area(const Vec3<T> A, const Vec3<T> B, const Vec3<T> C) {
    //  1  |   1      1      1   |
    // --- | Bx-Ax  By-Ay  Bz-Az |
    //  2  | Cx-Ax  Cy-Ay  Cz-Az |
    T x1 = B.x - A.x, y1 = B.y - A.y, z1 = B.z - A.z;
    T x2 = C.x - A.x, y2 = C.y - A.y, z2 = C.z - A.z;
    return static_cast<T>(fabs(0.5*determinant_3<T>(1, 1, 1, x1, y1, z1, x2, y2, z2)));
}


#endif // UTIL_H

