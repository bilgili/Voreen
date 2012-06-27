/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef PROCESSORTIMELINEWIDGET_H
#define PROCESSORTIMELINEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <map>

#include "voreen/core/animation/animation.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"

class AnimatedProcessor;
class QComboBox;
class QPushButton;
class QMenu;
class QVBoxLayout;

namespace voreen {

class ProcessorTimelineWidget : public QWidget {
    Q_OBJECT
public:
    ProcessorTimelineWidget(std::string, AnimatedProcessor*, int, QWidget* = 0);

    const AnimatedProcessor* getAnimatedProcessor() const;
    int getTimelineCount();

public slots:
    /// for setting the fixed width. Sadly there ssems to be no easy way to do the correct resizing within qscrollareas
    void setFixedWidthSlot(int);

protected:
    /// contains all propertytimelines for visiblility switching
    QWidget* timelineWidget_;
    /// contains propertywidgets for visiblility switching
    QWidget* propertyContainer_;
    /// the coresponding animated processor
    AnimatedProcessor* animatedProcessor_;
    QVBoxLayout* timelineLayout_;
    /// contains which propertyTimelines are instantiated
    std::map<PropertyTimeline*, bool> propertyTimelines_;
    QPushButton* hidePropertyTimelines_;
    /// shows all not yet instantiated properties
    QMenu* availablePropertiesMenu_;
    QPushButton* availablePropertiesButton_;
    bool hiddenTimelines_;
    int currentFrame_;

    /// initializes all PropertyTimelines of the Processor
    void initPropertyTimelines();
    /// populates the PropertyMenu
    void populatePropertyMenu();
    void createPropertyTimelineWidget(PropertyTimeline* tl);
    QSize sizeHint();
    int timelineCount_;

protected slots:
    /// hides all timelines of a processor
    void hideTimelines();
    /// show the propertytimeline corresponding to the given QString
    void showPropertyTimeline(QString);
    /// shows all Available Properties
    void showAvailableProperties();
    /// show all AnimatedPropertyTimelines
    void showAnimatedPropertyTimelines();

    void currentFrameChangedSlot(int);

signals:
    void zoomOrder(int);
    void zoomRequest(int);
    void sceneRequest(QMatrix);
    void sceneOrder(QMatrix);
    void scrollBarRequest(int);
    void scrollBarOrder(int);
    void currentFrameChanged(int);
    void renderAt(float);
    void durationChanged(int);
    void fpsChanged(int);
    void viewResizeSignal(int);
    void keyframeAdded();
    void barMovement(int);
    void viewFrameChange(int);
};

} // namespace voreen

#endif

