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

#include "voreen/core/application.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"

#include "tgt/animation/bsplineanimation.h"
#include "tgt/animation/circleanimation.h"
#include "tgt/ffmpeg/videoencoder.h"

#include <QApplication>
#include <QLabel>
#include <QMessageBox>

namespace voreen {

AnimationPlugin::AnimationPlugin(QWidget* parent)
    : WidgetPlugin(parent),
    processorNetwork_(0),
    camera_(0),
    canvas_(0),
    painter_(0),
    canvasSize_(256, 256),
    readyState_(false),
    renderingVideo_(false),
    animationState_(Stopped),
    animation_(0),
    currentAnimation_(),
    currentAnimationFrame_(0),
    currentKeyframe_(0),
    numAnimationFrames_(0),
    timeOffset_(0.5f),
    recordPathName_(""),
    animationTimer_(new QBasicTimer())
{
    setObjectName(tr("Animation"));
    createWidgets();
    createConnections();
}

AnimationPlugin::~AnimationPlugin() {
    delete animation_;

    // disconnecting is necessary before deleting controls in order
    // to prevent Qt calls to be send to dead controls
    //
    comboCameras_->disconnect();
    comboCanvases_->disconnect();
    loadSaveBox_->disconnect();
    playerBox_->disconnect();
    renderBox_->disconnect();
    editBox_->disconnect();

    delete tabFrames_;
    delete tabCircle_;
    delete loadSaveBox_;
    delete playerBox_;
    delete renderBox_;
    delete editBox_;
    delete mainLayout_;

    delete animationTimer_;
}

void AnimationPlugin::networkChanged() {
    if (processorNetwork_ == 0)
        return;

    readyState_ = false;

    camera_ = 0;
    canvas_ = 0;
    painter_ = 0;
    allCanvases_.clear();
    allCameras_.clear();

    // Get all canvases from all CanvasRenderer which currently exist within the network
    // and all CameraProperties from all Processors.
    //
    const std::vector<Processor*>& processors = processorNetwork_->getProcessors();
    for (size_t i = 0; i < processors.size(); ++i) {
        CanvasRenderer* cr = dynamic_cast<CanvasRenderer*>(processors[i]);
        if (cr != 0) {
            tgt::QtCanvas* canvas = dynamic_cast<tgt::QtCanvas*>(cr->getCanvas());
            if (canvas != 0)
                allCanvases_.insert(std::make_pair(canvas, cr->getName()));
        }

        const std::vector<Property*>& properties = processors[i]->getProperties();
        for (size_t j = 0; j < properties.size(); ++j) {
            CameraProperty* cp = dynamic_cast<CameraProperty*>(properties[j]);
            if ((cp != 0) && (cp->get() != 0))
                allCameras_.insert(std::make_pair(cp, processors[i]->getName()));
        }
    }

    if (allCameras_.empty() == false)
        camera_ = (allCameras_.begin())->first;

    if (allCanvases_.empty() == false) {
        canvas_ = allCanvases_.begin()->first;
        if (canvas_ != 0)
            painter_ = dynamic_cast<VoreenPainter*>(canvas_->getPainter());
    }

    refreshComboBoxes();
    updateReadyState();
}

void AnimationPlugin::setNetwork(ProcessorNetwork* processorNetwork) {
    processorNetwork_ = processorNetwork;
    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    networkChanged();
}

// private slots
//

void AnimationPlugin::controlledCameraChanged(int index) {
    if ((comboCameras_ == 0) || (index < 0) || (index >= comboCameras_->count()))
        return;
    camera_ = reinterpret_cast<CameraProperty*>(comboCameras_->itemData(index).toULongLong());
    updateReadyState();
}

void AnimationPlugin::controlledCanvasChanged(int index) {
    if ((comboCanvases_ == 0) || (index < 0) || (index >= comboCanvases_->count()))
        return;
    canvas_ = reinterpret_cast<tgt::QtCanvas*>(comboCanvases_->itemData(index).toULongLong());
    if (canvas_ != 0)
        painter_ = dynamic_cast<VoreenPainter*>(canvas_->getPainter());
    updateReadyState();
}

void AnimationPlugin::clearAnimation() {
    if (QMessageBox::question(this, tr("Clear Animation"), tr("Remove all keyframes?"),
        (QMessageBox::Yes | QMessageBox::Cancel), QMessageBox::Cancel) == QMessageBox::Yes)
    {
        currentAnimation_.clear();
        currentKeyframe_ = 0;

        refreshFrameMenu();
        setWidgetState();
    }
}

void AnimationPlugin::loadAnimation() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open voreen animation"),
        ".", tr("Voreen animations (*.vam)"));
    std::string s = fn.toStdString();
    if (s.length() == 0)
        return;

    TiXmlDocument doc(s.c_str());
    if (!doc.LoadFile())
        return;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* elem = 0;
    TiXmlHandle hRoot(0);

    elem = hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it doesn't
    if (!elem)
        return;

    // save this for later
    hRoot = TiXmlHandle(elem);
    currentAnimation_.clear();
    currentKeyframe_ = 0;

    TiXmlElement* keyNode = hRoot.FirstChild("Keyframes").FirstChild("Frame").Element();
    for (; keyNode != 0; keyNode = keyNode->NextSiblingElement()) {
        KeyFrameQt frame;

        TiXmlElement* position = keyNode->FirstChildElement();
        position->QueryFloatAttribute("x", &(frame.position_.x));
        position->QueryFloatAttribute("y", &(frame.position_.y));
        position->QueryFloatAttribute("z", &(frame.position_.z));

        TiXmlElement* focus = position->NextSiblingElement();
        focus->QueryFloatAttribute("x", &(frame.focus_.x));
        focus->QueryFloatAttribute("y", &(frame.focus_.y));
        focus->QueryFloatAttribute("z", &(frame.focus_.z));

        TiXmlElement* up = focus->NextSiblingElement();
        up->QueryFloatAttribute("x", &(frame.up_.x));
        up->QueryFloatAttribute("y", &(frame.up_.y));
        up->QueryFloatAttribute("z", &(frame.up_.z));

        TiXmlElement* duration = up->NextSiblingElement();
        duration->QueryDoubleAttribute("t", &(frame.timeToNextFrame_));

        currentAnimation_.push_back(frame);
    }

    refreshFrameMenu();
    selectFrame(0, true);
    setWidgetState();
}

