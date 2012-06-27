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

#ifndef VRN_MERGEPP_H
#define VRN_MERGEPP_H

//TODO: Check if all of this is needed
#include <vector>
#include <string>

#include "tgt/shadermanager.h"
#include "tgt/quadric.h"
#include "tgt/types.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/collect.h"

namespace voreen {

// TODO doxygen
class Merge : public Collect {
public:
    Merge();
    virtual std::string getCategory() const { return "ImageProcessor"; }
    virtual std::string getClassName() const { return "Merge"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new Merge();}

protected:
    virtual void renderTwo(int source0, int source1, int pass);

    static const std::string shadeTexUnit1_;
    static const std::string depthTexUnit1_;
};

} // namespace voreen

#endif //VRN_MERGEPP_H
