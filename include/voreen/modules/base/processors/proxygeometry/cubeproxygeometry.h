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

#ifndef VRN_CUBEPROXYGEOMETRY_H
#define VRN_CUBEPROXYGEOMETRY_H

#include "voreen/modules/base/processors/proxygeometry/proxygeometry.h"

namespace voreen {

/**
 * Provides a simple cube proxy with clipping.
 */
class CubeProxyGeometry : public ProxyGeometry {
public:
    CubeProxyGeometry();
    virtual ~CubeProxyGeometry();

    virtual std::string getClassName() const    { return "CubeProxyGeometry"; }
    virtual std::string getCategory() const     { return "Volume Proxy Geometry"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const { return new CubeProxyGeometry(); }

    virtual void render();

protected:
    virtual void process();
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
    void setFrontClipPlane();
    void setBackClipPlane();
    void setBottomClipPlane();
    void setTopClipPlane();

    /// clipping
    BoolProperty useClipping_;
    FloatProperty clipLeftX_;
    FloatProperty clipRightX_;
    FloatProperty clipFrontY_;
    FloatProperty clipBackY_;
    FloatProperty clipBottomZ_;
    FloatProperty clipTopZ_;
    IntProperty brickSelectionPriority_;

    GLuint displayList_;

    VolumeHandle* currentVolumeHandle_;
    VolumeHandle* oldHandle_;
};

} // namespace

#endif // VRN_CUBEPROXYGEOMETRY_H
