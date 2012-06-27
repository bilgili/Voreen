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
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/messagedistributor.h"

#include "tinyxml/tinyxml.h"

#include "tgt/camera.h"
#include "tgt/qt/qtcanvas.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QInputDialog> // JE: temporary FIXME
#include <QTabWidget>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QBasicTimer>
#include <QFileDialog>
#include <QToolBox>
#include <QMenu>
#include <QCheckBox>
#include <QGroupBox>

#ifdef VRN_WITH_FFMPEG
#include "tgt/ffmpeg/videoencoder.h"
#include <sstream>
#endif

// JE: anims
#include "tgt/animation/abstractanimation.h"
#include "tgt/animation/bsplineanimation.h"
#include "tgt/animation/circleanimation.h"

namespace voreen {

using tgt::vec3;
using tgt::ivec2;
using tgt::BSpline;
using tgt::KeyFrame;


VideoResolutionSpinBox::VideoResolutionSpinBox(QWidget* parent) : QSpinBox(parent) {
    lineEdit()->setReadOnly(true);
	setRange(64, 1280);
	setSingleStep(4);
	setValue(512);
	setAccelerated(true);
}

VideoResolutionSpinBox::~VideoResolutionSpinBox() {
}



//-------------------------------------------------------------------------------

AnimationPlugin::AnimationPlugin(QWidget* parent, tgt::Camera* camera,
                                 tgt::QtCanvas* canvas, tgt::Trackball* trackBall)
    : WidgetPlugin(parent),
      animationState_(Stopped),
      trackBall_(trackBall),
      camera_(camera),
      canvas_(canvas),
      currentKeyframe_(0),
      animation_(0),
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
    delete animation_;
}

