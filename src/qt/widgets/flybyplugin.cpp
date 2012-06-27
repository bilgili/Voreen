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

#include "voreen/qt/widgets/flybyplugin.h"

#include <QProgressDialog>
#include <QAction>
#include "tgt/quaternion.h"
#include "tinyxml/tinyxml.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "voreen/core/vis/voreenpainter.h"

using tgt::quat;

namespace voreen {

FlybyPlugin::FlybyPlugin(QWidget* parent, MessageReceiver* msgReceiver, Processor* processor, tgt::Trackball* track,  WId canvasID) :
WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Animation"));
    icon_ = QIcon(":/icons/movie.png");

    flybyTimer_ = new QBasicTimer();

    currentFlyby_ = std::vector<Orientation>();
//     fbThread_ = new FlybyThread(this);
    fbInfo_ = 0;
    parent_ = parent;

    currentFrame_ = 0;
    timeOffset_ = 0.6f;
    goOnSelectEnabled_ = false;

	canvas_ = 0;

    track_ = track;
    processor_ = processor;

    canvasID_ = canvasID;

}

void FlybyPlugin::createWidgets() {
    resize(300,300);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    QGridLayout* gridLayout;
    QHBoxLayout* hboxLayout = new QHBoxLayout();
    QVBoxLayout* vboxLayout;

    // Playback group box
    playerBox_ = new QGroupBox(tr("Flyby Playback"), this);
    playerBox_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    vboxLayout = new QVBoxLayout();
    QHBoxLayout* playerLayout = new QHBoxLayout();

    startFlyby_ = new QPushButton(playerBox_);
    startFlyby_->setIcon(QIcon(":/icons/player_play-grey.png"));
    playerLayout->addWidget(startFlyby_);

    pauseFlyby_ = new QPushButton(playerBox_);
    pauseFlyby_->setIcon(QIcon(":/icons/player_pause-grey.png"));
    playerLayout->addWidget(pauseFlyby_);

    stopFlyby_ = new QPushButton(playerBox_);
    stopFlyby_->setIcon(QIcon(":/icons/player_stop-grey.png"));
    playerLayout->addWidget(stopFlyby_);
    vboxLayout->addLayout(playerLayout);

    checkLoop_ = new QCheckBox(tr("Play as loop"));
    checkLoop_->setChecked(false);
    vboxLayout->addWidget(checkLoop_);

    playerBox_->setLayout(vboxLayout);
    mainLayout->addWidget(playerBox_);


    // Create group box
    createBox_ = new QGroupBox(tr("Flyby Creation"));
    gridLayout = new QGridLayout();

    addKeyframe_ = new QPushButton(tr("Add keyframe"), this);
    gridLayout->addWidget(addKeyframe_, 0, 0);

    deleteLast_ = new QPushButton(tr("Delete last"));
    gridLayout->addWidget(deleteLast_, 0, 1);

    QGridLayout* timeLayout = new QGridLayout();
    timeLayout->setColumnStretch(0,2);
    timeLayout->setColumnStretch(1,1);
    timeLayout->addWidget(new QLabel(tr("Time until next keyframe (sec):"), 0, 0));
    selectFrameTimeCreate_ = new QDoubleSpinBox();
    selectFrameTimeCreate_->setSingleStep(0.1);
    selectFrameTimeCreate_->setValue(timeOffset_);
    timeLayout->addWidget(selectFrameTimeCreate_, 0, 1);
    timeLayout->addWidget(laFlyby_ = new QLabel(tr("Current number of keyframes:")), 1, 0);
    timeLayout->addWidget(laNumberKeyframes_ = new QLabel(" 0"), 1, 1);
    gridLayout->addLayout(timeLayout, 1, 0, 1, 0);

    clearFlyby_ = new QPushButton(tr("Clear Flyby"), this);
    gridLayout->addWidget(clearFlyby_, 2, 1);

    QFrame* separator = new QFrame();
    separator->setFrameStyle(QFrame::HLine);
    gridLayout->addWidget(separator, 3, 0, 1, 0);
    saveFlyby_ = new QPushButton(tr("Save Flyby"), this);
    gridLayout->addWidget(saveFlyby_, 4, 0);
    loadFlyby_ = new QPushButton(tr("Load Flyby"), this);
    gridLayout->addWidget(loadFlyby_, 4, 1);

    createBox_->setLayout(gridLayout);
    mainLayout->addWidget(createBox_);

    // Flyby editing
    editBox_ = new QGroupBox(tr("Flyby Editing"));
    editBox_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    vboxLayout = new QVBoxLayout();

    selectFrame_ = new QPushButton(tr("Select keyframe..."), editBox_);
    frameMenu_ = new QMenu(editBox_);
    selectFrame_->setMenu(frameMenu_);
    vboxLayout->addWidget(selectFrame_);

    gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 2);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->addWidget(new QLabel(tr("Time until next keyframe (sec):"), 0, 0));
    selectFrameTimeEdit_ = new QDoubleSpinBox();
    selectFrameTimeEdit_->setSingleStep(0.1);
    gridLayout->addWidget(selectFrameTimeEdit_, 0, 1);
    vboxLayout->addLayout(gridLayout);

    hboxLayout = new QHBoxLayout();
    replaceFrame_ = new QPushButton(tr("Replace frame"), editBox_);
    deleteFrame_ = new QPushButton(tr("Delete frame"), editBox_);
    hboxLayout->addWidget(replaceFrame_);
    hboxLayout->addWidget(deleteFrame_);
    vboxLayout->addLayout(hboxLayout);

    editBox_->setLayout(vboxLayout);
    mainLayout->addWidget(editBox_);


    // Recording group box
    recordBox_ = new QGroupBox(tr("Flyby Recording"));
    vboxLayout = new QVBoxLayout();

    hboxLayout = new QHBoxLayout();
    recordFlyby_ = new QPushButton(tr("Record Flyby"), this);
    hboxLayout->addWidget(recordFlyby_);
   // hboxLayout->addStretch();
    vboxLayout->addLayout(hboxLayout);

    hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(new QLabel(tr("Video dimensions: "), 0, 0));
    hboxLayout->addStretch();
    hboxLayout->addWidget(spWidth_ = new QSpinBox(0));
    hboxLayout->addWidget(new QLabel(tr(" x "), 0, 0));
    hboxLayout->addWidget(spHeight_ = new QSpinBox(0));
    spWidth_->setRange(32, 1280);
    spHeight_->setRange(32, 1024);
    spWidth_->setSingleStep(32);
    spHeight_->setSingleStep(32);
    spWidth_->setValue(512);
    spHeight_->setValue(384);
    vboxLayout->addLayout(hboxLayout);

    recordBox_->setLayout(vboxLayout);
    mainLayout->addWidget(recordBox_);

    mainLayout->addStretch();

    setLayout(mainLayout);

    frameMenu_->setTearOffEnabled(true);

    setWidgetState();
}

