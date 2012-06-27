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

#include "voreen/core/vis/processors/geometry/geometryprocessor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/interaction/camerainteractionhandler.h"

#include <typeinfo>

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/quadric.h"

using tgt::vec4;
using tgt::vec3;

namespace voreen {

GeometryProcessor::GeometryProcessor()
    : RenderProcessor()
    , shaderPrg_(0)
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , inport_(Port::INPORT, "image.input")
    , outport_(Port::OUTPORT, "image.output")
    , tempPort_(Port::OUTPORT, "image.temp")
    , pickingPort_(Port::OUTPORT, "pickingTarget")
    , cpPort_(Port::INPORT, "coprocessor.geometryrenderers", true)
{

    addProperty(camera_);
    cameraHandler_ = new CameraInteractionHandler(&camera_);
    addInteractionHandler(cameraHandler_);

    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(tempPort_);
    addPrivateRenderPort(pickingPort_);
    addPort(cpPort_);
}

GeometryProcessor::~GeometryProcessor() {

    if (shaderPrg_)
        ShdrMgr.dispose(shaderPrg_);

    delete cameraHandler_;
}

void GeometryProcessor::initialize() throw (VoreenException) {
    idm_.setRenderTarget(pickingPort_.getData());
    idm_.initializeTarget();

    RenderProcessor::initialize();

    shaderPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "pp_geometry.frag",
        generateHeader(), false, false);

    if (!shaderPrg_) {
        LERROR("Failed to load shaders!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shaders!");
    }

    initialized_ = true;
}

Processor* GeometryProcessor::create() const {
    return new GeometryProcessor();
}

const std::string GeometryProcessor::getProcessorInfo() const {
    return "Manages the GeometryRenderer objects. Holds a vector of GeometryRenderer \
            Objects and renders all of them on <i>render()</i>";
}

bool GeometryProcessor::isReady() const {
    if (!outport_.isReady())
        return false;
    return true;
}

void GeometryProcessor::process() {
    //if there is no data on the inport we can render directly into the outport:
    if(!inport_.isReady())
        outport_.activateTarget();
    else
        tempPort_.activateTarget();

    LGL_ERROR;
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    tgt::loadMatrix(camera_.get()->getViewMatrix());
    LGL_ERROR;

    //render geometry:
    glDepthFunc(GL_LESS);
    std::vector<GeometryRenderer*> portData = cpPort_.getConnectedProcessors();
    for (size_t i=0; i<portData.size(); i++) {
        GeometryRenderer* pdcp = portData.at(i);
        if(pdcp->isReady()) {
            pdcp->setCamera(camera_.get());
            pdcp->render();
            LGL_ERROR;
        }
    }

    //render picking objects:
    idm_.activateTarget(getName());
    idm_.clearTarget();
    for (size_t i=0; i<portData.size(); i++) {
        GeometryRenderer* pdcp = portData.at(i);
        if(pdcp->isReady()) {
            pdcp->setIDManager(&idm_);
            pdcp->renderPicking();
            LGL_ERROR;
        }
    }

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    LGL_ERROR;

    // if inport is connected, combine both results:
    if (inport_.isReady()) {
        outport_.activateTarget();

        LGL_ERROR;
        glClearDepth(1.0);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;

        inport_.bindTextures(GL_TEXTURE0, GL_TEXTURE1);
        tempPort_.bindTextures(GL_TEXTURE2, GL_TEXTURE3);

        shaderPrg_->activate();

        setGlobalShaderParameters(shaderPrg_, camera_.get());
        shaderPrg_->setUniform("volumeShadeTex_", 0);
        shaderPrg_->setUniform("volumeDepthTex_", 1);
        inport_.setTextureParameters(shaderPrg_, "volumeTextureParameters_");

        shaderPrg_->setUniform("geometryShadeTex_", 2);
        shaderPrg_->setUniform("geometryDepthTex_", 3);
        tempPort_.setTextureParameters(shaderPrg_, "geometryTextureParameters_");

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        shaderPrg_->deactivate();
    }

    glDepthFunc(GL_LESS);
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}

//---------------------------------------------------------------------------

