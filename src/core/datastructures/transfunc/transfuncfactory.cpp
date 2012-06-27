/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/datastructures/transfunc/transfuncfactory.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"

namespace voreen {

const std::string TransFuncFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(TransFunc1DKeys))
        return "TransFuncIntensity";
    else if (type == typeid(TransFunc2DPrimitives))
        return "TransFuncIntensityGradient";
    else if (type == typeid(TransFuncMappingKey))
        return "TransFuncMappingKey";
    else if (type == typeid(TransFuncQuad))
        return "TransFuncQuad";
    else if (type == typeid(TransFuncBanana))
        return "TransFuncBanana";
    else
        return "";
}

Serializable* TransFuncFactory::createType(const std::string& typeString) {
    if (typeString == "TransFuncIntensity")
        return new TransFunc1DKeys();
    else if (typeString == "TransFuncIntensityGradient")
        return new TransFunc2DPrimitives();
    else if (typeString == "TransFuncMappingKey")
        return new TransFuncMappingKey();
    else if (typeString == "TransFuncQuad")
        return new TransFuncQuad();
    else if (typeString == "TransFuncBanana")
        return new TransFuncBanana();
    else
        return 0;
}

} // namespace voreen
