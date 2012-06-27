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
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

#include <typeinfo>

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/quadric.h"

#include "voreen/core/vis/lightmaterial.h"

using tgt::vec4;

namespace voreen {

GeometryProcessor::GeometryProcessor()
    : RenderProcessor()
    , shaderPrg_(0)
{
    setName("Geometry Processor");

    createInport("image.inport");
    createOutport("image.outport");
    createCoProcessorInport("coprocessor.geometryrenderers", true);

}

GeometryProcessor::~GeometryProcessor() {
    if (shaderPrg_)
        ShdrMgr.dispose(shaderPrg_);
}

int GeometryProcessor::initializeGL() {
    initStatus_ = Processor::initializeGL();
    if (initStatus_ != VRN_OK)
        return initStatus_;

    shaderPrg_ = ShdrMgr.loadSeparate("pp_identity.vert", "vrn_interactionmode.frag",
        generateHeader(), false);

    if (!shaderPrg_)
        initStatus_ = VRN_ERROR;

    return initStatus_;
}

const Identifier GeometryProcessor::getClassName() const {
    return "GeometryRenderer.GeometryProcessor";
}

Processor* GeometryProcessor::create() const {
    return new GeometryProcessor();
}

const std::string GeometryProcessor::getProcessorInfo() const {
    return "Manages the GeometryRenderer objects. Holds a vector of GeometryRenderer \
            Objects and renders all of them on <i>render()</i>";
}

void GeometryProcessor::process(LocalPortMapping* portMapping) {
    
    int source;
    try {
        source = portMapping->getTarget("image.inport");
    } 
    catch (std::exception& ) {
        source = -1;
    }

    int dest = portMapping->getTarget("image.outport");

    tc_->setActiveTarget(dest, "GeometryProcessor::process() add geometry");
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // if inport is connected, copy previous result to new texturetarget
    if (source > -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));

        shaderPrg_->activate();
        setGlobalShaderParameters(shaderPrg_);
        shaderPrg_->setUniform("shadeTex_", 0);
        shaderPrg_->setUniform("depthTex_", 1);

        shaderPrg_->setUniform("interactionCoarseness_", 1);

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);
        shaderPrg_->deactivate();
    }

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_->getViewMatrix());

    std::vector<PortDataCoProcessor*> portData = portMapping->getAllCoProcessorData("coprocessor.geometryrenderers");
    for (size_t i=0; i<portData.size(); i++) {
        PortDataCoProcessor* pdcp = portData.at(i);
        pdcp->call("render", portMapping->createLocalPortMapping(pdcp->getProcessor()));
    }
    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

//---------------------------------------------------------------------------

GeometryRenderer::GeometryRenderer()
    : RenderProcessor()
{}

tgt::vec3 GeometryRenderer::getOGLPos(int x, int y,float z) const {
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
    // use size from texture container, as processor size may be changed by coarseness mode
    viewport[2] = tc_->getSize().x;
    viewport[3] = tc_->getSize().y;

    winX = static_cast<GLdouble>(x);
    winY = static_cast<GLdouble>(viewport[3]) - static_cast<GLint>(y);
    winZ = static_cast<GLdouble>(z);

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posXh, &posYh, &posZh);

    tgt::vec3 returned = tgt::vec3(static_cast<float>(posXh), static_cast<float>(posYh), static_cast<float>(posZh));
    return returned;
}

void GeometryRenderer::process(LocalPortMapping* /*portMapping*/) {
    // DO NOTHING
}

Message* GeometryRenderer::call(Identifier ident,LocalPortMapping* portMapping) {
    if (ident =="render") {
        render(portMapping);
        return 0;
    }
    return 0;
}

//---------------------------------------------------------------------------

GeomLightWidget::GeomLightWidget()
    : GeometryRenderer()
    , tgt::EventListener()
    , showLightWidget_("set.showLightWidget", "Show Light Widget?", true)
    , moveEvent_("Move the light widget", tgt::Event::NONE, tgt::MouseEvent::MOUSE_BUTTON_LEFT)
    , isClicked_(false)
{
    setName("Light - Widget");

    addProperty(&showLightWidget_);
    addProperty(&moveEvent_);

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

    IDManager id1;
    id1.addNewPickObj("lightCtrlSph");

    createCoProcessorOutport("coprocessor.light", &Processor::call);
    createCoProcessorInport("coprocessor.proxygeometry");

    setIsCoprocessor(true);
}

GeomLightWidget::~GeomLightWidget() {
}

const Identifier GeomLightWidget::getClassName() const {
    return "GeometryRenderer.GeomLightWidget";
}

