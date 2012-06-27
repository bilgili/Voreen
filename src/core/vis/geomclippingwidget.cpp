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

#include "voreen/core/vis/geomclippingwidget.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/event/eventlistener.h"

#include <typeinfo>

namespace voreen {

ClippingWidget::ClippingWidget(tgt::Camera* camera, TextureContainer* tc)
  : GeometryRenderer(camera, tc)
  , tgt::EventListener()
  , sliceColor_("set.ClipSliceColor", "Color for Clipping Slices", tgt::vec4(0.18f, 0.68f, 1.0f, 0.3f))
  , polyOffsetFact_("set.polyOffsetFact", "PolygonOffset Factor", 20.f, 0.f, 100.f, false)
  , polyOffsetUnit_("set.polyOffsetUnit", "PolygonOffset Unit", 10.0f, 0.f, 100.f, false)
  , x_lock_("switch.ClipXLock","Lock X-Clipping planes", false)
  , y_lock_("switch.ClipYLock","Lock Y-Clipping planes", false)
  , z_lock_("switch.ClipZLock","Lock Z-Clipping planes", false)
  , volumeSizeValid_(false)
{
    setName("ClippingWidget");
    IDManager id1;
    id1.addNewPickObj("clipp.x-contr1");
    id1.addNewPickObj("clipp.x-contr2");
    id1.addNewPickObj("clipp.y-contr1");
    id1.addNewPickObj("clipp.y-contr2");
    id1.addNewPickObj("clipp.z-contr1");
    id1.addNewPickObj("clipp.z-contr2");

    isClicked_ = 0;
    
    x_control1 = 0.0f;
    x_control2 = 1.0f;
    y_control1 = 0.0f;
    y_control2 = 1.0f;
    z_control1 = 0.0;
    z_control2 = 1.0f;

    bx_control1 = false;
    bx_control2 = false;
    by_control1 = false;
    by_control2 = false;
    bz_control1 = false;
    bz_control2 = false;

    addProperty(&sliceColor_);
    addProperty(&x_lock_);
    addProperty(&y_lock_);
    addProperty(&z_lock_);

    GroupProp* group = new GroupProp("group.clipping_locks", "Lock Clipping Planes");
    addProperty(group);

    x_lock_.setGrouped("group.clipping_locks");
    y_lock_.setGrouped("group.clipping_locks");
    z_lock_.setGrouped("group.clipping_locks");

    shift_lock_ = false;

    setLightParams();

    arrowDisplayList_ = 0;
	
    createCoProcessorOutport("coprocessor.clipping", &Processor::call);
	createCoProcessorInport("coprocessor.proxygeometry");
    setIsCoprocessor(true);
}

ClippingWidget::~ClippingWidget()
{
    if (arrowDisplayList_)
        glDeleteLists(arrowDisplayList_, 1);
	MsgDistr.postMessage(new TemplateMessage<tgt::EventListener*>(VoreenPainter::removeMouseListener_,
				    (tgt::EventListener*)this));
}

const std::string ClippingWidget::getProcessorInfo() const {
	return "3D-Widget to manipulate the clipping planes.";
}

Processor* ClippingWidget::create() {
	ClippingWidget* cw = new ClippingWidget();
		MsgDistr.postMessage(
            new TemplateMessage<tgt::EventListener*>(VoreenPainter::addMouseListener_,
            (tgt::EventListener*)cw));
		return cw;
}

void ClippingWidget::processMessage(Message* msg, const Identifier& /*dest*/)
{
    if (msg->id_ == "set.ClipSliceColor"){
        sliceColor_.set(msg->getValue<tgt::vec4>());
    }
    else if (msg->id_ == "set.polyOffsetFact"){
        polyOffsetFact_.set(msg->getValue<float>());
    }
    else if (msg->id_ == "set.polyOffsetUnit"){
        polyOffsetUnit_.set(msg->getValue<float>());
    }
    else if (msg->id_ == "switch.ClipXLock"){
        x_lock_.set(msg->getValue<bool>());
    }
    else if (msg->id_ == "switch.ClipYLock"){
        y_lock_.set(msg->getValue<bool>());
    }
    else if (msg->id_ == "switch.ClipZLock"){
        z_lock_.set(msg->getValue<bool>());
    }
    else if (msg->id_ == ProxyGeometry::resetClipPlanes_) {
        x_control1 = 0.0f;
        x_control2 = 1.0f;
        y_control1 = 0.0f;
        y_control2 = 1.0f;
        z_control1 = 0.0;
        z_control2 = 1.0f;
        x_lock_.set(false);
        y_lock_.set(false);
        z_lock_.set(false);
        invalidate();
    }
}

void ClippingWidget::saveMatrixStacks() {
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    LGL_ERROR;
	glLoadIdentity();
	tgt::multMatrix(camera_->getViewMatrix());
}

void ClippingWidget::restoreMatrixStacks() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;
}

