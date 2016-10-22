#ifndef INTERSECTIONINFO_H
#define INTERSECTIONINFO_H

#include "Vec3.h"

class Shape;

struct IntersectionInfo {
    float t; // Intersection distance along the ray
    const Shape* object; // Object that was hit
    Vec3f hit; // Location of the intersection
};


#endif // INTERSECTIONINFO_H