namespace {

inline QString tr(const char* c) {
	return QObject::tr(c);
}

QGroupBox *createAnimationPlayerBox(QWidget *parent,
		QPushButton **startAnimation, QPushButton **pauseAnimation,
		QPushButton **stopAnimation, QSpinBox **playbackFps, QCheckBox **repeat) {
	QGroupBox *playerBox = new QGroupBox(tr("Playback"), parent);
	playerBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	QVBoxLayout* playerLayout = new QVBoxLayout();
	QHBoxLayout* playbackRow = new QHBoxLayout(); // used twice

	// playback controls
	*startAnimation = new QPushButton(playerBox);
	(*startAnimation)->setIcon(QIcon(":/icons/player_play.png"));
	//startAnimation->setEnabled(false);//TODO warn in case playback cannot start due to anim's requirements
	playbackRow->addWidget(*startAnimation);

	*pauseAnimation = new QPushButton(playerBox);
	(*pauseAnimation)->setIcon(QIcon(":/icons/player_pause.png"));
	(*pauseAnimation)->setEnabled(false);// just enable during playback or recording
	playbackRow->addWidget(*pauseAnimation);

	*stopAnimation = new QPushButton(playerBox);
	(*stopAnimation)->setIcon(QIcon(":/icons/player_stop.png"));
	(*stopAnimation)->setEnabled(false);// just enable during playback or recording
	playbackRow->addWidget(*stopAnimation);

	playerLayout->addLayout(playbackRow);

	// playback meta controls
	playbackRow = new QHBoxLayout();

	playbackRow->addWidget(new QLabel(tr("FPS: "), playerBox));
	*playbackFps = new QSpinBox(playerBox);
	(*playbackFps)->setRange(25, 100);
	(*playbackFps)->setValue(25);
	playbackRow->addWidget(*playbackFps);

	playbackRow->addStretch();
	*repeat = new QCheckBox(tr("Play as loop"), playerBox);
	(*repeat)->setChecked(false);
	playbackRow->addWidget(*repeat);
	playbackRow->addStretch();

	playerLayout->addLayout(playbackRow);

	playerBox->setLayout(playerLayout);
	return playerBox;
}

QGroupBox *createAnimationEditFramesBox(QWidget *parent,
		QPushButton **selectKeyframe, /**and its dropdown**/
		QMenu **frames, QDoubleSpinBox **selectFrameTime, float offsetTime,
		QPushButton **appendKeyframe, QPushButton **insertKeyframe,
		QPushButton **updateKeyframe, QPushButton **deleteKeyframe,
		QPushButton **clearAnimation)
{
	QGroupBox* editBox = new QGroupBox(tr("Keyframe Animation"));
	QGridLayout *groupLayout = new QGridLayout();

	*selectKeyframe = new QPushButton(tr("Select Keyframe..."), editBox);
	*frames = new QMenu(*selectKeyframe);
	(*frames)->setTearOffEnabled(true);
	(*selectKeyframe)->setMenu(*frames);
	groupLayout->addWidget(*selectKeyframe, 0, 0, 1, 2);

	QGridLayout* timeLayout = new QGridLayout();
	timeLayout->setColumnStretch(0, 2);
	timeLayout->setColumnStretch(1, 1);
	timeLayout->addWidget(new QLabel(tr("Time to next keyframe (sec):"), 0, 0));
	*selectFrameTime = new QDoubleSpinBox();
	(*selectFrameTime)->setSingleStep(0.1);
	(*selectFrameTime)->setRange(0.01, 60.0);
	(*selectFrameTime)->setValue(offsetTime);
	timeLayout->addWidget(*selectFrameTime, 0, 1);
	groupLayout->addLayout(timeLayout, 1, 0, 1, 2);

	*appendKeyframe = new QPushButton(tr("Append Frame"), parent);
	groupLayout->addWidget(*appendKeyframe, 2, 0, 1, 2);

	*insertKeyframe = new QPushButton(tr("Insert Frame"));
	*updateKeyframe = new QPushButton(tr("Update Frame"), editBox);
	groupLayout->addWidget(*insertKeyframe, 3, 0);
	groupLayout->addWidget(*updateKeyframe, 3, 1);

	*deleteKeyframe = new QPushButton(tr("Delete Frame"), editBox);
	*clearAnimation = new QPushButton(tr("Clear Animation"), parent);
	groupLayout->addWidget(*deleteKeyframe, 4, 0);
	groupLayout->addWidget(*clearAnimation, 4, 1);

	editBox->setLayout(groupLayout);
	return editBox;
}

QGroupBox *createAnimationEditCircleBox(QWidget *parent,
		QSpinBox **selectAngle, QSpinBox **selectCircleSteps) {
	QGroupBox *circleEdit = new QGroupBox(tr("Rotation"), parent);
	QVBoxLayout *layout = new QVBoxLayout();

	layout->addWidget(new QLabel(tr("Rotation angle")));
	layout->addWidget(*selectAngle = new QSpinBox(circleEdit));
	(*selectAngle)->setRange(0,360);
	(*selectAngle)->setSingleStep(1);
	(*selectAngle)->setValue(360);
	layout->addWidget(new QLabel(tr("Rotation time (seconds)")));
	layout->addWidget(*selectCircleSteps = new QSpinBox(circleEdit));
	(*selectCircleSteps)->setRange(1,60);
	(*selectCircleSteps)->setSingleStep(1);
	(*selectCircleSteps)->setValue(8);

	circleEdit->setLayout(layout);
	return circleEdit;
}

/**
 * @param framesEditing init outside
 * @param circleEditing init outside
 */
QTabWidget *createTabbedEditBox(QWidget *parent, QWidget *framesEditing,
		QWidget *circleEditing) {
	QTabWidget *editBox = new QTabWidget(parent);

	editBox->addTab(framesEditing, tr("Keyframes")); // id==0
	editBox->addTab(circleEditing, tr("Simple Rotation")); // id==1

	return editBox;
}

QGroupBox *createAnimationLoadSaveBox(QWidget *parent,
		QPushButton **loadAnimation, QPushButton **saveAnimation) {
	QGroupBox *loadSaveBox = new QGroupBox(tr("Load/Save"));
	QHBoxLayout *layout = new QHBoxLayout();

	layout->addWidget(*loadAnimation = new QPushButton(tr("Load Animation"),
			parent));
	layout->addWidget(*saveAnimation = new QPushButton(tr("Save Animation"),
			parent));

	loadSaveBox->setLayout(layout);
	return loadSaveBox;
}

QGroupBox *createAnimationRenderBox(QWidget *parent,
		QPushButton **saveAsFrameSeq, QPushButton **saveAsVideo,
		QSpinBox **recordingFps, VideoResolutionSpinBox **spinWidth, VideoResolutionSpinBox **spinHeight) {
	QGroupBox *recordBox = new QGroupBox(tr("Recording"), parent);
	QVBoxLayout *layout = new QVBoxLayout();

	QHBoxLayout *rowLayout = new QHBoxLayout();//used twice
	// render meta settings
	rowLayout->addWidget(new QLabel(tr("FPS: ")));
	rowLayout->addWidget(*recordingFps = new QSpinBox(recordBox));
	(*recordingFps)->setRange(25, 100);
	(*recordingFps)->setValue(25);
	rowLayout->addStretch();

	rowLayout->addWidget(new QLabel(tr("Video dimensions: ")), 1, 0);
	rowLayout->addWidget(*spinWidth = new VideoResolutionSpinBox(recordBox));
	rowLayout->addWidget(new QLabel(" x ", 0, 0));
	rowLayout->addWidget(*spinHeight = new VideoResolutionSpinBox(recordBox));

	layout->addLayout(rowLayout);

	// render 2 vid or frameSeq
	rowLayout = new QHBoxLayout();
	rowLayout->addWidget(*saveAsVideo = new QPushButton(
			tr("Save as video-file")));
#ifndef VRN_WITH_FFMPEG
	(*saveAsVideo)->setVisible(false);
#endif
	rowLayout->addWidget(*saveAsFrameSeq = new QPushButton(tr(
			"Save as frame-sequence")));

	layout->addLayout(rowLayout);

	recordBox->setLayout(layout);
	return recordBox;
}

} // anonymous namespace

