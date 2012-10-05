/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

namespace voreen {

using tgt::vec4;
using tgt::vec3;

SlicePositionRenderer::SlicePositionRenderer()
    : GeometryRendererBase()
    , renderXSlice_("renderXSlice", "Render X Slice", true)
    , xColor_("xColor", "X Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , xSliceIndexProp_("xSliceIndex", "X Slice Number", 0, 0, 10000)
    , renderYSlice_("renderYSlice", "Render Y Slice", true)
    , yColor_("yColor", "Y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , ySliceIndexProp_("ySliceIndex", "Y Slice Number", 0, 0, 10000)
    , renderZSlice_("renderZSlice", "Render Z Slice", true)
    , zColor_("zColor", "Z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , zSliceIndexProp_("zSliceIndex", "Z Slice Number", 0, 0, 10000)
    , width_("boundingBoxWidth", "Line Width", 1.0f, 1.0f, 10.0f)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple Factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple Pattern", 65535, 1,65535)
    , renderPlanes_("renderPlanes", "Render Planes", true)
    , planeAlphaFactor_("planeAlpha", "Plane Alpha Factor", 0.25f, 0.f, 1.f)
    , inport_(Port::INPORT, "volume", "Volume Input")
{
    addPort(inport_);

    addProperty(renderXSlice_);
    addProperty(xColor_);
    addProperty(xSliceIndexProp_);
    addProperty(renderYSlice_);
    addProperty(yColor_);
    addProperty(ySliceIndexProp_);
    addProperty(renderZSlice_);
    addProperty(zColor_);
    addProperty(zSliceIndexProp_);

    addProperty(width_);
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);
    addProperty(renderPlanes_);
    addProperty(planeAlphaFactor_);

    xColor_.setViews(Property::COLOR);
    yColor_.setViews(Property::COLOR);
    zColor_.setViews(Property::COLOR);
}

void SlicePositionRenderer::process() {}

void SlicePositionRenderer::render() {
    if (!inport_.isReady())
        return;

    glPushMatrix();
    tgt::multMatrix(inport_.getData()->getPhysicalToWorldMatrix());

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    tgt::vec3 geomLlf = inport_.getData()->getLLF();
    tgt::vec3 geomUrb = inport_.getData()->getURB();
    tgt::vec3 sp = inport_.getData()->getSpacing();

    glLineWidth(width_.get());
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(stippleFactor_.get(), stipplePattern_.get());

    tgt::ivec3 numSlices = inport_.getData()->getRepresentation<VolumeRAM>()->getDimensions();

    // We want our slice to be in the center of voxels
    float xSlice = (((float) xSliceIndexProp_.get() + 0.5f) * sp.x) + geomLlf.x;
    float ySlice = (((float) ySliceIndexProp_.get() + 0.5f) * sp.y) + geomLlf.y;
    float zSlice = (((float) zSliceIndexProp_.get() + 0.5f) * sp.z) + geomLlf.z;

    if (xColor_.get().a > 0.f && renderXSlice_.get()) {
        glColor4f(xColor_.get().r, xColor_.get().g, xColor_.get().b, xColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(xSlice, geomUrb.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomLlf.z);
        glVertex3f(xSlice, geomUrb.y, geomLlf.z);
        glEnd();

        if (renderPlanes_.get()) {
            glColor4f(xColor_.get().r, xColor_.get().g, xColor_.get().b, xColor_.get().a * planeAlphaFactor_.get());
            glBegin(GL_QUADS);
            glVertex3f(xSlice, geomUrb.y, geomUrb.z);
            glVertex3f(xSlice, geomLlf.y, geomUrb.z);
            glVertex3f(xSlice, geomLlf.y, geomLlf.z);
            glVertex3f(xSlice, geomUrb.y, geomLlf.z);
            glEnd();
        }
    }

    if (yColor_.get().a > 0.f && renderYSlice_.get()) {
        glColor4f(yColor_.get().r, yColor_.get().g, yColor_.get().b, yColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, ySlice, geomLlf.z);
        glVertex3f(geomLlf.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomLlf.z);
        glEnd();

        if (renderPlanes_.get()) {
            glColor4f(yColor_.get().r, yColor_.get().g, yColor_.get().b, yColor_.get().a * planeAlphaFactor_.get());
            glBegin(GL_QUADS);
            glVertex3f(geomLlf.x, ySlice, geomLlf.z);
            glVertex3f(geomLlf.x, ySlice, geomUrb.z);
            glVertex3f(geomUrb.x, ySlice, geomUrb.z);
            glVertex3f(geomUrb.x, ySlice, geomLlf.z);
            glEnd();
        }
    }

    if (zColor_.get().a > 0.f && renderZSlice_.get()) {
        glColor4f(zColor_.get().r, zColor_.get().g, zColor_.get().b, zColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, geomUrb.y, zSlice);
        glVertex3f(geomLlf.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomUrb.y, zSlice);
        glEnd();

        if (renderPlanes_.get()) {
            glColor4f(zColor_.get().r, zColor_.get().g, zColor_.get().b, zColor_.get().a * planeAlphaFactor_.get());
            glBegin(GL_QUADS);
            glVertex3f(geomLlf.x, geomUrb.y, zSlice);
            glVertex3f(geomLlf.x, geomLlf.y, zSlice);
            glVertex3f(geomUrb.x, geomLlf.y, zSlice);
            glVertex3f(geomUrb.x, geomUrb.y, zSlice);
            glEnd();
        }
    }

    glPopMatrix();
    glPopAttrib();
}

void SlicePositionRenderer::invalidate(int inv) {
    GeometryRendererBase::invalidate(inv);

    if (inport_.hasChanged() && inport_.hasData()) {
        tgt::ivec3 numSlices = inport_.getData()->getRepresentation<VolumeRAM>()->getDimensions();

        xSliceIndexProp_.setMaxValue(numSlices.x-1);
        ySliceIndexProp_.setMaxValue(numSlices.y-1);
        zSliceIndexProp_.setMaxValue(numSlices.z-1);

        if (xSliceIndexProp_.get() > xSliceIndexProp_.getMaxValue())
            xSliceIndexProp_.set(xSliceIndexProp_.getMaxValue());
        if (ySliceIndexProp_.get() > ySliceIndexProp_.getMaxValue())
            ySliceIndexProp_.set(ySliceIndexProp_.getMaxValue());
        if (zSliceIndexProp_.get() > zSliceIndexProp_.getMaxValue())
            zSliceIndexProp_.set(zSliceIndexProp_.getMaxValue());
    }
}

void SlicePositionRenderer::togglePropertyVisibility() {
    xColor_.setVisible(renderXSlice_.get());
    xSliceIndexProp_.setVisible(renderXSlice_.get());

    yColor_.setVisible(renderYSlice_.get());
    ySliceIndexProp_.setVisible(renderYSlice_.get());

    zColor_.setVisible(renderZSlice_.get());
    zSliceIndexProp_.setVisible(renderZSlice_.get());

    bool noneVisible = !renderXSlice_.get() && !renderYSlice_.get() && !renderZSlice_.get();
    if (noneVisible)
        renderPlanes_.set(false);
    planeAlphaFactor_.setVisible(renderPlanes_.get());

    stippleFactor_.setVisible(noneVisible);
    stipplePattern_.setVisible(noneVisible);
}

}

