/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

/**
* This file contains the functions necessary to use interblock interpolation
* to retrieve a sample. Interblock interpolation basically performs the interpolation
* that would otherwise run in hardware manually by retrieving samples from the eight
* surrounding bricks and interpolating between them, weighting the samples by their
* distance to the sample position. For more information see Ljung et al.'s paper
* "Multiresolution Interblock Interpolation in Direct Volume Rendering".
*/

/**
* Calculates the weight of an edge between two bricks. As the weight only depends
* on the boundary distance of the bricks and the coordinates in the local
* coordinate system (local block coordinates), only these parameters are passed,
* and not the brick positions themselves. There are three ways to calculate the
* weights presented in Ljung et al.'s paper, the one used here is maximum
* distance interpolation.
*/
float calculateEdgeWeight(float coordinate, float boundaryDistance1,
                                      float boundaryDistance2) {

    float term1 = coordinate + boundaryDistance1;
    float term2 = boundaryDistance1 + boundaryDistance2;

    float result = term1 / term2;

    result = clamp(result,0.0,1.0);

    return result;
}

/**
* Calculates all edge weights which are afterwards used in the interblock
* interpolation. The only parameter are the local block coordinates, i.e.
* the samples position in local block coordinates. All other variables
* are declared globally (see mod_global_variables.frag).
*/
void calculateAllEdgeWeights(vec3 coords) {

    edge12 = llfBlockValid * lrfBlockValid*
        calculateEdgeWeight(coords.x, llfBlockBoundaryDistance, lrfBlockBoundaryDistance);

    edge13 = llfBlockValid * ulfBlockValid*
        calculateEdgeWeight(coords.y, llfBlockBoundaryDistance, ulfBlockBoundaryDistance);

    edge15 = llfBlockValid * llbBlockValid*
        calculateEdgeWeight(coords.z, llfBlockBoundaryDistance, llbBlockBoundaryDistance);

    edge24 = lrfBlockValid * urfBlockValid*
        calculateEdgeWeight(coords.y, lrfBlockBoundaryDistance,urfBlockBoundaryDistance);

    edge26 = lrfBlockValid * lrbBlockValid*
        calculateEdgeWeight(coords.z, lrfBlockBoundaryDistance, lrbBlockBoundaryDistance);

    edge34 = ulfBlockValid * urfBlockValid*
        calculateEdgeWeight(coords.x, ulfBlockBoundaryDistance, urfBlockBoundaryDistance);

    edge37 = ulfBlockValid * ulbBlockValid*
        calculateEdgeWeight(coords.z, ulfBlockBoundaryDistance, ulbBlockBoundaryDistance);

    edge48 = urfBlockValid * urbBlockValid*
        calculateEdgeWeight(coords.z, urfBlockBoundaryDistance, urbBlockBoundaryDistance);

    edge56 = llbBlockValid * lrbBlockValid*
        calculateEdgeWeight(coords.x, llbBlockBoundaryDistance, lrbBlockBoundaryDistance);

    edge57 = llbBlockValid * ulbBlockValid*
        calculateEdgeWeight(coords.y, llbBlockBoundaryDistance, ulbBlockBoundaryDistance);

    edge68 = lrbBlockValid * urbBlockValid*
        calculateEdgeWeight(coords.y, lrbBlockBoundaryDistance, urbBlockBoundaryDistance);

    edge78 = ulbBlockValid * urbBlockValid*
        calculateEdgeWeight(coords.x, ulbBlockBoundaryDistance, urbBlockBoundaryDistance);
}

