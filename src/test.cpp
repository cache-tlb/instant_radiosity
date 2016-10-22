#include "GLMesh.h"

void test_obj_loader() {
    std::string obj_path = "C:/Users/liubin/Documents/mitsuba/cornell_box/cornell_box.obj";
    std::string mtl_path = "C:/Users/liubin/Documents/mitsuba/cornell_box/";
    std::vector<GLMesh*> meshes;
    std::vector<Material*> materals;
    GLMesh::LoadFromObjFile(NULL, obj_path, mtl_path, meshes, materals);
}

void test() {
    test_obj_loader();
}

