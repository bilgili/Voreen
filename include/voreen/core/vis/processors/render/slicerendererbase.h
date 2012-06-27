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

#ifndef VRN_SLICERENDERER_H
#define VRN_SLICERENDERER_H

#include <cstring>

#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/plane.h"
#include "tgt/shadermanager.h"
#include "tgt/camera.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/vis/transfunc/transfunc.h"

namespace voreen {

class EnumProp;

/**
 * Base class for all SliceRendering sub classes.
 * Provides basic functionality.
 */
class SliceRendererBase : public VolumeRenderer {
public:
    SliceRendererBase();
    virtual ~SliceRendererBase();

    /// Inits some OpenGL states
    //virtual void init();

    /// Sets previously defined OpenGL states back to orignal values
    //virtual void deinit();

protected:

    /**
     * Returns <kbd>true</kbd> if all necessary data have been initialized so rendering can be started,
     * <kbd>false</kbd> otherwise.
     */
    bool ready() const;

    /**
     * Initializes the shader if applicable and sets all uniforms.
     */
    virtual bool setupShader();

    /**
     * Generates the header for the shader depending on the choice of
     * features to be used.
     */
    virtual std::string buildShaderHeader();

    /**
     * Recompiles the shader.
     */
    bool rebuildShader();

    void updateShaderUniforms();

    /**
     * Deactivates the shader
     */
    void deactivateShader();

protected:
    TransFuncProp transferFunc_;
    tgt::Shader* transferFuncShader_;

    static const Identifier transFuncTexUnit_;
    static const Identifier volTexUnit_;
};

// ----------------------------------------------------------------------------

} // namespace voreen

#endif // VRN_SLICERENDERER_H
