#version 330

uniform mat4 m_pvm;
in vec4 position;
out float depth;

void main()
{
    gl_Position = m_pvm * position;
    depth = gl_Position.z;
} 
