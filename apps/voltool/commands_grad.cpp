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

#include "commands_grad.h"
#include "voreen/core/volume/gradient.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
 
namespace voreen {

CommandGrad::CommandGrad() {
    help_ = "simple: neighborhood of 6\n";
    help_ += "26: neighborhood of 26\n";
    help_ += "sobel: neighborhood of 26 using sobel mask\n";
    help_ += "sobelic: sobel using intensity check\n";
    
    info_ = "Calculate gradients. Writes 32-bit dataset.";
    name_ = "grad";
    syntax_ = name_ + " [simple|26|sobel|sobelic] IN[8|12|16] OUT32";
    loggerCat_ += "." + name_;
}
    
bool CommandGrad::execute(const std::vector<std::string>& parameters) {
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    
    checkParameters(parameters.size() == 3);

    //load volume dataset
    Volume* sourceDataset_;
    Volume* targetDataset_;

    VolumeSet* volumeSet = serializer->load(parameters[1],false);
    sourceDataset_ = volumeSet->getFirstVolume();


    if(parameters[0] == "simple")
        targetDataset_ = calcGradients<tgt::col4>(sourceDataset_);
    else if(parameters[0] == "26")
        targetDataset_ = calcGradients26(sourceDataset_);
    else if(parameters[0] == "sobel")
        targetDataset_ = calcGradientsSobel<tgt::col4>(sourceDataset_, false);
    else if(parameters[0] == "sobelic")
        targetDataset_ = calcGradientsSobel<tgt::col4>(sourceDataset_, true);
    else {
        delete sourceDataset_;
        throw SyntaxException("Unknown filter!");
    }

    if(targetDataset_) {
        VolumeSerializerPopulator volLoadPop;
        VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
        serializer->save(parameters[2], targetDataset_);
        delete serializer;
        delete targetDataset_;
    }
    else {
        LERROR("Failed to calculate target dataset!");
        delete sourceDataset_;
        return false;
    }

    delete sourceDataset_;
    return true;
}

//-----------------------------------------------------------------------------

CommandFilterGrad::CommandFilterGrad() {
    help_ = "";
    
    info_ = "Filter gradients.";
    name_ = "filtergrad";
    syntax_ = name_ + " [simple|mid|weighted TIMES|weightedic TIMES] IN OUT";
    loggerCat_ += "." + name_;
}
    
bool CommandFilterGrad::execute(const std::vector<std::string>& parameters) {
    VolumeSerializerPopulator volLoadPop;
    VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
    
    int numparams = parameters.size();
    
    checkParameters(parameters.size() == 3 || (parameters.size() == 4 && (parameters[0] == "weighted" || parameters[0] == "weightedic")));
    
    //load volume dataset
    Volume* sourceDataset_;
    Volume* targetDataset_;

    VolumeSet* volumeSet = serializer->load(parameters[numparams-2],false);
    sourceDataset_ = volumeSet->getFirstVolume();

    if(parameters[0] == "simple")
        targetDataset_ = filterGradients(sourceDataset_);
    else if(parameters[0] == "mid")
        targetDataset_ = filterGradientsMid(sourceDataset_);
    else if(parameters[0] == "weighted") {
        int times;
        times = asInt(parameters[1]);
        for (int i=0; i<times; ++i) {
            targetDataset_ = filterGradientsWeighted(sourceDataset_, false);
            sourceDataset_ = targetDataset_;
        }
    }
    else if(parameters[0] == "weightedic") {
        int times;
        times = asInt(parameters[1]);
        for (int i=0; i<times; ++i) {
            targetDataset_ = filterGradientsWeighted(sourceDataset_, true);
            sourceDataset_ = targetDataset_;
        }
    }
    else {
        delete sourceDataset_;
        throw SyntaxException("Unknown filter!");
    }

    if(targetDataset_) {
        VolumeSerializerPopulator volLoadPop;
        VolumeSerializer* serializer = volLoadPop.getVolumeSerializer();
        serializer->save(parameters[numparams-1], targetDataset_);
        delete serializer;
        delete targetDataset_;
    }
    else {
        LERROR("Failed to calculate target dataset!");
        delete sourceDataset_;
        return false;
    }

    delete sourceDataset_;
    return true;
}

}   //namespace voreen
