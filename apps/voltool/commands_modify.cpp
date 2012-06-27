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

#include "commands_modify.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/volume/volumeatomic.h"
#include "tgt/vector.h"
 
namespace voreen {

CommandCutToPieces::CommandCutToPieces() {
    help_ = "Devide dataset into pieces with dimensions (DX, DY, DZ)\n";
    
    info_ = "Devide dataset into pieces.";
    name_ = "cuttopieces";
    syntax_ = name_ + "DX DY DZ IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandCutToPieces::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 5);
    
    tgt::ivec3 start, dimensions;
    dimensions.x = asInt(parameters[0]);
    dimensions.y = asInt(parameters[1]);
    dimensions.z = asInt(parameters[2]);
    
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    
    VolumeSet* volumeSet = serializer->load(parameters[3], false);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    int cx, cy, cz; // number of pieces in each dimension
    cx = sourceDataset_->getDimensions().x / dimensions.x;
    cy = sourceDataset_->getDimensions().y / dimensions.y;
    cz = sourceDataset_->getDimensions().z / dimensions.z;
    if((sourceDataset_->getDimensions().x % dimensions.x) != 0) ++cx;
    if((sourceDataset_->getDimensions().y % dimensions.y) != 0) ++cy;
    if((sourceDataset_->getDimensions().z % dimensions.z) != 0) ++cz;
    LINFO("Cutting input dataset with dimensions: " << sourceDataset_->getDimensions());
    LINFO("Into " << cx << " * " << cy << " * " << cz << " = " << (cx * cy * cz) << " pieces with dimensions: " << dimensions);

    char string[500];
    for(int x = 0; x < cx; x++) {
        for(int y = 0; y < cy; y++) {
            for(int z = 0; z < cz; z++) {
                tgt::ivec3 dim = dimensions;
                start.x = dimensions.x * x;
                start.y = dimensions.y * y;
                start.z = dimensions.z * z;
                if(start.x + dimensions.x > sourceDataset_->getDimensions().x) dim.x = (sourceDataset_->getDimensions().x - start.x);
                if(start.y + dimensions.y > sourceDataset_->getDimensions().y) dim.y = (sourceDataset_->getDimensions().y - start.y);
                if(start.z + dimensions.z > sourceDataset_->getDimensions().z) dim.z = (sourceDataset_->getDimensions().z - start.z);
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

CommandScale::CommandScale() {
    help_ =  "Downsample dataset to target dimensions dx dy dz\n";
    help_ += "FILTER:\n";
    help_ += "n|nearest: nearest filtering\n";
    help_ += "l|linear: linear filtering\n";
    info_ = "Rescale Datasets\n";
    name_ = "scale";
    syntax_ = name_ + " FILTER DX DY DZ IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandScale::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 6);

    Volume::Filter filter;

    if (parameters[0] == "n") {
        filter = Volume::NEAREST;
        LINFO("using nearest filtering");
    }
    else if (parameters[0] == "nearest") {
        filter = Volume::NEAREST;
        LINFO("using nearest filtering");
    }
    else if (parameters[0] == "l") {
        filter = Volume::LINEAR;
        LINFO("using linear filtering");
    }
    else if (parameters[0] == "linear") {
        filter = Volume::LINEAR;
        LINFO("using linear filtering");
    }
    else
        throw SyntaxException("Unknown filter!");

    tgt::ivec3 dimensions;
    dimensions.x = asInt(parameters[1]);
    dimensions.y = asInt(parameters[2]);
    dimensions.z = asInt(parameters[3]);

    if( hor(lessThan(dimensions, tgt::ivec3(1))) )
        throw SyntaxException("target dimensions must be greater equal one");
    
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[4], false);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    Volume* targetDataset_ = sourceDataset_->scale(dimensions, filter);
    serializer->save(parameters.back(), targetDataset_);
    delete sourceDataset_;
    delete targetDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandMirrorZ::CommandMirrorZ() {
    help_ =  "\n";
    info_ = "Mirror volume on z axis";
    name_ = "mirrorz";
    syntax_ = name_ + " IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandMirrorZ::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 2);
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();

    VolumeSet* volumeSet = serializer->load(parameters[0], false);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();
    
    Volume* targetDataset_ = sourceDataset_->mirrorZ();
    serializer->save(parameters.back(), targetDataset_);
    delete targetDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandSubSet::CommandSubSet() {
    help_ =  "Write subset of input volume starting at (X,Y,Z) with dimensions (DX,DY,DZ)\n";
    info_ = "Write subset of volume";
    name_ = "subset";
    syntax_ = name_ + " X Y Z DX DY DZ IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandSubSet::execute(const std::vector<std::string>& parameters) {
    checkParameters(parameters.size() == 8);

    tgt::ivec3 start, dimensions;
    start.x = asInt(parameters[0]);
    start.y = asInt(parameters[1]);
    start.z = asInt(parameters[2]);
    dimensions.x = asInt(parameters[3]);
    dimensions.y = asInt(parameters[4]);
    dimensions.z = asInt(parameters[5]);
    
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    
    VolumeSet* volumeSet = serializer->load(parameters[6], false);
    Volume* sourceDataset_ = volumeSet->getFirstVolume();

    Volume* targetDataset_ = sourceDataset_->createSubset(start, dimensions);
    
    targetDataset_->setSpacing(sourceDataset_->getSpacing());
    serializer->save(parameters.back(), targetDataset_);
    delete targetDataset_;
    return true;
}


}   //namespace voreen
