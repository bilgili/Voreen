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

#include "voreen/core/volume/volumecontainer.h"

#include "voreen/core/application.h"

#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/rawvolumereader.h"


namespace voreen {

const std::string VolumeContainer::loggerCat_ = "voreen.core.VolumeContainer";

VolumeContainer::VolumeContainer() :
    VolumeCollection()
{}

VolumeContainer::~VolumeContainer() {
    freeAll();
}

void VolumeContainer::add(VolumeHandle* volumeHandle) {
    VolumeCollection::add(volumeHandle);
}

void VolumeContainer::add(const VolumeCollection& volumeCollection) {
    VolumeCollection::add(volumeCollection);
}

VolumeCollection* VolumeContainer::loadVolume(const std::string& filename) throw (tgt::FileException, std::bad_alloc) {

    VolumeSerializerPopulator serializerPopulator;
    VolumeCollection* volumeCollection = 0;
    try {
        volumeCollection = serializerPopulator.getVolumeSerializer()->load(filename);
    }
    catch (tgt::FileException e) {
        LWARNING(e.what());
        throw e;
    }
    catch (std::bad_alloc) {
        LWARNING("std::Error BAD ALLOCATION");
        throw std::bad_alloc();
    }

    if (volumeCollection) {

        // remove duplicates
        for (size_t i=0; i<volumeCollection->size(); ++i) {
           tgtAssert(selectOrigin(volumeCollection->at(i)->getOrigin()).size() <= 1, "More than one volume to remove");
           free(selectOrigin(volumeCollection->at(i)->getOrigin()));
        }

        // add loaded collection
        VolumeCollection::add(*volumeCollection);
    }

    return volumeCollection;

}

VolumeHandle* VolumeContainer::loadRawVolume(const std::string& filename, const std::string& objectModel,
    const std::string& format, const tgt::ivec3& dimensions, const tgt::vec3& spacing, int headerskip)
    throw (tgt::FileException, std::bad_alloc)
{

    IOProgress* progress = VoreenApplication::app()->createProgressDialog();
    RawVolumeReader rawReader(progress);
    rawReader.readHints(dimensions, spacing, 0, objectModel, format, headerskip);

    VolumeCollection* collection = 0;
    try {
        collection = rawReader.read(filename);
    }
    catch (tgt::FileException e) {
        LWARNING(e.what());
        delete progress;
        throw e;
    }
    catch (std::bad_alloc) {
        LWARNING("std::Error BAD ALLOCATION");
        delete progress;
        throw std::bad_alloc();
    }

    VolumeHandle* handle = 0;
    if (collection && !collection->empty()) {
        tgtAssert(collection->size() == 1, "More than one raw volume returned");
        handle = collection->first();
    }
    delete collection;

    if (handle) {
        // remove duplicates
        tgtAssert(selectOrigin(handle->getOrigin()).size() <= 1, "More than one volume to remove");
        free(selectOrigin(handle->getOrigin()));

        // add loaded volume
        VolumeCollection::add(handle);
    }

    delete progress;

    return handle;
}


void VolumeContainer::free(const VolumeHandle* handle) {

    if (contains(handle)) {
        remove(handle); // also notifies the observers
        delete handle;
    }
}

void VolumeContainer::free(const VolumeCollection& volumeCollection) {
    for (size_t i=0; i<volumeCollection.size(); ++i) {
        free(volumeCollection.at(i)); // also notifies the observers
    }
}

void VolumeContainer::freeAll() {
    free(static_cast<VolumeCollection>(*this)); // also notifies the observers
}

} // namespace
