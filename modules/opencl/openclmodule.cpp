/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "openclmodule.h"
#include "utils/clwrapper.h"

#include "processors/grayscale_cl.h"
#include "processors/raycaster_cl.h"
#include "processors/dynamicclprocessor.h"
#include "processors/raytracingentryexitpoints.h"
#include "processors/volumegradient_cl.h"

#include "voreen/core/voreenapplication.h"

namespace voreen {

OpenCLModule* OpenCLModule::instance_ = 0;

OpenCLModule::OpenCLModule(const std::string& modulePath)
    : VoreenModule(modulePath)
    , opencl_(0)
    , context_(0)
    , queue_(0)
    //, device_(0)
    , glSharing_(true)
{
    setName("OpenCL");
    instance_ = this;

    registerProcessor(new DynamicCLProcessor());
    registerProcessor(new GrayscaleCL());
    registerProcessor(new RaycasterCL());
    registerProcessor(new RaytracingEntryExitPoints());
    registerProcessor(new VolumeGradientCL());
}

void OpenCLModule::deinitialize() throw (tgt::Exception) {
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

    const std::vector<cl::Device>& devices = platforms[0].getDevices();
    if (devices.empty()) {
        LERRORC("voreen.OpenCLModule", "Found no devices in platform!");
        throw VoreenException("Found no devices in platform");
    }
    device_ = devices[0];
    if (glSharing_) {
        LINFO("Using OpenGL sharing");
        context_ = new cl::Context(cl::Context::generateGlSharingProperties(), device_);
    }
    else {
        LINFO("No OpenGL sharing");
        context_ = new cl::Context(device_);
    }
    queue_ = new cl::CommandQueue(context_, device_);
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

cl::Device& OpenCLModule::getCLDevice() {
    if (device_.getId() == 0)
        LERRORC("voreen.OpenCLModule", "No OpenCL device. Call initCL first!");
    return device_;
}

void OpenCLModule::setGLSharing(bool enabled) {
    if (context_) {
        LWARNING("setGLSharing() no effect: OpenCL context already initialized");
    }
    else {
        glSharing_ = enabled;
    }
}

bool OpenCLModule::getGLSharing() const {
    return glSharing_;
}

OpenCLModule* OpenCLModule::getInstance() {
    return instance_;
}

cl::Program* OpenCLModule::loadProgram(const std::string& path) throw (VoreenException) {
    if (!opencl_)
        throw VoreenException("OpenCLModule: no OpenCL wrapper. Call initCL first!");

    std::string kernelFile = VoreenApplication::app()->getModulePath("opencl") + "/cl/voreenblas.cl";
    LINFOC("voreen.OpenCLModule", "Loading program " << path);
    cl::Program* prog = new cl::Program(getCLContext());
    if (!prog->loadSource(kernelFile)) {
        delete prog;
        throw VoreenException("Failed to load OpenCL program: " + kernelFile);
    }

    if (!prog->build(getCLDevice())) {
        delete prog;
        throw VoreenException("Failed to build OpenCL program: " + kernelFile);
    }

    return prog;
}

} // namespace
