/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "modules/bricking/mod_adaptive_sampling.frag"
#include "modules/bricking/mod_basics.frag"
#include "modules/bricking/mod_global_variables.frag"
#include "modules/bricking/mod_interpolation.frag"
#include "modules/bricking/mod_math.frag"
#include "modules/bricking/mod_uniforms.frag"

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
	float bitDepthScale_;
};

/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates 
 * the corresponding VOLUME_PARAMETERS struct has to be passed. 
 * Before returning the fetched value it is normalized to the interval [0,1], in order to deal
 * with 12 bit data sets.
 */
vec4 textureLookup3D(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
	vec4 result;
    #if defined(VRN_TEXTURE_3D)
        result = texture3D(volume, texCoords);
    #elif defined(VRN_TEXTURE_3D_SCALED)
        result = texture3D(volume, texCoords*volumeParameters.texCoordScaleFactor_);
    #endif
	result *= volumeParameters.bitDepthScale_;
	return result;
}       

uniform sampler3D packedVolume_;                             
uniform VOLUME_PARAMETERS packedVolumeParameters_;          

uniform sampler3D indexVolume_;                            
uniform VOLUME_PARAMETERS indexVolumeParameters_;

uniform float numbricksX_;
uniform float numbricksY_;
uniform float numbricksZ_;
uniform float brickSizeX_;
uniform float brickSizeY_;
uniform float brickSizeZ_;
uniform float maxbricksize_;

uniform float indexVolumeFactorX_;
uniform float indexVolumeFactorY_;
uniform float indexVolumeFactorZ_;
uniform float offsetFactorX_;
uniform float offsetFactorY_;
uniform float offsetFactorZ_;

uniform bool useInterpolation_;
uniform bool transparentBorders_;
uniform bool coarsenessOn_;

//variables neccessary for the interpolation. They are needed in several
//functions and therefor global, as it would'nt be nice to pass them all
//everytime we call one of those functions
float edge12,edge13,edge15,edge24,edge26,edge34,edge37;
float edge48,edge56,edge57,edge68,edge78;

vec3 llfBlock,llbBlock,ulfBlock,ulbBlock,lrfBlock,lrbBlock,urfBlock,urbBlock;

vec4 llfBlockValue,llbBlockValue,ulfBlockValue,ulbBlockValue,lrfBlockValue;
vec4 lrbBlockValue,urfBlockValue,urbBlockValue;

vec4 llfBlockIndexVolSample,llbBlockIndexVolSample,ulfBlockIndexVolSample,ulbBlockIndexVolSample;
vec4 lrfBlockIndexVolSample,lrbBlockIndexVolSample,urfBlockIndexVolSample,urbBlockIndexVolSample;

float llfBlockBoundaryDistance,llbBlockBoundaryDistance,ulfBlockBoundaryDistance,ulbBlockBoundaryDistance;
float lrfBlockBoundaryDistance,lrbBlockBoundaryDistance,urfBlockBoundaryDistance,urbBlockBoundaryDistance;

float llfBlockWeight,llbBlockWeight,ulfBlockWeight,ulbBlockWeight,lrfBlockWeight;
float lrbBlockWeight,urfBlockWeight,urbBlockWeight;

//is that specific block inside the volume?
bool llfBlockValid,llbBlockValid,ulfBlockValid,ulbBlockValid,lrfBlockValid,lrbBlockValid;
bool urfBlockValid,urbBlockValid;

//This is the scalefactor of the brick holding the current sample. To make this global is not nice
//FIXME (s_rade02)
float currentScaleFactor;

float exponent(float basis, float exponent) {
	return exp(exponent*log(basis));
}

float log2(float value) {
	return log(value) / log(2.0);
}

/**
* Clamps the given value to the interval [min,max]
*/
float clampValue(float value,float min,float max) {
	if (value < min)
		return min;
	if (value >max)
		return max;
	return value;
}

bool isValueBetween(float min,float max, float value) {
	if (value < min)
		return false;
	if (value > max)
		return false;
	return true;
}


