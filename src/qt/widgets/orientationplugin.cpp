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

#include "voreen/qt/widgets/orientationplugin.h"

#include <vector>

#include "tgt/quaternion.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"
#include "tgt/texturemanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QComboBox>

#include "tinyxml/tinyxml.h"

using tgt::quat;
using std::abs;

namespace voreen {

SchematicOverlayObject::SchematicOverlayObject()
    : showTextures_(false)
    , frontTex_(0)
    , backTex_(0)
    , topTex_(0)
    , leftTex_(0)
    , bottomTex_(0)
    , rightTex_(0)
{
    loadTextures();
}

SchematicOverlayObject::~SchematicOverlayObject() {
    TexMgr.dispose(frontTex_);
    TexMgr.dispose(backTex_);
    TexMgr.dispose(bottomTex_);
    TexMgr.dispose(leftTex_);
    TexMgr.dispose(topTex_);
    TexMgr.dispose(rightTex_);
}

void SchematicOverlayObject::loadTextures(Identifier set) {
    TexMgr.dispose(frontTex_);
    TexMgr.dispose(backTex_);
    TexMgr.dispose(bottomTex_);
    TexMgr.dispose(leftTex_);
    TexMgr.dispose(topTex_);
    TexMgr.dispose(rightTex_);
    if (set == "cardiac") {
        textureNames_[Front] = "";
        textureNames_[Back] = "";
        textureNames_[Bottom] = "anterior2.png";
        textureNames_[Left] = "septal2.png";
        textureNames_[Top] = "inferior2.png";
        textureNames_[Right] = "lateral2.png";
    }
    if (textureNames_[Front] != "")
        frontTex_ = TexMgr.load(textureNames_[Front]);
    else 
        frontTex_ = 0;

    if (textureNames_[Back] != "")
        backTex_ = TexMgr.load(textureNames_[Back]);
    else 
        backTex_ = 0;

    if (textureNames_[Bottom] != "")
        bottomTex_ = TexMgr.load(textureNames_[Bottom]);
    else
        bottomTex_ = 0;

    if (textureNames_[Left] != "")
        leftTex_ = TexMgr.load(textureNames_[Left]);
    else
        leftTex_ = 0;

    if (textureNames_[Top] != "")
        topTex_ = TexMgr.load(textureNames_[Top]);
    else 
        topTex_ = 0;

    if (textureNames_[Right] != "")
        rightTex_ = TexMgr.load(textureNames_[Right]);
    else
        rightTex_ = 0;
}

void SchematicOverlayObject::setShowTextures(bool show) {
    showTextures_ = show;
}

bool SchematicOverlayObject::getShowTextures() {
    return showTextures_;
}

void SchematicOverlayObject::setCubeColorFront(tgt::Color color) {
    cubeColorFrontObject_ = color;
}

void SchematicOverlayObject::setCubeColorBack(tgt::Color color) {
    cubeColorBackObject_ = color;
}

void SchematicOverlayObject::setCubeColorLeft(tgt::Color color) {
    cubeColorLeftObject_ = color;
}
    
void SchematicOverlayObject::setCubeColorRight(tgt::Color color) {
    cubeColorRightObject_ = color;
}
    
void SchematicOverlayObject::setCubeColorTop(tgt::Color color) {
    cubeColorTopObject_ = color;
}
    
void SchematicOverlayObject::setCubeColorDown(tgt::Color color) {
    cubeColorDownObject_ = color;
}
    
void SchematicOverlayObject::setCubeSize(float cubeSizeObject) {
    cubeSizeObject_ = cubeSizeObject;
}

tgt::Color SchematicOverlayObject::getCubeColorFront() {
    return cubeColorFrontObject_;
}
    
tgt::Color SchematicOverlayObject::getCubeColorBack() {
    return cubeColorBackObject_;
}
    
tgt::Color SchematicOverlayObject::getCubeColorLeft() {
    return cubeColorLeftObject_;
}

tgt::Color SchematicOverlayObject::getCubeColorRight() {
    return cubeColorRightObject_;
}
    
tgt::Color SchematicOverlayObject::getCubeColorTop() {
    return cubeColorTopObject_;
}

tgt::Color SchematicOverlayObject::getCubeColorDown() {
    return cubeColorDownObject_;
}
    
float SchematicOverlayObject::getCubeSize() {
    return cubeSizeObject_;
}

void SchematicOverlayObject::renderCubeToBuffer() {
    IDManager myIDMan;

    myIDMan.startBufferRendering("OrientationWidget.cubeBackClicked");
    glBegin(GL_QUADS);
    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    myIDMan.startBufferRendering("OrientationWidget.cubeFrontClicked");
    glBegin(GL_QUADS);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    myIDMan.startBufferRendering("OrientationWidget.cubeLeftClicked");
    glBegin(GL_QUADS);
    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    myIDMan.startBufferRendering("OrientationWidget.cubeRightClicked");
    glBegin(GL_QUADS);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    myIDMan.startBufferRendering("OrientationWidget.cubeTopClicked");
    glBegin(GL_QUADS);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    myIDMan.startBufferRendering("OrientationWidget.cubeDownClicked");
    glBegin(GL_QUADS);
    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
    glEnd();
    myIDMan.stopBufferRendering();

    LGL_ERROR;
}

void SchematicOverlayObject::renderCube() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    //top
    if (showTextures_ && topTex_) {
        topTex_->bind();
        glColor4fv(cubeColorTopObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 1.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorTopObject_.elem);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
	//bottom
    if (showTextures_ && bottomTex_) {
        bottomTex_->bind();
	    glColor4fv(cubeColorDownObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(1.f, 1.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorDownObject_.elem);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }
	//front
    if (showTextures_ && frontTex_) {
        frontTex_->bind();
        glColor4fv(cubeColorFrontObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(1.f, 1.f);
        glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorFrontObject_.elem);
        glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
    //back
    if (showTextures_ && backTex_) {
        backTex_->bind();
        glColor4fv(cubeColorBackObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 1.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorBackObject_.elem);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }
    //left
    if (showTextures_ && leftTex_) {
        leftTex_->bind();
	    glColor4fv(cubeColorLeftObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 1.f);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorLeftObject_.elem);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f(-cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glEnd();
    }
	//right
    if (showTextures_ && rightTex_) {
        rightTex_->bind();
	    glColor4fv(cubeColorRightObject_.elem);
        glBegin(GL_QUADS);
        glTexCoord2f(1.f, 0.f);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
        glTexCoord2f(1.f, 1.f);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 1.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
        glTexCoord2f(0.f, 0.f);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
        glColor4fv(cubeColorRightObject_.elem);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_,-cubeSizeObject_);
	    glVertex3f( cubeSizeObject_, cubeSizeObject_, cubeSizeObject_);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_, cubeSizeObject_);
	    glVertex3f( cubeSizeObject_,-cubeSizeObject_,-cubeSizeObject_);
        glEnd();
    }

    LGL_ERROR;
    glPopAttrib();
}

/*-------------------------------------end orientation overlay object-------------------------------------------*/

QtCanvasSchematicOverlay::QtCanvasSchematicOverlay(tgt::QtCanvas* canvas)
    : VoreenPainterOverlay(canvas)
{
    setName("OrientationOverlay");
    if (canvas_)
        canvas_->getGLFocus();
    IDManager myIdMan;

    myIdMan.addNewPickObj("OrientationWidget.cubeFrontClicked");
    myIdMan.addNewPickObj("OrientationWidget.cubeBackClicked");
    myIdMan.addNewPickObj("OrientationWidget.cubeTopClicked");
    myIdMan.addNewPickObj("OrientationWidget.cubeDownClicked");
    myIdMan.addNewPickObj("OrientationWidget.cubeLeftClicked");
    myIdMan.addNewPickObj("OrientationWidget.cubeRightClicked");

	isClicked_ = 0;

    cubeColorFront_ = new ColorProp("set.OrientationWidget.cubeColorFront", QObject::tr("Front color").toStdString(), tgt::Color(0.9f));
    cubeColorBack_ = new ColorProp("set.OrientationWidget.cubeColorBack", QObject::tr("Back color").toStdString(), tgt::Color(0.8f));
    cubeColorLeft_ = new ColorProp("set.OrientationWidget.cubeColorLeft", QObject::tr("Left color").toStdString(), tgt::Color(0.7f));
    cubeColorRight_ = new ColorProp("set.OrientationWidget.cubeColorRight", QObject::tr("Right color").toStdString(), tgt::Color(0.6f));
    cubeColorTop_ = new ColorProp("set.OrientationWidget.cubeColorTop", QObject::tr("Top color").toStdString(), tgt::Color(0.5f));
    cubeColorDown_ = new ColorProp("set.OrientationWidget.cubeColorDown", QObject::tr("Down color").toStdString(), tgt::Color(0.4f));

    cubeColorFront_->setAutoChange(true);
    cubeColorBack_->setAutoChange(true);
    cubeColorLeft_->setAutoChange(true);
    cubeColorRight_->setAutoChange(true);
    cubeColorTop_->setAutoChange(true);
    cubeColorDown_->setAutoChange(true);

    addProperty(cubeColorFront_);
    addProperty(cubeColorBack_);
    addProperty(cubeColorLeft_);
    addProperty(cubeColorRight_);
    addProperty(cubeColorTop_);
    addProperty(cubeColorDown_);

    cubeSize_ = new FloatProp("set.OrientationWidget.cubeSize", QObject::tr("Cube size").toStdString(), 0.15f);
    cubePosX_ = new FloatProp("set.OrientationWidget.cubePosX", QObject::tr("Cube Pos X").toStdString(),  0.7f, -1.f, 1.f, false);
    cubePosY_ = new FloatProp("set.OrientationWidget.cubePosY", QObject::tr("Cube Pos Y").toStdString(), -0.95f, -1.f, 1.f, false);

    cubeSize_->setAutoChange(true);
    cubePosX_->setAutoChange(true);
    cubePosY_->setAutoChange(true);

    addProperty(cubeSize_);
    addProperty(cubePosX_);
    addProperty(cubePosY_);
}

QtCanvasSchematicOverlay::~QtCanvasSchematicOverlay() {
    delete cubeColorFront_;
    delete cubeColorBack_;
    delete cubeColorLeft_;
    delete cubeColorRight_;
    delete cubeColorTop_;
    delete cubeColorDown_;

    delete cubeSize_;
    delete cubePosX_;
    delete cubePosY_;
}

void QtCanvasSchematicOverlay::setCubePosX(float x) {
    cubePosX_->set(x);
}

void QtCanvasSchematicOverlay::setCubePosY(float y) {
    cubePosX_->set(y);
}

void QtCanvasSchematicOverlay::paint() {
    
    if (!canvas_)
        return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    tgt::loadMatrix(canvas_->getCamera()->getProjectionMatrix());

    glMatrixMode(GL_MODELVIEW);

    // move cube to desired position
    glTranslatef(cubePosX_->get(), cubePosY_->get(), -3.0f);
    tgt::mat4 rot = canvas_->getCamera()->getRotateMatrix();
    tgt::multMatrix(rot);

    // render the cube
    schematicOverlayObject_.setCubeColorFront(cubeColorFront_->get());
    schematicOverlayObject_.setCubeColorBack(cubeColorBack_->get());
    schematicOverlayObject_.setCubeColorLeft(cubeColorLeft_->get());
    schematicOverlayObject_.setCubeColorRight(cubeColorRight_->get());
    schematicOverlayObject_.setCubeColorTop(cubeColorTop_->get());
    schematicOverlayObject_.setCubeColorDown(cubeColorDown_->get());
    schematicOverlayObject_.setCubeSize(cubeSize_->get());
    schematicOverlayObject_.renderCubeToBuffer();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_ALWAYS);
    schematicOverlayObject_.renderCube();
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    schematicOverlayObject_.renderCube();

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPopAttrib();
}

