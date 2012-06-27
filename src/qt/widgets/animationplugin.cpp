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

#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/messagedistributor.h"

#include "tinyxml/tinyxml.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QAction>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#ifdef VRN_WITH_FFMPEG
#include "tgt/ffmpeg/videoencoder.h"
#include <sstream>
#endif

namespace voreen {

using tgt::vec3;
using tgt::ivec2;
using tgt::BSpline;

AnimationPlugin::AnimationPlugin(QWidget* parent, tgt::Camera* camera, tgt::QtCanvas* canvas)
    : WidgetPlugin(parent),
      camera_(camera),
      canvas_(canvas),
      currentKeyframe_(0),
      camPositionSpline_(0),
      camFocusSpline_(0),
      camUpSpline_(0),
      animationState_(Stopped),
      timeOffset_(0.5f),
      renderingVideo_(false)
{
    tgtAssert(camera_, "No camera");

    setObjectName(tr("Animation"));
    icon_ = QIcon(":/icons/movie.png");

    animationTimer_ = new QBasicTimer();
}

AnimationPlugin::~AnimationPlugin() {
    delete animationTimer_;
    delete camPositionSpline_;
    delete camFocusSpline_;
    delete camUpSpline_;
}

void AnimationPlugin::createWidgets() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    QGridLayout* gridLayout;
    QHBoxLayout* hboxLayout = new QHBoxLayout();
    QVBoxLayout* vboxLayout;

    // Playback group box
    playerBox_ = new QGroupBox(tr("Playback"), this);
    playerBox_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    vboxLayout = new QVBoxLayout();
    QHBoxLayout* playerLayout = new QHBoxLayout();

    startAnimation_ = new QPushButton(playerBox_);
    startAnimation_->setIcon(QIcon(":/icons/player_play.png"));
    startAnimation_->setEnabled(false);
    playerLayout->addWidget(startAnimation_);

    pauseAnimation_ = new QPushButton(playerBox_);
    pauseAnimation_->setIcon(QIcon(":/icons/player_pause.png"));
    pauseAnimation_->setEnabled(false);
    playerLayout->addWidget(pauseAnimation_);

    stopAnimation_ = new QPushButton(playerBox_);
    stopAnimation_->setIcon(QIcon(":/icons/player_stop.png"));
    stopAnimation_->setEnabled(false);
    playerLayout->addWidget(stopAnimation_);
    vboxLayout->addLayout(playerLayout);

    playerLayout = new QHBoxLayout();
    spinPlaybackFPS_ = new QSpinBox(playerBox_);
    spinPlaybackFPS_->setRange(1, 100);
    spinPlaybackFPS_->setValue(25);
    checkLoop_ = new QCheckBox(tr("Play as loop"), playerBox_);
    checkLoop_->setChecked(false);
    playerLayout->addWidget(new QLabel(tr("FPS: "), playerBox_));
    playerLayout->addWidget(spinPlaybackFPS_);
    //playerLayout->addSpacing(20);
    playerLayout->addStretch();
    playerLayout->addWidget(checkLoop_);
    playerLayout->addStretch();

    vboxLayout->addLayout(playerLayout);

    playerBox_->setLayout(vboxLayout);
    mainLayout->addWidget(playerBox_);

    // Editing group box
    editBox_ = new QGroupBox(tr("Editing"));
    gridLayout = new QGridLayout();

    selectKeyframe_ = new QPushButton(tr("Select Keyframe..."), editBox_);
    frameMenu_ = new QMenu(editBox_);
    selectKeyframe_->setMenu(frameMenu_);
    gridLayout->addWidget(selectKeyframe_, 0, 0, 1, 2);

    QGridLayout* timeLayout = new QGridLayout();
    timeLayout->setColumnStretch(0, 2);
    timeLayout->setColumnStretch(1, 1);
    timeLayout->addWidget(new QLabel(tr("Time to next keyframe (sec):"), 0, 0));
    selectFrameTime_ = new QDoubleSpinBox();
    selectFrameTime_->setSingleStep(0.1);
    selectFrameTime_->setRange(0.01, 60.0);
    selectFrameTime_->setValue(timeOffset_);
    timeLayout->addWidget(selectFrameTime_, 0, 1);
    gridLayout->addLayout(timeLayout, 1, 0, 1, 2);

