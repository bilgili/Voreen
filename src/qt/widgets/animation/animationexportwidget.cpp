/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/animation/animationexportwidget.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/processors/canvasrenderer.h"
#include "voreen/core/network/networkevaluator.h"

#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QTimeEdit>

namespace voreen {

AnimationExportWidget::AnimationExportWidget(QWidget* parent, Animation* animation, NetworkEvaluator* network, float duration, int startframe, int endframe, float stretchFactor)
    : QDialog(parent)
    , animation_(animation)
    , network_(network)
    , duration_(duration)
    , startframe_(startframe)
    , endframe_(endframe)
    , canvas_(0)
    , painter_(0)
    , canvasSize_(256, 256)
    , renderingVideo_(false)
    , recordPathName_("")
    , fpsFactor_(stretchFactor)
    , renderState_(Inactive)
    , renderTimer_(new QBasicTimer())
{
    setWindowTitle(tr("Animation Export"));
    setObjectName(tr("Export Optionen"));
    createWidgets();
    createConnections();
}

AnimationExportWidget::~AnimationExportWidget(){
    comboCanvases_->disconnect();
    renderBox_->disconnect();
    delete renderTimer_;
}
void AnimationExportWidget::networkChanged() {
    if (network_ == 0)
        return;

    canvas_ = 0;
    painter_ = 0;
    allCanvases_.clear();


    const std::vector<Processor*>& processors = network_->getProcessorNetwork()->getProcessors();;
    for (size_t i = 0; i < processors.size(); ++i) {
        CanvasRenderer* cr = dynamic_cast<CanvasRenderer*>(processors[i]);
        if (cr != 0) {
            tgt::QtCanvas* canvas = dynamic_cast<tgt::QtCanvas*>(cr->getCanvas());
            if (canvas != 0)
                allCanvases_.insert(std::make_pair(canvas, cr->getName()));
        }
    }


    if (allCanvases_.empty() == false) {
        canvas_ = allCanvases_.begin()->first;
        if (canvas_ != 0)
            painter_ = dynamic_cast<VoreenPainter*>(canvas_->getPainter());
    }

    refreshComboBoxes();

}


void AnimationExportWidget::controlledCanvasChanged(int index) {
    if ((comboCanvases_ == 0) || (index < 0) || (index >= comboCanvases_->count()))
        return;
    canvas_ = reinterpret_cast<tgt::QtCanvas*>(comboCanvases_->itemData(index).toULongLong());
    if (canvas_ != 0)
        painter_ = dynamic_cast<VoreenPainter*>(canvas_->getPainter());

}

void AnimationExportWidget::createWidgets(){

    mainLayout_ = new QVBoxLayout();
    renderBox_ = createAnimationRenderBox(this);
    mainLayout_->addWidget(renderBox_);

    setLayout(mainLayout_);
    setWidgetState();
}

void AnimationExportWidget::createConnections() {
    connect(comboCanvases_, SIGNAL(currentIndexChanged(int)), this, SLOT(controlledCanvasChanged(int)));
    connect(saveAsFrameSequenceButton_, SIGNAL(clicked()), this, SLOT(recordAnimationFrameSeq()));
    connect(saveAsVideoButton_, SIGNAL(clicked()), this, SLOT(recordAnimationVideo()));
    connect(videoSetupButton_, SIGNAL(clicked()), this, SLOT(videoSetup()));
}
void AnimationExportWidget::setWidgetState() {
    comboCanvases_->setEnabled(true);

}
void AnimationExportWidget::refreshComboBoxes() {
    if (comboCanvases_ != 0) {
        comboCanvases_->clear();
        for (CanvasMap::const_iterator it = allCanvases_.begin(); it != allCanvases_.end(); ++it)
            comboCanvases_->addItem(tr(it->second.c_str()), reinterpret_cast<qulonglong>(it->first));
    }

}
QGroupBox* AnimationExportWidget::createAnimationRenderBox(QWidget* parent) {
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
    spinRecordingFPS_->setRange(1, 120);
    spinRecordingFPS_->setValue(30);
    rowLayout->addWidget(spinRecordingFPS_);
    rowLayout->addStretch();

    rowLayout->addWidget(new QLabel(("     Video dimensions")), 1, 0);
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

    QTime temp(0, 0, 0, 0);
    QTime duration = temp.addSecs((int)duration_/30);
    QHBoxLayout* row2Layout = new QHBoxLayout();
    spinStartTime_ = new QTimeEdit(QTime(0, 0 , 0, 0));
    spinEndTime_ = new QTimeEdit(duration);
    row2Layout->addWidget(new QLabel("Start Time: "));
    row2Layout->addWidget(spinStartTime_);

    spinStartTime_->setMaximumTime(duration);
    spinEndTime_->setMaximumTime(duration);

    row2Layout->addWidget(new QLabel("End Time: "));
    row2Layout->addWidget(spinEndTime_);
    layout->addLayout(row2Layout);

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
void AnimationExportWidget::recordAnimationFrameSeq() {
    recordAnimation(false);
}

void AnimationExportWidget::recordAnimationVideo() {
    recordAnimation(true);
}
//collecting directories
void AnimationExportWidget::recordAnimation(bool recordVideo) {
    renderingVideo_ = recordVideo;
    QStringList fileList;
    if (renderingVideo_ == false) {
        renderState_= Snapshot;
        QString s = QFileDialog::getExistingDirectory(this,tr( "Choose a directory to save the image sequence"),
            VoreenApplication::app()->getDocumentsPath().c_str());

        if (!s.isEmpty())
            fileList.push_back(s);
    }
#ifdef VRN_WITH_FFMPEG
    else {
        renderState_= Recording;
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
        startRendering();
    }
}
//initialization
void AnimationExportWidget::startRendering() {

    // gather some useful params
    fps_ = spinRecordingFPS_->value();
    startframe_= fps_ * (spinStartTime_->time().hour() * 3600 + spinStartTime_->time().minute()*60+ spinStartTime_->time().second());
    endframe_ = fps_ * (spinEndTime_->time().hour() * 3600 + spinEndTime_->time().minute()*60+ spinEndTime_->time().second());
    currentFrame_ = startframe_;
    duration_= endframe_;
    //fpsfactor_ = fps_/30.0f;
    //fpsfactor_ = 0.5;

    // set accurate recording dimension
    canvasSize_ = canvas_->getSize();
    //canvas_->resize(spinWidth_->value(), spinHeight_->value());
    painter_->getCanvasRenderer()->resizeCanvas(tgt::ivec2(spinWidth_->value(), spinHeight_->value()));
    qApp->processEvents();

#ifdef VRN_WITH_FFMPEG
    if (renderState_== Recording) {
        tgt::Texture* texture_ = painter_->getCanvasRenderer()->getImageColorTexture();
        ffmpegEncoder_.startVideoEncoding(recordPathName_.c_str(),
        fps_, spinWidth_->value(), spinHeight_->value(),texture_->getFormat(), texture_->getDataType());

    }
#endif // VRN_WITH_FFMPEG

    setWidgetState();

    if (fps_ > 0)
        renderTimer_->start(1000 / fps_, this);
}
void AnimationExportWidget::timerEvent(QTimerEvent*) {
    renderingStep();
}
//renderingprocess
void AnimationExportWidget::renderingStep(){

    if (animation_ == 0)
        return;

    tgtAssert(canvas_, "No canvas");

    if (currentFrame_ > duration_ / fpsFactor_){
        endRendering();
    }
    else {
        animation_->renderAt((float)currentFrame_*fpsFactor_/ fps_);

        #ifdef VRN_WITH_FFMPEG
        if ((renderState_== Recording) &&(painter_->getCanvasRenderer())) {
            if (canvas_->getSize() != tgt::ivec2(spinWidth_->value(), spinHeight_->value()))
                canvas_->resize(spinWidth_->value(), spinHeight_->value());
            canvas_->repaint();
            tgt::Texture* texture = painter_->getCanvasRenderer()->getImageColorTexture();
            if (texture && texture->getDimensions().xy() == tgt::ivec2(spinWidth_->value(), spinHeight_->value())) {
                texture->downloadTexture();
                ffmpegEncoder_.nextFrame(texture->getPixelData());
            }
            else {
                LERRORC("voreenqt.AnimationExportWidget", "Frame texture could not be downloaded or dimensions do not match");
            }
        } else {
        #endif
            // render frame to file
            char fn[1024];
            sprintf(fn, "%s%05d%s", std::string(recordPathName_ + "/frame").c_str(), currentFrame_, ".png");
            try {
                painter_->renderToSnapshot(fn, tgt::ivec2(spinWidth_->value(), spinHeight_->value()));
            } catch (...) {}
        #ifdef VRN_WITH_FFMPEG
        }
        #endif
        ++currentFrame_;
    }
}
void AnimationExportWidget::endRendering(){
    renderTimer_->stop();
    renderState_= Inactive;
    #ifdef VRN_WITH_FFMPEG
    if(renderingVideo_){
        ffmpegEncoder_.stopVideoEncoding();
        renderingVideo_ = false;
    }
    #endif
    canvas_->resize(canvasSize_.x,canvasSize_.y);
}
void AnimationExportWidget::videoSetup() {
    #ifdef VRN_WITH_FFMPEG
    int curPreset = ffmpegEncoder_.getPreset();
    int curBitrate = ffmpegEncoder_.getBitrate();
    QDialog* dialog = createVideoSetupDialog(this, curPreset, curBitrate);
    if (dialog->exec() == QDialog::Accepted)
        ffmpegEncoder_.setup(preset_->currentIndex(), bitrate_->value() * 1024);

    delete dialog;
    #endif
}
#ifdef VRN_WITH_FFMPEG
QDialog* AnimationExportWidget::createVideoSetupDialog(QWidget* parent, int curPreset, int curBitrate) {
    QDialog* dialog = new QDialog(parent);
    QVBoxLayout* layout = new QVBoxLayout();
    dialog->setLayout(layout);

    // Preset
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