/**
* Checks if the given position in volume coordinates ( [0,1] ) is inside the
* volume or not.
*/
bool isInsideVolume(vec3 volumeCoordinates) {
	if (volumeCoordinates.x < 0.0 || volumeCoordinates.y < 0.0 || volumeCoordinates.z < 0.0)
		return false;

	if (volumeCoordinates.x >= 1.0 || volumeCoordinates.y >= 1.0 || volumeCoordinates.z >= 1.0)
		return false;

	return true;
}



/**
* Clamps the sample position to the inside of the bounding 
* box of all samples in a brick. That bounding box depends 
* on the bricks level of detail, indicated by the scalefactor.
* This is not thoroughly tested but apperas to be working.
*/
vec3 clampPositionToSampleBoundaries(vec3 blockStart,vec3 pos, float scalefactor) {
	vec3 result;
	float numVoxels = maxbricksize_* (1.0/scalefactor);
	float levelOfDetail = log2(numVoxels);
	float distance = 1.0 / exponent(2.0,1.0+levelOfDetail);
	
	//That distance corresponds to the global original texture,we
	//have to adapt it to the block
	vec3 distanceInBlock;
	distanceInBlock.x = distance / numbricksX_;
	distanceInBlock.y = distance / numbricksY_;
	distanceInBlock.z = distance / numbricksZ_;
	
	vec3 lowerBoundary;
	lowerBoundary.x = blockStart.x + distanceInBlock.x;
	lowerBoundary.y = blockStart.y + distanceInBlock.y;
	lowerBoundary.z = blockStart.z + distanceInBlock.z;
	
	vec3 upperBoundary;
	upperBoundary.x = blockStart.x+(1.0 / numbricksX_)-distanceInBlock.x; 
	upperBoundary.y = blockStart.y+(1.0 / numbricksY_)-distanceInBlock.y;
	upperBoundary.z = blockStart.z+(1.0 / numbricksZ_)-distanceInBlock.z;
	
	result.x = clampValue(pos.x,lowerBoundary.x,upperBoundary.x);
	
	result.y = clampValue(pos.y,lowerBoundary.y,upperBoundary.y);
	
	result.z = clampValue(pos.z,lowerBoundary.z,upperBoundary.z);
	
	return result;
}

/**
* More or less the same function as above, but here only the fact if clamping 
* is neccessary or not is returned. Not thoroughly tested, but looks ok.
*/
bool insideSampleBoundaries(vec3 blockStart,vec3 pos, float scalefactor) {
	vec3 result;
	float numVoxels = maxbricksize_* (1.0/scalefactor);
	float levelOfDetail = log2(numVoxels);
	float distance = 1.0 / exponent(2.0,1.0+levelOfDetail);

	//That distance corresponds to the global original texture,we
	//have to adapt it to the block
	vec3 distanceInBlock;
	distanceInBlock.x = distance / numbricksX_;
	distanceInBlock.y = distance / numbricksY_;
	distanceInBlock.z = distance / numbricksZ_;
	
	vec3 lowerBoundary;
	lowerBoundary.x = blockStart.x + distanceInBlock.x;
	lowerBoundary.y = blockStart.y + distanceInBlock.y;
	lowerBoundary.z = blockStart.z + distanceInBlock.z;
	
	vec3 upperBoundary;
	upperBoundary.x = blockStart.x+(1.0 / numbricksX_)-distanceInBlock.x; 
	upperBoundary.y = blockStart.y+(1.0 / numbricksY_)-distanceInBlock.y;
	upperBoundary.z = blockStart.z+(1.0 / numbricksZ_)-distanceInBlock.z;
	
	result.x = clampValue(pos.x,lowerBoundary.x,upperBoundary.x);
	if (result.x != pos.x)
		return false;
	result.y = clampValue(pos.y,lowerBoundary.y,upperBoundary.y);
	if (result.y != pos.y)
		return false;
	result.z = clampValue(pos.z,lowerBoundary.z,upperBoundary.z);
	if (result.z != pos.z)
		return false;
	
	return true;
}

