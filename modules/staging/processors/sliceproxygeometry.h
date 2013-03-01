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

#ifndef VRN_SLICEPROXYGEOMETRY_H
#define VRN_SLICEPROXYGEOMETRY_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/interaction/mwheelnumpropinteractionhandler.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/vectorproperty.h"

namespace voreen {

class SliceProxyGeometry : public RenderProcessor {
public:
    SliceProxyGeometry();
    virtual ~SliceProxyGeometry();

    virtual std::string getCategory() const { return "Volume"; }
    virtual std::string getClassName() const { return "SliceProxyGeometry"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

    virtual Processor* create() const { return new SliceProxyGeometry(); }

    virtual void process();
protected:
    virtual void setDescriptions() {
        setDescription("Generates a proxy-geometry defines by a plane equation. (Combine with MultiSliceRenderer)");
    }

    FloatVec3Property normal_;   ///< Clipping plane normal
    FloatProperty position_;     ///< Clipping plane position (distance to the world origin)
    FloatProperty size_;         ///< Size of the generated quad

    MWheelNumPropInteractionHandler<float> mwheelCycleHandler_;

    GeometryPort geomPort_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif
