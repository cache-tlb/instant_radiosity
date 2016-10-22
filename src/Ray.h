#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

struct Ray {
    typedef Vec3f Vector3;

    Vector3 o; // Ray Origin
    Vector3 d; // Ray Direction
    Vector3 inv_d; // Inverse of each Ray Direction component

    Ray(const Vector3& o, const Vector3& d)
    : o(o), d(d), inv_d(1.f/d.x, 1.f/d.y, 1.f/d.z) { }
};

#endif // RAY_H

