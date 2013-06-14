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

#ifndef VRN_STREAMLINERENDERER3D_H
#define VRN_STREAMLINERENDERER3D_H

#include "tgt/shadermanager.h"
#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "modules/flowreen/utils/colorcodingability.h"
#include "flowreenprocessor.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/datastructures/volume/volumegl.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/renderport.h"

namespace voreen {

class Flow3D;
class CameraInteractionHandler;

/**
 * Performs rendering of streamlines from a stationary input flow volume by
 * using geometric primitives like lines, tubes or arrows.
 */
class StreamlineRenderer3D : public RenderProcessor, public FlowreenProcessor {

public:
    StreamlineRenderer3D();
    virtual ~StreamlineRenderer3D();

    virtual Processor* create() const { return new StreamlineRenderer3D(); }

    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "StreamlineRenderer3D"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual void initialize() throw (tgt::Exception);
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

    virtual std::string generateShaderHeader();

    bool setupShader(const Flow3D& flow, const tgt::vec2& thresholds);

private:
    virtual void setDescriptions() {
        setDescription("Renders streamline objects from 3D vector fields for visualizing flow");
    }

    enum StreamlineStyle {
        STYLE_LINES,
        STYLE_TUBES,
        STYLE_ARROWS,
        STYLE_ARROW_GRID
    };
    friend class OptionProperty<StreamlineStyle>;

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
