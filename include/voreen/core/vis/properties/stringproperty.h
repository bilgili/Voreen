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

#ifndef VRN_STRINGPROPERTY_H
#define VRN_STRINGPROPERTY_H

#include "voreen/core/vis/properties/templateproperty.h"

namespace voreen {

class StringProp : public TemplateProperty<std::string> {
public:
    StringProp(const std::string& id, const std::string& guiText, const std::string& value = "",
        bool invalidate = true, bool invalidateShader = false);

    virtual ~StringProp() {}

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;

    PropertyWidget* createWidget(PropertyWidgetFactory* f);
    virtual std::string toString() const { return value_; }
};

// ============================================================================

class StringVectorProp : public TemplateProperty<std::vector<std::string> > {
public:
    StringVectorProp(const std::string& id, const std::string& guiText, const std::vector<std::string>& value,
        bool invalidate = true, bool invalidateShader = false);

    virtual ~StringVectorProp() {}

    PropertyWidget* createWidget(PropertyWidgetFactory* f);
    virtual std::string toString() const;
};

}   // namespace

#endif