void FlybyPlugin::createConnections() {
    connect(addKeyframe_,  SIGNAL(clicked()), this, SLOT(addKeyframe()));
    connect(deleteLast_, SIGNAL(clicked()), this, SLOT(deleteLastFrame()));
    connect(startFlyby_,      SIGNAL(clicked()), this, SLOT(startFlyby()));
    connect(clearFlyby_,      SIGNAL(clicked()), this, SLOT(clearFlyby()));
    connect(recordFlyby_,     SIGNAL(clicked()), this, SLOT(recordFlyby()));
    connect(saveFlyby_,       SIGNAL(clicked()), this, SLOT(saveFlyby()));
    connect(loadFlyby_,       SIGNAL(clicked()), this, SLOT(loadFlyby()));
    connect(stopFlyby_,       SIGNAL(clicked()), this, SLOT(stopFlyby()));
    connect(pauseFlyby_,      SIGNAL(clicked()), this, SLOT(pauseFlyby()));

    connect(frameMenu_,       SIGNAL(triggered(QAction*)), this, SLOT(selectFrame(QAction*)));
    connect(replaceFrame_,    SIGNAL(clicked()),           this, SLOT(replaceKeyframe()) );
    connect(deleteFrame_,     SIGNAL(clicked()), this, SLOT(deleteFrame()));
    connect(selectFrameTimeEdit_, SIGNAL(valueChanged(double)), this, SLOT(updateFrameTime(double)));
}

