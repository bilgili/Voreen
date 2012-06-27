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

#ifndef VRN_SLICINGPROXYGEOMETRY_H
#define VRN_SLICINGPROXYGEOMETRY_H

#ifndef VRN_PROXYGEOMETRY_H
#include "voreen/core/vis/processors/render/proxygeometry.h"
#endif

#ifndef VRN_POLYGON3D_H
#include "voreen/core/vis/slicing/polygon3d.h"
#endif

namespace voreen
{

class SlicingProxyGeometry : public ProxyGeometry
{
public:
    static const Identifier msgSetClipPlane_;
    static const Identifier msgSetSliceThickness_;

    SlicingProxyGeometry();
    ~SlicingProxyGeometry();

    virtual const Identifier getClassName() const {return "ProxyGeometry.SlicingProxyGeometry";}
    virtual Processor* create() { return new SlicingProxyGeometry(); }
    virtual bool isMultipassCompatible() { return true; }

	virtual const std::string getProcessorInfo() const;
    virtual void processMessage(Message *msg, const Identifier& dest=Message::all_ );
    virtual void process(LocalPortMapping* portMapping);
    virtual void setVolumeHandle(VolumeHandle* const handle);

    virtual Message* call (Identifier ident, LocalPortMapping* portMapping);
    virtual void renderDisplayList();

    /**
     * Renders the proxy geometry.
     *
     * @param   faces   "true" indicates rendering of front faces, "false" indicates
     *                  rendering of back faces
     * @param   renderBoth  if "true", the value of faces is ignored and front and
     *                      back faces are rendered
     */
    virtual void render(const bool renderFrontFaces = true);

    tgt::vec4& getClipPlane();
    void setClipPlane(const tgt::vec4& plane);
    float getSliceThickness() const;
    void setSliceThickness(const float thickness);

protected:
    int displayList_;
    Polygon3D* cubeProxy_;
    Polygon3D* proxyGeometry_;
    tgt::vec4 clipPlane_;
    float sliceThickness_;

    // properties
    //
    FloatVec4Prop* clipPlaneProp_;
    FloatProp* sliceThicknessProp_;

private:
    void buildCubeProxyGeometry();
    void buildDisplayList();
    void buildProxyGeometry();
};

}   // namespace

#endif
