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

#include "voreen/modules/opencl/openclmodule.h"
#include "voreen/core/voreenapplication.h"

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
    //technique_.addOption("sobel", "Sobel");
    //technique_.addOption("linear-regression", "Linear regression");
    addProperty(technique_);
    addProperty(copyIntensityChannel_);

    addPort(inport_);
    addPort(outport_);
}

Processor* VolumeGradientCL::create() const {
    return new VolumeGradientCL();
}

std::string VolumeGradientCL::getProcessorInfo() const {
    return std::string("OpenCL-based version of the VolumeGradient processor.");
}

void VolumeGradientCL::initialize() throw(VoreenException) {
    VolumeProcessor::initialize();

    OpenCLModule::getInstance()->initCL();
    if (!OpenCLModule::getInstance()->getCLContext()) 
        throw VoreenException("No OpenCL context created");

    opencl_ = OpenCLModule::getInstance()->getOpenCL();
    context_ = OpenCLModule::getInstance()->getCLContext();
    queue_ = OpenCLModule::getInstance()->getCLCommandQueue();

    std::string kernelFile = VoreenApplication::app()->getModulePath() + "/opencl/gradient.cl";
    LINFO("Loading program " << kernelFile);
    prog_ = new Program(OpenCLModule::getInstance()->getCLContext());
    prog_->loadSource(kernelFile);
    bool success = prog_->build(OpenCLModule::getInstance()->getCLDevice());
    if (!success) 
        throw VoreenException("Unable to load program: " + kernelFile);

    if (!prog_->getKernel("gradient"))
        throw VoreenException("Kernel 'gradient' not found");
}

void VolumeGradientCL::deinitialize() throw (VoreenException) {
    delete prog_;
    prog_ = 0;

    VolumeProcessor::deinitialize();
}   

void VolumeGradientCL::process() {

    tgtAssert(prog_, "No program");

    Volume* inputVolume = inport_.getData()->getVolume();
    Volume* outputVolume = 0;

    if (dynamic_cast<VolumeUInt8*>(inputVolume)) { 

        Kernel* kernel = prog_->getKernel("gradient");
        if (kernel) {
            glFinish();

            if (copyIntensityChannel_.get())
                outputVolume = new Volume4xUInt8(inputVolume->getDimensions()); 
            else
                outputVolume = new Volume3xUInt8(inputVolume->getDimensions()); 

            Buffer inputBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, inputVolume->getNumBytes(), inputVolume->getData());
                Buffer outputBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, outputVolume->getNumBytes());

            kernel->setArg(0, inputBuffer);
            kernel->setArg(1, outputBuffer);
            kernel->setArg(2, copyIntensityChannel_.get() ? 1 : 0);

            queue_->enqueue(kernel, inputVolume->getDimensions());
            queue_->enqueueRead(&outputBuffer, outputVolume->getData(), true);

            queue_->finish();
        }
        else {
            LERROR("Kernel 'gradient' not found");
        }
    }
    else {
        LERROR("Currently only VolumeUInt8 supported");
    }

    // assign computed volume to outport
    if (outputVolume)
        outport_.setData(new VolumeHandle(outputVolume), true);
    else
        outport_.setData(0, true);

}

}   // namespace
