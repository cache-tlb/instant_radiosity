#include "Renderer.h"

Renderer::Renderer(QOpenGLFunctionsType *context, VSMathLibQT *vsml):
    context(context), vsml(vsml),
    model_shader_(NULL), shadow_map_shader_(NULL),
    show_texture_shader_(NULL), acc_texture_shader_(NULL),
    read_buf_(NULL), write_buf_(NULL), single_pass_(NULL),
    screen_plane_(NULL),
    gbuffer_shader_(NULL), ssao_shader_(NULL),
    gbuffer_normal_texture_(NULL), gbuffer_pos_texture_(NULL), gbuffer_depth_texture_(NULL),
    bvh_(NULL),
    near_clip_(1e-2), far_clip_(1e3),
    window_width_(512), window_height_(512),
    eye_(0,0,0), look_at_(0,0,-1), up_(0,1,0),
    light_pos_(278/20., 548/20.-0.1, 279.5/20.-0.1)
{

}

void Renderer::BuildBVH() {
    primitives_.clear();
    for (int k = 0; k < scene_meshes_.size(); k++) {
        GLMesh *mesh = scene_meshes_[k];
        for (int triangle_id = 0; triangle_id*3 < mesh->triangles.size(); triangle_id++) {
            int id1 = mesh->triangles[triangle_id*3+0];
            int id2 = mesh->triangles[triangle_id*3+1];
            int id3 = mesh->triangles[triangle_id*3+2];
            Vec3f v1 = Vec3f(mesh->vertices[3*id1 + 0], mesh->vertices[3*id1 + 1], mesh->vertices[3*id1 + 2]);
            Vec3f v2 = Vec3f(mesh->vertices[3*id2 + 0], mesh->vertices[3*id2 + 1], mesh->vertices[3*id2 + 2]);
            Vec3f v3 = Vec3f(mesh->vertices[3*id3 + 0], mesh->vertices[3*id3 + 1], mesh->vertices[3*id3 + 2]);
            Triangle tri(v1*(1./20),v2*(1./20),v3*(1./20));
            tri.material_ptr_ = materals_[k];
            primitives_.push_back(tri);
        }
    }

    primitive_ptrs_.resize(primitives_.size());
    for (int i = 0; i < primitives_.size(); i++) {
        primitive_ptrs_[i] = &primitives_[i];
    }
    bvh_ = new BVH(&primitive_ptrs_);
}

void Renderer::DistributeVPLs() {
    light_colors_.clear();
    light_poses_.clear();
    light_weights_.clear();
    light_directions_.clear();

    int target_vpl_number = 512*4;
    double delta = 0.01;
    Vec3f light_dir(0,-1,0);
    while (light_poses_.size() < target_vpl_number) {
        light_pos_ = Vec3f(213/20. + (343-213)/20.*rand()/RAND_MAX, 548/20.-0.5, 227/20. + (332-227)/20.*rand()/RAND_MAX);
        light_colors_.push_back(Vec3f(1.0, 1.0, 1.0));
        light_poses_.push_back(light_pos_);
        light_weights_.push_back(1. / double(target_vpl_number)*6);
        light_directions_.push_back(light_dir);
        Vec3f sample_dir = CosWeightedHemisphereSample(light_dir);
        Ray ray(light_pos_, sample_dir);
        IntersectionInfo intersection;
        bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
        if (!is_intersect) continue;
        Material *mat = intersection.object->GetMaterial();
        Vec3f normal = intersection.object->GetNoamal(intersection);
        if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
        Vec3f color = mat->GetReflectance() * fabs(normal.dot(sample_dir));
        light_poses_.push_back(intersection.hit + normal*delta);
        light_colors_.push_back(color);
        light_weights_.push_back(1. / double(target_vpl_number)*18);
        light_directions_.push_back(normal);

        do {
            Vec3f sample_dir = CosWeightedHemisphereSample(normal);
            Ray ray(intersection.hit + normal*delta, sample_dir);
            bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
            if (!is_intersect) continue;
            Material *mat = intersection.object->GetMaterial();
            Vec3f normal = intersection.object->GetNoamal(intersection);
            if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
            Vec3f color2 = mat->GetReflectance() * fabs(normal.dot(sample_dir));
            light_poses_.push_back(intersection.hit + normal*delta);
            Vec3f c2(color.x*color2.x, color.y*color2.y, color.z*color2.z);
            light_colors_.push_back(c2);
            light_weights_.push_back(1. / double(target_vpl_number)*18);
            light_directions_.push_back(normal);
            do {
                Vec3f sample_dir = CosWeightedHemisphereSample(normal);
                Ray ray(intersection.hit + normal*delta, sample_dir);
                bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
                if (!is_intersect) continue;
                Material *mat = intersection.object->GetMaterial();
                Vec3f normal = intersection.object->GetNoamal(intersection);
                if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
                Vec3f color3 = mat->GetReflectance() * fabs(normal.dot(sample_dir));
                light_poses_.push_back(intersection.hit + normal*delta);
                Vec3f c3(color2.x*color3.x, color2.y*color3.y, color2.z*color3.z);
                light_colors_.push_back(c3);
                light_weights_.push_back(1. / double(target_vpl_number)*18);
                light_directions_.push_back(normal);
            } while (false);
        } while (false);
    }
}