void QtCanvasSchematicOverlay::resize(int /*width*/, int /*height*/) {
}

void QtCanvasSchematicOverlay::setShowTextures(bool show) {
    schematicOverlayObject_.setShowTextures(show);
}

void QtCanvasSchematicOverlay::loadTextures(Identifier set) {
    schematicOverlayObject_.loadTextures(set);
}

/*-------------------------------------end orientation overlay--------------------------------------------------*/

OrientationPlugin::OrientationPlugin(QWidget* parent, 
                                     MessageReceiver* rec, 
                                     tgt::QtCanvas* canvas,
                                     tgt::Trackball* track, 
                                     TextureContainer* tc)
    : WidgetPlugin(parent, rec)
    , schematicOverlay_(canvas)
    , track_(track)
    , canvas_(canvas)
    , MIN_CAM_DIST(5)
    , MAX_CAM_DIST(1500)
    , CAM_DIST_SCALE_FACTOR(100.f)
    , AXIAL_VIEW(tgt::vec3(0.f,0.f,-1.f))
    , CORONAL_VIEW(tgt::vec3(0.f,1.f,0.f))
    , SAGITTAL_VIEW(tgt::vec3(1.f,0.f,0.f))
    , AXIAL_INV_VIEW(tgt::vec3(0.f,0.f,1.f))
    , CORONAL_INV_VIEW(tgt::vec3(0.f,-1.f,0.f))
    , SAGITTAL_INV_VIEW(tgt::vec3(-1.f,0.f,0.f))
    , features_(ALL_FEATURES)
    , tc_(tc)
{
    setObjectName(tr("Orientation"));
    icon_ = QIcon(":/icons/trackball-reset.png");
    dist_ = 5;
    timer_ = new QBasicTimer();
    rotateX_ = rotateY_ = rotateZ_ = false;

    MsgDistr.insert(this);

    restore_ = false;
    startupRestore();

    WidgetPlugin::postMessage(new TemplateMessage<VoreenPainterOverlay*>(VoreenPainter::addCanvasOverlay_,
                                                                         &schematicOverlay_), "mainview");
    schematicOverlay_.setIsActivated(false);
}

