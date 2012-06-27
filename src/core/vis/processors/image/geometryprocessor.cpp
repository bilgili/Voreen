/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"

#include <typeinfo>

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/quadric.h"

#include "voreen/core/vis/lightmaterial.h"

using tgt::vec4;

namespace voreen {

GeometryProcessor::GeometryProcessor(tgt::Camera* camera, TextureContainer* tc)
    : Processor(camera, tc)
{
	setName("Geometry Processor");

	createInport("image.inport");
	createOutport("image.outport",false,"image.inport");
	createCoProcessorInport("coprocessor.geometryrenderers",true);

}

GeometryProcessor::~GeometryProcessor() {
}

const std::string GeometryProcessor::getProcessorInfo() const {
	return "Manages the GeometryRenderer objects. Holds a vector of GeometryRenderer \
           Objects and renders all of them on <i>render()</i>";
}

void GeometryProcessor::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.inport");
    tc_->setActiveTarget(source, "GeometryProcessor::process: add geometry");
    glViewport(0, 0, static_cast<int>(size_.x), static_cast<int>(size_.y));

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
		pdcp->call("render",portMapping->createLocalPortMapping(pdcp->getProcessor()));
	}
    // restore matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

//---------------------------------------------------------------------------

GeometryRenderer::GeometryRenderer(tgt::Camera* camera, TextureContainer* tc)
    : Processor(camera, tc)
{}

tgt::vec3 GeometryRenderer::getOGLPos(int x, int y,float /* z*/) {
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
    viewport[2] = size_.x;
    viewport[3] = size_.y;

	winX = static_cast<GLdouble>(x);
	winY = static_cast<GLdouble>(viewport[3]) - static_cast<GLint>(y);
	winZ = 0;

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

GeomLightWidget::GeomLightWidget(tgt::Camera* camera, TextureContainer* tc)
    : GeometryRenderer(camera, tc), tgt::EventListener()
    , showLightWidget_("set.showLightWidget", "Show Light Widget?", true)
    , shadowLightPosX_("set.shadowLightPos.x", "Light Position X", 2.3f, -10.0f, 10.0f)
    , shadowLightPosY_("set.shadowLightPos.y", "Light Position Y", 1.5f, -10.0f, 10.0f)
    , shadowLightPosZ_("set.shadowLightPos.z", "Light Position Z", 1.5f, -10.0f, 10.0f)
    , volumeSizeValid_(false)
{
    setName("Light - Widget");

    shadowMode_ = 0;

    isClicked_ = false;

    addProperty(&shadowLightPosX_);
    addProperty(&shadowLightPosY_);
    addProperty(&shadowLightPosZ_);

	showLightWidget_.setAutoChange(true);
	addProperty(&showLightWidget_);

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

    // parameters for yellow light
    ye_ambient[0]	= 0.25f;
    ye_ambient[1]	= 0.2f;
    ye_ambient[2]	= 0.07f;
    ye_ambient[3]	= 1.0f;
    ye_diffuse[0]	= 0.75f;
    ye_diffuse[1]	= 0.61f;
    ye_diffuse[2]	= 0.23f;
    ye_diffuse[3]	= 1.0f;
    ye_specular[0]	= 0.63f;
    ye_specular[1]	= 0.56f;
    ye_specular[2]	= 0.37f;
    ye_specular[3]	= 1.0f;
    ye_shininess	= 51.0f;

	//lightPosition_.set(tgt::vec4(shadowLightPosX_->get(), shadowLightPosY_->get(), shadowLightPosZ_.get(), 1.f));

    IDManager id1;
    id1.addNewPickObj("lightCtrlSph");

	createCoProcessorOutport("coprocessor.light",&Processor::call);
	createCoProcessorInport("coprocessor.proxygeometry");

	setIsCoprocessor(true);
}

GeomLightWidget::~GeomLightWidget() {
	MsgDistr.postMessage(new TemplateMessage<tgt::EventListener*>(VoreenPainter::removeMouseListener_,
				    (tgt::EventListener*)this));
}

const std::string GeomLightWidget::getProcessorInfo() const {
	return "Draws the light widget.";
}

Processor* GeomLightWidget::create() {
    GeomLightWidget* glw = new GeomLightWidget();
    MsgDistr.postMessage(
        new TemplateMessage<tgt::EventListener*>(VoreenPainter::addMouseListener_,
        (tgt::EventListener*)glw));
    return glw;
}

void GeomLightWidget::processMessage(Message* msg, const Identifier& /*dest*/){
    if (msg->id_ == "set.lightPosition") {
        shadowLightPosX_.set(msg->getValue<tgt::vec4>().x);
        shadowLightPosY_.set(msg->getValue<tgt::vec4>().y);
        shadowLightPosZ_.set(msg->getValue<tgt::vec4>().z);
        invalidate();
    }
    else if (msg->id_ == "rotate.lightYAxis") {
        float angle = msg->getValue<float>();
		float radAngle = angle/180.f*tgt::PIf;
        shadowLightPosX_.set(cos(radAngle) * shadowLightPosX_.get()
            + sinf(radAngle) * shadowLightPosZ_.get());
        shadowLightPosZ_.set(-1.f * sin(radAngle) * shadowLightPosX_.get()
            + cosf(radAngle) * shadowLightPosZ_.get());
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec3(shadowLightPosX_.get(),shadowLightPosY_.get(),
            shadowLightPosZ_.get()).elem);
        invalidate();
        MsgDistr.postMessage(new Message("light.changed"), VoreenPainter::visibleViews_);
    }
    else if (msg->id_ == "set.shadowLightPos.x") {
        shadowLightPosX_.set(msg->getValue<float>());
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec3(shadowLightPosX_.get(),shadowLightPosY_.get(),
            shadowLightPosZ_.get()).elem);
        invalidate();
        MsgDistr.postMessage(new Message("light.changed"), VoreenPainter::visibleViews_);
    }
    else if (msg->id_ == "set.shadowLightPos.y") {
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec3(shadowLightPosX_.get(),shadowLightPosY_.get(),
            shadowLightPosZ_.get()).elem);
        shadowLightPosY_.set(msg->getValue<float>());
        invalidate();
        MsgDistr.postMessage(new Message("light.changed"), VoreenPainter::visibleViews_);
    }
    else if (msg->id_ == "set.shadowLightPos.z") {
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec3(shadowLightPosX_.get(),shadowLightPosY_.get(),
            shadowLightPosZ_.get()).elem);
        shadowLightPosZ_.set(msg->getValue<float>());
        invalidate();
        MsgDistr.postMessage(new Message("light.changed"), VoreenPainter::visibleViews_);
    }
    else if (msg->id_ == "switch.shadowMode") {
        shadowMode_ = msg->getValue<int>();
    }
}