void ClippingWidget::render(LocalPortMapping* portMapping) {
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

	glPushAttrib(GL_LIGHTING_BIT);

    IDManager id1;
    if( volumeSizeValid_ )
    {
        glEnable(GL_LIGHTING);

        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        
        glDisable(GL_LIGHT0); 
        glLightfv(GL_LIGHT3,GL_POSITION,light_pos);
        glLightfv(GL_LIGHT3,GL_AMBIENT,light_ambient);
        glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
        glLightfv(GL_LIGHT3,GL_SPECULAR,light_specular);
        glEnable(GL_LIGHT3);

        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,light_diffuse);
        glMaterialf( GL_FRONT_AND_BACK,	GL_SHININESS,	re_plastic_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_AMBIENT,		re_plastic_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_DIFFUSE,		re_plastic_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_SPECULAR,	re_plastic_specular);

		//X-Control1 rendering
		glColor3f(1.0f,0.0f,0.0f);
        paintBoundingGeometry(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),volumeSize_.y,volumeSize_.z , 4);
		id1.startBufferRendering("clipp.x-contr1");
        paintBoundingGeometry(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),volumeSize_.y,volumeSize_.z , 4);
		id1.stopBufferRendering();
        paintBoundingGeometry(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),-volumeSize_.y,-volumeSize_.z,3);
		id1.startBufferRendering("clipp.x-contr1");
        paintBoundingGeometry(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),-volumeSize_.y,-volumeSize_.z,3);
		id1.stopBufferRendering();
        if (bx_control1)
            renderQuad(1);
        glEnable(GL_LIGHTING);

        //X-Control2 rendering
        glEnable(GL_LIGHTING);
        glColor3f(1.0f,0.0f,0.0f);
		paintBoundingGeometry(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),volumeSize_.y,volumeSize_.z,4);
		id1.startBufferRendering("clipp.x-contr2");
        paintBoundingGeometry(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),volumeSize_.y,volumeSize_.z,4);
		id1.stopBufferRendering();
		paintBoundingGeometry(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),-volumeSize_.y,-volumeSize_.z,3);
		id1.startBufferRendering("clipp.x-contr2");
        paintBoundingGeometry(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),-volumeSize_.y,-volumeSize_.z,3);
		id1.stopBufferRendering();
        if (bx_control2)
            renderQuad(2);
        glEnable(GL_LIGHTING);

        if(bx_control1 && bx_control2){
            glLineWidth(3.0f);
            glBegin(GL_LINES);
            glVertex3f(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),volumeSize_.y + 0.09f,volumeSize_.z);
            glVertex3f(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),volumeSize_.y + 0.09f,volumeSize_.z);

            glVertex3f(-volumeSize_.x + (x_control1 * 2 * volumeSize_.x),-volumeSize_.y - 0.09f,-volumeSize_.z);
            glVertex3f(-volumeSize_.x + (x_control2 * 2 * volumeSize_.x),-volumeSize_.y - 0.09f,-volumeSize_.z);
            glEnd();
            glLineWidth(1.0f);
        }

        glMaterialf( GL_FRONT_AND_BACK,	GL_SHININESS,	gr_plastic_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_AMBIENT,		gr_plastic_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_DIFFUSE,		gr_plastic_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_SPECULAR,	gr_plastic_specular);

		//Y-Control1 rendering
		glColor3f(0.0f,1.0f,0.0f);
        paintBoundingGeometry(volumeSize_.x,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),volumeSize_.z, 2);
		id1.startBufferRendering("clipp.y-contr1");
		paintBoundingGeometry(volumeSize_.x,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),volumeSize_.z, 2);
		id1.stopBufferRendering();
        paintBoundingGeometry(-volumeSize_.x,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),-volumeSize_.z, 5);
		id1.startBufferRendering("clipp.y-contr1");
		paintBoundingGeometry(-volumeSize_.x,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),-volumeSize_.z, 5);
		id1.stopBufferRendering();
        if (by_control1)
			renderQuad(3);
        glEnable(GL_LIGHTING);


        //Y-Control2 rendering
        glColor3f(0.0f,1.0f,0.0f);
		paintBoundingGeometry(volumeSize_.x,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),volumeSize_.z,2);
		id1.startBufferRendering("clipp.y-contr2");
        paintBoundingGeometry(volumeSize_.x,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),volumeSize_.z,2);
		id1.stopBufferRendering();
		paintBoundingGeometry(-volumeSize_.x,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),-volumeSize_.z,5);
		id1.startBufferRendering("clipp.y-contr2");
		paintBoundingGeometry(-volumeSize_.x,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),-volumeSize_.z,5);
		id1.stopBufferRendering();
        if (by_control2)
            renderQuad(4);
        glEnable(GL_LIGHTING);

        if (by_control1 && by_control2) {
            glLineWidth(3.0f);
            glBegin(GL_LINES);
            glVertex3f(volumeSize_.x + 0.09f,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),volumeSize_.z);
            glVertex3f(volumeSize_.x + 0.09f,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),volumeSize_.z);

            glVertex3f(-volumeSize_.x,-volumeSize_.y + (y_control1 * 2 * volumeSize_.y),-volumeSize_.z - 0.09f);
            glVertex3f(-volumeSize_.x,-volumeSize_.y + (y_control2 * 2 * volumeSize_.y),-volumeSize_.z - 0.09f);
            glEnd();
            glLineWidth(1.0f);
        }

        glMaterialf( GL_FRONT_AND_BACK,	GL_SHININESS,	bl_plastic_shininess);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_AMBIENT,		bl_plastic_ambient);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_DIFFUSE,		bl_plastic_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK,	GL_SPECULAR,	bl_plastic_specular);

		//Z-Control1 rendering
		glColor3f(0.0f,0.0f,1.0f);
		paintBoundingGeometry(volumeSize_.x,-volumeSize_.y,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z), 3);
		id1.startBufferRendering("clipp.z-contr1");
        paintBoundingGeometry(volumeSize_.x,-volumeSize_.y,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z), 3);
		id1.stopBufferRendering();
		paintBoundingGeometry(-volumeSize_.x,volumeSize_.y,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z), 1);
		id1.startBufferRendering("clipp.z-contr1");
		paintBoundingGeometry(-volumeSize_.x,volumeSize_.y,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z), 1);
		id1.stopBufferRendering();
        if(bz_control1)
            renderQuad(5);
        glEnable(GL_LIGHTING);

        //Z-Control2 rendering
        glColor3f(0.0f,0.0f,1.0f);
		paintBoundingGeometry(volumeSize_.x,-volumeSize_.y,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z), 3);
		id1.startBufferRendering("clipp.z-contr2");
		paintBoundingGeometry(volumeSize_.x,-volumeSize_.y,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z), 3);
		id1.stopBufferRendering();
        paintBoundingGeometry(-volumeSize_.x,volumeSize_.y,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z), 1);
		id1.startBufferRendering("clipp.z-contr2");
		paintBoundingGeometry(-volumeSize_.x,volumeSize_.y,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z), 1);
		id1.stopBufferRendering();
        if (bz_control2)
            renderQuad(6);

        if (bz_control1 && bz_control2) {
            glLineWidth(3.0f);
            glBegin(GL_LINES);
            glVertex3f(volumeSize_.x,-volumeSize_.y - 0.09f,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z));
            glVertex3f(volumeSize_.x,-volumeSize_.y - 0.09f,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z));

            glVertex3f(-volumeSize_.x - 0.09f,volumeSize_.y,-volumeSize_.z + (z_control1 * 2 * volumeSize_.z));
            glVertex3f(-volumeSize_.x - 0.09f,volumeSize_.y,-volumeSize_.z + (z_control2 * 2 * volumeSize_.z));
            glEnd();
            glLineWidth(1.0f);
        }
	}

    glPopAttrib();
}

