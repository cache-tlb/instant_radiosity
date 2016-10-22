#include "GLMesh.h"
#include "VSShaderLibQT.h"
#include "tiny_obj_loader/tiny_obj_loader.h"
#include "util.h"

GLMesh::GLMesh(QOpenGLFunctionsType *env):
    has_coords(true), has_normals(false),
    context(env),
    vao(-1), vertex_coord_buff(-1), texture_coord_buff(-1), normal_buff(-1), tri_index_buff(-1)
{
    // TODO
}

void GLMesh::clear() {
    vertices.clear();
    texture_coords.clear();
    normals.clear();
    triangles.clear();
}

void GLMesh::add_vertex(const Vec3f &pos, const Vec2f &uv, const Vec3f &normal) {
    vertices.push_back(pos.x);
    vertices.push_back(pos.y);
    vertices.push_back(pos.z);
    texture_coords.push_back(uv.x);
    texture_coords.push_back(uv.y);
    normals.push_back(normal.x);
    normals.push_back(normal.y);
    normals.push_back(normal.z);
}

void GLMesh::add_face(const Vec3i &face) {
    triangles.push_back(face.x);
    triangles.push_back(face.y);
    triangles.push_back(face.z);
}

void GLMesh::compile() {
    context->glGenVertexArrays(1, &vao);
    context->glBindVertexArray(vao);

    context->glGenBuffers(1, &vertex_coord_buff);
    context->glBindBuffer(GL_ARRAY_BUFFER, vertex_coord_buff);
    context->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    context->glEnableVertexAttribArray(VSShaderLibQT::VERTEX_COORD_ATTRIB);
    context->glVertexAttribPointer(VSShaderLibQT::VERTEX_COORD_ATTRIB, 3, GL_FLOAT, 0, 0, 0);  // note: it is 3 for xyz !

    if (has_coords) {
        context->glGenBuffers(1, &texture_coord_buff);
        context->glBindBuffer(GL_ARRAY_BUFFER, texture_coord_buff);
        context->glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(float), &texture_coords[0], GL_STATIC_DRAW);
        context->glEnableVertexAttribArray(VSShaderLibQT::TEXTURE_COORD_ATTRIB);
        context->glVertexAttribPointer(VSShaderLibQT::TEXTURE_COORD_ATTRIB, 2, GL_FLOAT, 0, 0, 0);
    }

    if (has_normals) {
        context->glGenBuffers(1, &normal_buff);
        context->glBindBuffer(GL_ARRAY_BUFFER, normal_buff);
        context->glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
        context->glEnableVertexAttribArray(VSShaderLibQT::NORMAL_ATTRIB);
        context->glVertexAttribPointer(VSShaderLibQT::NORMAL_ATTRIB, 3, GL_FLOAT, 0, 0, 0);
    }

    context->glGenBuffers(1, &tri_index_buff);
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tri_index_buff);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), &triangles[0], GL_STATIC_DRAW);

    context->glBindVertexArray(0);
}

GLMesh *GLMesh::plane(QOpenGLFunctionsType *context, int detailX /* = 1 */, int detailY /* = 1 */) {
    GLMesh *mesh = new GLMesh(context);
    for (int y = 0; y <= detailY; y++) {
        float t = (float)y / (float)detailY;
        for (int x = 0; x <= detailX; x++) {
            float s = (float)x / (float)detailX;
            mesh->vertices.push_back(2.f * s - 1.f);
            mesh->vertices.push_back(2.f * t - 1.f);
            mesh->vertices.push_back(0.f);
            // texture coords
            mesh->texture_coords.push_back(s);
            mesh->texture_coords.push_back(t);
            // normals
            mesh->normals.push_back(0.f);
            mesh->normals.push_back(0.f);
            mesh->normals.push_back(1.f);
            if (x < detailX && y < detailY) {
                int i = x + y * (detailX + 1);
                mesh->triangles.push_back(i);
                mesh->triangles.push_back(i + 1);
                mesh->triangles.push_back(i + detailX + 1);
                mesh->triangles.push_back(i + detailX + 1);
                mesh->triangles.push_back(i + 1);
                mesh->triangles.push_back(i + detailX + 2);
            }
        }
    }
    mesh->has_normals = true;
    mesh->has_coords = true;
    mesh->compile();
    return mesh;
}

