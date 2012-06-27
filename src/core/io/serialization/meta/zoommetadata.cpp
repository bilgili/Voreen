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

#include "voreen/core/io/serialization/meta/zoommetadata.h"

namespace voreen {

ZoomMetaData::ZoomMetaData(const tgt::Matrix3d transform)
    : transform_(transform)
{}

ZoomMetaData::~ZoomMetaData() {}

void ZoomMetaData::serialize(XmlSerializer& s) const {
    s.serialize("transform", transform_);
}

void ZoomMetaData::deserialize(XmlDeserializer& s) {
    s.deserialize("transform", transform_);
}

const std::string ZoomMetaData::getTypeString(const std::type_info& type) const {
    if (type == typeid(ZoomMetaData))
        return "ZoomMetaData";
    else
        return "";
}

Serializable* ZoomMetaData::createType(const std::string& typeString) {
    if (typeString == "ZoomMetaData")
        return new ZoomMetaData;
    else
        return 0;
}

void ZoomMetaData::setTransform(tgt::mat3 transform) {
    transform_ = transform;
}

tgt::Matrix3d ZoomMetaData::getTransform() const {
    return transform_;
}

} // namespace
