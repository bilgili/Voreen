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
#include "voreen/core/voreenapplication.h"

namespace voreen {

using namespace cl;

GrayscaleCL::GrayscaleCL()
    : RenderProcessor(),
      saturation_("saturation", "Saturation", 0.0f),
      inport_(Port::INPORT, "inport"),
      outport_(Port::OUTPORT, "outport"),
      opencl_(0),
      context_(0),
      queue_(0),
      prog_(0)
{
    // register properties and ports:
    addProperty(saturation_);

    addPort(inport_);
    addPort(outport_);
}

GrayscaleCL::~GrayscaleCL() {
    delete prog_;
    delete queue_;
    delete context_;
    delete opencl_;
}

std::string GrayscaleCL::getProcessorInfo() const {
    return "Converts a color image to grayscale version.";
}

void GrayscaleCL::initialize() throw (VoreenException) {
    //TODO: Central OpenCL init in voreen
    RenderProcessor::initialize();

    opencl_ = new OpenCL();

    const std::vector<Platform>&  platforms = opencl_->getPlatforms();
    if (platforms.empty()) {
        LERROR("Found no OpenCL platforms!");
        return;
    }

    const std::vector<Device*>& devices = platforms[0].getDevices();
    if (devices.empty()) {
        LERROR("Found no devices in platform!");
        return;
    }
    //Device* dev = OpenCL::getCurrentDeviceForGlContext();

    context_ = new Context(Context::generateGlSharingProperties(), devices[0]);
    queue_ = new CommandQueue(context_, devices.back());
    //context_ = new Context(Context::generateGlSharingProperties(), dev);
    //queue_ = new CommandQueue(context_, dev);

    prog_ = new Program(context_);

    //prog_->loadSource("../../src/modules/opencl/grayscale.cl");
    prog_->loadSource(VoreenApplication::app()->getModulePath() + "/opencl/grayscale.cl");

    //prog_->build(dev);
    prog_->build(devices.back());
}

void GrayscaleCL::process() {
    if(prog_) {
        Kernel* k = prog_->getKernel("gr");
        if(k) {
            glFinish();

            SharedTexture in(context_, CL_MEM_READ_ONLY, inport_.getColorTexture());
            SharedTexture out(context_, CL_MEM_WRITE_ONLY, outport_.getColorTexture());

            k->setArg(0, in);
            k->setArg(1, out);
            k->setArg(2, saturation_.get());

            queue_->enqueueAcquireGLObject(&in);
            queue_->enqueueAcquireGLObject(&out);
            queue_->enqueue(k, inport_.getSize());
            queue_->enqueueReleaseGLObject(&in);
            queue_->enqueueReleaseGLObject(&out);

            queue_->finish();

            outport_.validateResult();
        }
    }
}

} // namespace
