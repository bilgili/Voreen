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

#include "voreen/core/vis/processors/volume/visiblehumandatasetcreator.h"
#include "voreen/core/io/volumeserializer.h"

#ifndef VRN_VOLUMESERIALIZERPOPULATOR_H
#include "voreen/core/io/volumeserializerpopulator.h"
#endif


#include <sstream>
#include <fstream>
#include <iostream>

namespace voreen {

VisibleHumanDatasetCreator::VisibleHumanDatasetCreator()
    : imageMatrixSizeXProp_("image.matrix.size.x.changed","Image Matrix Size X",512,0,4096),
    imageMatrixSizeYProp_("image.matrix.size.y.changed","Image Matrix Size Y",512,0,4096),
    thicknessXProp_("thickness.x.changed","Slice Thickness X",1.0f,0.0f,20.0f),
    thicknessYProp_("thickness.y.changed","Slice Thickness Y",1.0f,0.0f,20.0f),
    thicknessZProp_("thickness.z.changed","Slice Thickness Z",1.0f,0.0f,20.0f),
    bitsStoredProp_("bits.stored.changed","BitsStored",12,0,256),
    headerSizeProp_("header.size.changed","Header Size",0,0,20000),
    readInfosFromHeaderProp_("read.infos.from.header.changed","Read infos from header",false),
    datasetNameProp_("dataset.name.changed","Dataset filename","Dataset filename", "", "*.*"),
    cropBottomProp_("crop.bottom.changed","Crop Bottom",0,0,1216),
    cropTopProp_("crop.top.changed","Crop Top",0,0,1216),
    cropLeftProp_("crop.left.changed","Crop Left",0,0,2048),
    cropRightProp_("crop.right.changed","Crop Right",0,0,2048),
    outport_(Port::OUTPORT, "volumeset.newdataset", true)
{
    addPort(outport_);

    datasetReady_=false;

    std::vector<std::string> formats;
    formats.push_back("UCHAR");
    formats.push_back("USHORT");
    std::vector<std::string> models;
    models.push_back("I");
    models.push_back("RGB");

//    objectModelProp_ = new EnumProperty("object.model.changed","Object Model",models,0);
//    formatProp_ = new EnumProperty("format.changed","Data format",formats,1);
    std::vector<std::string> emptyVector;
/*    sliceNamesProp_ = new StringVectorProperty("slice.names.changed","Slices",emptyVector);
    headerNamesProp_ = new StringVectorProperty("header.names.changed","Headers",emptyVector); */

    addProperty(datasetNameProp_);
//    addProperty(sliceNamesProp_);
//    addProperty(headerNamesProp_);
    addProperty(imageMatrixSizeXProp_);
    addProperty(imageMatrixSizeYProp_);
    addProperty(thicknessXProp_);
    addProperty(thicknessYProp_);
    addProperty(thicknessZProp_);
//    addProperty(formatProp_);
//    addProperty(objectModelProp_);
    addProperty(bitsStoredProp_);
    addProperty(headerSizeProp_);
    addProperty(readInfosFromHeaderProp_);
    addProperty(cropTopProp_);
    addProperty(cropBottomProp_);
    addProperty(cropLeftProp_);
    addProperty(cropRightProp_);

    sliceThicknessX_ = 1.0f;
    sliceThicknessY_ = 1.0f;
    sliceThicknessZ_ = 1.0f;
    format_ = "USHORT";
    objectModel_ = "I";
    bitsStored_ = 12;
    headerSize_ = 0;
    readInfosFromHeader_ = false;
    imageDimensionX_ = 512;
    imageDimensionY_= 512;
    imageMatrixSizeX_ = 512;
    imageMatrixSizeY_ = 512;
    cropLeft_ = 0;
    cropRight_= 0;
    cropTop_ = 0;
    cropBottom_ =0;
}

const std::string VisibleHumanDatasetCreator::getProcessorInfo() const {
    return "Creates visible human data set";
}

void VisibleHumanDatasetCreator::createDataset() {
    if (objectModel_ == "I") {
        vishumReader_->createDataset(sliceNames_,datasetName_,headerSize_,false);
    }
    else  {
        vishumReader_->createDataset(sliceNames_,datasetName_,headerSize_,true);
    }

    std::string::size_type pos = datasetName_.find(".raw");
    if (pos != std::string::npos) {
        datFileName_ = datasetName_.substr(0,pos);
        datFileName_ = datFileName_ + ".dat";

        std::fstream outputStream(datFileName_.c_str(), std::ios::out);
        outputStream << "ObjectFileName: " << datasetName_ << std::endl;
        outputStream << "Resolution: " << imageMatrixSizeX_ << " " << imageMatrixSizeY_ << " " << sliceNames_.size() << std::endl;
        outputStream << "SliceThickness: " << sliceThicknessX_ << " "  << sliceThicknessY_ << " " << sliceThicknessZ_ << std::endl;
        outputStream << "ObjectModel: " << objectModel_ << std::endl;
        outputStream << "Format: " << format_ << std::endl;
        outputStream << "BitsStored: " << bitsStored_ << std::endl;

        outputStream.close();

        datasetReady_=true;

        //VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
        //const VolumeSerializer* serializer = populator->getVolumeSerializer();

        //VolumeCollection* volumeCollection = serializer->load(datFileName_);

        //VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(getOutports().at(0)->getConnected().at(0)->getProcessor());
        /*if (vsp != 0)
            vsp->setVolumeSet(volumeset); */
    }
}

void VisibleHumanDatasetCreator::readHeaderInfos() {
    for (size_t i=0; i< headerNames_.size(); i++) {
        std::fstream in(headerNames_.at(i).c_str(), std::ios::in);

        std::string readString;
        std::string::size_type found;

        getline(in,readString);
        bool lineFinished = false;

        while ( in ) {
            lineFinished = false;
            found=readString.find("Slice Thickness (mm)");
            if (found != std::string::npos) {
                found = readString.find_last_of(" ");
                if (found != std::string::npos) {
                    std::istringstream isst(readString.substr(found));;
                    isst >> sliceThicknessZ_;
                    lineFinished=true;
                }
            }
            if (!lineFinished) {
                found=readString.find("Image pixel size - X");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> sliceThicknessX_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("Image pixel size - Y");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> sliceThicknessY_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("Image dimension - X");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> imageDimensionX_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("Image dimension - Y");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> imageDimensionY_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("Image matrix size - X");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> imageMatrixSizeX_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("Image matrix size - Y");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> imageMatrixSizeY_;
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("depth");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> format_;
                        if (format_ =="16") {
                            format_ ="USHORT";
                        } else if (format_ == "8") {
                            format_ ="UCHAR";
                        }
                        lineFinished=true;
                    }
                }
            }
            if (!lineFinished) {
                found=readString.find("length.......");
                if (found != std::string::npos) {
                    found = readString.find_last_of(" ");
                    if (found != std::string::npos) {
                        std::istringstream isst(readString.substr(found));;
                        isst >> headerSize_;
                        lineFinished=true;
                    }
                }
            }

            getline(in,readString);   // Try to get another line.
        }

        in.close();
        /*
        if ( (sliceThicknessZ_ != thickZ) || (sliceThicknessX_ != thickX) || (imageDimensionX_ != imageDemX) || (imageDimensionY_ != imageDemY) ) {
            thickZ = sliceThicknessZ_;
            thickX = sliceThicknessX_;
            imageDemX = imageDimensionX_;
            imageDemY = imageDimensionY_;
            sliceNumbers.push_back(headerNames_.at(i).toStdString());
            std::cout << "Format changed in slice:  " << headerNames_.at(i).toStdString() << " ThicknessX: " << sliceThicknessX_ << " ThicknessZ: " << sliceThicknessZ_<< std::endl;
            std::cout << "Image DimensionX: " << imageDimensionX_ << "Image DimensionY: " << imageDimensionY_<< std::endl;
        }*/
    }

}

void VisibleHumanDatasetCreator::process() {
    if (!datasetReady_) {
        if (readInfosFromHeader_) {
            readHeaderInfos();
        }
        createDataset();
    }
}

} //namespace voreen
