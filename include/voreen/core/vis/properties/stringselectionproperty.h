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

#ifndef VRN_STRINGSELECTIONPROPERTY_H
#define VRN_STRINGSELECTIONPROPERTY_H

#include "voreen/core/vis/properties/stringproperty.h"

namespace voreen {

/**
 * Behaves like a StringProp but has an additional list of pre-defined choices from which the
 * user can choose one. Note that the list of choices is handled by the application and
 * therefore not serialized.
 *
 * @param editable Is the string editable or can the user only select one of the predefined choices?
 */
class StringSelectionProp : public StringProp {
public:
    StringSelectionProp(const std::string& id, const std::string& guiText, bool editable,
                        const std::string& value = "", bool invalidate = true, bool invalidateShader = false);

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;

    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    void setChoices(const std::vector<std::string>& choices);
    std::vector<std::string> getChoices() const;

    bool getEditable() const { return editable_; }
    
protected:
    bool editable_;
    std::vector<std::string> choices_;
};

} // namespace

#endif //VRN_STRINGSELECTIONPROPERTY_H