GeometryRenderer::GeometryRenderer()
    : RenderProcessor()
    //, camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , camera_(0)
    , outPort_(Port::OUTPORT, "coprocessor.geometryrenderer")
    , idm_(0)
{
    //addProperty(camera_);
    addPort(outPort_);
}

tgt::vec3 GeometryRenderer::getOGLPos(int x, int y,float z) const {
    if (!idm_)
        return tgt::vec3(0.0f);
    // taken from NEHE article 13
    // http://nehe.gamedev.net/data/articles/article.asp?article=13
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLdouble winX, winY, winZ;
    GLdouble posXh, posYh, posZh;

    tgt::mat4 projection_tgt = tgt::getTransposeProjectionMatrix();
    tgt::mat4 modelview_tgt = tgt::getTransposeModelViewMatrix();
    for (int i = 0; i < 4; ++i) {
        modelview[4*i]    = modelview_tgt[i].x;
        modelview[4*i+1]  = modelview_tgt[i].y;
        modelview[4*i+2]  = modelview_tgt[i].z;
        modelview[4*i+3]  = modelview_tgt[i].w;
        projection[4*i]   = projection_tgt[i].x;
        projection[4*i+1] = projection_tgt[i].y;
        projection[4*i+2] = projection_tgt[i].z;
        projection[4*i+3] = projection_tgt[i].w;
    }
    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = idm_->getRenderTarget()->getSize().x;
    viewport[3] = idm_->getRenderTarget()->getSize().y;

    winX = static_cast<GLdouble>(x);
    winY = static_cast<GLdouble>(viewport[3]) - static_cast<GLint>(y);
    winZ = static_cast<GLdouble>(z);

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posXh, &posYh, &posZh);

    tgt::vec3 returned = tgt::vec3(static_cast<float>(posXh), static_cast<float>(posYh), static_cast<float>(posZh));
    return returned;
}

tgt::vec3 GeometryRenderer::getWindowPos(tgt::vec3 pos) const {
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];

	tgt::mat4 projection_tgt = camera_->getProjectionMatrix();
	tgt::mat4 modelview_tgt = camera_->getViewMatrix();
	for (int i = 0; i < 4; ++i) {
		modelview[i+0]   = modelview_tgt[i].x;
		modelview[i+4]   = modelview_tgt[i].y;
		modelview[i+8]   = modelview_tgt[i].z;
		modelview[i+12]  = modelview_tgt[i].w;
		projection[i+0]  = projection_tgt[i].x;
		projection[i+4]  = projection_tgt[i].y;
		projection[i+8]  = projection_tgt[i].z;
		projection[i+12] = projection_tgt[i].w;
	}
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = idm_->getRenderTarget()->getSize().x;
	viewport[3] = idm_->getRenderTarget()->getSize().y;

	GLdouble pointProjectedGL[3];
	gluProject(pos.x, pos.y, pos.z, modelview, projection, viewport,
			   &pointProjectedGL[0], &pointProjectedGL[1], &pointProjectedGL[2]);

	return tgt::vec3(static_cast<float>(pointProjectedGL[0]),
					 static_cast<float>(pointProjectedGL[1]),
					 static_cast<float>(pointProjectedGL[2]));
}

void GeometryRenderer::process() {
    // DO NOTHING
}

//---------------------------------------------------------------------------

