/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "pointlistrenderer.h"

#include "voreen/core/datastructures/geometry/pointlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointsegmentlistgeometry.h"
#include "voreen/core/utils/voreenpainter.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"

#include "tgt/material.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

const std::string PointListRenderer::loggerCat_("voreen.PointListRenderer");

PointListRenderer::PointListRenderer()
    : GeometryRendererBase()
    , displayList_(0)
    , coordinateSystem_("coordinateSystem", "Coordinate System")
    , renderingPrimitiveProp_("renderingPrimitive", "Rendering Primitive")
    , color_("color", "Primitive Color", tgt::Color(0.75f, 0.25f, 0.f, 1.f))
    , depthTest_("depthTest", "Depth Test", true)
    , pointSize_("pointSize", "Point Size", 3.f, 1.f, 20.f)
    , pointSmooth_("pointSmooth", "Point Smooth", false)
    , pointDistAttenuation_("pointDistAttenuation", "Point Distance Attenuation", tgt::vec3(1.f, 0.f, 0.f), tgt::vec3(0.f), tgt::vec3(10.f))
    , sphereDiameter_("sphereDiameter", "Sphere Diameter", 0.01f, 0.001f, 0.1f)
    , sphereSlicesStacks_("sphereSlicesStacks", "Sphere Slices/Stacks", 20, 10, 100)
    , geometryInport_(Port::INPORT, "geometry.input", "Geometry Input")
{
    color_.setViews(Property::COLOR);
    // Coordinate systems
    coordinateSystem_.addOption("world", "World coordinates");
    coordinateSystem_.addOption("viewport", "Viewport coordinates");
    coordinateSystem_.addOption("untransformed", "No transformation");

    // Rendering primitives
    renderingPrimitiveProp_.addOption("points", "Points");
    renderingPrimitiveProp_.addOption("line-strip", "Line Strip");
    renderingPrimitiveProp_.addOption("spheres", "Spheres");
    renderingPrimitiveProp_.addOption("illuminated-spheres", "Illuminated Spheres");

    coordinateSystem_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    renderingPrimitiveProp_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    color_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    depthTest_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    pointSize_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    pointSmooth_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    pointDistAttenuation_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    sphereDiameter_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    sphereSlicesStacks_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));

    pointSize_.setStepping(0.5f);
    pointSize_.setNumDecimals(1);
    sphereDiameter_.setStepping(0.001f);
    sphereDiameter_.setNumDecimals(3);

    addProperty(coordinateSystem_);
    addProperty(renderingPrimitiveProp_);
    addProperty(color_);
    addProperty(depthTest_);
    addProperty(pointSize_);
    addProperty(pointSmooth_);
    addProperty(pointDistAttenuation_);
    addProperty(sphereDiameter_);
    addProperty(sphereSlicesStacks_);

    addPort(geometryInport_);
}

void PointListRenderer::process() {
    tgtAssert(geometryInport_.isReady(), "inport not ready");
    // force display list update, if input has changed
    if (geometryInport_.hasChanged())
        invalidateDisplayList();
}

void PointListRenderer::deinitialize() throw (VoreenException) {
    if (glIsList(displayList_))
        glDeleteLists(displayList_, 1);

    GeometryRendererBase::deinitialize();
}

void PointListRenderer::render() {

    tgtAssert(geometryInport_.isReady(), "inport not ready");

    // regenerate display list, if input data has changed
    if (geometryInport_.hasChanged() || displayList_ == 0) {

        if (glIsList(displayList_))
            glDeleteLists(displayList_, 1);
        displayList_ = 0;

        // cast geometry to PointListGeometry or PointSegmentListGeometry and generate display list
        const PointListGeometry<vec3>* pointList = dynamic_cast<const  PointListGeometry<vec3>* >(geometryInport_.getData());
        const PointSegmentListGeometry<vec3>* segmentList = dynamic_cast<const  PointSegmentListGeometry<vec3>* >(geometryInport_.getData());
        if (pointList)
            generateDisplayList(pointList->getData(), pointList->getTransformationMatrix());
        else if (segmentList)
            generateDisplayList(segmentList->getPoints(), segmentList->getTransformationMatrix());

        // message on invalid geometry
        if (!pointList && !segmentList) {
            LWARNING("Invalid geometry. PointListGeometry<vec3> or PointSegmentListGeometry<vec3> expected.");
        }
    }

    // render display list
    if (glIsList(displayList_)) {
        if(renderingPrimitiveProp_.get() == "points"){
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, (GLfloat*)(pointDistAttenuation_.get().elem));
        }
        glCallList(displayList_);
        if(renderingPrimitiveProp_.get() == "points"){
            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, (GLfloat*)(tgt::vec3(1.f,0.f,0.f).elem));
        }
    }

    glBegin(GL_POINTS);
    glVertex3f(0.f,0.f,0.f);
    glEnd();

}