OrientationPlugin::~OrientationPlugin() {
    delete timer_;
    
    /*if (restore_)
        shutdownSave(true);
    else
        shutdownSave(false); */
}

void OrientationPlugin::deinit() {
    WidgetPlugin::postMessage(new TemplateMessage<VoreenPainterOverlay*>(VoreenPainter::delCanvasOverlay_,
                                                                         &schematicOverlay_), "mainview");
    MsgDistr.remove(this);
}

void OrientationPlugin::enableFeatures(int features) {
     features_ |= features;
}

void OrientationPlugin::disableFeatures(int features) {
     features_ &= ~features;
}

bool OrientationPlugin::isFeatureEnabled(OrientationPlugin::Features feature) {
    return (features_ & feature);
}

void OrientationPlugin::createWidgets() {
	resize(300,300);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    // Group box for orientation
    orientationBox_ = new QGroupBox(tr("Orientation and Distance"));
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 3);
    gridLayout->setColumnStretch(1, 2);

    gridLayout->addWidget(new QLabel(tr("Orientation: ")), 1, 0);

    comboOrientation_ = new QComboBox();
    comboOrientation_->addItem(tr("custom"));
    comboOrientation_->addItem(tr("axial"));
    comboOrientation_->addItem(tr("coronal"));
    comboOrientation_->addItem(tr("sagittal"));
    comboOrientation_->addItem(tr("axial (bottom)"));
    comboOrientation_->addItem(tr("coronal (back)"));
    comboOrientation_->addItem(tr("sagittal (left)"));
    gridLayout->addWidget(comboOrientation_, 1, 1);

    gridLayout->addWidget(new QLabel(tr("Distance: ")), 2, 0);
    gridLayout->addWidget(slDistance_ = new QSlider(Qt::Horizontal, 0), 2, 1);
    slDistance_->setRange(MIN_CAM_DIST, MAX_CAM_DIST);
    slDistance_->setSliderPosition(static_cast<int>(CAM_DIST_SCALE_FACTOR));
    slDistance_->setToolTip(tr("Adjust distance of point of view"));

    QFrame* separator = new QFrame();
    separator->setFrameStyle(QFrame::HLine);
    gridLayout->addWidget(separator, 4, 0, 1, 0);
    showOrientationOverlay_ = new QCheckBox(tr("Show orientation overlay"));
    gridLayout->addWidget(showOrientationOverlay_, 5, 0, 1, 0);
    if (!isFeatureEnabled(ORIENTATION_OVERLAY)) {
        separator->setVisible(false);
        showOrientationOverlay_->setVisible(false);
    }

    orientationBox_->setLayout(gridLayout);
    mainLayout->addWidget(orientationBox_);

    if (!isFeatureEnabled(ORIENTATION_AND_DISTANCE))
        orientationBox_->setVisible(false);

    // Group box for motion settings
    motionBox_ = new QGroupBox(tr("Continuous Motion"));
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    rotateAroundX_ = new QCheckBox(tr("Continually rotate around x-Axis"));
    rotateAroundY_ = new QCheckBox(tr("Continually rotate around y-Axis"));
    rotateAroundZ_ = new QCheckBox(tr("Continually rotate around z-Axis"));
    continueSpin_  = new QCheckBox(tr("Give trackball a spin with\neach mouse-movement"));

    vboxLayout->addWidget(rotateAroundX_);
    vboxLayout->addWidget(rotateAroundY_);
    vboxLayout->addWidget(rotateAroundZ_);
    vboxLayout->addWidget(continueSpin_);
    motionBox_->setLayout(vboxLayout);
    mainLayout->addWidget(motionBox_);
    if (!isFeatureEnabled(CONTINUOUS_MOTION))
        motionBox_->setVisible(false);

    // Group box for trackball
    trackballBox_ = new QGroupBox(tr("Load/Save Camera Settings"));
    QHBoxLayout* hboxLayout2 = new QHBoxLayout();

    hboxLayout2->addWidget(buRestoreTrackball_ = new QToolButton(0));
    hboxLayout2->addSpacing(10);
    hboxLayout2->addWidget(buSaveTrackball_ = new QToolButton(0));
    hboxLayout2->addSpacing(10);
    buRestoreTrackball_->setIcon(QIcon(":/icons/open.png"));
    buRestoreTrackball_->setToolTip(tr("Load camera settings from a file"));
    buSaveTrackball_->setIcon(QIcon(":/icons/save.png"));
    buSaveTrackball_->setToolTip(tr("Save camera settings to a file"));
    hboxLayout2->addWidget(cbRestoreOnStartup_ = new QCheckBox(tr("Restore on startup")));
    if (restore_)
        cbRestoreOnStartup_->setCheckState(Qt::Checked);
    cbRestoreOnStartup_->setEnabled(false);
    hboxLayout2->addStretch();

    trackballBox_->setLayout(hboxLayout2);
    mainLayout->addWidget(trackballBox_);
    if (!isFeatureEnabled(TRACKBALL_BOX))
        trackballBox_->setVisible(false);

    mainLayout->addStretch();

    setLayout(mainLayout);
}

