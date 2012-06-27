
#include "modules/mod_sampler3d.frag"           // contains struct VOLUME_PARAMETERS 

uniform VOLUME_PARAMETERS volumeParameters_;    // additional information about the volume the eep are generated for

varying vec4 eep_;

/**
 * Simply get the provided proxy geometry's vertex position, transform it to texture space
 * and put it into eep_. OpenGL will interpolate the values for us.
 */
void main()
{
    // transform eep vertex from object space to texture space
    eep_.xyz = (gl_Vertex.xyz * volumeParameters_.volumeCubeSizeRCP_) + vec3(0.5);
    eep_ = vec4(eep_.xyz, 1.0);

    // transform vertex position into world coordinates
    vec4 vertexPos = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ProjectionMatrix * vertexPos;
    
    #ifdef VRN_USE_CLIP_PLANE
		gl_ClipVertex = gl_Vertex;
    #endif
}
