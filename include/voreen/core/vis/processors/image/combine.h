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

#ifndef VRN_COMBINEPP_H
#define VRN_COMBINEPP_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/collect.h"

namespace voreen {

/// Combines the result of two Renderer objects with a set of blending methods.
class Combine : public Collect {
public:

    Combine();
    virtual const Identifier getClassName() const {return "ImageProcessor.Combine";}
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new Combine();}
    ~Combine();

    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

protected:

    /**
     * Renders two sources.
     *
     * @param source0 First source.
     * @param source1 Second source.
     */
    virtual void renderTwo(int source0, int source1, int pass);

    virtual void compile();

    virtual std::string generateHeader();

    EnumProp* blendMode_;
    //ConditionProp* cond_;
    std::vector<std::string> blendModeMethods_;
    std::vector<std::string> blendModeMethodDefines_;

    ColorProp firstModifyColor_;
    ColorProp secondModifyColor_;

    FloatProp blendFactor_;

    static const Identifier shadeTexUnit1_;
    static const Identifier depthTexUnit1_;
};


} // namespace voreen

#endif //VRN_MERGEPP_H