void OrientationPlugin::createConnections() {
    connect(rotateAroundX_,              SIGNAL(toggled(bool)),     this, SLOT(enableX(bool)));
    connect(rotateAroundY_,              SIGNAL(toggled(bool)),     this, SLOT(enableY(bool)));
    connect(rotateAroundZ_,              SIGNAL(toggled(bool)),     this, SLOT(enableZ(bool)));
    connect(continueSpin_,               SIGNAL(toggled(bool)),     this, SLOT(enableContSpin(bool)));
    connect(showOrientationOverlay_,     SIGNAL(toggled(bool)),     this, SLOT(enableOrientationOverlay(bool)));

    connect(comboOrientation_,   SIGNAL(activated(int)),    this, SLOT(orientationChanged(int)));
    connect(slDistance_,         SIGNAL(valueChanged(int)), this, SLOT(distanceSliderChanged(int)));
    connect(slDistance_,         SIGNAL(sliderPressed()),   this, SLOT(distanceSliderPressed()));
    connect(slDistance_,         SIGNAL(sliderReleased()),  this, SLOT(distanceSliderReleased()));

    connect(buRestoreTrackball_, SIGNAL(clicked()),     this, SLOT(restoreTrackball()));
    connect(buSaveTrackball_,    SIGNAL(clicked()),     this, SLOT(saveTrackballToDisk()));
    connect(cbRestoreOnStartup_, SIGNAL(toggled(bool)), this, SLOT(setRestore(bool)));

    if (canvas_)
        canvas_->getEventHandler()->addListenerToBack(this);
}

