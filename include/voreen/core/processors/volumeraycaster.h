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

namespace voreen {

/**
 * Abstract base class for GPU-based raycasters.
 */
class VolumeRaycaster : public VolumeRenderer {
public:
    VolumeRaycaster();
    ~VolumeRaycaster();

    ///Save the requested size(newsize) in size_ and resize all renderports, using interactionMode() and interactionCoarseness_
    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);

    ///Switches interactioncoarseness on/off by resizing all renderports.
    virtual void interactionModeToggled();

    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Binds volume textures (inherited from VolumeRenderer) and sets the sampling step size
     * relative to the resolution of the first volume.
     */
    virtual void bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes);

    static const std::string setSegment_;
    static const std::string switchSegmentation_;

protected:
    /**
     * Disposes the shader.
     */
    void deinitialize() throw (VoreenException);

    /**
     * Load the shader into memory.
     */
    virtual void loadShader(VolumeHandle* /*volumeHandle*/ = 0) {}

    /**
     * Compile and link the shader program
     */
    virtual void compile(VolumeHandle* /*volumeHandle*/ = 0) {}

    /**
     * Generates header defines for the shader.
     */
    virtual std::string generateHeader(VolumeHandle* volumehandle = 0);

    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0);

    /**
     * Initialize the properties of the raycaster.
     */
    void initProperties();

    void showBrickingProperties(bool b);

    virtual void setBrickedVolumeUniforms(VolumeHandle* volumeHandle);
    virtual void addBrickedVolumeModalities(VolumeHandle* volumeHandle, std::vector<VolumeStruct>& volumeTextures,
                                            tgt::TextureUnit* unit1 = 0, tgt::TextureUnit* unit2 = 0);

    void updateBrickingParameters(VolumeHandle* volumeHandle);
    void changeBrickResolutionCalculator(std::string);
    void changeBrickingUpdateStrategy(std::string);
    void changeBrickLodSelector(std::string);

    tgt::Shader* raycastPrg_; ///< The shader-program to be used with this raycaster.

    FloatProperty samplingRate_;  ///< Sampling rate of the raycasting, specified relative to
                                  ///the size of one voxel
    FloatProperty isoValue_;      ///< The used isovalue, when isosurface raycasting is enabled

    // properties for all volume raycasters
    StringOptionProperty maskingMode_;                      ///< What masking should be applied (thresholding, segmentation)
    StringOptionProperty gradientMode_;                     ///< What type of calculation should be used for on-the-fly gradients
    StringOptionProperty classificationMode_;               ///< What type of transfer function should be used for classification
    StringOptionProperty shadeMode_;                        ///< What shading method should be applied
    StringOptionProperty compositingMode_;                  ///< What compositing mode should be applied

    StringOptionProperty brickingInterpolationMode_;        ///< Which interpolation method to use when rendering bricked volumes.
    StringOptionProperty brickingStrategyMode_;                ///< Which bricking strategy to use when rendering bricked volumes.
    StringOptionProperty brickingUpdateStrategy_;            ///< When to update bricks when rendering bricked volumes.
    StringOptionProperty brickLodSelector_;
    BoolProperty useAdaptiveSampling_;

    IntProperty segment_;                           ///< Controls the segment that is to be rendered
    BoolProperty useSegmentation_;                  ///< Controls whether or not
                                                    ///< segmentation-mode is used. FIXME: deprecated

    IntProperty interactionCoarseness_;             ///< RenderPorts are resized to size_/interactionCoarseness_ in interactionmode
    FloatProperty interactionQuality_;
    BoolProperty useInterpolationCoarseness_;

    tgt::ivec2 size_;                               ///< The size expected by the processors connected to the outports. (Needed to switch to/from interactionmode)
    bool switchToInteractionMode_;

    bool brickingParametersChanged_;
    std::string brickResoluationModeStr_;
    std::string brickUpdateStrategyStr_;
    std::string brickLodSelectorStr_;

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace voreen

#endif // VRN_VOLUMERAYCASTER_H
