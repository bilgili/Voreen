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

#include "voreen/core/voreenapplication.h"

#include "tgt/glmath.h"
#include "voreen/core/utils/stringutils.h"

namespace voreen {

using tgt::vec4;
using tgt::vec3;

const std::string BoundingBoxRenderer::loggerCat_("voreen.base.BoundingBoxRenderer");

BoundingBoxRenderer::BoundingBoxRenderer()
    : GeometryRendererBase()
    , volumeInport_(Port::INPORT, "volume", "Volume Input")
    , geometryInport_(Port::INPORT, "geometry", "Geometry Input")
    , enable_("enable", "Enable", true)
    , width_("boundingBoxWidth", "Line width", 1.0f, 0.0f, 10.0f)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple pattern", 65535, 1,65535)
    , bboxColor_("boundingboxColor", "Color", tgt::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , showGrid_("boundingboxGridShow", "Show Grid", false)
    , tilesSections_("boundingboxGridSize", "Grid Elements", 10, 2, 255)
    , tilesOpacity_("boundingboxGridOpacity", "Opacity Factor",1.f, 0.f,1.f)
    , showLegend_("showLegend", "Show Distance Legend",false)
    , fontProp_("fontProp","Legend Font")
{
    //ports
    addPort(volumeInport_);
    addPort(geometryInport_);
    //enable
    addProperty(enable_);
    //line
    addProperty(width_);
        width_.setGroupID("line");
    addProperty(stippleFactor_);
        stippleFactor_.setGroupID("line");
    addProperty(stipplePattern_);
        stipplePattern_.setGroupID("line");
    addProperty(bboxColor_);
        bboxColor_.setViews(Property::COLOR);
        bboxColor_.setGroupID("line");
    setPropertyGroupGuiName("line","Line Settings");
    //grid
    addProperty(showGrid_);
        showGrid_.setGroupID("grid");
    addProperty(tilesSections_);
        tilesSections_.setGroupID("grid");
    addProperty(tilesOpacity_);
        tilesOpacity_.setGroupID("grid");
    setPropertyGroupGuiName("grid","Grid Settings");
    //legend
    addProperty(showLegend_);
        showLegend_.setGroupID("legend");
   //addProperty(fontProp_);
        //fontProp_.setGroupID("legend");
        fontProp_.get()->setFontName(VoreenApplication::app()->getFontPath("Vera.ttf"));
        fontProp_.get()->setFontType(tgt::Font::BitmapFont);
        fontProp_.get()->setSize(12);
        fontProp_.get()->setTextAlignment(tgt::Font::Center);
        fontProp_.get()->setVerticalTextAlignment(tgt::Font::Top);
    setPropertyGroupGuiName("legend","Legend Settings");
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
    MatStack.pushMatrix();
    tgt::vec3 geomLlf, geomUrb;
    if (volumeInport_.getData()) {
        geomLlf = volumeInport_.getData()->getLLF();
        geomUrb = volumeInport_.getData()->getURB();

        MatStack.multMatrix(volumeInport_.getData()->getPhysicalToWorldMatrix());
        inputToWorldTransformation_ = volumeInport_.getData()->getPhysicalToWorldMatrix();
    }
    else if (geometryInport_.hasData()) {
        tgt::Bounds bounds = geometryInport_.getData()->getBoundingBox(false);
        geomLlf = bounds.getLLF();
        geomUrb = bounds.getURB();

        MatStack.multMatrix(geometryInport_.getData()->getTransformationMatrix());
        inputToWorldTransformation_ = geometryInport_.getData()->getTransformationMatrix();
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

    tgt::vec3 tileDim((geomUrb[0]-geomLlf[0]), (geomUrb[1]-geomLlf[1]), (geomUrb[2]-geomLlf[2]));
    float maxDim = tgt::max(tileDim);
    float tileWidth = maxDim / static_cast<float>(tilesSections_.get());

    if (showGrid_.get()) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT, GL_LINE);
        glBegin(GL_QUADS);
        glColor4f(bboxColor_.get().r, bboxColor_.get().g, bboxColor_.get().b, bboxColor_.get().a*tilesOpacity_.get());
        for (float x = tileWidth; x < tileDim.x + tileWidth ; x += tileWidth) {
            for (float y = tileWidth; y < tileDim.y + tileWidth; y += tileWidth) {
                glVertex3f(geomLlf[0] + x - tileWidth,          geomLlf[1] + y - tileWidth,          geomLlf[2]);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomLlf[1] + y - tileWidth,          geomLlf[2]);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), std::min(geomLlf[1] + y,geomUrb[1]), geomLlf[2]);
                glVertex3f(geomLlf[0] + x - tileWidth,          std::min(geomLlf[1] + y,geomUrb[1]), geomLlf[2]);

                glVertex3f(geomLlf[0] + x - tileWidth,          geomLlf[1] + y - tileWidth,          geomUrb[2]);
                glVertex3f(geomLlf[0] + x - tileWidth,          std::min(geomLlf[1] + y,geomUrb[1]), geomUrb[2]);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), std::min(geomLlf[1] + y,geomUrb[1]), geomUrb[2]);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomLlf[1] + y - tileWidth,          geomUrb[2]);
            }
        }
        for (float x = tileWidth; x < tileDim.x + tileWidth; x += tileWidth) {
            for (float z = tileWidth; z < tileDim.z + tileWidth; z += tileWidth) {
                glVertex3f(geomLlf[0] + x - tileWidth, geomLlf[1], std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomLlf[1], std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomLlf[1], geomLlf[2] + z - tileWidth);
                glVertex3f(geomLlf[0] + x - tileWidth, geomLlf[1], geomLlf[2] + z - tileWidth);

                glVertex3f(geomLlf[0] + x - tileWidth,          geomUrb[1], std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(geomLlf[0] + x - tileWidth,          geomUrb[1], geomLlf[2] + z - tileWidth);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomUrb[1], geomLlf[2] + z - tileWidth);
                glVertex3f(std::min(geomLlf[0] + x,geomUrb[0]), geomUrb[1], std::min(geomLlf[2] + z,geomUrb[2]));
            }
        }
        for (float y = tileWidth; y < tileDim.y + tileWidth; y+=tileWidth) {
            for (float z = tileWidth; z < tileDim.z + tileWidth; z+=tileWidth) {
                glVertex3f(geomLlf[0], geomLlf[1] + y - tileWidth,          geomLlf[2] + z - tileWidth);
                glVertex3f(geomLlf[0], std::min(geomLlf[1] + y,geomUrb[1]), geomLlf[2] + z - tileWidth);
                glVertex3f(geomLlf[0], std::min(geomLlf[1] + y,geomUrb[1]), std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(geomLlf[0], geomLlf[1] + y - tileWidth,          std::min(geomLlf[2] + z,geomUrb[2]));

                glVertex3f(geomUrb[0], geomLlf[1] + y - tileWidth,          geomLlf[2] + z - tileWidth);
                glVertex3f(geomUrb[0], geomLlf[1] + y - tileWidth,          std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(geomUrb[0], std::min(geomLlf[1] + y,geomUrb[1]), std::min(geomLlf[2] + z,geomUrb[2]));
                glVertex3f(geomUrb[0], std::min(geomLlf[1] + y,geomUrb[1]), geomLlf[2] + z - tileWidth);
            }
        }
        glEnd();
    }

    if(showLegend_.get()) {

        //compute spatial length
        tgt::vec3 lx = (inputToWorldTransformation_ * tgt::vec4(geomUrb.x, geomLlf.y, geomLlf.z, 1.f)).xyz()
            - (inputToWorldTransformation_ * tgt::vec4(geomLlf, 1.f)).xyz();
        tgt::vec3 ly = (inputToWorldTransformation_ * tgt::vec4(geomLlf.x, geomUrb.y, geomLlf.z, 1.f)).xyz()
            - (inputToWorldTransformation_ * tgt::vec4(geomLlf, 1.f)).xyz();
        tgt::vec3 lz = (inputToWorldTransformation_ * tgt::vec4(geomLlf.x, geomLlf.y, geomUrb.z, 1.f)).xyz()
            - (inputToWorldTransformation_ * tgt::vec4(geomLlf, 1.f)).xyz();

        tgt::vec3 worldDimensions(tgt::length(lx), tgt::length(ly), tgt::length(lz));
        float worldMaxDim = tgt::max(worldDimensions);
        float worldTileWidth = worldMaxDim / static_cast<float>(tilesSections_.get());

        //paint legend
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
            MatStack.pushMatrix();
            MatStack.loadIdentity();
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
            MatStack.loadIdentity();

        MatStack.translate(-1,-1,0);
        float scaleFactorX = 2.0f / (float)viewport_.x;
        float scaleFactorY = 2.0f / (float)viewport_.y;
        MatStack.scale(scaleFactorX, scaleFactorY, 1);

        float lineWidth = std::min(100.f,(float)viewport_.x);
        float leftOffset = (float)(viewport_.x)/60.f;
        float bottomOffset = (float)(viewport_.y)/40.f;
        float centerOffset = leftOffset + lineWidth/2.f;

        glColor4fv(bboxColor_.get().elem);
        fontProp_.get()->setLineWidth(lineWidth);
        if(showGrid_.get()) {
            fontProp_.get()->render(tgt::vec3(leftOffset-5.f, bottomOffset, 0), formatSpatialLength(worldTileWidth));
            glBegin(GL_LINE_LOOP);
                glVertex2f(leftOffset+0.20f*lineWidth,bottomOffset+14.f);
                glVertex2f(leftOffset+0.70f*lineWidth,bottomOffset+14.f);
                glVertex2f(leftOffset+0.90f*lineWidth,bottomOffset+34.f);
                glVertex2f(leftOffset+0.40f*lineWidth,bottomOffset+34.f);
            glEnd();
        }
        else {
            fontProp_.get()->render(tgt::vec3(leftOffset, bottomOffset, 0), formatSpatialLength(worldMaxDim));
            glBegin(GL_LINE_LOOP);
                glVertex2f(leftOffset+0.2f*lineWidth,bottomOffset+13.f);
                glVertex2f(leftOffset+0.8f*lineWidth,bottomOffset+13.f);
                glVertex2f(leftOffset+0.8f*lineWidth,bottomOffset+38.f);
                glVertex2f(leftOffset+0.2f*lineWidth,bottomOffset+38.f);
            glEnd();
        }

        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
            MatStack.popMatrix();
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    }


    glPopAttrib();
    MatStack.popMatrix();

    LGL_ERROR;
}

}

