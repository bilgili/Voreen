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

#ifndef VRN_LINKDIALOGPROCESSORGRAPHICSITEM_H
#define VRN_LINKDIALOGPROCESSORGRAPHICSITEM_H

#include <QGraphicsItem>

#include "textgraphicsitem.h"
#include "networkeditor_common.h"

namespace voreen {

/**
 * This shallow class represents a Processor in a \sa PropertyDialog
 * but has no functionality of its own. It simply draws an item
 * visually simpilar to a \sa ProcessorGraphicsItem
 */
class LinkDialogProcessorGraphicsItem : public QGraphicsItem {
public:
    /// Simple constructor
    LinkDialogProcessorGraphicsItem(const QString& name);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesLinkDialogProcessorGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    // Constructs a QRectF for round boxes.
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
     TextGraphicsItem textItem_;
};

} // namespace

#endif // VRN_LINKDIALOGPROCESSORGRAPHICSITEM_H