const std::string GeomLightWidget::getProcessorInfo() const {
    return "Draws a yellow sphere that indicates the position of the lightsource.";
}

Processor* GeomLightWidget::create() const {
    GeomLightWidget* glw = new GeomLightWidget();
    return glw;
}

void GeomLightWidget::mousePressEvent(tgt::MouseEvent* e) {
    IDManager id1;
    if (id1.isClicked("lightCtrlSph", e->x(), tc_->getSize().y - e->y())) {
        e->accept();
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true), VoreenPainter::visibleViews_);
        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        isClicked_ = true;
        lightPositionAbs_.x = lightPosition_.get().x;
        lightPositionAbs_.y = lightPosition_.get().y;
        lightPositionAbs_.z = lightPosition_.get().z;
        startCoord_.x = e->coord().x;
        startCoord_.y = e->coord().y;
    }
    else
        e->ignore();
}

void GeomLightWidget::mouseMoveEvent(tgt::MouseEvent* e) {
    if (isClicked_ && moveEvent_.accepts(e)) {
        e->accept();
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
        // use size from texture container, as processor size may be changed by coarseness mode
        viewport[2] = static_cast<GLint>(tc_->getSize().x);
        viewport[3] = static_cast<GLint>(tc_->getSize().y);

        posX = lightPositionAbs_.x;
        posY = lightPositionAbs_.y;
        posZ = lightPositionAbs_.z;

        gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);

        winX = winX + deltaX;
        winY = winY + deltaY;

        gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

        lightPosition_.set(vec4(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ), 1.f));

        MsgDistr.postMessage(new Vec4Msg(LightMaterial::setLightPosition_, lightPosition_.get()));
        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void GeomLightWidget::mouseReleaseEvent(tgt::MouseEvent* e) {
    if (isClicked_) {
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), VoreenPainter::visibleViews_);
        e->accept();
        isClicked_ = false;
        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void GeomLightWidget::render(LocalPortMapping* /*portMapping*/) {
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

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(lightPosition_.get().x, lightPosition_.get().y, lightPosition_.get().z);

        IDManager id1;
        id1.startBufferRendering("lightCtrlSph");
        gluSphere(quadric,0.03f,20,20);
        id1.stopBufferRendering();
        gluSphere(quadric,0.03f,20,20);

        glPopMatrix();
        glPopAttrib();

        gluDeleteQuadric(quadric);
    }
}

//---------------------------------------------------------------------------

