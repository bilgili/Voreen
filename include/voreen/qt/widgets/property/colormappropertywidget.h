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

#ifndef VRN_COLORMAPPROPERTYWIDGET_H
#define VRN_COLORMAPPROPERTYWIDGET_H

#include "voreen/core/plotting/colormap.h"
#include "voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h"
#include "tgt/vector.h"

#include <QLabel>
#include <QComboBox>

namespace voreen {

class ColorMapProperty;

class ClickableColorMapLabel : public QLabel {
    Q_OBJECT
public:
    ClickableColorMapLabel(const QString& text, QWidget* parent=0, Qt::WindowFlags f=0);
    void setColorMap(const ColorMap& cm);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* event);
    tgt::Color toTgtColor(QColor color);
    QColor toQColor(tgt::Color color);

    ColorMap cm_;
};



// --------------------------------------------------------------------------------------



class ColorMapPropertyWidget : public QPropertyWidget {
    Q_OBJECT
public:
    ColorMapPropertyWidget(ColorMapProperty* prop, QWidget* parent = 0);
    void updateFromProperty();


private slots:
    void selectColormap(int index);

protected:

    void updateColorLabel();
    tgt::Color toTgtColor(QColor color);
    QColor toQColor(tgt::Color color);

private:
    ColorMapProperty* property_;
    QComboBox* cbMaps_;
    ClickableColorMapLabel* colorLbl_;
};

} // namespace

#endif // VRN_COLORMAPPROPERTYWIDGET_H
