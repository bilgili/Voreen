uniform float sketchExp_;
uniform float sketchTH_;
uniform vec4 sketchColor_;

vec4 sketchShading(in vec4 normal, in vec3 first, in float t, in vec3 dir, vec3 kd) {

    // obtain normal and voxel position in texspace
    normal.xyz = normal.xyz * 2.0 - vec3(1.0);
    normal.w = 0.0;
    vec4 vpos = vec4(first + t * dir, 1.0);

    // transform normal and voxel position to eyespace
    vpos.xyz = vpos.xyz * 2.0 - vec3(1.0);
    vpos = viewMatrix_ * vpos;
    normal = viewMatrix_ * normal;

    vec3 N = normalize(normal.xyz);
    vec3 V = normalize(-vpos.xyz);

    float NdotV = max(dot(N,V),0.0);
        
    if (length(normal) > sketchTH_)
		return sketchColor_*vec4(1.0-pow(NdotV, sketchExp_));
    else return vec4(0.0);
}