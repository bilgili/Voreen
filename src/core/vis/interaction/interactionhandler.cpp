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

#include "voreen/core/vis/interaction/interactionhandler.h"
#include "voreen/core/vis/properties/property.h"

using tgt::Event;

namespace voreen {

InteractionHandler::InteractionHandler() :
    tgt::EventListener(),
    owner_(0)
{
}

Processor* InteractionHandler::getOwner() const {
    return owner_;
}

void InteractionHandler::setOwner(Processor* p) {
    if(owner_)
        LWARNINGC("voreen.interactionhandler", "Setting new owner for InteractionHandler!");
    owner_ = p;
}

void InteractionHandler::addProperty(Property* prop) {
    prop->setOwner(owner_);
    properties_.push_back(prop);
}

void InteractionHandler::addProperty(Property& prop) {
    addProperty(&prop);
}

const std::vector<Property*>& InteractionHandler::getProperties() const {
    return properties_;
}

} // namespace