    appendKeyframe_ = new QPushButton(tr("Append Frame"), this);
    gridLayout->addWidget(appendKeyframe_, 2, 0, 1, 2);

    insertKeyframe_ = new QPushButton(tr("Insert Frame"));
    updateKeyframe_ = new QPushButton(tr("Update Frame"), editBox_);
    gridLayout->addWidget(insertKeyframe_, 3, 0);
    gridLayout->addWidget(updateKeyframe_, 3, 1);

    deleteKeyframe_ = new QPushButton(tr("Delete Frame"), editBox_);
    clearAnimation_ = new QPushButton(tr("Clear Animation"), this);
    gridLayout->addWidget(deleteKeyframe_, 4, 0);
    gridLayout->addWidget(clearAnimation_, 4, 1);

    QFrame* separator = new QFrame();
    separator->setFrameStyle(QFrame::HLine);
    gridLayout->addWidget(separator, 5, 0, 1, 0);

    hboxLayout = new QHBoxLayout();
    saveAnimation_ = new QPushButton(tr("Save Animation"), this);
    gridLayout->addWidget(saveAnimation_, 6, 0);
    loadAnimation_ = new QPushButton(tr("Load Animation"), this);
    gridLayout->addWidget(loadAnimation_, 6, 1);

    editBox_->setLayout(gridLayout);
    mainLayout->addWidget(editBox_);

    // Recording group box
    recordBox_ = new QGroupBox(tr("Recording"));

    gridLayout = new QGridLayout();

    spinRecordingFPS_ = new QSpinBox(recordBox_);
    spinRecordingFPS_->setRange(1, 100);
    spinRecordingFPS_->setValue(25);
    hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(new QLabel(tr("FPS: ")));
    hboxLayout->addWidget(spinRecordingFPS_);
    hboxLayout->addStretch();
    gridLayout->addLayout(hboxLayout, 0, 0);

    saveAsVideoButton_ = new QPushButton(tr("Save as video-file"));
    saveAsFrameSequenceButton_ = new QPushButton(tr("Save as frame-sequence"));
    // JE: TODO check wether these are placed next to each other and nicely auto-sized
    gridLayout->addWidget(saveAsFrameSequenceButton_, 0, 1);
#ifdef VRN_WITH_FFMPEG
    gridLayout->addWidget(saveAsVideoButton_, 0, 2);
#endif

    gridLayout->addWidget(new QLabel(tr("Video dimensions: ")), 1, 0);

    hboxLayout = new QHBoxLayout();
    hboxLayout->addWidget(spinWidth_ = new QSpinBox(recordBox_));
    hboxLayout->addWidget(new QLabel(" x ", 0, 0));
    hboxLayout->addWidget(spinHeight_ = new QSpinBox(recordBox_));
    spinWidth_->setRange(64, 1280);
    spinHeight_->setRange(64, 1024);
    spinWidth_->setSingleStep(64);
    spinHeight_->setSingleStep(64);
    spinWidth_->setValue(512);
    spinHeight_->setValue(512);
    gridLayout->addLayout(hboxLayout, 1, 1);

    recordBox_->setLayout(gridLayout);
    mainLayout->addWidget(recordBox_);

    mainLayout->addStretch();
    setLayout(mainLayout);

    frameMenu_->setTearOffEnabled(true);

    setWidgetState();
}

