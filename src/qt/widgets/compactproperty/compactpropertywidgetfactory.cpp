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

#include "voreen/qt/widgets/compactproperty/compactpropertywidgetfactory.h"

#include "voreen/qt/widgets/compactproperty/compactboolpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactcolorpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactenumpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactfiledialogpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactfloatpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactfloatvec2propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactfloatvec3propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactfloatvec4propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactintpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactintvec2propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactintvec3propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactintvec4propertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactoptionpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactpropertyvectorwidget.h"
#include "voreen/qt/widgets/compactproperty/compactstringpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactstringselectionpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactstringvectorpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compacttransfuncpropertywidget.h"

namespace voreen {

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(FloatProp* p) {
    return new CompactFloatPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(IntProp* p) {
    return new CompactIntPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(BoolProp* p) {
    return new CompactBoolPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(StringProp* p) {
    return new CompactStringPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(StringVectorProp* p) {
    return new CompactStringVectorPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(StringSelectionProp* p) {
    return new CompactStringSelectionPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(ColorProp* p) {
    return new CompactColorPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(EnumProp* p) {
    return new CompactEnumPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(FileDialogProp* p) {
    return new CompactFileDialogPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(TransFuncProp* p) {
    return new CompactTransFuncPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(FloatVec2Prop* p) {
    return new CompactFloatVec2PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(FloatVec3Prop* p) {
    return new CompactFloatVec3PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(FloatVec4Prop* p) {
    return new CompactFloatVec4PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(IntVec2Prop* p) {
    return new CompactIntVec2PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(IntVec3Prop* p) {
    return new CompactIntVec3PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(IntVec4Prop* p) {
    return new CompactIntVec4PropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(OptionPropertyBase* p) {
    return new CompactOptionPropertyWidget(p, 0);
}

QPropertyWidget* CompactPropertyWidgetFactory::createWidget(PropertyVector* p) {
    return new CompactPropertyVectorWidget(p, 0);
}

} // namespace voreen