void ClippingWidget::renderQuad(int number) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
    glColor4fv(sliceColor_.get().elem);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Determine wether the quad's surface normal points towards the camera's half-space.
    // The surface normal is considered to point to the outside of the volume.
    tgt::vec3 camPos = getCamera()->getPosition();
    bool frontFace;
    switch (number) {
        case 1: {
            // left clipping plane
            frontFace = ( camPos.x < volumeSize_.x*(x_control1 * 2 - 1) );
            break;
        }
        case 2: {
            // right clipping plane
            frontFace = ( camPos.x >= volumeSize_.x*(x_control2 * 2 - 1) );
            break;
        }
        case 3: {
            // back clipping plane
            frontFace = ( camPos.y < volumeSize_.y*(y_control1 * 2 - 1) );
            break;
        }
        case 4: {
            // front clipping plane
            frontFace = ( camPos.y >= volumeSize_.y*(y_control2 * 2 - 1) );
            break;
        }
        case 5: {
            // bottom clipping plane
            frontFace = ( camPos.z < volumeSize_.z*(z_control1 * 2 - 1) );
            break;
        }
        case 6: {
            // top clipping plane
            frontFace = ( camPos.z >= volumeSize_.z*(z_control2 * 2 - 1) );
            break;
        }
    }

    // If we are looking at the front face of the clipping quad, we
    // need a negative offset, since we want to shift the quad towards the camera.
    // Otherwise the offset has to be positive in order to shift the quad away from the camera.
    if (frontFace) {
        glPolygonOffset(-polyOffsetFact_.get(), -polyOffsetUnit_.get());
    }
    else {
        glPolygonOffset(polyOffsetFact_.get(), polyOffsetUnit_.get());
    }

    glBegin(GL_QUADS);
    switch (number) {
        case 1: {
            glVertex3f(volumeSize_.x * (x_control1 * 2 - 1),  volumeSize_.y,  volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control1 * 2 - 1),  volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control1 * 2 - 1), -volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control1 * 2 - 1), -volumeSize_.y,  volumeSize_.z);
            break;
        }
        case 2: {
            glVertex3f(volumeSize_.x * (x_control2 * 2 - 1),  volumeSize_.y,  volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control2 * 2 - 1),  volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control2 * 2 - 1), -volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (x_control2 * 2 - 1), -volumeSize_.y,  volumeSize_.z);
            break;
        }
        case 3: {
            glVertex3f( volumeSize_.x, volumeSize_.y * (y_control1 * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (y_control1 * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (y_control1 * 2 - 1), -volumeSize_.z);
            glVertex3f( volumeSize_.x, volumeSize_.y * (y_control1 * 2 - 1), -volumeSize_.z);
            break;
        }
        case 4: {
            glVertex3f( volumeSize_.x, volumeSize_.y * (y_control2 * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (y_control2 * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (y_control2 * 2 - 1), -volumeSize_.z);
            glVertex3f( volumeSize_.x, volumeSize_.y * (y_control2 * 2 - 1), -volumeSize_.z);
            break;
        }
        case 5: {
            glVertex3f( volumeSize_.x,  volumeSize_.y, volumeSize_.z * (z_control1 * 2 - 1));
            glVertex3f(-volumeSize_.x,  volumeSize_.y, volumeSize_.z * (z_control1 * 2 - 1));
            glVertex3f(-volumeSize_.x, -volumeSize_.y, volumeSize_.z * (z_control1 * 2 - 1));
            glVertex3f( volumeSize_.x, -volumeSize_.y, volumeSize_.z * (z_control1 * 2 - 1));
            break;
        }
        case 6: {
            glVertex3f( volumeSize_.x,  volumeSize_.y, volumeSize_.z * (z_control2 * 2 - 1));
            glVertex3f(-volumeSize_.x,  volumeSize_.y, volumeSize_.z * (z_control2 * 2 - 1));
            glVertex3f(-volumeSize_.x, -volumeSize_.y, volumeSize_.z * (z_control2 * 2 - 1));
            glVertex3f( volumeSize_.x, -volumeSize_.y, volumeSize_.z * (z_control2 * 2 - 1));
            break;
        }
    }
    glEnd();
    glPolygonOffset(0.f, 0.f);
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

}

void ClippingWidget::mousePressEvent(tgt::MouseEvent *e)
{
	isClicked_ = isClicked(e->coord().x, e->coord().y);
	if (isClicked_) {

        e->accept();

        x_prev = e->coord().x;
		y_prev = e->coord().y;

        switch(isClicked_)
        {
        case 1 :
            bx_control1 = true;
            if( x_lock_.get() ) {
                bx_control2=true;
            }
            break;
        case 2 :
            bx_control2 = true;
            if( x_lock_.get() ) {
                bx_control1=true;
            }
            break;
        case 3 :
            by_control1=true;
            if( y_lock_.get() ) {
                by_control2=true;
            }
            break;
        case 4 :
            by_control2=true;
            if( y_lock_.get() ) {
                by_control1=true;
            }
            break;
        case 5 :
            bz_control1 = true;
            if( z_lock_.get() ) {
                bz_control2=true;
            }
            break;
        case 6 :
            bz_control2=true;
            if( z_lock_.get() ) {
                bz_control1=true;
            }
            break;
        }
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true), VoreenPainter::visibleViews_);
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else e->ignore();
    if (isClicked_ && e->modifiers() & (tgt::Event::LSHIFT | tgt::Event::RSHIFT)){
        shift_lock_ = true;
        if(isClicked_ == 1 || isClicked_ == 2){
            x_lock_.set(!x_lock_.get());
            e->accept();
            return;
        }
        if(isClicked_ == 3 || isClicked_ == 4){
            y_lock_.set(!y_lock_.get());
            e->accept();
            return;
        }
        if(isClicked_ == 5 || isClicked_ == 6){
            z_lock_.set(!z_lock_.get());
            e->accept();
            return;
        }
    }

}

void ClippingWidget::mouseMoveEvent(tgt::MouseEvent *e)
{
	if(isClicked_)
    {
        switch(isClicked_)
        {
        case 1 :
            posX = volumeSize_.x * ( x_control1 * 2 - 1);
            posY = volumeSize_.y;
            posZ = volumeSize_.z;
            break;
        case 2 :
            posX = volumeSize_.x * ( x_control2 * 2 - 1);
            posY = volumeSize_.y;
            posZ =volumeSize_.z;
          break;
        case 3 :
            posX = volumeSize_.x;
            posY = volumeSize_.y * ( y_control1 * 2 - 1);
            posZ = volumeSize_.z;
            break;
        case 4 :
            posX = volumeSize_.x;
            posY = volumeSize_.y * ( y_control2 * 2 - 1);
            posZ = volumeSize_.z;
            break;
        case 5 :
            posX = volumeSize_.x;
            posY = volumeSize_.y;
            posZ = volumeSize_.z * ( z_control1 * 2 - 1);
            break;
        case 6 :
            posX = volumeSize_.x;
            posY = volumeSize_.y;
            posZ = volumeSize_.z * ( z_control2 * 2 - 1);
            break;
        }
        e->accept();
		saveMatrixStacks();

		GLint deltaX, deltaY;

		GLint viewport[4];
        GLdouble modelview[16];
        GLdouble projection[16];
        GLdouble winX, winY, winZ;
		GLdouble posXh, posYh, posZh;

		deltaX = e->coord().x - x_prev;
		deltaY = y_prev - e->coord().y;

        //FIXME: must make sure that the correct canvas is current! (by calling makeCurrent())
        glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
        glGetDoublev( GL_PROJECTION_MATRIX, projection );
        glGetIntegerv( GL_VIEWPORT, viewport );

        gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);


        winX = winX + deltaX;
        winY = winY + deltaY;

        gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posXh, &posYh, &posZh);

        x_prev = e->coord().x;
        y_prev = e->coord().y;
        float temp;

        restoreMatrixStacks();
        switch(isClicked_)
        {
        case 1 :
            temp = x_control1;
            x_control1 = static_cast<float>(posXh) / (2.0f*volumeSize_.x) + 0.5f;
            if (x_control1 > 1.0f) x_control1 = 1.0f;
            if (x_control1 < 0.0f) x_control1 = 0.0f;
            if (x_control1 > x_control2) x_control2 = x_control1;
            bx_control1 = true;
            if( x_lock_.get() ) {
                x_control2 += x_control1 - temp;
                if (x_control2 > 1.0f) {
                    x_control1 -= x_control2 - 1.f;
                    x_control2 = 1.0f;
                }
                if (x_control2 < 0.0f) {
                    x_control1 -= x_control2;
                    x_control2 = 0.0f;
                }
                MsgDistr.postMessage(new IntMsg(ProxyGeometry::setRightClipPlane_, static_cast<int>(100 - (x_control2*100))));
                bx_control2=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setLeftClipPlane_, static_cast<int>(x_control1*100)));
            break;
        case 2 :
            temp = x_control2;
            x_control2 = static_cast<float>(posXh) / (2*volumeSize_.x) + 0.5f;
            if (x_control2 > 1.0) x_control2 = 1.0f;
            if (x_control2 < 0.0) x_control2 = 0.0f;
            if (x_control1 > x_control2) x_control1 = x_control2;
            bx_control2 = true;
            if(x_lock_.get()) {
                x_control1 += x_control2 - temp;
                 if (x_control1 > 1.0f) {
                    x_control2 -= x_control1 - 1.f;
                    x_control1 = 1.0f;
                }
                if (x_control1 < 0.0f) {
                    x_control2 -= x_control1;
                    x_control1 = 0.0f;
                }MsgDistr.postMessage(new IntMsg(ProxyGeometry::setLeftClipPlane_, static_cast<int>(x_control1*100)));
                bx_control1=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setRightClipPlane_, static_cast<int>(100 - (x_control2*100))));
            break;
        case 3 :
            temp = y_control1;
            y_control1 = static_cast<float>(posYh) / (2*volumeSize_.y) + 0.5f;
            if (y_control1 > 1.0) y_control1 = 1.0f;
            if (y_control1 < 0.0) y_control1 = 0.0f;
            if (y_control1 > y_control2) y_control2 = y_control1;
            by_control1=true;
            if (y_lock_.get()) {
                y_control2 += y_control1 - temp;
                if (y_control2 > 1.0f) {
                    y_control1 -= y_control2 - 1.f;
                    y_control2 = 1.0f;
                }
                if (y_control2 < 0.0f) {
                    y_control1 -= y_control2;
                    y_control2 = 0.0f;
                }
                MsgDistr.postMessage(new IntMsg(ProxyGeometry::setTopClipPlane_, static_cast<int>(100 - (y_control2*100))));
                by_control2=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBottomClipPlane_, static_cast<int>(y_control1*100)));
            break;
        case 4 :
            temp = y_control2;
            y_control2 = static_cast<float>(posYh) / (2*volumeSize_.y) + 0.5f;
            if (y_control2 > 1.0) y_control2 = 1.0f;
            if (y_control2 < 0.0) y_control2 = 0.0f;
            if (y_control1 > y_control2) y_control1 = y_control2;
            by_control2=true;
            if (y_lock_.get()) {
                y_control1 += y_control2 - temp;
                if (y_control1 > 1.0f) {
                    y_control2 -= y_control1 - 1.f;
                    y_control1 = 1.0f;
                }
                if (y_control1 < 0.0f) {
                    y_control2 -= y_control1;
                    y_control1 = 0.0f;
                }
                MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBottomClipPlane_, static_cast<int>(y_control1*100)));
                by_control1=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setTopClipPlane_, static_cast<int>(100 - (y_control2*100))));
            break;
        case 5 :
            temp = z_control1;
            z_control1 = static_cast<float>(posZh) / (2*volumeSize_.z) + 0.5f;
            if (z_control1 > 1.0) z_control1 = 1.0f;
            if (z_control1 < 0.0) z_control1 = 0.0f;
            if (z_control1 > z_control2) z_control2 = z_control1;
            bz_control1 = true;
            if (z_lock_.get()) {
                z_control2 += z_control1 - temp;
                if (z_control2 > 1.0f) {
                    z_control1 -= z_control2 - 1.f;
                    z_control2 = 1.0f;
                }
                if (z_control2 < 0.0f) {
                    z_control1 -= z_control2;
                    z_control2 = 0.0f;
                }
                MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBackClipPlane_, static_cast<int>(100 - (z_control2*100))));
                bz_control2=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setFrontClipPlane_, static_cast<int>(z_control1*100)));
            break;
        case 6 :
            temp = z_control2;
            z_control2 = static_cast<float>(posZh) / (2*volumeSize_.z) + 0.5f;
            if (z_control2 > 1.0) z_control2 = 1.0f;
            if (z_control2 < 0.0) z_control2 = 0.0f;
            if (z_control1 > z_control2) z_control1 = z_control2;
            bz_control2=true;
            if (z_lock_.get()) {
                z_control1 += z_control2 - temp;
                if (z_control1 > 1.0f) {
                    z_control2 -= z_control1 - 1.f;
                    z_control1 = 1.0f;
                }
                if (z_control1 < 0.0f) {
                    z_control2 -= z_control1;
                    z_control1 = 0.0f;
                }
                MsgDistr.postMessage(new IntMsg(ProxyGeometry::setFrontClipPlane_, static_cast<int>(z_control1*100)));
                bz_control1=true;
            }
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBackClipPlane_, static_cast<int>(100 - (z_control2*100))));
            break;
        }
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else e->ignore();
}