void AnimationPlugin::createConnections() {
    connect(appendKeyframe_, SIGNAL(clicked()), this, SLOT(appendKeyframe()));
    connect(insertKeyframe_, SIGNAL(clicked()), this, SLOT(insertKeyframe()));
    connect(updateKeyframe_, SIGNAL(clicked()), this, SLOT(updateKeyframe()));
    connect(deleteKeyframe_, SIGNAL(clicked()), this, SLOT(deleteKeyframe()));
    connect(clearAnimation_, SIGNAL(clicked()), this, SLOT(clearAnimation()));

    connect(saveAnimation_, SIGNAL(clicked()), this, SLOT(saveAnimation()));
    connect(loadAnimation_, SIGNAL(clicked()), this, SLOT(loadAnimation()));

    connect(startAnimation_, SIGNAL(clicked()), this, SLOT(startAnimation()));
    connect(stopAnimation_, SIGNAL(clicked()), this, SLOT(stopAnimation()));
    connect(pauseAnimation_, SIGNAL(clicked()), this, SLOT(pauseAnimation()));

    connect(saveAsFrameSequenceButton_, SIGNAL(clicked()), this, SLOT(
            recordAnimationFrameSeq()));
    connect(saveAsVideoButton_, SIGNAL(clicked()), this, SLOT(
            recordAnimationVideo()));

    connect(frameMenu_, SIGNAL(triggered(QAction*)), this, SLOT(selectFrame(QAction*)));

    connect(selectFrameTime_, SIGNAL(valueChanged(double)), this, SLOT(updateFrameTime(double)));
}

void AnimationPlugin::appendKeyframe() {
    KeyFrame frame;
    frame.position_ = camera_->getPosition();
    frame.focus_ = camera_->getFocus();
    frame.up_ = camera_->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());

    currentAnimation_.push_back(frame);

    refreshFrameMenu();
    selectFrame(currentAnimation_.size() - 1, false);

    setWidgetState();
}

void AnimationPlugin::updateKeyframe() {
    if (currentAnimation_.empty() || (size_t) currentKeyframe_
            > currentAnimation_.size() - 1)
        return;

    KeyFrame frame;
    frame.position_ = camera_->getPosition();
    frame.focus_ = camera_->getFocus();
    frame.up_ = camera_->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());

    currentAnimation_[currentKeyframe_] = frame;

    refreshFrameMenu();
    selectFrame(currentKeyframe_, false);

}

void AnimationPlugin::clearAnimation() {
    if (QMessageBox::question(this, tr("Clear Animation"), tr(
            "Remove all keyframes?"), QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel) == QMessageBox::Yes) {
        currentAnimation_.clear();
        currentKeyframe_ = 0;
        refreshFrameMenu();

        setWidgetState();
    }
}

void AnimationPlugin::startAnimation(bool record) {
    tgtAssert(static_cast<int>(currentAnimation_.size()) >= 2, "At least 2 control points necessary");

    MsgDistr.postMessage(new BoolMsg("switch.trackballSpinning", false));

    if (animationState_ != Paused) {

        std::vector<vec3> positionVector;
        std::vector<vec3> focusVector;
        std::vector<vec3> upVector;
        positionVector.push_back((*currentAnimation_.begin()).position_);
        focusVector.push_back((*currentAnimation_.begin()).focus_);
        upVector.push_back((*currentAnimation_.begin()).up_);
        for (size_t i = 0; i < currentAnimation_.size(); ++i) {
            positionVector.push_back(currentAnimation_.at(i).position_);
            focusVector.push_back(currentAnimation_.at(i).focus_);
            upVector.push_back(currentAnimation_.at(i).up_);
        }
        positionVector.push_back((*(currentAnimation_.end() - 1)).position_);
        focusVector.push_back((*(currentAnimation_.end() - 1)).focus_);
        upVector.push_back((*(currentAnimation_.end() - 1)).up_);

        std::vector<float> knotValues;
        knotValues.push_back(0.f);
        knotValues.push_back(0.f);
        float curTime = 0.f;
        for (int i = 1; i < static_cast<int> (currentAnimation_.size()); ++i) {
            curTime += currentAnimation_.at(i - 1).timeToNextFrame_;
            knotValues.push_back(curTime);
        }
        curTime
                += currentAnimation_.at(currentAnimation_.size() - 1).timeToNextFrame_;
        knotValues.push_back(curTime);

        delete camPositionSpline_;
        delete camFocusSpline_;
        delete camUpSpline_;

        camPositionSpline_ = new tgt::BSpline(positionVector, knotValues);
        camFocusSpline_ = new tgt::BSpline(focusVector, knotValues);
        camUpSpline_ = new tgt::BSpline(upVector, knotValues);

        currentAnimationFrame_ = 0;

        int fps = spinRecordingFPS_->value();
        if (record) {
            MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_,
                    false));
            //fps = spinRecordingFPS_->value(); // JE: see below
            numAnimationFrames_ = tgt::iround(curTime * fps);

            MsgDistr.postMessage(new IVec2Msg("msg.resize",
                                              ivec2(spinWidth_->value(), spinHeight_->value())));
            animationState_ = Recording;
#ifdef VRN_WITH_FFMPEG
            if (/*record &&*/ renderingVideo_) {
                VoreenPainter::getVideoEncoder()->startVideoEncoding(
                    recordPathName_.c_str(), static_cast<unsigned int>(fps),
                    spinWidth_->value(), spinHeight_->value());
            }
#endif /* VRN_WITH_FFMPEG */
        }
        else {
            MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
            //fps = spinPlaybackFPS_->value(); // JE: why do the same thing in both if branches?
            numAnimationFrames_ = tgt::iround(curTime * fps);

            animationState_ = Running;
        }

        setWidgetState();

        /* JE: clarify if this is the cycle-time animationStep is called
         FIXME encoding a video may exceed this magic constant,
         which will cause nice racing conditions,
         as ffmpeg runs its own bunch of threads */
        animationTimer_->start(1000 / fps, this);
    }
    else { // animationState_ != Paused
        if (record)
            animationState_ = Recording;
        else
            animationState_ = Running;
        setWidgetState();
    }
}