void FlybyPlugin::addKeyframe() {
    Orientation ori;

    ori.tripod_ = track_->getCamera()->getQuat();
    tgt::mat4 rot;
    tgt::generateMatrixFromQuat(ori.tripod_).invert(rot);
    tgt::vec3 pos = track_->getCamera()->getPosition();
    ori.position_ = (tgt::vec4(pos, 0.f)*rot).xyz();

/*    ori.lightposition_.x = lightPos_.x;
    ori.lightposition_.y = lightPos_.y;
    ori.lightposition_.z = lightPos_.z; */
    ori.timeToNextFrame_ = selectFrameTimeCreate_->value();

    ori.pic_ = QPixmap::grabWindow(canvasID_);

    currentFlyby_.push_back(ori);

    if (currentFlyby_.size() == 1)
        currentFlyby_.push_back(ori);

    if (currentFlyby_.size() > 2) {
        currentFlyby_[currentFlyby_.size()-2].timeToNextFrame_ = selectFrameTimeCreate_->value();
    }

    std::cout << "Control-Point for Flyby added..." << std::endl;

    QString num;
    num.setNum(currentFlyby_.size() - 1);

    if (currentFlyby_.size() > 2)
        startFlyby_->setIcon(QIcon(":/icons/player_play.png"));
    laNumberKeyframes_->setText(" " + QString(num));

    refreshFrameMenu();
    selectFrame(currentFlyby_[currentFlyby_.size() - 1].action_, false);

}

void FlybyPlugin::replaceKeyframe() {

    if (currentFlyby_.size() < 2 || (size_t)currentFrame_ > currentFlyby_.size() - 1)
        return;

    Orientation ori;

    ori.tripod_ = track_->getCamera()->getQuat();
    tgt::mat4 rot;
    tgt::generateMatrixFromQuat(ori.tripod_).invert(rot);
    tgt::vec3 pos = track_->getCamera()->getPosition();
    ori.position_ = (tgt::vec4(pos, 0.f)*rot).xyz();
/*    ori.lightposition_.x = lightPos_.x;
    ori.lightposition_.y = lightPos_.y;
    ori.lightposition_.z = lightPos_.z; */
    ori.timeToNextFrame_ = selectFrameTimeEdit_->value();

    ori.pic_ = QPixmap::grabWindow(canvasID_);

    currentFlyby_[currentFrame_] = ori;

    std::cout << "Control-Point for Flyby replaced..." << std::endl;

    refreshFrameMenu();
    selectFrame(currentFlyby_[currentFrame_].action_, false);

}

void FlybyPlugin::clearFlyby() {
    currentFlyby_.clear();
    currentFrame_ = 0;
    refreshFrameMenu();
    laNumberKeyframes_->setText(" 0");
    selectFrame_->setText(QString(tr("Select keyframe...")));
    startFlyby_->setIcon(QIcon(":/icons/player_play-grey.png"));
    selectFrameTimeEdit_->setValue(0.0);

    setWidgetState();
}

