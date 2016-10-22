#ifndef SHAPE_H
#define SHAPE_H

#include "IntersectionInfo.h"
#include "Ray.h"
#include "BBox.h"
#include "Material.h"

class Shape {
public:
    Shape() {}
    virtual ~Shape() = 0;

    virtual bool GetIntersection(const Ray& ray, IntersectionInfo* intersection) const = 0;

    virtual Vec3f GetNoamal(const IntersectionInfo& I) const = 0;

    virtual BBox GetBBox() const = 0;

    virtual Vec3f GetCentroid() const = 0;

    virtual Material *GetMaterial() const;
};

#endif // SHAPE_H

