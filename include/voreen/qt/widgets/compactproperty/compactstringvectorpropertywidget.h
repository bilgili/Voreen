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

#ifndef VRN_COMPACTSTRINGVECTORPROPERTYWIDGET_H
#define VRN_COMPACTSTRINGVECTORPROPERTYWIDGET_H

#include "voreen/qt/widgets/compactproperty/compactpropertywidget.h"

class QDialogButtonBox;
class QGridLayout;
class QPushButton;

namespace voreen {

class StringVectorProp;

class CompactStringVectorPropertyWidget : public CompactPropertyWidget {
Q_OBJECT
public:
    CompactStringVectorPropertyWidget(StringVectorProp* prop, QWidget* parent = 0);
    void update() {}

public slots:
    void setProperty();

protected:
    StringVectorProp* property_;
    QDialogButtonBox* buttonBox_;
    QGridLayout* grid_;
    QPushButton* sliceButton_;
};

} // namespace

#endif // VRN_COMPACTSTRINGVECTORPROPERTYWIDGET_H
