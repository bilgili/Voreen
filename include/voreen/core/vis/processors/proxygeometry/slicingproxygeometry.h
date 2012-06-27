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

#ifndef VRN_SLICINGPROXYGEOMETRY_H
#define VRN_SLICINGPROXYGEOMETRY_H

#ifndef VRN_PROXYGEOMETRY_H
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#endif

#ifndef VRN_POLYGON3D_H
#include "voreen/core/vis/slicing/polygon3d.h"
#endif

namespace voreen
{
/**
 * Processor class which can cut an abitrarily aligned slice of custom thickness from
 * a cubic volume. The slice is defined by a plane given by an equation.
 *
 * NOTE: slicing is performed by cutting a generic cube of size [2, 2, 2] ranking from
 * [-1, -1, -1] to [1, 1, 1] with a plane given by its normal and a point on it. The
 * code (in <code>buildProxyGeometry()</code> and the classes used there is very complicated
 * and sensitive to changes, so please be sure to know what you do when changing it or
 * ask about it! Thank you!
 *
 * @author  Dirk Feldmann, 2008 - 2009 A.D.
 *
 */
class SlicingProxyGeometry : public ProxyGeometry
{
public:
    SlicingProxyGeometry();
    ~SlicingProxyGeometry();

    virtual const Identifier getClassName() const { return "ProxyGeometry.SlicingProxyGeometry"; }
    virtual Processor* create() const { return new SlicingProxyGeometry(); }

    virtual void process(LocalPortMapping* portMapping);
    virtual const std::string getProcessorInfo() const;

    /**
     * Called from somewhere in the evaluation step of the network and implements
     * a kind of "call-back" mechanism. Used to call rendering method
     * <code>renderDisplayList()</code>
     */
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

    /**
     * Performs rendering of the proxy geometry. If changes to volume or slice have been
     * made previously, this method initializes the rebuilding of the geometry. It is called
     * by passing "render" to <code>call()</code>.
     */
    virtual void renderDisplayList();

    /**
     * Sets the clipping plane to the given one and sets an indicator that the proxy
     * geometry needs to be rebuild.
     * Usually this is set the processors property but if you need to modify the plane
     * "from outside" or if you need to reset it, use this method to ensure that all
     * necessary operations to keep the geometry valid will be performed.
     */
    void setClipPlane(const tgt::vec4& plane);

    /**
     * Sets the thickness of the sliceto the given one and sets an indicator that the
     * proxy geometry needs to be rebuild.
     * Usually this is set the processors property but if you need to modify the plane
     * "from outside" or if you need to reset it, use this method to ensure that all
     * necessary operations to keep the geometry valid will be performed.
     */
    void setSliceThickness(const float thickness);

    /**
     * Returns the plane currently used for clipping the volume.
     */
    tgt::vec4& getClipPlane() { return clipPlane_; }

    /**
     * Returns the thickness of the slice currently used for clipping the volume.
     */
    float getSliceThickness() const { return sliceThickness_; }

    /**
     * Returns the number of slices currently emerging form slice's thickness, not
     * yet regarding the direction of the plane's normal. Therefore the number is
     * calculated for the worst case which means that the plane is aligned along the
     * diagonal of the volume's bouding box.
     */
    int getNumSlices() const { return numSlices_; }

protected:
    /**
     * Called by clipPlaneProp_ when onChange() action is triggered.
     * Performs changes which are affected the clip plane settings.
     */
    void onClipPlaneChange();

    /**
     * Called by sliceThicknessProp_;
     * Performs changes being affected by changing the slice thickness.
     */
    void onSliceThicknessChange();

protected:
    int displayList_;           /** OpenGL display list*/
    Polygon3D* cubeProxy_;      /** Proxy geometry representing volume's bounding box */
    Polygon3D* proxyGeometry_;  /** The clipped proxy geometry */
    tgt::vec4 clipPlane_;       /** The clipping plane given as an equation */
    float sliceThickness_;      /** Thickness of the slice which shall be cut */
    int numSlices_;             /** Number of slices available according to thickness of one slice */

    FloatVec4Prop* clipPlaneProp_;
    FloatProp* sliceThicknessProp_;

private:
    /**
     * Sets up the basic proxy geometry (a cube) which serves as the
     * basis for clipping.
     */
    void buildCubeProxyGeometry();

    /**
     * Perfoms the composition of the an OpenGL display list which holds
     * rendering data for the clipped proxygeometry.
     */
    void buildDisplayList();

    /**
     * Performs the actual clipping of the cubic proxy geometry.
     */
    void buildProxyGeometry();

    /**
     * Calculates the number of slices being available based on the slice thickness not
     * yet regarding the clipping plane's normal.
     */
    int calculateNumSlices() const;
};

}   // namespace

#endif