void FlybyPlugin::startFlyby(bool record, std::string filename) {

    if (fbInfo_) {
        std::cout << "Playback is already running." << std::endl;
        return;
    }

    if (currentFlyby_.size() < 3) {
        std::cout << "At least 2 control-points are needed for a flyby!" << std::endl;
        return;
    }

    currentFlyby_.push_back(currentFlyby_[currentFlyby_.size() - 1]);
    currentFlyby_[currentFlyby_.size() - 1].timeToNextFrame_ = 0.f;

    postMessage(new BoolMsg("switch.trackballSpinning", false));

    if (!record) {
        startFlyby_->setIcon(QIcon(":/icons/player_play-grey.png"));
        pauseFlyby_->setIcon(QIcon(":/icons/player_pause.png"));
        stopFlyby_->setIcon(QIcon(":/icons/player_stop.png"));
        startTracking();
        processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
        std::cout << "Starting Flyby..." << std::endl;
    }
    else
        std::cout << "Starting Recording..." << std::endl;

    fbInfo_ = new FlybyInfo();

    fbInfo_->record_ = record;

// merge!!

//     VolumeRayCasting* volRayCast = dynamic_cast<VolumeRayCasting*>(canvas_->getVolumeRenderer());
//
//     if ( !volRayCast) {
//         delete fbInfo_;
//         fbInfo_ = 0;
//         return;
//     }
//     else
//         fbInfo_->volRayCast_ = volRayCast;
//
    fbInfo_->width_      = spWidth_->value();
    fbInfo_->height_     = spHeight_->value();
    fbInfo_->piccounter_ = 0;
    fbInfo_->sp_         = tgt::Spline();
//    fbInfo_->splight_    = tgt::Spline();
    fbInfo_->total_      = (currentFlyby_.size() - 3) * 1.f;
    fbInfo_->t_          = 0.f;
    fbInfo_->filename_   = filename;
    fbInfo_->pause_      = false;

    for (size_t i = 0; i < currentFlyby_.size(); i++) {
        fbInfo_->sp_.addControlPoint(currentFlyby_[i].position_);
//        fbInfo_->splight_.addControlPoint(currentFlyby_[i].lightposition_);
    }

    if (!record) {
        setWidgetState();
        flybyTimer_->start(50, this);
    }else {
        float total = 0.f;
        for (size_t i = 1; i < currentFlyby_.size() - 2; i++)
            total += 30.f * currentFlyby_[i].timeToNextFrame_;

        // disable main window while recording
        if (parent_)
            parent_->setEnabled(false);

        QProgressDialog progress(tr("Writing files..."), tr("Abort"), 0, static_cast<int>(total), parent_);
//         postMessage(new IVec2Msg(Identifier::resize, tgt::ivec2(fbInfo_->width_, fbInfo_->height_)));
        postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        progress.update();
        while (fbInfo_->t_ <= fbInfo_->total_) {
            progress.setValue(fbInfo_->piccounter_);
            progress.update();
            flybyStep();
            if (progress.wasCanceled())
                break;
        }

        progress.setValue(fbInfo_->piccounter_);
        stopFlyby();

        if (parent_)
            parent_->setEnabled(true);
    }

}

void FlybyPlugin::stopFlyby() {
// merge!!

    if (!fbInfo_)
        return;

    flybyTimer_->stop();

    std::cout << "Flyby complete!" << std::endl;
 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
//    tgt::vec4 tmp(currentFlyby_[0].lightposition_, 1.f);
//    glLightfv(GL_LIGHT0, GL_POSITION, tmp.elem);
    glPopMatrix();
//    lightPos_ = tmp;
    track_->reset();
    track_->getCamera()->setPosition(currentFlyby_[0].position_);
    track_->rotate(currentFlyby_[0].tripod_);

    processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));

    if (fbInfo_->record_)
        std::cout << "Recording complete!" << std::endl;

    currentFlyby_.pop_back();
    pauseFlyby_->setIcon(QIcon(":/icons/player_pause-grey.png"));
    stopFlyby_->setIcon(QIcon(":/icons/player_stop-grey.png"));
    startFlyby_->setIcon(QIcon(":/icons/player_play.png"));

    delete fbInfo_;
    fbInfo_ = 0;

    postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()), VoreenPainter::visibleViews_);
    postMessage(new Message(VoreenPainter::repaint_),  VoreenPainter::visibleViews_);

    // select first frame
    QExtAction* selectAction = new QExtAction("", this, 1);
    selectFrame(selectAction, false);

    setWidgetState();

}