/**
* Converts a position in the volume(the original volume)
* to block coordinates. 
*/ 
vec3 convertToBlockCoordinates(vec3 volumePos) {
	//convert the position to block coordinates (meaning: 7.3 -> 7th block comma 3)
	vec3 blockCoords;
	blockCoords.x = volumePos.x*numbricksX_;
	blockCoords.y = volumePos.y*numbricksY_;
	blockCoords.z = volumePos.z*numbricksZ_;
	return blockCoords;
}

/**
* Converts a position in block coordinates to a position
* in the original volume (between 0 and 1)
*/ 
vec3 convertToVolumeCoordinates(vec3 blockCoords) {
	vec3 volumePos;
	volumePos.x = blockCoords.x / numbricksX_;
	volumePos.y = blockCoords.y / numbricksY_;
	volumePos.z = blockCoords.z / numbricksZ_;
	return volumePos;
}

/**
* Calculates the block coordinates of the lower left 
* front block of a position in the original volume
* given in block coordinates. 
* 
*/
vec3 calculateLowerLeftFrontBlock(vec3 blockCoords) {
	vec3 result;
	result.x = floor( clampValue( (blockCoords.x - 0.5) ,0.0, numbricksX_-1.0) );
	result.y = floor( clampValue( (blockCoords.y - 0.5) ,0.0, numbricksY_-1.0) );
	result.z = floor( clampValue( (blockCoords.z - 0.5) ,0.0, numbricksZ_-1.0) );
	return result;
} 

/**
* Calculates the "local" coordinates of a position in the 
* original volume given in block coordinates. Local coordinates
* means the position relative to the intersection of the 8 blocks
* in the neighborhood.
*/
vec3 calculateLocalCoordinates( vec3 globalBlockCoords, vec3 lowerLeftFrontBlock) {
	vec3 coords;
	coords.x = globalBlockCoords.x - lowerLeftFrontBlock.x-1.0;
	coords.y = globalBlockCoords.y - lowerLeftFrontBlock.y-1.0;
	coords.z = globalBlockCoords.z - lowerLeftFrontBlock.z-1.0;
	return coords;	
}

/**
* Calculates in which brick a given sample resides. For example
* position (0.3,0.8,0.7) might lie in brick (3,5,7). This is just 
* an arbitrary example and might not be realistic :)
*/ 
vec3 calculateBrickStartPos(vec3 sample) {
	vec3 brickStartPos;
    brickStartPos.x = floor(sample.x*numbricksX_) / numbricksX_;
    brickStartPos.y = floor(sample.y*numbricksY_) / numbricksY_;
    brickStartPos.z = floor(sample.z*numbricksZ_) / numbricksZ_;
    return brickStartPos;
}

/**
* Calculates the coordinates for the lookup in the index volume. 
* Ideally this would not be neccessary and we could simply make the
* lookup at the brickStartPos, but if you have a 2D Texture with
* 4 texels, texel 1 would be at (0.25,0.25) and not (0,0) (which
* would be the brickStartPos). That's why we have to make this
* adjustment.
*/
vec3 calculateLookupPos(vec3 brickstart) {
	vec3 result;
	result.x = brickstart.x + 1.0 / (2.0 * numbricksX_ );
	result.y = brickstart.y + 1.0 / (2.0 * numbricksY_ );
	result.z = brickstart.z + 1.0 / (2.0 * numbricksZ_ );
	
	return result;
}

/**
* Make the lookup in the index volume in order to get the
* coordinates to the block (where the current sample lies in)
* in the packed texture.
*/
vec4 indexVolumeLookup(vec3 lookupPos) {
	vec4 indexVolumeSample = textureLookup3D(indexVolume_, indexVolumeParameters_,lookupPos);
	// The result has to be converted to [0,1]. That's because when you create the index
	// texture, 256 might be the maximum position in the x direction and should therefor equal
	// 1 in the shader. But the index texture is of the type uint16_t, and therefor has a maximum
	// of 65535, which results in 256 not equaling 1 at all. The following equations take care of that.
    indexVolumeSample.x = indexVolumeSample.x* indexVolumeFactorX_;
    indexVolumeSample.y = indexVolumeSample.y* indexVolumeFactorY_;
    indexVolumeSample.z = indexVolumeSample.z* indexVolumeFactorZ_;
    //This is the scalefactor.
    indexVolumeSample.a =indexVolumeSample.a * 65535.0; 
    return indexVolumeSample;
}

