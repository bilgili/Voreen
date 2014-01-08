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

#include "geometryrenderer.h"
#include "voreen/core/datastructures/geometry/geometry.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"

namespace voreen {

GeometryRenderer::GeometryRenderer()
    : GeometryRendererBase()
    , inport_(Port::INPORT, "inport.geometry", "Geometry Input")
    , texPort_(Port::INPORT, "inport.texture", "Texture Input")
    , polygonMode_("polygonMode", "Polygon Mode")
    , lineWidth_("lineWidth", "Line Width", 1.f, 1.f, 20.f)
    , pointSize_("pointSize", "Point Size", 1.f, 1.f, 20.f)
    , mapTexture_("mapTexture", "Map Texture", false)
    , textureMode_("textureMode", "Texture Mode")
    , enableLighting_("enableLighting", "Enable Lighting", false)
    , lightPosition_("lightPosition", "Light Source Position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f), tgt::vec4(-10), tgt::vec4(10))
    , lightAmbient_("lightAmbient", "Ambient Light", tgt::Color(0.4f, 0.4f, 0.4f, 1.f))
    , lightDiffuse_("lightDiffuse", "Diffuse Light", tgt::Color(0.8f, 0.8f, 0.8f, 1.f))
    , lightSpecular_("lightSpecular", "Specular Light", tgt::Color(0.6f, 0.6f, 0.6f, 1.f))
    , materialShininess_("materialShininess", "Shininess", 60.f, 0.1f, 128.f)
    , useShader_("use.shader", "Use shader", false)
    , shaderProp_("geometry.prg", "Shader", "trianglemesh.frag", "trianglemesh.vert", "trianglemesh.geom")
    , enableClipping_("enable.clipping", "Enable on-the-fly clipping", false)
    , planeNormal_("plane.normal", "Clipping plane normal", tgt::vec3(0.f, 0.f, 1.f), tgt::vec3(-1.f), tgt::vec3(1.f))
    , planeDistance_("plane.distance", "Clipping plane distance", 0.f, -10000.f, 10000.f)
{
    addPort(inport_);
    addPort(texPort_);

    polygonMode_.addOption("point", "Point", GL_POINT);
    polygonMode_.addOption("line",  "Line",  GL_LINE);
    polygonMode_.addOption("fill",  "Fill",  GL_FILL);
    polygonMode_.select("fill");
    polygonMode_.onChange(CallMemberAction<GeometryRenderer>(this, &GeometryRenderer::updatePropertyVisibilities));
    addProperty(polygonMode_);

    addProperty(pointSize_);
    addProperty(lineWidth_);

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

    addProperty(useShader_);
    addProperty(shaderProp_);
    addProperty(enableClipping_);
    addProperty(planeNormal_);
    addProperty(planeDistance_);

    useShader_.onChange(CallMemberAction<GeometryRenderer>(this, &GeometryRenderer::updatePropertyVisibilities));
    enableClipping_.onChange(CallMemberAction<GeometryRenderer>(this, &GeometryRenderer::updatePropertyVisibilities));

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

void GeometryRenderer::compile() {
    shaderProp_.setHeader(generateHeader());
    shaderProp_.rebuild();
}

bool GeometryRenderer::isReady() const {
    return inport_.isReady();
}

void GeometryRenderer::render() {
    tgtAssert(inport_.hasData(), "No geometry");

    if(useShader_.get() && (!shaderProp_.hasValidShader() || getInvalidationLevel() >= Processor::INVALID_PROGRAM))
        compile();

    if(useShader_.get() && !shaderProp_.hasValidShader()) {
        LERROR("Shader for geometry failed to compile");
        return;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode_.getValue());
    glShadeModel(GL_SMOOTH);

    if (polygonMode_.isSelected("point"))
        glPointSize(pointSize_.get());
    else if (polygonMode_.isSelected("line"))
        glLineWidth(lineWidth_.get());

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

    // for triangle meshes, we need to use a correctly layouted shader
    if(useShader_.get()) {
        tgt::Shader* prog = shaderProp_.getShader();
        prog->activate();
        setGlobalShaderParameters(prog, &camera_);
        prog->setIgnoreUniformLocationError(true);

        prog->setUniform("enableLighting_", enableLighting_.get());
        if(enableLighting_.get()) {
            prog->setUniform("lightPosition_", lightPosition_.get());
            prog->setUniform("lightSource_.ambientColor_", lightAmbient_.get().xyz());
            prog->setUniform("lightSource_.diffuseColor_", lightDiffuse_.get().xyz());
            prog->setUniform("lightSource_.specularColor_", lightSpecular_.get().xyz());
            //prog->setUniform("lightSource_.attenuation_", tgt::vec3(1.f, 0.f, 0.f));
            prog->setUniform("shininess_", materialShininess_.get());
        }

        prog->setUniform("enableClipping_", enableClipping_.get());
        if(enableClipping_.get())
            prog->setUniform("plane_", tgt::vec4(normalize(planeNormal_.get()), planeDistance_.get()));
    }

    inport_.getData()->render();

    if(useShader_.get())
        shaderProp_.getShader()->deactivate();

    glPopAttrib();
}

void GeometryRenderer::process() {
    if(useShader_.get() && getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
}

void GeometryRenderer::updatePropertyVisibilities() {
    pointSize_.setVisible(polygonMode_.isSelected("point"));
    lineWidth_.setVisible(polygonMode_.isSelected("line"));

    bool lighting = enableLighting_.get();
    setPropertyGroupVisible("lighting", lighting);

    textureMode_.setVisible(mapTexture_.get());

    shaderProp_.setVisible(useShader_.get());
    enableClipping_.setVisible(useShader_.get());
    planeNormal_.setVisible(useShader_.get() && enableClipping_.get());
    planeDistance_.setVisible(useShader_.get() && enableClipping_.get());
}

std::string GeometryRenderer::generateHeader() {
    std::string header = GeometryRendererBase::generateHeader();
    if(const TriangleMeshGeometryBase* tmgb = dynamic_cast<const TriangleMeshGeometryBase*>(inport_.getData())) {
        if(tmgb->getVertexLayout() == TriangleMeshGeometryBase::VEC3)
            header += "#define TRIANGLE_VEC3\n";
        else if(tmgb->getVertexLayout() == TriangleMeshGeometryBase::VEC4VEC3)
            header += "#define TRIANGLE_VEC4_VEC3\n";
    }
    return header;
}

}