GeomLightWidget::GeomLightWidget()
    : GeometryRenderer()
    , showLightWidget_("set.showLightWidget", "Show Light Widget?", true)
    , isClicked_(false)
    , lightPosition_("lightPosition", "Light source position", tgt::vec4(2.3f, 1.5f, 1.5f, 1.f),
                     tgt::vec4(-10), tgt::vec4(10))
{

    //FIXME: memory leak (new EventAction?)
    moveSphereProp_ = new TemplateMouseEventProperty<GeomLightWidget>("Move the light widget", new EventAction<GeomLightWidget, tgt::MouseEvent>(this, &GeomLightWidget::moveSphere), tgt::MouseEvent::PRESSED | tgt::MouseEvent::MOTION | tgt::MouseEvent::RELEASED, tgt::Event::NONE, tgt::MouseEvent::MOUSE_BUTTON_LEFT);

    addProperty(showLightWidget_);
    addProperty(lightPosition_);
    addEventProperty(moveSphereProp_);

    // light parameters
    light_pos[0] = 0.0f;
    light_pos[1] = 1.0f;
    light_pos[2] = 1.1f;
    light_pos[3] = 1.0f;
    light_ambient[0] = 1.0f;
    light_ambient[1] = 1.0f;
    light_ambient[2] = 1.0f;
    light_ambient[3] = 1.0f;
    light_diffuse[0] = 1.0f;
    light_diffuse[1] = 1.0f;
    light_diffuse[2] = 1.0f;
    light_diffuse[3] = 1.0f;
    light_specular[0] = 1.0f;
    light_specular[1] = 1.0f;
    light_specular[2] = 1.0f;
    light_specular[3] = 1.0f;

    // parameters for yellow plastic
    ye_ambient[0]    = 0.25f;
    ye_ambient[1]    = 0.2f;
    ye_ambient[2]    = 0.07f;
    ye_ambient[3]    = 1.0f;
    ye_diffuse[0]    = 0.75f;
    ye_diffuse[1]    = 0.61f;
    ye_diffuse[2]    = 0.23f;
    ye_diffuse[3]    = 1.0f;
    ye_specular[0]    = 0.63f;
    ye_specular[1]    = 0.56f;
    ye_specular[2]    = 0.37f;
    ye_specular[3]    = 1.0f;
    ye_shininess    = 51.0f;
}

GeomLightWidget::~GeomLightWidget() {
    delete moveSphereProp_;
}

const std::string GeomLightWidget::getProcessorInfo() const {
    return "Draws a yellow sphere that indicates the position of the lightsource and can be moved. Use linking to connect this widget to a light position of another processor. ";
}

Processor* GeomLightWidget::create() const {
    return new GeomLightWidget();
}

void GeomLightWidget::moveSphere(tgt::MouseEvent* e) {
    LGL_ERROR;
    if (!idm_)
        return;

    if (e->action() & tgt::MouseEvent::PRESSED) {
        if (idm_->isHit(tgt::ivec2(e->x(), e->viewport().y - e->y() ), this)) {
            toggleInteractionMode(true, this);
            e->accept();
            invalidate();
            isClicked_ = true;
            lightPositionAbs_.x = lightPosition_.get().x;
            lightPositionAbs_.y = lightPosition_.get().y;
            lightPositionAbs_.z = lightPosition_.get().z;
            startCoord_.x = e->coord().x;
            startCoord_.y = e->coord().y;
        }
        return;
    }

    if (e->action() & tgt::MouseEvent::MOTION) {
        if (isClicked_) {
            e->accept();

            LGL_ERROR;
            GLint deltaX, deltaY;

            GLint viewport[4];
            GLdouble modelview[16];
            GLdouble projection[16];
            GLdouble winX, winY, winZ;
            GLdouble posX, posY, posZ;

            deltaX = e->coord().x - startCoord_.x;
            deltaY = startCoord_.y - e->coord().y;

            tgt::mat4 projection_tgt = camera_->getProjectionMatrix();
            tgt::mat4 modelview_tgt = camera_->getViewMatrix();
            for (int i = 0; i < 4; ++i) {
                modelview[i+0]   = modelview_tgt[i].x;
                modelview[i+4]   = modelview_tgt[i].y;
                modelview[i+8]   = modelview_tgt[i].z;
                modelview[i+12]  = modelview_tgt[i].w;
                projection[i+0]  = projection_tgt[i].x;
                projection[i+4]  = projection_tgt[i].y;
                projection[i+8]  = projection_tgt[i].z;
                projection[i+12] = projection_tgt[i].w;
            }
            viewport[0] = 0;
            viewport[1] = 0;
            //viewport[2] = static_cast<GLint>(e->viewport().x);
            //viewport[3] = static_cast<GLint>(e->viewport().y);
            viewport[2] = static_cast<GLint>(idm_->getRenderTarget()->getSize().x);
            viewport[3] = static_cast<GLint>(idm_->getRenderTarget()->getSize().y);

            posX = lightPositionAbs_.x;
            posY = lightPositionAbs_.y;
            posZ = lightPositionAbs_.z;

            LGL_ERROR;
            gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);

            winX = winX + deltaX;
            winY = winY + deltaY;

            LGL_ERROR;
            gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
            LGL_ERROR;

            lightPosition_.set(vec4(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ), 1.f));

            LGL_ERROR;
            invalidate();
            LGL_ERROR;
        }
        return;
    }

    if (e->action() & tgt::MouseEvent::RELEASED) {
        if (isClicked_) {
            toggleInteractionMode(false, this);
            e->accept();
            isClicked_ = false;
            invalidate();
        }
        return;
    }
}

