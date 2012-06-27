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

#ifndef VRN_INTVEC3PROPERTYWIDGET_H
#define VRN_INTVEC3PROPERTYWIDGET_H

#include "voreen/core/vis/properties/vectorproperty.h"
#include "voreen/qt/widgets/property/intpropertywidget.h"
#include "voreen/qt/widgets/property/vecpropertywidget.h"

namespace voreen {

class IntVec3PropertyWidget : public VecPropertyWidget<SliderSpinBoxWidget, IntVec3Property, int> {
Q_OBJECT;
public:
    IntVec3PropertyWidget(IntVec3Property* prop, QWidget* parent = 0);

public slots:
    void setProperty(int value);

signals:
    void valueChanged(IntVec3Property::ElemType);
};

} // namespace

#endif // VRN_INTVEC3PROPERTYWIDGET_H