/**
* Retrieves the index volume sample of each brick in the eight brick neighborhood. This
* information is needed very often, and retrieving it once and storing it globally saves
* several lookups. Also calculates the boundary distance of each brick right away.
*/
void getIndexVolumeSamplesOfAllBlocks() {
    vec3 temp;

    temp = llfBlock;
    temp = getIndexVolumePosition(temp);
    llfBlockIndexVolSample = indexVolumeLookup(temp);
    llfBlockBoundaryDistance = getBoundaryDistanceOfBlock(llfBlockIndexVolSample.a);

    temp = llbBlock;
    temp = getIndexVolumePosition(temp);
    llbBlockIndexVolSample = indexVolumeLookup(temp);
    llbBlockBoundaryDistance = getBoundaryDistanceOfBlock(llbBlockIndexVolSample.a);

    temp = ulfBlock;
    temp = getIndexVolumePosition(temp);
    ulfBlockIndexVolSample = indexVolumeLookup(temp);
    ulfBlockBoundaryDistance = getBoundaryDistanceOfBlock(ulfBlockIndexVolSample.a);

    temp = ulbBlock;
    temp = getIndexVolumePosition(temp);
    ulbBlockIndexVolSample = indexVolumeLookup(temp);
    ulbBlockBoundaryDistance = getBoundaryDistanceOfBlock(ulbBlockIndexVolSample.a);

    temp = lrfBlock;
    temp = getIndexVolumePosition(temp);
    lrfBlockIndexVolSample = indexVolumeLookup(temp);
    lrfBlockBoundaryDistance = getBoundaryDistanceOfBlock(lrfBlockIndexVolSample.a);

    temp = lrbBlock;
    temp = getIndexVolumePosition(temp);
    lrbBlockIndexVolSample = indexVolumeLookup(temp);
    lrbBlockBoundaryDistance = getBoundaryDistanceOfBlock(lrbBlockIndexVolSample.a);

    temp = urfBlock;
    temp = getIndexVolumePosition(temp);
    urfBlockIndexVolSample = indexVolumeLookup(temp);
    urfBlockBoundaryDistance = getBoundaryDistanceOfBlock(urfBlockIndexVolSample.a);

    temp = urbBlock;
    temp = getIndexVolumePosition(temp);
    urbBlockIndexVolSample = indexVolumeLookup(temp);
    urbBlockBoundaryDistance = getBoundaryDistanceOfBlock(urbBlockIndexVolSample.a);
}

