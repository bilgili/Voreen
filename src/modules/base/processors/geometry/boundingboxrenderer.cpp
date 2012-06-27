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

#include "voreen/modules/base/processors/geometry/boundingboxrenderer.h"

#include "tgt/glmath.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

BoundingBoxRenderer::BoundingBoxRenderer()
    : GeometryRendererBase()
    , bboxColor_("boundingboxColor", "Color", tgt::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , width_("boundingBoxWidth", "Line width", 1.0f, 1.0f, 10.0f, true)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple pattern", 65535, 1,65535)
    , showGrid_("boundingboxGridShow", "Show Grid", false)
    , tilesProp_("boundingboxGridSize", "GridElements", tgt::ivec3(10), tgt::ivec3(2), tgt::ivec3(255))
    , applyDatasetTransformationMatrix_("applyDatasetTrafoMatrix", "Apply data set trafo matrix", true, Processor::INVALID_PARAMETERS)
    , inport_(Port::INPORT, "volume")
{

    addProperty(applyDatasetTransformationMatrix_);
    addProperty(width_);
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);
    addProperty(bboxColor_);

    addProperty(showGrid_);
    addProperty(tilesProp_);
    addPort(inport_);
}

std::string BoundingBoxRenderer::getProcessorInfo() const {
    return "Draws the bounding box around the data set and allows to show a grid behind the volume.";
}

void BoundingBoxRenderer::setLineWidth(float width) {
    width_.set(width);
}

void BoundingBoxRenderer::setStipplePattern(int stippleFactor, int stipplePattern) {
    stippleFactor_.set(stippleFactor);
    stipplePattern_.set(stipplePattern);
}

void BoundingBoxRenderer::render() {

    tgtAssert(inport_.isReady(), "render() called with an not-ready inport");

    tgt::vec3 dim = inport_.getData()->getVolume()->getCubeSize() / 2.f;

    if (applyDatasetTransformationMatrix_.get()) {
        glPushMatrix();
        tgt::multMatrix(inport_.getData()->getVolume()->getTransformation());
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    glColor4f(bboxColor_.get().r, bboxColor_.get().g, bboxColor_.get().b, bboxColor_.get().a);

    tgt::vec3 geomLlf = -dim;
    tgt::vec3 geomUrb = dim;

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

    if (applyDatasetTransformationMatrix_.get())
        glPopMatrix();

    LGL_ERROR;
}
}

