#version 330

in float depth;
out vec4 outputF;

void main()
{
    float d = depth*0.5 + 0.5;
    outputF = vec4(d, d, d, 1.0);
} 
