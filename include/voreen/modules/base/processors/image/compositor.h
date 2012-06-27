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

#ifndef VRN_COMPOSITOR_H
#define VRN_COMPOSITOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/imageprocessor.h"

#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/floatproperty.h"

namespace voreen {

/// Composites the the images at the two inports using the selected blending method.
class Compositor : public ImageProcessor {
public:

    Compositor();
    ~Compositor();

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "Compositor"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const {return new Compositor();}

    virtual bool isReady() const;

protected:
    void process();
    virtual std::string generateHeader();
    virtual void compile();

    void compositingModeChanged();

    StringOptionProperty compositingMode_;

    FloatProperty weightingFactor_;

    RenderPort inport0_;
    RenderPort inport1_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_COMPOSITOR_H