void AnimationPlugin::stopAnimation() {
    if (animationState_ == Stopped)
        return;

    bool record = (animationState_ == Recording);

    animationTimer_->stop();
    animationState_ = Stopped;
    setWidgetState();

#ifdef VRN_WITH_FFMPEG
    VoreenPainter::getVideoEncoder()->stopVideoEncoding();// this will finish encoded video file writing
#endif

    MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    MsgDistr.postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, camera_));
    if (record && canvas_) {
        MsgDistr.postMessage(new IVec2Msg("msg.resize",
                                          ivec2(canvas_->getWidth(), canvas_->getHeight())));
    }
    repaintCanvases();

}

void AnimationPlugin::pauseAnimation() {
    if (animationState_ != Running && animationState_ != Paused)
        return;

    animationState_ = Paused;

    setWidgetState();

    repaintCanvases();
}

void AnimationPlugin::recordAnimationFrameSeq() {
    recordAnimation(false);
}

void AnimationPlugin::recordAnimationVideo() {
    recordAnimation(true);
}

/**
 * will ask for resulting file(s)' path and start animation
 * @param recordVideo triggers which button was pushed
 * @see #recordAnimationFrameSeq()
 * @see #recordAnimationVideo()
 * @see #createConnections()
 */
void AnimationPlugin::recordAnimation(bool recordVideo) {
    renderingVideo_ = recordVideo;

    QStringList fileList;
    std::vector<std::string> formats;

    if (!recordVideo) {
        QString s
            = QFileDialog::getExistingDirectory(this,
                                                tr("Choose a directory to save the image sequence"),
                                                QDir::homePath());
        if (!s.isEmpty())
            fileList.push_back(s);
    }
#ifdef VRN_WITH_FFMPEG
    else {
        formats = VoreenPainter::getVideoEncoder()->getSupportedFormatsByFileEnding();
        std::stringstream formatsFilter;
        formatsFilter << "Video (";
        for (size_t i = 0; i < formats.size(); ++i) {
            formatsFilter << "*." << formats[i];
            if (i != formats.size()-1)
                formatsFilter << " ";
        }
        formatsFilter << ")";

        QFileDialog dialog(this);
        dialog.setDefaultSuffix(tr("avi"));
        dialog.setWindowTitle(tr("Save as video file"));
        dialog.setDirectory(QDir::homePath());       
        dialog.setFilter(formatsFilter.str().c_str());
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (dialog.exec())
            fileList = dialog.selectedFiles();

        if (fileList.empty())
        return;

        // test ending for validity
        bool validFileEnding = false;
        for (size_t i = 0; i < formats.size(); ++i) {
            std::string s = "." + formats[i];
            if (fileList[0].endsWith(s.c_str())) {
                validFileEnding = true;
                break;
            }
        }

        // append default ending if current one is not valid
        if (!validFileEnding)
            fileList[0].append(".avi");

    }
#endif //VRN_WITH_FFMPEG
    
    recordPathName_ = fileList[0].toStdString();
    
    startAnimation(true);
}

