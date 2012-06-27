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

#ifndef VRN_ANIMATIONPLUGIN_H
#define VRN_ANIMATIONPLUGIN_H

#include "voreen/qt/widgets/widgetplugin.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/network/processornetwork.h"
#include "voreen/core/vis/properties/cameraproperty.h"

#include "tgt/animation/abstractanimation.h"
#ifdef VRN_WITH_FFMPEG
    #include "tgt/ffmpeg/videoencoder.h"
#endif
#include "tgt/qt/qtcanvas.h"

using tgt::KeyFrame;

#include <QBasicTimer>
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
#include <QVBoxLayout>

#include <map>

namespace voreen {

class VoreenTrackball;

class AnimationPlugin : public WidgetPlugin, public ProcessorNetworkObserver {
    Q_OBJECT
public:
    AnimationPlugin(QWidget* parent);

    virtual ~AnimationPlugin();

    /// Implementation of ProcessorNetworkObserver interface
    virtual void networkChanged();

    /// Called by VoreenVisualization
    void setNetwork(ProcessorNetwork* processorNetwork);

private slots:
    void controlledCameraChanged(int index);
    void controlledCanvasChanged(int index);

    void clearAnimation();
    void loadAnimation();
    void pauseAnimation();
    void recordAnimationVideo();
    void recordAnimationFrameSeq();
    void saveAnimation();
    void startAnimation(const bool record = false);
    void stopAnimation();

    void selectFrame(QAction* action);
    void selectFrame(int frameID, bool jumpToFrame = true);

    void appendKeyframe();
    void deleteKeyframe();
    void insertKeyframe();
    void updateKeyframe();

    void setWidgetState();
    void tabChanged(int);
    void videoSetup();

private:
    enum AnimationState { Stopped, Running, Paused, Recording };

    struct KeyFrameQt : KeyFrame {
        QPixmap pic_;
    };

    struct QExtAction : public QAction {
        QExtAction(const QString& text, QObject* parent, int num)
            : QAction(text, parent), num_(num) {}

        int num_;
    };

    typedef std::map<tgt::QtCanvas*, std::string> CanvasMap;
    typedef std::map<CameraProperty*, std::string> CameraMap;

private:
    QWidget* createAnimationFramesTab();
    QWidget* createAnimationCircleTab();
    QGroupBox* createAnimationPlayerBox(QWidget* parent);
    QGroupBox* createAnimationRenderBox(QWidget* parent);

    void animationStep();
    void createConnections();
    void createWidgets();
    void gotoFrame(int frameID);
    std::vector<KeyFrame> keyFrameVec(const std::vector<AnimationPlugin::KeyFrameQt>& vec);
    void recordAnimation(bool recordVideo);
    void refreshComboBoxes();
    void refreshFrameMenu();
    virtual void timerEvent(QTimerEvent* event);
    void updateReadyState();

#ifdef VRN_WITH_FFMPEG
    tgt::VideoEncoder ffmpegEncoder_;

    QComboBox* preset_;
    QSpinBox* bitrate_;

    QDialog* createVideoSetupDialog(QWidget* parent, int curPreset, int curBitrate);
#endif

private:
    ProcessorNetwork* processorNetwork_;
    CameraProperty* camera_;
    tgt::QtCanvas* canvas_;
    VoreenPainter* painter_;
    tgt::ivec2 canvasSize_;

    bool readyState_;
    bool renderingVideo_;
    AnimationState animationState_;
    tgt::AbstractAnimation* animation_;
    std::vector<KeyFrameQt> currentAnimation_;
    int currentAnimationFrame_;
    int currentKeyframe_;
    int numAnimationFrames_;
    float timeOffset_;
    std::string recordPathName_;

    CanvasMap allCanvases_; /** All canvas from all CanvasRenderers in the current network */
    CameraMap allCameras_;  /** All cameras from all Processors in the current network */

    // Qt stuff
    //
    QBasicTimer* animationTimer_;

    QComboBox* comboCanvases_;
    QComboBox* comboCameras_;

    QWidget* tabFrames_;
    QWidget* tabCircle_;
    QGroupBox* loadSaveBox_;
    QGroupBox* playerBox_;
    QGroupBox* renderBox_;
    QTabWidget* editBox_;

    QVBoxLayout* mainLayout_;
    QPushButton* saveAsFrameSequenceButton_;
    QPushButton* saveAsVideoButton_; // will be disabled in case ffmpeg is not available
    QPushButton* videoSetupButton_;

    QPushButton* clearAnimation_;
    QPushButton* loadAnimation_;
    QPushButton* pauseAnimation_;
    QPushButton* saveAnimation_;
    QPushButton* startAnimation_;
    QPushButton* stopAnimation_;

    QPushButton* appendKeyframe_;
    QPushButton* deleteKeyframe_;
    QPushButton* insertKeyframe_;
    QPushButton* selectKeyframe_;
    QPushButton* updateKeyframe_;

    QMenu* frameMenu_;
    QSpinBox* selectCircleAngle_;
    QSpinBox* selectCircleSpeed_;
    QSpinBox* spinPlaybackFPS_;
    QSpinBox* spinRecordingFPS_;
    QSpinBox* spinWidth_;
    QSpinBox* spinHeight_;
    QCheckBox* checkLoop_;
    QDoubleSpinBox* selectFrameTime_;
};

}   // namespace voreen

#endif