void PointListRenderer::generateDisplayList(const std::vector<vec3>& pointList, const tgt::mat4 m) {

    if (glIsList(displayList_))
        glDeleteLists(displayList_, 1);

    displayList_ = glGenLists(1);
    glNewList(displayList_, GL_COMPILE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (coordinateSystem_.get() != "world") {
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.pushMatrix();
        MatStack.loadIdentity();
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.loadIdentity();
        if (coordinateSystem_.get() == "viewport") {
            MatStack.translate(-1.f, -1.f, 0.f);
            MatStack.scale(2.f/viewport_.x, 2.f/viewport_.y, 1.f);
        }
    }

    if (!depthTest_.get())
        glDisable(GL_DEPTH_TEST);

    // enable lighting for illuminated spheres
    if (renderingPrimitiveProp_.get() == "illuminated-spheres") {

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glLightfv(GL_LIGHT0, GL_AMBIENT, vec4(0.3f, 0.3f, 0.3f, 1.f).elem);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, vec4(0.6f, 0.6f, 0.6f, 1.f).elem);
        glLightfv(GL_LIGHT0, GL_SPECULAR, vec4(0.6f, 0.6f, 0.6f, 1.f).elem);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 128.f);

        tgt::Material material(color_.get(), color_.get(), color_.get(), 75.f);
        material.activate();

        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.loadIdentity();
        glLightfv(GL_LIGHT0, GL_POSITION, vec4(1.f, 1.f, 10.f, 1.f).elem);
        MatStack.popMatrix();
    }


    // render: point primitives
    if (renderingPrimitiveProp_.get() == "points") {
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.multMatrix(m);

        glColor4fv(color_.get().elem);
        glPointSize(pointSize_.get());
        if (pointSmooth_.get())
            glEnable(GL_POINT_SMOOTH);
        glBegin(GL_POINTS);
        for (size_t i=0; i<pointList.size(); ++i) {
            tgt::vertex(pointList[i]);
        }
        glEnd();

        MatStack.popMatrix();
    }

    // render: line strip
    if (renderingPrimitiveProp_.get() == "line-strip") {
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.multMatrix(m);

        glColor4fv(color_.get().elem);
        glLineWidth(pointSize_.get());
        if (pointSmooth_.get())
            glEnable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_STRIP);
        for (size_t i=0; i<pointList.size(); ++i) {
            tgt::vertex(pointList[i]);
        }
        glEnd();

        MatStack.popMatrix();
    }

    // render: spheres
    else if (renderingPrimitiveProp_.get() == "spheres" || renderingPrimitiveProp_.get() == "illuminated-spheres") {
        GLUquadricObj* quadric = gluNewQuadric();
        glColor4fv(color_.get().elem);
        for (size_t i=0; i<pointList.size(); ++i) {
            MatStack.pushMatrix();
            MatStack.translate(m * pointList[i]);
            gluSphere(quadric, sphereDiameter_.get(), sphereSlicesStacks_.get(), sphereSlicesStacks_.get());
            MatStack.popMatrix();
        }
        gluDeleteQuadric(quadric);
    }

    if (coordinateSystem_.get() != "world") {
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.popMatrix();
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.popMatrix();
    }

    glPopAttrib();
    glEndList();
}

void PointListRenderer::invalidateDisplayList() {

    if (!isInitialized())
        return;

    if (glIsList(displayList_))
        glDeleteLists(displayList_, 1);
    displayList_ = 0;

}

} // namespace voreen