void AnimationPlugin::pauseAnimation() {
    if ((animationState_ != Running) && (animationState_ != Paused))
        return;

    animationState_ = Paused;
    setWidgetState();
}

void AnimationPlugin::recordAnimationFrameSeq() {
    recordAnimation(false);
}

void AnimationPlugin::recordAnimationVideo() {
    recordAnimation(true);
}

void AnimationPlugin::saveAnimation() {
    QString fn = QFileDialog::getSaveFileName(this, tr("Choose a filename to save under"),
        ".", tr("Voreen animation files (*.vam)"));

    std::string chosenFileName = fn.toStdString();
    if (chosenFileName.length() == 0)
        return;

    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(decl);

    TiXmlElement * root = new TiXmlElement("root");
    doc.LinkEndChild(root);

    TiXmlComment* comment = new TiXmlComment();
    std::string s("Voreen Animation");
    comment->SetValue(s.c_str());
    root->LinkEndChild(comment);

    TiXmlElement* keyNode = new TiXmlElement("Keyframes");
    root->LinkEndChild(keyNode);

     for (std::vector<KeyFrameQt>::iterator iter = currentAnimation_.begin();
         iter != currentAnimation_.end(); ++iter)
     {
        const KeyFrameQt& keyframe = *iter;
        TiXmlElement* currKeyframe = new TiXmlElement("Frame");
        keyNode->LinkEndChild(currKeyframe);

        TiXmlElement* position = new TiXmlElement("position");
        currKeyframe->LinkEndChild(position);
        position->SetDoubleAttribute("x", keyframe.position_.x);
        position->SetDoubleAttribute("y", keyframe.position_.y);
        position->SetDoubleAttribute("z", keyframe.position_.z);

        TiXmlElement* focus = new TiXmlElement("focus");
        currKeyframe->LinkEndChild(focus);
        focus->SetDoubleAttribute("x", keyframe.focus_.x);
        focus->SetDoubleAttribute("y", keyframe.focus_.y);
        focus->SetDoubleAttribute("z", keyframe.focus_.z);

        TiXmlElement* up = new TiXmlElement("up");
        currKeyframe->LinkEndChild(up);
        up->SetDoubleAttribute("x", keyframe.up_.x);
        up->SetDoubleAttribute("y", keyframe.up_.y);
        up->SetDoubleAttribute("z", keyframe.up_.z);

        TiXmlElement* duration = new TiXmlElement("timeToNextFrame");
        currKeyframe->LinkEndChild(duration);
        duration->SetDoubleAttribute("t", keyframe.timeToNextFrame_);
    }

    doc.SaveFile(chosenFileName.c_str());
}

