#version 330

// mode: 
//       0 -> constant value (for debug)
//       1 -> pos (world)
//       2 -> normal (world)
//       3 -> depth (camera)
//       4 -> pos (camera)
//       5 -> normal (camera)
uniform int mode;

in vec3 frag_pos_world;      // frag_pos is the position of a pixel in world coordinate.
in vec3 frag_pos_cam;       // position in view space;
in vec3 frag_normal_world;
in vec3 frag_normal_cam;
in float depth;

out vec4 outputF;

void main() 
{
    vec4 ret;
    if (mode == 0) {
        ret = vec4(1.0,0.5,0.5,1.0);
    } else if (mode == 1) {
        vec3 pos = frag_pos_world;
        ret = vec4(pos, 1.0);
    } else if (mode == 2) {
        vec3 normal = normalize(frag_normal_world);
        ret = vec4(normal, 1.0);
    } else if (mode == 3) {
        float d = depth;
        ret = vec4(d, d, d, 1.0);
    } else if (mode == 4) {
        vec3 pos = frag_pos_cam;
        ret = vec4(pos, 1.0);
    } else if (mode == 5) {
        vec3 normal = normalize(frag_normal_cam);
        ret = vec4(normal, 1.0);
    }
    outputF = ret;
}
