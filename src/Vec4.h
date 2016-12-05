#ifndef VEC4_H
#define VEC4_H

template<typename _Tp>
class Vec4 {
public:
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(_Tp _x, _Tp _y, _Tp _z, _Tp _w) : x(_x), y(_y), z(_z), w(_w) {}

    template<typename _Tp2>
    Vec4(const Vec4<_Tp2> &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    template<typename _Tp2>
    Vec4 & operator = (const Vec4<_Tp2> &rhs) {
        if (this != &rhs) {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
            w = rhs.w;
        }
        return *this;
    }

    _Tp operator [] (int index) const {
        return val[index];
    }

    _Tp &operator [] (int index) {
        return val[index];
    }

    Vec4 unit() const {
        _Tp len = length();
        if (len < 1e-8) return Vec4<_Tp>();
        _Tp invlen = 1./len;
        return Vec4(x*invlen, y*invlen, z*invlen, w*invlen);
    }

    _Tp length() const {
        return _Tp(sqrt(x*x + y*y + z*z + w*w));
    }

    _Tp length_sqr() const {
        return x*x + y*y + z*z + w*w;
    }

    Vec4 multiply(const _Tp f) const {
        return Vec4<_Tp>(x*f, y*f, z*f, w*f);
    }
    Vec4 operator * (const _Tp f) const {
        return Vec4<_Tp>(x*f, y*f, z*f, w*f);
    }

    Vec4 divide(const _Tp f) const {
        _Tp inv_f = _Tp(1)/f;
        return Vec4<_Tp>(x*inv_f, y*inv_f, z*inv_f, w*inv_f);
    }
    Vec4 operator / (const _Tp f) const {
        _Tp inv_f = _Tp(1)/f;
        return Vec4<_Tp>(x*inv_f, y*inv_f, z*inv_f, w*inv_f);
    }

    void toMartix(_Tp m[]) const {
        _Tp sqw = w*w;
        _Tp sqx = x*x;
        _Tp sqy = y*y;
        _Tp sqz = z*z;

        // invs (inverse square length) is only required if quaternion is not already normalised
        _Tp invs = 1. / (sqx + sqy + sqz + sqw);
        _Tp m00 = ( sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
        _Tp m11 = (-sqx + sqy - sqz + sqw)*invs;
        _Tp m22 = (-sqx - sqy + sqz + sqw)*invs;

        _Tp tmp1 = x*y;
        _Tp tmp2 = z*w;
        _Tp m10 = 2.0 * (tmp1 + tmp2)*invs;
        _Tp m01 = 2.0 * (tmp1 - tmp2)*invs;

        tmp1 = x*z;
        tmp2 = y*w;
        _Tp m20 = 2.0 * (tmp1 - tmp2)*invs;
        _Tp m02 = 2.0 * (tmp1 + tmp2)*invs;
        tmp1 = y*z;
        tmp2 = x*w;
        _Tp m21 = 2.0 * (tmp1 + tmp2)*invs;
        _Tp m12 = 2.0 * (tmp1 - tmp2)*invs;

        m[0] = m00;
        m[1] = m01;
        m[2] = m02;
        m[3] = m10;
        m[4] = m11;
        m[5] = m12;
        m[6] = m20;
        m[7] = m21;
        m[8] = m22;
    }

    static Vec4 fromAxisAngle(_Tp axis_x, _Tp axis_y, _Tp axis_z, _Tp angle) {
        _Tp half_angle = angle / 2;
        _Tp s = sin(half_angle);
        return Vec4(axis_x*s, axis_y*s, axis_z*s, cos(half_angle));
    }

    static Vec4 fromMatrix(_Tp m[]) {
        Vec4 q;
        _Tp trace = m[0] + m[4] + m[8];   // I removed + 1.0f; see discussion with Ethan
        if( trace > 0 ) {                   // I changed M_EPSILON to 0
            _Tp s = 0.5f / sqrt(trace+ 1.0f);
            q.w = 0.25f / s;
            q.x = ( m[7] - m[5] ) * s;
            q.y = ( m[2] - m[6] ) * s;
            q.z = ( m[3] - m[1] ) * s;
        } else {
            if ( m[0] > m[4] && m[0] > m[8] ) {
                _Tp s = 2.0f * sqrt( 1.0f + m[0] - m[4] - m[8]);
                q.w = (m[7] - m[5] ) / s;
                q.x = 0.25f * s;
                q.y = (m[1] + m[3] ) / s;
                q.z = (m[2] + m[6] ) / s;
            } else if (m[4] > m[8]) {
                _Tp s = 2.0f * sqrt( 1.0f + m[4] - m[0] - m[8]);
                q.w = (m[2] - m[6] ) / s;
                q.x = (m[1] + m[3] ) / s;
                q.y = 0.25f * s;
                q.z = (m[5] + m[7] ) / s;
            } else {
                _Tp s = 2.0f * sqrt( 1.0f + m[8] - m[0] - m[4]);
                q.w = (m[3] - m[1] ) / s;
                q.x = (m[2] + m[6] ) / s;
                q.y = (m[5] + m[7] ) / s;
                q.z = 0.25f * s;
            }
        }
        return q;
    }

public:
    union {
        struct { _Tp x, y, z, w; };
        _Tp val[4];
    };
};

typedef Vec4<float> Vec4f;
typedef Vec4<double> Vec4d;

#endif // VEC4_H
