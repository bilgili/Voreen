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

#ifndef VRN_OPENCLMODULE_H
#define VRN_OPENCLMODULE_H

#include "voreen/core/voreenmodule.h"

#include "modules/opencl/utils/clwrapper.h"

namespace voreen {

//namespace cl {
    //class OpenCL;
    //class Context;
    //class CommandQueue;
    //class Program;
    //class Device;
//}

class VRN_CORE_API OpenCLModule : public VoreenModule {

public:
    OpenCLModule(const std::string& modulePath);

    virtual std::string getDescription() const {
        return "Processors utilizing OpenCL.";
    }

    /**
     * Frees the allocated OpenCL resources.
     */
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Initializes the OpenCL resources, to be called by OpenCL-based
     * processor before accessing OpenCL resources.
     *
     * Call is ignored, if OpenCL has already been initialized.
     */
    void initCL() throw (VoreenException);

    /**
     * Returns the OpenCL wrapper.
     *
     * @note initCL() must be called first!
     */
    cl::OpenCL* getOpenCL() const;

    /**
     * Returns the OpenCL context.
     *
     * @note initCL() must be called first!
     */
    cl::Context* getCLContext() const;

    /**
     * Returns the OpenCL command queue.
     *
     * @note initCL() must be called first!
     */
    cl::CommandQueue* getCLCommandQueue() const;

    /**
     * Returns the OpenCL platform.
     *
     * @note initCL() must be called first!
     */
    cl::Platform& getCLPlatform();

    /**
     * Returns the OpenCL device.
     *
     * @note initCL() must be called first!
     */
    cl::Device& getCLDevice();

    /**
     * \brief Enables or disables sharing of resources between
     *  OpenGL and OpenCL. Default: enabled
     *
     * @note Has to be called before initCL().
     */
    void setGLSharing(bool enabled);

    /**
     * Returns whether OpenGL sharing is enabled.
     */
    bool getGLSharing() const;

    /**
     * Loads an OpenCL kernel from file.
     *
     * @note initCL() must be called first!
     *
     * @throw VoreenException, if the kernel could not be loaded.
     */
    cl::Program* loadProgram(const std::string& path)
        throw (VoreenException);

    /**
     * Returns the global instance of this class.
     *
     * @note Does not create the instance. If the module class has not been
     *       instantiated yet, the null pointer is returned.
     */
    static OpenCLModule* getInstance();

private:
    // OpenCL resources
    cl::OpenCL* opencl_;
    cl::Context* context_;
    cl::CommandQueue* queue_;
    cl::Platform platform_;
    cl::Device device_;

    bool glSharing_;    ///< Determines whether OpenGL sharing is enabled

    static OpenCLModule* instance_;

    static const std::string loggerCat_;
};

} // namespace

#endif
