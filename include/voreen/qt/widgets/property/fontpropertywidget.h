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

#ifndef VRN_FONTPROPTERYWIDGET_H
#define VRN_FONTPROPERTYWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <math.h>
#include <tgt/vector.h>
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/core/properties/fontproperty.h"

namespace voreen {
class FontPropertyWidget : public QPropertyWidget {
Q_OBJECT
public:
    FontPropertyWidget(FontProperty* prop, QWidget* parent = 0);

protected:
    FontProperty* property_;

    void updateFromProperty();

public slots:
    void updateProperty();

private:
    QComboBox* tgtFontType_;
    QComboBox* tgtFontName_;
    QSlider* tgtFontSizeSlider_;
    QSlider* tgtLineWidthSlider_;
    QComboBox* tgtTextAlign_;
    QComboBox* tgtVerticalTextAlign_;
};

} // namespace voreen

#endif // VRN_LIGHTPROPTERYWIDGET_H