void FlybyPlugin::pauseFlyby() {
    if (!fbInfo_)
        return;

    fbInfo_->pause_ = !fbInfo_->pause_;

    if (fbInfo_->pause_) {
        //startTracking();
        processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    }
    else {
        //stopTracking();
        processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
    }

    repaintCanvases();
}

void FlybyPlugin::recordFlyby() {

    if (fbInfo_) {
        std::cout << "Please stop playback before recording." << std::endl;
        return;
    }

    QString s = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose a filename to save under"),
                    ".",
                    tr("Images (*.bmp)"));

    std::string chosenFileName = s.toStdString();
    if (chosenFileName.length() == 0) return;
    chosenFileName = chosenFileName.substr(0, chosenFileName.length()-4);

    startFlyby(true, chosenFileName);
}

void FlybyPlugin::timerEvent(QTimerEvent* /*event*/) {

    if (fbInfo_->pause_)
        return;

    if (fbInfo_->t_ > fbInfo_->total_) {
        bool record = fbInfo_->record_;
        stopFlyby();
        if (!record && checkLoop_->isChecked())
            startFlyby(false);

        return;
    }

    flybyStep();
}

void FlybyPlugin::flybyStep() {
//merge!!

    uint current = uint(floor(fbInfo_->t_)) + 1;
    float interpol = fmod(fbInfo_->t_, 1.f);

    if (current > 0 && current < (currentFlyby_.size() - 1)) {
        quat a = tgt::splineQuat(currentFlyby_[current - 1].tripod_,
                                 currentFlyby_[current    ].tripod_,
                                 currentFlyby_[current + 1].tripod_);
        quat b = tgt::splineQuat(currentFlyby_[current    ].tripod_,
                                 currentFlyby_[current + 1].tripod_,
                                 currentFlyby_[current + 2].tripod_);

        quat q = tgt::squadQuat (currentFlyby_[current    ].tripod_,
                                 currentFlyby_[current + 1].tripod_,
                                 a, b, interpol);

//        tgt::vec4 tmp(fbInfo_->splight_.interpolate(fbInfo_->t_, fbInfo_->total_), 1.f);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
//        glLightfv(GL_LIGHT0, GL_POSITION, tmp.elem);
        glPopMatrix();
//        lightPos_ = tmp;
        track_->reset();
        tgt::vec3 newPos = fbInfo_->sp_.interpolate(fbInfo_->t_, fbInfo_->total_);
        track_->getCamera()->setFocus(track_->getCamera()->getFocus() + (newPos - track_->getCamera()->getPosition()));
        track_->getCamera()->setPosition(newPos);
        track_->rotate(q);

        if (!fbInfo_->record_) {
            if (currentFlyby_[current].timeToNextFrame_ > 0.f) {
                fbInfo_->t_ += 1.f / (currentFlyby_[current].timeToNextFrame_ * 20.f);
                postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()), VoreenPainter::visibleViews_);
                postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
            } else
                fbInfo_->t_ += 1.f;
        }
        else {
            if (currentFlyby_[current].timeToNextFrame_ > 0.f)
                fbInfo_->t_ += 1.f / (currentFlyby_[current].timeToNextFrame_ * 30.f);
            else
                fbInfo_->t_ += 1.f;

            char fn[1024];
            sprintf(fn, "%s_%04d%s", fbInfo_->filename_.c_str(), fbInfo_->piccounter_, ".bmp");
            //RPTMERGE
            //processor_->postMessage(new StringMsg(Processor::setSnapFileName_, std::string(fn)));
            //processor_->postMessage(new IVec2Msg(Processor::paintToFile_, tgt::ivec2(fbInfo_->width_, fbInfo_->height_)));
            fbInfo_->piccounter_++;
        }
    }

}