GeomBoundingBox::GeomBoundingBox()
    : GeometryRenderer()
    , bboxColor_("set.BoundingboxColor", "Color", tgt::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , width_("set.BoundingBoxWidth", "Line Width", 1.0f, 1.0f, 10.0f, true)
    , stippleFactor_("set.BoundingBoxStippleFactor", "Stipple Factor", 1, 0, 255)
    , stipplePattern_("set.BoundingBoxStipplePattern", "Stipple Pattern", 65535, 1,65535)
    , showGrid_("set.BoundingBoxShowGrid", "Show Grid", false)
    , tilesProp_("BoutndingBoxTiles", "GridElements", tgt::ivec3(10), tgt::ivec3(2), tgt::ivec3(255))
{
    setName("Bounding Box");

    addProperty(&width_);
    addProperty(&stippleFactor_);
    addProperty(&stipplePattern_);
    addProperty(&bboxColor_);

    addProperty(&tilesProp_);

    createCoProcessorOutport("coprocessor.boundingbox",&Processor::call);
    createCoProcessorInport("coprocessor.proxygeometry");

    setIsCoprocessor(true);
}

const std::string GeomBoundingBox::getProcessorInfo() const {
    return "Draws bounding box around the data set.";
}

void GeomBoundingBox::setLineWidth(float width) {
    width_.set(width);
}

void GeomBoundingBox::setStipplePattern(int stippleFactor, int stipplePattern) {
    stippleFactor_.set(stippleFactor);
    stipplePattern_.set(stipplePattern);
}

void GeomBoundingBox::render(LocalPortMapping* portMapping) {
    tgt::vec3 dim;

    PortDataCoProcessor* pdcp = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    Message* sizeMsg = pdcp->call(ProxyGeometry::getVolumeSize_);
    if (sizeMsg)
        dim = sizeMsg->getValue<tgt::vec3>();
    else
        return;
    delete sizeMsg;

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
}

//---------------------------------------------------------------------------

PickingBoundingBox::PickingBoundingBox()
  : GeometryRenderer()
  , lowerLeftFront_(0.f,0.f,0.f)
  , upperRightBack_(1.f,1.f,1.f)
  , displaySelection_(false)
{
    setName("Picking Bounding Box");

    createCoProcessorOutport("coprocessor.pickingboundingbox",&Processor::call);

    setIsCoprocessor(true);
    createCoProcessorInport("coprocessor.proxygeometry");
}

const std::string PickingBoundingBox::getProcessorInfo() const {
    return "No information available.";
}

void PickingBoundingBox::render(LocalPortMapping* portMapping) {
    tgt::vec3 dim;
    PortDataCoProcessor* pdcp = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    Message* sizeMsg = pdcp->call(ProxyGeometry::getVolumeSize_);
    if (sizeMsg)
        dim = sizeMsg->getValue<tgt::vec3>();
    else
        return;
    delete sizeMsg;

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

void PickingBoundingBox::processMessage(Message* msg, const Identifier& /*dest*/) {
    if (msg->id_ == "set.PickingBoundingBox_lowerLeftFront") {
        lowerLeftFront_ = tgt::vec3(msg->getValue<tgt::vec3>());
        invalidate();
    }
    else if (msg->id_ == "set.PickingBoundingBox_upperRightBack") {
        upperRightBack_ = tgt::vec3(msg->getValue<tgt::vec3>());
        invalidate();
    }
    else if (msg->id_ == "set.PickingBoundingBox_hide") {
        displaySelection_ = !msg->getValue<bool>();

        invalidate();
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
{

    createCoProcessorOutport("coprocessor.geommarkers",&Processor::call);

    setIsCoprocessor(true);
    createCoProcessorInport("coprocessor.proxygeometry");
}

const std::string GeomRegistrationMarkers::getProcessorInfo() const {
    return "No information available.";
}

void GeomRegistrationMarkers::setDescription(std::string description){
    description_ = description;
}

void GeomRegistrationMarkers::render(LocalPortMapping* portMapping) {
    PortDataCoProcessor* pdcp = portMapping->getCoProcessorData("coprocessor.proxygeometry");
    tgt::vec3 dim;
    Message* sizeMsg = pdcp->call(ProxyGeometry::getVolumeSize_);
    if (sizeMsg)
        dim = sizeMsg->getValue<tgt::vec3>();
    else
        return;
    delete sizeMsg;

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

void GeomRegistrationMarkers::processMessage(Message* msg, const Identifier& /*dest*/) {
   if ( (msg->id_ == "ct.Marker1Selected") && (description_ == "target.ct-renderer"))
        marker1Selected_=msg->getValue<bool>();
   else if ((msg->id_ == "ct.Marker2Selected") && (description_ == "target.ct-renderer"))
        marker2Selected_=msg->getValue<bool>();
   else if ((msg->id_ == "ct.Marker3Selected") && (description_ == "target.ct-renderer"))
        marker3Selected_=msg->getValue<bool>();
   else if ( (msg->id_ == "pet.Marker1Selected") && (description_ == "target.pet-renderer"))
        marker1Selected_=msg->getValue<bool>();
   else if ((msg->id_ == "pet.Marker2Selected") && (description_ == "target.pet-renderer"))
        marker2Selected_=msg->getValue<bool>();
   else if ((msg->id_ == "pet.Marker3Selected") && (description_ == "target.pet-renderer"))
        marker3Selected_=msg->getValue<bool>();
   else if ((msg->id_ == "ct.Marker1Position") && (description_ == "target.ct-renderer"))
       marker1_ = msg->getValue<tgt::vec3>();
   else if ((msg->id_ == "ct.Marker2Position") && (description_ == "target.ct-renderer"))
       marker2_ = msg->getValue<tgt::vec3>();
   else if ((msg->id_ == "ct.Marker3Position") && (description_ == "target.ct-renderer"))
       marker3_ = msg->getValue<tgt::vec3>();
   else if ((msg->id_ == "pet.Marker1Position") && (description_ == "target.pet-renderer"))
       marker1_ = msg->getValue<tgt::vec3>();
   else if ((msg->id_ == "pet.Marker2Position") && (description_ == "target.pet-renderer"))
       marker2_ = msg->getValue<tgt::vec3>();
   else if ((msg->id_ == "pet.Marker3Position") && (description_ == "target.pet-renderer"))
       marker3_ = msg->getValue<tgt::vec3>();
   else if (msg->id_ == "Marker1Radius")
       marker1Radius_ = msg->getValue<float>();
   else if (msg->id_ == "Marker2Radius")
       marker2Radius_ = msg->getValue<float>();
   else if (msg->id_ == "Marker3Radius")
       marker3Radius_ = msg->getValue<float>();
}

} // namespace voreen