/**
* Lookup the sample in the packed texture. 
*/
vec4 normalPackedVolumeLookup(vec3 sample,vec3 brickStartPos,vec4 indexVolumeSample) {
	vec3 offset;
	float scaleFactor = 1.0 / indexVolumeSample.a;
    offset.x = (sample.x-brickStartPos.x)* scaleFactor* offsetFactorX_;
    offset.y = (sample.y-brickStartPos.y)* scaleFactor* offsetFactorY_;
    offset.z = (sample.z-brickStartPos.z)* scaleFactor* offsetFactorZ_;
	
	vec3 packedVolumeSample;

	packedVolumeSample.x = indexVolumeSample.x+ offset.x;
	packedVolumeSample.y = indexVolumeSample.y+ offset.y;
	packedVolumeSample.z = indexVolumeSample.z+ offset.z;
	              
	vec4 result = textureLookup3D(packedVolume_, packedVolumeParameters_, packedVolumeSample);

	return result;
}

/**
* Gets the boundary distance of a given block, meaning the distance
* of the texels to the boundary of the texture. Tested roughly and seems
* to work, but not tested extensively.
*/
float getBoundaryDistanceOfBlock(float scalefactor) {
	float numVoxels = maxbricksize_* (1.0/scalefactor);
	float levelOfDetail = log2(numVoxels);
	float distance = 1.0 / exponent(2.0,1.0+levelOfDetail);

	return distance;
}

/**
* Takes a sample from the block in block coordinates. coords are the sample coordinates in 
* the local coordinate system used in the interpolation (also in global block coordinates)
* Should work.
*/
vec4 takeClampedSampleFromBlock(vec3 block,vec3 sample,vec4 indexVolumeSample) {

	vec3 clampedSample = clampPositionToSampleBoundaries(block,sample,indexVolumeSample.a);
	
	vec4 result = normalPackedVolumeLookup(clampedSample,block,indexVolumeSample);
	return result;
}

/**
* Calculates the term used in the minimum distance interpolation.
*/
float calculateTermForMinimumDistance(float coordinateValue,float boundaryDistance1, float boundaryDistance2) {
	float result = min(boundaryDistance1,boundaryDistance2);
	
	result = coordinateValue / (2.0 * result);
	
	result = result+0.5;

	result = clampValue(result,0.0,1.0);

	return result;
}

/**
* Calculates the term used in the maximum distance interpolation.
*/
float calculateTermForMaximumDistance(float coordinateValue,float boundaryDistance1, float boundaryDistance2) {
	
	float term1 = coordinateValue + boundaryDistance1;
	float term2 = boundaryDistance1 + boundaryDistance2;

	float result = term1 / term2;

	result = clampValue(result,0.0,1.0);

	return result;
}

