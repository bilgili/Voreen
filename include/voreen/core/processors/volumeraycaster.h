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

#ifndef VRN_VOLUMERAYCASTER_H
#define VRN_VOLUMERAYCASTER_H

#include "tgt/types.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/exception.h"

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/volume/volumegl.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"

namespace voreen {

/**
 * Abstract base class for GPU-based ray-casters.
 *
 * It extends the generateHeader, setGlobalShaderParameters and
 * bindVolumes methods inherited from VolumeRenderer.
 * Additionally, it provides several ray-casting-related properties
 * and handles bricked volumes.
 */
class VRN_CORE_API VolumeRaycaster : public VolumeRenderer {
public:
    VolumeRaycaster();

    /// Switches interaction coarseness on/off by resizing all renderports.
    virtual void interactionModeToggled();

    /**
     * Resizes the RenderPorts, if interaction coarseness is currently active.
     *
     * @see RenderProcessor::invalidate
     */
    virtual void invalidate(int inv = INVALID_RESULT);

protected:
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Defines ray-casting macros to be used in the raycasting shader.
     *
     * @see VolumeRenderer::generateHeader
     */
    virtual std::string generateHeader(const tgt::GpuCapabilities::GlVersion* version = 0);

    /**
     * Sets frustum parameters necessary for depth value calculation in shaders.
     * The camera parameter is passed to the super class function.
     *
     * @see VolumeRenderer::setGlobalShaderParameters
     */
    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0, tgt::ivec2 screenDim = tgt::ivec2(-1));

    /**
     * Binds volume textures (inherited from VolumeRenderer) and sets the sampling step size
     * relative to the resolution of the first volume. The camera and light position
     * parameters are passed to the super class function.
     *
     * @see VolumeRenderer::bindVolumes
     */
    virtual bool bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera = 0, const tgt::vec4& lightPosition = tgt::vec4(0.f));

    /**
     * Copies over the content of the srcPort to the destPort,
     * thereby implicitly rescaling the image to the dest dimensions.
     * To be used by subclasses for implementing coarseness (i.e., rendering with reduced dimensions in interaction mode).
     */
    void rescaleRendering(RenderPort& srcPort, RenderPort& destPort);

    /// Calculate sampling step size for a given volume using the current sampling rate
    float getSamplingStepSize(const VolumeBase* vol);

    FloatProperty samplingRate_;  ///< Sampling rate of the raycasting, specified relative to the size of one voxel
    FloatProperty isoValue_;      ///< The used isovalue, when isosurface raycasting is enabled

    // properties for all volume raycasters
    StringOptionProperty maskingMode_;                 ///< What masking should be applied (thresholding, segmentation)
    StringOptionProperty gradientMode_;                ///< What type of calculation should be used for on-the-fly gradients
    StringOptionProperty classificationMode_;          ///< What type of transfer function should be used for classification
    StringOptionProperty shadeMode_;                   ///< What shading method should be applied
    StringOptionProperty compositingMode_;             ///< What compositing mode should be applied

    IntProperty interactionCoarseness_;                ///< RenderPorts are resized to size_/interactionCoarseness_ in interactionmode
    FloatProperty interactionQuality_;
    BoolProperty useInterpolationCoarseness_;

    tgt::Shader* rescaleShader_;                       ///< Shader used by the rescaleRendering() method

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace voreen

#endif // VRN_VOLUMERAYCASTER_H
