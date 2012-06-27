/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "commands_modify.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/bricking/brickinginformation.h"
#include "voreen/core/io/brickedvolumewriter.h"

#include "tgt/vector.h"

namespace voreen {

CommandCutToPieces::CommandCutToPieces() :
    Command("--cuttopieces", "", "Devide dataset into pieces with dimensions (DX, DY, DZ)",
        "<DX DY DZ IN OUT>", 5)
{
    loggerCat_ += "." + name_;
}

bool CommandCutToPieces::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 5) && is<int>(parameters[0]) && is<int>(parameters[1]) && is<int>(parameters[2]);
}

bool CommandCutToPieces::execute(const std::vector<std::string>& parameters) {
    tgt::ivec3 start, dimensions;
    dimensions.x = cast<int>(parameters[0]);
    dimensions.y = cast<int>(parameters[1]);
    dimensions.z = cast<int>(parameters[2]);

    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[3]);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    int cx, cy, cz; // number of pieces in each dimension
    cx = sourceDataset_->getDimensions().x / dimensions.x;
    cy = sourceDataset_->getDimensions().y / dimensions.y;
    cz = sourceDataset_->getDimensions().z / dimensions.z;
    if ((sourceDataset_->getDimensions().x % dimensions.x) != 0)
        ++cx;
    if ((sourceDataset_->getDimensions().y % dimensions.y) != 0)
        ++cy;
    if ((sourceDataset_->getDimensions().z % dimensions.z) != 0)
        ++cz;
    LINFO("Cutting input dataset with dimensions: " << sourceDataset_->getDimensions());
    LINFO("Into " << cx << " * " << cy << " * " << cz << " = " << (cx * cy * cz) << " pieces with dimensions: " << dimensions);

    char string[500];
    for (int x = 0; x < cx; x++) {
        for (int y = 0; y < cy; y++) {
            for (int z = 0; z < cz; z++) {
                tgt::ivec3 dim = dimensions;
                start.x = dimensions.x * x;
                start.y = dimensions.y * y;
                start.z = dimensions.z * z;

                if (start.x + dimensions.x > sourceDataset_->getDimensions().x)
                    dim.x = (sourceDataset_->getDimensions().x - start.x);
                if (start.y + dimensions.y > sourceDataset_->getDimensions().y)
                    dim.y = (sourceDataset_->getDimensions().y - start.y);
                if (start.z + dimensions.z > sourceDataset_->getDimensions().z)
                    dim.z = (sourceDataset_->getDimensions().z - start.z);

                Volume* targetDataset_ = sourceDataset_->createSubset(start, dim);
                sprintf( string, "%s-%i-%i-%i", parameters.back().c_str(), x, y, z);

                serializer->save(string, targetDataset_);
                //targetDataset_->save(string);
                delete targetDataset_;
            }
        }
    }
    delete serializer;
    return true;
}

//-----------------------------------------------------------------------------

CommandScale::CommandScale() :
    Command("--scale", "", "Rescale Datasets\n \
                           Downsample dataset to target dimensions dx dy dz\n \
                           FILTER:\n \
                           n|nearest: nearest filtering\n \
                           l|linear: linear filtering",
                           "<FILTER DX DY DZ IN OUT>", 6)
{
    loggerCat_ += "." + name_;
}

bool CommandScale::checkParameters(const std::vector<std::string>& parameters) {
    std::set<std::string> set;
    set.insert("n");
    set.insert("nearest");
    set.insert("l");
    set.insert("linear");
    return (parameters.size() == 6) && isValueInSet(parameters[0], &set) && is<int>(parameters[1]) && is<int>(parameters[2]) && is<int>(parameters[3]);
}

