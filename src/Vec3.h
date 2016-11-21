#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <algorithm>

template<typename _Tp>
class Vec3 {
public:
    Vec3() : x(0), y(0), z(0) {}
    Vec3(_Tp _x, _Tp _y, _Tp _z) : x(_x), y(_y), z(_z) {}

    explicit Vec3(const _Tp *array) : x(array[0]), y(array[1]), z(array[2]) {}

    template<typename _Tp2>
    Vec3(const Vec3<_Tp2> &v) : x(v.x), y(v.y), z(v.z) {}

    template<typename _Tp2>
    Vec3 & operator = (const Vec3<_Tp2> &rhs) {
        if (this != &rhs) {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
        }
        return *this;
    }

    bool operator < (const Vec3<_Tp> &that) const {
        return (x < that.x) || (x == that.x && y < that.y) || (x == that.x && y == that.y && z < that.z);
    }

    bool operator == (const Vec3<_Tp> &that) const {
        return (x == that.x && y == that.y && z == that.z);
    }

    _Tp operator [] (int index) const {
        return val[index];
    }

    _Tp &operator [] (int index) {
        return val[index];
    }

    Vec3 unit() const {
        _Tp len = length();
        if (len < 1e-8) return Vec3<_Tp>();
        double invlen = 1./len;
        return Vec3(x*invlen, y*invlen, z*invlen);
    }

    _Tp length() const {
        return _Tp(sqrt(x*x + y*y + z*z));
    }

    _Tp dot(const Vec3<_Tp> &v) const {
        return x*v.x + y*v.y + z*v.z;
    }

    Vec3 add(const Vec3<_Tp> &v) const {
        return Vec3<_Tp>(x + v.x, y + v.y, z + v.z);
    }
    Vec3 operator + (const Vec3<_Tp>  &v) const {
        return Vec3<_Tp>(x + v.x, y + v.y, z + v.z);
    }
    Vec3 add(const _Tp f) const {
        return Vec3<_Tp>(x + f, y + f, z + f);
    }
    Vec3 operator + (const _Tp f) const {
        return Vec3<_Tp>(x + f, y + f, z + f);
    }

    Vec3 subtract(const Vec3<_Tp> &v) const {
        return Vec3<_Tp>(x - v.x,  y - v.y, z - v.z);
    }
    Vec3 operator - (const Vec3<_Tp> &v) const {
        return Vec3<_Tp>(x - v.x,  y - v.y, z - v.z);
    }
    Vec3 subtract(const _Tp f) const {
        return Vec3<_Tp>(x - f, y - f, z - f);
    }
    Vec3 operator - (const _Tp f) const {
        return Vec3<_Tp>(x - f, y - f, z - f);
    }

    Vec3 multiply(const _Tp f) const {
        return Vec3<_Tp>(x*f, y*f, z*f);
    }
    Vec3 operator * (const _Tp f) const {
        return Vec3<_Tp>(x*f, y*f, z*f);
    }

    Vec3 divide(const _Tp f) const {
        _Tp inv_f = _Tp(1)/f;
        return Vec3<_Tp>(x*inv_f, y*inv_f, z*inv_f);
    }
    Vec3 operator / (const _Tp f) const {
        _Tp inv_f = _Tp(1)/f;
        return Vec3<_Tp>(x*inv_f, y*inv_f, z*inv_f);
    }


    Vec3 cross(const Vec3<_Tp> &v) const {
        return Vec3<_Tp>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
    }

    template<typename _Tp2>
    void toArray(_Tp2 a[3]) const {
        a[0] = x;
        a[1] = y;
        a[2] = z;
    }

    static Vec3<_Tp> lerp(const Vec3<_Tp> &a, const Vec3<_Tp> &b, float t) {
        _Tp x = a.x + (b.x - a.x) * t;
        _Tp y = a.y + (b.y - a.y) * t;
        _Tp z = a.z + (b.z - a.z) * t;
        return Vec3<_Tp>(x, y, z);
    }

    static Vec3 fromAngles(_Tp theta, _Tp phi) {
        return Vec3<_Tp>(cos(theta) * cos(phi), sin(phi), sin(theta) * cos(phi));
    }

public:
    union {
        struct { _Tp x, y, z; };
        _Tp val[3];
    };
};

// Component-wise min
template<typename _Tp>
inline Vec3<_Tp> min(const Vec3<_Tp>& a, const Vec3<_Tp>& b) {
  return Vec3<_Tp>(std::min<_Tp>(a.x, b.x), std::min<_Tp>(a.y, b.y), std::min<_Tp>(a.z, b.z));
}

// Component-wise max
template<typename _Tp>
inline Vec3<_Tp> max(const Vec3<_Tp>& a, const Vec3<_Tp>& b) {
  return Vec3<_Tp>(std::max<_Tp>(a.x, b.x), std::max<_Tp>(a.y, b.y), std::max<_Tp>(a.z, b.z));
}

template<typename _Tp>
Vec3<_Tp> operator * (const _Tp f, const Vec3<_Tp> &v) {
    return Vec3<_Tp>(f*v.x, f*v.y, f*v.z);
}

typedef Vec3<float> Vec3f;
typedef Vec3<double> Vec3d;
typedef Vec3<int> Vec3i;

#endif // VEC3_H
