#ifndef BBOX_H
#define BBOX_H

#include "common.h"
#include "Ray.h"
#include "Vec3.h"
#include <stdint.h>

struct BBox {
    typedef Vec3f Vector3;

    Vector3 min_p, max_p, extent;
    BBox() { }
    BBox(const Vector3& min_p, const Vector3& max_p);
    BBox(const Vector3& p);

    bool intersect(const Ray& ray, float *tnear, float *tfar) const;
    void expandToInclude(const Vector3& p);
    void expandToInclude(const BBox& b);
    uint32_t maxDimension() const;
    float surfaceArea() const;
};

#endif // BBOX_H