void AnimationPlugin::timerEvent(QTimerEvent* /*event*/) {
    animationStep();
}

void AnimationPlugin::animationStep() {
    if (animationState_ == Running || animationState_ == Recording) {
        if (currentAnimationFrame_ >= numAnimationFrames_) {
            if (checkLoop_->isChecked() && animationState_ != Recording)
                currentAnimationFrame_ = 0;
            else
                stopAnimation();
        } else {
            tgtAssert(camPositionSpline_ && camFocusSpline_ && camUpSpline_, "No spline");

            float param = float(currentAnimationFrame_) / (numAnimationFrames_
                    - 1);
            vec3 position = camPositionSpline_->getPoint(param);
            vec3 focus = camFocusSpline_->getPoint(param);
            vec3 up = camUpSpline_->getPoint(param);

            // update camera position and repaint
            MsgDistr.postMessage(new Vec3Msg("set.cameraPosition", position));
            MsgDistr.postMessage(new Vec3Msg("set.cameraFocus", focus));
            MsgDistr.postMessage(new Vec3Msg("set.cameraUp", up));
            MsgDistr.postMessage(new Message(VoreenPainter::repaint_));

            if (animationState_ == Recording) {
#ifdef VRN_WITH_FFMPEG
                if (renderingVideo_) {
                    // JE: send a msg to the voreen painter to render a frame and send it to the VideoEncoder
                    MsgDistr.postMessage(new Message("msg.paintToEncoder"));
                } else {
#endif
                    // render frame to file
                    char fn[1024];
                    sprintf(fn, "%s%05d%s", std::string(recordPathName_
                                                        + "/frame").c_str(), currentAnimationFrame_, ".png");
                    LINFOC("voreen.qt.animationplugin", "Frame " << currentAnimationFrame_ << " / " << (numAnimationFrames_-1));
                    MsgDistr.postMessage(new StringMsg("msg.paintToFile", std::string(fn)));
#ifdef VRN_WITH_FFMPEG
                }
#endif
            }

            currentAnimationFrame_++;
        }
    }
}

void AnimationPlugin::refreshFrameMenu() {
    frameMenu_->clear();

    for (size_t i = 0; i < currentAnimation_.size(); ++i) {
        QExtAction* act = new QExtAction(QString(tr("Frame %1, %2 secs")).arg(i
                + 1).arg(currentAnimation_[i].timeToNextFrame_), this, i);
        act->setIcon(QIcon(currentAnimation_[i].pic_));
        frameMenu_->addAction(act);
    }
}

void AnimationPlugin::selectFrame(int frameID, bool jumpToFrame) {
    tgtAssert(frameID >= 0 && frameID < static_cast<int>(currentAnimation_.size()), "Invalid frame id");

    currentKeyframe_ = frameID;
    blockSignals(true);
    selectFrameTime_->setValue(currentAnimation_[frameID].timeToNextFrame_);
    blockSignals(false);
    if (jumpToFrame)
        gotoFrame(frameID);

    setWidgetState();
}

void AnimationPlugin::selectFrame(QAction* action) {
    int frameID = dynamic_cast<QExtAction*> (action)->num_;
    selectFrame(frameID);
}

