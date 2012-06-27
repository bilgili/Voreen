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

#include "voreen/core/io/serialization/meta/primitivemetadata.h"

namespace voreen {

const std::string BoolMetaData::getTypeString(const std::type_info& type) const {
    if (type == typeid(BoolMetaData))
        return "BoolMetaData";
    else
        return "";
}

Serializable* BoolMetaData::createType(const std::string& typeString) {
    if (typeString == "BoolMetaData")
        return new BoolMetaData;
    else
        return 0;
}

//---------------------------------------------------------------------------------------

const std::string StringMetaData::getTypeString(const std::type_info& type) const {
    if (type == typeid(StringMetaData))
        return "StringMetaData";
    else
        return "";
}

Serializable* StringMetaData::createType(const std::string& typeString) {
    if (typeString == "StringMetaData")
        return new StringMetaData;
    else
        return 0;
}

//---------------------------------------------------------------------------------------

const std::string IntMetaData::getTypeString(const std::type_info& type) const {
    if (type == typeid(IntMetaData))
        return "IntMetaData";
    else
        return "";
}

Serializable* IntMetaData::createType(const std::string& typeString) {
    if (typeString == "IntMetaData")
        return new IntMetaData;
    else
        return 0;
}

//---------------------------------------------------------------------------------------

const std::string FloatMetaData::getTypeString(const std::type_info& type) const {
    if (type == typeid(FloatMetaData))
        return "FloatMetaData";
    else
        return "";
}

Serializable* FloatMetaData::createType(const std::string& typeString) {
    if (typeString == "FloatMetaData")
        return new FloatMetaData;
    else
        return 0;
}

} // namespace
