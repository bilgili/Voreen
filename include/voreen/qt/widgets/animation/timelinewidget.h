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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "voreen/core/animation/animation.h"
#include "voreen/core/animation/animationobserver.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/qt/widgets/animation/animationeditor.h"
#include "voreen/qt/widgets/animation/processortimelinewidget.h"

#include <QWidget>

class AnimatedProcessor;
class OverviewWidget;
class QGroupBox;
class QVBoxLayout;


namespace voreen {

class TimelineWidget : public QWidget, public AnimationObserver {
Q_OBJECT
public:
    TimelineWidget(Animation*, AnimationEditor*, NetworkEvaluator*);
    float getCurrentTime();
    void setCurrentTime(float /*time*/);

    /**
    * this function is called if there was an undo or a redo
    * all current ProcessorsTimelineWidgets and Propertytimelinewidgets are not valid anymore and have to be rebuild
    * this function is not called after creation
    */
    void reloadAnimation();

    /**
    * this function is called if a new animatedProcessors is added
    */
    void animatedProcessorAdded(const AnimatedProcessor* processor);

    /**
    * this function is called if an animatedProcessors gets deleted
    */
    void animatedProcessorRemoved(const AnimatedProcessor* processor);

    Animation* getAnimation() { return animation_; }

public slots:
    /// setWorkspace
    void rebuildAnimation(Animation*);
    /// performed when the record signal is triggered to check for changes. If none are made pops up an infodialog
    void checkForChanges();
    void removeProcessorTimelineWidget(ProcessorTimelineWidget* ptlw);

protected:
    /// takes all present timelines from the core and initializes templatepropertytimelinewidgets for them
    void populateProcessors();
    /// the Animation core, usually obtained from the workspace if present
    Animation* animation_;
    /// the current time
    float currentTime_;
    /// Mainlayout
    QVBoxLayout* mainLayout_;
    /// all processorTimelineWidgets
    std::vector<ProcessorTimelineWidget*> processorTimelineWidgets_;
    /// returns a String representation for LCD Displays of the current time
    QString getTimeString(int);

    /// LCDTimeCounter for visual feedback
    QLabel* timeCounter_;
    /// QWidget reimplementation of the QWidget resizeevent
    void resizeEvent(QResizeEvent*);
    /// Scrollarea which encapsulates all Processortimelines
    QScrollArea* scrollArea_;
    /// layout for the Scrollarea
    QVBoxLayout* scrollAreaLayout_;
    QWidget* containerWidget_;
    OverviewWidget* overviewTimeline_;
    QGroupBox* timeBox_;
    // indicates wether there was a change once
    bool changed_;

protected slots:
    /// renders the current Animation at the given time
    void renderAt(float);
    /// sets the currentFrame
    void currentFrame(int);

signals:
    /// orders a zoom to all timelines with zommfactor of the given integer
    void zoomOrder(int);

    void showActiveTimelines();
    /// orders a scene translation via the given matrix
    void sceneOrder(QMatrix);
    /// order to distribute vertical scrollbarpositions
    void scrollBarOrder(int);
    /// signals a framechange
    void currentFrameChanged(int);
    /// used to record the network state which is taken from the voreenve app
    void recordSignal();
    /**
    * signals a resizeEvent. Mainly used to work around some Layout bugs related to QScrollAreas
    * two days of ircing and forum writing couldn't solve this any other way.
    */
    void resizeSignal(int);
    /// The duration of the Animation has changed
    void durationChanged(int);
    /// The fps of the Animation has changed
    void fpsChanged(int);
    /**
    * signals a resizeEvent which comes from the propertytimelineview. This is used to determine the
    * geometry of the Viewportvisualisation in the OverviewWidget
    */
    void viewResizeSignal(int);

    void recordAt(int);

    void setAnimationEditorDuration(int);

    void autoPreview(bool);
    void updatePreviews();
};

} // namespace voreen

#endif