/**
* Performs minimum distance interpolation for the given coordinates in
* local block coordinates. That means the edge weights are calculated
* which are then used to calculate the current samples color. This 1.0 else 0.0
* approach doesn't work.
*/
void minimumDistanceInterpolation(vec3 coords) {

	if (llfBlockValid && lrfBlockValid)
		edge12 = calculateTermForMinimumDistance(coords.x,llfBlockBoundaryDistance,lrfBlockBoundaryDistance);

	if (llfBlockValid && ulfBlockValid)
		edge13 = calculateTermForMinimumDistance(coords.y,llfBlockBoundaryDistance,ulfBlockBoundaryDistance);

	if (llfBlockValid && llbBlockValid)
		edge15 = calculateTermForMinimumDistance(coords.z,llfBlockBoundaryDistance,llbBlockBoundaryDistance);

	if (lrfBlockValid && urfBlockValid)
		edge24 = calculateTermForMinimumDistance(coords.y,lrfBlockBoundaryDistance,urfBlockBoundaryDistance);

	if (lrfBlockValid && lrbBlockValid)
		edge26 = calculateTermForMinimumDistance(coords.z,lrfBlockBoundaryDistance,lrbBlockBoundaryDistance);

	if (ulfBlockValid && urfBlockValid)
		edge34 = calculateTermForMinimumDistance(coords.x,ulfBlockBoundaryDistance,urfBlockBoundaryDistance);

	if (ulfBlockValid && ulbBlockValid)
		edge37 = calculateTermForMinimumDistance(coords.z,ulfBlockBoundaryDistance,ulbBlockBoundaryDistance);

	if (urfBlockValid && urbBlockValid)
		edge48 = calculateTermForMinimumDistance(coords.z,urfBlockBoundaryDistance,urbBlockBoundaryDistance);

	if (llbBlockValid && lrbBlockValid)
		edge56 = calculateTermForMinimumDistance(coords.x,llbBlockBoundaryDistance,lrbBlockBoundaryDistance);

	if (llbBlockValid && ulbBlockValid)
		edge57 = calculateTermForMinimumDistance(coords.y,llbBlockBoundaryDistance,ulbBlockBoundaryDistance);

	if (lrbBlockValid && urbBlockValid)
		edge68 = calculateTermForMinimumDistance(coords.y,lrbBlockBoundaryDistance,urbBlockBoundaryDistance);

	if (ulbBlockValid && urbBlockValid)
		edge78 = calculateTermForMinimumDistance(coords.x,ulbBlockBoundaryDistance,urbBlockBoundaryDistance);

}

/**
* Performs maximum distance interpolation for the given coordinates in
* local block coordinates. That means the edge weights are calculated
* which are then used to calculate the current samples color. This 1.0 else 0.0
* approach doesn't work.
*/
void maximumDistanceInterpolation(vec3 coords) {

	if (llfBlockValid && lrfBlockValid)
		edge12 = calculateTermForMaximumDistance(coords.x,llfBlockBoundaryDistance,lrfBlockBoundaryDistance);

	if (llfBlockValid && ulfBlockValid)
		edge13 = calculateTermForMaximumDistance(coords.y,llfBlockBoundaryDistance,ulfBlockBoundaryDistance);

	if (llfBlockValid && llbBlockValid)
		edge15 = calculateTermForMaximumDistance(coords.z,llfBlockBoundaryDistance,llbBlockBoundaryDistance);

	if (lrfBlockValid && urfBlockValid)
		edge24 = calculateTermForMaximumDistance(coords.y,lrfBlockBoundaryDistance,urfBlockBoundaryDistance);

	if (lrfBlockValid && lrbBlockValid)
		edge26 = calculateTermForMaximumDistance(coords.z,lrfBlockBoundaryDistance,lrbBlockBoundaryDistance);

	if (ulfBlockValid && urfBlockValid)
		edge34 = calculateTermForMaximumDistance(coords.x,ulfBlockBoundaryDistance,urfBlockBoundaryDistance);

	if (ulfBlockValid && ulbBlockValid)
		edge37 = calculateTermForMaximumDistance(coords.z,ulfBlockBoundaryDistance,ulbBlockBoundaryDistance);

	if (urfBlockValid && urbBlockValid)
		edge48 = calculateTermForMaximumDistance(coords.z,urfBlockBoundaryDistance,urbBlockBoundaryDistance);

	if (llbBlockValid && lrbBlockValid)
		edge56 = calculateTermForMaximumDistance(coords.x,llbBlockBoundaryDistance,lrbBlockBoundaryDistance);

	if (llbBlockValid && ulbBlockValid)
		edge57 = calculateTermForMaximumDistance(coords.y,llbBlockBoundaryDistance,ulbBlockBoundaryDistance);

	if (lrbBlockValid && urbBlockValid)
		edge68 = calculateTermForMaximumDistance(coords.y,lrbBlockBoundaryDistance,urbBlockBoundaryDistance);

	if (ulbBlockValid && urbBlockValid)
		edge78 = calculateTermForMaximumDistance(coords.x,ulbBlockBoundaryDistance,urbBlockBoundaryDistance);
}

