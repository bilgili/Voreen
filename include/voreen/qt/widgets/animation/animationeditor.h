/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef ANIMATIONEDITOR_H
#define ANIMATIONEDITOR_H

#include <QWidget>
#include <QString>

#include "voreen/qt/voreenqtapi.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/workspace.h"

class QLineEdit;
class QMenuBar;
class QMenu;
class QPushButton;
class QTimer;
class QLCDNumber;

namespace voreen {

class TimelineWidget;

/**
* Global Container for all Animation specific Widgets.
* This is the place of animation control and it is the
* global interface node to the animation core
*/
class VRN_QT_API AnimationEditor : public QWidget {
Q_OBJECT

public:
    AnimationEditor(NetworkEvaluator* eval, Workspace*, QWidget* = 0, Qt::WindowFlags = 0);
    void setWorkspace(Workspace*);

public slots:
    /// sets the current Frame
    void setCurrentFrame(int);
    static float getDuration();
    void recordAt(int);

protected slots:
    void newAnimation();
    void videoExport();
    void undo();
    void redo();
    void start();
    void rewind();
    void pause();
    void play();
    void stop();
    void forward();
    void end();
    void record();
    void setInteractionMode(bool);
    void settings();
    void playerControl(QAction*);
    void update();
    void setDuration(int);
    void timeStretchChanged(double);
    void populateAddTimelineMenu();
    void addTimeline(QAction* action);

protected:
    NetworkEvaluator* evaluator_;
    QMenuBar* mainMenu_;
    QMenu* addTimelineMenu_;
    TimelineWidget* timelineWidget_;
    float currentFrame_;
    /// frameSkip determining the fastforward and backward speed
    int frameSkip_;
    float timeStretch_;
    bool interactionMode_;
    Workspace* workspace_;

    Animation* animation_;
    QTimer* timer_;
    float currentTime_;
    static float duration_;        //in seconds
    /// String Representation for usage in LCD Displays
    QString getTimeString();
    void clearGui();

    void init();

signals:
    void currentFrameChanged(int);
    void recordSignal();
    void durationChanged(int);
    void newAnimation(Animation*);
    void autoPreview(bool);
};

} // namespace voreen

#endif

