#version 330
uniform sampler2D buffer_A;
uniform sampler2D buffer_B;
uniform float w_A;
uniform float w_B;

in vec2 vUv;

out vec4 outputF;

void main()
{
    vec4 c1 = texture2D(buffer_A, vUv);
    vec4 c2 = texture2D(buffer_B, vUv);
    outputF = vec4(w_A*c1.rgb + w_B*c2.rgb, 1.0);
}
