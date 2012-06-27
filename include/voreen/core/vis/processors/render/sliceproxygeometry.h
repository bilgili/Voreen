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

#ifndef VRN_SLICEPROXYGEOMETRY_H
#define VRN_SLICEPROXYGEOMETRY_H

#include <vector>

#include "tgt/vector.h"
#include "tgt/tgt_gl.h"

#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"

namespace voreen {

/**
 * This class represents a slice proxygeometry, which allows to render
 * three othogonal slices, on which the volume is shown.
 */
class SliceProxyGeometry : public ProxyGeometry {
public:
	SliceProxyGeometry();
    virtual ~SliceProxyGeometry();

    static const Identifier setSlicePosX_;
    static const Identifier setSlicePosY_;
    static const Identifier setSlicePosZ_;

	virtual const Identifier getClassName() const { return "ProxyGeometry.SliceProxyGeometry"; }
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new SliceProxyGeometry();}

    virtual void processMessage(Message *msg, const Identifier& dest=Message::all_);
    virtual void setPropertyDestination(Identifier tag);
    
protected:
    void revalidateSliceGeometry();
    virtual void render();

	GLuint dl_;

	FloatProp slicePosX_;
    FloatProp slicePosY_;
    FloatProp slicePosZ_;
};

} // namespace voreen

#endif // VRN_SLICEPROXYGEOMETRY_H
