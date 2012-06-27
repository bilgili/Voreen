/**
 * Calculates the depth value for the current sample specified by the parameter t.
 **/
float calculateDepthValue(float t) {
	// obtain coordinates of proxy front face in world coordinates
	vec4 front = vec4((gl_FragCoord.x*screenDimRCP_.x*2.0)-1.0,
			    	  (gl_FragCoord.y*screenDimRCP_.y*2.0)-1.0,
				      (textureLookup2D(entryParamsDepth_, gl_FragCoord.xy).z*2.0)-1.0,
				      1.0);
	front = gl_ModelViewProjectionMatrixInverse * front;
	
	// obtain coordinates of proxy back face in world coordinates				   
	vec4 back  = vec4((gl_FragCoord.x*screenDimRCP_.x*2.0)-1.0,
			    	  (gl_FragCoord.y*screenDimRCP_.y*2.0)-1.0,
				      (textureLookup2D(exitParamsDepth_, gl_FragCoord.xy).z*2.0)-1.0,
				      1.0);

    back = gl_ModelViewProjectionMatrixInverse * back;
	
	// interpolate current depth value in world coordinates
	// transform back in window coordinates and return
	vec4 current = front+(t*(back-front));
	current = gl_ModelViewProjectionMatrix * current;
	return ((current.z/current.w)+1.0)/2.0;
}
