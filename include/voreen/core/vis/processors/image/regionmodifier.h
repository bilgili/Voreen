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

#ifndef VRN_REGIONMODIFIER_H
#define VRN_REGIONMODIFIER_H

//TODO: Check if all of this is needed
#include <vector>
#include <string>
#include <map>

#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/genericfragment.h"
#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

class RegionModifier : public GenericFragment {
public:
    RegionModifier();

    virtual const Identifier getClassName() const {return "PostProcessor.RegionModifier";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new RegionModifier();}

    void process(LocalPortMapping* portMapping);

    /**
     *  
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    enum RegionModifierModes {
        MODE_REPLACE,
        MODE_BLEND
    };

protected:
    virtual std::string generateHeader();
    virtual void compile();

    RegionModifierModes mode_;
    std::map<RegionModifierModes, std::string> modeDefinesMap_;
    EnumProp* modeProp_;
    ColorProp segmentId_;
    ColorProp destColor_;

    static const Identifier shadeTexUnit1_;
    static const Identifier depthTexUnit1_;
};


} // namespace voreen

#endif //VRN_REGIONMODIFIER_H