void Renderer::VPLInit() {
    BuildBVH();
    DistributeVPLs();
}

void Renderer::SSAOInit() {
    gbuffer_shader_ = new Shader(context, vsml, "./Shaders/gbuffer.vs.glsl", "./Shaders/gbuffer.fs.glsl");
    ssao_shader_ = new Shader(context, vsml, "./Shaders/ssao.vs.glsl", "./Shaders/ssao.fs.glsl");

    Option op;
    op["type"] = toString(GL_FLOAT);
    gbuffer_normal_texture_ = new GLTexture(context, window_width_, window_height_, op);
    gbuffer_pos_texture_ = new GLTexture(context, window_width_, window_height_, op);
    gbuffer_depth_texture_ = new GLTexture(context, window_width_, window_height_, op);
}

void Renderer::Init() {
    model_shader_ = new Shader(context, vsml, "./Shaders/draw_with_shadow.vs.glsl", "./Shaders/draw_with_shadow.fs.glsl");
    shadow_map_shader_ = new Shader(context, vsml, "./Shaders/shadow_depth_map.vs.glsl", "./Shaders/shadow_depth_map.fs.glsl");
    show_texture_shader_ = new Shader(context, vsml, "./Shaders/show_texture.vs.glsl", "./Shaders/show_texture.fs.glsl");
    acc_texture_shader_ = new Shader(context, vsml, "./Shaders/acc_texture.vs.glsl", "./Shaders/acc_texture.fs.glsl");
    // model_mesh_ = GLMesh::cube(context);
    std::string obj_path = "./Models/cornell_box.obj";
    std::string mtl_path = "./Models/";
    GLMesh::LoadFromObjFile(context, obj_path, mtl_path, scene_meshes_, materals_);

    Option op;
    op["type"] = toString(GL_FLOAT);
    read_buf_ = new GLTexture(context, window_width_, window_height_, op);
    write_buf_ = new GLTexture(context, window_width_, window_height_, op);
    single_pass_ = new GLTexture(context, window_width_, window_height_, op);
    screen_plane_ = GLMesh::plane(context, 1, 1);

    VPLInit();
    SSAOInit();

    // for temp
    cubemap_gen_shader_ = new Shader(context, vsml, "./Shaders/cubemap.vs.glsl", "./Shaders/cubemap.fs.glsl");
    cubemap_show_shader_ = new Shader(context, vsml, "./Shaders/envmap.vs.glsl", "./Shaders/envmap.fs.glsl");
    sphere_mesh_ = GLMesh::sphere(context, 100);
    cube_mash_ = GLMesh::cube(context);
    cubemap_texture_ = new GLCubeMap(context, 512, 512, op);
}

