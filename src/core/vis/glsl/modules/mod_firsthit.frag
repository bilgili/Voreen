// first hit mode
vec4 color;
vec4 tfcolor = applyTF(normal.a);
if (tfcolor.a > 0.0) {

	normal.xyz = normalize(normal.xyz);

#if (FIRST_HIT_MODE == FHM_POSITION)
    // position
    color = vec4(sample, 1.0);

#elif (FIRST_HIT_MODE == FHM_NORMAL_VECTOR)
    // normal vector
	// transform to color space
    color = vec4(normal.xyz / 2.0 + 0.5, 1.0);

#elif (FIRST_HIT_MODE == FHM_POSITION_AND_NORMAL)
    // position
    color = vec4(sample, 1.0);
    // normal vector
    // transform to color space
	gl_FragData[1] = vec4(normal.xyz / 2.0 + 0.5, 1.0);

#elif (FIRST_HIT_MODE == FHM_SKETCH)
    // silhouette sketch
    normal.w = 0.0;
    vec4 vpos = vec4(sample, 1.0);

    // transform normal and voxel position to eyespace
    vpos.xyz = vpos.xyz * 2.0 - vec3(1.0);
    vpos = viewMatrix_ * vpos;
    normal = viewMatrix_ * normal;

    vec3 N = normalize(normal.xyz);
    vec3 V = normalize(-vpos.xyz);

    float NdotV = max(dot(N,V),0.0);

    float sketchThreshold = 0.5;
    int sketchExp = 8;
    if (NdotV < sketchThreshold)
        color = vec4(sketchColor_.rgb,1);//vec4(sketchColor_.rgb, 1.0-pow(NdotV, sketchExp));
    else
        discard;

#elif (FIRST_HIT_MODE == FHM_SHADE)
    // shade
    color = vec4(phongShading(normal, first.rgb, volumeParameters_, tfcolor.rgb, tfcolor.rgb), 1.0);
#elif (FIRST_HIT_MODE == FHM_EST_SHADE)
    // first hit shade
    // 
    // This would require manual inlining on Linux nVidia 8776.
    color = vec4(phongShadingNoKa(normal, first.rgb, volumeParameters_, tfcolor.rgb), 1.0);

#elif (FIRST_HIT_MODE == FHM_EST_NORMALS)
    // first hit normal vector
    color = vec4(normal.rgb, 1.0);

#elif (FIRST_HIT_MODE == FHM_POSITION_UNDEFORMED)
    
    vec2 p = gl_FragCoord.xy;
    vec3 entry = textureLookup2D(entryParamsUndeformed_, p).xyz;
    vec3 exit = textureLookup2D(exitParamsUndeformed_, p).xyz;
    
    vec3 real_dir = exit - entry;
    float part = length(t * direction) / tend;    
    color = vec4(entry + part * real_dir, 1.0);    

#elif (FIRST_HIT_MODE == FHM_POSITION_AND_NORMAL_RAYTRACING)
    // position
    float tNew = t - 2.0*stepIncr;
    vec3 sampleToSave = first.rgb + tNew * direction;
    color = vec4(sampleToSave, 1.0);
    float value = normal.a;
    // normal vector
    // transform to color space
	normal = normal*0.5+0.5;
	gl_FragData[1] = vec4(normal.xyz, value);
#endif
    finished = true;

} else {
    color = vec4(0.0);
}