bool CommandScale::execute(const std::vector<std::string>& parameters) {
    Volume::Filter filter = Volume::LINEAR;

    if ((parameters[0] == "n") || (parameters[0] == "nearest")) {
        filter = Volume::NEAREST;
        LINFO("using nearest filtering");
    }
    else if ((parameters[0] == "l") || (parameters[0] == "linear")) {
        filter = Volume::LINEAR;
        LINFO("using linear filtering");
    }

    tgt::ivec3 dimensions;
    dimensions.x = cast<int>(parameters[1]);
    dimensions.y = cast<int>(parameters[2]);
    dimensions.z = cast<int>(parameters[3]);

    if ( hor(lessThan(dimensions, tgt::ivec3(1))) )
        throw tgt::Exception("target dimensions must be greater equal one");

    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[4]);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    Volume* targetDataset_ = sourceDataset_->scale(dimensions, filter);
    serializer->save(parameters.back(), targetDataset_);
    delete sourceDataset_;
    delete targetDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandMirrorZ::CommandMirrorZ() :
    Command("--mirrorz", "", "Mirror volume on z axis",
        "<IN OUT>", 2)
{
    loggerCat_ += "." + name_;
}

bool CommandMirrorZ::execute(const std::vector<std::string>& parameters) {
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[0]);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    Volume* targetDataset_ = sourceDataset_->mirrorZ();
    serializer->save(parameters.back(), targetDataset_);
    delete targetDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandSubSet::CommandSubSet() :
    Command("--subset", "", "Write subset of input volume starting at (X,Y,Z) with dimensions (DX,DY,DZ)",
        "<X Y Z DX DY DZ IN OUT>", 8)
{
    loggerCat_ += "." + name_;
}

bool CommandSubSet::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 8) && is<int>(parameters[0]) && is<int>(parameters[1]) && is<int>(parameters[2]) &&
        is<int>(parameters[3]) && is<int>(parameters[4]) && is<int>(parameters[5]);
}

bool CommandSubSet::execute(const std::vector<std::string>& parameters) {
    tgt::ivec3 start, dimensions;
    start.x = cast<int>(parameters[0]);
    start.y = cast<int>(parameters[1]);
    start.z = cast<int>(parameters[2]);
    dimensions.x = cast<int>(parameters[3]);
    dimensions.y = cast<int>(parameters[4]);
    dimensions.z = cast<int>(parameters[5]);

    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[6]);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    Volume* targetDataset_ = sourceDataset_->createSubset(start, dimensions);

    targetDataset_->setSpacing(sourceDataset_->getSpacing());
    serializer->save(parameters.back(), targetDataset_);
    delete targetDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandBrick::CommandBrick() :
    Command("--brick", "", "Bricks the volume into bricks of size BRICKSIZE^3 and writes them into a single file.",
        "<BRICKSIZE IN OUT>",3)
{
    loggerCat_ += "." + name_;
}

bool CommandBrick::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 3);
}
    