void AnimationPlugin::createWidgets() {
	QVBoxLayout* mainLayout = new QVBoxLayout();

	QGroupBox *playerBox = createAnimationPlayerBox(this,
			&startAnimation_, &pauseAnimation_,
			&stopAnimation_, &spinPlaybackFPS_, &checkLoop_);
	mainLayout->addWidget(playerBox);

	editFrames_ = createAnimationEditFramesBox(this,
			&selectKeyframe_, &frameMenu_, &selectFrameTime_,
			timeOffset_, &appendKeyframe_, &insertKeyframe_,
			&updateKeyframe_, &deleteKeyframe_,
			&clearAnimation_);
	editCircle_ = createAnimationEditCircleBox(this,
			&selectCircleAngle_, &selectCircleSpeed_);
	editBox_ = createTabbedEditBox(this, editFrames_,
			editCircle_);
    connect(editBox_, SIGNAL(currentChanged(int)), this, SLOT(setWidgetState()));
	mainLayout->addWidget(editBox_);

	QGroupBox *loadSaveBox = createAnimationLoadSaveBox(this,
			&loadAnimation_, &saveAnimation_);
	mainLayout->addWidget(loadSaveBox);

	QGroupBox *renderBox = createAnimationRenderBox(this,
			&saveAsFrameSequenceButton_, &saveAsVideoButton_,
			&spinRecordingFPS_, &spinWidth_, &spinHeight_);
	mainLayout->addWidget(renderBox);
	QWidget::setLayout(mainLayout);

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
	//connect(selectFrameTime_, SIGNAL(valueChanged(double)), this, SLOT(updateFrameTime(double)));
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

	// if already animating
	if (animationState_ != Stopped && animationState_ != Paused) {
		startAnimation_->setEnabled(false);
		return;
	}

	// navigation switch - JE: why?
	MsgDistr.postMessage(new BoolMsg("switch.trackballSpinning", false));

	// gather some useful params
	int fps;
	if (record) {
		fps = spinRecordingFPS_->value();
	} else {
		fps = spinPlaybackFPS_->value();
	}

	float curTime = 0.f;

	// check animation requirements and start it
	if (editBox_->currentIndex() == editBox_->indexOf(editFrames_)){
		if (currentAnimation_.size() < 2) {
			QMessageBox msg;
			msg.setText(tr("At least two frames are necessary for playback."));
			msg.exec();
			return;
		} 
        else {
			delete (animation_);
			animation_ = new tgt::BSplineAnimation(currentAnimation_, fps);
			currentAnimationFrame_ = 0;
			for (std::vector<KeyFrame>::iterator iter = currentAnimation_.begin(); iter
					!= currentAnimation_.end(); iter++) {
				curTime += (*iter).timeToNextFrame_;
			}
		}
	}
    else if (editBox_->currentIndex() == editBox_->indexOf(editCircle_)){
		if (selectCircleAngle_ == 0 || selectCircleSpeed_ == 0) {
			QMessageBox msg;
			msg.setText(tr("Zero angle or speed are not supported."));
			msg.exec();
			return;
		} 
        else {
			delete (animation_);
			animation_ = new tgt::CircleAnimation(selectCircleAngle_->value(),
														selectCircleSpeed_->value(),
														fps, trackBall_);
			currentAnimationFrame_ = 0;
			curTime=selectCircleSpeed_->value()*selectCircleAngle_->value()/360.0f;
		}
	}

	// set accurate animation-state (if not exited/returned out of here yet)
        // set accurate recording dimension
	if (record) {
	    MsgDistr.postMessage(new IVec2Msg("msg.resize", ivec2(
	            spinWidth_->value(),spinHeight_->value())));
		animationState_ = Recording;

#ifdef VRN_WITH_FFMPEG
		if (/*record &&*/renderingVideo_) {// enabled by button action
			VoreenPainter::getVideoEncoder()->startVideoEncoding(
					recordPathName_.c_str(), fps, spinWidth_->value(),
					spinHeight_->value());
		}
#endif /* VRN_WITH_FFMPEG */
	} else
		animationState_ = Running;

	// set accurate animation frames number
	numAnimationFrames_ = tgt::iround(curTime * fps);

	setWidgetState();

	/* JE: clarify if this is the cycle-time animationStep is called
	 FIXME encoding a video may exceed this magic constant,
	 which will cause nice racing conditions,
	 as ffmpeg runs its own bunch of threads */
	animationTimer_->start(1000 / fps, this);
}

