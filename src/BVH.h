#ifndef BVH_H
#define BVH_H


#include "BBox.h"
#include <vector>
#include <stdint.h>
#include "Shape.h"
#include "IntersectionInfo.h"
#include "Ray.h"

//! Node descriptor for the flattened tree
struct BVHFlatNode {
    BBox bbox;
    uint32_t start, nPrims, rightOffset;
};

//! \author Brandon Pelfrey
//! A Bounding Volume Hierarchy system for fast Ray-Object intersection tests
class BVH {
    uint32_t nNodes, nLeafs, leafSize;
    std::vector<Shape*>* build_prims;

    //! Build the BVH tree out of build_prims
    void Build();

    // Fast Traversal System
    BVHFlatNode *flatTree;

public:
    BVH(std::vector<Shape*>* objects, uint32_t leafSize=4);
    bool GetIntersection(const Ray& ray, IntersectionInfo *intersection, bool occlusion) const ;

    ~BVH();
};

#endif // BVH_H
