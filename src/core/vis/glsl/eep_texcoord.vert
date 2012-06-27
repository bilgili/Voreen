#include "modules/mod_sampler3d.frag"           // contains struct VOLUME_PARAMETERS 

uniform VOLUME_PARAMETERS volumeParameters_;    // additional information about the volume the eep are generated for

varying vec4 eep_;

/**
 * Similar to eep_simple.vert, but uses texture coordinates instead of vertex positions.
 * This allows for deformations when texture coordinate and vertex position differ. The
 * w-component of the texture coordinates is written to the alpha channel.
 */
void main() {
    eep_.xyz = (gl_MultiTexCoord0.xyz * volumeParameters_.volumeCubeSizeRCP_) + 0.5;
    eep_ = vec4(clamp(eep_.xyz, 0.0, 1.0), gl_MultiTexCoord0.w);

    // Brackets are important! Without them the compiler may do costly matrix multiplications
    // instead of a cheap matrix-vector multiplication.
    // 
    // On NVIDIA 97.55 (Linux) it also triggers a compiler(?) bug, resulting in black rendering
    // of the entry params in TexCoordEntryExitPoints.
    gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex);
}
