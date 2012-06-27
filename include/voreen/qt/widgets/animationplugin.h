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
#include <vector>
#include <string>

#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QBasicTimer>
#include <QFileDialog>
#include <QToolBox>
#include <QMenu>
#include <QCheckBox>
#include <QGroupBox>

#include "tgt/vector.h"
#include "tgt/spline.h"
#include "tgt/camera.h"
#include "tgt/qt/qtcanvas.h"

namespace voreen {

class AnimationPlugin : public WidgetPlugin {
    Q_OBJECT
public:
    struct QExtAction : public QAction {
        QExtAction(const QString& text, QObject* parent, int num)
            : QAction(text, parent), num_(num) {}

        int num_;
    };

    AnimationPlugin(QWidget* parent, tgt::Camera* camera, tgt::QtCanvas* canvas = 0);

    virtual ~AnimationPlugin();

    virtual void createWidgets();
    virtual void createConnections();

private:
    struct KeyFrame {
        tgt::vec3 position_;
        tgt::vec3 focus_;
        tgt::vec3 up_;
        double timeToNextFrame_;
        QPixmap pic_;
    };

    enum AnimationState {
        Stopped,
        Running,
        Paused,
        Recording
    };

    virtual void timerEvent(QTimerEvent *event);

    bool isRenderingVideo() const;

    void refreshFrameMenu();
    void gotoFrame(int num);
    void setWidgetState();

private slots:
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

    tgt::Camera* camera_;
    tgt::QtCanvas* canvas_;

    std::vector<KeyFrame> currentAnimation_;
    int currentKeyframe_;

    tgt::BSpline* camPositionSpline_;
    tgt::BSpline* camFocusSpline_;
    tgt::BSpline* camUpSpline_;

    AnimationState animationState_;
    int numAnimationFrames_;
    int currentAnimationFrame_;

    float timeOffset_;
    std::string recordPathName_;

    bool renderingVideo_;

    QPushButton* saveAsVideoButton_; // will be disabled in case ffmpeg is not available
    QPushButton* saveAsFrameSequenceButton_;

    QGroupBox* editBox_;
    QGroupBox* playerBox_;
    QGroupBox* recordBox_;
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
    QSpinBox *spinWidth_;
    QSpinBox *spinHeight_;
    QCheckBox *checkLoop_;
    QBasicTimer *animationTimer_;

    QSpinBox *spinPlaybackFPS_;
    QSpinBox *spinRecordingFPS_;

    QDoubleSpinBox *selectFrameTime_;
    QMenu *frameMenu_;

};

} // namespace

#endif // ANIMATIONPLUGIN_H