void OrientationPlugin::enableOrientationOverlay(bool b) {
    schematicOverlay_.setIsActivated(b);
    WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void OrientationPlugin::processMessage(Message* msg, const Identifier& dest) {
    MessageReceiver::processMessage(msg, dest);

    if (isVisible() && msg->id_ == VoreenPainter::cameraChanged_ && 
         canvas_ && msg->getValue<tgt::Camera*>() == canvas_->getCamera())
    {
        checkCameraState();    
    }
}

void OrientationPlugin::mousePressEvent(tgt::MouseEvent* e) {
    isClicked_ = isClicked(e->x(), e->y());
    if (isClicked_) {
        // cube front is clicked
        if (isClicked_ == 1) {
            toFront();
        }
        // cube back is clicked
        if (isClicked_ == 2) {
            toBehind();
        }
        // cube left is clicked
        if (isClicked_ == 3) {
            toRight();
        }
        // cube right is clicked
        if (isClicked_ == 4) {
            toLeft();
        }
        // cube top is clicked
        if (isClicked_ == 5) {
            toAbove();
        }
        // cube bottom is clicked
        if (isClicked_ == 6) {
            toBelow();
        }
        e->accept();
    }
    else
        e->ignore();
}

void OrientationPlugin::mouseMoveEvent(tgt::MouseEvent* e) {
    if (isClicked_ && tgt::MouseEvent::CTRL)
        e->accept();
    else
        e->ignore();
}

void OrientationPlugin::mouseDoubleClickEvent(tgt::MouseEvent* /*e*/){
}

void OrientationPlugin::mouseReleaseEvent(tgt::MouseEvent* e) {
	e->ignore();
}

int OrientationPlugin::isClicked(int x, int y) {
    if (!tc_ || !canvas_)
        return 0;

    IDManager id1;

    if (id1.isClicked("OrientationWidget.cubeFrontClicked", x, canvas_->getSize().y - y))
        return 1;
    if (id1.isClicked("OrientationWidget.cubeBackClicked", x, canvas_->getSize().y - y))
        return 2;
    if (id1.isClicked("OrientationWidget.cubeTopClicked", x, canvas_->getSize().y - y))
        return 3;
    if (id1.isClicked("OrientationWidget.cubeDownClicked", x, canvas_->getSize().y - y))
        return 4;
    if (id1.isClicked("OrientationWidget.cubeLeftClicked", x, canvas_->getSize().y - y))
        return 5;
    if (id1.isClicked("OrientationWidget.cubeRightClicked", x, canvas_->getSize().y - y))
        return 6;

	return 0;
}

void OrientationPlugin::toAbove() {
    quat q = quat(0.f, 0.f, 0.f, 1.f);
    applyOrientation(q);
}

void OrientationPlugin::toBelow() {
    quat q = quat(1.f, 0.f, 0.f, 0.f);
    applyOrientation(q);
}

void OrientationPlugin::toBehind() {
//     float c = 0.5f * sqrtf(2.f);
    // This is not very pretty, admittedly.  The quaternion should be (0, 0.5*sqrt(2), 0.5*sqrt(2), 0)
    // to be precise, but that seems to be some kind of edge case that breaks the modelview-matrix which
    // can also not be reached by turning the trackball by mouse.  So, we use a quaternion that is approximately
    // the mathematically correct one; the difference is so small that one cannot see any.
    quat q = normalize(quat(0.00306279, 0.710406, 0.708503, 0.00167364));
    applyOrientation(q);
}

void OrientationPlugin::toFront() {
    float c = 0.5f * sqrtf(2.f);
    quat q = quat(c, 0.f, 0.f, c);
    applyOrientation(q);
}

void OrientationPlugin::toLeft() {
    quat q = quat(-0.5f, -0.5f, -0.5f, -0.5f);
    applyOrientation(q);
}

void OrientationPlugin::toRight() {
    quat q = quat(-0.5f, 0.5f, 0.5f, -0.5f);
    applyOrientation(q);
}

void OrientationPlugin::applyOrientation(const quat& q) {
    std::vector<float> keyframe;
    keyframe.push_back(q.x);
    keyframe.push_back(q.y);
    keyframe.push_back(q.z);
    keyframe.push_back(q.w);
    keyframe.push_back(slDistance_->value() / CAM_DIST_SCALE_FACTOR);
    std::string message;
/*    if (checkAnimation_->isChecked()) {
        message = "set.cameraApplyOrientationAndDistanceAnimated";
    } else {
        message = "set.cameraApplyOrientation";
    } */
    //message = "set.cameraApplyOrientationAndDistanceAnimated";
    message = "set.cameraApplyOrientation";

    WidgetPlugin::postMessage(new TemplateMessage<std::vector<float> >(message,
        keyframe), MsgDistr.getCurrentViewId());
}

void OrientationPlugin::updateDistance() {
    dist_ = slDistance_->value() / CAM_DIST_SCALE_FACTOR;
    slDistance_->blockSignals(true);
    WidgetPlugin::postMessage(new FloatMsg("set.cameraZoom", dist_), MsgDistr.getCurrentViewId());
    slDistance_->blockSignals(false);
}

void OrientationPlugin::orientationChanged(int index) {
	switch (index) {
    case 1:
        // axial (top to bottom)
        toAbove();
        break;
    case 2:
        // coronal (front to back)
        toFront();
        break;
    case 3:
        // sagittal (volume's right to left)
        toRight();
        break;
    case 4:
        // axial bottom (bottom to top)
        toBelow();
        break;
    case 5:
        // coronal back (back to front)
        toBehind();
        break;
    case 6:
        // sagittal left (volume's left to right)
        toLeft();
        break;
    }
}

void OrientationPlugin::distanceSliderChanged(int value) {
    if (value != slDistance_->value()) {
        slDistance_->blockSignals(true);
        slDistance_->setValue(value);
        slDistance_->blockSignals(false);
    }
    updateDistance();
}

void OrientationPlugin::distanceSliderPressed() {
    WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
    updateDistance();
}


void OrientationPlugin::distanceSliderReleased() {
    WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    if (canvas_)
        canvas_->repaint();
}


void OrientationPlugin::timerEvent(QTimerEvent* /*event*/) {
     
    if (!track_)
        return;

    if (rotateX_ || rotateY_ || rotateZ_) {
         continueSpin_->setCheckState(Qt::Unchecked);
         WidgetPlugin::postMessage(new BoolMsg("switch.trackballContinuousSpin", false));
     }

    if (rotateX_)
        track_->rotate(tgt::vec3(1.f, 0.f, 0.f), 0.05f);

    if (rotateY_)
        track_->rotate(tgt::vec3(0.f, 1.f, 0.f), 0.05f);

    if (rotateZ_)
        track_->rotate(tgt::vec3(0.f, 0.f, 1.f), 0.05f);

    if (rotateX_ || rotateY_ || rotateZ_) {
        WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
        WidgetPlugin::postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()));
        WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        if (comboOrientation_->currentIndex() != 0)
            comboOrientation_->setCurrentIndex(0);
    }
}