bool CommandBrick::execute(const std::vector<std::string>& parameters) {

	int bricksize = cast<int>(parameters[0]);

	VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

	BrickingInformation brickingInformation;

	brickingInformation.brickSize = bricksize;
    brickingInformation.totalNumberOfResolutions = static_cast<int> ( ( log( 
            (float)bricksize) / log (2.0) ) + 1);

	VolumeSet* volumeSet;
	Volume* volume;
    bool readSliceWise = true;
	try {
		volumeSet = serializer->loadSlices(parameters[1],0,bricksize);
	} catch (std::exception e) {
        readSliceWise = false;
    }
    if (readSliceWise == false) {
        try {
            volumeSet = serializer->loadBrick(parameters[1],tgt::ivec3(0),bricksize);
        } catch (std::exception e) {
            LERROR(e.what() << "\nCouldn't read brick-wise or slice-wise.");
            return false;
        }
    }

	volume = volumeSet->getFirstVolume();
	getVolumeInformation(brickingInformation,volume);
	delete volume;
	volume = 0;

	BrickedVolumeWriter* brickedVolumeWriter = new BrickedVolumeWriter(brickingInformation);
	brickedVolumeWriter->openFile(parameters[2]);

	//The volume might not fit into memory, therefore we only read enough slices to
	//create some bricks, write those into the file and then delete them from memory.
	//Then we read the next slices, create bricks, and so on.
    if (readSliceWise == true) {
	    for (int i=0; i<brickingInformation.originalVolumeDimensions.z; i=i+bricksize) {
		    VolumeSet* volumeSet = 0;
    	
		    try {
			    volumeSet = serializer->loadSlices(parameters[1],i,i+bricksize);
		    } catch (std::exception e) {
			    LERROR(e.what());
            }
		    Volume* volume = volumeSet->getFirstVolume();
    		
		    for (int j=0; j < brickingInformation.numBricks.y; j++) {
			    for (int k=0; k < brickingInformation.numBricks.x; k++) {
				    int xpos,ypos,zpos;
				    xpos=k*bricksize;
				    ypos=j*bricksize;
				    zpos=0;

				    Volume* subset = volume->createSubset(tgt::ivec3(xpos,ypos,zpos),tgt::ivec3(bricksize) );
				    brickedVolumeWriter->writeVolume(subset);
				    delete subset;
			    }
		    }
		    std::stringstream msg;
            msg << i+bricksize << " of " << brickingInformation.originalVolumeDimensions.z << " slices done.";
		    LINFO(msg.str());
		    delete volume;
	    }
    } else {
        //We don't even have enough RAM to read enough slices, so we read single bricks.
        //This is very very very slow, but there is nothing we can do if the volumes are just 
        //too big. 
        for (int i=0; i<brickingInformation.numBricks.z; i++) {
            for (int j=0; j < brickingInformation.numBricks.y; j++) {
			    for (int k=0; k < brickingInformation.numBricks.x; k++) {
		            VolumeSet* volumeSet = 0;

		            try {
                        volumeSet = serializer->loadBrick(parameters[1],tgt::ivec3(k,j,i), bricksize);
		            } catch (std::exception e) {
			            LERROR(e.what());
                    }
		            Volume* volume = volumeSet->getFirstVolume();
                    brickedVolumeWriter->writeVolume(volume);
                    delete volume;
                }
            }
        }
    }

	brickedVolumeWriter->writeBviFile();
	brickedVolumeWriter->closeFile();
    return true;
}

void CommandBrick::getVolumeInformation(BrickingInformation &brickingInformation, Volume* volume) {

	if (dynamic_cast<VolumeUInt8*>(volume)) {
		brickingInformation.originalVolumeFormat = "UCHAR";
		brickingInformation.originalVolumeModel = "I";
	} else if (dynamic_cast<VolumeUInt16*>(volume)) {
		brickingInformation.originalVolumeFormat = "USHORT";
		brickingInformation.originalVolumeModel = "I";
	} else if (dynamic_cast<VolumeFloat*>(volume)) {
		brickingInformation.originalVolumeFormat = "FLOAT";
		brickingInformation.originalVolumeModel = "I";
	} else if (dynamic_cast<Volume4xUInt8*>(volume)) {
		brickingInformation.originalVolumeFormat = "UCHAR";
		brickingInformation.originalVolumeModel = "RGBA";
	} else if (dynamic_cast<Volume4xUInt16*>(volume)) {
		brickingInformation.originalVolumeFormat = "USHORT";
		brickingInformation.originalVolumeModel = "RGBA";
	} else if (dynamic_cast<Volume3xUInt8*>(volume)) {
		brickingInformation.originalVolumeFormat = "UCHAR";
		brickingInformation.originalVolumeModel = "RGB";
	} else if (dynamic_cast<Volume3xUInt16*>(volume)) {
		brickingInformation.originalVolumeFormat = "USHORT";
		brickingInformation.originalVolumeModel = "RGB";
	}
	brickingInformation.originalVolumeDimensions = volume->meta().getParentVolumeDimensions();
	brickingInformation.originalVolumeSpacing = volume->getSpacing();
	brickingInformation.originalVolumeBitsStored = volume->getBitsStored();
	brickingInformation.originalVolumeBytesAllocated = volume->getBitsAllocated()/8;

	tgt::ivec3 numbricks;
	numbricks.x = static_cast<int>( 
		ceil( (float)brickingInformation.originalVolumeDimensions.x / (float)brickingInformation.brickSize));
	numbricks.y = static_cast<int>( 
		ceil( (float)brickingInformation.originalVolumeDimensions.y / (float)brickingInformation.brickSize));
	numbricks.z = static_cast<int>( 
		ceil( (float)brickingInformation.originalVolumeDimensions.z / (float)brickingInformation.brickSize));

    brickingInformation.totalNumberOfBricksNeeded = numbricks.x * numbricks.y * numbricks.z;

	brickingInformation.numBricks = numbricks;


    tgt::vec3 cubeSize = tgt::vec3(numbricks*brickingInformation.brickSize) *
								brickingInformation.originalVolumeSpacing;

    cubeSize = cubeSize * 2.f / max(cubeSize);
	tgt::vec3 urb = cubeSize / 2.f;
    tgt::vec3 llf = -urb;

	brickingInformation.originalVolumeLLF = llf;
	brickingInformation.originalVolumeURB = urb;
    brickingInformation.numberOfBricksWithEmptyVolumes = 0;
}

