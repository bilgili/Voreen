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

#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/propertywidgetfactory.h"

namespace voreen {

FloatMat2Property::FloatMat2Property(const std::string& id, const std::string& guiText,
                                     const tgt::mat2& value, const tgt::mat2& minimum,
                                     const tgt::mat2& maximum, Processor::InvalidationLevel invalidationLevel)
    : NumericProperty<tgt::mat2>(id, guiText, value, minimum, maximum, tgt::mat2(0.1f),
        invalidationLevel)
{}

PropertyWidget* FloatMat2Property::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string FloatMat2Property::getTypeString() const {
    return "FloatMatrix2x2";
}

//---------------------------------------------------------------------------

FloatMat3Property::FloatMat3Property( const std::string& id, const std::string& guiText,
                                     const tgt::mat3& value, const tgt::mat3& minimum,
                                     const tgt::mat3& maximum, Processor::InvalidationLevel invalidationLevel)
    : NumericProperty<tgt::mat3>(id, guiText, value, minimum, maximum, tgt::mat3(0.1f),
        invalidationLevel)
{}

PropertyWidget* FloatMat3Property::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

std::string FloatMat3Property::getTypeString() const {
    return "FloatMatrix3x3";
}

//---------------------------------------------------------------------------

FloatMat4Property::FloatMat4Property(const std::string& id, const std::string& guiText,
                                     const tgt::mat4& value, const tgt::mat4& minimum,
                                     const tgt::mat4& maximum, Processor::InvalidationLevel invalidationLevel)
    : NumericProperty<tgt::mat4>(id, guiText, value, minimum, maximum, tgt::mat4(0.1f),
        invalidationLevel)
{}

PropertyWidget* FloatMat4Property::createWidget(PropertyWidgetFactory* f)  {
    return f->createWidget(this);
}

std::string FloatMat4Property::getTypeString() const {
    return "FloatMatrix4x4";
}

} // namespace voreen