void getIndexVolumeSamplesOfAllBlocks() {
	vec3 temp;
	if (llfBlockValid) {
		temp = llfBlock;
		temp = calculateLookupPos(temp);
		llfBlockIndexVolSample = indexVolumeLookup(temp);
		llfBlockBoundaryDistance = getBoundaryDistanceOfBlock(llfBlockIndexVolSample.a);
	}

	if (llbBlockValid) {
		temp = llbBlock;
		temp = calculateLookupPos(temp);
		llbBlockIndexVolSample = indexVolumeLookup(temp);
		llbBlockBoundaryDistance = getBoundaryDistanceOfBlock(llbBlockIndexVolSample.a);
	}

	if (ulfBlockValid) {
		temp = ulfBlock;
		temp = calculateLookupPos(temp);
		ulfBlockIndexVolSample = indexVolumeLookup(temp);
		ulfBlockBoundaryDistance = getBoundaryDistanceOfBlock(ulfBlockIndexVolSample.a);
	}

	if (ulbBlockValid) {
		temp = ulbBlock;
		temp = calculateLookupPos(temp);
		ulbBlockIndexVolSample = indexVolumeLookup(temp);
		ulbBlockBoundaryDistance = getBoundaryDistanceOfBlock(ulbBlockIndexVolSample.a);
	}

	if (lrfBlockValid) {
		temp = lrfBlock;
		temp = calculateLookupPos(temp);
		lrfBlockIndexVolSample = indexVolumeLookup(temp);
		lrfBlockBoundaryDistance = getBoundaryDistanceOfBlock(lrfBlockIndexVolSample.a);
	}

	if (lrbBlockValid) {
		temp = lrbBlock;
		temp = calculateLookupPos(temp);
		lrbBlockIndexVolSample = indexVolumeLookup(temp);
		lrbBlockBoundaryDistance = getBoundaryDistanceOfBlock(lrbBlockIndexVolSample.a);
	}

	if (urfBlockValid) {
		temp = urfBlock;
		temp = calculateLookupPos(temp);
		urfBlockIndexVolSample = indexVolumeLookup(temp);
		urfBlockBoundaryDistance = getBoundaryDistanceOfBlock(urfBlockIndexVolSample.a);
	}
	
	if (urbBlockValid) {
		temp = urbBlock;
		temp = calculateLookupPos(temp);
		urbBlockIndexVolSample = indexVolumeLookup(temp);
		urbBlockBoundaryDistance = getBoundaryDistanceOfBlock(urbBlockIndexVolSample.a);
	}
}

