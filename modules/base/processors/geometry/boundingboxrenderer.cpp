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

#include "boundingboxrenderer.h"

#include "tgt/glmath.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

const std::string BoundingBoxRenderer::loggerCat_("voreen.base.BoundingBoxRenderer");

BoundingBoxRenderer::BoundingBoxRenderer()
    : GeometryRendererBase()
    , volumeInport_(Port::INPORT, "volume", "Volume Input")
    , geometryInport_(Port::INPORT, "geometry", "Geometry Input")
    , enable_("enable", "Enable", true)
    , bboxColor_("boundingboxColor", "Color", tgt::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , width_("boundingBoxWidth", "Line width", 1.0f, 1.0f, 10.0f)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple pattern", 65535, 1,65535)
    , showGrid_("boundingboxGridShow", "Show Grid", false)
    , tilesProp_("boundingboxGridSize", "GridElements", tgt::ivec3(10), tgt::ivec3(2), tgt::ivec3(255))
{
    addPort(volumeInport_);
    addPort(geometryInport_);

    addProperty(enable_);
    addProperty(showGrid_);
    addProperty(tilesProp_);

    addProperty(width_);
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);
    addProperty(bboxColor_);
    bboxColor_.setViews(Property::COLOR);
}

Processor* BoundingBoxRenderer::create() const {
    return new BoundingBoxRenderer();
}

bool BoundingBoxRenderer::isReady() const {
    if (!isInitialized())
        return false;

    return (volumeInport_.isReady() || geometryInport_.isReady());
}

void BoundingBoxRenderer::render() {
    if (!enable_.get())
        return;

    tgtAssert(volumeInport_.isReady() || geometryInport_.isReady(), "neither inport is ready");
    if (volumeInport_.isReady() && geometryInport_.isReady()) {
        LWARNING("Either volume inport or geometry inport may be connected");
        return;
    }

    // use either volume or geometry bounding box
    glPushMatrix();
    tgt::vec3 geomLlf, geomUrb;
    if (volumeInport_.getData()) {
        geomLlf = volumeInport_.getData()->getLLF();
        geomUrb = volumeInport_.getData()->getURB();

        tgt::multMatrix(volumeInport_.getData()->getPhysicalToWorldMatrix());
    }
    else if (geometryInport_.hasData()) {
        tgt::Bounds bounds = geometryInport_.getData()->getBoundingBox();
        geomLlf = bounds.getLLF();
        geomUrb = bounds.getURB();
    }
    else {
        tgtAssert(false, "no input has data"); //< should never get here
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    glColor4f(bboxColor_.get().r, bboxColor_.get().g, bboxColor_.get().b, bboxColor_.get().a);

    glLineWidth(width_.get());
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(stippleFactor_.get(), stipplePattern_.get());
    glBegin(GL_LINE_LOOP);
    // back face
    glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
    glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
    glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
    glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
    glEnd();
    glBegin(GL_LINE_LOOP);
    // front face
    glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
    glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
    glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
    glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
    glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);

    glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
    glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);

    glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
    glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);

    glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
    glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
    glEnd();

    if (showGrid_.get()) {
        tgt::vec3 tileDim((geomUrb[0]-geomLlf[0]), (geomUrb[1]-geomLlf[1]), (geomUrb[2]-geomLlf[2]));
        const tgt::ivec3& tilePropValue = tilesProp_.get();
        tileDim.x /= tilePropValue.x;
        tileDim.y /= tilePropValue.y;
        tileDim.z /= tilePropValue.z;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT, GL_LINE);
        glBegin(GL_QUADS);
        for (int x = 1; x <= tilePropValue.x; x++) {
            for (int y = 1; y <= tilePropValue.y; y++) {
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1] + (y-1) * tileDim.y, geomLlf[2]);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1] + (y-1) * tileDim.y, geomLlf[2]);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1] + y * tileDim.y, geomLlf[2]);
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1] + y * tileDim.y, geomLlf[2]);

                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1] + y * tileDim.y, geomUrb[2]);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1] + y * tileDim.y, geomUrb[2]);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1] + (y-1) * tileDim.y, geomUrb[2]);
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1] + (y-1) * tileDim.y, geomUrb[2]);
            }
        }
        for (int x = 1; x <= tilePropValue.x; x++) {
            for (int z = 1; z <= tilePropValue.z; z++) {
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1], geomLlf[2] + z * tileDim.z);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1], geomLlf[2] + z * tileDim.z);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomLlf[1], geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomLlf[1], geomLlf[2] + (z-1) * tileDim.z);

                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomUrb[1], geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomUrb[1], geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomLlf[0] + x * tileDim.x, geomUrb[1], geomLlf[2] + z * tileDim.z);
                glVertex3f(geomLlf[0] + (x-1) * tileDim.x, geomUrb[1], geomLlf[2] + z * tileDim.z);
            }
        }
        for (int y = 1; y <= tilePropValue.y; y++) {
            for (int z = 1; z <= tilePropValue.z; z++) {
                glVertex3f(geomLlf[0], geomLlf[1] + (y-1) * tileDim.y, geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomLlf[0], geomLlf[1] + y * tileDim.y, geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomLlf[0], geomLlf[1] + y * tileDim.y, geomLlf[2] + z * tileDim.z);
                glVertex3f(geomLlf[0], geomLlf[1] + (y-1) * tileDim.y, geomLlf[2] + z * tileDim.z);

                glVertex3f(geomUrb[0], geomLlf[1] + (y-1) * tileDim.y, geomLlf[2] + z * tileDim.z);
                glVertex3f(geomUrb[0], geomLlf[1] + y * tileDim.y, geomLlf[2] + z * tileDim.z);
                glVertex3f(geomUrb[0], geomLlf[1] + y * tileDim.y, geomLlf[2] + (z-1) * tileDim.z);
                glVertex3f(geomUrb[0], geomLlf[1] + (y-1) * tileDim.y, geomLlf[2] + (z-1) * tileDim.z);
            }
        }
        glEnd();
    }
    glPopAttrib();
    glPopMatrix();

    LGL_ERROR;
}

}