void GeomLightWidget::render() {
    if (showLightWidget_.get()) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        LGL_ERROR;

        glLightfv(GL_LIGHT3,GL_POSITION,light_pos);
        glLightfv(GL_LIGHT3,GL_AMBIENT,light_ambient);
        glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
        glLightfv(GL_LIGHT3,GL_SPECULAR,light_specular);
        glDisable(GL_LIGHT0);
        glEnable(GL_LIGHT3);
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_diffuse);

        GLUquadricObj* quadric = gluNewQuadric();

        //light sphere widget
        glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    ye_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        ye_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        ye_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    ye_specular);
        LGL_ERROR;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
        LGL_ERROR;

        gluSphere(quadric,0.03f,20,20);
        LGL_ERROR;

        glPopMatrix();
        LGL_ERROR;
        glPopAttrib();
        LGL_ERROR;

        gluDeleteQuadric(quadric);
        LGL_ERROR;
    }
}

void GeomLightWidget::renderPicking() {
    if (!idm_)
        return;
    if (showLightWidget_.get()) {
        GLUquadricObj* quadric = gluNewQuadric();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);
        LGL_ERROR;

        idm_->setGLColor(this);
        gluSphere(quadric,0.03f,20,20);
        LGL_ERROR;

        glPopMatrix();
        gluDeleteQuadric(quadric);
        LGL_ERROR;
    }

}

void GeomLightWidget::setIDManager(IDManager* idm) {
    if (idm_ == idm)
        return;

    idm_ = idm;
    if (idm_) {
        idm->registerObject(this);
    }
}

//---------------------------------------------------------------------------

GeomBoundingBox::GeomBoundingBox()
    : GeometryRenderer()
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

const std::string GeomBoundingBox::getProcessorInfo() const {
    return "Draws the bounding box around the data set and allows to show a grid behind the volume.";
}

void GeomBoundingBox::setLineWidth(float width) {
    width_.set(width);
}

void GeomBoundingBox::setStipplePattern(int stippleFactor, int stipplePattern) {
    stippleFactor_.set(stippleFactor);
    stipplePattern_.set(stipplePattern);
}