void FlybyPlugin::refreshFrameMenu() {

    frameMenu_->clear();

    for (size_t i = 1; i < currentFlyby_.size(); i++) {
        QExtAction* act = new QExtAction(QString(tr("Frame %1, %2 secs")).arg
                                    (i).arg(currentFlyby_[i].timeToNextFrame_), this, i);
        act->setIcon(QIcon(currentFlyby_[i].pic_));
        frameMenu_->addAction(act);
        currentFlyby_[i].action_ = act;
    }
}

void FlybyPlugin::selectFrame(QAction* action, bool jumpToFrame) {
    currentFrame_ = ((QExtAction*)action)->num_;
    selectFrame_->setText((QString(tr("Select Keyframe... (current: %1)"))).arg(currentFrame_));
    blockSignals(true);
    selectFrameTimeEdit_->setValue(currentFlyby_[currentFrame_].timeToNextFrame_);
    blockSignals(false);
    if (jumpToFrame)
        gotoFrame(currentFrame_);

    setWidgetState();
}

void FlybyPlugin::selectFrame(QAction* action) {

    selectFrame(action, true);
}

void FlybyPlugin::gotoFrame(int num) {
    // morph there
    tgt::quat q = currentFlyby_[num].tripod_;
    tgt::vec3 loc = currentFlyby_[num].position_;

//    tgt::vec3 lightLoc = currentFlyby_[num].lightposition_;
    tgt::quat oldQuat = track_->getCamera()->getQuat();
    tgt::vec3 oldLoc = track_->getCamera()->getPosition();

    // we have to do the interpolation of the camera in world-coordinates,
    // so we calculate the start position of the flyby in WCs (the end position was
    // already saved in WCs when the keyframe was created)
    tgt::mat4 oldRot;
    generateMatrixFromQuat(oldQuat).invert(oldRot);
    oldLoc = (tgt::vec4(oldLoc, 0.f)*oldRot).xyz();

/*   tgt::vec4 tmp = lightPos_;
    tgt::vec3 oldLightLoc;
    oldLightLoc.x = tmp.x;
    oldLightLoc.y = tmp.y;
    oldLightLoc.z = tmp.z; */

//     if (camera_->getTrackball()->isSpinning())
//         camera_->getTrackball()->setSpinning(false);

    processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
//     canvas_->getVolumeRenderer()->switchInteractionMode(true);
    float t = 0.f;

    while(t <= 1.f) {
        quat tmp = tgt::slerpQuat(oldQuat, q, t);
        tgt::vec3 newLoc = t*(loc - oldLoc) + oldLoc;
//        tgt::vec3 newLightLoc = t*(lightLoc - oldLightLoc) + oldLightLoc;
        track_->reset();
        track_->getCamera()->setFocus(track_->getCamera()->getFocus() + (newLoc - track_->getCamera()->getPosition()));
        track_->getCamera()->setPosition(newLoc);
        track_->rotate(tmp);
/*        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec4(newLightLoc, 1.f).elem);
        glPopMatrix();
        lightPos_ = tgt::vec4(newLightLoc, 1.f); */
        postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()), VoreenPainter::visibleViews_);
        postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        t += 0.2f;
    }

    track_->reset();
    track_->getCamera()->setFocus(track_->getCamera()->getFocus() + (loc - track_->getCamera()->getPosition()));
    track_->getCamera()->setPosition(loc);
    track_->rotate(q);

