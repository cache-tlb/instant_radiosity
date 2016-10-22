#include "Triangle.h"

bool Triangle::GetIntersection(const Ray& ray, IntersectionInfo* intersection) const {
    const float epsilon = 1e-6;
    Vec3f e1 = v1_ - v3_;
    Vec3f e2 = v2_ - v3_;

    Vec3f h = ray.d.cross(e2);
    float a = e1.dot(h);

    if (fabs(a) < epsilon) return false;

    float f = 1.f/a;
    Vec3f s = ray.o - v3_;
    float u = f*s.dot(h);

    if (u < 0 || u > 1) return false;

    Vec3f q = s.cross(e1);
    float v = f*ray.d.dot(q);

    if (v < 0 || u + v > 1) return false;

    float t = f*e2.dot(q);
    if (t > epsilon) {
        intersection->hit = v3_*(1-u-v) + v1_*u + v2_*v;
        intersection->t = t;
        intersection->object = this;
        return true;
    }else {
        return false;
    }
}
