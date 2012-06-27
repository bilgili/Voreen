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

#ifndef VRN_LINKDIALOGGRAPHICSVIEW_H
#define VRN_LINKDIALOGGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMenu>

namespace voreen {

/**
 * This QGraphicsView is extended by methods for keyboard events
 * and context events.  The <code>Delete</code> or <code>Backspace</code>
 * key will trigger the <code>deleteActionTriggered(QGraphicsItem*)</code>
 * signal.
 */
class LinkDialogGraphicsView : public QGraphicsView {
Q_OBJECT
public:
    /**
     * Constructor which sets certain default values:</br>
     * <ul>
     * <li>Minimum Size: 640, 480</li>
     * <li>Maximum Height: 480</li>
     * <li>Mouse Tracking: true</li>
     * <li>Background Brush: Qt::darkGray</li>
     * <li>Creates a context menu with a single <code>Delete</code> action</li>
     * </ul>
     * \param parent The parent widget. Will be passed to the QGraphicsView ctor
     */
    LinkDialogGraphicsView(QWidget* parent = 0);

signals:
    /**
     * This signal will be triggered if either <code>Delete</code> or
     * <code>Backspace</code> is pressed while this instance has the
     * keyboard focus
     * \param arrow The arrow which is selected at the moment. Might be 0
     */
    void deleteActionTriggered(QGraphicsItem* arrow);

private slots:
    void deleteAction();

protected:
    void keyPressEvent(QKeyEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

    QGraphicsItem* selectedArrow_;
    QMenu contextMenu_;
};

} // namespace

#endif // VRN_LINKDIALOGGRAPHICSVIEW_H
