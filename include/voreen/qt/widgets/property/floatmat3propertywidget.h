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

#ifndef VRN_FLOATMAT3PROPTERYWIDGET_H
#define VRN_FLOATMAT3PROPERTYWIDGET_H

#include "voreen/core/properties/matrixproperty.h"
#include "voreen/qt/widgets/property/matrixpropertywidget.h"

#include <QDoubleSpinBox>
#include <QWidget>

class FloatMat3Property;

namespace voreen {

class FloatMat3PropertyWidget : public MatrixPropertyWidget {
Q_OBJECT
public:
    FloatMat3PropertyWidget(FloatMat3Property*, QWidget* = 0);

    void updateFromProperty();

protected:
    FloatMat3Property* prop_;
    QDoubleValidator* doubleValidator_;
    QLineEdit* x0_;
    QLineEdit* y0_;
    QLineEdit* z0_;
    QLineEdit* x1_;
    QLineEdit* y1_;
    QLineEdit* z1_;
    QLineEdit* x2_;
    QLineEdit* y2_;
    QLineEdit* z2_;

protected slots:
    void updateValue();
    void identity();
};

}// namespace voreen

#endif // FLOATMAT3PROPTERYWIDGET_H
