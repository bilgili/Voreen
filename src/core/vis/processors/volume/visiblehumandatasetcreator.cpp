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

#include "voreen/core/vis/processors/volume/visiblehumandatasetcreator.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/core/io/volumeserializer.h"

#ifndef VRN_VOLUMESELECTIONPROCESSOR_H
#include "voreen/core/vis/processors/volumeselectionprocessor.h"
#endif
#ifndef VRN_VOLUMESERIALIZERPOPULATOR_H
#include "voreen/core/io/volumeserializerpopulator.h"
#endif


#include <sstream>
#include <fstream>
#include <iostream>

namespace voreen {

	VisibleHumanDatasetCreator::VisibleHumanDatasetCreator() {
		createOutport("volumeset.newdataset");
		
		datasetReady_=false;
	
		imageMatrixSizeXProp_ = new IntProp("image.matrix.size.x.changed","Image Matrix Size X",512,0,4096);
		imageMatrixSizeYProp_ = new IntProp("image.matrix.size.y.changed","Image Matrix Size Y",512,0,4096);
		thicknessXProp_ = new FloatProp("thickness.x.changed","Slice Thickness X",1.0f,0.0f,20.0f);
		thicknessYProp_ = new FloatProp("thickness.y.changed","Slice Thickness Y",1.0f,0.0f,20.0f);
		thicknessZProp_ = new FloatProp("thickness.z.changed","Slice Thickness Z",1.0f,0.0f,20.0f);
		std::vector<std::string> formats;
		formats.push_back("UCHAR");
		formats.push_back("USHORT");
		std::vector<std::string> models;
		models.push_back("I");
		models.push_back("RGB");
		objectModelProp_ = new EnumProp("object.model.changed","Object Model",models,0);
		formatProp_ = new EnumProp("format.changed","Data format",formats,1);
		bitsStoredProp_ = new IntProp("bits.stored.changed","BitsStored",12,0,256);
		headerSizeProp_ = new IntProp("header.size.changed","Header Size",0,0,20000);
		readInfosFromHeaderProp_ = new BoolProp("read.infos.from.header.changed","Read infos from header",false);
		datasetNameProp_ = new StringProp("dataset.name.changed","Dataset filename");
		std::vector<std::string> emptyVector;
		sliceNamesProp_ = new StringVectorProp("slice.names.changed","Slices",emptyVector);
		headerNamesProp_ = new StringVectorProp("header.names.changed","Headers",emptyVector);
		cropTopProp_ = new IntProp("crop.top.changed","Crop Top",0,0,1216);
		cropBottomProp_ = new IntProp("crop.bottom.changed","Crop Bottom",0,0,1216);
		cropLeftProp_ = new IntProp("crop.left.changed","Crop Left",0,0,2048);
		cropRightProp_ = new IntProp("crop.right.changed","Crop Right",0,0,2048);

		props_.push_back(datasetNameProp_);
		props_.push_back(sliceNamesProp_);
		props_.push_back(headerNamesProp_);
		props_.push_back(imageMatrixSizeXProp_);
		props_.push_back(imageMatrixSizeYProp_);
		props_.push_back(thicknessXProp_);
		props_.push_back(thicknessYProp_);
		props_.push_back(thicknessZProp_);
		props_.push_back(formatProp_);
		props_.push_back(objectModelProp_);
		props_.push_back(bitsStoredProp_);
		props_.push_back(headerSizeProp_);
		props_.push_back(readInfosFromHeaderProp_);
		props_.push_back(cropTopProp_);
		props_.push_back(cropBottomProp_);
		props_.push_back(cropLeftProp_);
		props_.push_back(cropRightProp_);

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
			
			VolumeSerializerPopulator* populator = new VolumeSerializerPopulator();
			VolumeSerializer* serializer = populator->getVolumeSerializer();
      
			VolumeSet* volumeset = serializer->load(datFileName_, true);
		
			VolumeSelectionProcessor* vsp = dynamic_cast<VolumeSelectionProcessor*>(getOutports().at(0)->getConnected().at(0)->getProcessor());
			if (vsp != 0) {
				vsp->setVolumeSet(volumeset);
			}
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

	void VisibleHumanDatasetCreator::process(LocalPortMapping* /*portMapping*/) {
		if (!datasetReady_) {
			if (readInfosFromHeader_) {
				readHeaderInfos();
			}
			createDataset();
		}
	}

	void VisibleHumanDatasetCreator::processMessage(Message* msg, const Identifier& /*dest*/) {
		if (msg->id_ == Processor::setVolumeContainer_) {
			volumeContainer_ = msg->getValue<VolumeContainer*>();
		}
		else if (msg->id_ == "thickness.x.changed") {
			sliceThicknessX_ = msg->getValue<float>();
		}
		else if (msg->id_ == "thickness.y.changed") {
			sliceThicknessY_ = msg->getValue<float>();
		}
		else if (msg->id_ == "thickness.z.changed") {
			sliceThicknessZ_ = msg->getValue<float>();
		}
		else if (msg->id_ == "object.model.changed") {
			objectModel_ = msg->getValue<std::string>();
		}
		else if (msg->id_ == "format.changed") {
			format_ = msg->getValue<std::string>();
		}
		else if (msg->id_ == "bits.stored.changed") {
			bitsStored_ = msg->getValue<int>();
		}
		else if (msg->id_ == "header.size.changed") {
			headerSize_ = msg->getValue<int>();
		}
		else if (msg->id_ == "image.matrix.size.x.changed") {
			imageMatrixSizeX_ = msg->getValue<int>();
		}
		else if (msg->id_ == "image.matrix.size.y.changed") {
			imageMatrixSizeY_ = msg->getValue<int>();
		}
		else if (msg->id_ == "read.infos.from.header.changed") {
			readInfosFromHeader_ = msg->getValue<bool>();
		}
		else if (msg->id_ == "dataset.name.changed") {
			datasetName_ = msg->getValue<std::string>();
		}
		else if (msg->id_ == "slice.names.changed") {
			sliceNames_ = msg->getValue<std::vector<std::string> >();
		}
		else if (msg->id_ == "header.names.changed") {
			headerNames_ = msg->getValue<std::vector<std::string> >();
		}
	}
} //namespace voreen