void GeomLightWidget::mousePressEvent(tgt::MouseEvent *e) {
    IDManager id1;
    if (id1.isClicked("lightCtrlSph", e->x(),tc_->getSize().y - e->y())) {
        e->accept();
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true), VoreenPainter::visibleViews_);
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        isClicked_ = true;
        lightPositionAbs_.x = lightPosition_.get().x;
        lightPositionAbs_.y = lightPosition_.get().y;
        lightPositionAbs_.z = lightPosition_.get().z;
        oldPos_.x = e->coord().x;
		oldPos_.y = e->coord().y;
    }
    else
        e->ignore();
}

void GeomLightWidget::mouseMoveEvent(tgt::MouseEvent *e) {
    if (isClicked_) {
        e->accept();
		GLint deltaX, deltaY;

		GLint viewport[4];
        GLdouble modelview[16];
        GLdouble projection[16];
        GLdouble winX, winY, winZ;
		GLdouble posX, posY, posZ;

		deltaX = e->coord().x - oldPos_.x;
		deltaY = oldPos_.y - e->coord().y;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        tgt::loadMatrix(camera_->getViewMatrix());
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
        viewport[2] = static_cast<GLint>(size_.x);
        viewport[3] = static_cast<GLint>(size_.y);

		posX = lightPositionAbs_.x;
		posY = lightPositionAbs_.y;
		posZ = lightPositionAbs_.z;

        gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);

		winX = winX + deltaX;
        winY = winY + deltaY;

        gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

        //if shadowMode == DSM light has to be outside the volume
        if (shadowMode_ == 1 && volumeSizeValid_) {
            if (posX<volumeSize_.x && posX>-volumeSize_.x &&
                posY<volumeSize_.y && posY>-volumeSize_.y &&
                posZ<volumeSize_.z && posZ>-volumeSize_.z) {
                double minX1 = static_cast<double>(posX-volumeSize_.x);
                double minX2 = static_cast<double>(posX+volumeSize_.x);
                double minY1 = static_cast<double>(posY-volumeSize_.y);
                double minY2 = static_cast<double>(posY+volumeSize_.y);
                double minZ1 = static_cast<double>(posZ-volumeSize_.z);
                double minZ2 = static_cast<double>(posZ+volumeSize_.z);
                double absMin = std::min(minX1,std::min(minX2, std::min(minY1, std::min(minY2, std::min(minZ1, minZ2)))));
                if (absMin == minX1)
                    posX = volumeSize_.x;
                if (absMin == minX2)
                    posX = -volumeSize_.x;
                if (absMin == minY1)
                    posY = volumeSize_.y;
                if (absMin == minY2)
                    posY = -volumeSize_.y;
                if (absMin == minZ1)
                    posZ = volumeSize_.z;
                if (absMin == minZ2)
                    posZ = -volumeSize_.z;
            }
        }
    
        lightPosition_.set(vec4(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ), 1.f));

        shadowLightPosX_.set(lightPosition_.get().x);
        shadowLightPosY_.set(lightPosition_.get().y);
        shadowLightPosZ_.set(lightPosition_.get().z);

        glPopMatrix();

        MsgDistr.postMessage(new Vec4Msg(LightMaterial::setLightPosition_, lightPosition_.get()));
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void GeomLightWidget::mouseReleaseEvent(tgt::MouseEvent *e) {
    if (isClicked_) {
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), VoreenPainter::visibleViews_);
        e->accept();
        isClicked_ = false;
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition_.get().elem);
        MsgDistr.postMessage(new Message("light.changed"), VoreenPainter::visibleViews_);
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void GeomLightWidget::render(LocalPortMapping* portMapping) {
    PortDataCoProcessor* pdcp = portMapping->getCoProcessorData("coprocessor.proxygeometry");
 
    Message* sizeMsg = pdcp->call(ProxyGeometry::getVolumeSize_);
    if (sizeMsg) {
        volumeSize_ = sizeMsg->getValue<tgt::vec3>();
        volumeSizeValid_ = true;
    }
    else {
        volumeSize_ = tgt::vec3(0.f);
        volumeSizeValid_ = false;
    }
    delete sizeMsg;

	if (showLightWidget_.get()) {
		glPushAttrib(GL_LIGHTING_BIT);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_LIGHTING);
		LGL_ERROR;

		tgt::vec3 lightPosCorrected = tgt::vec3(lightPosition_.get().elem);

		glLightfv(GL_LIGHT3,GL_POSITION,light_pos);
		glLightfv(GL_LIGHT3,GL_AMBIENT,light_ambient);
		glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
		glLightfv(GL_LIGHT3,GL_SPECULAR,light_specular);
		glDisable(GL_LIGHT0);
		glEnable(GL_LIGHT3);
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_diffuse);

		GLUquadricObj* quadric = gluNewQuadric();

		//light sphere widget
		glMaterialf( GL_FRONT_AND_BACK,	GL_SHININESS,	ye_shininess);
		glMaterialfv(GL_FRONT_AND_BACK,	GL_AMBIENT,		ye_ambient);
		glMaterialfv(GL_FRONT_AND_BACK,	GL_DIFFUSE,		ye_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK,	GL_SPECULAR,	ye_specular);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
		glTranslatef(lightPosCorrected.x, lightPosCorrected.y, lightPosCorrected.z);

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