void Renderer::SetCamera(const Vec3f &eye, const Vec3f &look_at, const Vec3f &up) {
    eye_ = eye;
    look_at_ = look_at;
    up_ = up;
}

void Renderer::RenderSingleLight(int light_id) {
    /*std::function<void(void)> call_back = [this, light_id]() {
        float light_pos_array[3];
        float light_color_array[3];
        light_poses_[light_id].toArray<float>(light_pos_array);
        light_colors_[light_id].toArray<float>(light_color_array);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < scene_meshes_.size(); i++) {
            Vec3d diffuse = materals_[i]->GetReflectance();
            float diffuse_array[3];
            diffuse.toArray<float>(diffuse_array);
            model_shader_->uniforms("diffuse_color", diffuse_array);
            model_shader_->uniforms("light_color", light_color_array);
            model_shader_->uniforms("light_pos", light_pos_array);
            model_shader_->draw_mesh(scene_meshes_[i]);
        }
    };
    single_pass_->drawTo(call_back, 3);*/

    float fov = 45.f;
    float aspect = (1.0f * window_width_) / window_height_;;

    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(fov, aspect, near_clip_, far_clip_);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);
    std::function<void(void)> call_back = [this, light_id]() {
        float light_pos_array[3];
        float light_color_array[3];
        float light_direction_array[3];
        light_colors_[light_id].toArray<float>(light_color_array);
        light_poses_[light_id].toArray<float>(light_pos_array);
        light_directions_[light_id].toArray<float>(light_direction_array);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < scene_meshes_.size(); i++) {
            Vec3d diffuse = materals_[i]->GetReflectance();
            float diffuse_array[3];
            diffuse.toArray<float>(diffuse_array);
            cubemap_texture_->bind(0);
            model_shader_->uniforms("shadow_cube", 0);
            model_shader_->uniforms("diffuse_color", diffuse_array);
            model_shader_->uniforms("light_color", light_color_array);
            model_shader_->uniforms("light_pos", light_pos_array);
            model_shader_->uniforms("light_direction", light_direction_array);
            model_shader_->draw_mesh(scene_meshes_[i]);
        }
    };
    single_pass_->drawTo(call_back, 3);
}

void Renderer::RenderDepthCube(int light_id) {
    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(90, 1, 1e-2, 1e4);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    float p[][6] = {
        { 1, 0, 0, 0,-1, 0},    // x+
        {-1, 0, 0, 0,-1, 0},    // x-
        { 0, 1, 0, 0, 0, 1},    // y+
        { 0,-1, 0, 0, 0,-1},    // y-
        { 0, 0, 1, 0,-1, 0},    // z+
        { 0, 0,-1, 0,-1, 0}     // z-
    };
    Vec3f loc = light_poses_[light_id];
    for (int f = 0; f < 6; f++) {
        vsml->loadIdentity(VSMathLibQT::VIEW);
        vsml->lookAt(0 + loc.x, 0 + loc.y, 0 + loc.z, p[f][0] + loc.x, p[f][1] + loc.y, p[f][2] + loc.z, p[f][3], p[f][4], p[f][5]);
        std::function<void(void)> clear_call_back = []() {
            glClearColor(1.0, 1.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0, 0.0, 0.0, 1.0);
        };
        cubemap_texture_->drawTo(clear_call_back, f, 3);
        for (int i = 0; i < scene_meshes_.size(); i++) {
            std::function<void(void)> call_back = [this, i, loc]() {
                Vec3d diffuse = materals_[i]->GetReflectance();
                float diffuse_array[3];
                diffuse.toArray<float>(diffuse_array);
                float light_pos_array[3];
                float light_color_array[3] = {1,1,1};
                loc.toArray<float>(light_pos_array);
                cubemap_gen_shader_->uniforms("mode", 0);
                cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                cubemap_gen_shader_->uniforms("light_color", light_color_array);
                cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                cubemap_gen_shader_->draw_mesh(scene_meshes_[i]);
            };
            cubemap_texture_->drawTo(call_back, f, 0);
        }
    }
}

