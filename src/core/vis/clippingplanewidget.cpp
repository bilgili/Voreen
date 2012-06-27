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

#include "voreen/core/vis/clippingplanewidget.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/voreenpainter.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"
#include "tgt/event/eventlistener.h"

#include <typeinfo>

namespace voreen {

ClippingPlaneWidget::ControlElement::ControlElement(tgt::vec3 val, std::string n,
                                               bool act, tgt::vec3 trans, tgt::vec3 rotAxis,
                                               float rotAngle, tgt::vec3 edge1,
                                               tgt::vec3 edge2, tgt::vec3 col)
{
    clipValue = val;
    name = n;
    activated = act;
    translation = trans;
    rotationAxis = rotAxis;
    rotationAngle = rotAngle;
    adjacentEdge1 = edge1;
    adjacentEdge2 = edge2;
    color = col;
}

ClippingPlaneWidget::ClippingPlaneWidget()
    : GeometryRenderer()
    , tgt::EventListener()
    , isClicked_(0)
    , arrowDisplayList_(0)
    , shift_lock_(false)
    , volumeSizeValid_(false)
    , sliceColor_("set.ClipSliceColor", "Color for Clipping Slices", tgt::vec4(0.18f, 0.68f, 1.0f, 0.3f))
    , polyOffsetFact_("set.polyOffsetFact", "PolygonOffset Factor", 20.f, 0.f, 100.f, false)
    , polyOffsetUnit_("set.polyOffsetUnit", "PolygonOffset Unit", 10.0f, 0.f, 100.f, false)
    , x_lock_("switch.ClipXLock","Lock X-Clipping planes", false)
    , y_lock_("switch.ClipYLock","Lock Y-Clipping planes", false)
    , z_lock_("switch.ClipZLock","Lock Z-Clipping planes", false)
    , eventProp_("Clipplane movement", tgt::Event::NONE, tgt::MouseEvent::MOUSE_BUTTON_LEFT)
    , syncEventProp_("Syncronized movement", tgt::Event::SHIFT, tgt::MouseEvent::MOUSE_BUTTON_LEFT)
{
    setName("ClippingPlaneWidget");

    clipValueLeft_ =   0.f;
    clipValueRight_ =  1.f;
    clipValueBottom_ = 0.f;
    clipValueTop_ =    1.f;
    clipValueFront_ =  0.f;
    clipValueBack_ =   1.f;

    //init controlElements
    ControlElement element = ControlElement(tgt::vec3(2.f*clipValueLeft_-1.f, 1.f, 1.f), "leftClip control 1",
                                            false, tgt::vec3(-0.05f, 0.0f, 0.0f),
                                            tgt::vec3(0.0f, 1.0f, 0.0f), 90.f, tgt::vec3(1.f, 1.f, 1.f),
                                            tgt::vec3(-1.f, 1.f, 1.f), tgt::vec3(1.f, 0.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(2.f*clipValueLeft_-1.f, -1.f, -1.f), "leftClip control 2",
                             false, tgt::vec3(-0.05f, -0.0f, 0.0f),
                             tgt::vec3(0.0f, 1.0f, 0.0f), 90.f, tgt::vec3(1.f, -1.f, -1.f),
                             tgt::vec3(-1.f, -1.f, -1.f), tgt::vec3(1.f, 0.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(2.f*clipValueRight_-1.f, 1.f, 1.f), "rightClip control 1",
                             false, tgt::vec3(0.05f, 0.0f, 0.0f),
                             tgt::vec3(0.0f, 1.0f, 0.0f), -90.f, tgt::vec3(1.f, 1.f, 1.f),
                             tgt::vec3(-1.f, 1.f, 1.f), tgt::vec3(1.f, 0.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(2.f*clipValueRight_-1.f, -1.f, -1.f), "rightClip control 2",
                             false, tgt::vec3(0.05f, 0.0f, 0.0f),
                             tgt::vec3(0.0f, 1.0f, 0.0f), -90.f,  tgt::vec3(1.f, -1.f, -1.f),
                             tgt::vec3(-1.f, -1.f, -1.f), tgt::vec3(1.f, 0.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(1.f, 2.f*clipValueBottom_-1.f, 1.f), "bottomClip control 1",
                             false, tgt::vec3(0.0f, -0.05f, 0.0f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), -90.f, tgt::vec3(1.f, -1.f, 1.f),
                             tgt::vec3(1.f, 1.f, 1.f), tgt::vec3(0.f, 1.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(-1.f, 2.f*clipValueBottom_-1.f, -1.f), "bottomClip control 2",
                             false, tgt::vec3(0.0f, -0.05f, 0.0f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), -90.f, tgt::vec3(-1.f, -1.f, -1.f),
                             tgt::vec3(-1.f, 1.f, -1.f), tgt::vec3(0.f, 1.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(1.f, 2.f*clipValueTop_-1.f, 1.f), "topClip control 1",
                             false, tgt::vec3(0.0f, 0.05f, 0.0f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 90.f, tgt::vec3(1.f, -1.f, 1.f),
                             tgt::vec3(1.f, 1.f, 1.f), tgt::vec3(0.f, 1.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(-1.f, 2.f*clipValueTop_-1.f, -1.f), "topClip control 2",
                             false, tgt::vec3(0.0f, 0.05f, 0.0f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 90.f, tgt::vec3(-1.f, -1.f, -1.f),
                             tgt::vec3(-1.f, 1.f, -1.f), tgt::vec3(0.f, 1.f, 0.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(1.f, -1.f, 2.f*clipValueFront_-1.f), "frontClip control 1",
                             false, tgt::vec3(0.0f, 0.0f, -0.05f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 0.f, tgt::vec3(1.f, -1.f, -1.f),
                             tgt::vec3(1.f, -1.f, 1.f), tgt::vec3(0.f, 0.f, 1.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(-1.f, 1.f, 2.f*clipValueFront_-1.f), "frontClip control 2",
                             false, tgt::vec3(0.0f, 0.0f, -0.05f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 0.f, tgt::vec3(-1.f, 1.f, -1.f),
                             tgt::vec3(-1.f, 1.f, 1.f), tgt::vec3(0.f, 0.f, 1.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(1.f, -1.f, 2.f*clipValueBack_-1.f), "backClip control 1",
                             false, tgt::vec3(0.0f, 0.0f, 0.05f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 180.f, tgt::vec3(1.f, -1.f, -1.f),
                             tgt::vec3(1.f, -1.f, 1.f), tgt::vec3(0.f, 0.f, 1.f));
    controlElements_.push_back(element);
    element = ControlElement(tgt::vec3(-1.f, 1.f, 2.f*clipValueBack_-1.f), "backClip control 2",
                             false, tgt::vec3(0.0f, 0.0f, 0.05f),
                             tgt::vec3(1.0f, 0.0f, 0.0f), 180.f, tgt::vec3(-1.f, 1.f, -1.f),
                             tgt::vec3(-1.f, 1.f, 1.f), tgt::vec3(0.f, 0.f, 1.f));
    controlElements_.push_back(element);

    IDManager id1;
    for (size_t i = 0; i < controlElements_.size(); ++i)
        id1.addNewPickObj(controlElements_[i].name);

    addProperty(&sliceColor_);
    addProperty(&x_lock_);
    addProperty(&y_lock_);
    addProperty(&z_lock_);
    addProperty(&eventProp_);
    addProperty(&syncEventProp_);

    initLightAndMaterial();

    createCoProcessorOutport("coprocessor.clipping", &Processor::call);
    createCoProcessorInport("coprocessor.proxygeometry");
    setIsCoprocessor(true);
}

ClippingPlaneWidget::~ClippingPlaneWidget() {
    if (arrowDisplayList_)
        glDeleteLists(arrowDisplayList_, 1);
}

const Identifier ClippingPlaneWidget::getClassName() const {
    return "GeometryRenderer.ClippingPlaneWidget";
}

const std::string ClippingPlaneWidget::getProcessorInfo() const {
    return "3D-ControlElements to manipulate the clipping planes.";
}

Processor* ClippingPlaneWidget::create() const {
    ClippingPlaneWidget* cw = new ClippingPlaneWidget();
    return cw;
}

void ClippingPlaneWidget::processMessage(Message* msg, const Identifier& dest) {
    GeometryRenderer::processMessage(msg, dest);

    if (msg->id_ == ProxyGeometry::resetClipPlanes_) {
        clipValueLeft_ =   0.f;
        clipValueRight_ =  1.f;
        clipValueBottom_ = 0.f;
        clipValueTop_ =    1.f;
        clipValueFront_ =  0.f;
        clipValueBack_ =   1.f;
        controlElements_[0].clipValue  = tgt::vec3(2.0f * clipValueLeft_ - 1.0f,   1.0f,  1.0f);
        controlElements_[1].clipValue  = tgt::vec3(2.0f * clipValueLeft_ - 1.0f,  -1.0f, -1.0f);
        controlElements_[2].clipValue  = tgt::vec3(2.0f * clipValueRight_ - 1.0f,  1.0f,  1.0f);
        controlElements_[3].clipValue  = tgt::vec3(2.0f * clipValueRight_ - 1.0f, -1.0f, -1.0f);
        controlElements_[4].clipValue  = tgt::vec3( 1.0f, 2.0f * clipValueBottom_ - 1.0f,  1.0f);
        controlElements_[5].clipValue  = tgt::vec3(-1.0f, 2.0f * clipValueBottom_ - 1.0f, -1.0f);
        controlElements_[6].clipValue  = tgt::vec3( 1.0f, 2.0f * clipValueTop_ - 1.0f,     1.0f);
        controlElements_[7].clipValue  = tgt::vec3(-1.0f, 2.0f * clipValueTop_ - 1.0f,    -1.0f);
        controlElements_[8].clipValue  = tgt::vec3( 1.0f, -1.0f, 2.0f * clipValueFront_ - 1.0f);
        controlElements_[9].clipValue  = tgt::vec3(-1.0f,  1.0f, 2.0f * clipValueFront_ - 1.0f);
        controlElements_[10].clipValue = tgt::vec3( 1.0f, -1.0f, 2.0f * clipValueBack_ - 1.0f);
        controlElements_[11].clipValue = tgt::vec3(-1.0f,  1.0f, 2.0f * clipValueBack_ - 1.0f);

        x_lock_.set(false);
        y_lock_.set(false);
        z_lock_.set(false);
        invalidate();
    }
}

void ClippingPlaneWidget::render(LocalPortMapping* portMapping) {
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

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    IDManager id1;
    if (volumeSizeValid_) {
        glEnable(GL_LIGHTING);

        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);

        glDisable(GL_LIGHT0);
        glLightfv(GL_LIGHT3, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT3, GL_AMBIENT,  light_ambient);
        glLightfv(GL_LIGHT3, GL_DIFFUSE,  light_diffuse);
        glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);
        glEnable(GL_LIGHT3);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, light_diffuse);

        bool lock;
        if (isClicked_ <= 4)
            lock = x_lock_.get();
        else if (isClicked_ <= 8)
            lock = y_lock_.get();
        else
            lock = z_lock_.get();
        bool oneInLast4Active = false;
        for (size_t i = 0; i < controlElements_.size(); ++i) {
            //change material after 4 controlElements
            if ((i+4)%4 == 0) {
                int index = i/4;
                glMaterialf( GL_FRONT_AND_BACK,    GL_SHININESS,    materials_[index].shininess);
                glMaterialfv(GL_FRONT_AND_BACK,    GL_AMBIENT,        materials_[index].ambient);
                glMaterialfv(GL_FRONT_AND_BACK,    GL_DIFFUSE,        materials_[index].diffuse);
                glMaterialfv(GL_FRONT_AND_BACK,    GL_SPECULAR,    materials_[index].specular);
            }
            glColor3fv(&controlElements_[i].color[0]);

            paintArrow(controlElements_[i].clipValue, controlElements_[i].translation,
                    controlElements_[i].rotationAxis, controlElements_[i].rotationAngle);
            id1.startBufferRendering(controlElements_[i].name);
            paintArrow(controlElements_[i].clipValue, controlElements_[i].translation,
                    controlElements_[i].rotationAxis, controlElements_[i].rotationAngle);
            id1.stopBufferRendering();
            if (controlElements_[i].activated) {
                renderQuad((i+2)/2);
                oneInLast4Active = true;
            }

            //paint lock lines
            if (oneInLast4Active && lock && ((i+1)%4 == 0)) {
                oneInLast4Active = false;
                tgt::vec3 point1 = controlElements_[i-3].clipValue*volumeSize_;
                tgt::vec3 point2 = controlElements_[i-1].clipValue*volumeSize_;

                tgt::vec3 point3 = controlElements_[i-2].clipValue*volumeSize_;
                tgt::vec3 point4 = controlElements_[i-0].clipValue*volumeSize_;

                glLineWidth(3.0f);
                glBegin(GL_LINES);
                    glVertex3fv(&point1[0]);
                    glVertex3fv(&point2[0]);

                    glVertex3fv(&point3[0]);
                    glVertex3fv(&point4[0]);
                glEnd();
                glLineWidth(1.0f);
            }
        }
    }
    glPopAttrib();
}

void ClippingPlaneWidget::renderQuad(int number) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_LIGHTING);
    glColor4fv(sliceColor_.get().elem);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Determine wether the quad's surface normal points towards the camera's half-space.
    // The surface normal is considered to point to the outside of the volume.
    tgt::vec3 camPos = getCamera()->getPosition();
    bool frontFace = false;
    switch (number) {
    case 1:
        // left clipping plane
        frontFace = ( camPos.x < volumeSize_.x*(clipValueLeft_ * 2 - 1) );
        break;
    case 2:
        // right clipping plane
        frontFace = ( camPos.x >= volumeSize_.x*(clipValueRight_ * 2 - 1) );
        break;
    case 3:
      // back clipping plane
        frontFace = ( camPos.y < volumeSize_.y*(clipValueBottom_ * 2 - 1) );
        break;
    case 4:
        // front clipping plane
        frontFace = ( camPos.y >= volumeSize_.y*(clipValueTop_ * 2 - 1) );
        break;
    case 5:
        // bottom clipping plane
        frontFace = ( camPos.z < volumeSize_.z*(clipValueFront_ * 2 - 1) );
        break;
    case 6:
        frontFace = ( camPos.z >= volumeSize_.z*(clipValueBack_ * 2 - 1) );
        break;
    }

    // If we are looking at the front face of the clipping quad, we
    // need a negative offset, since we want to shift the quad towards the camera.
    // Otherwise the offset has to be positive in order to shift the quad away from the camera.
    if (frontFace)
        glPolygonOffset(-polyOffsetFact_.get(), -polyOffsetUnit_.get());
    else
        glPolygonOffset(polyOffsetFact_.get(), polyOffsetUnit_.get());

    glBegin(GL_QUADS);
    switch (number) {
        case 1:
            glVertex3f(volumeSize_.x * (clipValueLeft_ * 2 - 1),  volumeSize_.y,  volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueLeft_ * 2 - 1),  volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueLeft_ * 2 - 1), -volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueLeft_ * 2 - 1), -volumeSize_.y,  volumeSize_.z);
            break;
        case 2:
            glVertex3f(volumeSize_.x * (clipValueRight_ * 2 - 1),  volumeSize_.y,  volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueRight_ * 2 - 1),  volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueRight_ * 2 - 1), -volumeSize_.y, -volumeSize_.z);
            glVertex3f(volumeSize_.x * (clipValueRight_ * 2 - 1), -volumeSize_.y,  volumeSize_.z);
            break;
        case 3:
            glVertex3f( volumeSize_.x, volumeSize_.y * (clipValueBottom_ * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (clipValueBottom_ * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (clipValueBottom_ * 2 - 1), -volumeSize_.z);
            glVertex3f( volumeSize_.x, volumeSize_.y * (clipValueBottom_ * 2 - 1), -volumeSize_.z);
            break;
        case 4:
            glVertex3f( volumeSize_.x, volumeSize_.y * (clipValueTop_ * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (clipValueTop_ * 2 - 1),  volumeSize_.z);
            glVertex3f(-volumeSize_.x, volumeSize_.y * (clipValueTop_ * 2 - 1), -volumeSize_.z);
            glVertex3f( volumeSize_.x, volumeSize_.y * (clipValueTop_ * 2 - 1), -volumeSize_.z);
            break;
        case 5:
            glVertex3f( volumeSize_.x,  volumeSize_.y, volumeSize_.z * (clipValueFront_ * 2 - 1));
            glVertex3f(-volumeSize_.x,  volumeSize_.y, volumeSize_.z * (clipValueFront_ * 2 - 1));
            glVertex3f(-volumeSize_.x, -volumeSize_.y, volumeSize_.z * (clipValueFront_ * 2 - 1));
            glVertex3f( volumeSize_.x, -volumeSize_.y, volumeSize_.z * (clipValueFront_ * 2 - 1));
            break;
        case 6:
            glVertex3f( volumeSize_.x,  volumeSize_.y, volumeSize_.z * (clipValueBack_ * 2 - 1));
            glVertex3f(-volumeSize_.x,  volumeSize_.y, volumeSize_.z * (clipValueBack_ * 2 - 1));
            glVertex3f(-volumeSize_.x, -volumeSize_.y, volumeSize_.z * (clipValueBack_ * 2 - 1));
            glVertex3f( volumeSize_.x, -volumeSize_.y, volumeSize_.z * (clipValueBack_ * 2 - 1));
            break;
    }
    glEnd();
    glPopAttrib();
}

void ClippingPlaneWidget::mousePressEvent(tgt::MouseEvent* e) {
    isClicked_ = isClicked(e->coord().x, e->coord().y);
    if (isClicked_ && syncEventProp_.accepts(e)) {
        shift_lock_ = true;
        if (isClicked_ == 1 || isClicked_ == 2 || isClicked_ == 3 || isClicked_ == 4)
            x_lock_.set(!x_lock_.get());
        if (isClicked_ == 5 || isClicked_ == 6 || isClicked_ == 7 || isClicked_ == 8)
            y_lock_.set(!y_lock_.get());
        if (isClicked_ == 9 || isClicked_ == 10 || isClicked_ == 11 || isClicked_ == 12)
            z_lock_.set(!z_lock_.get());
    }
    if (isClicked_) {
        e->accept();

        bool lock;
        if (isClicked_ <= 4)
            lock = x_lock_.get();
        else if (isClicked_ <= 8)
            lock = y_lock_.get();
        else
            lock = z_lock_.get();

        int index[4];
        index[0] = (isClicked_-1)/2*2;
        index[1] = index[0]+1;
        index[2] = (index[1]+1)%4 == 0? index[1]-3 : index[1]+1;
        index[3] = index[2]+1;
        controlElements_[index[0]].activated = true;
        controlElements_[index[1]].activated = true;
        if (lock) {
            controlElements_[index[2]].activated = true;
            controlElements_[index[3]].activated = true;
        }

        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true), "evaluator");
        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void ClippingPlaneWidget::mouseMoveEvent(tgt::MouseEvent* e) {
    if (isClicked_ && (eventProp_.accepts(e) || syncEventProp_.accepts(e))) {
        e->accept();

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
        // use size from texture container, as processor size may be changed by coarseness mode
        viewport[2] = static_cast<GLint>(tc_->getSize().x);
        viewport[3] = static_cast<GLint>(tc_->getSize().y);

        //adjacent edges
        tgt::vec3 edge1 = volumeSize_*controlElements_[isClicked_-1].adjacentEdge1;
        tgt::vec3 edge2 = volumeSize_*controlElements_[isClicked_-1].adjacentEdge2;

        //convert coordinates of both points in windowcoordinates
        GLdouble edge1ProjectedGL[3];
        gluProject(edge1.x, edge1.y, edge1.z, modelview, projection, viewport,
                   &edge1ProjectedGL[0], &edge1ProjectedGL[1], &edge1ProjectedGL[2]);
        tgt::vec3 edge1Projected = tgt::vec3(static_cast<float>(edge1ProjectedGL[0]),
                                             static_cast<float>(edge1ProjectedGL[1]),
                                             static_cast<float>(edge1ProjectedGL[2]));
        GLdouble edge2ProjectedGL[3];
        gluProject(edge2.x, edge2.y, edge2.z, modelview, projection, viewport,
                   &edge2ProjectedGL[0], &edge2ProjectedGL[1], &edge2ProjectedGL[2]);
        tgt::vec3 edge2Projected = tgt::vec3(static_cast<float>(edge2ProjectedGL[0]),
                                             static_cast<float>(edge2ProjectedGL[1]),
                                             static_cast<float>(edge2ProjectedGL[2]));
        //calculate projection of mouseposition to line between both edges
        tgt::vec2 mousePos = tgt::vec2(static_cast<float>(e->coord().x),
                                       tc_->getSize().y-static_cast<float>(e->coord().y));
        tgt::vec3 direction = edge2Projected-edge1Projected;
        float t = tgt::dot(mousePos-edge1Projected.xy(), direction.xy()) / tgt::lengthSq(direction.xy());
        if (t < 0.f)
            t = 0.f;
        if (t > 1.f)
            t = 1.f;
        tgt::vec3 pOnLine = edge1Projected+t*direction;

        //unproject calculated point
        GLdouble projectedClipPointGL[3];
        gluUnProject(pOnLine.x, pOnLine.y, pOnLine.z, modelview, projection, viewport,
                     &projectedClipPointGL[0], &projectedClipPointGL[1], &projectedClipPointGL[2]);

        updateClippingValues(projectedClipPointGL);

        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();

}

void ClippingPlaneWidget::updateClippingValues(GLdouble clipPoint[3]) {
    clipPoint[0] = clipPoint[0] / (2.f * volumeSize_.x) + 0.5f;
    clipPoint[1] = clipPoint[1] / (2.f * volumeSize_.y) + 0.5f;
    clipPoint[2] = clipPoint[2] / (2.f * volumeSize_.z) + 0.5f;
    //calculate values
    switch (isClicked_) {
        case 1:
        case 2:
            calculateClippingValues(clipPoint[0], clipValueLeft_, clipValueRight_, x_lock_.get(), false);
            break;
        case 3:
        case 4:
            calculateClippingValues(clipPoint[0], clipValueRight_, clipValueLeft_, x_lock_.get(), true);
            break;
        case 5:
        case 6:
            calculateClippingValues(clipPoint[1], clipValueBottom_, clipValueTop_, y_lock_.get(), false);
            break;
        case 7:
        case 8:
            calculateClippingValues(clipPoint[1], clipValueTop_, clipValueBottom_, y_lock_.get(), true);
            break;

        case 9:
        case 10:
            calculateClippingValues(clipPoint[2], clipValueFront_, clipValueBack_, z_lock_.get(), false);
            break;
        case 11:
        case 12:
            calculateClippingValues(clipPoint[2], clipValueBack_, clipValueFront_, z_lock_.get(), true);
            break;
    }

    //post Messages and update values in conrolElements
    switch (isClicked_) {
        case 1:
        case 2:
        case 3:
        case 4:
            controlElements_[0].clipValue = tgt::vec3(2.f*clipValueLeft_-1.f,  1.f,  1.f);
            controlElements_[1].clipValue = tgt::vec3(2.f*clipValueLeft_-1.f, -1.f, -1.f);
            controlElements_[2].clipValue = tgt::vec3(2.f*clipValueRight_-1.f,  1.f,  1.f);
            controlElements_[3].clipValue = tgt::vec3(2.f*clipValueRight_-1.f, -1.f, -1.f);
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setRightClipPlane_, static_cast<int>(100 - (clipValueRight_*100))));
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setLeftClipPlane_, static_cast<int>(clipValueLeft_*100)));
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            controlElements_[4].clipValue = tgt::vec3( 1.f, 2.f*clipValueBottom_-1.f,  1.f);
            controlElements_[5].clipValue = tgt::vec3(-1.f, 2.f*clipValueBottom_-1.f, -1.f);
            controlElements_[6].clipValue = tgt::vec3( 1.f, 2.f*clipValueTop_-1.f,  1.f);
            controlElements_[7].clipValue = tgt::vec3(-1.f, 2.f*clipValueTop_-1.f, -1.f);
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setTopClipPlane_, static_cast<int>(100 - (clipValueTop_*100))));
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBottomClipPlane_, static_cast<int>(clipValueBottom_*100)));
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            controlElements_[8].clipValue = tgt::vec3( 1.f, -1.f, 2.f*clipValueFront_-1.f);
            controlElements_[9].clipValue = tgt::vec3(-1.f,  1.f, 2.f*clipValueFront_-1.f);
            controlElements_[10].clipValue = tgt::vec3( 1.f, -1.f, 2.f*clipValueBack_-1.f);
            controlElements_[11].clipValue = tgt::vec3(-1.f,  1.f, 2.f*clipValueBack_-1.f);
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setBackClipPlane_, static_cast<int>(100 - (clipValueBack_*100))));
            MsgDistr.postMessage(new IntMsg(ProxyGeometry::setFrontClipPlane_, static_cast<int>(clipValueFront_*100)));
            break;
    }
}

