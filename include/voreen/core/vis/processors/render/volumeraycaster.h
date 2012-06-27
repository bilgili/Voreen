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

#ifndef VRN_VOLUMERAYCASTER_H
#define VRN_VOLUMERAYCASTER_H

#include "tgt/types.h"
#include "tgt/shadermanager.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/volume/volumegl.h"

namespace voreen {

/**
 * A VolumeRenderer that uses raycasting to produce pictures.  This class makes extensive use of shaders.
 * All classes using raycasting inherit this class.
 */
class VolumeRaycaster : public VolumeRenderer, public HasShader {
public:
    /**
     * Constructor.
     */
    VolumeRaycaster();

    virtual ~VolumeRaycaster();

    /**
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - msg.invalidate, which forcefully invalidates the raycaster
     *      - setSegment, which specifies the current segment that is supposed to be rendererd, Msg-Type: int
     *      - switchSegmentation, which switches Segmentation-mode on/off, Msg-Type: bool
     *
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    /**
     * Load the shader into memory.
     */
    virtual void loadShader() {}

    /**
     * Generates header defines for the current raycastPrg_
     */
    virtual std::string generateHeader();

    virtual void setGlobalShaderParameters(tgt::Shader* shader);

    static const Identifier setSegment_;
    static const Identifier switchSegmentation_;

protected:
    /**
     * Initialize the properties of the raycaster.
     */
    void initProperties();

    /**
     * Compile and link the shader program
     */
    virtual void compile() {}

    void setRaycastingQualityFactorEvt();

    tgt::Shader* raycastPrg_; ///< The shader-program to be used with this raycaster.

    static const Identifier entryParamsTexUnit_;       ///< The texture unit used for entry-parameters
    static const Identifier entryParamsDepthTexUnit_;  ///< The texture unit used for the depth-values of the entry-parameters
    static const Identifier exitParamsTexUnit_;        ///< The texture unit used for exit-parameters
    static const Identifier exitParamsDepthTexUnit_;   ///< The texture unit used for the depth-values of exit-parameters
    static const Identifier entryParamsTexUnit2_;      ///< The texture unit used for entry-parameters 2
    static const Identifier entryParamsDepthTexUnit2_; ///< The texture unit used for the depth-values of the entry-parameters 2
    static const Identifier exitParamsTexUnit2_;       ///< The texture unit used for exit-parameters 2
    static const Identifier exitParamsDepthTexUnit2_;  ///< The texture unit used for the depth-values of exit-parameters 2

    static const Identifier volTexUnit_;              ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier volTexUnit2_;             ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier volTexUnit3_;             ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier transferTexUnit_;         ///< The texture unit used for the transfer-function
    static const Identifier transferTexUnit2_;        ///< The texture unit used for the second transfer-function
    static const Identifier transferTexUnit3_;        ///< The texture unit used for the third transfer-function

    static const Identifier segmentationTexUnit_;       ///< The texture unit used for the segmentation-texture

    static const Identifier ambTexUnit_;                ///< The texture unit used for dao
    static const Identifier ambLookupTexUnit_;          ///< The texture unit used for dao
    static const Identifier normalsTexUnit_;            ///< The texture unit used for normals
    static const Identifier gradientsTexUnit_;          ///< The texture unit used for gradients
    static const Identifier gradientMagnitudesTexUnit_; ///< The texture unit used for gradient magnitudes

    OptionProperty<float>* raycastingQualityFactor_; ///< The property that can be used to change the quality-factor of the raycasting

    // properties for all volume raycasters
    EnumProp* maskingMode_;                              ///< What masking should be applied (thresholding, segmentation)
    std::vector<std::string> maskingModes_;
    EnumProp* gradientMode_;                        ///< What type of calculation should be used for on-the-fly gradients
    std::vector<std::string> gradientModes_;
    EnumProp* classificationMode_;                    ///< What type of transfer function should be used for classification
    std::vector<std::string> classificationModes_;
    EnumProp* shadeMode_;                            ///< What shading method should be applied
    std::vector<std::string> shadeModes_;
    EnumProp* compositingMode_;                        ///< What compositing mode should be applied
    std::vector<std::string> compositingModes_;

    IntProp segment_;                                ///< Controls the segment that is to be rendered
    BoolProp useSegmentation_;                        ///< Controls whether or not
                                                    ///< segmentation-mode is used. FIXME: deprecated

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace voreen

#endif // VRN_VOLUMERAYCASTER_H
