/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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
#include "processors/singleoctreeraycastercl.h"
#include "processors/volumegradient_cl.h"

#include "voreen/core/voreenapplication.h"

namespace voreen {

OpenCLModule* OpenCLModule::instance_ = 0;
const std::string OpenCLModule::loggerCat_ = "voreen.OpenCLModule";

OpenCLModule::OpenCLModule(const std::string& modulePath)
    : VoreenModule(modulePath)
    , opencl_(0)
    , context_(0)
    , queue_(0)
    , glSharing_(true)
{
    setID("OpenCL");
    setGuiName("OpenCL");
    instance_ = this;

    registerSerializableType(new DynamicCLProcessor());
    registerSerializableType(new GrayscaleCL());
    registerSerializableType(new RaycasterCL());
    registerSerializableType(new RaytracingEntryExitPoints());
    registerSerializableType(new SingleOctreeRaycasterCL());
    registerSerializableType(new VolumeGradientCL());

    addShaderPath(getModulePath("glsl"));
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

    if (glSharing_ && !tgt::Singleton<tgt::GpuCapabilities>::isInited()) {
        LWARNING("No OpenGL context available: disabling OpenGL sharing");
        glSharing_ = false;
    }

    opencl_ = new cl::OpenCL();

    // select platform
    const std::vector<cl::Platform>&  platforms = opencl_->getPlatforms();
    if (opencl_->getPlatforms().empty()) {
        LERROR("Found no OpenCL platforms!");
        throw VoreenException("Found no OpenCL platforms");
    }
    else if (opencl_->getPlatforms().size() == 1) {
        platform_ = opencl_->getPlatforms().front();
    }
    else {
        // multiple platforms available
        if (tgt::Singleton<tgt::GpuCapabilities>::isInited())
            platform_ = opencl_->getPlatformByVendor(GpuCaps.getVendor());
        else
            platform_ = platforms.front();
        LINFO("Selected platform: " << platform_.getName());
    }
    platform_.logInfos();

    // select device
    const std::vector<cl::Device>& devices = platform_.getDevices();
    if (devices.empty()) {
        LERROR("Found no devices in platform!");
        throw VoreenException("Found no devices in platform");
    }
    else if (devices.size() == 1){
        device_ = devices.front();
        LINFO("Device 0: " << device_.getName());
    }
    else {
        // multiple devices available
        for (size_t i=0; i<devices.size(); i++) {
            LINFO("Device " << i << ": " << devices.at(i).getName());
        }

        // select GPU device
        bool found = false;
        for (size_t i=0; i<devices.size() && !found; i++) {
            if (devices.at(i).getType() == CL_DEVICE_TYPE_GPU) {
                device_ = devices.at(i);
                found = true;
            }
        }

        if (found) {
            LINFO("Selected GPU device: " << device_.getName());
        }
        else {
            device_ = devices.front();
            LWARNING("No GPU device found! Using: " << device_.getName());
        }
    }

    // create context and command queue
    if (glSharing_) {
        LINFO("OpenGL sharing: enabled");
        context_ = new cl::Context(cl::Context::generateGlSharingProperties(), device_);
    }
    else {
        LINFO("OpenGL sharing: disabled");
        context_ = new cl::Context(device_);
    }
    queue_ = new cl::CommandQueue(context_, device_, CL_QUEUE_PROFILING_ENABLE);

    device_.logInfos();
}

cl::OpenCL* OpenCLModule::getOpenCL() const {
    if (!opencl_)
        LERROR("No OpenCL wrapper. Call initCL first!");
    return opencl_;
}

cl::Context* OpenCLModule::getCLContext() const {
    if (!context_)
        LERROR("No OpenCL context. Call initCL first!");
     return context_;
}

cl::CommandQueue* OpenCLModule::getCLCommandQueue() const {
    if (!queue_)
        LERROR("No OpenCL queue. Call initCL first!");
    return queue_;
}

cl::Platform& OpenCLModule::getCLPlatform() {
    if (!opencl_)
        LERROR("No OpenCL platform. Call initCL first!");
    return platform_;
}

cl::Device& OpenCLModule::getCLDevice() {
    if (device_.getId() == 0)
        LERROR("No OpenCL device. Call initCL first!");
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
    LINFO("Loading program " << path);
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