static int cubeData[6][7] = {
    {0, 4, 2, 6, -1, 0, 0},
    {1, 3, 5, 7, +1, 0, 0},
    {0, 1, 4, 5, 0, -1, 0},
    {2, 6, 3, 7, 0, +1, 0},
    {0, 2, 1, 3, 0, 0, -1},
    {4, 5, 6, 7, 0, 0, +1}
};

void pickOctant(int input, int &x, int &y, int &z) {
    x = (input & 1) * 2 - 1;
    y = (input & 2) - 1;
    z = (input & 4) / 2 - 1;
}

GLMesh *GLMesh::cube(QOpenGLFunctionsType *context) {
    GLMesh *mesh = new GLMesh(context);

    for (int i = 0; i < 6; i++) {
        int *data = cubeData[i], v = i * 4;
        for (int j = 0; j < 4; j++) {
            int d = data[j];
            int x, y, z;
            pickOctant(d, x, y, z);
            mesh->vertices.push_back(x);
            mesh->vertices.push_back(y);
            mesh->vertices.push_back(z);
            // TODO: add coord
            int nx = cubeData[i][4], ny = cubeData[i][5], nz = cubeData[i][6];
            mesh->normals.push_back(nx);
            mesh->normals.push_back(ny);
            mesh->normals.push_back(nz);
        }
        mesh->triangles.push_back(v);
        mesh->triangles.push_back(v + 1);
        mesh->triangles.push_back(v + 2);
        mesh->triangles.push_back(v + 2);
        mesh->triangles.push_back(v + 1);
        mesh->triangles.push_back(v + 3);
    }
    mesh->has_normals = true;
    mesh->compile();
    return mesh;
}

GLMesh *GLMesh::sphere(QOpenGLFunctionsType *context, int detail /* = 6 */) {
    auto tri = [](bool flip, int a, int b, int c){ return flip ? std::tuple<int,int,int>(a, c, b) : std::tuple<int,int,int>(a, b, c); };
    auto fix = [](float x){ return x + (x - x * x) / 2.f; };
    GLMesh *mesh = new GLMesh(context);
    Indexer<float> indexer;

    for (int octant = 0; octant < 8; octant++) {
        int sx, sy, sz;
        pickOctant(octant, sx, sy, sz);
        bool flip = sx*sy*sz > 0;
        std::vector<int> data;
        for (int i = 0; i <= detail; i++) {
            // Generate a row of vertices on the surface of the sphere
            // using barycentric coordinates.
            for (int j = 0; i + j <= detail; j++) {
                float a = (float)i / (float)detail;
                float b = (float)j / (float)detail;
                float c = float(detail - i - j) / (float)detail;
                float vx = fix(a), vy = fix(b), vz = fix(c);
                double invlen = 1.0 / sqrt(double(vx*vx + vy*vy + vz*vz));
                vx *= (invlen*sx); vy *= (invlen*sy); vz *= (invlen*sz);
                // TODO: coords
                data.push_back(indexer.add(Vec3<float>(vx,vy,vz)));
            }

            // Generate triangles from this row and the previous row.
            if (i > 0) {
                for (int j = 0; i + j <= detail; j++) {
                    int a = (i - 1) * (detail + 1) + ((i - 1) - (i - 1) * (i - 1)) / 2 + j;
                    int b = i * (detail + 1) + (i - i * i) / 2 + j;
                    std::tuple<int,int,int> t = tri(flip, data[a], data[a + 1], data[b]);
                    mesh->triangles.push_back(std::get<0>(t));
                    mesh->triangles.push_back(std::get<1>(t));
                    mesh->triangles.push_back(std::get<2>(t));
                    if (i + j < detail) {
                        std::tuple<int,int,int> t2 = tri(flip, data[b], data[a + 1], data[b + 1]);
                        mesh->triangles.push_back(std::get<0>(t2));
                        mesh->triangles.push_back(std::get<1>(t2));
                        mesh->triangles.push_back(std::get<2>(t2));
                    }
                }
            }
        }
    }

    // Reconstruct the geometry from the indexer.
    for (int i = 0; i < indexer.unique.size(); i++) {
        auto &v = indexer.unique[i];
        mesh->vertices.push_back(v.x);
        mesh->vertices.push_back(v.y);
        mesh->vertices.push_back(v.z);
        // TODO: coords
        mesh->normals.push_back(v.x);
        mesh->normals.push_back(v.z);
        mesh->normals.push_back(v.y);
    }
    mesh->has_normals = true;
    mesh->compile();
    return mesh;
}