void AnimationPlugin::startAnimation(const bool record) {
    if (! readyState_)
        return;

    // if already animating
    if ((animationState_ != Stopped) && (animationState_ != Paused)) {
        startAnimation_->setEnabled(false);
        return;
    }

    // gather some useful params
    int fps = (record) ? spinRecordingFPS_->value() : spinPlaybackFPS_->value();

    // check animation requirements and start it
    if (editBox_->currentIndex() == editBox_->indexOf(tabFrames_)) {
        if (currentAnimation_.size() < 2) {
            QMessageBox msg;
            msg.setText(tr("At least two frames are necessary for playback."));
            msg.exec();
            return;
        } else if (animationState_ != Paused) {
            delete animation_;
            animation_ = new tgt::BSplineAnimation(keyFrameVec(currentAnimation_), fps);
            currentAnimationFrame_ = 0;
        }
    } else if (editBox_->currentIndex() == editBox_->indexOf(tabCircle_)) {
        if ((selectCircleAngle_ == 0) || (selectCircleSpeed_ == 0)) {
            QMessageBox msg;
            msg.setText(tr("Zero angle or speed are not supported."));
            msg.exec();
            return;
        } else if (animationState_ != Paused) {
            delete animation_;
            animation_ = new tgt::CircleAnimation(selectCircleAngle_->value(),
                selectCircleSpeed_->value(), fps, camera_->get());
            currentAnimationFrame_ = 0;
        }
    }

    // set accurate animation-state (if not exited/returned out of here yet)
    // set accurate recording dimension
    if (record) {
        canvasSize_ = canvas_->getSize();
        canvas_->resize(spinWidth_->value(), spinHeight_->value());
        animationState_ = Recording;

#ifdef VRN_WITH_FFMPEG
        if (renderingVideo_) {
            ffmpegEncoder_.startVideoEncoding(recordPathName_.c_str(),
                fps, spinWidth_->value(), spinHeight_->value());
        }
#endif /* VRN_WITH_FFMPEG */
    } else {
        animationState_ = Running;
		camera_->toggleInteractionMode(true, this);
	}

    setWidgetState();

    if (fps > 0)
        animationTimer_->start(1000 / fps, this);
}

void AnimationPlugin::stopAnimation() {
    if (animationState_ == Stopped) { // nothing to do
        stopAnimation_->setEnabled(false);
        return;
    }

    animationTimer_->stop();
    bool record = (animationState_ == Recording);
    animationState_ = Stopped;

    setWidgetState();

#ifdef VRN_WITH_FFMPEG
    if (record && renderingVideo_) {
        // this will finish encoded video file writing
        ffmpegEncoder_.stopVideoEncoding();
        renderingVideo_ = false;
    }
#endif
	if(!record)
		camera_->toggleInteractionMode(false, this);

    // reset Canvas Viewing
    if (record && canvas_)
        canvas_->resize(canvasSize_.x, canvasSize_.y);

    camera_->notifyChange();
}

void AnimationPlugin::selectFrame(QAction* action) {
    QExtAction* xAction = dynamic_cast<QExtAction*>(action);
    int frameID = (xAction != 0) ? (xAction->num_) : 0;
    selectFrame(frameID);
}

void AnimationPlugin::selectFrame(int frameID, bool jumpToFrame) {
    tgtAssert((frameID >= 0) && (frameID < static_cast<int>(currentAnimation_.size())),
        "Invalid frame id");

    currentKeyframe_ = frameID;
    blockSignals(true);
    selectFrameTime_->setValue(currentAnimation_[frameID].timeToNextFrame_);
    blockSignals(false);
    if (jumpToFrame)
        gotoFrame(frameID);

    setWidgetState();
}

void AnimationPlugin::appendKeyframe() {
    if (readyState_ == false)
        return;

    KeyFrameQt frame;
    frame.position_ = camera_->get()->getPosition();
    frame.focus_ = camera_->get()->getFocus();
    frame.up_ = camera_->get()->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());
    currentAnimation_.push_back(frame);

    refreshFrameMenu();
    selectFrame(currentAnimation_.size() - 1, false);
    setWidgetState();
}

