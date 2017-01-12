#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat4 m_model;
uniform mat3 m_normal;
uniform mat3 m_normalModel;

in vec4 position;
in vec3 normal;

out vec3 frag_pos_world;      // frag_pos is the position of a pixel in world coordinate.
out vec3 frag_pos_cam;       // position in view space;
out vec3 frag_normal_world;
out vec3 frag_normal_cam;
out float depth;

void main()
{
    gl_Position = m_pvm * position;

    vec4 frag_pos_4 = m_model * position;
    frag_pos_world = frag_pos_4.xyz;
    frag_normal_world = normalize(m_normalModel * normal);

    vec4 c_pos = m_viewModel * position;
    depth = -c_pos.z;
    frag_pos_cam = c_pos.xyz;
    frag_normal_cam = normalize(m_normal * normal);
} 
