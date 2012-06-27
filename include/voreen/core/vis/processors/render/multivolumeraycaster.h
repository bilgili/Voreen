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

#ifndef VRN_MULTIVOLUMERAYCASTER_H
#define VRN_MULTIVOLUMERAYCASTER_H

#include "voreen/core/vis/processors/render/volumeraycaster.h"

namespace voreen {

class DepthPeelingEntryExitPoints;
/**
 * Performs  multi-pass raycasting on the layers computed by depth peeling
 */
class MultiVolumeRaycaster : public VolumeRaycaster {
public:
    /**
     * Constructor.
     */
    MultiVolumeRaycaster();

    virtual ~MultiVolumeRaycaster();

    virtual std::string getCategory() const { return "Raycasting"; }
    virtual std::string getClassName() const { return "MultiVolumeRaycaster"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual void initialize() throw (VoreenException);

    /**
     * Load the needed shader.
     *
     */
    virtual void loadShader();

    /**
     * Performs the raycasting.
     *
     * Initialize two texture units per pass where one represent the entry
     * params of a layer and the other represents the exit params rendering
     * the corresponding volumes to a layer.
     * After getting the output textures for each layer, a second shader
     * blends all the outputs into a single texture.
     */
    virtual void process();

    /**
     * Lookup into the map vector to get the
     * unique ID from the object ID.
     */
    virtual int lookup(int objectid);

protected:
    virtual std::string generateHeader();  // for multi-volume rendering shader
    virtual std::string generateHeader2(); // for the blending shader
    virtual void compile();

    std::vector<int> map; // vector containing the object IDs of all the volumes present in a scene

private:
    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    TransFuncProperty transferFunc2_;  ///< the property that controls the transfer-function
    TransFuncProperty transferFunc3_;

//    EnumProperty* compositingMode1_;    ///< What compositing mode should be applied for second volume
//    EnumProperty* compositingMode2_;    ///< What compositing mode should be applied for third volume

    bool activ1,activ2,activ3;  // to track which volumes are available in a layer
    int numOfVolumes_;
    tgt::Shader* raycastPrg2_; // the blending shader which works in an alpha compositing way

    GenericCoProcessorPort<DepthPeelingEntryExitPoints> dpEepPort_;
    RenderPort private1_;
    RenderPort private2_;
    RenderPort private3_;
    RenderPort private4_;
    RenderPort private5_;
    RenderPort private6_;

    VolumePort volumeInport_;
    RenderPort layersInport_;
    RenderPort outport_;
};

} // namespace

#endif // VRN_MULTIVOLUMERAYCASTER_H