void AnimationPlugin::deleteKeyframe() {
    tgtAssert((currentKeyframe_ >= 0) && (currentKeyframe_ < static_cast<int>(currentAnimation_.size())),
        "Invalid keyframe id");

    std::vector<KeyFrameQt>::iterator it = currentAnimation_.begin();
    it += currentKeyframe_;
    currentAnimation_.erase(it);
    refreshFrameMenu();

    if (currentAnimation_.empty() == false)
        selectFrame(std::min(currentKeyframe_, static_cast<int>(currentAnimation_.size() - 1)));
    setWidgetState();
}

void AnimationPlugin::insertKeyframe() {
    if (readyState_ == false)
        return;

    tgtAssert((currentKeyframe_ >= 0) && (currentKeyframe_ <= static_cast<int>(currentAnimation_.size())),
        "Invalid keyframe id");

    KeyFrameQt frame;
    frame.position_ = camera_->get()->getPosition();
    frame.focus_ = camera_->get()->getFocus();
    frame.up_ = camera_->get()->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());

    std::vector<KeyFrameQt>::iterator iter = currentAnimation_.begin();
    iter += currentKeyframe_;
    currentAnimation_.insert(iter, frame);

    refreshFrameMenu();
    selectFrame(currentKeyframe_, false);
    setWidgetState();
}

void AnimationPlugin::updateKeyframe() {
    if (readyState_ == false)
        return;

    if ((currentAnimation_.empty() == true)
        || (static_cast<size_t>(currentKeyframe_) > (currentAnimation_.size() - 1)))
    {
        return;
    }

    KeyFrameQt frame;
    frame.position_ = camera_->get()->getPosition();
    frame.focus_ = camera_->get()->getFocus();
    frame.up_ = camera_->get()->getUpVector();
    frame.timeToNextFrame_ = selectFrameTime_->value();
    if (canvas_)
        frame.pic_ = QPixmap::grabWindow(canvas_->winId());

    currentAnimation_[currentKeyframe_] = frame;

    refreshFrameMenu();
    selectFrame(currentKeyframe_, false);
}

void AnimationPlugin::setWidgetState() {
    editBox_->setEnabled(animationState_ == Stopped);

    bool animationEditable = ((currentAnimation_.size() > 0) && (animationState_ == Stopped)
        && (editBox_->currentIndex() == editBox_->indexOf(tabFrames_)));
    updateKeyframe_->setEnabled(animationEditable);
    deleteKeyframe_->setEnabled(animationEditable);
    clearAnimation_->setEnabled(animationEditable);
    saveAnimation_->setEnabled(animationEditable);

    loadAnimation_->setEnabled((editBox_->currentIndex() == editBox_->indexOf(tabFrames_))
        && (animationState_ == Stopped));

    //selectFrameTime_->setEnabled(animationEditable);

    startAnimation_->setEnabled( ((currentAnimation_.size() >= 2)
        || (editBox_->currentIndex() == editBox_->indexOf(tabCircle_)))
        && ((animationState_ == Stopped) || (animationState_ == Paused)) );
    comboCanvases_->setEnabled(animationState_ == Stopped);
    comboCameras_->setEnabled(animationState_ == Stopped);
    pauseAnimation_->setEnabled(animationState_ == Running);
    stopAnimation_->setEnabled(animationState_ != Stopped);
    checkLoop_->setEnabled(animationState_ != Recording);
    spinPlaybackFPS_->setEnabled(animationState_ == Stopped);

#ifndef VRN_WITH_FFMPEG
    saveAsVideoButton_->setEnabled(false); // disable if no ffmpeg available
#endif

    QString animationSize;
    animationSize.setNum(currentAnimation_.size());

    if (currentAnimation_.empty()) {
        selectKeyframe_->setText(tr("No Animation"));
        selectKeyframe_->setEnabled(false);
    } else {
        selectKeyframe_->setText(
            (QString(tr("Select Keyframe... (current: %1)"))).arg(currentKeyframe_ + 1) );
        selectKeyframe_->setEnabled(true);
    }
}

void AnimationPlugin::tabChanged(int /*index*/) {
    setWidgetState();
}

void AnimationPlugin::videoSetup() {
#ifdef VRN_WITH_FFMPEG
    int curPreset = ffmpegEncoder_.getPreset();
    int curBitrate = ffmpegEncoder_.getBitrate();
    QDialog* dialog = createVideoSetupDialog(this, curPreset, curBitrate);
    if (dialog->exec() == QDialog::Accepted)
        ffmpegEncoder_.setup(preset_->currentIndex(), bitrate_->value() * 1024);

    delete dialog;
#endif
}

// private methods
//

