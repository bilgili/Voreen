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

#include "voreen/qt/widgets/lineeditresetwidget.h"

#include <QWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QStyle>

namespace voreen {

LineEditResetWidget::LineEditResetWidget(QWidget* parent)
    : QLineEdit(parent)
{
    clearButton=new QToolButton(this);
    QPixmap pixmap(":/voreenve/icons/input-reset.png");
    clearButton->setIcon(QIcon(pixmap));
    clearButton->setStyleSheet("QToolButton { border: none; padding: 1px; }");
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->hide();
    clearButton->move(20,0);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(updateClearButton(const QString&)));
}

void LineEditResetWidget::resizeEvent(QResizeEvent*){
    QSize size = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right()- frameWidth - size.width(), (rect().bottom() + 1 - size.height())/2);
}

void LineEditResetWidget::updateClearButton(const QString &text){
    clearButton->setVisible(!text.isEmpty());
}

} //namespace voreen
