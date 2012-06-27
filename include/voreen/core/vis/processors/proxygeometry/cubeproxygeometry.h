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

#ifndef VRN_CUBEPROXYGEOMETRY_H
#define VRN_CUBEPROXYGEOMETRY_H

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

namespace voreen {

class CameraInteractionHandler;

/**
 * Provides a simple cube proxy with clipping.
 */
class CubeProxyGeometry : public ProxyGeometry {
public:
    CubeProxyGeometry();
    virtual ~CubeProxyGeometry();

    virtual std::string getCategory() const { return "ProxyGeometry"; }
    virtual std::string getClassName() const { return "CubeProxyGeometry"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new CubeProxyGeometry(); }

    virtual void render();
    virtual void process();

protected:
    void revalidateCubeGeometry();
    /**
     * Resets the clipping planes, i.e. no clipping is performed.
     */
    void resetClippingPlanes();

    void defineBoxBrickingRegion();
    /** reactions for changes on properties */
    void setUseClipping();
    void setLeftClipPlane();
    void setRightClipPlane();
    void setTopClipPlane();
    void setBottomClipPlane();
    void setFrontClipPlane();
    void setBackClipPlane();

    /// clipping
    BoolProperty useClipping_;
    IntProperty clipLeftX_;
    IntProperty clipRightX_;
    IntProperty clipUpY_;
    IntProperty clipDownY_;
    IntProperty clipFrontZ_;
    IntProperty clipBackZ_;
    IntProperty brickSelectionPriority_;

    GLuint dl_;
    BoolProperty useVirtualClipplane_;
    FloatVec4Property clipPlane_;  ///< stored as (n.x, n.y, n.z, d)

    VolumeHandle* currentVolumeHandle_;
    VolumeHandle* oldHandle_;
};

} // namespace

#endif // VRN_CUBEPROXYGEOMETRY_H
