#ifndef MATERIAL_H
#define MATERIAL_H

#include "common.h"
#include "Vec3.h"

class Material {
public:
    Material() {}
    virtual ~Material() {}

    inline void SetReflectance(double r, double g, double b) {
        reflectance_.x = r;
        reflectance_.y = g;
        reflectance_.z = b;
    }

    inline Vec3d GetReflectance() const {
        return reflectance_;
    }

protected:
    Vec3d reflectance_;
};

#endif // MATERIAL_H
