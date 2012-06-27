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

#ifndef VRN_COMPACTFLOATVEC3PROPERTYWIDGET_H
#define VRN_COMPACTFLOATVEC3PROPERTYWIDGET_H

#include "voreen/qt/widgets/compactproperty/compactpropertywidget.h"

class QDoubleSpinBox;

namespace voreen {

class FloatVec3Prop;

class CompactFloatVec3PropertyWidget : public CompactPropertyWidget {
Q_OBJECT
public:
    CompactFloatVec3PropertyWidget(FloatVec3Prop* prop, QWidget* parent = 0);
    void update();

public slots:
    void setPropertyX(double x);
    void setPropertyY(double y);
    void setPropertyZ(double z);

protected:
    FloatVec3Prop* property_;
    QDoubleSpinBox* spinBox_x;
    QDoubleSpinBox* spinBox_y;
    QDoubleSpinBox* spinBox_z;
};

} // namespace

#endif // VRN_COMPACTFLOATVEC3PROPERTYWIDGET_H