/**
* Does interblock interpolation for the given position in the volume.
*/
vec4 interBlockInterpolation(vec3 samplePos) {

    /*Declare the variables that will hold the samples of the bricks in the
    * eight brick neighborhood later. The samples value will be interpolated
    * between these eight values.
    */
    vec4 llfBlockValue,llbBlockValue,ulfBlockValue,ulbBlockValue,lrfBlockValue;
    vec4 lrbBlockValue,urfBlockValue,urbBlockValue;

    /**
    * Declare the variables holding the weight of each brick in the eight brick
    * neighborhood. These weights indicate weight each brick's sample will have
    * in the interpolation.
    */
    float llfBlockWeight,llbBlockWeight,ulfBlockWeight,ulbBlockWeight,lrfBlockWeight;
    float lrbBlockWeight,urfBlockWeight,urbBlockWeight;

    //Convert the sample position to the block coordinate system.
    vec3 blockCoordinates = convertToBlockCoordinates(samplePos);

    //Calculate the brick to the lower left front of the sample position.
    llfBlock = calculateLowerLeftFrontBlock(blockCoordinates);

    //Setup the local coordinate system (local block coordinates) with the
    //boundary point of the eight bricks in the neighborhood as origin. Convert
    //the sample position to that coordinate system. (=> localCoordinates € [-1,1]^3)
    vec3 localCoordinates = calculateLocalCoordinates(blockCoordinates,llfBlock);

    //Convert the lower-left-front brick's coordinates to the normal coordinate system.
    //The values from the block coordinate system are no longer needed, as they were
    //only necessary to setup the local block coordinate system into which the sample
    //position was converted.
    llfBlock = convertToNormalCoordinates(llfBlock);

    //Calculate the position of the seven remaining bricks in the eight block neighborhood.
    llbBlock=vec3(llfBlock.x,llfBlock.y,llfBlock.z +brickSizeZ_);
    ulfBlock=vec3(llfBlock.x,llfBlock.y+brickSizeY_,llfBlock.z);
    ulbBlock=vec3(llfBlock.x,llfBlock.y+brickSizeY_,llfBlock.z+brickSizeZ_);
    lrfBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y,llfBlock.z);
    lrbBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y,llfBlock.z+brickSizeZ_);
    urfBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y+brickSizeY_,llfBlock.z);
    urbBlock=vec3(llfBlock.x+brickSizeX_,llfBlock.y+brickSizeY_,llfBlock.z+brickSizeZ_);

    //Check if these bricks are actually inside the volume.
    llfBlockValid = isInsideVolume(llfBlock);
    llbBlockValid = isInsideVolume(llbBlock);
    ulfBlockValid = isInsideVolume(ulfBlock);
    ulbBlockValid = isInsideVolume(ulbBlock);
    lrfBlockValid = isInsideVolume(lrfBlock);
    lrbBlockValid = isInsideVolume(lrbBlock);
    urfBlockValid = isInsideVolume(urfBlock);
    urbBlockValid = isInsideVolume(urbBlock);

    //Retrieve the index volume sample of all these bricks, so that lookups can be done.
    getIndexVolumeSamplesOfAllBlocks();

    //Set all sample values of the eight bricks to 0.
    llfBlockValue=vec4(0.0);
    llbBlockValue=vec4(0.0);
    ulfBlockValue=vec4(0.0);
    ulbBlockValue=vec4(0.0);
    lrfBlockValue=vec4(0.0);
    lrbBlockValue=vec4(0.0);
    urfBlockValue=vec4(0.0);
    urbBlockValue=vec4(0.0);

    //Take a sample from all eight bricks to use in the interpolation.
    llfBlockValue = llfBlockValid * takeClampedSampleFromBlock(llfBlock, samplePos,llfBlockIndexVolSample,
        llfBlockBoundaryDistance);
    llbBlockValue = llbBlockValid * takeClampedSampleFromBlock(llbBlock, samplePos,llbBlockIndexVolSample,
        llbBlockBoundaryDistance);
    ulfBlockValue = ulfBlockValid * takeClampedSampleFromBlock(ulfBlock, samplePos,ulfBlockIndexVolSample,
        ulfBlockBoundaryDistance);
    ulbBlockValue = ulbBlockValid * takeClampedSampleFromBlock(ulbBlock, samplePos,ulbBlockIndexVolSample,
        ulbBlockBoundaryDistance);
    lrfBlockValue = lrfBlockValid * takeClampedSampleFromBlock(lrfBlock, samplePos,lrfBlockIndexVolSample,
        lrfBlockBoundaryDistance);
    lrbBlockValue = lrbBlockValid * takeClampedSampleFromBlock(lrbBlock, samplePos,lrbBlockIndexVolSample,
        lrbBlockBoundaryDistance);
    urfBlockValue = urfBlockValid * takeClampedSampleFromBlock(urfBlock, samplePos,urfBlockIndexVolSample,
        urfBlockBoundaryDistance);
    urbBlockValue = urbBlockValid * takeClampedSampleFromBlock(urbBlock, samplePos,urbBlockIndexVolSample,
        urbBlockBoundaryDistance);

    //Set all edge weights to 0.
    edge12 = 0.0;
    edge13 = 0.0;
    edge15 = 0.0;
    edge24 = 0.0;
    edge26 = 0.0;
    edge34 = 0.0;
    edge37 = 0.0;
    edge48 = 0.0;
    edge56 = 0.0;
    edge57 = 0.0;
    edge68 = 0.0;
    edge78 = 0.0;

    //Calculate all edge weights.
    calculateAllEdgeWeights(localCoordinates);

    //Calculate the weights of the bricks.
    llfBlockWeight = (1.0 - edge12) * (1.0-edge13) * (1.0-edge15);
    lrfBlockWeight = edge12 * (1.0-edge24) * (1.0-edge26);
    ulfBlockWeight = (1.0 - edge34) * edge13 * (1.0-edge37);
    urfBlockWeight = edge34 * edge24 * (1.0-edge48);
    llbBlockWeight = (1.0 - edge56) * (1.0-edge57) * edge15;
    lrbBlockWeight = edge56 * (1.0-edge68) * edge26;
    ulbBlockWeight = (1.0 - edge78) * edge57 * edge37;
    urbBlockWeight = edge78 * edge68 * edge48;

    //Do the actual interpolation.
    vec4 numerator=vec4(0.0);

    numerator=llfBlockValue * llfBlockWeight+
        llbBlockValue * llbBlockWeight+
        ulfBlockValue * ulfBlockWeight+
        ulbBlockValue * ulbBlockWeight+
        lrfBlockValue * lrfBlockWeight+
        lrbBlockValue * lrbBlockWeight+
        urfBlockValue * urfBlockWeight+
        urbBlockValue * urbBlockWeight;

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
* Performs a lookup which uses interblock interpolation when retrieving the sample.
*/
vec4 interBlockInterpolationLookup(vec3 brick, vec3 sample, vec4 indexVolumeSample) {
    vec4 voxel;
    //The "if" here is actually faster than using interBlockInterpolation everywhere (but just a bit).
    if (insideSampleBoundary(brick, sample, indexVolumeSample.a) ) {
        //The sample is inside the sample boundary of the brick and no interblock interpolation
        //is necessary.
        voxel = normalPackedVolumeLookup(sample, brick, indexVolumeSample);
    }
    else {
        #ifdef INTERPOLATION_COARSENESS
            if (coarsenessOn_) {
                voxel = clampedPackedVolumeLookup(sample, brick, indexVolumeSample);
            }
            else {
                voxel = interBlockInterpolation(sample);
            }
        #else
            voxel = interBlockInterpolation(sample);
        #endif
    }
    return voxel;
}
