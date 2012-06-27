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

#include "voreen/core/interaction/interactionhandlerfactory.h"

#include "voreen/core/interaction/interactionhandler.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/interaction/mwheelnumpropinteractionhandler.h"

namespace voreen {

InteractionHandlerFactory* InteractionHandlerFactory::instance_ = 0;

InteractionHandlerFactory::InteractionHandlerFactory() {
}

InteractionHandlerFactory* InteractionHandlerFactory::getInstance() {
    if (!instance_)
        instance_ = new InteractionHandlerFactory();

    return instance_;
}

const std::string InteractionHandlerFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(CameraInteractionHandler))
        return "CameraInteractionHandler";
    else if (type == typeid(MWheelNumPropInteractionHandler<int>))
        return "MWheelNumPropInteractionHandler.Integer";
    else if (type == typeid(MWheelNumPropInteractionHandler<float>))
        return "MWheelNumPropInteractionHandler.Float";
    else if (type == typeid(MWheelNumPropInteractionHandler<double>))
        return "MWheelNumPropInteractionHandler.Double";
    else
        return "";
}

Serializable* InteractionHandlerFactory::createType(const std::string& typeString) {
    if (typeString == "CameraInteractionHandler")
        return new CameraInteractionHandler();
    else if (typeString == "MWheelNumPropInteractionHandler.Integer")
        return new MWheelNumPropInteractionHandler<int>();
    else if (typeString == "MWheelNumPropInteractionHandler.Float")
        return new MWheelNumPropInteractionHandler<float>();
    else if (typeString == "MWheelNumPropInteractionHandler.Float")
        return new MWheelNumPropInteractionHandler<double>();
    else
        return 0;
}

} // namespace voreen
