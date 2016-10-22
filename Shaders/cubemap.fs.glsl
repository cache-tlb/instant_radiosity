#version 330



uniform vec3 light_pos;
uniform vec3 diffuse_color;
uniform vec3 light_color;

uniform int mode;

in vec3 frag_pos;
in vec3 N;
in float depth;

out vec4 outputF;

void main()
{
    vec3 normal = normalize(N);

    // Diffuse
    vec3 light_dir = normalize(light_pos - frag_pos);
//    float diff = max(dot(light_dir, normal), 0.0);
    float diff = abs(dot(light_dir, normal));
    vec3 diffuse = diff * light_color;

    vec3 lighting = diffuse * diffuse_color;
    // outputF = vec4(lighting, 1.0f);
    float len = length(light_pos - frag_pos);
    if (mode == 0) outputF = vec4(len, len, len, 1.0f);
    else if (mode == 1) outputF = vec4(lighting, 1.0);
    else if (mode == 2) outputF = vec4(frag_pos, 1.0);
    else if (mode == 3) {
        if (dot(normal, light_dir) > 0) outputF = vec4(normal, 1.0);
        else outputF = vec4(-normal, 1.0);
    } 
}