void GLMesh::LoadFromObjFile(QOpenGLFunctionsType *context, const std::string &filepath, const std::string &mtl_base_path, std::vector<GLMesh*> &meshes, std::vector<Material*> &material_ptrs) {
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool ok = tinyobj::LoadObj(shapes, materials, err, filepath.c_str(), mtl_base_path.c_str());
    if (!ok) {
        std::cout << "Error in Load Obj: " << err << std::endl;
    }

    /*
    std::cout << "# of shapes    : " << shapes.size() << std::endl;
    std::cout << "# of materials : " << materials.size() << std::endl;

    for (size_t i = 0; i < shapes.size(); i++) {
        printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
        printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
        printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
        assert((shapes[i].mesh.indices.size() % 3) == 0);
        for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
            printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, shapes[i].mesh.indices[3*f+0], shapes[i].mesh.indices[3*f+1], shapes[i].mesh.indices[3*f+2], shapes[i].mesh.material_ids[f]);
        }

        printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
        assert((shapes[i].mesh.positions.size() % 3) == 0);
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            printf("  v[%ld] = (%f, %f, %f)\n", v,
                   shapes[i].mesh.positions[3*v+0],
                   shapes[i].mesh.positions[3*v+1],
                   shapes[i].mesh.positions[3*v+2]);
        }
    }

    for (size_t i = 0; i < materials.size(); i++) {
        printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
        printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
        printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
        printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
        printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
        printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
        printf("  material.Ns = %f\n", materials[i].shininess);
        printf("  material.Ni = %f\n", materials[i].ior);
        printf("  material.dissolve = %f\n", materials[i].dissolve);
        printf("  material.illum = %d\n", materials[i].illum);
        printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
        printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
        printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
        printf("  material.map_Ns = %s\n", materials[i].specular_highlight_texname.c_str());
        std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
        std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
        for (; it != itEnd; it++) {
            printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
        }
        printf("\n");
    }
    */

    // suppose each object shares the same material
    meshes.clear();
    material_ptrs.clear();
    for (size_t i = 0; i < shapes.size(); i++) {
        GLMesh *mesh = new GLMesh(context);
        bool has_normal = !shapes[i].mesh.normals.empty();
        bool has_texture = !shapes[i].mesh.texcoords.empty();
        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
            mesh->vertices.push_back(shapes[i].mesh.positions[3*v+0]);
            mesh->vertices.push_back(shapes[i].mesh.positions[3*v+1]);
            mesh->vertices.push_back(shapes[i].mesh.positions[3*v+2]);
            if (has_normal) {
                mesh->normals.push_back(shapes[i].mesh.normals[3*v+0]);
                mesh->normals.push_back(shapes[i].mesh.normals[3*v+1]);
                mesh->normals.push_back(shapes[i].mesh.normals[3*v+2]);
            }
            if (has_texture) {
                mesh->texture_coords.push_back(shapes[i].mesh.texcoords[2*v+0]);
                mesh->texture_coords.push_back(shapes[i].mesh.texcoords[2*v+1]);
            }
        }

        for (int f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
            mesh->triangles.push_back(shapes[i].mesh.indices[3*f+0]);
            mesh->triangles.push_back(shapes[i].mesh.indices[3*f+1]);
            mesh->triangles.push_back(shapes[i].mesh.indices[3*f+2]);
        }

        if (!has_normal) {
            // insert the normal value
            int n_vertex = shapes[i].mesh.positions.size() / 3;
            std::vector<std::vector<Vec3d> > vertex_normals(n_vertex);
            std::vector<std::vector<double> > areas(n_vertex);
            for (int idx = 0; idx < n_vertex; idx++) {
                areas[idx].clear();
                vertex_normals[idx].clear();
            }
            for (int f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
                int id1 = shapes[i].mesh.indices[3*f+0], id2 = shapes[i].mesh.indices[3*f+1], id3 = shapes[i].mesh.indices[3*f+2];
                assert(id1 < n_vertex && id2 < n_vertex && id3 < n_vertex);
                double v1[3], v2[3], v3[3];
                for (int j = 0; j < 3; j++) {
                    v1[j] = shapes[i].mesh.positions[3*id1 + j];
                    v2[j] = shapes[i].mesh.positions[3*id2 + j];
                    v3[j] = shapes[i].mesh.positions[3*id3 + j];
                }
                Vec3d v3d1(v1), v3d2(v2), v3d3(v3);
                Vec3d norm_vec3;
                plane_normal(v3d1, v3d2, v3d3, norm_vec3);
                double area = triangle_area(v3d1, v3d2, v3d3);
                vertex_normals[id1].push_back(norm_vec3);
                vertex_normals[id2].push_back(norm_vec3);
                vertex_normals[id3].push_back(norm_vec3);
                areas[id1].push_back(area);
                areas[id2].push_back(area);
                areas[id3].push_back(area);
            }
            std::vector<Vec3d> res_normal(n_vertex);
            for (int idx = 0; idx < vertex_normals.size(); idx++) {
                double sum_area = 0;
                for (int j = 0; j < areas[idx].size(); j++) {
                    sum_area += areas[idx][j];
                }
                if (sum_area == 0) {
                    res_normal[idx] = Vec3d(1,0,0);
                    continue;
                }
                double inv_sum = 1./sum_area;
                Vec3d normal(0,0,0);
                for (int j = 0; j < vertex_normals[idx].size(); j++) {
                    double weight = inv_sum * areas[idx][j];
                    normal.x += weight*vertex_normals[idx][j].x;
                    normal.y += weight*vertex_normals[idx][j].y;
                    normal.z += weight*vertex_normals[idx][j].z;
                }
                res_normal[idx] = normal.unit();
            }
            for (int v = 0; v < n_vertex; v++) {
                mesh->normals.push_back(res_normal[v].x);
                mesh->normals.push_back(res_normal[v].y);
                mesh->normals.push_back(res_normal[v].z);
            }
        }
        mesh->has_coords = has_texture;
        mesh->has_normals = true;
        mesh->compile();
        meshes.push_back(mesh);

        Material *mat_ptr = new Material();
        int mat_id = shapes[i].mesh.material_ids[0];
        for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
            if (mat_id != shapes[i].mesh.material_ids[f]) {
                printf("warning: material id varys in same obj mesh\n");
                break;
            }
        }
        tinyobj::material_t &obj_mat = materials[mat_id];
        mat_ptr->SetReflectance(obj_mat.diffuse[0], obj_mat.diffuse[1], obj_mat.diffuse[2]);
        material_ptrs.push_back(mat_ptr);
    }
}
