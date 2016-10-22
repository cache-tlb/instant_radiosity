#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat4 m_model;
uniform mat3 m_normal;

in vec4 position;
in vec3 normal;

out vec3 frag_pos;      // frag_pos is the position of a pixel in world coordinate.
out vec3 N;
out float depth;

void main()
{
    gl_Position = m_pvm * position;
    vec4 frag_pos_4 = m_model * position;
    frag_pos = frag_pos_4.xyz;
    N = normalize(transpose(inverse(mat3(m_model))) * normal);
    // N = normalize(m_normal * normal);
    vec4 c_pos = m_viewModel * position;
    depth = -c_pos.z;
} 
