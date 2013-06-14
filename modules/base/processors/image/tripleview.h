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

#ifndef VRN_TRIPLEVIEW_H
#define VRN_TRIPLEVIEW_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/vectorproperty.h"

namespace voreen {

class VRN_CORE_API TripleView : public RenderProcessor {
    enum WindowConfiguration {
        abc = 0,
        Abc = 1,
        Bac = 2,
        Cab = 3,
        A = 4,
        B = 5,
        C = 6
    };

public:
    TripleView();
    ~TripleView();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "TripleView";       }
    virtual std::string getCategory() const  { return "View";             }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE;  }

    virtual bool isReady() const;
    virtual void invalidate(int inv = INVALID_RESULT);

protected:
    void renderPortQuad(RenderPort& rp, tgt::vec3 translate, tgt::vec3 scale);
    void renderLargeSmallSmall(RenderPort& large, RenderPort& small1, RenderPort& small2);
    void distributeEventLargeSmallSmall(RenderPort& large, RenderPort& small1, RenderPort& small2, tgt::MouseEvent* me);

    virtual void setDescriptions() {
        setDescription("Combines three input images in a configurable layout.");
    }

    virtual void process();
    virtual void initialize() throw (tgt::Exception);

    virtual void onEvent(tgt::Event* e);

    void updateSizes();

    void mouseMove(tgt::MouseEvent* e);

    BoolProperty showGrid_;
    FloatVec4Property gridColor_;
    IntOptionProperty configuration_;
    EventProperty<TripleView> maximizeEventProp_;
    EventProperty<TripleView> mouseMoveEventProp_;

    /// Inport whose rendering is mapped to the frame buffer.
    RenderPort outport_;

    RenderPort inport1_;
    RenderPort inport2_;
    RenderPort inport3_;

    int currentPort_;

    bool isDragging_;
};

} // namespace voreen

#endif
