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

#ifndef VRN_LIGHTPROPERTY_H
#define VRN_LIGHTPROPERTY_H

#include "voreen/core/properties/vectorproperty.h"


namespace voreen {

/**
 * @deprecated Use FloatVec4Property with view Property::LIGHT_POSITION instead!
 */
class LightProperty : public FloatVec4Property {
public:
    LightProperty(const std::string& id, const std::string& guiText, const tgt::vec4& value,
                  const tgt::vec4& minimum = tgt::vec4(-10.0f), const tgt::vec4& maximum = tgt::vec4(10.0f),
                  Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~LightProperty() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

};

} //namespace voreen

#endif // VRN_LIGHTPROPTERYWIDGET_H