void AnimationPlugin::gotoFrame(int frameID) {
    tgtAssert(frameID >= 0 && frameID < static_cast<int>(currentAnimation_.size()), "Invalid frame id");

    KeyFrame frame = currentAnimation_.at(frameID);
    MsgDistr.postMessage(new Vec3Msg("set.cameraPosition", frame.position_));
    MsgDistr.postMessage(new Vec3Msg("set.cameraFocus", frame.focus_));
    MsgDistr.postMessage(new Vec3Msg("set.cameraUp", frame.up_));
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_));

}

void AnimationPlugin::setWidgetState() {
    editBox_->setEnabled(animationState_ == Stopped);

    bool animationEditable = ((currentAnimation_.size() > 0) && animationState_
            == Stopped);
    updateKeyframe_->setEnabled(animationEditable);
    deleteKeyframe_->setEnabled(animationEditable);
    clearAnimation_->setEnabled(animationEditable);
    saveAnimation_->setEnabled(animationEditable);

    //selectFrameTime_->setEnabled(animationEditable);

    playerBox_->setEnabled(currentAnimation_.size() >= 2);
    startAnimation_->setEnabled(currentAnimation_.size() >= 2
            && (animationState_ == Stopped || animationState_ == Paused));
    pauseAnimation_->setEnabled(animationState_ == Running);
    stopAnimation_->setEnabled(animationState_ != Stopped);
    checkLoop_->setEnabled(animationState_ != Recording);
    spinPlaybackFPS_->setEnabled(animationState_ == Stopped);

    // Recording
#ifndef VRN_WITH_FFMPEG
    saveAsVideoButton_->setEnabled(false); // disable if no ffmpeg available
#endif
    recordBox_->setEnabled(currentAnimation_.size() >= 2 && (animationState_ == Stopped));

    QString animationSize;
    animationSize.setNum(currentAnimation_.size());
    editBox_->setTitle(tr("Editing") + "  (" + tr("Keyframes:") + " "
            + animationSize + ")");

    if (currentAnimation_.empty()) {
        selectKeyframe_->setText(tr("No Animation"));
        selectKeyframe_->setEnabled(false);
    } else {
        selectKeyframe_->setText((QString(tr("Select Keyframe... (current: %1)"))).arg(currentKeyframe_ + 1));
        selectKeyframe_->setEnabled(true);
    }
}

void AnimationPlugin::deleteKeyframe() {
    tgtAssert(currentKeyframe_ >= 0 && currentKeyframe_ < static_cast<int>(currentAnimation_.size()),
              "Invalid keyframe id");

    std::vector<KeyFrame>::iterator it = currentAnimation_.begin();
    it += currentKeyframe_;
    currentAnimation_.erase(it);
    refreshFrameMenu();

    if (!currentAnimation_.empty())
        selectFrame(std::min(currentKeyframe_, static_cast<int> (currentAnimation_.size() - 1)));

    setWidgetState();
}

void AnimationPlugin::insertKeyframe() {

    tgtAssert(camera_, "No camera"); tgtAssert(currentKeyframe_ >= 0 &&
                                               currentKeyframe_ <= static_cast<int>(currentAnimation_.size()),
            "Invalid keyframe id");

    KeyFrame frame;
    frame.position_ = camera_->getPosition();
    frame.focus_ = camera_->getFocus();
    frame.up_ = camera_->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());

    std::vector<KeyFrame>::iterator iter = currentAnimation_.begin();
    iter += currentKeyframe_;
    currentAnimation_.insert(iter, frame);

    refreshFrameMenu();
    selectFrame(currentKeyframe_, false);

    setWidgetState();

}

void AnimationPlugin::updateFrameTime(double val) {
    tgtAssert(currentKeyframe_ >= 0 && currentKeyframe_ < static_cast<int>(currentAnimation_.size()),
            "Invalid keyframe id");

    currentAnimation_[currentKeyframe_].timeToNextFrame_ = val;

}