void ClippingPlaneWidget::calculateClippingValues(GLdouble newClipValue, float& clip1, float& clip2,
                             bool lock, bool switched)
{
    float temp = clip1;
    clip1 = static_cast<float>(newClipValue);
    if (clip1 > 1.0)
        clip1 = 1.0f;
    if (clip1 < 0.0)
        clip1 = 0.0f;

    if (switched) {
        if (clip2 > clip1)
            clip2 = clip1;
    }
    else {
        if (clip1 > clip2)
            clip2 = clip1;
    }
    if (lock) {
        clip2 += clip1 - temp;
        if (clip2 > 1.0f) {
            clip1 -= clip2 - 1.f;
            clip2 = 1.0f;
        }
        if (clip2 < 0.0f) {
            clip1 -= clip2;
            clip2 = 0.0f;
        }
    }
}

void ClippingPlaneWidget::mouseReleaseEvent(tgt::MouseEvent* e) {
    if (isClicked_) {
        e->accept();
        isClicked_ = 0;
        //set all controlElements inactive
        for (size_t i = 0; i < controlElements_.size(); ++i)
            controlElements_[i].activated = false;

        if (shift_lock_) {
            x_lock_.set(false);
            y_lock_.set(false);
            z_lock_.set(false);
            shift_lock_ = false;
        }
        MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false), "evaluator");
        invalidate();
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    else
        e->ignore();
}

