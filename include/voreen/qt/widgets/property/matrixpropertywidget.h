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

#ifndef VRN_MATRIXPROPERTYWIDGET_H
#define VRN_MATRIXPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidget.h"

class QCheckBox;
class QDialog;
class QGridLayout;
class QHBoxLayout;
class QPushButton;
class Property;

namespace voreen {

class MatrixPropertyWidget : public QPropertyWidget {
Q_OBJECT;
public:
    MatrixPropertyWidget(Property* prop, QWidget* parent = 0);

protected:
    //QHBoxLayout* layout_;
    QPushButton* editButton_;
    QPushButton* identityButton_;
    QPushButton* updateButton_;
    QGridLayout* mainLayout_;
    QDialog* matrixDialog_;
    QCheckBox* autoUpdate_;

protected slots:
    void executeDialog();
    virtual void identity() = 0;
    virtual void updateValue() = 0;
    void autoUpdate();
};

} // namespace voreen

#endif // VRN_MATRIXPROPERTYWIDGET_H
