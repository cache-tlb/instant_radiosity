#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Shape.h"
#include "util.h"

class Triangle : public Shape {
public:
    Triangle() {}
    Triangle(const Vec3f &v1, const Vec3f &v2, const Vec3f &v3)
        : v1_(v1), v2_(v2), v3_(v3), bbox_(v1), centroid_((v1+v2+v3)*(1./3.)) {
        bbox_.expandToInclude(v2);
        bbox_.expandToInclude(v3);
        plane_normal<float>(v1,v2,v3,default_noraml_);
    }
    virtual ~Triangle() {}

    virtual bool GetIntersection(const Ray& ray, IntersectionInfo* intersection) const;

    virtual Vec3f GetNoamal(const IntersectionInfo& I) const {
        return default_noraml_;
    }

    virtual BBox GetBBox() const {
        return bbox_;
    }

    virtual Vec3f GetCentroid() const {
        return centroid_;
    }
    virtual Material *GetMaterial() const {
        return material_ptr_;
    }

public:
    Vec3f v1_, v2_, v3_;
    BBox bbox_;
    Vec3f centroid_;
    Vec3f default_noraml_;
    Material *material_ptr_;
};

#endif // TRIANGLE_H
