#version 330

// mode: 1 -> pos
//       2 -> normal
//       3 -> depth
//       4 -> constant value (for debug)
uniform int mode;

in vec3 frag_pos;
in vec3 N;
in float depth;

out vec4 outputF;

void main() 
{
    vec4 ret;
    if (mode == 1) {
        // pos
        vec3 pos = frag_pos;
        ret = vec4(pos, 1.0);
    } else if (mode == 2) {
        // normal
        vec3 normal = normalize(N);
        ret = vec4(normal, 1.0);
    } else if (mode == 3) {
        float d = depth;
        ret = vec4(d, d, d, 1.0);
    } else if (mode == 0) {
        ret = vec4(1.0,0.5,0.5,1.0);
    }
    outputF = ret;
}
