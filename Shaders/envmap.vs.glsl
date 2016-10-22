#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec3 eye_pos;
uniform float bg_distance;

in vec4 position;
in vec3 normal;

out vec3 eye_ray;

void main()
{
        vec4 new_pos = vec4(position.xyz * bg_distance + eye_pos.xyz, 1.0);
        eye_ray = position.xyz;
        gl_Position = m_pvm * new_pos;
}
