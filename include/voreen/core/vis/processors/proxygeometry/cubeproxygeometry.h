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

/**
 * Provides a simple cube proxy with clipping.
 */
class CubeProxyGeometry : public ProxyGeometry {
public:
    CubeProxyGeometry();
    virtual ~CubeProxyGeometry();

    virtual const Identifier getClassName() const { return "ProxyGeometry.CubeProxyGeometry"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new CubeProxyGeometry(); }

    /**
     * Handles the Identifier
     *       - setUseClipping \n bool
     *     - setLeftClipPlane \c int
     *     - setRightClipPlane \c int
     *     - setTopClipPlane \c int
     *     - setBottomClipPlane \c int
     *     - setLeftFrontPlane \c int
     *     - setLeftBackPlane \c int
     *     - resetClipPlanes
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
    virtual bool getUseVirtualClipplane();

    /**
     * returns (left down front) vector
     */
    virtual tgt::vec3 getClipPlaneLDF();

    /**
     * returns (right up back) vector
     */
    virtual tgt::vec3 getClipPlaneRUB();
    virtual void render();

protected:
    void revalidateCubeGeometry();
    /**
     * Resets the clipping planes, i.e. no clipping is performed.
     */
    void resetClippingPlanes();

    void defineBoxBrickingRegion();

    /// clipping
    BoolProp useClipping_;
    IntProp clipLeftX_;
    IntProp clipRightX_;
    IntProp clipUpY_;
    IntProp clipDownY_;
    IntProp clipFrontZ_;
    IntProp clipBackZ_;
    EnumProp* brickSelectionProp_;
    std::vector<std::string> brickSelectionPropValues_;
    IntProp brickSelectionPriority_;

    GLuint dl_;
    BoolProp useVirtualClipplane_;
    FloatVec4Prop clipPlane_;  ///< stored as (n.x, n.y, n.z, d)

    /** reactions for changes on properties */
    void setUseClipping();
    void setLeftClipPlane();
    void setRightClipPlane();
    void setTopClipPlane();
    void setBottomClipPlane();
    void setFrontClipPlane();
    void setBackClipPlane();

    void markAsChanged();
};

} // namespace

#endif // VRN_CUBEPROXYGEOMETRY_H
