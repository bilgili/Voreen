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

#ifndef VRN_MESHCLIPPING_H
#define VRN_MESHCLIPPING_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

namespace voreen {

/**
 * Provides clipping a list of mesh geometries against an arbitrarily oriented clipping plane.
 *
 * @see CubeMeshProxyGeometry
 * @see MeshEntryExitPoints
 * @see MeshClippingWidget
 * @see MeshGeometry
 */
class MeshClipping : public Processor {
public:
    MeshClipping();
    virtual ~MeshClipping();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "MeshClipping"; }
    virtual std::string getCategory() const  { return "Geometry"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();

    GeometryPort inport_;        ///< Inport for a list of mesh geometries to clip.
    GeometryPort outport_;       ///< Outport for a list of mesh geometries that were clipped.

    MeshListGeometry geometry_;  ///< List of mesh geometries to clip, retrieved through the inport.
    FloatVec3Property normal_;   ///< Clipping plane normal
    FloatProperty position_;     ///< Clipping plane position (distance to the world origin)

    /// category used in logging
    static const std::string loggerCat_;
};

} //namespace

#endif // VRN_MESHCLIPPING_H
