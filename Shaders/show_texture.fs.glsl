#version 330
uniform sampler2D t_diffuse;

in vec2 vUv;

out vec4 outputF;

void main()
{
    /*vec2 texelSize = textureSize(t_diffuse, 0);
    vec4 acc = vec4(0.0, 0.0, 0.0, 0.0);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            acc = acc + texture2D(t_diffuse, vUv + vec2(dx/texelSize.x, dy/texelSize.y));
        }
    }
    outputF = acc / 9.0;*/
    outputF = texture2D(t_diffuse, vUv);
}
