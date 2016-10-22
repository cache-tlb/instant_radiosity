#version 330

uniform samplerCube cube_texture;

in vec3 eye_ray;
out vec4 outputF;

void main()
{
    outputF = texture(cube_texture, normalize(eye_ray));
}
