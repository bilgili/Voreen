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

#include "voreen/core/vis/processors/proxygeometry/slicingproxygeometry.h"
#include "tgt/camera.h"

namespace voreen {

SlicingProxyGeometry::SlicingProxyGeometry()
    : ProxyGeometry()
    , displayList_(0)
    , cubeProxy_(0)
    , proxyGeometry_(0)
    , clipPlane_(1.0f, 0.0f, 0.0f, 0.1f)
    , sliceThickness_(0.1f)
    , numSlices_(0)
    , clipPlaneProp_(0)
    , sliceThicknessProp_(0)
{

    clipPlaneProp_ = new FloatVec4Property("clipPlaneProp", "plane equation: ", clipPlane_,
        tgt::vec4(-2.0f), tgt::vec4(2.0f));
    clipPlaneProp_->setStepping(tgt::vec4(0.1f));
    clipPlaneProp_->onChange(
        CallMemberAction<SlicingProxyGeometry>(this, &SlicingProxyGeometry::onClipPlaneChange));

    sliceThicknessProp_ = new FloatProperty("sliceThicknessProp", "slice thickness: ", sliceThickness_,
        0.0f, 1.0f);
    sliceThicknessProp_->setStepping(0.05f);
    sliceThicknessProp_->onChange(
        CallMemberAction<SlicingProxyGeometry>(this, &SlicingProxyGeometry::onSliceThicknessChange));

    addProperty(clipPlaneProp_);
    addProperty(sliceThicknessProp_);
}

SlicingProxyGeometry::~SlicingProxyGeometry() {
    delete cubeProxy_;
    cubeProxy_ = 0;

    delete proxyGeometry_;
    proxyGeometry_ = 0;

    delete clipPlaneProp_;
    clipPlaneProp_ = 0;

    delete sliceThicknessProp_;
    sliceThicknessProp_ = 0;
}

const std::string SlicingProxyGeometry::getProcessorInfo() const {
    return "Provides a proxy geometry based on a cube which can be cut arbitrarily. \
           The resulting geometry depends on the ratio of the values in dim.";
}

void SlicingProxyGeometry::process() {

    if (!inport_.isReady() || !inport_.hasChanged())
        return;

    // the volume has changed and is not NULL, so the proxy geometry needs to be
    // rebuilt
    //
    volume_ = inport_.getData()->getVolume();
    volumeSize_ = volume_->getCubeSize() / 2.0f;
    numSlices_ = calculateNumSlices();

    // adjust maximum slice thickness for wort case:
    // the slice is the entire volume along its diagonal
    //
    if ( sliceThicknessProp_ != 0 ) {
        float len = tgt::length((volumeSize_ * 2.0f));
        sliceThicknessProp_->setMinValue(0.0f);
        sliceThicknessProp_->setMaxValue(len);
    }

    buildCubeProxyGeometry();
    needsBuild_ = true;
}

void SlicingProxyGeometry::renderDisplayList() {
    if (volume_ == 0)
        return;

    if (needsBuild_ == true) {
        if (displayList_ <= 0)
            displayList_ = glGenLists(1);

        buildDisplayList();
        needsBuild_ = false;
    }
    glCallList(displayList_);
}

void SlicingProxyGeometry::setClipPlane(const tgt::vec4& plane) {
    if (clipPlane_ != plane) {
        clipPlane_ = plane;

        if (clipPlaneProp_ != 0)
            clipPlaneProp_->set(clipPlane_);
    }

    needsBuild_ = true;
}

void SlicingProxyGeometry::setSliceThickness(const float thickness) {
    if (thickness != sliceThickness_) {
        sliceThickness_ = thickness;
        numSlices_ = calculateNumSlices();

        if (sliceThicknessProp_ != 0)
            sliceThicknessProp_->set(sliceThickness_);
    }
    needsBuild_ = true;
}

// protected methods for reaction on property changes
//

void SlicingProxyGeometry::onClipPlaneChange() {
    if (clipPlaneProp_ != 0)
        setClipPlane(clipPlaneProp_->get());
}

void SlicingProxyGeometry::onSliceThicknessChange() {
    if (sliceThicknessProp_ != 0)
        setSliceThickness(sliceThicknessProp_->get());
}

// private methods
//

void SlicingProxyGeometry::buildDisplayList() {
    buildProxyGeometry();

    if (proxyGeometry_ == 0)
        return;

    // recreate display list
    //
    if (displayList_ > 0)
        glDeleteLists(displayList_, 1);

    glNewList(displayList_, GL_COMPILE);
    const PolygonFace3D::FaceSet& faces = proxyGeometry_->getFaces();
    PolygonFace3D::FaceSet::const_iterator itFaces = faces.begin();

    for ( ; itFaces != faces.end(); ++itFaces) {
        PolygonFace3D* face = *itFaces;
        const std::list<tgt::vec3>& v = face->getVertices();
        std::list<tgt::vec3>::const_iterator it = v.begin();
        glBegin(GL_POLYGON);

        for ( ; it != v.end(); ++it) {
            glTexCoord3fv(vertexToTexCoord(*it).elem);
            glVertex3fv(it->elem);
        }

        glEnd();
    }
    glEndList();
}

void SlicingProxyGeometry::buildCubeProxyGeometry() {
    delete cubeProxy_;
    cubeProxy_ = new Polygon3D();

    tgt::vec3 llf = -volumeSize_;
    tgt::vec3 urb = volumeSize_;

    // 8 vertices for the cube
    //
    tgt::vec3 cubeVertices[8] = {
        tgt::vec3(llf.x, llf.y, urb.z),    // 0, left lower back
        tgt::vec3(llf.x, llf.y, llf.z),    // 1, left lower front
        tgt::vec3(urb.x, llf.y, llf.z),    // 2, right lower front
        tgt::vec3(urb.x, llf.y, urb.z),    // 3, right lower back
        tgt::vec3(llf.x, urb.y, urb.z),    // 4, left upper back
        tgt::vec3(llf.x, urb.y, llf.z),    // 5, left upper front
        tgt::vec3(urb.x, urb.y, llf.z),    // 6, right upper front
        tgt::vec3(urb.x, urb.y, urb.z)     // 7, right upper back
    };

    // indices for the 6 sides of the cube. the side
    // consists of the line segments defined by a
    // successive pair of indices for the vertices above.
    //
    unsigned long top[8] = {4, 5, 5, 6, 6, 7, 7, 4};
    unsigned long bottom[8] = {0, 3, 3, 2, 2, 1, 1, 0};
    unsigned long front[8] = {0, 4, 4, 7, 7, 3, 3, 0};
    unsigned long back[8] = {1, 2, 2, 6, 6, 5, 5, 1};
    unsigned long left[8] = {0, 1, 1, 5, 5, 4, 4, 0};
    unsigned long right[8] = {2, 3, 3, 7, 7, 6, 6, 2};

    cubeProxy_->addFace(cubeVertices, 8, bottom, 8, 0, tgt::vec3(0.0f, -1.0f, 0.0f));
    cubeProxy_->addFace(cubeVertices, 8, top, 8, 1, tgt::vec3(0.0f, 1.0f, 0.0f));
    cubeProxy_->addFace(cubeVertices, 8, front, 8, 2, tgt::vec3(0.0f, 0.0f, 1.0f));
    cubeProxy_->addFace(cubeVertices, 8, back, 8, 3, tgt::vec3(0.0f, 0.0f, -1.0f));
    cubeProxy_->addFace(cubeVertices, 8, left, 8, 4, tgt::vec3(-1.0f, 0.0f, 0.0f));
    cubeProxy_->addFace(cubeVertices, 8, right, 8, 5, tgt::vec3(1.0f, 0.0f, 0.0f));
}

void SlicingProxyGeometry::buildProxyGeometry() {
    if (cubeProxy_ == 0)
        return;

    // create a new proxy geometry based on the cube proxy
    // stored in cubeProxy_
    //
    delete proxyGeometry_;
    proxyGeometry_ = new Polygon3D(*cubeProxy_);

    if (clipPlane_.xyz() == tgt::vec3(0.0f))
        return;

    // clip the proxy against the plane defined by the equation
    // in clipPlane_ if it is not null
    //
    tgt::vec3 normal(clipPlane_.xyz());
    normal = tgt::normalize(normal);
    tgt::vec3 pos(0.0f);
    if (clipPlane_.x != 0.0f)
        pos.x = clipPlane_.w;
    else if (clipPlane_.y != 0.0f)
        pos.y = clipPlane_.w;
    else
        pos.z = clipPlane_.w;
    proxyGeometry_->clip(normal, pos);

    // invert normal and adjust position according to slice thickness
    //
    normal *= -1.0f;
    pos += (normal * sliceThickness_);
    proxyGeometry_->clip(normal, pos);
    needsBuild_ = true;
}

inline int SlicingProxyGeometry::calculateNumSlices() const {
    // FIXME: this calculates only the worst case for the length:
    // a plane might be aligned along the diagonal axis of volume
    //
    float len = tgt::length((volumeSize_ * 2.0f));
    return static_cast<int>( ceilf((len / sliceThickness_)) );
}

}   // namespace
