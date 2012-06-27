
/**
 * Calculates a normal for the ray entry point 'front' (and its corresponding exit point 'back') from the 
 * neighbouring entry points. The normal points towards the camera's half-space and can thus be 
 * used for lighting calculations. Additionally, the volume parameters have to be passed for coordinate transformations.
 */
vec3 calcNormalFromEntryPoints(vec4 front, vec4 back, VOLUME_PARAMETERS volumeParams) {
	
	const float OFFSET = 5.0;
	
	vec3 v0;
	vec3 v1;
		
    // calculate x-tangent v0
	vec4 f0 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x+OFFSET, gl_FragCoord.y) );
    vec4 b0 = textureLookup2D(exitParams_, vec2(gl_FragCoord.x+OFFSET, gl_FragCoord.y) );
    vec4 f1 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x-OFFSET, gl_FragCoord.y) );
    vec4 b1 = textureLookup2D(exitParams_, vec2(gl_FragCoord.x-OFFSET, gl_FragCoord.y) );
    if (f0.a == 0.0 || b0.a == 0.0) {
        f0 = front;
        b0 = back;
    } 
	else if (f1.a == 0.0 || b1.a == 0.0) {
	    f1 = front;
        b1 = back;
    }
	// invert jittering of entry points
    f0.rgb = (f0.rgb + (f0.a-1.0)*b0.rgb)/f0.a;
    f1.rgb = (f1.rgb + (f1.a-1.0)*b1.rgb)/f1.a;
	v0 = f0.rgb - f1.rgb;
	
    // calculate y-tangent v1
	f0 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x, gl_FragCoord.y+OFFSET) );
    b0 = textureLookup2D(exitParams_, vec2(gl_FragCoord.x, gl_FragCoord.y+OFFSET) );
    f1 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x, gl_FragCoord.y-OFFSET) );
    b1 = textureLookup2D(exitParams_, vec2(gl_FragCoord.x, gl_FragCoord.y-OFFSET) );
    if (f0.a == 0.0 || b0.a == 0.0) {
        f0 = front;
        b0 = back;
    } 
	else if (f1.a == 0.0 || b1.a == 0.0) {
        f1 = front;
        b1 = back;
    }
    // invert jittering of entry points
    f0.rgb = (f0.rgb + (f0.a-1.0)*b0.rgb)/f0.a;
    f1.rgb = (f1.rgb + (f1.a-1.0)*b1.rgb)/f1.a;
    v1 = f0.rgb - f1.rgb;
	
	return cross(v0, v1);

}

/*vec3 calcNormalFromEntryPoints(vec3 voxPos) {
    
	//FIXME: error handling if v0 or v1 is (0,0,0)
    
    vec3 v0 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x+2.0, gl_FragCoord.y) ).rgb - voxPos;
    vec3 v1 = textureLookup2D(entryParams_, vec2(gl_FragCoord.x, gl_FragCoord.y+2.0) ).rgb - voxPos;
    
    return cross(v0, v1);	
} */

#if defined(CALC_GRADIENTS)

/**
 * Necessary functions for gradient calculation.
 * Gradients are returned unnormalized.
 **/

vec3 fixClipBorderGradient(vec3 voxPos, vec3 dir) {
    
    // calculate new normal for entry points
    
    //FIXME: this should only be enabled if clipping is active!
    //       Also not necessary at texture borders!
    //FIXME: error handling if v0 or v1 is (0,0,0)
    
    vec3 v0 = normalize(textureLookup2D(entryParams_, vec2(gl_FragCoord.x+2.0, gl_FragCoord.y) ).rgb - voxPos);
    vec3 v1 = normalize(textureLookup2D(entryParams_, vec2(gl_FragCoord.x, gl_FragCoord.y+2.0) ).rgb - voxPos);
    
    vec3 gradient = normalize(cross(v0, v1));
    gradient = faceforward(gradient, dir, gradient);
    return gradient;
}


/**
 * Calculate the gradient based on the R channel
 * using forward differences.
 */
vec3 calcGradientRFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;
    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

   		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos).r);
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).r);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).r);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).r);
		#else
			float v = textureLookup3D(volume, volumeParameters, voxPos).r;
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).r;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).r;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).r;
		#endif

		gradient = vec3(v - v0, v - v1, v - v2);
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the G channel
 * using forward differences.
 */
vec3 calcGradientGFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos).g);
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).g);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).g);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).g);
		#else
			float v = textureLookup3D(volume, volumeParameters, voxPos).g;
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).g;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).g;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).g;
		#endif

		gradient = vec3(v - v0, v - v1, v - v2);
		gradient *= volumeParameters.datasetSpacingRCP_;
	
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the B channel
 * using forward differences.
 */
vec3 calcGradientBFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos).b);
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).b);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).b);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).b);
		#else
			float v = textureLookup3D(volume, volumeParameters, voxPos).b;
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).b;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).b;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).b;
		#endif

		gradient = vec3(v - v0, v - v1, v - v2);
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the A channel
 * using forward differences.
 */
