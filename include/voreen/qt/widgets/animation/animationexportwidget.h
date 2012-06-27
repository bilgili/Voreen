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

#ifndef VRN_ANIMATIONEXPORTWIDGET_H
#define VRN_ANIMATIONEXPORTWIDGET_H

#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/utils/voreenpainter.h"
#ifdef VRN_WITH_FFMPEG
    #include "tgt/ffmpeg/videoencoder.h"
#endif
#include "tgt/qt/qtcanvas.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QDialog>
#include <map>

namespace voreen {

class NetworkEvaluator;

/**
 * This Class creates a widget for Animation to Video or Screencapture
 */
class AnimationExportWidget : public QDialog {
Q_OBJECT
public:
    AnimationExportWidget(QWidget* parent, Animation* animation, NetworkEvaluator* network, float duration, int startframe, int endframe, float);
    ~AnimationExportWidget();
    void networkChanged();

private slots:
    void controlledCanvasChanged(int index);
    void videoSetup();
    void recordAnimationFrameSeq();
    void recordAnimationVideo();
    void setWidgetState();

private:
    void closeEvent(QCloseEvent* );
    void recordAnimation(bool recordVideo);
    void startRendering();
    void endRendering();
    void renderingStep();
    void createWidgets();
    void createConnections();
    void refreshComboBoxes();

    Animation* animation_;
    NetworkEvaluator* network_;
    float duration_;
    int startframe_;
    int endframe_;

    tgt::QtCanvas* canvas_;
    VoreenPainter* painter_;
    tgt::ivec2 canvasSize_;
    bool renderingVideo_;
    std::string recordPathName_;
    float fpsFactor_;
    int currentFrame_;
    int fps_;
    enum RenderState { Snapshot, Recording, Inactive };
    RenderState renderState_;
    typedef std::map<tgt::QtCanvas*, std::string> CanvasMap;

    QGroupBox* createAnimationRenderBox(QWidget* parent);

#ifdef VRN_WITH_FFMPEG
    tgt::VideoEncoder ffmpegEncoder_;
    QComboBox* preset_;
    QSpinBox* bitrate_;
    QDialog* createVideoSetupDialog(QWidget* parent, int curPreset, int curBitrate);
#endif

    CanvasMap allCanvases_;
    QGroupBox* renderBox_;
    QSpinBox* spinRecordingFPS_;
    QSpinBox* spinWidth_;
    QSpinBox* spinHeight_;
    QTimeEdit* spinStartTime_;
    QTimeEdit* spinEndTime_;
    QComboBox* comboCanvases_;
    QComboBox* comboCameras_;
    QVBoxLayout* mainLayout_;
    QPushButton* saveAsFrameSequenceButton_;
    QPushButton* saveAsVideoButton_; // will be disabled in case ffmpeg is not available
    QPushButton* videoSetupButton_;
};
}
#endif // VRN_ANIMATIONEXPORTWIDGET_H