void ClippingWidget::mouseDoubleClickEvent(tgt::MouseEvent *e){
    isClicked_ = isClicked(e->coord().x, e->coord().y);
    if(isClicked_ == 1 || isClicked_ == 2){
        x_lock_.set(!x_lock_.get());
        e->accept();
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), VoreenPainter::visibleViews_);
        return;
    }
    if(isClicked_ == 3 || isClicked_ == 4){
        y_lock_.set(!y_lock_.get());
        e->accept();
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), VoreenPainter::visibleViews_);
        return;
    }
    if(isClicked_ == 5 || isClicked_ == 6){
        z_lock_.set(!z_lock_.get());
        e->accept();
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), VoreenPainter::visibleViews_);
        return;
    }
    e->ignore();
    isClicked_=0;
}

void ClippingWidget::mouseReleaseEvent(tgt::MouseEvent *e)
{
	if(isClicked_){
		isClicked_ = 0;
        bx_control1=false;
        bx_control2=false;
        by_control1=false;
        by_control2=false;
        bz_control1=false;
        bz_control2=false;
        if(shift_lock_){
            x_lock_.set(false);
            y_lock_.set(false);
            z_lock_.set(false);
            shift_lock_ = false;
        }
	}
	e->ignore();
}

void ClippingWidget::paintBoundingGeometry(float trans_x,float trans_y,float trans_z, int direction)
{
    //should be used to render new sliders
    //glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(trans_x, trans_y, trans_z);
    switch(direction){
        case 1 : //x-direction
            glTranslatef(-0.05f,0.0f,0.0f);
            glRotatef(90,0.0f,1.0f,0.0f);
            break;
        case 2 : //neg x-direction
            glTranslatef(0.05f,0.0f,0.0f);
            glRotatef(-90,0.0f,1.0f,0.0f);
            break;
        case 3 : //y-direction
            glTranslatef(0.0f,-0.05f,0.0f);
            glRotatef(-90,1.0f,0.0f,0.0f);
            break;
        case 4 : //neg y-direction
            glTranslatef(0.0f,0.05f,0.0f);
            glRotatef(90,1.0f,0.0f,0.0f);
            break;
        case 5 : //z-direction
            glTranslatef(0.0f,0.0f,-0.05f);
            break;
        case 6 : //neg z-direction
            glTranslatef(0.0f,0.0f,0.05f);
            glRotatef(180,1.0f,0.0f,0.0f);
            break;
    }

    if (!arrowDisplayList_) {
        GLUquadricObj* quadric = gluNewQuadric();
        arrowDisplayList_ = glGenLists(1);
        glNewList(arrowDisplayList_, GL_COMPILE);
        gluCylinder(quadric, 0.025f, 0.0f, 0.05f, 30, 10);
        glTranslatef(0.0f,0.0f,-0.035f);
        gluCylinder(quadric, 0.01f, 0.01f, 0.06f, 20, 2);
        glEndList();
        gluDeleteQuadric(quadric);
    }
    glCallList(arrowDisplayList_);
    glPopMatrix();
}