/**
* Does interblock interpolation for the given position in the volume.
* If that position isn't at the border of a block, this interpolation should'nt
* be done. So a check should be performed earlier. 
*/
vec4 interBlockInterpolation(vec3 volumePos) {
	vec3 blockCoordinates = convertToBlockCoordinates(volumePos); 

	llfBlock = calculateLowerLeftFrontBlock(blockCoordinates);

	vec3 localCoordinates = calculateLocalCoordinates(blockCoordinates,llfBlock);

	llfBlock = convertToVolumeCoordinates(llfBlock);

	llbBlock=vec3(llfBlock.x,llfBlock.y,llfBlock.z +brickSizeZ_);
	ulfBlock=vec3(llfBlock.x,llfBlock.y+brickSizeY_,llfBlock.z);
	ulbBlock=vec3(llfBlock.x,llfBlock.y+brickSizeY_,llfBlock.z+brickSizeZ_);
	lrfBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y,llfBlock.z);
	lrbBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y,llfBlock.z+brickSizeZ_);
	urfBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y+brickSizeY_,llfBlock.z);
	urbBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y+brickSizeY_,llfBlock.z+brickSizeZ_);

	llfBlockValid = isInsideVolume(llfBlock);
	llbBlockValid = isInsideVolume(llbBlock);
	ulfBlockValid = isInsideVolume(ulfBlock);
	ulbBlockValid = isInsideVolume(ulbBlock);
	lrfBlockValid = isInsideVolume(lrfBlock);
	lrbBlockValid = isInsideVolume(lrbBlock);
	urfBlockValid = isInsideVolume(urfBlock);
	urbBlockValid = isInsideVolume(urbBlock);

	getIndexVolumeSamplesOfAllBlocks();

	llfBlockValue=vec4(0.0);
	llbBlockValue=vec4(0.0);
	ulfBlockValue=vec4(0.0);
	ulbBlockValue=vec4(0.0);
	lrfBlockValue=vec4(0.0);
	lrbBlockValue=vec4(0.0);
	urfBlockValue=vec4(0.0);
	urbBlockValue=vec4(0.0);

	if (llfBlockValid) {
		llfBlockValue=takeClampedSampleFromBlock(llfBlock,volumePos,llfBlockIndexVolSample);
	}
	if (llbBlockValid) {
		llbBlockValue=takeClampedSampleFromBlock(llbBlock,volumePos,llbBlockIndexVolSample);
	}
	if (ulfBlockValid) {
		ulfBlockValue=takeClampedSampleFromBlock(ulfBlock,volumePos,ulfBlockIndexVolSample);
	}
	if (ulbBlockValid) {
		ulbBlockValue=takeClampedSampleFromBlock(ulbBlock,volumePos,ulbBlockIndexVolSample);
	}
	if (lrfBlockValid) {
		lrfBlockValue=takeClampedSampleFromBlock(lrfBlock,volumePos,lrfBlockIndexVolSample);
	}
	if (lrbBlockValid) {
		lrbBlockValue=takeClampedSampleFromBlock(lrbBlock,volumePos,lrbBlockIndexVolSample);
	}
	if (urfBlockValid) {
		urfBlockValue=takeClampedSampleFromBlock(urfBlock,volumePos,urfBlockIndexVolSample);
	}
	if (urbBlockValid) {
		urbBlockValue=takeClampedSampleFromBlock(urbBlock,volumePos,urbBlockIndexVolSample);
	}

	edge12 =0.0;
	edge13 =0.0;
	edge15 =0.0;
	edge24 =0.0;
	edge26 =0.0;
	edge34 =0.0;
	edge37 =0.0;
	edge48 =0.0;
	edge56 =0.0;
	edge57 =0.0;
	edge68 =0.0;
	edge78 =0.0;

	//minimumDistanceInterpolation(localCoordinates);
	maximumDistanceInterpolation(localCoordinates);

	llfBlockWeight = (1.0 - edge12)*(1.0-edge13)*(1.0-edge15);
	lrfBlockWeight = edge12*(1.0-edge24)*(1.0-edge26);
	ulfBlockWeight = (1.0 - edge34)*edge13*(1.0-edge37);
	urfBlockWeight = edge34*edge24*(1.0-edge48);
	llbBlockWeight = (1.0 - edge56)*(1.0-edge57)*edge15;
	lrbBlockWeight = edge56*(1.0-edge68)*edge26;
	ulbBlockWeight = (1.0 - edge78)*edge57*edge37;
	urbBlockWeight = edge78*edge68*edge48;

	vec4 numerator=vec4(0.0);

	numerator=llfBlockValue*llfBlockWeight+
		llbBlockValue*llbBlockWeight+
		ulfBlockValue*ulfBlockWeight+
		ulbBlockValue*ulbBlockWeight+
		lrfBlockValue*lrfBlockWeight+
		lrbBlockValue*lrbBlockWeight+
		urfBlockValue*urfBlockWeight+
		urbBlockValue*urbBlockWeight;
	
	float denominator;
	denominator=llfBlockWeight+
		llbBlockWeight+
		ulfBlockWeight+
		ulbBlockWeight+
		lrfBlockWeight+
		lrbBlockWeight+
		urfBlockWeight+
		urbBlockWeight;	
	
	vec4 result = numerator / denominator;

	return result;
}

