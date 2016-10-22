#version 330

uniform samplerCube shadow_cube;

uniform vec3 light_pos;
uniform vec3 diffuse_color;
uniform vec3 light_color;

in vec3 frag_pos;
in vec3 N;
// in vec4 frag_pos_lightspace;
in float depth;

out vec4 outputF;

/*float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(N);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);
    // Check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);
    for(int x = -8; x <= 8; ++x)
    {
        for(int y = -8; y <= 8; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= (17.0*17.0);
    
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
        shadow = 0.0;

    return shadow;
}*/

float ShadowCubeCalc(vec3 frag_pos_world) {
    vec3 frag_dir = frag_pos_world - light_pos;
    float frag_light_dist = length(frag_pos_world - light_pos);
    float shadow_cube_depth = texture(shadow_cube, normalize(frag_dir)).r;
    float shadow = 0.0;
    if (shadow_cube_depth + 0.05 < frag_light_dist) shadow = 1.0;
    else shadow = 0.0;
    return shadow;
}

void main() 
{
    vec3 normal = normalize(N);

    // Diffuse
    vec3 light_dir = normalize(light_pos - frag_pos);
//    float diff = max(dot(light_dir, normal), 0.0);
    float diff = abs(dot(light_dir, normal));
    vec3 diffuse = diff * light_color;

    // compute shadow
    float shadow = ShadowCubeCalc(frag_pos);
    // float shadow = 0.0;
    float dist = length(light_pos - frag_pos);
    float att = 1.0 / (1.0 + 0.5*dist + 0.1*dist*dist);
    att = 1;
    vec3 lighting = (1.0 - shadow) * diffuse * diffuse_color * att;
    outputF = vec4(lighting, 1.0f);
}
