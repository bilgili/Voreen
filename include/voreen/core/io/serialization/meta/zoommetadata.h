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

#ifndef VRN_ZOOMMETADATA_H
#define VRN_ZOOMMETADATA_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

/**
 * The @c ZoomMetaData class stores some zoom properties like zoom factor or focus position.
 *
 * @see MetaDataBase
 */

class ZoomMetaData : public MetaDataBase {
public:
    ZoomMetaData(const tgt::Matrix3d transform = tgt::Matrix3d());
    virtual ~ZoomMetaData();
    virtual void serialize(XmlSerializer& s) const; ///< @see Serializable::serialize
    virtual void deserialize(XmlDeserializer& s);  ///< @see Serializable::deserialize
    virtual const std::string getTypeString(const std::type_info& type) const; ///< @see SerializableFactory::getTypeString
    virtual Serializable* createType(const std::string& typeString); ///< @see SerializableFactory::createType
    void setTransform(tgt::mat3 transform);
    tgt::Matrix3d getTransform() const;

private:
    tgt::Matrix3d transform_;
};


} // namespace

#endif // VRN_ZOOMMETADATA_H