void GeomBoundingBox::render() {
    
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

//---------------------------------------------------------------------------

SlicePositionRenderer::SlicePositionRenderer()
    : GeometryRenderer()
    , xColor_("xColor", "x Color", tgt::vec4(1.0f, 0.0f, 0.0f, 1.0f))
    , yColor_("yColor", "y Color", tgt::vec4(0.0f, 1.0f, 0.0f, 1.0f))
    , zColor_("zColor", "z Color", tgt::vec4(0.0f, 0.0f, 1.0f, 1.0f))
    , xSliceIndexProp_("xSliceIndex", "X slice number: ", 1, 1, 100)
    , ySliceIndexProp_("ySliceIndey", "y slice number: ", 1, 1, 100)
    , zSliceIndexProp_("zSliceIndez", "z slice number: ", 1, 1, 100)
    , width_("boundingBoxWidth", "Line width", 1.0f, 1.0f, 10.0f, true)
    , stippleFactor_("boundingBoxStippleFactor", "Stipple factor", 1, 0, 255)
    , stipplePattern_("boundingBoxStipplePattern", "Stipple pattern", 65535, 1,65535)
    , inport_(Port::INPORT, "volume")
{

    addProperty(width_);
    addProperty(stippleFactor_);
    addProperty(stipplePattern_);
    addProperty(xColor_);
    addProperty(xSliceIndexProp_);
    addProperty(yColor_);
    addProperty(ySliceIndexProp_);
    addProperty(zColor_);
    addProperty(zSliceIndexProp_);

    addPort(inport_);
}

const std::string SlicePositionRenderer::getProcessorInfo() const {
    return "Indicates the position of axis-aligned slices in a 3D view. \
           Can be used when linking multiple views, to show the position \
           of the 2D views with respect to the 3D view.";
}

void SlicePositionRenderer::process() {
    if(inport_.hasChanged()) {
        tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

        xSliceIndexProp_.setMaxValue(numSlices.x);
        xSliceIndexProp_.set(numSlices.x / 2);

        ySliceIndexProp_.setMaxValue(numSlices.y);
        ySliceIndexProp_.set(numSlices.y / 2);

        zSliceIndexProp_.setMaxValue(numSlices.z);
        zSliceIndexProp_.set(numSlices.z / 2);
    }
}

void SlicePositionRenderer::render() {
    if (!inport_.isReady())
        return;

    tgt::vec3 dim = inport_.getData()->getVolume()->getCubeSize() / 2.f;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glDisable(GL_LIGHTING);

    tgt::vec3 geomLlf = -dim;
    tgt::vec3 geomUrb = dim;

    glLineWidth(width_.get());
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(stippleFactor_.get(), stipplePattern_.get());

    tgt::ivec3 numSlices = inport_.getData()->getVolume()->getDimensions();

    float xSlice = (((float) xSliceIndexProp_.get() / (float) numSlices.x) * (geomUrb.x - geomLlf.x)) + geomLlf.x;
    float ySlice = (((float) ySliceIndexProp_.get() / (float) numSlices.y) * (geomUrb.y - geomLlf.y)) + geomLlf.y;
    float zSlice = (((float) zSliceIndexProp_.get() / (float) numSlices.z) * (geomUrb.z - geomLlf.z)) + geomLlf.z;

    glColor4f(xColor_.get().r, xColor_.get().g, xColor_.get().b, xColor_.get().a);
    glBegin(GL_LINE_LOOP);
    glVertex3f(xSlice, geomUrb.y, geomUrb.z);
    glVertex3f(xSlice, geomLlf.y, geomUrb.z);
    glVertex3f(xSlice, geomLlf.y, geomLlf.z);
    glVertex3f(xSlice, geomUrb.y, geomLlf.z);
    glEnd();

    glColor4f(yColor_.get().r, yColor_.get().g, yColor_.get().b, yColor_.get().a);
    glBegin(GL_LINE_LOOP);
    glVertex3f(geomLlf.x, ySlice, geomLlf.z);
    glVertex3f(geomLlf.x, ySlice, geomUrb.z);
    glVertex3f(geomUrb.x, ySlice, geomUrb.z);
    glVertex3f(geomUrb.x, ySlice, geomLlf.z);
    glEnd();

    glColor4f(zColor_.get().r, zColor_.get().g, zColor_.get().b, zColor_.get().a);
    glBegin(GL_LINE_LOOP);
    glVertex3f(geomLlf.x, geomUrb.y, zSlice);
    glVertex3f(geomLlf.x, geomLlf.y, zSlice);
    glVertex3f(geomUrb.x, geomLlf.y, zSlice);
    glVertex3f(geomUrb.x, geomUrb.y, zSlice);
    glEnd();

    glPopAttrib();
}

//---------------------------------------------------------------------------
PickingBoundingBox::PickingBoundingBox()
    : GeometryRenderer()
    , lowerLeftFront_(0.f,0.f,0.f)
    , upperRightBack_(1.f,1.f,1.f)
    , displaySelection_(false)
    , proxyGeomPort_(Port::INPORT, "coprocessor.proxygeometry", false)
{
    addPort(proxyGeomPort_);
}

const std::string PickingBoundingBox::getProcessorInfo() const {
    return "No information available.";
}

void PickingBoundingBox::render() {
    tgt::vec3 dim = proxyGeomPort_.getConnectedProcessor()->getVolumeSize() / 2.f;

    glDisable(GL_LIGHTING);

    glColor4f(0.8f,0.f,0.f,1.f);

    tgt::vec3 geomLlf = -dim;
    tgt::vec3 geomUrb = dim;

    glLineWidth(1.f);

    if (displaySelection_) {
        glBegin(GL_LINE_LOOP);
        // back face
        glVertex3f(lowerLeftFront_.x ,lowerLeftFront_.y ,upperRightBack_.z);
        glVertex3f(upperRightBack_.x ,lowerLeftFront_.y,upperRightBack_.z);
        glVertex3f(upperRightBack_.x ,upperRightBack_.y,upperRightBack_.z);
        glVertex3f(lowerLeftFront_.x ,upperRightBack_.y,upperRightBack_.z);
        glEnd();
        glBegin(GL_LINE_LOOP);
        // front face
            glVertex3f(lowerLeftFront_.x ,lowerLeftFront_.y ,lowerLeftFront_.z);
            glVertex3f(upperRightBack_.x ,lowerLeftFront_.y,lowerLeftFront_.z);
            glVertex3f(upperRightBack_.x ,upperRightBack_.y,lowerLeftFront_.z);
            glVertex3f(lowerLeftFront_.x ,upperRightBack_.y,lowerLeftFront_.z);
        glEnd();

        glBegin(GL_LINES);

        glVertex3f(lowerLeftFront_.x ,lowerLeftFront_.y ,lowerLeftFront_.z);
        glVertex3f(lowerLeftFront_.x ,lowerLeftFront_.y ,upperRightBack_.z);

        glVertex3f(upperRightBack_.x ,lowerLeftFront_.y ,lowerLeftFront_.z);
        glVertex3f(upperRightBack_.x ,lowerLeftFront_.y ,upperRightBack_.z);


        glVertex3f(lowerLeftFront_.x ,upperRightBack_.y ,lowerLeftFront_.z);
        glVertex3f(lowerLeftFront_.x ,upperRightBack_.y ,upperRightBack_.z);

        glVertex3f(upperRightBack_.x ,upperRightBack_.y ,lowerLeftFront_.z);
        glVertex3f(upperRightBack_.x ,upperRightBack_.y ,upperRightBack_.z);

        glEnd();
    }
}

//------------------------------------------------------------------------------

GeomRegistrationMarkers::GeomRegistrationMarkers()
  : GeometryRenderer(),
  marker1Selected_(false),
  marker2Selected_(false),
  marker3Selected_(false),
  marker1_(0.f),
  marker2_(0.f),
  marker3_(0.f),
  marker1Radius_(0.014f),
  marker2Radius_(0.014f),
  marker3Radius_(0.014f),
  description_(""),
  disableReceiving_(false)
    , proxyGeomPort_(Port::INPORT, "coprocessor.proxygeometry", false)
{
    addPort(proxyGeomPort_);
}

const std::string GeomRegistrationMarkers::getProcessorInfo() const {
    return "No information available.";
}

void GeomRegistrationMarkers::setDescription(std::string description){
    description_ = description;
}

void GeomRegistrationMarkers::render() {
    tgt::vec3 dim = proxyGeomPort_.getConnectedProcessor()->getVolumeSize() / 2.f;

    glDisable(GL_LIGHTING);

    //tgt::vec3 dim = tgt::vec3(200,200,200);//  pg_->getVolumeSize();

    glDisable(GL_DEPTH_TEST);

    GLUquadricObj*  quadric = gluNewQuadric();

    glColor4f(0.16f,0.91f,0.95f,1.f);

    if (marker1Selected_) {
        glTranslatef(-dim.x + marker1_.x *dim.x * 2,-dim.y + marker1_.y * dim.y * 2,
                     -dim.z + marker1_.z * dim.z *2);

        glColor3f(1.f,0.f,0.f);
        gluSphere(quadric,marker1Radius_,20,20);
    }

    if (marker2Selected_) {
        glLoadIdentity();
        tgt::multMatrix(camera_->getViewMatrix());
        glTranslatef(-dim.x + marker2_.x *dim.x * 2,-dim.y + marker2_.y * dim.y * 2,
                     -dim.z + marker2_.z * dim.z *2);
        glColor3f(0.f,1.f,0.f);
        gluSphere(quadric,marker2Radius_,20,20);
    }

    if (marker3Selected_) {
        glLoadIdentity();
        tgt::multMatrix(camera_->getViewMatrix());
        glTranslatef(-dim.x + marker3_.x *dim.x * 2,-dim.y + marker3_.y * dim.y * 2,
                     -dim.z + marker3_.z * dim.z *2);
        glColor3f(0.f,0.f,1.f);
        gluSphere(quadric,marker3Radius_,20,20);
    }

    glEnable(GL_DEPTH_TEST);
}


} // namespace voreen