void AnimationPlugin::stopAnimation() {

	if (animationState_ == Stopped){ // nothing to do
		stopAnimation_->setEnabled(false);
		return;
	}

	animationTimer_->stop();
	bool record = (animationState_ == Recording);
	animationState_ = Stopped;

	setWidgetState();

#ifdef VRN_WITH_FFMPEG
	if (record && renderingVideo_) {
		VoreenPainter::getVideoEncoder()->stopVideoEncoding();// this will finish encoded video file writing
		renderingVideo_ = false;
	}
#endif

	// reset Canvas Viewing
	MsgDistr.postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
	MsgDistr.postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_,
			camera_));
	if (record && canvas_) {
		MsgDistr.postMessage(new IVec2Msg("msg.resize", ivec2(
				canvas_->getWidth(), canvas_->getHeight())));
	}
	repaintCanvas();
}

void AnimationPlugin::pauseAnimation() {
	if (animationState_ != Running && animationState_ != Paused)
		return;

	animationState_ = Paused;

	setWidgetState();

	repaintCanvas();
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
		QString s = QFileDialog::getExistingDirectory(this, tr( "Choose a directory to save the image sequence"),
                                                      VoreenApplication::app()->getDocumentsPath().c_str());
		if (!s.isEmpty())
			fileList.push_back(s);
	}
