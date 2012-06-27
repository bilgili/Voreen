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

#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_STREAMLINERENDERER3D_H
#define VRN_STREAMLINERENDERER3D_H

#include "tgt/shadermanager.h"
#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/modules/flowreen/colorcodingability.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/cameraproperty.h"

namespace voreen {

class Flow3D;
class CameraInteractionHandler;

/**
 * Performs rendering of streamlines from a stationary input flow volume by
 * using geometric primitives like lines, tubes or arrows.
 */
class StreamlineRenderer3D : public RenderProcessor, public FlowreenProcessor
{
public:
    StreamlineRenderer3D();
    virtual ~StreamlineRenderer3D();

    virtual Processor* create() const { return new StreamlineRenderer3D(); }

    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "StreamlineRenderer3D"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual void initialize() throw (VoreenException);
    virtual void process();

private:
    void buildDisplayLists(const Flow3D& flow, const tgt::vec2& thresholds);
    void buildDisplayListsArrowGrid(const Flow3D& flow);

    void initSeedingPositions();

    void invalidateRendering();

    void onColorCodingChange();
    void onStreamlineNumberChange();
    void onStyleChange();

    void renderStreamlineArrows(const std::vector<tgt::vec3>& streamline) const;
    void renderStreamlineLines(const std::vector<tgt::vec3>& streamline, const Flow3D& flow) const;
    void renderStreamlineTubes(const std::vector<tgt::vec3>& streamline) const;

    tgt::vec3 reseedPosition(const tgt::vec3& flowDimensions, const size_t validPositions = 0);

    void setPropertyVisibilities();

    bool setupShader(const Flow3D& flow, const tgt::vec2& thresholds);

private:
    enum StreamlineStyle {
        STYLE_LINES,
        STYLE_TUBES,
        STYLE_ARROWS,
        STYLE_ARROW_GRID
    };

    ColorCodingAbility colorCoding_;
    IntProperty numStreamlinesProp_;
    OptionProperty<StreamlineStyle>* styleProp_;
    IntProperty geometrySpacingProp_;
    IntProperty geometrySizeProp_;
    BoolProperty useAlphaBlendingProp_;
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    StreamlineStyle currentStyle_;
    size_t numStreamlines_;
    bool rebuildDisplayLists_;
    bool reinitSeedingPositions_;
    GLuint displayLists_;
    tgt::Shader* shader_;
    tgt::vec3* seedingPositions_;

    VolumePort volInport_;
    RenderPort imgOutport_;
};

}   // namespace

#endif  // VRN_STREAMLINERENDERER3D_H
#endif  // VRN_MODULE_FLOWREEN
