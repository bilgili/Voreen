/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

ButtonProperty::ButtonProperty(const std::string& id, const std::string& guiText, int invalidationLevel)
    : Property(id, guiText, invalidationLevel)
{
}

ButtonProperty::ButtonProperty() {
}

ButtonProperty::~ButtonProperty() {
    for(size_t i=0; i<actions_.size(); ++i)
        delete actions_[i];

    actions_.clear();
}

Property* ButtonProperty::create() const {
    return new ButtonProperty();
}

void ButtonProperty::onChange(const Action& action) {
    actions_.push_back(action.clone());
}

void ButtonProperty::onClick(const Action& action) {
    onChange(action);
}

void ButtonProperty::clicked() {
    invalidateOwner();

    for (size_t i = 0; i < actions_.size(); ++i)
        actions_[i]->exec();

    for (std::vector<PropertyLink*>::iterator it = links_.begin(); it != links_.end(); it++)
        (*it)->onChange();
}


}   // namespace
