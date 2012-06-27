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

#ifndef VRN_COMPACTPROPERTYWIDGETFACTORY_H
#define VRN_COMPACTPROPERTYWIDGETFACTORY_H

#include "voreen/core/vis/properties/property.h"
#include "voreen/core/vis/propertywidget.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/qt/widgets/qpropertywidget.h"

namespace voreen {

/**
 * Creates compact Qt PropertyWidgets (used in VoreenVE)
 */
class CompactPropertyWidgetFactory : public PropertyWidgetFactory {
public:
    QPropertyWidget* createWidget(FloatProp* p);
    QPropertyWidget* createWidget(IntProp* p);
    QPropertyWidget* createWidget(BoolProp* p);
    QPropertyWidget* createWidget(StringProp* p);
    QPropertyWidget* createWidget(StringVectorProp* p);
    QPropertyWidget* createWidget(StringSelectionProp* p);
    QPropertyWidget* createWidget(ColorProp* p);
    QPropertyWidget* createWidget(EnumProp* p);
    QPropertyWidget* createWidget(FileDialogProp* p);
    QPropertyWidget* createWidget(TransFuncProp* p);
    QPropertyWidget* createWidget(FloatVec2Prop* p);
    QPropertyWidget* createWidget(FloatVec3Prop* p);
    QPropertyWidget* createWidget(FloatVec4Prop* p);
    QPropertyWidget* createWidget(IntVec2Prop* p);
    QPropertyWidget* createWidget(IntVec3Prop* p);
    QPropertyWidget* createWidget(IntVec4Prop* p);
    QPropertyWidget* createWidget(OptionPropertyBase* p);
    QPropertyWidget* createWidget(PropertyVector* p);
};

} // namespace

#endif // VRN_COMPACTPROPERTYWIDGETFACTORY_H
