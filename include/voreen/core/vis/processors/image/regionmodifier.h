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

#ifndef VRN_REGIONMODIFIER_H
#define VRN_REGIONMODIFIER_H

//TODO: Check if all of this is needed
#include <vector>
#include <string>
#include <map>

#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/imageprocessor.h"

namespace voreen {

class RegionModifier : public ImageProcessor {
public:
    RegionModifier();

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "RegionModifier"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    void process();

    enum RegionModifierModes {
        MODE_REPLACE,
        MODE_BLEND
    };

protected:
    virtual std::string generateHeader();
    virtual void compile();
    void setRegionModifierModeEvt();

    RegionModifierModes mode_;
    std::map<RegionModifierModes, std::string> modeDefinesMap_;
    StringOptionProperty modeProp_;
    ColorProperty segmentId_;
    ColorProperty destColor_;

    static const std::string shadeTexUnit1_;
    static const std::string depthTexUnit1_;

    RenderPort inport_;
    RenderPort maskPort_;
    RenderPort outport_;
};


} // namespace voreen

#endif //VRN_REGIONMODIFIER_H
