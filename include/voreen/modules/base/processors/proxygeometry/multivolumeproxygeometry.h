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

#ifndef VRN_MULTIVOLUMEPROXYGEOMETRY_H
#define VRN_MULTIVOLUMEPROXYGEOMETRY_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

namespace voreen {

/**
 * Provides a cube mesh proxy geometry for multi-volume raycasting.
 *
 * @see MeshEntryExitPoints
 * @see MeshClipping
 * @see MultiVolumeRaycaster
 */
class MultiVolumeProxyGeometry : public Processor {

public:
    MultiVolumeProxyGeometry();
    virtual ~MultiVolumeProxyGeometry();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "MultiVolumeProxyGeometry"; }
    virtual std::string getCategory() const   { return "Volume Proxy Geometry"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    /**
     * Cube mesh proxy geometry.
     */
    MeshListGeometry* geometry_;

    /**
     * Inport for the dataset.
     */
    VolumePort inport_;

    /**
     * Outport for the cube mesh proxy geometry.
     */
    GeometryPort outport_;
};

} // namespace

#endif // VRN_MULTIVOLUMEPROXYGEOMETRY_H
