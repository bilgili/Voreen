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

#include "voreen/core/vis/processors/geometry/pointlistrenderer.h"

#include "voreen/core/geometry/geometrycontainer.h"
#include "voreen/core/geometry/pointlistgeometry.h"
#include "voreen/core/geometry/pointsegmentlistgeometry.h"
#include "voreen/core/vis/voreenpainter.h"

#include "voreen/core/vis/transfunc/transfunc.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

const std::string PointListRenderer::loggerCat_("voreen.PointListRenderer");

/*
    constructor
*/
PointListRenderer::PointListRenderer()
    : GeometryRenderer(),    
      geometry_(0),
      displayList_(0),
      color_("PointListRenderer.Color", "Primitive Color", tgt::Color(0.75f, 0.25f, 0.f, 1.f)),
      depthTest_("PointListRenderer.DepthTest", "Depth Test", true),
      pointSize_("PointListRenderer.PointSize", "Point Size", 3.f, 1.f, 20.f),
      pointSmooth_("PointListRenderer.PointSmooth", "Point Smooth", false),
      sphereDiameter_("PointListRenderer.SphereDiameter", "Sphere Diameter", 0.01f, 0.001f, 0.1f)
{
    setName("PointListRenderer");

    renderingPrimitives_.push_back("Points");
    renderingPrimitives_.push_back("Spheres");
    renderingPrimitives_.push_back("Illuminated Spheres");
    renderingPrimitiveProp_ = new EnumProp("PointListRenderer.renderingPrimitive", "Rendering Primitive", renderingPrimitives_, 0);

    renderingPrimitiveProp_->onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    color_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    depthTest_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    pointSize_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    pointSmooth_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));
    sphereDiameter_.onChange(CallMemberAction<PointListRenderer>(this, &PointListRenderer::invalidateDisplayList));

    pointSize_.setStepping(0.5f);
    pointSize_.setNumDecimals(1);
    sphereDiameter_.setStepping(0.001f);
    sphereDiameter_.setNumDecimals(3); 

    addProperty(renderingPrimitiveProp_);
    addProperty(&color_);
    addProperty(&depthTest_);
    addProperty(&pointSize_);
    addProperty(&pointSmooth_);
    addProperty(&sphereDiameter_);

    createInport("geometry.pointlist");
    createCoProcessorOutport("coprocessor.geometryprocessor", &Processor::call);

}

PointListRenderer::~PointListRenderer() {
    delete renderingPrimitiveProp_;
}

void PointListRenderer::process(LocalPortMapping* portMapping){

    try {
        int pointListID = portMapping->getGeometryNumber("geometry.pointlist");
        geometry_ = geoContainer_->getGeometry(pointListID);
    } 
    catch (std::exception& ) {
        geometry_ = 0;
    }
    
    // invalidate display list on geometry change
    if (!geometry_ || geometry_->hasChanged()) {
        if (glIsList(displayList_))
            glDeleteLists(displayList_, 1);
        displayList_ = 0;
    }

}

void PointListRenderer::render(LocalPortMapping* /*localPortMapping*/) {

    if (!geometry_)
        return;

    // (re)-generated display list from geometry, if list is not present
    if (!glIsList(displayList_)) {
        
        // cast geometry to PointListGeometry or PointSegmentListGeometry and generate display list
        PointListGeometry<vec3>* pointList = dynamic_cast< PointListGeometry<vec3>* >(geometry_);
        PointSegmentListGeometry<vec3>* segmentList = dynamic_cast< PointSegmentListGeometry<vec3>* >(geometry_);
        if (pointList)
            generateDisplayList(pointList->getData());
        else if (segmentList)
            generateDisplayList(segmentList->getPoints());
    
        // message on invalid geometry
        if (!pointList && !segmentList) {
            LWARNING("Invalid geometry. PointListGeometry<vec3> or PointSegmentListGeometry<vec3> expected.");
        }
    }

    // render display list
    if (glIsList(displayList_)) {
        glCallList(displayList_);
    }
}

void PointListRenderer::generateDisplayList(const std::vector<vec3>& pointList) {

    if (glIsList(displayList_))
        glDeleteLists(displayList_, 1);

    displayList_ = glGenLists(1);
    glNewList(displayList_, GL_COMPILE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (!depthTest_.get())
        glDisable(GL_DEPTH_TEST);

    // enable lighting for illuminated spheres
    if (renderingPrimitiveProp_->get() == 2) {

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glLightfv(GL_LIGHT0, GL_AMBIENT, vec4(0.3f, 0.3f, 0.3f, 1.f).elem);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, vec4(0.6f, 0.6f, 0.6f, 1.f).elem);
        glLightfv(GL_LIGHT0, GL_SPECULAR, vec4(0.6f, 0.6f, 0.6f, 1.f).elem);
        glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 128.f);

        tgt::Material material(color_.get(), color_.get(), color_.get(), 75.f);
        material.activate();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glLightfv(GL_LIGHT0, GL_POSITION, vec4(1.f, 1.f, 10.f, 1.f).elem);
        glPopMatrix();
    }

    // render: point primitives
    if (renderingPrimitiveProp_->get() == 0) {
        glColor4fv(color_.get().elem);
        glPointSize(pointSize_.get());
        if (pointSmooth_.get())
            glEnable(GL_POINT_SMOOTH);
        glBegin(GL_POINTS);
        for (size_t i=0; i<pointList.size(); ++i) {
            tgt::vertex(pointList[i]);
        }
        glEnd();
    }
    // render: spheres
    else if (renderingPrimitiveProp_->get() == 1 || renderingPrimitiveProp_->get() == 2) {
        GLUquadricObj* quadric = gluNewQuadric();
        glColor4fv(color_.get().elem);
        for (size_t i=0; i<pointList.size(); ++i) {
            glPushMatrix();      
            tgt::translate(pointList[i]);
            gluSphere(quadric, sphereDiameter_.get(), 20, 20);
            glPopMatrix();
        }
        gluDeleteQuadric(quadric);
    }

    glPopAttrib();
    glEndList();

}


void PointListRenderer::invalidateDisplayList() {
    
    if (glIsList(displayList_))
        glDeleteLists(displayList_, 1);
    displayList_ = 0;
    
    // temporary hack for invalidating the geometry source
    MsgDistr.postMessage( new CameraPtrMsg(VoreenPainter::cameraChanged_, camera_) );
}


} // namespace voreen