#ifdef VRN_WITH_FFMPEG
	else {
		formats
				= VoreenPainter::getVideoEncoder()->getSupportedFormatsByFileEnding();
		std::stringstream formatsFilter;
		formatsFilter << "Video (";
		for (size_t i = 0; i < formats.size(); ++i) {
			formatsFilter << "*." << formats[i];
			if (i != formats.size() - 1)
				formatsFilter << " ";
		}
		formatsFilter << ")";

		QFileDialog dialog(this);
		dialog.setDefaultSuffix(tr("avi"));
		dialog.setWindowTitle(tr("Save as video file"));
		dialog.setDirectory(VoreenApplication::app()->getDocumentsPath().c_str());
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
#endif // VRN_WITH_FFMPEG
    if (fileList.size() > 0) {
        recordPathName_ = fileList[0].toStdString();

        startAnimation(true);
    }
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
			/*
			 * at this point we don't mind the exact implementation of AbstractAnimation
			 */
			animation_->moveToFrame(currentAnimationFrame_);
			vec3 position = animation_->getEye();
			vec3 focus = animation_->getCenter();
			vec3 up = animation_->getUp();

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
					MsgDistr.postMessage(new StringMsg("msg.paintToFile",
							std::string(fn)));
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

/**
 * JE: this is kinda strange work-around; will merge as much as possible to widget creation
 * i admit, that the enabling state of gui elements should be based to underlying logic (considering detachment of these)
 * TODO consider sending SIGNALS instead of everytime calling this
 */
void AnimationPlugin::setWidgetState() {
	editBox_->setEnabled(animationState_ == Stopped);

	bool animationEditable = ((currentAnimation_.size() > 0) && animationState_ == Stopped &&
            (editBox_->currentIndex() == editBox_->indexOf(editFrames_)));
	updateKeyframe_->setEnabled(animationEditable);
	deleteKeyframe_->setEnabled(animationEditable);
	clearAnimation_->setEnabled(animationEditable);
    saveAnimation_->setEnabled(animationEditable);
    
    loadAnimation_->setEnabled(editBox_->currentIndex() == editBox_->indexOf(editFrames_)
        && animationState_ == Stopped);

	selectFrameTime_->setEnabled(animationEditable);
	//playerBox_->setEnabled(currentAnimation_.size() >= 2);
	startAnimation_->setEnabled(
        (currentAnimation_.size() >= 2 || editBox_->currentIndex() == editBox_->indexOf(editCircle_)) && 
        (animationState_ == Stopped || animationState_ == Paused));
	pauseAnimation_->setEnabled(animationState_ == Running);
	stopAnimation_->setEnabled(animationState_ != Stopped);
	checkLoop_->setEnabled(animationState_ != Recording);
	spinPlaybackFPS_->setEnabled(animationState_ == Stopped);

	// Recording
#ifndef VRN_WITH_FFMPEG
	saveAsVideoButton_->setEnabled(false); // disable if no ffmpeg available
#endif
	//recordBox_->setEnabled(/*currentAnimation_.size() >= 2 &&*/ (animationState_ == Stopped));

	QString animationSize;
	animationSize.setNum(currentAnimation_.size());
	//editBox_->setTitle(tr("Editing") + "  (" + tr("Keyframes:") + " "+ animationSize + ")");

	if (currentAnimation_.empty()) {
		selectKeyframe_->setText(tr("No Animation"));
		selectKeyframe_->setEnabled(false);
	} else {
		selectKeyframe_->setText((QString(
				tr("Select Keyframe... (current: %1)"))).arg(currentKeyframe_
				+ 1));
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
		selectFrame(std::min(currentKeyframe_,
				static_cast<int> (currentAnimation_.size() - 1)));

	setWidgetState();
}

void AnimationPlugin::insertKeyframe() {

	tgtAssert(camera_, "No camera");tgtAssert(currentKeyframe_ >= 0 &&
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
	QString fn = QFileDialog::getOpenFileName(this,
			tr("Open voreen animation"), ".", tr("Voreen animations (*.vam)"));
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