void AnimationPlugin::animationStep() {
    if ((! readyState_) || (animation_ == 0))
        return;

    if ((animationState_ != Running) && (animationState_ != Recording))
        return;

    if (currentAnimationFrame_ >= animation_->getNumFrames()) {
        if (checkLoop_->isChecked() && animationState_ != Recording)
            currentAnimationFrame_ = 0;
        else
            stopAnimation();
    } else {
        // at this point we don't mind the exact implementation of AbstractAnimation
        //
        animation_->moveToFrame(currentAnimationFrame_);

        // update camera position and repaint
        camera_->get()->setPosition(animation_->getEye());
        camera_->get()->setFocus(animation_->getCenter());
        camera_->get()->setUpVector(animation_->getUp());
        camera_->notifyChange();

        if (animationState_ == Recording) {
#ifdef VRN_WITH_FFMPEG
            if ((renderingVideo_) && (painter_->getCanvasRenderer())) {
                tgt::Texture* texture = painter_->getCanvasRenderer()->getImageColorTexture();
                if (texture) {
                    texture->downloadTexture();
                    ffmpegEncoder_.nextFrame(texture->getPixelData());
                }
            } else {
#endif
                // render frame to file
                char fn[1024];
                sprintf(fn, "%s%05d%s", std::string(recordPathName_ + "/frame").c_str(),
                    currentAnimationFrame_, ".png");
                try {
                    painter_->renderToSnapshot(fn, canvas_->getSize());
                } catch (...) {}    // not very good, but better than not catching the exception at all
#ifdef VRN_WITH_FFMPEG
            }
#endif
        }

        ++currentAnimationFrame_;
    }
}

void AnimationPlugin::createWidgets() {
    mainLayout_ = new QVBoxLayout();

    playerBox_ = createAnimationPlayerBox(this);
    mainLayout_->addWidget(playerBox_);

    tabFrames_ = createAnimationFramesTab();
    tabCircle_ = createAnimationCircleTab();
    editBox_ = new QTabWidget(this);
    editBox_->addTab(tabFrames_, tr("Keyframes"));
    editBox_->addTab(tabCircle_, tr("Simple Rotation"));
    mainLayout_->addWidget(editBox_);

    loadSaveBox_ = new QGroupBox(tr("Load/Save"));
    QHBoxLayout* loadSaveLayout = new QHBoxLayout();
    loadAnimation_ = new QPushButton(tr("Load Animation"), loadSaveBox_);
    saveAnimation_ = new QPushButton(tr("Save Animation"), loadSaveBox_);
    loadSaveLayout->addWidget(loadAnimation_);
    loadSaveLayout->addWidget(saveAnimation_);
    loadSaveBox_->setLayout(loadSaveLayout);
    mainLayout_->addWidget(loadSaveBox_);

    renderBox_ = createAnimationRenderBox(this);
    mainLayout_->addWidget(renderBox_);

    setLayout(mainLayout_);
    setWidgetState();
}

