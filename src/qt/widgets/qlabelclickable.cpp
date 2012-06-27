/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/qlabelclickable.h"

#include <QPalette>

namespace voreen {

QLabelClickable::QLabelClickable ( const char * text, QWidget * parent, Qt::WFlags f, 
        Qt::CursorShape hoverCursor) : 
        QLabel ( text, parent, f ),
        hoverCursor_(hoverCursor) {

    this->state = false;
    this->text_ = (std::string)text;

    QPalette palette = QPalette();
    palette.setColor(QPalette::Dark, QColor(140, 140, 140));
    setPalette(palette);

    setForegroundRole(QPalette::Dark);
    expanded_ = false;
}

void QLabelClickable::mousePressEvent ( QMouseEvent * e ) {
    this->state = true;
    emit pressed();
    e->ignore();
}

void QLabelClickable::mouseReleaseEvent ( QMouseEvent * e ) {
    this->state = false;
    emit released();
    emit clicked();
    e->ignore();
}

void QLabelClickable::mouseDoubleClickEvent ( QMouseEvent * e ) {
    this->state = false;
    emit doubleClicked();
    e->ignore();
}

void QLabelClickable::enterEvent(QEvent *e) {
    setCursor(hoverCursor_);
    setForegroundRole(QPalette::Text);
    e->ignore();
}

void QLabelClickable::leaveEvent(QEvent *e) {
    unsetCursor();
    if (!expanded_)
        setForegroundRole(QPalette::Dark);
    else
        setForegroundRole(QPalette::Text);
    e->ignore();
}

void QLabelClickable::toggleExpanded() {
    expanded_ = !expanded_;
    if (expanded_)
        setForegroundRole(QPalette::Text);
    else
        setForegroundRole(QPalette::Dark);
}

} // namespace voreen
