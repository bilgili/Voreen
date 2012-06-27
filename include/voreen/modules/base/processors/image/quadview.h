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

#ifndef VRN_MULTIVIEW_H
#define VRN_MULTIVIEW_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/eventproperty.h"

namespace voreen {

class QuadView : public RenderProcessor {
public:
    QuadView();
    ~QuadView();

    virtual bool isReady() const;
    virtual void process();

    virtual std::string getCategory() const { return "View"; }
    virtual std::string getClassName() const { return "QuadView"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual void initialize() throw (VoreenException);

    virtual void invalidate(int inv = INVALID_RESULT);

    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);
    virtual void sizeOriginChanged(RenderPort* p);
    bool testSizeOrigin(const RenderPort* /*p*/, void* /*so*/) const;

    virtual void onEvent(tgt::Event* e);
protected:
    void toggleMaximization(tgt::MouseEvent* me);
    void updateSizes();

    BoolProperty showGrid_;
    ColorProperty gridColor_;
    IntProperty maximized_;
    BoolProperty maximizeOnDoubleClick_;
    EventProperty<QuadView> maximizeEventProp_;

    /// Inport whose rendering is mapped to the frame buffer.
    RenderPort outport_;

    RenderPort inport1_;
    RenderPort inport2_;
    RenderPort inport3_;
    RenderPort inport4_;
};

} // namespace voreen

#endif // VRN_MULTIVIEW_H
