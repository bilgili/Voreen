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

#ifndef VRN_TEXTGRAPHICSITEM_H
#define VRN_TEXTGRAPHICSITEM_H

#include <QGraphicsSimpleTextItem>
#include "networkeditor_common.h"

namespace voreen {

/**
 * This item behaves like a QGraphicsTextItem, but it offers to additional signals
 * and responds to the Return and ESC keys as it should (i.e. Return saves the new
 * text, ESC restores the last valid text)
 */
class TextGraphicsItem : public QGraphicsTextItem  {
Q_OBJECT
public:
    /**
     * Constructor for an instance. Will make this item selectable, sets the text color
     * to bright white and the default font to "Helvetica"
     * \param text The text which should be displayed
     * \param parent The parent QGraphicsItem which has ownership over this item
     */
    TextGraphicsItem(const QString& text, QGraphicsItem* parent = 0);

    /// The type of this QGraphicsItem subclass
    enum { Type = UserType + UserTypesTextGraphicsItem };

    /**
     * Returns the type of this class. Necessary for all QGraphicsItem subclasses.
     * \return The type of this class
     */
    int type() const;

    /// \sa QGraphicsTextItem::setFocus(Qt::FocusReason)
    void setFocus(Qt::FocusReason focusReason = Qt::OtherFocusReason);

    /// \sa QGraphicsTextItem::setPlainText(const QString&)
    void setPlainText(const QString& text);

signals:
    /**
     * This signal is emitted if the renaming processor is finished, whether it was
     * because of a ESC-based abort or a Return-based finish
     */
    void renameFinished();

    /// This signal is emitted each time a character is changed
    void textChanged();

protected:
    /// \sa QGraphicsTextItem::keyPressEvent(QKeyEvent*)
    void keyPressEvent(QKeyEvent* event);
    void focusOutEvent(QFocusEvent* event);

private:
    /// Stores the old text, if the renaming process is aborted
    QString previousText_;
};

} // namespace

#endif // VRN_TEXTGRAPHICSITEM_H