//-----------------------------------------------------------------------------

CommandScaleTexCoords::CommandScaleTexCoords() :
	Command("--scaleTC", "", "Sample dataset to target dimensions dx dy dz\n", "<DX DY DZ IN OUT>", 5)
{
    loggerCat_ += "." + name_;
}
    
bool CommandScaleTexCoords::execute(const std::vector<std::string>& parameters) {
    tgt::ivec3 newDims;
    newDims.x = cast<int>(parameters[0]);
    newDims.y = cast<int>(parameters[1]);
    newDims.z = cast<int>(parameters[2]);

    if( hor(lessThan(newDims, tgt::ivec3(1))) )
        throw tgt::Exception("target dimensions must be greater equal one");
    
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[3]);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();
    
    Volume4xUInt8* source = dynamic_cast<Volume4xUInt8*>(sourceDataset_);
    
    LINFO("resampling with sampledimensions from " << source->getDimensions() << " to " << newDims);

    // build target volume
    Volume4xUInt8* v;
    try {
         v = new Volume4xUInt8(newDims, source->getSpacing());
    }
    catch (std::bad_alloc) {
        throw; // throw it to the caller
    }

    tgt::vec3 ratio = tgt::vec3(source->getDimensions()) / tgt::vec3(newDims);
    tgt::vec3 invDims = 1.f / tgt::vec3(source->getDimensions());

    tgt::ivec3 pos = tgt::ivec3::zero; // iteration variable
    tgt::vec3 nearest; // knows the new position of the target volume

        for (pos.z = 0; pos.z < newDims.z; ++pos.z) {
            nearest.z = static_cast<float>(pos.z) * ratio.z;

            for (pos.y = 0; pos.y < newDims.y; ++pos.y) {
                nearest.y = static_cast<float>(pos.y) * ratio.y;

                for (pos.x = 0; pos.x < newDims.x; ++pos.x) {
                    nearest.x = static_cast<float>(pos.x) * ratio.x;
                    tgt::vec3 p = nearest - floor(nearest); // get decimal part
                    tgt::ivec3 llb = tgt::ivec3(nearest);
                    tgt::ivec3 urf = tgt::ivec3(ceil(nearest));
                    urf = tgt::min(urf, source->getDimensions() - 1); // clamp so the lookups do not exceed the dimensions

                    //calculate target segment:
                    double segments[256];
                    for(int k=0; k<256; ++k)
                        segments[k] = 0.0f;

                    segments[source->voxel(llb.x, llb.y, llb.z).a] += ((1.f-p.x)*(1.f-p.y)*(1.f-p.z));  // llB
                    segments[source->voxel(urf.x, llb.y, llb.z).a] += ((    p.x)*(1.f-p.y)*(1.f-p.z));  // lrB
                    segments[source->voxel(urf.x, urf.y, llb.z).a] += ((    p.x)*(    p.y)*(1.f-p.z));  // urB
                    segments[source->voxel(llb.x, urf.y, llb.z).a] += ((1.f-p.x)*(    p.y)*(1.f-p.z));  // ulB
                    segments[source->voxel(llb.x, llb.y, urf.z).a] += ((1.f-p.x)*(1.f-p.y)*(    p.z));  // llF
                    segments[source->voxel(urf.x, llb.y, urf.z).a] += ((    p.x)*(1.f-p.y)*(    p.z));  // lrF
                    segments[source->voxel(urf.x, urf.y, urf.z).a] += ((    p.x)*(    p.y)*(    p.z));  // urF
                    segments[source->voxel(llb.x, urf.y, urf.z).a] += ((1.f-p.x)*(    p.y)*(    p.z));  // ulF

                    if(segments[0] < 1.0f) {
                        int seg = 1;
                        for(int k=0; k<256; ++k)
                            if(segments[k] > segments[seg])
                                seg = k;

                        tgt::dvec3 res = tgt::dvec3(.0f,.0f,.0f);
                        if(source->voxel(llb.x, llb.y, llb.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(llb.x, llb.y, llb.z).x, (double)source->voxel(llb.x, llb.y, llb.z).y, (double)source->voxel(llb.x, llb.y, llb.z).z);
                            res += value * (double)((1.f-p.x)*(1.f-p.y)*(1.f-p.z));  // llB
                        }
                        if(source->voxel(urf.x, llb.y, llb.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(urf.x, llb.y, llb.z).x, (double)source->voxel(urf.x, llb.y, llb.z).y, (double)source->voxel(urf.x, llb.y, llb.z).z);
                            res += value * (double)((    p.x)*(1.f-p.y)*(1.f-p.z));  // lrB
                        }
                        if(source->voxel(urf.x, urf.y, llb.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(urf.x, urf.y, llb.z).x, (double)source->voxel(urf.x, urf.y, llb.z).y, (double)source->voxel(urf.x, urf.y, llb.z).z);
                            res += value * double((    p.x)*(    p.y)*(1.f-p.z));  // urB
                        }
                        if(source->voxel(llb.x, urf.y, llb.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(llb.x, urf.y, llb.z).x, (double)source->voxel(llb.x, urf.y, llb.z).y, (double)source->voxel(llb.x, urf.y, llb.z).z);
                            res += value * double((1.f-p.x)*(    p.y)*(1.f-p.z));  // ulB
                        }
                        if(source->voxel(llb.x, llb.y, urf.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(llb.x, llb.y, urf.z).x, (double)source->voxel(llb.x, llb.y, urf.z).y, (double)source->voxel(llb.x, llb.y, urf.z).z);
                            res += value * double((1.f-p.x)*(1.f-p.y)*(    p.z));  // llF
                        }
                        if(source->voxel(urf.x, llb.y, urf.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(urf.x, llb.y, urf.z).x, (double)source->voxel(urf.x, llb.y, urf.z).y, (double)source->voxel(urf.x, llb.y, urf.z).z);
                            res += value * double((    p.x)*(1.f-p.y)*(    p.z));  // lrF
                        }
                        if(source->voxel(urf.x, urf.y, urf.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(urf.x, urf.y, urf.z).x, (double)source->voxel(urf.x, urf.y, urf.z).y, (double)source->voxel(urf.x, urf.y, urf.z).z);
                            res += value * double((    p.x)*(    p.y)*(    p.z));  // urF
                        }
                        if(source->voxel(llb.x, urf.y, urf.z).a == seg) {
                            tgt::dvec3 value((double)source->voxel(llb.x, urf.y, urf.z).x, (double)source->voxel(llb.x, urf.y, urf.z).y, (double)source->voxel(llb.x, urf.y, urf.z).z);
                            res += value * double((1.f-p.x)*(    p.y)*(    p.z));// ulF
                        }

                        res /= segments[seg];
                        v->voxel(pos).x = static_cast<uint8_t>(res.x);
                        v->voxel(pos).y = static_cast<uint8_t>(res.y);
                        v->voxel(pos).z = static_cast<uint8_t>(res.z);
                        v->voxel(pos).w = static_cast<uint8_t>(seg);
                    }
                    else {
                        v->voxel(pos).x = 0;
                        v->voxel(pos).y = 0;
                        v->voxel(pos).z = 0;
                        v->voxel(pos).w = 0;
                    }
                }
            }
        }

    serializer->save(parameters.back(), v);
    delete sourceDataset_;
    delete v;
    return true;
}

}   //namespace voreen
