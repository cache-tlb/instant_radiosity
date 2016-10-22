#include "Renderer.h"

Renderer::Renderer(QOpenGLFunctionsType *context, VSMathLibQT *vsml):
    context(context), vsml(vsml),
    model_shader_(NULL), shadow_map_shader_(NULL),
    show_texture_shader_(NULL), acc_texture_shader_(NULL),
    read_buf_(NULL), write_buf_(NULL), single_pass_(NULL),
    screen_plane_(NULL),
    bvh_(NULL),
    near_clip_(1e-2), far_clip_(1e3),
    window_width_(512), window_height_(512),
    eye_(0,0,0), look_at_(0,0,-1), up_(0,1,0),
    light_pos_(278/20., 548/20.-0.1, 279.5/20.-0.1),
    vpl_computed_(false)
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
//        qDebug() << primitives_[i].v1_.x << primitives_[i].v1_.y << primitives_[i].v1_.z << primitives_[i].v2_.x << primitives_[i].v2_.y << primitives_[i].v2_.z << primitives_[i].v3_.x << primitives_[i].v3_.y << primitives_[i].v3_.z;
    }
    bvh_ = new BVH(&primitive_ptrs_);
}

void Renderer::DistributeVPLs() {
    double direct_weight = 0.0;
    light_colors_.clear();
    light_poses_.clear();
    light_weights_.clear();
    light_colors_.push_back(Vec3f(1.0, 1.0, 1.0));
    light_poses_.push_back(light_pos_);
    light_weights_.push_back(direct_weight);

    int target_vpl_number = 512*4;
    double delta = 0.001;
    Vec3f light_dir(0,-1,0);
    while (light_poses_.size() < target_vpl_number) {
        light_pos_ = Vec3f(213/20. + (343-213)/20.*rand()/RAND_MAX, 548/20.-0.5, 227/20. + (332-227)/20.*rand()/RAND_MAX);
        // light_colors_.push_back(Vec3f(1.0, 1.0, 1.0));
        // light_poses_.push_back(light_pos_);
        // light_weights_.push_back((1-direct_weight) / double(target_vpl_number)*2);
        Vec3f sample_dir = CosWeightedHemisphereSample(light_dir);
        Ray ray(light_pos_, sample_dir);
        IntersectionInfo intersection;
        bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
        if (!is_intersect) continue;
        Material *mat = intersection.object->GetMaterial();
        Vec3f normal = intersection.object->GetNoamal(intersection);
        if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
//        qDebug() << intersection.hit.x << intersection.hit.y << intersection.hit.z << normal.x << normal.y << normal.z;
        Vec3f color = mat->GetReflectance() * fabs(normal.dot(sample_dir));
        float t = random_float<float>();
        // t = 0.001;
        Vec3f pos = light_pos_*t + intersection.hit*(1-t);
        light_poses_.push_back(pos);
        // light_poses_.push_back(intersection.hit + normal*delta);
        light_colors_.push_back(color);
        light_weights_.push_back((1-direct_weight) / double(target_vpl_number)*3);

        do {
            Vec3f sample_dir = CosWeightedHemisphereSample(normal);
            Vec3f old_pos = intersection.hit;
            Ray ray(intersection.hit + normal*delta, sample_dir);
            bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
            if (!is_intersect) continue;
            Material *mat = intersection.object->GetMaterial();
            Vec3f normal = intersection.object->GetNoamal(intersection);
            if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
//            qDebug() << intersection.hit.x << intersection.hit.y << intersection.hit.z << normal.x << normal.y << normal.z;
            Vec3f color2 = mat->GetReflectance() * fabs(normal.dot(sample_dir));
            float t = random_float<float>();
            Vec3f pos = old_pos*t + intersection.hit*(1-t);
            light_poses_.push_back(pos);
            // light_poses_.push_back(intersection.hit + normal*delta);
            Vec3f c2(color.x*color2.x, color.y*color2.y, color.z*color2.z);
            light_colors_.push_back(c2);
            light_weights_.push_back((1-direct_weight) / double(target_vpl_number)*3);
            /*do {
                Vec3f sample_dir = CosWeightedHemisphereSample(normal);
                Vec3f old_pos = intersection.hit;
                Ray ray(intersection.hit + normal*delta, sample_dir);
                bool is_intersect = bvh_->GetIntersection(ray, &intersection, false);
                if (!is_intersect) continue;
                Material *mat = intersection.object->GetMaterial();
                Vec3f normal = intersection.object->GetNoamal(intersection);
//                qDebug() << intersection.hit.x << intersection.hit.y << intersection.hit.z << normal.x << normal.y << normal.z;
                if (normal.dot(sample_dir) > 0) normal = normal * -1.f;
                Vec3f color3 = mat->GetReflectance() * fabs(normal.dot(sample_dir));
                float t = random_float<float>();
                Vec3f pos = old_pos*t + intersection.hit*(1-t);
                light_poses_.push_back(pos);
                // light_poses_.push_back(intersection.hit + normal*delta);
                Vec3f c3(color2.x*color3.x, color2.y*color3.y, color2.z*color3.z);
                light_colors_.push_back(c3);
                light_weights_.push_back((1-direct_weight) / double(target_vpl_number)*3);
            } while (false);*/
        } while (false);
    }
    vpl_computed_ = true;
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

    BuildBVH();
    DistributeVPLs();

    Option op;
    op["type"] = toString(GL_FLOAT);
    read_buf_ = new GLTexture(context, window_width_, window_height_, op);
    write_buf_ = new GLTexture(context, window_width_, window_height_, op);
    single_pass_ = new GLTexture(context, window_width_, window_height_, op);
    screen_plane_ = GLMesh::plane(context, 1, 1);

    // generate light samples
    /*int num_samples = 1;
    light_colors_.resize(num_samples);
    light_poses_.resize(num_samples);
    for(int i = 0; i < num_samples; i++) {
        light_colors_[i] = Vec3f(1,1,1);
        light_poses_[i] = light_pos_ + Vec3f(i*.1, 0, -i*.1);
    }*/

    // for temp
    cubemap_gen_shader_ = new Shader(context, vsml, "./Shaders/cubemap.vs.glsl", "./Shaders/cubemap.fs.glsl");
    cubemap_show_shader_ = new Shader(context, vsml, "./Shaders/envmap.vs.glsl", "./Shaders/envmap.fs.glsl");
    sphere_mesh_ = GLMesh::sphere(context, 100);
    cube_mash_ = GLMesh::cube(context);
    QImage xn = QImage(QString("./Images/cubemap/x-f.png"));
    QImage xp = QImage(QString("./Images/cubemap/x+f.png"));
    QImage yn = QImage(QString("./Images/cubemap/y-f.png"));
    QImage yp = QImage(QString("./Images/cubemap/y+f.png"));
    QImage zn = QImage(QString("./Images/cubemap/z-f.png"));
    QImage zp = QImage(QString("./Images/cubemap/z+f.png"));
//    cubemap_texture_ = GLCubeMap::fromQImages(context, xn, xp, yn, yp, zn, zp);
    cubemap_texture_ = new GLCubeMap(context, 512, 512, op);
//    op = Option();
    frame_buf_ = new GLTexture(context, 512, 512, op);
    pixel_buf = new float[1024*1024*4];
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
        light_colors_[light_id].toArray<float>(light_color_array);
        light_poses_[light_id].toArray<float>(light_pos_array);
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
        frame_buf_->drawTo(clear_call_back, 3);
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
            frame_buf_->drawTo(call_back, 0);
        }
        GLenum dstTarget = f + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        frame_buf_->bind();
        cubemap_texture_->bind();
        context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel_buf);
        context->glTexImage2D(dstTarget, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, pixel_buf);
    }
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
        frame_buf_->drawTo(clear_call_back, 3);
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
            frame_buf_->drawTo(call_back, 0);
        }
        GLenum dstTarget = f + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        frame_buf_->bind();
        cubemap_texture_->bind();
        context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel_buf);
        context->glTexImage2D(dstTarget, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, pixel_buf);
    }*/

    /*
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

    if (vpl_computed_) {
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
        float aspect = (1.0f * window_width_) / window_height_;;

        vsml->loadIdentity(VSMathLibQT::PROJECTION);
        vsml->perspective(fov, aspect, near_clip_, far_clip_);

        vsml->loadIdentity(VSMathLibQT::VIEW);
        vsml->lookAt(eye_.x, eye_.y, eye_.z, look_at_.x, look_at_.y, look_at_.z, up_.x, up_.y, up_.z);

        vsml->loadIdentity(VSMathLibQT::MODEL);
        vsml->scale(0.05, 0.05, 0.05);
        read_buf_->bind(0);
        show_texture_shader_->uniforms("t_diffuse", 0);
        show_texture_shader_->draw_mesh(screen_plane_);

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
    } else {
        // distribute the VPLs, I need consider it again.
        double sum_intensity = 0.;
        // first, render a cube map from the light source.
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
        Vec3f loc = light_pos_;
        for (int f = 0; f < 6; f++) {
            vsml->loadIdentity(VSMathLibQT::VIEW);
            vsml->lookAt(0 + loc.x, 0 + loc.y, 0 + loc.z, p[f][0] + loc.x, p[f][1] + loc.y, p[f][2] + loc.z, p[f][3], p[f][4], p[f][5]);
            std::function<void(void)> clear_call_back = []() {
                glClearColor(0.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            };
            frame_buf_->drawTo(clear_call_back, 3);
            for (int i = 0; i < scene_meshes_.size(); i++) {
                std::function<void(void)> call_back = [this, i, loc]() {
                    Vec3d diffuse = materals_[i]->GetReflectance();
                    float diffuse_array[3];
                    diffuse.toArray<float>(diffuse_array);
                    float light_pos_array[3];
                    float light_color_array[3] = {1,1,1};
                    loc.toArray<float>(light_pos_array);
                    cubemap_gen_shader_->uniforms("mode", 1);
                    cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                    cubemap_gen_shader_->uniforms("light_color", light_color_array);
                    cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                    cubemap_gen_shader_->draw_mesh(scene_meshes_[i]);
                };
                frame_buf_->drawTo(call_back, 0);
            }
            GLenum dstTarget = f + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            frame_buf_->bind();
            cubemap_texture_->bind();
            context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            context->glTexImage2D(dstTarget, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            for (int i = 0; i < 512; i++) {
                for (int j = 0; j < 512; j++) {
                    float r = pixel_buf[i*512*4+j*4+0];
                    float g = pixel_buf[i*512*4+j*4+1];
                    float b = pixel_buf[i*512*4+j*4+2];
                    Vec3d c(r, g, b);
                    double intensity = c.length();
                    sum_intensity += intensity;
                    cube_colors_.push_back(c);
                    cube_intensities_.push_back(intensity);
                }
            }
        }
        for (int f = 0; f < 6; f++) {
            vsml->loadIdentity(VSMathLibQT::VIEW);
            vsml->lookAt(0 + loc.x, 0 + loc.y, 0 + loc.z, p[f][0] + loc.x, p[f][1] + loc.y, p[f][2] + loc.z, p[f][3], p[f][4], p[f][5]);
            std::function<void(void)> clear_call_back = []() {
                glClearColor(0.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            };
            frame_buf_->drawTo(clear_call_back, 3);
            for (int i = 0; i < scene_meshes_.size(); i++) {
                std::function<void(void)> call_back = [this, i, loc]() {
                    Vec3d diffuse = materals_[i]->GetReflectance();
                    float diffuse_array[3];
                    diffuse.toArray<float>(diffuse_array);
                    float light_pos_array[3];
                    float light_color_array[3] = {1,1,1};
                    loc.toArray<float>(light_pos_array);
                    cubemap_gen_shader_->uniforms("mode", 2);
                    cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                    cubemap_gen_shader_->uniforms("light_color", light_color_array);
                    cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                    cubemap_gen_shader_->draw_mesh(scene_meshes_[i]);
                };
                frame_buf_->drawTo(call_back, 0);
            }
            GLenum dstTarget = f + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            frame_buf_->bind();
            cubemap_texture_->bind();
            context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            context->glTexImage2D(dstTarget, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            for (int i = 0; i < 512; i++) {
                for (int j = 0; j < 512; j++) {
                    float x = pixel_buf[i*512*4+j*4+0];
                    float y = pixel_buf[i*512*4+j*4+1];
                    float z = pixel_buf[i*512*4+j*4+2];
                    Vec3d p(x, y, z);
                    cube_poses_.push_back(p);
                }
            }
        }

        for (int f = 0; f < 6; f++) {
            vsml->loadIdentity(VSMathLibQT::VIEW);
            vsml->lookAt(0 + loc.x, 0 + loc.y, 0 + loc.z, p[f][0] + loc.x, p[f][1] + loc.y, p[f][2] + loc.z, p[f][3], p[f][4], p[f][5]);
            std::function<void(void)> clear_call_back = []() {
                glClearColor(0.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            };
            frame_buf_->drawTo(clear_call_back, 3);
            for (int i = 0; i < scene_meshes_.size(); i++) {
                std::function<void(void)> call_back = [this, i, loc]() {
                    Vec3d diffuse = materals_[i]->GetReflectance();
                    float diffuse_array[3];
                    diffuse.toArray<float>(diffuse_array);
                    float light_pos_array[3];
                    float light_color_array[3] = {1,1,1};
                    loc.toArray<float>(light_pos_array);
                    cubemap_gen_shader_->uniforms("mode", 3);
                    cubemap_gen_shader_->uniforms("diffuse_color", diffuse_array);
                    cubemap_gen_shader_->uniforms("light_color", light_color_array);
                    cubemap_gen_shader_->uniforms("light_pos", light_pos_array);
                    cubemap_gen_shader_->draw_mesh(scene_meshes_[i]);
                };
                frame_buf_->drawTo(call_back, 0);
            }
            GLenum dstTarget = f + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            frame_buf_->bind();
            cubemap_texture_->bind();
            context->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            context->glTexImage2D(dstTarget, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, pixel_buf);
            for (int i = 0; i < 512; i++) {
                for (int j = 0; j < 512; j++) {
                    float x = pixel_buf[i*512*4+j*4+0];
                    float y = pixel_buf[i*512*4+j*4+1];
                    float z = pixel_buf[i*512*4+j*4+2];
                    Vec3d n(x, y, z);
                    cube_normals_.push_back(n);
                }
            }
        }

        /*vsml->loadIdentity(VSMathLibQT::MODEL);
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
        cubemap_show_shader_->draw_mesh(sphere_mesh_);*/
        // then sample the positions and generate a vpl for each position.
        std::uniform_real_distribution<double> unif(0, 1);
        std::default_random_engine re;

        double direct_weight = 0.1;
        light_colors_.clear();
        light_poses_.clear();
        light_weights_.clear();
        light_colors_.push_back(Vec3d(1.0, 1.0, 1.0));
        light_poses_.push_back(light_pos_);
        light_weights_.push_back(direct_weight);
        int num_vpls = 64;
        for (int i = 0; i < num_vpls;) {
            double r = unif(re) * sum_intensity;
//            qDebug() << r / sum_intensity;
            double acc = 0;
            /*int index = -1;
            for (int i = 0; i < cube_intensities_.size(); i++) {
                if (acc < r && acc + cube_intensities_[i] >= r) {
                    index = i;
                    break;
                }
                acc += cube_intensities_[i];
            }*/
             int index = cube_colors_.size()*((i+0.5)/double(num_vpls));
             i++;
//            qDebug() << index;
            if (index < 0) {
                qDebug() << "something wrong";
                continue;
            }
            else {
                Vec3d light_color = cube_colors_[index]*2;
                Vec3d light_pos = cube_poses_[index] + 0.05 * cube_normals_[index];
                Vec3d light_dir = cube_poses_[index] - light_pos_;
                double dot = light_dir.unit().dot(Vec3d(0, -1, 0));
//                if ( dot < 0.1 ) continue;
                light_colors_.push_back(light_color);
                light_poses_.push_back(light_pos);
                light_weights_.push_back((1 - direct_weight) / double(num_vpls));
                sum_intensity -= cube_intensities_[index];
//                qDebug() << light_pos.x << light_pos.y << light_pos.z;

                cube_colors_[index] = cube_colors_.back(); cube_colors_.pop_back();
                cube_intensities_[index] = cube_intensities_.back(); cube_intensities_.pop_back();
                cube_normals_[index] = cube_normals_.back(); cube_normals_.pop_back();
                cube_poses_[index] = cube_poses_.back(); cube_poses_.pop_back();
            }
//            i++;
        }
//        light_colors_[0] = light_colors_[0]; light_colors_.resize(1);
//        light_poses_[0] = light_poses_[0]; light_poses_.resize(1);
        vpl_computed_ = true;
    }
}
