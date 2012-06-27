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

#ifndef ANIMATIONPLUGIN_H
#define ANIMATIONPLUGIN_H

#include "widgetplugin.h"

#include "tgt/navigation/trackball.h"

#include <vector>
#include <string>

#include <QAction>
#include <QSpinBox>

// forward declarations
class QTabWidget;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QBasicTimer;
class QFileDialog;
class QToolBox;
class QMenu;
class QCheckBox;
class QGroupBox;
class QString;

namespace tgt{
	class AbstractAnimation;
	struct KeyFrame;
	class Camera;
	class QtCanvas;
}

namespace voreen {


class VideoResolutionSpinBox : public QSpinBox {
public:
    VideoResolutionSpinBox(QWidget * parent = 0);
    ~VideoResolutionSpinBox();

};

class AnimationPlugin : public WidgetPlugin {
    Q_OBJECT
public:
    struct QExtAction : public QAction {
        QExtAction(const QString& text, QObject* parent, int num)
            : QAction(text, parent), num_(num) {}

        int num_;
    };

    AnimationPlugin(QWidget* parent, tgt::Camera* camera, tgt::QtCanvas* canvas = 0, tgt::Trackball* trackBall = 0);

    virtual ~AnimationPlugin();

    virtual void createWidgets();
    virtual void createConnections();

private slots:
    void setWidgetState();

    void appendKeyframe();
    void updateKeyframe();
    void startAnimation(bool record = false);
    void stopAnimation();
    void pauseAnimation();
    void clearAnimation();
    void recordAnimationVideo();
    void recordAnimationFrameSeq();
    void saveAnimation();
    void loadAnimation();
    void animationStep();
    void selectFrame(int frameID, bool jumpToFrame = true);
    void selectFrame(QAction* action);
    void deleteKeyframe();
    void insertKeyframe();
    void updateFrameTime(double val);

private:
    void recordAnimation(bool videoRecord = false);

    virtual void timerEvent(QTimerEvent *event);

    bool isRenderingVideo() const;

    void refreshFrameMenu();
    void gotoFrame(int num);

    enum AnimationState {
        Stopped,
        Running,
        Paused,
        Recording
    } animationState_;

    tgt::Trackball* trackBall_;
	tgt::Camera* camera_;
	tgt::QtCanvas* canvas_;

    std::vector<tgt::KeyFrame> currentAnimation_;
    int currentKeyframe_;

    /*
     * in general animation-plugin's widget consists of four parts, playback-control, animation-edit, animation-load/save and rendering
     */
    QTabWidget* editBox_;
    /*QGroupBox* playerBox_;
    QGroupBox* recordBox_;
    QGroupBox* renderBox_;*/// not of interest after init and signal-emiters are exported

    /**
     * <emph>animation-edit</emph> will be switched depending the chosen animType given by current animTabBar estate
     */
    QGroupBox *editCircle_;
    QSpinBox *selectCircleAngle_;
    QSpinBox *selectCircleSpeed_;

    tgt::AbstractAnimation *animation_;

    /*
     * numAnimationFrames_/fps is playback speed-indicator
     */
    int numAnimationFrames_;
    int currentAnimationFrame_;

    QSpinBox *spinPlaybackFPS_;
    QSpinBox *spinRecordingFPS_;

    float timeOffset_;// TODO check if droppable
    std::string recordPathName_;

    bool renderingVideo_;

    QPushButton* saveAsVideoButton_; // will be disabled in case ffmpeg is not available
    QPushButton* saveAsFrameSequenceButton_;

    QGroupBox *editFrames_;

    QPushButton *selectKeyframe_;
    QPushButton *appendKeyframe_;
    QPushButton *insertKeyframe_;
    QPushButton *updateKeyframe_;
    QPushButton *deleteKeyframe_;
    QPushButton *clearAnimation_;

    QPushButton *startAnimation_;
    QPushButton *stopAnimation_;
    QPushButton *pauseAnimation_;
    QPushButton *saveAnimation_;
    QPushButton *loadAnimation_;

    VideoResolutionSpinBox *spinWidth_;
    VideoResolutionSpinBox *spinHeight_;
    QCheckBox *checkLoop_;
    QBasicTimer *animationTimer_;

    QDoubleSpinBox *selectFrameTime_;
    QMenu *frameMenu_;

};

} // namespace

#endif // ANIMATIONPLUGIN_H
