#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat4 m_view;
uniform mat4 m_model;

in vec4 position;
in vec2 uv;

out vec2 vUv;

void main()
{
    gl_Position = position;
    vUv = uv;
} 