/**
* Lookup the current sample. This function calls several others, you
* however need only to use this one. 
*/ 
vec4 getVoxel(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {
	
	vec3 brickStartPos = calculateBrickStartPos(sample);
            
    vec3 lookupPos = calculateLookupPos(brickStartPos);
    
    vec4 indexVolumeSample = indexVolumeLookup(lookupPos);  

	currentScaleFactor = indexVolumeSample.a;
  
	vec4 voxel;

	if (useInterpolation_ && !coarsenessOn_) {
		if (insideSampleBoundaries(brickStartPos,sample,indexVolumeSample.a) ) {
			voxel = normalPackedVolumeLookup(sample, brickStartPos,indexVolumeSample);
		} else {
			voxel = interBlockInterpolation(sample);
		}
	} else {
		if (insideSampleBoundaries(brickStartPos,sample,indexVolumeSample.a) ) {
			voxel = normalPackedVolumeLookup(sample, brickStartPos,indexVolumeSample);
		} else {
			if (!transparentBorders_) {
				vec3 clampedSample = clampPositionToSampleBoundaries(brickStartPos,sample,indexVolumeSample.a);
				voxel = normalPackedVolumeLookup(clampedSample,brickStartPos,indexVolumeSample);
			} else {
				voxel = vec4(0.0,0.0,0.0,0.0);
			}
		}
	}
	//voxel *= volumeParameters.bitDepthScale_;
    return voxel;
}

vec4 getVoxelUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 sample) {
	
	vec3 brickStartPos = calculateBrickStartPos(sample);
            
    vec3 lookupPos = calculateLookupPos(brickStartPos);
    
    vec4 indexVolumeSample = indexVolumeLookup(lookupPos);  

	currentScaleFactor = indexVolumeSample.a;
  
	vec4 voxel;

	if (useInterpolation_ && !coarsenessOn_) {
		if (insideSampleBoundaries(brickStartPos,sample,indexVolumeSample.a) ) {
			voxel = normalPackedVolumeLookup(sample, brickStartPos,indexVolumeSample);
		} else {
			voxel = interBlockInterpolation(sample);
		}
	} else {
		if (insideSampleBoundaries(brickStartPos,sample,indexVolumeSample.a) ) {
			voxel = normalPackedVolumeLookup(sample, brickStartPos,indexVolumeSample);
		} else {
			if (!transparentBorders_) {
				vec3 clampedSample = clampPositionToSampleBoundaries(brickStartPos,sample,indexVolumeSample.a);
				voxel = normalPackedVolumeLookup(clampedSample,brickStartPos,indexVolumeSample);
			} else {
				voxel = vec4(0.0,0.0,0.0,0.0);
			}
		}
	}
    return voxel;
}

/*
 * Function for volume texture lookup. In addition to the volume and the texture coordinates 
 * the corresponding VOLUME_PARAMETERS struct has to be passed .
 * In contrast to textureLookup3D() this function does not normalize the intensity values,
 * in order to deal with 12 bit data sets.
 */
vec4 textureLookup3DUnnormalized(sampler3D volume, VOLUME_PARAMETERS volumeParameters, vec3 texCoords) {
    return getVoxelUnnormalized(volume, volumeParameters, texCoords);
}

int calculateStepsToSkip(vec3 sample, vec3 direction) {
	vec3 blockCoordinates = convertToBlockCoordinates(sample); 
	direction = convertToBlockCoordinates(direction);

	vec3 intraBlockCoordinates = blockCoordinates - floor(blockCoordinates);

	vec3 temp;
	if (direction.x < 0.0)
		temp.x = 0.0;
	else temp.x = 1.0;

	if (direction.y < 0.0)
		temp.y = 0.0;
	else temp.y = 1.0;

	if (direction.z < 0.0)
		temp.z = 0.0;
	else temp.z = 1.0;

	float term1 = (temp.x - intraBlockCoordinates.x) / direction.x;
	float term2 = (temp.y - intraBlockCoordinates.y) / direction.y;
	float term3 = (temp.z - intraBlockCoordinates.z) / direction.z;

	float steps = min(term1 , term2);
	steps = min(steps , term3);
	
	float stepsToSkip = min (1.0 + floor(steps), currentScaleFactor);
	int result = int(stepsToSkip);

	return result;
}