void ClippingPlaneWidget::paintArrow(tgt::vec3 translation_first, tgt::vec3 translation_second,
                                 tgt::vec3 rotationAxis, float rotationAngle)
{
    tgt::vec3 trans = volumeSize_*translation_first;
    glPushMatrix();
    glTranslatef(trans.x, trans.y, trans.z);
    glTranslatef(translation_second.x, translation_second.y, translation_second.z);
    glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

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

int ClippingPlaneWidget::isClicked(int x, int y) {
    IDManager id1;
    for (size_t i = 0; i < controlElements_.size(); ++i) {
        bool clicked = id1.isClicked(controlElements_[i].name, x, static_cast<int>(size_.y) - y);
        if (clicked)
            return i+1;
    }
    return 0;
}

void ClippingPlaneWidget::initLightAndMaterial() {
    //Light parameter
    light_pos[0]      =  0.0f;
    light_pos[1]      =  1.0f;
    light_pos[2]      =  1.1f;
    light_pos[3]      =  1.0f;
    light_ambient[0]  =  0.6f;
    light_ambient[1]  =  0.6f;
    light_ambient[2]  =  0.6f;
    light_ambient[3]  =  1.0f;
    light_diffuse[0]  =  0.9f;
    light_diffuse[1]  =  0.9f;
    light_diffuse[2]  =  0.9f;
    light_diffuse[3]  =  1.0f;
    light_specular[0] =  1.0f;
    light_specular[1] =  1.0f;
    light_specular[2] =  1.0f;
    light_specular[3] =  1.0f;

    //Material red plastic
    Material mat;
    mat.ambient[0]  = 0.3f;
    mat.ambient[1]    = 0.0f;
    mat.ambient[2]    = 0.0f;
    mat.ambient[3]    = 1.0f;
    mat.diffuse[0]    = 0.5f;
    mat.diffuse[1]    = 0.0f;
    mat.diffuse[2]    = 0.0f;
    mat.diffuse[3]    = 1.0f;
    mat.specular[0] = 0.7f;
    mat.specular[1] = 0.6f;
    mat.specular[2] = 0.6f;
    mat.specular[3] = 1.0f;
    mat.shininess    = 25.0f;
    materials_[0] = mat;

    //Material green plastic
    mat = Material();
    mat.ambient[0]    = 0.0f;
    mat.ambient[1]    = 0.3f;
    mat.ambient[2]    = 0.0f;
    mat.ambient[3]    = 1.0f;
    mat.diffuse[0]    = 0.1f;
    mat.diffuse[1]    = 0.5f;
    mat.diffuse[2]    = 0.1f;
    mat.diffuse[3]    = 1.0f;
    mat.specular[0] = 0.45f;
    mat.specular[1] = 0.55f;
    mat.specular[2] = 0.45f;
    mat.specular[3] = 1.0f;
    mat.shininess    = 25.0f;
    materials_[1] = mat;

    //Material blue plastic
    mat = Material();
    mat.ambient[0]    = 0.0f;
    mat.ambient[1]    = 0.0f;
    mat.ambient[2]    = 0.3f;
    mat.ambient[3]    = 1.0f;
    mat.diffuse[0]    = 0.0f;
    mat.diffuse[1]    = 0.3f;
    mat.diffuse[2]    = 0.509f;
    mat.diffuse[3]    = 1.0f;
    mat.specular[0]    = 0.501f;
    mat.specular[1]    = 0.501f;
    mat.specular[2]    = 0.501f;
    mat.specular[3]    = 1.0f;
    mat.shininess    = 25.0f;
    materials_[2] = mat;

}

} //namespace voreen
