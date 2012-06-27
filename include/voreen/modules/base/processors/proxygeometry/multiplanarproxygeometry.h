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

#ifndef VRN_MULTIPLANARPROXYGEOMETRY_H
#define VRN_MULTIPLANARPROXYGEOMETRY_H

#include <vector>

#include "tgt/vector.h"
#include "tgt/tgt_gl.h"

#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/modules/base/processors/proxygeometry/proxygeometry.h"

namespace voreen {

/**
 * This class represents a slice proxygeometry, which allows to render
 * three othogonal slices, on which the volume is shown.
 */
class MultiPlanarProxyGeometry : public ProxyGeometry {
public:
    MultiPlanarProxyGeometry();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "MultiplanarProxyGeometry"; }
    virtual std::string getCategory() const  { return "Volume Proxy Geometry";    }
    virtual CodeState getCodeState() const   { return CODE_STATE_EXPERIMENTAL;    }
    virtual std::string getProcessorInfo() const;

protected:
    void deinitialize() throw (VoreenException);

    void revalidateSliceGeometry();
    virtual void render();

    // methods for reaction on property changes
    void changeSlicePos();

    GLuint dl_;

    FloatVec3Property slicePos_;
};

} // namespace voreen

#endif // VRN_SLICEPROXYGEOMETRY_H
