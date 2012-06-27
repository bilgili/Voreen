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

#include "voreen/modules/opencl/grayscale_cl.h"

#include "voreen/modules/opencl/openclmodule.h"
#include "voreen/core/voreenapplication.h"

namespace voreen {

using namespace cl;

GrayscaleCL::GrayscaleCL()
    : RenderProcessor(),
      saturation_("saturation", "Saturation", 0.0f),
      inport_(Port::INPORT, "inport"),
      outport_(Port::OUTPORT, "outport"),
      prog_(0)
{
    // register properties and ports:
    addProperty(saturation_);

    addPort(inport_);
    addPort(outport_);
}

std::string GrayscaleCL::getProcessorInfo() const {
    return "Converts a color image to grayscale version using an OpenCL kernel.";
}

Processor* GrayscaleCL::create() const {
    return new GrayscaleCL();
}

void GrayscaleCL::initialize() throw (VoreenException) {
    RenderProcessor::initialize();

    OpenCLModule::getInstance()->initCL();
    if (!OpenCLModule::getInstance()->getCLContext()) 
        throw VoreenException("No OpenCL context created");

    prog_ = new Program(OpenCLModule::getInstance()->getCLContext());
    prog_->loadSource(VoreenApplication::app()->getModulePath() + "/opencl/grayscale.cl");
    prog_->build(OpenCLModule::getInstance()->getCLDevice());
}

void GrayscaleCL::deinitialize() throw (VoreenException) {
    delete prog_;
    prog_ = 0;

    RenderProcessor::deinitialize();
}

void GrayscaleCL::process() {
    if (prog_) {
        Kernel* k = prog_->getKernel("gr");
        if(k) {
            cl::Context* context = OpenCLModule::getInstance()->getCLContext();
            cl::CommandQueue* commandQueue = OpenCLModule::getInstance()->getCLCommandQueue();
            tgtAssert(context, "No OpenCL context");
            tgtAssert(commandQueue, "No OpenCL command queue");

            glFinish();

            SharedTexture in(context, CL_MEM_READ_ONLY, inport_.getColorTexture());
            SharedTexture out(context, CL_MEM_WRITE_ONLY, outport_.getColorTexture());

            k->setArg(0, in);
            k->setArg(1, out);
            k->setArg(2, saturation_.get());

            commandQueue->enqueueAcquireGLObject(&in);
            commandQueue->enqueueAcquireGLObject(&out);
            commandQueue->enqueue(k, inport_.getSize());
            commandQueue->enqueueReleaseGLObject(&in);
            commandQueue->enqueueReleaseGLObject(&out);

            commandQueue->finish();

            outport_.validateResult();
        }
    }
}

} // namespace
