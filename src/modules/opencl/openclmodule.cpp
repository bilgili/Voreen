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

#include "voreen/modules/opencl/openclmodule.h"
#include "voreen/modules/opencl/clwrapper.h"

#include "voreen/modules/opencl/grayscale_cl.h"
#include "voreen/modules/opencl/raytracingentryexitpoints.h"
#include "voreen/modules/opencl/volumegradient_cl.h"

namespace voreen {

OpenCLModule* OpenCLModule::instance_ = 0;

OpenCLModule::OpenCLModule()
    : VoreenModule()
    , opencl_(0)
    , context_(0)
    , queue_(0)
    , device_(0)
{
    setName("OpenCL");

    addProcessor(new GrayscaleCL());
    addProcessor(new RaytracingEntryExitPoints());
    addProcessor(new VolumeGradientCL());

    instance_ = this;
}

void OpenCLModule::deinitialize() throw (VoreenException) {
    delete queue_;
    delete context_;
    delete opencl_;
    queue_ = 0;
    context_ = 0;
    opencl_ = 0;

    VoreenModule::deinitialize();
}

void OpenCLModule::initCL() throw (VoreenException) {
    if (opencl_)
        return;

    opencl_ = new cl::OpenCL();

    const std::vector<cl::Platform>&  platforms = opencl_->getPlatforms();
    if (platforms.empty()) {
        LERRORC("voreen.OpenCLModule", "Found no OpenCL platforms!");
        throw VoreenException("Found no OpenCL platforms");
    }

    const std::vector<cl::Device*>& devices = platforms[0].getDevices();
    if (devices.empty()) {
        LERRORC("voreen.OpenCLModule", "Found no devices in platform!");
        throw VoreenException("Found no devices in platform");
    }
    //Device* dev = OpenCL::getCurrentDeviceForGlContext();

    device_ = devices[0];
    context_ = new cl::Context(cl::Context::generateGlSharingProperties(), device_);
    queue_ = new cl::CommandQueue(context_, device_);
    //context_ = new Context(Context::generateGlSharingProperties(), dev);
    //queue_ = new CommandQueue(context_, dev);
}

cl::OpenCL* OpenCLModule::getOpenCL() const {
    if (!opencl_) 
        LERRORC("voreen.OpenCLModule", "No OpenCL wrapper. Call initCL first!");
    return opencl_;
}

cl::Context* OpenCLModule::getCLContext() const {
    if (!context_)
        LERRORC("voreen.OpenCLModule", "No OpenCL context. Call initCL first!");
     return context_;
}

cl::CommandQueue* OpenCLModule::getCLCommandQueue() const {
    if (!queue_) 
        LERRORC("voreen.OpenCLModule", "No OpenCL queue. Call initCL first!");
    return queue_;
}

cl::Device* OpenCLModule::getCLDevice() const {
    if (!device_) 
        LERRORC("voreen.OpenCLModule", "No OpenCL device. Call initCL first!");
    return device_;
}

OpenCLModule* OpenCLModule::getInstance() {
    if (!instance_) 
        LERRORC("voreen.OpenCLModule", "not instantiated");

    return instance_;
}


} // namespace
