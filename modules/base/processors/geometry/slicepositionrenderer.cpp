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

#include "slicepositionrenderer.h"

#include "tgt/glmath.h"
#include "tgt/quadric.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

SlicePositionRenderer::SlicePositionRenderer()
    : GeometryRendererBase()
    //port
    , inport_(Port::INPORT, "volume", "Volume Input")
    //properties
    , enable_("enable", "Enable", true)
        //x slice
    , renderXSlice_("renderXSlice", "Render X Slice", true)
    , xSliceIndexProp_("xSliceIndex", "X Slice Number", 0, 0, 10000)
    , xColor_("xColor", "X Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , lineXWidth_("lineXWidth","Line X Width",1.f,1.f,10.f)
    , alphaFactorXPlane_("alphaFactorXPlane"," X Plane Alpha", 0.f, 0.f, 1.f)
        //y slice
    , renderYSlice_("renderYSlice", "Render Y Slice", true)
    , ySliceIndexProp_("ySliceIndex", "Y Slice Number", 0, 0, 10000)
    , yColor_("yColor", "Y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , lineYWidth_("lineYWidth","Line Y Width",1.f,1.f,10.f)
    , alphaFactorYPlane_("alphaFactorYPlane"," Y Plane Alpha", 0.f, 0.f, 1.f)
        //z slice
    , renderZSlice_("renderZSlice", "Render Z Slice", true)
    , zSliceIndexProp_("zSliceIndex", "Z Slice Number", 0, 0, 10000)
    , lineZWidth_("lineZWidth","Line Z Width",1.f,1.f,10.f)
    , zColor_("zColor", "Z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , alphaFactorZPlane_("alphaFactorZPlane"," Z Plane Alpha", 0.f, 0.f, 1.f)
    // position
    , renderSphere_("renderSphere", "Render Voxel Position", false)
    , sphereColor_("sphereColor", "Sphere Color", tgt::vec4(1.0f, 1.0f, 0.0f, 1.0f))
    , sphereRadius_("sphereRadius", "Sphere Radius",0.01f,0.001f,0.1f)
    , sphereDepthTest_("sphereDepthTest","Depth Test", false)
    , sphereXPos_("sphereXPos", "X Voxel", 0, -1, 10000)
    , sphereYPos_("sphereYPos", "Y Voxel", 0, -1, 10000)
    , sphereZPos_("sphereZPos", "Z Voxel", 0, -1, 10000)
        // deprecated
    , stippleFactor_("deprecatedStippleFactor", "Stipple Factor", 1, 0, 255)
    , stipplePattern_("deprecatedStipplePattern", "Stipple Pattern", 65535, 1,65535)
{
    //ports
    addPort(inport_);
    //properties
    addProperty(enable_);
        // x slice
    addProperty(renderXSlice_);
    renderXSlice_.setGroupID("x");
    addProperty(xSliceIndexProp_);
    xSliceIndexProp_.setGroupID("x");
    addProperty(xColor_);
    xColor_.setGroupID("x");
    addProperty(lineXWidth_);
    lineXWidth_.setGroupID("x");
    addProperty(alphaFactorXPlane_);
    alphaFactorXPlane_.setGroupID("x");
    setPropertyGroupGuiName("x","X Slice");
        // y slice
    addProperty(renderYSlice_);
    renderYSlice_.setGroupID("y");
    addProperty(ySliceIndexProp_);
    ySliceIndexProp_.setGroupID("y");
    addProperty(yColor_);
    yColor_.setGroupID("y");
    addProperty(lineYWidth_);
    lineYWidth_.setGroupID("y");
    addProperty(alphaFactorYPlane_);
    alphaFactorYPlane_.setGroupID("y");
    setPropertyGroupGuiName("y","Y Slice");
        // z slice
    addProperty(renderZSlice_);
    renderZSlice_.setGroupID("z");
    addProperty(zSliceIndexProp_);
    zSliceIndexProp_.setGroupID("z");
    addProperty(zColor_);
    zColor_.setGroupID("z");
    addProperty(lineZWidth_);
    lineZWidth_.setGroupID("z");
    addProperty(alphaFactorZPlane_);
    alphaFactorZPlane_.setGroupID("z");
    setPropertyGroupGuiName("z","Z Slice");
        //position
    addProperty(renderSphere_);
    renderSphere_.setGroupID("pos");
    addProperty(sphereColor_);
    sphereColor_.setGroupID("pos");
    addProperty(sphereRadius_);
    sphereRadius_.setGroupID("pos");
    addProperty(sphereDepthTest_);
    sphereDepthTest_.setGroupID("pos");
    addProperty(sphereXPos_);
    sphereXPos_.setGroupID("pos");
    addProperty(sphereYPos_);
    sphereYPos_.setGroupID("pos");
    addProperty(sphereZPos_);
    sphereZPos_.setGroupID("pos");
    setPropertyGroupGuiName("pos","Voxel Position");
        //deprecated
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);

    //set property mode
    xColor_.setViews(Property::COLOR);
    yColor_.setViews(Property::COLOR);
    zColor_.setViews(Property::COLOR);
    sphereColor_.setViews(Property::COLOR);

    sphereRadius_.setNumDecimals(3);

    stippleFactor_.setVisible(false);
    stipplePattern_.setVisible(false);

    ON_PROPERTY_CHANGE(renderXSlice_,SlicePositionRenderer,togglePropertyVisibility);
    ON_PROPERTY_CHANGE(renderYSlice_,SlicePositionRenderer,togglePropertyVisibility);
    ON_PROPERTY_CHANGE(renderZSlice_,SlicePositionRenderer,togglePropertyVisibility);
    ON_PROPERTY_CHANGE(renderSphere_,SlicePositionRenderer,togglePropertyVisibility);
    togglePropertyVisibility();
}

void SlicePositionRenderer::process() {}

void SlicePositionRenderer::render() {
    if (!inport_.isReady() || !enable_.get())
        return;

    MatStack.pushMatrix();
    MatStack.multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    tgt::vec3 geomLlf = inport_.getData()->getLLF();
    tgt::vec3 geomUrb = inport_.getData()->getURB();
    tgt::vec3 sp = inport_.getData()->getSpacing();

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(stippleFactor_.get(), stipplePattern_.get());

    tgt::ivec3 numSlices = inport_.getData()->getDimensions();

    // We want our slice to be in the center of voxels
    float xSlice = (((float) xSliceIndexProp_.get() + 0.5f) * sp.x) + geomLlf.x;
    float ySlice = (((float) ySliceIndexProp_.get() + 0.5f) * sp.y) + geomLlf.y;
    float zSlice = (((float) zSliceIndexProp_.get() + 0.5f) * sp.z) + geomLlf.z;

    // Voxel Position
    if (renderSphere_.get() && sphereColor_.get().a > 0.f &&
        sphereXPos_.get() >= 0 && sphereYPos_.get() >= 0 && sphereZPos_.get() >= 0)
    {
        tgt::vec3 spherePos = ((tgt::vec3((float)sphereXPos_.get(),(float)sphereYPos_.get(),(float)sphereZPos_.get()) + tgt::vec3(0.5f)) * sp) + geomLlf;

        // calc radius
        tgt::Sphere sphere(tgt::length(geomUrb - geomLlf)*sphereRadius_.get(),36,16);
        GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
        GLfloat mat_shininess[] = { 1 };//100.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT, sphereColor_.get().elem);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, sphereColor_.get().elem);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec4(camera_.getPosition(),0.f).elem);

        MatStack.translate(spherePos.x,spherePos.y,spherePos.z);
        if(!sphereDepthTest_.get())
            glDepthRange(0.f,0.f); //pushes sphere to the front
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_CULL_FACE);
        sphere.render();
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);
        glDepthRange(0.f,1.f);
        MatStack.translate(-spherePos.x,-spherePos.y,-spherePos.z);
    }

    if(lineXWidth_.get() != 1.f || lineYWidth_.get() != 1.f || lineZWidth_.get() != 1.f)
        glEnable(GL_LINE_SMOOTH);

    // X Slice
    if (xColor_.get().a > 0.f && renderXSlice_.get()) {
        glColor4fv(xColor_.get().elem);
        glLineWidth(lineXWidth_.get());
        glBegin(GL_LINE_LOOP);
        glVertex3f(xSlice, geomUrb.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomLlf.z);
        glVertex3f(xSlice, geomUrb.y, geomLlf.z);
        glEnd();

        if (alphaFactorXPlane_.get() > 0.f) {
            glColor4f(xColor_.get().r, xColor_.get().g, xColor_.get().b, xColor_.get().a * alphaFactorXPlane_.get());
            glBegin(GL_QUADS);
            glVertex3f(xSlice, geomUrb.y, geomUrb.z);
            glVertex3f(xSlice, geomLlf.y, geomUrb.z);
            glVertex3f(xSlice, geomLlf.y, geomLlf.z);
            glVertex3f(xSlice, geomUrb.y, geomLlf.z);
            glEnd();
        }
    }
    // Y Slice
    if (yColor_.get().a > 0.f && renderYSlice_.get()) {
        glColor4fv(yColor_.get().elem);
        glLineWidth(lineYWidth_.get());
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, ySlice, geomLlf.z);
        glVertex3f(geomLlf.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomLlf.z);
        glEnd();

        if (alphaFactorYPlane_.get() > 0.f) {
            glColor4f(yColor_.get().r, yColor_.get().g, yColor_.get().b, yColor_.get().a * alphaFactorYPlane_.get());
            glBegin(GL_QUADS);
            glVertex3f(geomLlf.x, ySlice, geomLlf.z);
            glVertex3f(geomLlf.x, ySlice, geomUrb.z);
            glVertex3f(geomUrb.x, ySlice, geomUrb.z);
            glVertex3f(geomUrb.x, ySlice, geomLlf.z);
            glEnd();
        }
    }
    // Z Slice
    if (zColor_.get().a > 0.f && renderZSlice_.get()) {
        glColor4fv(zColor_.get().elem);
        glLineWidth(lineZWidth_.get());
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, geomUrb.y, zSlice);
        glVertex3f(geomLlf.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomUrb.y, zSlice);
        glEnd();

        if (alphaFactorZPlane_.get() > 0.f) {
            glColor4f(zColor_.get().r, zColor_.get().g, zColor_.get().b, zColor_.get().a * alphaFactorZPlane_.get());
            glBegin(GL_QUADS);
            glVertex3f(geomLlf.x, geomUrb.y, zSlice);
            glVertex3f(geomLlf.x, geomLlf.y, zSlice);
            glVertex3f(geomUrb.x, geomLlf.y, zSlice);
            glVertex3f(geomUrb.x, geomUrb.y, zSlice);
            glEnd();
        }
    }

    glDisable(GL_LINE_SMOOTH);

    MatStack.popMatrix();
    glPopAttrib();
}

