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

#include "voreen/qt/widgets/customlabel.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"

#include <QFont>
#include <QWidget>

namespace {
#ifdef __APPLE__
    const int fontSize = 13;
#else
    const int fontSize = 8;
#endif
}

namespace voreen {

CustomLabel::CustomLabel(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f) {
    initFont();
    processText();
}

CustomLabel::CustomLabel(const QString& text, QWidget* parent, Qt::WindowFlags f, bool disallowWordwrap)
    : QLabel(text, parent, f)
    , disallowWordwrap_(disallowWordwrap)
{
    initFont();
    processText();
}

void CustomLabel::initFont() {
    QFontInfo fontInfo(font());
    setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));
    setToolTip(text());
}
void CustomLabel::processText() {
    //this should somehow reflect the size of a possible parent widget and therefore react on resize events
    if (text().length() > 20 && !disallowWordwrap_)
        setWordWrap(true);
}

} // namespace
