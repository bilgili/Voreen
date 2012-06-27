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

#ifndef VRN_GLSLRAYCASTER_H
#define VRN_GLSLRAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/shaderproperty.h"

#include "voreen/core/ports/volumeport.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Performs a simple single pass raycasting which can be modified by changing the shader property.
 */
class GLSLRaycaster : public VolumeRaycaster {
public:
    GLSLRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "GLSLRaycaster";   }
    virtual std::string getCategory() const   { return "Raycasting";      }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);

    /// Calls rebuild() on the shader property.
    virtual void loadShader();

    /// Assigns the generated header to the property and rebuilds the shader.
    virtual void compile(VolumeHandle* volumeHandle);

    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);

private:
    VolumePort volumePort_;   ///< volume inport
    RenderPort entryPort_;    ///< entry points texture
    RenderPort exitPort_;     ///< exit points texture
    RenderPort outport_;      ///< output rendering

    TransFuncProperty transferFunc_;  ///< the property that controls the transfer function
    ShaderProperty shader_;   ///< the property that controls the used shader
    CameraProperty camera_;   ///< camera needed for lighting calculations
};


} // namespace

#endif // VRN_GLSLRAYCASTER_H