void OrientationPlugin::checkCameraState() {
    if (track_ && track_->getCamera()) {
        // update distance slider
        slDistance_->blockSignals(true);
        slDistance_->setValue(tgt::iround(CAM_DIST_SCALE_FACTOR*track_->getCenterDistance()));
        slDistance_->blockSignals(false);

        // update orientation box
        const float MAX_DEVIATION = 1e-4;
        tgt::vec3 look = track_->getCamera()->getLook();
        comboOrientation_->blockSignals(true);
        if (abs(tgt::dot(AXIAL_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(1);
        else if (abs(tgt::dot(CORONAL_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(2);
        else if (abs(tgt::dot(SAGITTAL_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(3);
        else if (abs(tgt::dot(AXIAL_INV_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(4);
        else if (abs(tgt::dot(CORONAL_INV_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(5);
        else if (abs(tgt::dot(SAGITTAL_INV_VIEW,look)-1.f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(6);
        else
            comboOrientation_->setCurrentIndex(0);
        comboOrientation_->blockSignals(false);

    }
}

void OrientationPlugin::showEvent(QShowEvent* event) {
    WidgetPlugin::showEvent(event);
    checkCameraState();
}

void OrientationPlugin::shutdownSave(bool b) {
    saveTrackballToDisk(std::string("./lastcam.tbp"), b);
}

void OrientationPlugin::setRestore(bool b) {
    restore_ = b;
}

bool OrientationPlugin::getRestore() {
    return restore_;
}

void OrientationPlugin::enableContSpin(bool b) {
    WidgetPlugin::postMessage(new BoolMsg("switch.trackballContinuousSpin", b));
}

void OrientationPlugin::enableX(bool b) {
    if (!b && !(rotateY_ || rotateZ_)) {
        WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
        WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    rotateX_ = b;
    setTimerState();
}
        
void OrientationPlugin::enableY(bool b) {
    if (!b && !(rotateX_ || rotateZ_)) {
        WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
        WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    rotateY_ = b;
    setTimerState();
}

void OrientationPlugin::enableZ(bool b) {
    if (!b && !(rotateX_ || rotateY_)) {
        WidgetPlugin::postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
        WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
    rotateZ_ = b;
    setTimerState();
}

void OrientationPlugin::saveTrackballToDisk() {
    QString s = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose a filename to save under"),
                    ".",
                    tr("Trackball Position (*.tbp)"));

    std::string chosenFileName = s.toStdString();

    if (chosenFileName.length() == 0)
        return;
    else
        saveTrackballToDisk(chosenFileName);
}

void OrientationPlugin::saveTrackballToDisk(std::string fn, bool shutdown) {
    TiXmlDocument doc;
    TiXmlComment * comment;
    std::string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
    doc.LinkEndChild(decl);

    TiXmlElement * root = new TiXmlElement("root");
    doc.LinkEndChild(root);

    tgt::quat tripod = track_->getCamera()->getQuat();
    tgt::vec3 pos    = track_->getCamera()->getPosition();

    comment = new TiXmlComment();
    s = "Saved Flyby";
    comment->SetValue(s.c_str());
    root->LinkEndChild(comment);

    TiXmlElement* sdElem = new TiXmlElement("shutdown");
    root->LinkEndChild(sdElem);
    sdElem->SetAttribute("shut", static_cast<int>(shutdown));

    TiXmlElement* quatElem;
    quatElem = new TiXmlElement("tripod");
    root->LinkEndChild(quatElem);
    quatElem->SetDoubleAttribute("x", tripod.x);
    quatElem->SetDoubleAttribute("y", tripod.y);
    quatElem->SetDoubleAttribute("z", tripod.z);
    quatElem->SetDoubleAttribute("w", tripod.w);

    TiXmlElement* posElem;
    posElem = new TiXmlElement("position");
    root->LinkEndChild(posElem);
    posElem->SetDoubleAttribute("x", pos.x);
    posElem->SetDoubleAttribute("y", pos.y);
    posElem->SetDoubleAttribute("z", pos.z);

    doc.SaveFile(fn.c_str());
}

void OrientationPlugin::restoreTrackball(std::string fn) {
    TiXmlDocument doc(fn.c_str());
    if (!doc.LoadFile())
        return;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem;
    TiXmlHandle hRoot(0);

    elem=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it doesn't
    if (!elem)
        return;

    // save this for later
    hRoot=TiXmlHandle(elem);

    TiXmlElement* tri = hRoot.FirstChild("tripod").Element();

    tgt::quat tripod;
    tgt::vec3 pos;

    tri->QueryFloatAttribute("x", &(tripod.x));
    tri->QueryFloatAttribute("y", &(tripod.y));
    tri->QueryFloatAttribute("z", &(tripod.z));
    tri->QueryFloatAttribute("w", &(tripod.w));

    TiXmlElement* posElem;
    posElem = tri->NextSiblingElement();
    posElem->QueryFloatAttribute("x", &(pos.x));
    posElem->QueryFloatAttribute("y", &(pos.y));
    posElem->QueryFloatAttribute("z", &(pos.z));

    track_->reset();
    track_->rotate(tripod);
    track_->getCamera()->setPosition(pos);

    WidgetPlugin::postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()), VoreenPainter::visibleViews_);

    WidgetPlugin::postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void OrientationPlugin::restoreTrackball() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open trackball position"), ".",
                                              tr("Trackball positions (*.tbp)"));
    std::string s = fn.toStdString();
    if (!s.empty())
        restoreTrackball(s);
}

void OrientationPlugin::startupRestore() {
    TiXmlDocument doc("./lastcam.tbp");
    if (!doc.LoadFile())
        return;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem;
    TiXmlHandle hRoot(0);

    elem = hDoc.FirstChildElement().Element();
    if (!elem)
        return;

    hRoot=TiXmlHandle(elem);
    TiXmlElement* restElem = hRoot.FirstChild("shutdown").Element();

    int restore;
    restElem->QueryIntAttribute("shut", &restore);

    if (restore) {
        restoreTrackball(std::string("./lastcam.tbp"));
        restore_ = true;
    }
    else {
        restore_ = false;
    }
}

void OrientationPlugin::setShowTextures(bool show) {
    schematicOverlay_.setShowTextures(show);
}

void OrientationPlugin::loadTextures(Identifier set) {
    schematicOverlay_.loadTextures(set);
}

void OrientationPlugin::setTimerState() {
    if (timer_->isActive()) {
        if (!(rotateX_ || rotateY_ || rotateZ_))
            timer_->stop();
    } else {
        if (rotateX_ || rotateY_ || rotateZ_)
            timer_->start(25, this);
    }    
}

} // namespace voreen
