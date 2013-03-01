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

#ifndef VRN_VVDFORMAT_H
#define VRN_VVDFORMAT_H

#include "voreen/core/io/serialization/serializable.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumedisk.h"

namespace voreen {

///Helper class to save .vvd files.
class VRN_CORE_API VvdRawDataObject : public Serializable {
public:
    VvdRawDataObject() {}
    VvdRawDataObject(const VolumeRAM* volume, std::string filename);

    tgt::ivec3 getDimensions() { return dimensions_; }
    std::string getFilename() { return filename_; }
    std::string getFormat() { return format_; }

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);
private:
    std::string filename_;
    tgt::ivec3 dimensions_;
    std::string format_;
};

///Helper class to save .vvd files.
class VRN_CORE_API VvdObject : public Serializable {
public:
    VvdObject() {}
    VvdObject(const VolumeBase* vh, std::string rawFilename);
    ~VvdObject();

    ///Because the filename is relative to the vvd file we need the directory
    Volume* createVolume(std::string directory);

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);
private:
    VvdRawDataObject rawData_;
    MetaDataContainer metaData_;
    std::set<VolumeDerivedData*> derivedData_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif
