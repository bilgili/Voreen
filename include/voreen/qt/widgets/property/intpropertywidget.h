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

#ifndef VRN_INTPROPERTYWIDGET_H
#define VRN_INTPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidget.h"

namespace voreen {

class IntProperty;
class SliderSpinBoxWidget;

class IntPropertyWidget : public QPropertyWidget {
Q_OBJECT;
public:
    IntPropertyWidget(IntProperty* prop, QWidget* parent = 0, bool addVisibilityControl = true);
    virtual ~IntPropertyWidget();
    void updateFromProperty();
    void setWidget(const int value, const int minValue, const int maxValue, const int stepping);

public slots:
    void setProperty(int value);

signals:
    void valueChanged(int);

protected:
    IntProperty* property_;
    SliderSpinBoxWidget* widget_;
};

} // namespace

#endif // VRN_INTPROPERTYWIDGET_H