void SlicePositionRenderer::invalidate(int inv) {
    GeometryRendererBase::invalidate(inv);

    if (inport_.hasChanged() && inport_.hasData()) {
        tgt::ivec3 numSlices = inport_.getData()->getDimensions();

        xSliceIndexProp_.setMaxValue(numSlices.x-1);
        ySliceIndexProp_.setMaxValue(numSlices.y-1);
        zSliceIndexProp_.setMaxValue(numSlices.z-1);

        sphereXPos_.setMaxValue(numSlices.x-1);
        sphereYPos_.setMaxValue(numSlices.y-1);
        sphereZPos_.setMaxValue(numSlices.z-1);
    }
}

void SlicePositionRenderer::togglePropertyVisibility() {
    /*xColor_.setVisible(renderXSlice_.get());
    xSliceIndexProp_.setVisible(renderXSlice_.get());
    lineXWidth_.setVisible(renderXSlice_.get());
    alphaFactorXPlane_.setVisible(renderXSlice_.get());

    yColor_.setVisible(renderYSlice_.get());
    ySliceIndexProp_.setVisible(renderYSlice_.get());
    lineYWidth_.setVisible(renderYSlice_.get());
    alphaFactorYPlane_.setVisible(renderYSlice_.get());

    zColor_.setVisible(renderZSlice_.get());
    zSliceIndexProp_.setVisible(renderZSlice_.get());
    lineZWidth_.setVisible(renderZSlice_.get());
    alphaFactorZPlane_.setVisible(renderZSlice_.get());

    sphereColor_.setVisible(renderSphere_.get());
    sphereRadius_.setVisible(renderSphere_.get());
    sphereDepthTest_.setVisible(renderSphere_.get());
    sphereXPos_.setVisible(renderSphere_.get());
    sphereYPos_.setVisible(renderSphere_.get());
    sphereZPos_.setVisible(renderSphere_.get()); */
}

}

