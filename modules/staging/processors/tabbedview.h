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

#ifndef VRN_TABBEDVIEW_H
#define VRN_TABBEDVIEW_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/fontproperty.h"
#include "voreen/core/properties/vectorproperty.h"

namespace voreen {

class TabbedView : public RenderProcessor {
public:
    TabbedView();
    ~TabbedView();

    virtual bool isReady() const;
    virtual void process();

    virtual std::string getCategory() const { return "View"; }
    virtual std::string getClassName() const { return "TabbedView"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; }
    virtual Processor* create() const;

    virtual void initialize() throw (tgt::Exception);

    virtual void invalidate(int inv = INVALID_RESULT);

    virtual void onEvent(tgt::Event* e);
protected:
    virtual void setDescriptions() {
        setDescription("Allows you to switch between multiple views using a tab-bar.");
    }

    int getBorderWidth();
    tgt::ivec2 getInternalSize();

    void toggleMaximization(tgt::MouseEvent* me);
    void updateSizes();

    void handleMouseEvent(tgt::MouseEvent* e);

    FloatVec4Property borderColor_;
    FloatVec4Property borderHoverColor_;
    FloatVec4Property buttonColor_;
    FloatVec4Property buttonHoverColor_;
    FloatVec4Property textColor_;
    FloatVec4Property textHoverColor_;
    IntProperty currentView_;
    FontProperty fontProp_;
    BoolProperty renderAtBottom_;
    BoolProperty hideTabbar_;

    StringProperty tabText1_;
    StringProperty tabText2_;
    StringProperty tabText3_;
    StringProperty tabText4_;

    RenderPort outport_;

    RenderPort inport1_;
    RenderPort inport2_;
    RenderPort inport3_;
    RenderPort inport4_;

    bool insideViewPort_;
    int mouseOverButton_;

    bool isDragging_;
};

} // namespace voreen

#endif