int ClippingWidget::isClicked(int x, int y)
{
	IDManager id1;
	if(id1.isClicked("clipp.x-contr1", x, tc_->getSize().y - y)) return 1;
	if(id1.isClicked("clipp.x-contr2", x, tc_->getSize().y - y)) return 2;
	if(id1.isClicked("clipp.y-contr1", x, tc_->getSize().y - y)) return 3;
	if(id1.isClicked("clipp.y-contr2", x, tc_->getSize().y - y)) return 4;
	if(id1.isClicked("clipp.z-contr1", x, tc_->getSize().y - y)) return 5;
	if(id1.isClicked("clipp.z-contr2", x, tc_->getSize().y - y)) return 6;
	return 0;
}

void ClippingWidget::setLightParams(){
    //Light parameter
    light_pos[0] = 0.0f;
    light_pos[1] = 1.0f;
    light_pos[2] = 1.1f;
    light_pos[3] = 1.0f;
    light_pos2[0] = 0.0f;
    light_pos2[1] = -1.0f;
    light_pos2[2] = -1.1f;
    light_pos2[3] = 1.0f;
    light_ambient[0] = 0.6f;
    light_ambient[1] = 0.6f;
    light_ambient[2] = 0.6f;
    light_ambient[3] = 1.0f;
    light_diffuse[0] = 0.9f;
    light_diffuse[1] = 0.9f;
    light_diffuse[2] = 0.9f;
    light_diffuse[3] = 1.0f;
    light_specular[0] = 1.0f;
    light_specular[1] = 1.0f;
    light_specular[2] = 1.0f;
    light_specular[3] = 1.0f;

    //Material green plastic
    gr_plastic_ambient[0]	= 0.0f;
    gr_plastic_ambient[1]	= 0.3f;
    gr_plastic_ambient[2]	= 0.0f;
    gr_plastic_ambient[3]	= 1.0f;
    gr_plastic_diffuse[0]	= 0.1f;
    gr_plastic_diffuse[1]	= 0.5f;
    gr_plastic_diffuse[2]	= 0.1f;
    gr_plastic_diffuse[3]	= 1.0f;
    gr_plastic_specular[0]	= 0.45f;
    gr_plastic_specular[1]	= 0.55f;
    gr_plastic_specular[2]	= 0.45f;
    gr_plastic_specular[3]	= 1.0f;
    gr_plastic_shininess	= 25.0f;

    //Material red plastic
    re_plastic_ambient[0]	= 0.3f;
    re_plastic_ambient[1]	= 0.0f;
    re_plastic_ambient[2]	= 0.0f;
    re_plastic_ambient[3]	= 1.0f;
    re_plastic_diffuse[0]	= 0.5f;
    re_plastic_diffuse[1]	= 0.0f;
    re_plastic_diffuse[2]	= 0.0f;
    re_plastic_diffuse[3]	= 1.0f;
    re_plastic_specular[0]	= 0.7f;
    re_plastic_specular[1]	= 0.6f;
    re_plastic_specular[2]	= 0.6f;
    re_plastic_specular[3]	= 1.0f;
    re_plastic_shininess	= 25.0f;

    //Material blue plastic
    bl_plastic_ambient[0]	= 0.0f;
    bl_plastic_ambient[1]	= 0.0f;
    bl_plastic_ambient[2]	= 0.3f;
    bl_plastic_ambient[3]	= 1.0f;
    bl_plastic_diffuse[0]	= 0.0f;
    bl_plastic_diffuse[1]	= 0.3f;
    bl_plastic_diffuse[2]	= 0.509f;
    bl_plastic_diffuse[3]	= 1.0f;
    bl_plastic_specular[0]	= 0.501f;
    bl_plastic_specular[1]	= 0.501f;
    bl_plastic_specular[2]	= 0.501f;
    bl_plastic_specular[3]	= 1.0f;
    bl_plastic_shininess	= 25.0f;
}

} //namespace
