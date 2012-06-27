/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/property.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"
#include "voreen/core/volume/volumegl.h"

namespace voreen {

class ShadowHelper;

/**
 * A VolumeRenderer that uses raycasting to produce pictures.  This class makes extensive use of shaders.
 * All classes using raycasting inherit this class.
 */
class VolumeRaycaster : public VolumeRenderer {
public:

    /**
     * The Constructor.
     *
     * @param camera The camera from wich we will get information about the current modelview-matrix.
     * @param tc The TextureContainer that will be used to manage TextureUnits for all render-to-texture work done by the VolumeRaycaster.
     * @param fragmentShaderFilename The name of the fragment-shader this VolumeRaycaster will use.
     */
    VolumeRaycaster(std::string fragmentShaderFilename = "rc_simple.frag");

    virtual ~VolumeRaycaster();

    friend class ShadowHelper;

    virtual void setVolumeHandle(VolumeHandle* const handle);

    /**
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - msg.invalidate, which forcefully invalidates the raycaster
     *      - set.splitMode, which tells the raycaster the number of split-datasets used in split-mode. Msg-Type: int
     *      - setLowerThreshold, which sets the lower threshold which will be used during the raycasting. Msg-Type: float
     *      - setUpperThreshold, which sets the upper threshold which will be used during the raycasting. Msg-Type: float
     *      - setSegment, which specifies the current segment that is supposed to be rendererd, Msg-Type: int
     *      - switchSegmentation, which switches Segmentation-mode on/off, Msg-Type: bool
     *      - setRaycastingQualityFactor, which specifies the quality of the raycasting, Msg-Type: string
     *
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    /**
     * Sets the message-tag of the propertiy-members.
     *
     * @param tag The message-tag to be used.
     */
    virtual void setPropertyDestination(Identifier tag);

    /**
     * Load the shader into memory.
     */
    virtual void loadShader(){};

    /**
     * Restores all matrix stacks.
     */
    virtual void restoreMatrixStacks();

    /**
     * Raycast through a volume dataset. Textures with the entry/exit
     * params are stored in tc. Front textures are stored
     * at even position (starting with 0); back textures at odd positions.
     * Output should be written into the last render target.
     *
     * The getter/setter are functional but don't invalidate shaders. If this
     * is required, override the corresponding method and call the base method
     * and invalidateShader().
     */
    virtual int initializeGL();

    /**
     * Generates header defines for the current raycastPrg_
     */
    virtual std::string generateHeader();

    virtual void setGlobalShaderParameters(tgt::Shader* shader);

    static const Identifier setRaycastingQualityFactor_;
    static const Identifier setSegment_;
    static const Identifier switchSegmentation_;
    static const Identifier switchGradientsOnTheFly_;
    static const Identifier selectRaycaster_;

protected:

    /**
     * Initialize the properties of the raycaster.
     */
    void initProperties();

    /**
     * Compile the shader if necessary.
     *
     * Checks the bool flag isShaderValid_ and calls the virtual method compile()
     * if necessary. So derived classes should place their compile code there and
     * call the method invalidateShader() when recompiling is required.
     */
    virtual void compileShader();

    /**
     * Invalidate the shader, so that recompiling is performed before the next
     * rendering.
     */
    virtual void invalidateShader();

    /**
     * Compile and link the shader program
     */
    virtual void compile(){};

    bool needRecompileShader_; ///< should the shader recompiled?
    tgt::Shader* raycastPrg_; ///< The shader-program to be used with this raycaster.
    std::string fragmentShaderFilename_; ///< The name of the used Fragment-Shader.

    static const Identifier entryParamsTexUnit_;      ///< The texture unit used for entry-parameters
    static const Identifier entryParamsDepthTexUnit_; ///< The texture unit used for the depth-values of the entry-parameters
    static const Identifier exitParamsTexUnit_;       ///< The texture unit used for exit-parameters
    static const Identifier exitParamsDepthTexUnit_;  ///< The texture unit used for the depth-values of exit-parameters
    static const Identifier volTexUnit_;              ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier volTexUnit2_;             ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier volTexUnit3_;             ///< The texture unit used for the actual volume that is supposed to be raycasted
    static const Identifier transferTexUnit_;         ///< The texture unit used for the transfer-function
    static const Identifier transferTexUnit2_;        ///< The texture unit used for the second transfer-function
	static const Identifier transferTexUnit3_;        ///< The texture unit used for the third transfer-function
    static const Identifier segmentationTexUnit_;     ///< The texture unit used for the segmentation-texture
    static const Identifier ambTexUnit_;              ///< The texture unit used for dao
    static const Identifier ambLookupTexUnit_;        ///< The texture unit used for dao
    static const Identifier normalsTexUnit_;        ///< The texture unit used for normals
    static const Identifier gradientMagnitudesTexUnit_; ///< The texture unit used for gradient magnitudes

    typedef std::vector<std::string> SplitNames;
    SplitNames splitNames_;

    IntProp splitMode_; ///< The property that controls the split-mode

    FloatProp raycastingQualityFactor_; ///< The property that can be used to change the quality-factor of the raycasting
    std::vector<std::string> raycastingQualities_; ///< contains the different quality-modes
    EnumProp* raycastingQualitiesEnumProp_; /// Controls the kind of quality used for raycasting

    IntProp segment_; ///< controls the segment that is to be rendered
    BoolProp useSegmentation_; ///< controls whether or not segmentation-mode is used

    ProxyGeometry* pg_;
};

} // namespace voreen

#endif // VRN_VOLUMERAYCASTER_H
