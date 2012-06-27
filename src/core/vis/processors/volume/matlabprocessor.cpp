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

#include "voreen/core/vis/processors/volume/matlabprocessor.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

const std::string MatlabProcessor::loggerCat_("voreen.MatlabProcessor");

MatlabProcessor::MatlabProcessor()
    : VolumeProcessor(),
    inputVolumeHandle_(0),
    processedVolumeHandle_(0),
    enableProcessingProp_("enableProcessing", "enable: ", true),
    //forceUpdate_(false),
    forceUpdate_(true),
    matlabEngine_(0),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0)
{
    setName("MatlabProcessor");
    addPort(inport_);
    addPort(outport_);

    enableProcessingProp_.onChange(
        CallMemberAction<MatlabProcessor>(this, &MatlabProcessor::forceUpdate));
    addProperty(enableProcessingProp_);
}

MatlabProcessor::~MatlabProcessor() {
    if(matlabEngine_)
        if(engClose(matlabEngine_) == 0)
            LINFO("Closed matlab engine.");
        else
            LERROR("Failed to close matlab engine!");
    delete processedVolumeHandle_;
}

const std::string MatlabProcessor::getClassName() const {
    return std::string("VolumeProcessor.MatlabProcessor");
}

const std::string MatlabProcessor::getProcessorInfo() const {
    return std::string("Matlab volume preprocessor");
}

void MatlabProcessor::initialize() throw (VoreenException) {
    if(!matlabEngine_) {
        matlabEngine_ = engOpen("\0");
        if(!matlabEngine_) {
            LERROR("Can't start MATLAB engine!");
            throw VoreenException(getClassName().getName() + ": Can't start MATLAB engine!");
        }
        else {
            LINFO("Started MATLAB engine!");
        }
    }
    initialized_ = true;
}

void MatlabProcessor::process() {
    bool handleChanged = false;
    //bool res = VolumeHandleValidator::checkVolumeHandle(,
        //getVolumeHandle(inportName_), &handleChanged);
    inputVolumeHandle_ = inport_.getData();
    // if the processor is disable, "short-circuit" the in- and outport
    //
    if (enableProcessingProp_.get() == false) {
        outport_.setData(inputVolumeHandle_);
        return;
    }

    if (inputVolumeHandle_ && ((forceUpdate_ == true) || (handleChanged == true)))
        processMatlab();
}

// private methods
//

void MatlabProcessor::forceUpdate() {
    forceUpdate_ = true;
}

void MatlabProcessor::processMatlab() {
    forceUpdate_ = false;

    if (inputVolumeHandle_ == 0)
        return;

    writeMatrix(inputVolumeHandle_->getVolume(), "input");

    engEvalString(matlabEngine_, "output = testfkt(input);\n");
    //engEvalString(matlabEngine_, "[output,gy,gz] = gradient(input);\n");

    Volume* v = readMatrix("output");
    if(!v) {
        outport_.setData(inputVolumeHandle_);
    }
    else  {
        delete processedVolumeHandle_;
        processedVolumeHandle_ = new VolumeHandle(v, 0.0f);
        outport_.setData(processedVolumeHandle_);
    }
}

void MatlabProcessor::writeMatrix(Volume* dataset, std::string name) {
    mxArray* pa = 0;

    mwSize dim[4];
    dim[0] = dataset->getDimensions().x;
    dim[1] = dataset->getDimensions().y;
    dim[2] = dataset->getDimensions().z;
    dim[3] = 1;

    //pa = mxCreateNumericArray(3, (mwSize *)dim, mxUINT8_CLASS, mxREAL);

    if(dynamic_cast<VolumeAtomic<float>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxSINGLE_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<float>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<double>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxDOUBLE_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<double>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<int8_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxINT8_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<int8_t>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<uint8_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxUINT8_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<uint8_t>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<int16_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxINT16_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<int16_t>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<uint16_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxUINT16_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<uint16_t>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<int32_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxINT32_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<int32_t>* >(dataset), 0);
    }
    else if(dynamic_cast<VolumeAtomic<uint32_t>* >(dataset)) {
        pa = mxCreateNumericArray(3, (mwSize *)dim, mxUINT32_CLASS, mxREAL);
        writeMatrix(pa, dynamic_cast<VolumeAtomic<uint32_t>* >(dataset), 0);
    }
    else {
        LERROR("Unhandled data set type");
    }

    //Place the variable into the MATLAB workspace
    engPutVariable(matlabEngine_, name.c_str(), pa);
    //mxDestroyArray(input);
}

Volume* MatlabProcessor::readMatrix(std::string name) {
    mxArray* pa = engGetVariable(matlabEngine_, name.c_str());
    if (!pa) {
        LERROR("Failed to get Variable output from matlab engine.");
        return 0;
    }
    LINFO("output is class " << mxGetClassName(pa));

    int dims = mxGetNumberOfDimensions(pa);
    const mwSize* matDim = mxGetDimensions(pa);
    tgt::ivec3 dim(matDim[0], matDim[1], matDim[2]);
    int wMax;
    switch (dims) {
        case 3:
            wMax = 1;
            LINFO("  Matrix with 3 dimensions found.");
            break;
        case 4:
            wMax = matDim[3];
            LINFO("  Matrix with 4 dimensions found.");
            break;
        default:
            LERROR("  Matrix has " << dims << " dimensions. Currently only matrices with 3 or 4 dimensions are supported.");
            return 0;
    }

    //for (int w=0; w<wMax; ++w) {
        int w=0;
        LINFO("  Read and add volume " << w);
        Volume* dataset;
        switch (mxGetClassID(pa)) {
            case mxSINGLE_CLASS:
                dataset = readMatrix<float>(pa, dim, w);
                break;
            case mxDOUBLE_CLASS:
                dataset = readMatrix<double>(pa, dim, w);
                break;
            case mxINT8_CLASS:
                dataset = readMatrix<int8_t>(pa, dim, w);
                break;
            case mxUINT8_CLASS:
                dataset = readMatrix<uint8_t>(pa, dim, w);
                break;
            case mxINT16_CLASS:
                dataset = readMatrix<int16_t>(pa, dim, w);
                break;
            case mxUINT16_CLASS:
                dataset = readMatrix<uint16_t>(pa, dim, w);
                break;
            case mxINT32_CLASS:
                dataset = readMatrix<int32_t>(pa, dim, w);
                break;
            case mxUINT32_CLASS:
                dataset = readMatrix<uint32_t>(pa, dim, w);
                break;
            default:
                LERROR("Unhandled ClassID: " << mxGetClassName(pa));
                dataset = 0;
                break;
        }
        return dataset;
        //TODO: handle 4d data sets
        //VolumeHandle* handle = new VolumeHandle(dataset, static_cast<const float>(w));
        //handle->setOrigin(
            //series->getParentSet()->getName(),
            //name,
            //static_cast<const float>(w));
        //series->addVolumeHandle(handle);
    //}
        mxDestroyArray(pa);
}
}   // namespace
