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

#include "geometryrenderer.h"
#include "voreen/core/datastructures/geometry/geometry.h"

namespace voreen {

GeometryRenderer::GeometryRenderer()
    : GeometryRendererBase()
    , inport_(Port::INPORT, "inport.geometry", "Geometry Input")
    , texPort_(Port::INPORT, "inport.texture", "Texture Input")
    , polygonMode_("polygonMode", "Polygon Mode")
    , mapTexture_("mapTexture", "Map Texture", false)
    , textureMode_("textureMode", "Texture Mode")
    , enableLighting_("enableLighting", "Enable Lighting", false)
    , lightPosition_("lightPosition", "Light Source Position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f), tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_("lightAmbient", "Ambient Light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_("lightDiffuse", "Diffuse Light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_("lightSpecular", "Specular Light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , materialShininess_("materialShininess", "Shininess", 60.f, 0.1f, 128.f)
{
    addPort(inport_);
    addPort(texPort_);

    polygonMode_.addOption("point", "Point", GL_POINT);
    polygonMode_.addOption("line",  "Line",  GL_LINE);
    polygonMode_.addOption("fill",  "Fill",  GL_FILL);
    polygonMode_.select("fill");
    addProperty(polygonMode_);

    mapTexture_.onChange(CallMemberAction<GeometryRenderer>(this, &GeometryRenderer::updatePropertyVisibilities));
    textureMode_.addOption("modulate", "GL_MODULATE",  GL_MODULATE);
    textureMode_.addOption("replace",  "GL_REPLACE",   GL_REPLACE);
    textureMode_.addOption("decal",    "GL_DECAL",     GL_DECAL);
    textureMode_.addOption("blend",    "GL_BLEND",     GL_BLEND);
    textureMode_.selectByKey("replace");
    addProperty(mapTexture_);
    addProperty(textureMode_);

    enableLighting_.onChange(CallMemberAction<GeometryRenderer>(this, &GeometryRenderer::updatePropertyVisibilities));

    //lightPosition_.setViews(Property::View(Property::LIGHT_POSITION | Property::DEFAULT));
    lightAmbient_.setViews(Property::COLOR);
    lightDiffuse_.setViews(Property::COLOR);
    lightSpecular_.setViews(Property::COLOR);
    addProperty(enableLighting_);
    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);

    // assign lighting properties to property group
    lightPosition_.setGroupID("lighting");
    lightAmbient_.setGroupID("lighting");
    lightDiffuse_.setGroupID("lighting");
    lightSpecular_.setGroupID("lighting");
    materialShininess_.setGroupID("lighting");
    setPropertyGroupGuiName("lighting", "Lighting Parameters");

    updatePropertyVisibilities();
}

Processor* GeometryRenderer::create() const {
    return new GeometryRenderer();
}

bool GeometryRenderer::isReady() const {
    return inport_.isReady();
}

void GeometryRenderer::render() {
    tgtAssert(inport_.hasData(), "No geometry");

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode_.getValue());
    glShadeModel(GL_SMOOTH);

    if (mapTexture_.get() && texPort_.isReady()) {
        texPort_.bindColorTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textureMode_.getValue());
    }

    if (enableLighting_.get()) {
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, tgt::vec4(0.f).elem);

        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition_.get().elem);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient_.get().elem);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse_.get().elem);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular_.get().elem);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess_.get());
        LGL_ERROR;
    }

    inport_.getData()->render();

    glPopAttrib();
}

void GeometryRenderer::updatePropertyVisibilities() {
    bool lighting = enableLighting_.get();
    /*lightPosition_.setVisible(lighting);
    lightAmbient_.setVisible(lighting);
    lightDiffuse_.setVisible(lighting);
    lightSpecular_.setVisible(lighting);
    materialShininess_.setVisible(lighting); */
    setPropertyGroupVisible("lighting", lighting);

    textureMode_.setVisible(mapTexture_.get());
}

}

