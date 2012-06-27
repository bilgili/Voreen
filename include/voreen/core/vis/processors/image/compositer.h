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

#ifndef VRN_COMPOSITER_H
#define VRN_COMPOSITER_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/imageprocessor.h"

namespace voreen {

/// Composites the the images at the two inports using the selected blending method.
class Compositer : public ImageProcessor {
public:

    Compositer();
    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "Compositer"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new Compositer();}
    ~Compositer();

    void process();

protected:

    virtual std::string generateHeader();
    virtual void compile();

    StringOptionProperty compositingMode_;

    FloatProperty blendFactor_;

    static const std::string shadeTexUnit1_;
    static const std::string depthTexUnit1_;

    RenderPort inport0_;
    RenderPort inport1_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_COMPOSITER_H
