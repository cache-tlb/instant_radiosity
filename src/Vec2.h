#ifndef VEC2_H
#define VEC2_H


template<typename _Tp>
class Vec2 {
public:
    Vec2() : x(0), y(0) {}
    Vec2(_Tp _x, _Tp _y) : x(_x), y(_y) {}

    template<typename _Tp2>
    Vec2(const Vec2<_Tp2> &v) : x(v.x), y(v.y){}

    template<typename _Tp2>
    Vec2 & operator = (const Vec2<_Tp2> &rhs) {
        if (this != &rhs) {
            x = rhs.x;
            y = rhs.y;
        }
        return *this;
    }

    bool operator < (const Vec2<_Tp> &that) const {
        return (x < that.x) || (x == that.x && y < that.y);
    }

    bool operator == (const Vec2<_Tp> &that) const {
        return (x == that.x) && (y == that.y);
    }

    _Tp length() const {
        return _Tp(sqrt(x*x + y*y));
    }

    _Tp dot(const Vec2<_Tp> &v) const {
        return x*v.x + y*v.y;
    }

    Vec2 add(const Vec2<_Tp> &v) const {
        return Vec2<_Tp>(x + v.x, y + v.y);
    }
    Vec2 operator + (const Vec2<_Tp>  &v) const {
        return Vec2<_Tp>(x + v.x, y + v.y);
    }
    Vec2 add(const _Tp f) const {
        return Vec2<_Tp>(x + f, y + f);
    }
    Vec2 operator + (const _Tp f) const {
        return Vec2<_Tp>(x + f, y + f);
    }

    Vec2 subtract(const Vec2<_Tp> &v) const {
        return Vec2<_Tp>(x - v.x,  y - v.y);
    }
    Vec2 operator - (const Vec2<_Tp> &v) const {
        return Vec2<_Tp>(x - v.x,  y - v.y);
    }
    Vec2 subtract(const _Tp f) const {
        return Vec2<_Tp>(x - f, y - f);
    }
    Vec2 operator - (const _Tp f) const {
        return Vec2<_Tp>(x - f, y - f);
    }

public:
    union {
        struct { _Tp x, y; };
        _Tp val[2];
    };
};

typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;
typedef Vec2<int> Vec2i;


#endif // VEC2_H
