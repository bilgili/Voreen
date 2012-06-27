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

#include "voreen/core/io/serialization/xmlserializationconstants.h"

namespace voreen {

const std::ios::fmtflags XmlSerializationConstants::FLOATFORMAT = std::ios::fixed;
const int XmlSerializationConstants::FLOATPRECISION = 6;

const std::string XmlSerializationConstants::XMLVERSION = "1.0";
const std::string XmlSerializationConstants::XMLENCODING = "";
const std::string XmlSerializationConstants::XMLSTANDALONE = "";

const std::string XmlSerializationConstants::ROOTNODE = "VoreenData";
const std::string XmlSerializationConstants::VERSIONATTRIBUTE = "version";
const std::string XmlSerializationConstants::VERSION = "1";

const std::string XmlSerializationConstants::IDATTRIBUTE = "id";
const std::string XmlSerializationConstants::REFERENCEATTRIBUTE = "ref";
const std::string XmlSerializationConstants::TYPEATTRIBUTE = "type";
const std::string XmlSerializationConstants::VALUEATTRIBUTE = "value";

const std::string XmlSerializationConstants::IDPREFIX = "ref";

const std::string XmlSerializationConstants::ITEMNODE = "item";
const std::string XmlSerializationConstants::KEYNODE = "key";
const std::string XmlSerializationConstants::VALUENODE = "value";

const std::string XmlSerializationConstants::TEMPNODE = "temp";

const std::string XmlSerializationConstants::VECTORXATTRIBUTE = "x";
const std::string XmlSerializationConstants::VECTORYATTRIBUTE = "y";
const std::string XmlSerializationConstants::VECTORZATTRIBUTE = "z";
const std::string XmlSerializationConstants::VECTORWATTRIBUTE = "w";

const std::string XmlSerializationConstants::COLORRATTRIBUTE = "r";
const std::string XmlSerializationConstants::COLORGATTRIBUTE = "g";
const std::string XmlSerializationConstants::COLORBATTRIBUTE = "b";
const std::string XmlSerializationConstants::COLORAATTRIBUTE = "a";

} // namespace