void AnimationPlugin::saveAnimation() {
    QString fn = QFileDialog::getSaveFileName(this, tr(
            "Choose a filename to save under"), ".", tr(
            "Voreen animation files (*.vam)"));

    std::string chosenFileName = fn.toStdString();
    if (chosenFileName.length() == 0)
        return;

    TiXmlDocument doc;
    TiXmlComment * comment;
    std::string s;
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(decl);

    TiXmlElement * root = new TiXmlElement("root");
    doc.LinkEndChild(root);

    comment = new TiXmlComment();
    s = "Voreen Animation";
    comment->SetValue(s.c_str());
    root->LinkEndChild(comment);

    TiXmlElement* keyNode = new TiXmlElement("Keyframes");
    root->LinkEndChild(keyNode);

    std::vector<KeyFrame>::iterator iter;

    for (iter = currentAnimation_.begin(); iter != currentAnimation_.end(); ++iter) {
        const KeyFrame& keyframe = *iter;
        TiXmlElement* currKeyframe = new TiXmlElement("Frame");
        keyNode->LinkEndChild(currKeyframe);

        TiXmlElement* position;
        position = new TiXmlElement("position");
        currKeyframe->LinkEndChild(position);
        position->SetDoubleAttribute("x", keyframe.position_.x);
        position->SetDoubleAttribute("y", keyframe.position_.y);
        position->SetDoubleAttribute("z", keyframe.position_.z);

        TiXmlElement* focus;
        focus = new TiXmlElement("focus");
        currKeyframe->LinkEndChild(focus);
        focus->SetDoubleAttribute("x", keyframe.focus_.x);
        focus->SetDoubleAttribute("y", keyframe.focus_.y);
        focus->SetDoubleAttribute("z", keyframe.focus_.z);

        TiXmlElement* up;
        up = new TiXmlElement("up");
        currKeyframe->LinkEndChild(up);
        up->SetDoubleAttribute("x", keyframe.up_.x);
        up->SetDoubleAttribute("y", keyframe.up_.y);
        up->SetDoubleAttribute("z", keyframe.up_.z);

        TiXmlElement* duration;
        duration = new TiXmlElement("timeToNextFrame");
        currKeyframe->LinkEndChild(duration);
        duration->SetDoubleAttribute("t", keyframe.timeToNextFrame_);
    }

    doc.SaveFile(chosenFileName.c_str());
}

void AnimationPlugin::loadAnimation() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open voreen animation"), ".",
                                              tr("Voreen animations (*.vam)"));
    std::string s = fn.toStdString();
    if (s.length() == 0)
        return;

    TiXmlDocument doc(s.c_str());
    if (!doc.LoadFile())
        return;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem;
    TiXmlHandle hRoot(0);

    elem = hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it doesn't
    if (!elem)
        return;

    // save this for later
    hRoot = TiXmlHandle(elem);
    currentAnimation_.clear();
    currentKeyframe_ = 0;

    TiXmlElement* keyNode =
            hRoot.FirstChild("Keyframes").FirstChild("Frame").Element();
    for (; keyNode; keyNode = keyNode->NextSiblingElement()) {
        KeyFrame frame;

        TiXmlElement* position;
        position = keyNode->FirstChildElement();
        position->QueryFloatAttribute("x", &(frame.position_.x));
        position->QueryFloatAttribute("y", &(frame.position_.y));
        position->QueryFloatAttribute("z", &(frame.position_.z));

        TiXmlElement* focus;
        focus = position->NextSiblingElement();
        focus->QueryFloatAttribute("x", &(frame.focus_.x));
        focus->QueryFloatAttribute("y", &(frame.focus_.y));
        focus->QueryFloatAttribute("z", &(frame.focus_.z));

        TiXmlElement* up;
        up = focus->NextSiblingElement();
        up->QueryFloatAttribute("x", &(frame.up_.x));
        up->QueryFloatAttribute("y", &(frame.up_.y));
        up->QueryFloatAttribute("z", &(frame.up_.z));

        TiXmlElement* duration;
        duration = up->NextSiblingElement();
        duration->QueryDoubleAttribute("t", &(frame.timeToNextFrame_));

        currentAnimation_.push_back(frame);
    }

    refreshFrameMenu();
    selectFrame(0, true);
    setWidgetState();
}

} // namespace voreen