void Renderer::VPLRender() {
    // VPL render begin
    std::function<void(void)> clear_call_back = []() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };
    read_buf_->drawTo(clear_call_back, 3);

    for (int k = 0; k < light_poses_.size(); k++) {
        // first, render the cubic shadow map (in world coordinate.
        RenderDepthCube(k);
        RenderSingleLight(k);
        std::function<void(void)> call_back2 = [this, k]() {
            single_pass_->bind(0);
            read_buf_->bind(1);
            acc_texture_shader_->uniforms("buffer_A", 0);
            acc_texture_shader_->uniforms("buffer_B", 1);
            acc_texture_shader_->uniforms("w_A", float(light_weights_[k]));
            acc_texture_shader_->uniforms("w_B", 1.f);
            acc_texture_shader_->draw_mesh(screen_plane_);
        };
        write_buf_->drawTo(call_back2, 3);
        read_buf_->swapWith(write_buf_);
    }
    float fov = 45.f;
    float aspect = (1.0f * window_width_) / window_height_;

    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(fov, aspect, near_clip_, far_clip_);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);
    read_buf_->bind(0);
    show_texture_shader_->uniforms("t_diffuse", 0);
    show_texture_shader_->draw_mesh(screen_plane_);

    // render the positions of VPLs
    if (0) {
        glClear(GL_DEPTH_BUFFER_BIT);
        for (int k = 0; k < light_poses_.size(); k++) {
            Vec3d p = light_poses_[k];
            vsml->loadIdentity(VSMathLibQT::MODEL);
            vsml->translate(p.x, p.y, p.z);
            vsml->scale(0.2, 0.2, 0.2);
            std::function<void(void)> call_back3 = [this, k]() {
                float diffuse_array[] = {0.7, 0.7, 0.7};
                float light_color_array[] = {1.0, 1.0, 1.0};
                float light_pos_array[3];
                light_pos_.toArray<float>(light_pos_array);
                cubemap_gen_shader_->uniforms("mode", 1);
                cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                cubemap_gen_shader_->uniforms("light_color", light_color_array);
                cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                cubemap_gen_shader_->draw_mesh(sphere_mesh_);
            };
            call_back3();
        }
    }
    // VPL render end
}

void Renderer::SSAORender() {
    float fov = 45.f;
    float aspect = (1.0f * window_width_) / window_height_;

    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(fov, aspect, near_clip_, far_clip_);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::function<void(void)> call_back = [this]() {
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        const float huge = 1e5;
        glClearColor(huge,huge,huge,huge);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPopAttrib();
        for (int i = 0; i < scene_meshes_.size(); i++) {
            gbuffer_shader_->draw_mesh(scene_meshes_[i]);
        }
    };
    gbuffer_shader_->uniforms("mode", 1);
    gbuffer_pos_texture_->drawTo(call_back, 0);
    gbuffer_shader_->uniforms("mode", 2);
    gbuffer_normal_texture_->drawTo(call_back, 0);
    gbuffer_shader_->uniforms("mode", 3);
    gbuffer_depth_texture_->drawTo(call_back, 0);

    gbuffer_pos_texture_->bind(0);
    gbuffer_normal_texture_->bind(1);
    gbuffer_depth_texture_->bind(2);
    ssao_shader_->uniforms("gbuffer_pos", 0);
    ssao_shader_->uniforms("gbuffer_normal", 1);
    ssao_shader_->uniforms("gbuffer_depth", 2);
    ssao_shader_->uniforms("num_sample", 500);
    ssao_shader_->uniforms("sample_radius", 3.75f);
    ssao_shader_->uniforms("range_chack_thres", 1.f);
    ssao_shader_->draw_mesh(screen_plane_);
}