GeomBoundingBox::GeomBoundingBox(tgt::Camera* camera, TextureContainer* tc)
    : GeometryRenderer(camera, tc)
    , bboxColor_("set.BoundingboxColor", "Color", tgt::vec4(0.8f, 0.8f, 0.8f, 1.0f))
    , width_("set.BoundingBoxWidth", "Line Width", 1.0f, 1.0f, 10.0f, true)
    , stippleFactor_("set.BoundingBoxStippleFactor", "Stipple Factor", 1, 0, 255)
    , stipplePattern_("set.BoundingBoxStipplePattern", "Stipple Pattern", 65535, 1,65535)
    , showGrid_("set.BoundingBoxShowGrid", "Show Grid", false)
    , tilesX_("set.BoundingBoxTilesX", "Grid Elements X", 10, 2, 255)
    , tilesY_("set.BoundingBoxTilesY", "Grid Elements Y", 10, 2, 255)
    , tilesZ_("set.BoundingBoxTilesZ", "Grid Elements Z", 10, 2, 255)
{
    setName("Bounding Box");

    addProperty(&width_);
    addProperty(&stippleFactor_);
    addProperty(&stipplePattern_);
    addProperty(&bboxColor_);

    cond_ = new ConditionProp("GridCond", &showGrid_);
    addProperty(cond_);
    addProperty(&showGrid_);
    tilesX_.setConditioned("GridCond", 1);
    tilesY_.setConditioned("GridCond", 1);
    tilesZ_.setConditioned("GridCond", 1);
    addProperty(&tilesX_);
    addProperty(&tilesY_);
    addProperty(&tilesZ_);

	createCoProcessorOutport("coprocessor.boundingbox",&Processor::call);
	createCoProcessorInport("coprocessor.proxygeometry");

	setIsCoprocessor(true);
}

