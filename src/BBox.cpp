#include "BBox.h"
#include <algorithm>

BBox::BBox(const Vector3& min_p, const Vector3& max_p)
  : min_p(min_p), max_p(max_p) { extent = max_p - min_p; }

BBox::BBox(const Vector3& p)
  : min_p(p), max_p(p) { extent = max_p - min_p; }

void BBox::expandToInclude(const Vector3& p) {
    min_p = ::min(min_p, p);
    max_p = ::max(max_p, p);
    extent = max_p - min_p;
}

void BBox::expandToInclude(const BBox& b) {
    min_p = ::min(min_p, b.min_p);
    max_p = ::max(max_p, b.max_p);
    extent = max_p - min_p;
}

uint32_t BBox::maxDimension() const {
    uint32_t result = 0;
    if(extent.y > extent.x) result = 1;
    if(extent.z > extent.y) result = 2;
    return result;
}

float BBox::surfaceArea() const {
    return 2.f*( extent.x*extent.z + extent.x*extent.y + extent.y*extent.z );
}

bool BBox::intersect(const Ray& ray, float *tnear, float *tfar) const {
    float t1 = (min_p.x - ray.o.x)*ray.inv_d.x;
    float t2 = (max_p.x - ray.o.x)*ray.inv_d.x;
    float t3 = (min_p.y - ray.o.y)*ray.inv_d.y;
    float t4 = (max_p.y - ray.o.y)*ray.inv_d.y;
    float t5 = (min_p.z - ray.o.z)*ray.inv_d.z;
    float t6 = (max_p.z - ray.o.z)*ray.inv_d.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    if (tmax < 0) {
//        t = tmax;
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax) {
//        t = tmax;
        return false;
    }

    *tnear = tmin;
    *tfar = tmax;
    return true;
}
