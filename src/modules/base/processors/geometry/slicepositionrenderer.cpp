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

#include "voreen/modules/base/processors/geometry/slicepositionrenderer.h"

#include "tgt/glmath.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

SlicePositionRenderer::SlicePositionRenderer()
    : GeometryRendererBase()
    , xColor_("xColor", "X Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "Y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "Z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , xSliceIndexProp_("xSliceIndex", "X Slice Number", 0, 0, 10000)
    , ySliceIndexProp_("ySliceIndex", "Y Slice Number", 0, 0, 10000)
    , zSliceIndexProp_("zSliceIndex", "Z Slice Number", 0, 0, 10000)
    , width_("boundingBoxWidth", "Line Width", 1.0f, 1.0f, 10.0f)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple Factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple Pattern", 65535, 1,65535)
    , inport_(Port::INPORT, "volume")
{
    addPort(inport_);

    addProperty(xColor_);
    addProperty(xSliceIndexProp_);
    addProperty(yColor_);
    addProperty(ySliceIndexProp_);
    addProperty(zColor_);
    addProperty(zSliceIndexProp_);
    addProperty(width_);
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);

    xColor_.setViews(Property::COLOR);
    yColor_.setViews(Property::COLOR);
    zColor_.setViews(Property::COLOR);
}

Processor* SlicePositionRenderer::create() const {
    return new SlicePositionRenderer();
}

std::string SlicePositionRenderer::getProcessorInfo() const {
    return "Indicates the position of axis-aligned slices in a 3D view. "
           "Can be used when linking multiple views, to show the position "
           "of the 2D views with respect to the 3D view.";
}

void SlicePositionRenderer::process() {
    // nothing, see render()
}

void SlicePositionRenderer::render() {
    if (!inport_.isReady())
        return;

    tgt::vec3 dim = inport_.getData()->getVolume()->getCubeSize() / 2.f;

    glPushMatrix();
    tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    tgt::vec3 geomLlf = -dim;
    tgt::vec3 geomUrb = dim;

    glLineWidth(width_.get());
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(stippleFactor_.get(), stipplePattern_.get());

    tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

    float xSlice = (((float) xSliceIndexProp_.get() / (float) (numSlices.x-1)) * (geomUrb.x - geomLlf.x)) + geomLlf.x;
    float ySlice = (((float) ySliceIndexProp_.get() / (float) (numSlices.y-1)) * (geomUrb.y - geomLlf.y)) + geomLlf.y;
    float zSlice = (((float) zSliceIndexProp_.get() / (float) (numSlices.z-1)) * (geomUrb.z - geomLlf.z)) + geomLlf.z;

    if (xColor_.get().a > 0.f) {
        glColor4f(xColor_.get().r, xColor_.get().g, xColor_.get().b, xColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(xSlice, geomUrb.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomUrb.z);
        glVertex3f(xSlice, geomLlf.y, geomLlf.z);
        glVertex3f(xSlice, geomUrb.y, geomLlf.z);
        glEnd();
    }

    if (yColor_.get().a > 0.f) {
        glColor4f(yColor_.get().r, yColor_.get().g, yColor_.get().b, yColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, ySlice, geomLlf.z);
        glVertex3f(geomLlf.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomUrb.z);
        glVertex3f(geomUrb.x, ySlice, geomLlf.z);
        glEnd();
    }

    if (zColor_.get().a > 0.f) {
        glColor4f(zColor_.get().r, zColor_.get().g, zColor_.get().b, zColor_.get().a);
        glBegin(GL_LINE_LOOP);
        glVertex3f(geomLlf.x, geomUrb.y, zSlice);
        glVertex3f(geomLlf.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomLlf.y, zSlice);
        glVertex3f(geomUrb.x, geomUrb.y, zSlice);
        glEnd();
    }

    glPopMatrix();
    glPopAttrib();
}

void SlicePositionRenderer::invalidate(int inv) {

    GeometryRendererBase::invalidate(inv);

    if (inport_.hasChanged() && inport_.hasData()) {
        tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

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

}

