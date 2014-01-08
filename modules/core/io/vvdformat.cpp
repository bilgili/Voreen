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

#include "vvdformat.h"

#include "voreen/core/datastructures/volume/volumehash.h"
#include "voreen/core/datastructures/volume/volumeminmax.h"
#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volumepreview.h"
#include "voreen/core/datastructures/volume/volumefactory.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"

namespace voreen {

VvdRawDataObject::VvdRawDataObject(const VolumeRAM* volume, std::string filename) : filename_(filename), dimensions_(volume->getDimensions()) {
    VolumeFactory vf;
    format_ = vf.getFormat(volume);
    if(format_ == "")
        LERRORC("voreen.VvdRawDataObject", "Format currently not supported");
}

void VvdRawDataObject::serialize(XmlSerializer& s) const {
    s.serialize("filename", filename_);
    s.serialize("format", format_);
    int x = dimensions_.x;
    int y = dimensions_.y;
    int z = dimensions_.z;
    s.serialize("x", x);
    s.serialize("y", y);
    s.serialize("z", z);
}

void VvdRawDataObject::deserialize(XmlDeserializer& s) {
    s.deserialize("filename", filename_);
    s.deserialize("format", format_);
    int x;
    int y;
    int z;
    s.deserialize("x", x);
    s.deserialize("y", y);
    s.deserialize("z", z);
    dimensions_ = tgt::ivec3(x,y,z);
}

//-----------------------------------------------------------------------------------------------------

VvdObject::~VvdObject() {
    // no need to delete: derived data pointers are either copied from existing volume or assigned to new volume
    //while(!derivedData_.empty()) {
        //delete *derivedData_.begin();
        //derivedData_.erase(derivedData_.begin());
    //}
}

VvdObject::VvdObject(const VolumeBase* vh, std::string rawFilename) : rawData_(vh->getRepresentation<VolumeRAM>(), rawFilename) {
    std::vector<std::string> keys = vh->getMetaDataKeys();
    for(size_t i=0; i<keys.size(); i++) {
        const MetaDataBase* md = vh->getMetaData(keys[i]);
        if(md) {

            if(keys[i] == "Transformation") {
                const PrimitiveMetaDataBase<tgt::mat4>* matrixMD = dynamic_cast<const PrimitiveMetaDataBase<tgt::mat4>*>(md);
                if(matrixMD) {
                    // write transformation matrix unless it is the identity matrix (default)
                    tgt::mat4 transformation = matrixMD->getValue();
                    if (transformation == tgt::mat4::createIdentity())
                        continue;
                }
            }

            if(keys[i] == "Modality") {
                const PrimitiveMetaDataBase<std::string>* stringMD = dynamic_cast<const PrimitiveMetaDataBase<std::string>*>(md);
                if(stringMD) {
                    // write modality unless it is unknown (default)
                    std::string modality = stringMD->getValue();
                    if (modality == "unknown")
                        continue;
                }
            }

            if(keys[i] == "Timestep") {
                const PrimitiveMetaDataBase<float>* floatMD = dynamic_cast<const PrimitiveMetaDataBase<float>*>(md);
                if(floatMD) {
                    // write timestep unless it is 0.0f (default)
                    float timestep = floatMD->getValue();
                    if (timestep == 0.0f)
                        continue;
                }
            }

            MetaDataBase* cl = md->clone();
            if(cl)
                metaData_.addMetaData(keys[i], cl);
            else {
                std::cout << "Failed to clone!";
            }
        }
    }

    derivedData_.insert(vh->getDerivedData<VolumeHash>());
    derivedData_.insert(vh->getDerivedData<VolumeMinMax>());
    derivedData_.insert(vh->getDerivedData<VolumePreview>());
    derivedData_.insert(vh->getDerivedData<VolumeHistogramIntensity>());
    //derivedData_.insert(vh->getDerivedData<VolumeHistogramIntensityGradient>()); //TODO: currently not implemented
    //TODO: removed hard-coded classes
}

Volume* VvdObject::createVolume(std::string directory) {
    VolumeRepresentation* volume;
    volume = (VolumeRAM*) new VolumeDiskRaw(directory+"/"+rawData_.getFilename(), rawData_.getFormat(), rawData_.getDimensions());

    Volume* vh = new Volume(volume, &metaData_, derivedData_);

    return vh;
}

void VvdObject::serialize(XmlSerializer& s) const {
    s.serialize("RawData", rawData_);
    metaData_.serialize(s);
    s.serialize("DerivedData", derivedData_, "DerivedItem");
}

void VvdObject::deserialize(XmlDeserializer& s) {
    s.deserialize("RawData", rawData_);
    metaData_.deserialize(s);

    metaData_.renameMetaData("transformation", "Transformation");
    metaData_.renameMetaData("timestep", "Timestep");
    metaData_.renameMetaData("modality", "Modality");
    metaData_.renameMetaData("spacing", "Spacing");
    metaData_.renameMetaData("offset", "Offset");

    try {
        s.deserialize("DerivedData", derivedData_, "DerivedItem");
    }
    catch (XmlSerializationNoSuchDataException& /*e*/) {
        s.removeLastError();
    }
}

} // namespace voreen

