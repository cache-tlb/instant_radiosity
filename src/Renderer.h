#ifndef RENDERER_H
#define RENDERER_H

#include "GLMesh.h"
#include "Shader.h"
#include "GLTexture.h"
#include "GLCubeMap.h"
#include "Triangle.h"
#include "BVH.h"

class Renderer {
public:
    Renderer(QOpenGLFunctionsType *context, VSMathLibQT *vsml);

    void Render();
    void Init();


    void set_near_clip(float near_clip) {
        near_clip_ = near_clip;
    }

    void set_far_clip(float far_clip) {
        far_clip_ = far_clip;
    }

    void set_window_dim(int width, int height) {
        window_height_ = height;
        window_width_ = width;
    }

    void ToggleShowMedia() {
        is_show_media = !is_show_media;
    }

    void SetCamera(const Vec3f &eye, const Vec3f &look_at, const Vec3f &up);

protected:

    void RenderSingleLight(int light_id);
    void RenderDepthCube(int light_id);

    void BuildBVH();
    void DistributeVPLs(int num, int depth);    // depth -> 0: direct, 1~3: k boundces

    void VPLInit();
    void SSAOInit();
    void VSInit();

    void VPLRender();
    void SSAORender();
    void VolumetricScatteringRender();

    QOpenGLFunctionsType *context;
    VSMathLibQT *vsml;

    Shader *model_shader_;
    Shader *shadow_map_shader_;
    Shader *show_texture_shader_;
    Shader *acc_texture_shader_;

    float near_clip_, far_clip_;
    int window_width_, window_height_;

    Vec3f eye_, look_at_, up_;
    Vec3f light_pos_;
    std::vector<Vec3f> light_poses_;
    std::vector<Vec3f> light_colors_;
    std::vector<double> light_weights_;
    std::vector<Vec3f> light_directions_;

    std::vector<GLMesh*> scene_meshes_;
    std::vector<Material*> materals_;

    // for bvh
    std::vector<Triangle> primitives_;
    std::vector<Shape*> primitive_ptrs_;
    BVH* bvh_;

    // for accumulate buffer
    GLTexture *read_buf_, *write_buf_, *single_pass_;
    GLMesh *screen_plane_;

    Shader *cubemap_gen_shader_, *cubemap_show_shader_;
    GLMesh *sphere_mesh_, *cube_mash_;
    GLCubeMap *cubemap_texture_;

    Shader *gbuffer_shader_, *ssao_shader_;
    GLTexture *gbuffer_normal_texture_, *gbuffer_pos_texture_, *gbuffer_depth_texture_, *gbuffer_albedo_texture_;
    bool is_show_media;

    Shader *vs_shader_;

};

#endif // RENDERER_H