/*    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, tgt::vec4(lightLoc, 1.f).elem);
    glPopMatrix();
    lightPos_ = tgt::vec4(lightLoc, 1.f); */
    processor_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, track_->getCamera()), VoreenPainter::visibleViews_);
    postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void FlybyPlugin::setWidgetState() {

    // disable all group boxes except playback box,
    // if flyby is played
    if (fbInfo_) {
        playerBox_->setEnabled(true);
        createBox_->setEnabled(false);
        editBox_->setEnabled(false);
        recordBox_->setEnabled(false);
        return;
    } else {
        createBox_->setEnabled(true);
    }

    // flyby created with minium one key frame
    bool flybyCreated = (currentFlyby_.size() > 1);
    editBox_->setEnabled(flybyCreated);
    clearFlyby_->setEnabled(flybyCreated);
    saveFlyby_->setEnabled(flybyCreated);
    deleteLast_->setEnabled(flybyCreated);

    // flyby created with minimum two key frames
    bool flybyReady = (currentFlyby_.size() > 2);
    recordBox_->setEnabled(flybyReady);

}


void FlybyPlugin::deleteFrame() {
    if (currentFrame_ == 1) {
        if (currentFlyby_.size() < 3) {
            clearFlyby();
            return;
        } else {
            std::vector<Orientation>::iterator it = currentFlyby_.begin();
            currentFlyby_.erase(it);
            it = currentFlyby_.begin();
            currentFlyby_.erase(it);
            it = currentFlyby_.begin();
            currentFlyby_.insert(it, currentFlyby_[0]);
            refreshFrameMenu();
            selectFrame(currentFlyby_[1].action_, true);
        }
    } else if (currentFrame_ > 1) {
        std::vector<Orientation>::iterator it = currentFlyby_.begin();
        it += currentFrame_;
        currentFlyby_.erase(it);
        refreshFrameMenu();
        if (currentFrame_ != static_cast<int>(currentFlyby_.size()))
            selectFrame(currentFlyby_[currentFrame_].action_, true);
        else
            selectFrame(currentFlyby_[currentFrame_ - 1].action_, true);
    } else
        return;

    QString flybySize;
    flybySize.setNum(currentFlyby_.size() - 1);
    laNumberKeyframes_->setText(" " + flybySize);

    setWidgetState();
}

void FlybyPlugin::deleteLastFrame() {

    if (currentFlyby_.size() < 3) {
        clearFlyby();
        return;
    }

    currentFlyby_.erase(currentFlyby_.end()-1);

    QString num;
    num.setNum(currentFlyby_.size() - 1);

    if (currentFlyby_.size() < 3)
        startFlyby_->setIcon(QIcon(":/icons/player_play-grey.png"));
    laNumberKeyframes_->setText(" " + QString(num));

    refreshFrameMenu();
    selectFrame(currentFlyby_[currentFlyby_.size() - 1].action_, true);

    setWidgetState();
}

void FlybyPlugin::updateFrameTime(double val) {
    if (currentFrame_ == 0)
        return;

    currentFlyby_[currentFrame_].timeToNextFrame_ = val;
    currentFlyby_[currentFrame_].action_->setText(QString(tr("Frame %1, %2 secs")).arg
                                    (currentFrame_).arg(val));
}

