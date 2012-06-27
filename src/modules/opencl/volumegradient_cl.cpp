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

#include "voreen/modules/opencl/volumegradient_cl.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/gradient.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

using namespace cl;

const std::string VolumeGradientCL::loggerCat_("voreen.VolumeGradientCL");

VolumeGradientCL::VolumeGradientCL()
    : VolumeProcessor(),
    technique_("technique", "Technique"),
    copyIntensityChannel_("copyIntensityChannel", "Copy intensity channel", false),
    inport_(Port::INPORT, "volumehandle.input"),
    outport_(Port::OUTPORT, "volumehandle.output", 0),
    opencl_(0),
    context_(0),
    queue_(0),
    prog_(0)
{
    technique_.addOption("central-differences", "Central differences");
    technique_.addOption("sobel", "Sobel");
    technique_.addOption("linear-regression", "Linear regression");
    addProperty(technique_);
    addProperty(copyIntensityChannel_);

    addPort(inport_);
    addPort(outport_);
}

VolumeGradientCL::~VolumeGradientCL() {
    clearCL();
}

std::string VolumeGradientCL::getProcessorInfo() const {
    return std::string("OpenCL-based version of the VolumeGradient processor.");
}

void VolumeGradientCL::clearCL() {
    delete prog_;
    delete queue_;
    delete context_;
    delete opencl_;
    prog_ = 0;
    queue_ = 0;
    context_ = 0;
    opencl_ = 0;
}

void VolumeGradientCL::initialize() throw(VoreenException) {
    //TODO: Central OpenCL init in voreen
    VolumeProcessor::initialize();

    opencl_ = new OpenCL();

    const std::vector<Platform>& platforms = opencl_->getPlatforms();
    if (platforms.empty()) {
        LERROR("No OpenCL platforms found");
        clearCL();
        initialized_ = false;
        throw VoreenException("No OpenCL platforms found");
    }

    const std::vector<Device*>& devices = platforms[0].getDevices();
    if (devices.empty()) {
        LERROR("No devices in platform found");
        clearCL();
        initialized_ = false;
        throw VoreenException("No devices in platform found");
    }

    context_ = new Context(Context::generateGlSharingProperties(), devices[0]);
    queue_ = new CommandQueue(context_, devices.back());

    std::string kernelFile = "../../src/modules/opencl/gradient.cl";
    LINFO("Loading program: " << kernelFile);
    prog_ = new Program(context_);
    bool success = (prog_->loadSource(kernelFile) && prog_->build(devices.back()));
    if (!success) {
        clearCL();
        initialized_ = false;
        LERROR("Unable to load program: " << kernelFile);
        throw VoreenException("Unable to load program: " + kernelFile);
    }
}

void VolumeGradientCL::process() {

    tgtAssert(prog_, "No program");

    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = 0;

    // expecting a single-channel volume
    /*if (inputVolume->getNumChannels() == 1) {

        bool bit16 = inputVolume->getBitsAllocated() > 8;

        if (technique_.get() == "central-differences") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradients<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradients<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradients<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradients<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else if (technique_.get() == "sobel") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsSobel<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsSobel<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsSobel<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsSobel<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else if (technique_.get() == "linear-regression") {
            if (copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsLinearRegression<tgt::Vector4<uint16_t> >(inputVolume);
            else if (copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsLinearRegression<tgt::col4>(inputVolume);
            else if (!copyIntensityChannel_.get() && bit16)
                outputVolume = calcGradientsLinearRegression<tgt::Vector3<uint16_t> >(inputVolume);
            else if (!copyIntensityChannel_.get() && !bit16)
                outputVolume = calcGradientsLinearRegression<tgt::col3>(inputVolume);
            else {
                tgtAssert(false, "Should not get here");
            }
        }
        else {
            LERROR("Unknown technique");
        }
    }
    else {
        LWARNING("Intensity volume expected, but passed volume consists of " << inputVolume->getNumChannels() << " channels.");
    }
    */

    Kernel* kernel = prog_->getKernel("gradient");
    if (kernel) {
        glFinish();

        outputVolume = new Volume3xUInt8(inputVolume->getDimensions());

        Buffer inputBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputVolume->getNumBytes(), inputVolume->getData());
        Buffer outputBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, inputVolume->getNumBytes()*3);

        kernel->setArg(0, inputBuffer);
        kernel->setArg(1, outputBuffer);

        queue_->enqueue(kernel, inputVolume->getDimensions());

        queue_->enqueueRead(&outputBuffer, outputVolume->getData(), true);
        //outport_.getData()->generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);

        //SharedTexture in(context_, CL_MEM_READ_ONLY, inport_.getData()->getVolumeGL()->getTexture());
        //SharedTexture out(context_, CL_MEM_WRITE_ONLY, outport_.getColorTexture());

        //kernel->setArg(0, in);
        //k->setArg(1, out);
        //k->setArg(2, saturation_.get());

        //queue_->enqueueAcquireGLObject(&in);
        //queue_->enqueueAcquireGLObject(&out);
        //queue_->enqueue(kernel, inputVolume->getDimensions());
        //queue_->enqueueReleaseGLObject(&in);
        //queue_->enqueueReleaseGLObject(&out);

        queue_->finish();

        // assign computed volume to outport
        VolumeHandle* prevVolume = outport_.getData();
        VolumeHandle* newHandle = new VolumeHandle(outputVolume);
        if (outputVolume)
            outport_.setData(newHandle);
        else
            outport_.setData(0);
        delete prevVolume;
    }

}

}   // namespace