vec3 calcGradientAFD(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

		vec3 offset = volumeParameters.datasetDimensionsRCP_;
		
		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos).a);
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).a);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).a);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).a);
		#else
			float v = textureLookup3D(volume, volumeParameters, voxPos).a;
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).a;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).a;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).a;
		#endif

		gradient = vec3(v - v0, v - v1, v - v2);
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the R channel.
 */
vec3 calcGradientR(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).r);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).r);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).r);
			float v3 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).r);
			float v4 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).r);
			float v5 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).r);
		#else
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).r;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).r;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).r;
			float v3 = textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).r;
			float v4 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).r;
			float v5 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).r;
		#endif

		gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the G channel.
 */
vec3 calcGradientG(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

        vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).g);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).g);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).g);
			float v3 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).g);
			float v4 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).g);
			float v5 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).g);
		#else
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).g;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).g;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).g;
			float v3 = textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).g;
			float v4 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).g;
			float v5 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).g;
		#endif

		gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the B channel.
 */
vec3 calcGradientB(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {
		
		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).b);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).b);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).b);
			float v3 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).b);
			float v4 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).b);
			float v5 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).b);
		#else
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).b;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).b;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).b;
			float v3 = textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).b;
			float v4 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).b;
			float v5 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).b;
		#endif

		gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
		gradient *= volumeParameters.datasetSpacingRCP_;
		
		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculate the gradient based on the A channel.
 */
vec3 calcGradientA(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos, float t, vec3 dir) {
    vec3 gradient;

    if (t == 0.0) gradient = fixClipBorderGradient(voxPos, dir);
    else {

		vec3 offset = volumeParameters.datasetDimensionsRCP_;

		//#define CONSIDER_TF
		#ifdef CONSIDERTF
			float v0 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).a);
			float v1 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).a);
			float v2 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).a);
			float v3 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).a);
			float v4 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).a);
			float v5 = texture1D(transferFunc_, textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).a);
		#else
			float v0 = textureLookup3D(volume, volumeParameters, voxPos + vec3(offset.x, 0.0, 0.0)).a;
			float v1 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, offset.y, 0)).a;
			float v2 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, offset.z)).a;
			float v3 = textureLookup3D(volume, volumeParameters, voxPos + vec3(-offset.x, 0, 0)).a;
			float v4 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, -offset.y, 0)).a;
			float v5 = textureLookup3D(volume, volumeParameters, voxPos + vec3(0, 0, -offset.z)).a;
		#endif

		gradient = vec3(v3 - v0, v4 - v1, v5 - v2) * 0.5;
		gradient *= volumeParameters.datasetSpacingRCP_;
		
   		#ifdef BITDEPTH_12
			gradient *= 16.0;
		#endif

	}    
    return gradient;
}

/**
 * Calculates gradients for the eight neighbours and
 * filters linearily between those.
 */
vec3 calcGradientFiltered(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPos) {

	vec3 delta = volumeParameters.datasetDimensionsRCP_;
	
	vec3 g0 = calcGradientA(volume, volumeParameters, voxPos, 0.5, vec3(0.0));

	vec3 g1 = calcGradientA(volume, volumeParameters, voxPos+vec3(-delta.x, -delta.y, -delta.z), 0.5, vec3(0.0));
	vec3 g2 = calcGradientA(volume, volumeParameters, voxPos+vec3( delta.x,  delta.y,  delta.z), 0.5, vec3(0.0));

	vec3 g3 = calcGradientA(volume, volumeParameters, voxPos+vec3(-delta.x,  delta.y, -delta.z), 0.5, vec3(0.0));
	vec3 g4 = calcGradientA(volume, volumeParameters, voxPos+vec3( delta.x, -delta.y,  delta.z), 0.5, vec3(0.0));

	vec3 g5 = calcGradientA(volume, volumeParameters, voxPos+vec3(-delta.x, -delta.y,  delta.z), 0.5, vec3(0.0));
	vec3 g6 = calcGradientA(volume, volumeParameters, voxPos+vec3( delta.x,  delta.y, -delta.z), 0.5, vec3(0.0));

	vec3 g7 = calcGradientA(volume, volumeParameters, voxPos+vec3(-delta.x,  delta.y,  delta.z), 0.5, vec3(0.0));
	vec3 g8 = calcGradientA(volume, volumeParameters, voxPos+vec3( delta.x, -delta.y, -delta.z), 0.5, vec3(0.0));
	
	vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
	vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
	return mix(g0, mix(mix0, mix1, 0.5), 0.75);
}

/**
 * Calculates a voxel's gradient in volume object space.
 *
 * @par am volume the voxel's volume
 * @param volumeParameters additional information about the passed volume
 * @param voxPosTex the voxel position in texture space
 */
vec3 calcGradient(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 voxPosTex) {
	return calcGradientA(volume, volumeParameters, voxPosTex, 0.5, vec3(0.0));
}

#endif //CALC_GRADIENTS
