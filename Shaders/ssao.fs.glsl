#version 330

uniform sampler2D gbuffer_pos;
uniform sampler2D gbuffer_normal;
uniform sampler2D gbuffer_depth;
uniform mat4 m_projection;

uniform int num_sample;
uniform float sample_radius;
uniform float range_chack_thres;

in vec2 vUv;

out vec4 outputF;

// range is [-1,1]
float rand(float n){return fract(sin(n*17.3) * 43758.5453123)*2.0-1.0;}

// sample solid Hemisphere (z >= 0)
vec3 sampleSolidHemisphere(int id) {
    float x = rand(id + 3e4);
    float y = rand(id+ 1e4);
    float z = rand(id+ 2e4)*0.5+0.5;
    float scale = rand(id + 4e4)*0.5 + 0.5;
    scale = mix(0.1, 1.0, scale*scale);
    vec3 v = normalize(vec3(x, y, z));
    return v*scale;
}

void main() 
{
    vec3 pos = texture2D(gbuffer_pos, vUv).rgb;
    vec3 normal = texture2D(gbuffer_normal, vUv).rgb;
    float depth = texture2D(gbuffer_depth, vUv).r;

    normal = -1.0*normalize(normal);
    if (dot(normal, pos) > 0) normal = -normal;

    // if (depth > 1000) discard;

    // random rotate the sample kernel then rotate it to normal
    vec3 rvec = normalize(vec3(rand(vUv.x*1479.8 + pos.x), rand(vUv.y*8743.45 + pos.y), 0.0));
    vec3 tangent = normalize(rvec - normal * dot(rvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    int valid = 0;
    vec3 res = vec3(0.0,0.0,0.0);
    for (int i = 0; i < num_sample; i++) {
        vec3 sample_v = tbn*sampleSolidHemisphere(i);
        vec3 sample_pos = sample_v*sample_radius + pos;
        vec4 proj_sample_pos = m_projection * vec4(sample_pos, 1.0);
        vec4 sample_view = vec4(sample_pos, 1.0);
        vec2 proj_sample_xy = proj_sample_pos.xy / proj_sample_pos.z;
        vec2 proj_sample_uv = proj_sample_xy*0.5+0.5;
        if (proj_sample_uv.x < 0 || proj_sample_uv.x > 1 || proj_sample_uv.y < 0 || proj_sample_uv.y > 1) continue;
        proj_sample_uv = clamp(proj_sample_uv, 0.0, 1.0); 
        float sample_proj_depth = texture2D(gbuffer_depth, proj_sample_uv).r;      
        float range_check = abs(depth - sample_proj_depth) < range_chack_thres ? 1.0 : 0.0;
        // range_check = 1.0;

        // note the depth value is along -z
        occlusion += (sample_proj_depth <= -sample_view.z ? 1.0 : 0.0) * range_check;
        valid += 1;
    }

    valid = max(1, valid);
    float ssao = 1.0 - (occlusion / valid);
    res = vec3(ssao, ssao, ssao);

    outputF = vec4(res, 1.0);
}
