#include "modules/mod_sampler3d.frag"           // contains struct VOLUME_PARAMETERS 

uniform VOLUME_PARAMETERS volumeParameters_;    // additional information about the volume the eep are generated for

varying vec4 eep_;

uniform vec3 volSize_;

/**
 * Simple EEP using vertex positions, used for deformation. 
 */
void main() {
    eep_.xyz = (gl_Vertex.xyz * volumeParameters_.volumeCubeSizeRCP_) + 0.5;
    
    eep_.xyz = clamp(eep_.xyz, 0.0, 1.0);
      
    gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex);
}
