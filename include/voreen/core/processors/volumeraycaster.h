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

#ifndef VRN_VOLUMERAYCASTER_H
#define VRN_VOLUMERAYCASTER_H

#include "tgt/types.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
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
class VolumeRaycaster : public VolumeRenderer {
public:
    VolumeRaycaster();

    /// Save the requested size(newsize) in size_ and resize all renderports, using interactionMode() and interactionCoarseness_.
    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);

    /// Switches interaction coarseness on/off by resizing all renderports.
    virtual void interactionModeToggled();

    /**
     * Resizes the RenderPorts, if interaction coarseness is currently active.
     *
     * @see RenderProcessor::invalidate
     */
    virtual void invalidate(int inv = INVALID_RESULT);

protected:
    /**
     * Defines ray-casting macros to be used in the shader.
     * The volume handle parameter has only to be passed
     * for bricked rendering.
     *
     * @see VolumeRenderer::generateHeader
     */
    virtual std::string generateHeader(VolumeHandle* volumehandle = 0);

    /**
     * Sets frustum parameters necessary for depth value calculation in shaders.
     * The camera parameter is passed to the super class function.
     *
     * @see VolumeRenderer::setGlobalShaderParameters
     */
    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0);

    /**
     * Binds volume textures (inherited from VolumeRenderer) and sets the sampling step size
     * relative to the resolution of the first volume. The camera and light position
     * parameters are passed to the super class function.
     *
     * @see VolumeRenderer::bindVolumes
     */
    virtual void bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera = 0, const tgt::vec4& lightPosition = tgt::vec4(0.f));

    void showBrickingProperties(bool b);

    virtual void setBrickedVolumeUniforms(tgt::Shader* shader, VolumeHandle* volumeHandle);
    virtual void addBrickedVolumeModalities(VolumeHandle* volumeHandle, std::vector<VolumeStruct>& volumeTextures,
                                            tgt::TextureUnit* unit1 = 0, tgt::TextureUnit* unit2 = 0);

    void updateBrickingParameters(VolumeHandle* volumeHandle);
    void changeBrickResolutionCalculator(std::string);
    void changeBrickingUpdateStrategy(std::string);
    void changeBrickLodSelector(std::string);

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

    StringOptionProperty brickingInterpolationMode_;   ///< Which interpolation method to use when rendering bricked volumes.
    StringOptionProperty brickingStrategyMode_;        ///< Which bricking strategy to use when rendering bricked volumes.
    StringOptionProperty brickingUpdateStrategy_;      ///< When to update bricks when rendering bricked volumes.
    StringOptionProperty brickLodSelector_;
    BoolProperty useAdaptiveSampling_;

    tgt::ivec2 size_;                                  ///< The size expected by the processors connected to the outports. ()
    bool switchToInteractionMode_;                     ///< Needed to switch to/from interactionmode.

    bool brickingParametersChanged_;
    std::string brickResoluationModeStr_;
    std::string brickUpdateStrategyStr_;
    std::string brickLodSelectorStr_;

    static const std::string loggerCat_; ///< category used in logging

private:
    /// Initializes and adds the ray-casters properties. Called by the constructor.
    void initProperties();
};

} // namespace voreen

#endif // VRN_VOLUMERAYCASTER_H