void FlybyPlugin::saveFlyby() {
    QString fn = QFileDialog::getSaveFileName(
                    this,
                    tr("Choose a filename to save under"),
                    ".",
                    tr("Flyby files (*.flb)"));

    std::string chosenFileName = fn.toStdString();
    if (chosenFileName.length() == 0) return;

    TiXmlDocument doc;
    TiXmlComment * comment;
    std::string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement * root = new TiXmlElement("root");
    doc.LinkEndChild( root );

    comment = new TiXmlComment();
    s = "Saved Flyby";
    comment->SetValue(s.c_str());
    root->LinkEndChild( comment );

    TiXmlElement * oriNode = new TiXmlElement( "Orientations" );
    root->LinkEndChild( oriNode );

    std::vector<Orientation>::iterator iter;

    for (iter = currentFlyby_.begin(); iter != currentFlyby_.end(); iter++) {
        const Orientation& ori = *iter;
        TiXmlElement* currOri = new TiXmlElement( "orientation" );
        oriNode->LinkEndChild( currOri );

        TiXmlElement* quat;
        quat = new TiXmlElement( "tripod" );
        currOri->LinkEndChild( quat );
        quat->SetDoubleAttribute("x", ori.tripod_.x);
        quat->SetDoubleAttribute("y", ori.tripod_.y);
        quat->SetDoubleAttribute("z", ori.tripod_.z);
        quat->SetDoubleAttribute("w", ori.tripod_.w);

        TiXmlElement* pos;
        pos = new TiXmlElement( "position" );
        currOri->LinkEndChild( pos );
        pos->SetDoubleAttribute("x", ori.position_.x);
        pos->SetDoubleAttribute("y", ori.position_.y);
        pos->SetDoubleAttribute("z", ori.position_.z);

        TiXmlElement* lightpos;
        lightpos = new TiXmlElement( "lightposition" );
        currOri->LinkEndChild( lightpos );
        lightpos->SetDoubleAttribute("x", ori.lightposition_.x);
        lightpos->SetDoubleAttribute("y", ori.lightposition_.y);
        lightpos->SetDoubleAttribute("z", ori.lightposition_.z);

        TiXmlElement* duration;
        duration = new TiXmlElement( "timetonextframe" );
        currOri->LinkEndChild( duration );
        duration->SetDoubleAttribute("t", ori.timeToNextFrame_);
    }

    doc.SaveFile(chosenFileName.c_str());
}

void FlybyPlugin::loadFlyby() {

    QString fn = QFileDialog::getOpenFileName(this, tr("Open Flyby"), ".", tr("Flybys (*.flb)"));
    std::string s = fn.toStdString();
    if (s.length() == 0) return;

    TiXmlDocument doc(s.c_str());
    if (!doc.LoadFile()) return;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem;
    TiXmlHandle hRoot(0);

    elem=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it doesn't
    if (!elem) return;

    // save this for later
    hRoot=TiXmlHandle(elem);
    currentFlyby_.clear();
    currentFrame_ = 0;

    TiXmlElement* oriNode = hRoot.FirstChild("Orientations").FirstChild("orientation").Element();
    for ( ; oriNode; oriNode = oriNode->NextSiblingElement()) {
        Orientation ori;

        TiXmlElement* tri;
        tri = oriNode->FirstChildElement();
        tri->QueryFloatAttribute("x", &(ori.tripod_.x));
        tri->QueryFloatAttribute("y", &(ori.tripod_.y));
        tri->QueryFloatAttribute("z", &(ori.tripod_.z));
        tri->QueryFloatAttribute("w", &(ori.tripod_.w));

        TiXmlElement* pos;
        pos = tri->NextSiblingElement();
        pos->QueryFloatAttribute("x", &(ori.position_.x));
        pos->QueryFloatAttribute("y", &(ori.position_.y));
        pos->QueryFloatAttribute("z", &(ori.position_.z));

        TiXmlElement* lpos;
        lpos = pos->NextSiblingElement();
        lpos->QueryFloatAttribute("x", &(ori.lightposition_.x));
        lpos->QueryFloatAttribute("y", &(ori.lightposition_.y));
        lpos->QueryFloatAttribute("z", &(ori.lightposition_.z));

        TiXmlElement* dest;
        dest = lpos->NextSiblingElement();
        dest->QueryDoubleAttribute("t", &(ori.timeToNextFrame_));

        currentFlyby_.push_back(ori);
    }

    std::cout << "Flyby was loaded..." << std::endl;

    QString num;
    num.setNum(currentFlyby_.size() - 1);

    if (currentFlyby_.size() > 2)
        startFlyby_->setIcon(QIcon(":/icons/player_play.png"));
    laNumberKeyframes_->setText(" " + num);

    refreshFrameMenu();
    selectFrame(currentFlyby_[1].action_, true);

    setWidgetState();
}
} // namespace voreen

