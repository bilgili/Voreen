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

#ifndef VRN_PROXYGEOMETRY_H
#define VRN_PROXYGEOMETRY_H

#include <vector>

#include "tgt/vector.h"
#include "tgt/tgt_gl.h"

#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/volume/volumeatomic.h"

namespace voreen {

/**
 * Abstract base class for all proxy geometries. Derived classes create
 * their proxy geometry.
 * */
class ProxyGeometry : public VolumeRenderer {
public:
    ProxyGeometry();
    virtual ~ProxyGeometry() { }
    virtual const Identifier getClassName() const {return "ProxyGeometry.ProxyGeometry";}
    virtual tgt::vec3 getVolumeSize();

    virtual void setVolumeHandle(VolumeHandle* const handle);


    /**
     * Should called when OpenGL is initialized.
     */
    virtual int initializeGL();

    static const Identifier setUseClipping_;
    static const Identifier setLeftClipPlane_;
    static const Identifier setRightClipPlane_;
    static const Identifier setTopClipPlane_;
    static const Identifier setBottomClipPlane_;
    static const Identifier setFrontClipPlane_;
    static const Identifier setBackClipPlane_;
    static const Identifier resetClipPlanes_;
    static const Identifier getVolumeSize_;

    virtual void render() {}

	virtual void process(LocalPortMapping* portMapping);
	virtual Message* call (Identifier ident, LocalPortMapping* portMapping);
protected:
    bool needsBuild_;
    tgt::vec3 volumeSize_;
    Volume* volume_;
};

/**
 * Provides a simple cube proxy. The resulting geometry depends
 * on the ratio of the values in dim.
 */
class CubeProxyGeometry : public ProxyGeometry {
public:
	CubeProxyGeometry();
    virtual ~CubeProxyGeometry();

	virtual const Identifier getClassName() const {return "ProxyGeometry.CubeProxyGeometry";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new CubeProxyGeometry();}

    /**
     * Handles the Identifier
	 *	   - setUseClipping \n bool
     *     - setLeftClipPlane \c int
     *     - setRightClipPlane \c int
     *     - setTopClipPlane \c int
     *     - setBottomClipPlane \c int
     *     - setLeftFrontPlane \c int
     *     - setLeftBackPlane \c int
     *     - resetClipPlanes
     */
    virtual void processMessage(Message *msg, const Identifier& dest=Message::all_);
    virtual bool getUseVirtualClipplane();
    virtual void setPropertyDestination(Identifier tag);
    
    /**
      *returns (left down front) vector
      */
    virtual tgt::vec3 getClipPlaneLDF();

    /**
      *returns (right up back) vector
      */
    virtual tgt::vec3 getClipPlaneRUB();
    virtual void render();
    
    virtual bool isMultipassCompatible() { return true; }

protected:
    void revalidateCubeGeometry();
    /**
     * Resets the clipping planes, i.e. no clipping is performed.
     */
    void resetClippingPlanes();

    /// clipping
	BoolProp useClipping_;
    IntProp clipLeftX_;
    IntProp clipRightX_;
    IntProp clipUpY_;
    IntProp clipDownY_;
    IntProp clipFrontZ_;
    IntProp clipBackZ_;

	GLuint dl_;
    BoolProp useVirtualClipplane_;
    FloatVec4Prop clipPlane_;  ///< stored as (n.x, n.y, n.z, d)

    GroupProp* clipplaneGroup_;
};

//---------------------------------------------------------------------------

/**
 * Provides a simple cube proxy. The resulting geometry depends
 * on the ratio of the values in dim.
 */
class CubeCutProxyGeometry : public ProxyGeometry {
public:
	CubeCutProxyGeometry();
    virtual ~CubeCutProxyGeometry();

	virtual const Identifier getClassName() const {return "ProxyGeometry.CubeCutProxyGeometry";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new CubeCutProxyGeometry();}


    /**
     * Handles the Identifier
     * ...
	 */
    virtual void processMessage(Message *msg, const Identifier& dest=Message::all_);
    virtual void setPropertyDestination(Identifier tag);

	virtual Message* call(Identifier ident, LocalPortMapping* portMapping);
    
protected:
    virtual void render();
    void revalidateCubeGeometry();
    void renderCubeWithCutting();
    void renderCuttedCube();

    /// clipping cube
	BoolProp cutCube_;
    FloatVec3Prop cubeSize_;
    
	GLuint dl_;
};

} // namespace voreen

#endif // VRN_PROXYGEOMETRY_H
