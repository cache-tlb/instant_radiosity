#version 330

uniform samplerCube shadow_cube;
uniform sampler2D gbuffer_pos;
uniform sampler2D gbuffer_normal;
uniform sampler2D gbuffer_albedo;

uniform float tau;
uniform int raymarching_num;

uniform vec3 eye_pos;
uniform vec3 media_albedo;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform bool add_media;

in vec2 vUv;
out vec4 outputF;

float ShadowCubeCalc(vec3 frag_pos_world) {
    vec3 frag_dir = frag_pos_world - light_pos;
    float frag_light_dist = length(frag_pos_world - light_pos);
    float shadow_cube_depth = texture(shadow_cube, normalize(frag_dir)).r;
    float shadow = 0.0;
    if (shadow_cube_depth + 0.05 < frag_light_dist) shadow = 1.0;
    else shadow = 0.0;
    return shadow;
}

const float pi = 3.1415926;

float P(vec3 x, vec3 view_dir) {
    return 1.0;
}

void main()
{
    vec3 normal = texture2D(gbuffer_normal, vUv).rgb;
    vec3 frag_pos = texture2D(gbuffer_pos, vUv).rgb;
    vec3 albedo = texture2D(gbuffer_albedo, vUv).rgb;

//    // Diffuse
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = abs(dot(light_dir, normal));
    vec3 diffuse = diff * light_color;

//    // compute shadow
    float shadow = ShadowCubeCalc(frag_pos);
    float dist = length(light_pos - frag_pos);
    float att = max(-1.0*dot(light_dir, normalize(light_direction)), 0.0);
    float att_dist = 1.0 / (1.0 + 0.02*dist + 0.01*dist*dist);
    att_dist = sqrt(att_dist);
//    float att_dist = 50.0/(dist*dist);
    vec3 lighting = (1.0 - shadow) * diffuse * albedo * att * att_dist;
    vec3 L0 = lighting;
    vec3 view_dir = eye_pos - frag_pos;
    float scale_factor = 1.;
    float s = length(view_dir);
    vec3 L = L0*exp(-s*tau/scale_factor);
    vec3 media_L = vec3(0.0,0.0,0.0);
    float dl = s / float(raymarching_num);
    vec3 x = frag_pos;
    vec3 delta_x = normalize(view_dir);
    int count = 0;
    for (float l = s - dl; l > 0; l -= dl) {
        x += delta_x*dl;
        float v = 1.0 - ShadowCubeCalc(x);
        light_dir = normalize(light_pos - x);
        float d = length(light_dir)/scale_factor;
        vec3 phi = max(-1.0*dot(light_dir, normalize(light_direction)), 0.0)*light_color;
//        vec3 phi = light_color;
        vec3 Lin = exp(-d*tau)*v*phi*4.0/pi/d/d;
        vec3 Li = Lin * tau * media_albedo * P(x, delta_x);
        if (add_media)
            media_L += Li* exp(-l/scale_factor * tau) * dl/scale_factor;
    }
    outputF = vec4(media_L + L, 1.0f);
}
