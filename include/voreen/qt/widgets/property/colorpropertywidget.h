/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_COLORPROPERTYWIDGET_H
#define VRN_COLORPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidget.h"

#include "tgt/vector.h"

#include <QLabel>

namespace voreen {

class ColorProperty;

class ClickableColorLabel : public QLabel {
    Q_OBJECT
public:
    ClickableColorLabel(const QString& text, QWidget* parent=0, Qt::WindowFlags f=0);
    void setColor(const QColor& color);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* event);

    QColor color_;
};

class ColorPropertyWidget : public QPropertyWidget {
    Q_OBJECT
public:
    ColorPropertyWidget(ColorProperty* prop, QWidget* parent = 0);

    void updateFromProperty();

public slots:
    void setProperty();

protected:
    void colorDialog();
    void updateColorLabel();
    tgt::Color toTgtColor(QColor color);
    QColor toQColor(tgt::Color color);

private:
    ColorProperty* property_;
    ClickableColorLabel* colorLbl_;
    QColor currentColor_;
};

} // namespace

#endif // VRN_COLORPROPERTYWIDGET_H
