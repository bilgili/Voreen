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

#ifndef QLABELCLICKABLE_H
#define QLABELCLICKABLE_H

#include <QLabel>
#include <QMouseEvent>
#include <string>


namespace voreen {

class QLabelClickable : public QLabel {
    Q_OBJECT

public:
    QLabelClickable ( const char * text, QWidget * parent, Qt::WFlags f = 0, 
        Qt::CursorShape hoverCursor = Qt::PointingHandCursor);
    virtual ~QLabelClickable() { }

    virtual void mousePressEvent ( QMouseEvent * e );
    virtual void mouseReleaseEvent ( QMouseEvent * e );
    virtual void mouseDoubleClickEvent ( QMouseEvent * e );
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);

public slots:

    virtual void toggleExpanded();

signals:
    void clicked();
    void doubleClicked();
    void pressed();
    void released();

protected:
    //! If true, we've had a pressed event and are waiting for a released event
    bool state;
    bool expanded_;
    std::string text_;
    Qt::CursorShape hoverCursor_;

private:
    QLabelClickable(const QLabelClickable& rhs);
    QLabelClickable& operator=(const QLabelClickable& rhs);
};

} // namespace voreen

#endif