GeomBoundingBox::~GeomBoundingBox() {
    delete cond_;
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
        float tileDimX = (geomUrb[0]-geomLlf[0])/tilesX_.get();
        float tileDimY = (geomUrb[1]-geomLlf[1])/tilesY_.get();
        float tileDimZ = (geomUrb[2]-geomLlf[2])/tilesZ_.get();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT, GL_LINE);
        glBegin(GL_QUADS);
        for (int x=1; x<=tilesX_.get(); x++) {
            for (int y=1; y<=tilesY_.get(); y++) {
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1]+(y-1)*tileDimY, geomLlf[2]);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1]+(y-1)*tileDimY, geomLlf[2]);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1]+y*tileDimY, geomLlf[2]);
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1]+y*tileDimY, geomLlf[2]);

                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1]+y*tileDimY, geomUrb[2]);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1]+y*tileDimY, geomUrb[2]);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1]+(y-1)*tileDimY, geomUrb[2]);
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1]+(y-1)*tileDimY, geomUrb[2]);
            }
        }
        for (int x=1; x<=tilesX_.get(); x++) {
            for (int z=1; z<=tilesZ_.get(); z++) {
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1], geomLlf[2]+z*tileDimZ);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1], geomLlf[2]+z*tileDimZ);
                glVertex3f(geomLlf[0]+x*tileDimX, geomLlf[1], geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomLlf[1], geomLlf[2]+(z-1)*tileDimZ);

                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomUrb[1], geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomLlf[0]+x*tileDimX, geomUrb[1], geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomLlf[0]+x*tileDimX, geomUrb[1], geomLlf[2]+z*tileDimZ);
                glVertex3f(geomLlf[0]+(x-1)*tileDimX, geomUrb[1], geomLlf[2]+z*tileDimZ);
            }
        }
        for (int y=1; y<=tilesY_.get(); y++) {
            for (int z=1; z<=tilesZ_.get(); z++) {
                glVertex3f(geomLlf[0], geomLlf[1]+(y-1)*tileDimY, geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomLlf[0], geomLlf[1]+y*tileDimY, geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomLlf[0], geomLlf[1]+y*tileDimY, geomLlf[2]+z*tileDimZ);
                glVertex3f(geomLlf[0], geomLlf[1]+(y-1)*tileDimY, geomLlf[2]+z*tileDimZ);

                glVertex3f(geomUrb[0], geomLlf[1]+(y-1)*tileDimY, geomLlf[2]+z*tileDimZ);
                glVertex3f(geomUrb[0], geomLlf[1]+y*tileDimY, geomLlf[2]+z*tileDimZ);
                glVertex3f(geomUrb[0], geomLlf[1]+y*tileDimY, geomLlf[2]+(z-1)*tileDimZ);
                glVertex3f(geomUrb[0], geomLlf[1]+(y-1)*tileDimY, geomLlf[2]+(z-1)*tileDimZ);
            }
        }
        glEnd();
        glPolygonMode(GL_FRONT, GL_FILL);
        glDisable(GL_CULL_FACE);
    }
}

void GeomBoundingBox::processMessage(Message* msg, const Identifier& /*dest*/) {
    if (msg->id_ == "set.BoundingboxColor") {
        bboxColor_.set(msg->getValue<tgt::Color>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxWidth") {
        width_.set(msg->getValue<float>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxStippleFactor") {
        stippleFactor_.set(msg->getValue<int>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxStipplePattern") {
        stipplePattern_.set(msg->getValue<int>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxShowGrid") {
        showGrid_.set(msg->getValue<bool>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxTilesX") {
        tilesX_.set(msg->getValue<int>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxTilesY") {
        tilesY_.set(msg->getValue<int>());
        invalidate();
    }
    else if (msg->id_ == "set.BoundingBoxTilesZ") {
        tilesZ_.set(msg->getValue<int>());
        invalidate();
    }
}

//---------------------------------------------------------------------------

PickingBoundingBox::PickingBoundingBox(tgt::Camera* camera, TextureContainer* tc)
  : GeometryRenderer(camera, tc)
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
