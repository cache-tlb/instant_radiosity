#ifndef MAT_H
#define MAT_H

#include <cstring>
#include "Vec3.h"

template<typename _Tp, int N>
class Mat {
public:
    Mat() {
        memset(val, 0, sizeof(_Tp)*N*N);
    }

    Mat(const Mat<_Tp, N>& m) {
        memcpy(val, m.val, sizeof(_Tp)*N*N);
    }


    Mat & operator = (const Mat<_Tp, N> &rhs) {
        if (this != &rhs) {
            memcpy(val, rhs.val, sizeof(_Tp)*N*N);
        }
        return *this;
    }

    Mat transpose() const {
        Mat m;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                m.val[i*N+j] = val[j*N+i];
            }
        }
        return m;
    }

    void setTo(_Tp v) {
        memset(val, v, sizeof (_Tp)*N*N);
    }

    _Tp at(int i, int j) const {
        return val[i*N+j];
    }
    _Tp& at(int i, int j) {
        return val[i*N+j];
    }

    _Tp at(int i) const {
        return val[i];
    }
    _Tp& at(int i) {
        return val[i];
    }

    Mat multiply(const _Tp f) const {
        Mat ret;
        for (int i = 0; i < N*N; i++) {
            ret.val[i] = val[i]*f;
        }
        return ret;
    }

    void toArray(_Tp *a) const {
        memcpy(a, val, sizeof(_Tp)*N*N);
    }

    static Mat fromArray(const _Tp *a) {
        Mat<_Tp, N> m;
        memcpy(m.val, a, sizeof(_Tp)*N*N);
    }

public:
    _Tp val[N*N];
};

typedef Mat<double, 4> Mat4d;
typedef Mat<float, 4>  Mat4f;

typedef Mat<double, 3> Mat3d;
typedef Mat<float, 3>  Mat3f;

template<typename _Tp>
bool SolveLinear3(const Mat<_Tp, 3> &a, const Vec3<_Tp> &b, Vec3<_Tp> &x) {
    _Tp detA = a.at(0)*a.at(4)*a.at(8) + a.at(3)*a.at(7)*a.at(2) + a.at(6)*a.at(1)*a.at(5) - a.at(0)*a.at(7)*a.at(5) - a.at(6)*a.at(4)*a.at(2) - a.at(3)*a.at(1)*a.at(8);
    if (fabs(detA) < 1e-30) return false;
    Mat<_Tp, 3> invA, M;
    invA.at(0) = a.at(4)*a.at(8) - a.at(5)*a.at(7);
    invA.at(1) = a.at(2)*a.at(7) - a.at(1)*a.at(8);
    invA.at(2) = a.at(1)*a.at(5) - a.at(2)*a.at(4);
    invA.at(3) = a.at(5)*a.at(6) - a.at(3)*a.at(8);
    invA.at(4) = a.at(0)*a.at(8) - a.at(2)*a.at(6);
    invA.at(5) = a.at(2)*a.at(3) - a.at(0)*a.at(5);
    invA.at(6) = a.at(3)*a.at(7) - a.at(4)*a.at(6);
    invA.at(7) = a.at(1)*a.at(6) - a.at(0)*a.at(7);
    invA.at(8) = a.at(0)*a.at(4) - a.at(1)*a.at(3);
    _Tp inv_det = 1./detA;
    M = invA.multiply(inv_det);
    x.x = M.at(0,0)*b.x + M.at(0,1)*b.y + M.at(0,2)*b.z;
    x.y = M.at(1,0)*b.x + M.at(1,1)*b.y + M.at(1,2)*b.z;
    x.z = M.at(2,0)*b.x + M.at(2,1)*b.y + M.at(2,2)*b.z;
    return true;
}

template<typename _Tp>
Vec3<_Tp> HomogeneousTransform(const Mat<_Tp,4> &m, const Vec3<_Tp> &v) {
    _Tp x = m.at(0)*v.x + m.at(1)*v.y + m.at(2)*v.z + m.at(3)*1;
    _Tp y = m.at(4)*v.x + m.at(5)*v.y + m.at(6)*v.z + m.at(7)*1;
    _Tp z = m.at(8)*v.x + m.at(9)*v.y + m.at(10)*v.z + m.at(11)*1;
    _Tp w = m.at(12)*v.x + m.at(13)*v.y + m.at(14)*v.z + m.at(15)*1;;
    if (fabs(w) < 1e-10) w = 1;
    _Tp inv_w = 1./w;
    return Vec3<_Tp>(x*inv_w, y*inv_w, z*inv_w);
}

#endif // MAT_H