void AnimationPlugin::createConnections() {
    connect(comboCanvases_, SIGNAL(currentIndexChanged(int)), this, SLOT(controlledCanvasChanged(int)));
    connect(comboCameras_, SIGNAL(currentIndexChanged(int)), this, SLOT(controlledCameraChanged(int)));

    connect(editBox_, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(frameMenu_, SIGNAL(triggered(QAction*)), this, SLOT(selectFrame(QAction*)));

    connect(appendKeyframe_, SIGNAL(clicked()), this, SLOT(appendKeyframe()));
    connect(deleteKeyframe_, SIGNAL(clicked()), this, SLOT(deleteKeyframe()));
    connect(insertKeyframe_, SIGNAL(clicked()), this, SLOT(insertKeyframe()));
    connect(updateKeyframe_, SIGNAL(clicked()), this, SLOT(updateKeyframe()));

    connect(clearAnimation_, SIGNAL(clicked()), this, SLOT(clearAnimation()));
    connect(loadAnimation_, SIGNAL(clicked()), this, SLOT(loadAnimation()));
    connect(pauseAnimation_, SIGNAL(clicked()), this, SLOT(pauseAnimation()));
    connect(saveAnimation_, SIGNAL(clicked()), this, SLOT(saveAnimation()));
    connect(startAnimation_, SIGNAL(clicked()), this, SLOT(startAnimation()));
    connect(stopAnimation_, SIGNAL(clicked()), this, SLOT(stopAnimation()));

    connect(saveAsFrameSequenceButton_, SIGNAL(clicked()), this,
        SLOT(recordAnimationFrameSeq()));
    connect(saveAsVideoButton_, SIGNAL(clicked()), this, SLOT(recordAnimationVideo()));
    connect(videoSetupButton_, SIGNAL(clicked()), this, SLOT(videoSetup()));
}

void AnimationPlugin::gotoFrame(int frameID) {
    if (! readyState_)
        return;

    tgtAssert((frameID >= 0) && (frameID < static_cast<int>(currentAnimation_.size())),
        "Invalid frame id");

    AnimationPlugin::KeyFrameQt frame = currentAnimation_[frameID];

    // update camera position and repaint
    camera_->get()->setPosition(frame.position_);
    camera_->get()->setFocus(frame.focus_);
    camera_->get()->setUpVector(frame.up_);
    camera_->notifyChange();
}

std::vector<KeyFrame> AnimationPlugin::keyFrameVec(const std::vector<AnimationPlugin::KeyFrameQt>& vec) {
    std::vector<KeyFrame> res;
    for (vector<KeyFrameQt>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter)
        res.push_back(static_cast<KeyFrame>(*iter));
    return res;
}

void AnimationPlugin::recordAnimation(bool recordVideo) {
    renderingVideo_ = recordVideo;

    QStringList fileList;
    if (renderingVideo_ == false) {
        QString s = QFileDialog::getExistingDirectory(this,
            tr( "Choose a directory to save the image sequence"),
            VoreenApplication::app()->getDocumentsPath().c_str());

        if (!s.isEmpty())
            fileList.push_back(s);
    }
#ifdef VRN_WITH_FFMPEG
    else {
        std::vector<std::string> formats = tgt::VideoEncoder::getSupportedFormatsByFileEnding();
        QStringList lstFormats;
        for (size_t i = 0; i < formats.size(); ++i) {
            formats[i] = ("*." + formats[i]);
            lstFormats.append(formats[i].c_str());
        }

        QFileDialog dialog(this);
        dialog.setDefaultSuffix(ffmpegEncoder_.getContainerAppendix());
        dialog.setWindowTitle(tr("Save as video file"));
        dialog.setDirectory(VoreenApplication::app()->getDocumentsPath().c_str());
#if QT_VERSION >= 0x040400
        dialog.setNameFilters(lstFormats);
#endif
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        if (dialog.exec())
            fileList = dialog.selectedFiles();
    }
#endif // VRN_WITH_FFMPEG
    if (fileList.size() > 0) {
        recordPathName_ = fileList.first().toStdString();
        startAnimation(true);
    }
}

void AnimationPlugin::refreshComboBoxes() {
    if (comboCanvases_ != 0) {
        comboCanvases_->clear();
        for (CanvasMap::const_iterator it = allCanvases_.begin(); it != allCanvases_.end(); ++it)
            comboCanvases_->addItem(tr(it->second.c_str()), reinterpret_cast<qulonglong>(it->first));
    }

    if (comboCameras_ != 0) {
        comboCameras_->clear();
        for (CameraMap::const_iterator it = allCameras_.begin(); it != allCameras_.end(); ++it)
            comboCameras_->addItem(tr(it->second.c_str()), reinterpret_cast<qulonglong>(it->first));
    }
}

void AnimationPlugin::refreshFrameMenu() {
    frameMenu_->clear();

    for (size_t i = 0; i < currentAnimation_.size(); ++i) {
        QExtAction* act = new QExtAction(QString(tr("Frame %1, %2 secs")).arg(i + 1).arg(
            currentAnimation_[i].timeToNextFrame_), this, i);
        act->setIcon(QIcon(currentAnimation_[i].pic_));
        frameMenu_->addAction(act);
    }
}

void AnimationPlugin::timerEvent(QTimerEvent*) {
    animationStep();
}

void AnimationPlugin::updateReadyState() {
    if ((canvas_ != 0) && (camera_ != 0) && (camera_->get() != 0)
        && (processorNetwork_ != 0) && (painter_ != 0)) {
        readyState_ = true;
    } else
        readyState_ = false;
}

QGroupBox* AnimationPlugin::createAnimationPlayerBox(QWidget* parent)
{
    QGroupBox* playerBox = new QGroupBox(tr("Playback"), parent);
    playerBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QVBoxLayout* playerLayout = new QVBoxLayout();

    QHBoxLayout* playbackRow = new QHBoxLayout();
    comboCameras_ = new QComboBox(playerBox);
    playbackRow->addWidget(new QLabel(tr("Camera"), playerBox));
    playbackRow->addWidget(comboCameras_);
    playerLayout->addLayout(playbackRow);

    // playback controls
    playbackRow = new QHBoxLayout();
    startAnimation_ = new QPushButton(playerBox);
    startAnimation_->setIcon(QIcon(":/voreenve/icons/player-start.png"));
    playbackRow->addWidget(startAnimation_);

    pauseAnimation_ = new QPushButton(playerBox);
    pauseAnimation_->setIcon(QIcon(":/voreenve/icons/player-pause.png"));
    pauseAnimation_->setEnabled(false); // just enable during playback or recording
    playbackRow->addWidget(pauseAnimation_);

    stopAnimation_ = new QPushButton(playerBox);
    stopAnimation_->setIcon(QIcon(":/voreenve/icons/player-stop.png"));
    stopAnimation_->setEnabled(false);// just enable during playback or recording
    playbackRow->addWidget(stopAnimation_);

    playerLayout->addLayout(playbackRow);

    // playback meta controls
    //
    playbackRow = new QHBoxLayout();
    playbackRow->addWidget(new QLabel(tr("FPS "), playerBox));
    spinPlaybackFPS_ = new QSpinBox(playerBox);
    spinPlaybackFPS_->setRange(1, 100);
    spinPlaybackFPS_->setValue(25);
    playbackRow->addWidget(spinPlaybackFPS_);
    playbackRow->addStretch();

    checkLoop_ = new QCheckBox(tr("Play as loop"), playerBox);
    checkLoop_->setChecked(false);
    playbackRow->addWidget(checkLoop_);
    playbackRow->addStretch();

    playerLayout->addLayout(playbackRow);
    playerBox->setLayout(playerLayout);
    return playerBox;
}

QWidget* AnimationPlugin::createAnimationFramesTab() {
    QWidget* tabFrames = new QWidget();
    QGridLayout* groupLayout = new QGridLayout();

    selectKeyframe_ = new QPushButton(tr("Select Keyframe..."), tabFrames);
    frameMenu_ = new QMenu(selectKeyframe_);
    frameMenu_->setTearOffEnabled(true);
    selectKeyframe_->setMenu(frameMenu_);
    groupLayout->addWidget(selectKeyframe_, 0, 0, 1, 2);

    QGridLayout* timeLayout = new QGridLayout();
    timeLayout->setColumnStretch(0, 2);
    timeLayout->setColumnStretch(1, 1);
    timeLayout->addWidget(new QLabel(tr("Time to next keyframe (sec)"), 0, 0));

    selectFrameTime_ = new QDoubleSpinBox();
    selectFrameTime_->setSingleStep(0.1);
    selectFrameTime_->setRange(0.01, 60.0);
    selectFrameTime_->setValue(timeOffset_);
    timeLayout->addWidget(selectFrameTime_, 0, 1);
    groupLayout->addLayout(timeLayout, 1, 0, 1, 2);

    appendKeyframe_ = new QPushButton(tr("Append Frame"), tabFrames);
    groupLayout->addWidget(appendKeyframe_, 2, 0, 1, 2);

    clearAnimation_ = new QPushButton(tr("Clear Animation"), tabFrames);
    groupLayout->addWidget(clearAnimation_, 4, 1);

    deleteKeyframe_ = new QPushButton(tr("Delete Frame"), tabFrames);
    groupLayout->addWidget(deleteKeyframe_, 4, 0);

    insertKeyframe_ = new QPushButton(tr("Insert Frame"));
    groupLayout->addWidget(insertKeyframe_, 3, 0);

    updateKeyframe_ = new QPushButton(tr("Update Frame"), tabFrames);
    groupLayout->addWidget(updateKeyframe_, 3, 1);

    tabFrames->setLayout(groupLayout);
    return tabFrames;
}

QWidget* AnimationPlugin::createAnimationCircleTab() {
    QWidget* circleTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    QHBoxLayout* hLayout = new QHBoxLayout();
    selectCircleAngle_ = new QSpinBox(circleTab);
    selectCircleAngle_->setRange(0, 360);
    selectCircleAngle_->setSingleStep(1);
    selectCircleAngle_->setValue(360);
    hLayout->addWidget(new QLabel(tr("Rotation angle")));
    hLayout->addWidget(selectCircleAngle_);
    layout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    selectCircleSpeed_ = new QSpinBox(circleTab);
    selectCircleSpeed_->setRange(1, 60);
    selectCircleSpeed_->setSingleStep(1);
    selectCircleSpeed_->setValue(8);
    hLayout->addWidget(new QLabel(tr("Rotation time (seconds)")));
    hLayout->addWidget(selectCircleSpeed_);
    layout->addLayout(hLayout);

    circleTab->setLayout(layout);
    return circleTab;
}

QGroupBox* AnimationPlugin::createAnimationRenderBox(QWidget* parent) {
    QGroupBox* recordBox = new QGroupBox(tr("Recording"), parent);
    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* rowLayout = new QHBoxLayout();

    comboCanvases_ = new QComboBox(recordBox);
    rowLayout->addWidget(new QLabel(tr("Canvas"), recordBox));
    rowLayout->addWidget(comboCanvases_);
    layout->addLayout(rowLayout);

    // render meta settings
    rowLayout = new QHBoxLayout();
    rowLayout->addWidget(new QLabel(tr("FPS")));
    spinRecordingFPS_ = new QSpinBox(recordBox);
    spinRecordingFPS_->setRange(25, 100);
    spinRecordingFPS_->setValue(25);
    rowLayout->addWidget(spinRecordingFPS_);
    rowLayout->addStretch();

    rowLayout->addWidget(new QLabel(tr("Video dimensions")), 1, 0);
    spinWidth_ = new QSpinBox(recordBox);
    spinHeight_ = new QSpinBox(recordBox);

    spinWidth_->setRange(64, 1280);
    spinWidth_->setSingleStep(4);
    spinWidth_->setValue(512);
    spinWidth_->setAccelerated(true);

    spinHeight_->setRange(64, 1280);
    spinHeight_->setSingleStep(4);
    spinHeight_->setValue(512);
    spinHeight_->setAccelerated(true);

    rowLayout->addWidget(spinWidth_);
    rowLayout->addWidget(new QLabel(" x ", 0, 0));
    rowLayout->addWidget(spinHeight_);
    layout->addLayout(rowLayout);

    // render 2 vid or frameSeq
    rowLayout = new QHBoxLayout();

    saveAsFrameSequenceButton_ = new QPushButton(tr("Save as frame-sequence"));
    saveAsVideoButton_ = new QPushButton(tr("Save as video-file"));
    videoSetupButton_ = new QPushButton(tr("Setup video"));
#ifndef VRN_WITH_FFMPEG
    saveAsVideoButton_->setVisible(false);
    videoSetupButton_->setVisible(false);
#endif
    rowLayout->addWidget(saveAsFrameSequenceButton_);
    rowLayout->addWidget(saveAsVideoButton_);
    rowLayout->addWidget(videoSetupButton_);
    layout->addLayout(rowLayout);

    recordBox->setLayout(layout);
    return recordBox;
}

#ifdef VRN_WITH_FFMPEG
QDialog* AnimationPlugin::createVideoSetupDialog(QWidget* parent, int curPreset, int curBitrate) {
        QDialog* dialog = new QDialog(parent);
        QVBoxLayout* layout = new QVBoxLayout();
        dialog->setLayout(layout);

        // Preset
        //
        QHBoxLayout* rowLayout = new QHBoxLayout();
        preset_ = new QComboBox(dialog);
        rowLayout->addWidget(new QLabel(tr("Preset")));
        rowLayout->addWidget(preset_);
        const char** ccPairNames = tgt::VideoEncoder::getContainerCodecPairNames();
        for (int i = tgt::GUESS; i < tgt::LAST; ++i)
            preset_->addItem(ccPairNames[i]);
        preset_->setCurrentIndex(curPreset);
        layout->addLayout(rowLayout);

        // Bitrate
        //
        rowLayout = new QHBoxLayout();
        bitrate_ = new QSpinBox(dialog);
        rowLayout->addWidget(new QLabel(tr("Bitrate (kbit/s)")));
        rowLayout->addWidget(bitrate_);
        bitrate_->setMinimum(400);
        bitrate_->setMaximum(4000);
        bitrate_->setSingleStep(1);
        bitrate_->setValue(curBitrate / 1024);
        layout->addLayout(rowLayout);

        rowLayout = new QHBoxLayout();
        QPushButton* ok = new QPushButton(QObject::tr("OK"));
        QPushButton* cancel = new QPushButton(QObject::tr("Cancel"));

        connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

        rowLayout->addWidget(ok);
        rowLayout->addWidget(cancel);
        layout->addLayout(rowLayout);

        return dialog;
}
#endif

}   // namespace voreen
