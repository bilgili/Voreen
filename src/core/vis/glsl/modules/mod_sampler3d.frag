
/*
 * This struct contains information about a volume, like
 * its dimensions and spacing. Additionally, the reciprocal
 * values of all parameters are available (suffix RCP) .
 * The values are set automatically by 
 * VolumeRenderer::bindVolumes() if necessary.
 */
struct VOLUME_PARAMETERS {
    vec3 datasetDimensions_;        // the dataset's resolution, e.g. [ 256.0, 128.0, 128.0]
    vec3 datasetDimensionsRCP_;
    vec3 datasetSpacing_;           // set dataset's voxel size, e.g. [ 2.0, 0.5, 1.0]
    vec3 datasetSpacingRCP_;
    vec3 volumeCubeSize_;           // the volume's size in its object coordinates, e.g. [ 1.0, 0.5, 0.5]
    vec3 volumeCubeSizeRCP_;
    vec3 texCoordScaleFactor_;      // scale factor for tex coords, if VRN_TEXTURE_3D_SCALED is used
    vec3 texCoordScaleFactorRCP_; 
};
	
/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates 
 * the corresponding VOLUME_PARAMETERS struct has to be passed 
 *
 */
vec4 textureLookup3D(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
	
    #if defined(VRN_TEXTURE_3D)
        #if defined(SPLITNUM)
            // this is not really correct, since it's possible a volume other than the volume to be rendered
            // is passed... it's a temporary solution until I find a better way to include the split-Volumes
            // into the rest of the shader-sourcecode FL
            return lookupSplitVolume(texCoords);
        #else
            return texture3D(volume, texCoords);
        #endif
    #elif defined(VRN_TEXTURE_3D_SCALED)
        return texture3D(volume, texCoords*volumeParameters.texCoordScaleFactor_);
    #endif   
}