void Renderer::Render() {
    // gen cubemap (render depth cube)
    /*vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(90, 1, 1e-2, 1e4);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    float p[][6] = {
        { 1, 0, 0, 0,-1, 0},    // x+
        {-1, 0, 0, 0,-1, 0},    // x-
        { 0, 1, 0, 0, 0, 1},    // y+
        { 0,-1, 0, 0, 0,-1},    // y-
        { 0, 0, 1, 0,-1, 0},    // z+
        { 0, 0,-1, 0,-1, 0}     // z-
    };
    Vec3f loc = light_pos_;
    for (int f = 0; f < 6; f++) {
        vsml->loadIdentity(VSMathLibQT::VIEW);
        vsml->lookAt(0 + loc.x, 0 + loc.y, 0 + loc.z, p[f][0] + loc.x, p[f][1] + loc.y, p[f][2] + loc.z, p[f][3], p[f][4], p[f][5]);
        std::function<void(void)> clear_call_back = []() {
            glClearColor(1.0, 1.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0, 0.0, 0.0, 1.0);
        };
        cubemap_texture_->drawTo(clear_call_back, f, 3);
        for (int i = 0; i < scene_meshes_.size(); i++) {
            std::function<void(void)> call_back = [this, i, loc]() {
                Vec3d diffuse = materals_[i]->GetReflectance();
                float diffuse_array[3];
                diffuse.toArray<float>(diffuse_array);
                float light_pos_array[3];
                float light_color_array[3] = {1,1,1};
                loc.toArray<float>(light_pos_array);
                cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                cubemap_gen_shader_->uniforms("light_color", light_color_array);
                cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                cubemap_gen_shader_->draw_mesh(scene_meshes_[i]);
            };
            cubemap_texture_->drawTo(call_back, f, 0);
        }
    }


    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->loadIdentity(VSMathLibQT::VIEW);
    vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);
    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(90, 1, 1e-2, 1e4);
    float eye_pos[3];
    eye_.toArray(eye_pos);
    cubemap_show_shader_->uniforms("eye_pos", eye_pos);
    cubemap_show_shader_->uniforms("bg_distance", 100.f);
    cubemap_texture_->bind(0);
    cubemap_show_shader_->uniforms("cube_texture", 0);
    cubemap_show_shader_->draw_mesh(sphere_mesh_);
    return;*/

    // RenderSingleLight
    /*float fov = 45.f;
    float aspect = (1.0f * window_width_) / window_height_;;

    vsml->loadIdentity(VSMathLibQT::PROJECTION);
    vsml->perspective(fov, aspect, near_clip_, far_clip_);

    vsml->loadIdentity(VSMathLibQT::VIEW);
    vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);

    vsml->loadIdentity(VSMathLibQT::MODEL);
    vsml->scale(0.05, 0.05, 0.05);
    std::function<void(void)> call_back = [this]() {
        float light_pos_array[3];
        float light_color_array[3] = {0.5,0.5,0.5};
        light_pos_.toArray<float>(light_pos_array);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (int i = 0; i < scene_meshes_.size(); i++) {
            Vec3d diffuse = materals_[i]->GetReflectance();
            float diffuse_array[3];
            diffuse.toArray<float>(diffuse_array);
            cubemap_texture_->bind(0);
            model_shader_->uniforms("shadow_cube", 0);
            model_shader_->uniforms("diffuse_color", diffuse_array);
            model_shader_->uniforms("light_color", light_color_array);
            model_shader_->uniforms("light_pos", light_pos_array);
            model_shader_->draw_mesh(scene_meshes_[i]);
        }
    };
    single_pass_->drawTo(call_back, 3);
    single_pass_->bind(0);
    show_texture_shader_->uniforms("t_diffuse", 0);
    show_texture_shader_->draw_mesh(screen_plane_);
    return;*/

//    VPLRender();
    SSAORender();

}
