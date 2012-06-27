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

#include "voreen/core/datastructures/volume/volumecontainer.h"

#include "voreen/core/voreenapplication.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/io/progressbar.h"


namespace voreen {

const std::string VolumeContainer::loggerCat_ = "voreen.core.VolumeContainer";

VolumeContainer::VolumeContainer() :
    VolumeCollection()
{}

VolumeContainer::~VolumeContainer() {
    clear();
}

void VolumeContainer::add(VolumeHandle* volumeHandle) {
    VolumeCollection::add(volumeHandle);
}

void VolumeContainer::add(const VolumeCollection* volumeCollection) {
    VolumeCollection::add(volumeCollection);
}

void VolumeContainer::remove(const VolumeHandle* handle) {
    if (contains(handle)) {
        VolumeCollection::remove(handle); // also notifies the observers
        delete handle;
    }
}

void VolumeContainer::remove(const VolumeCollection* volumeCollection) {

    tgtAssert(volumeCollection, "Unexpected null pointer");

    // the passed parameter might be object itself,
    // therefore we have to copy over the volume handles to a
    // temporary vector
    std::vector<VolumeHandle*> deleteList;
    for (size_t i=0; i<volumeCollection->size(); ++i) {
        deleteList.push_back(volumeCollection->at(i));
    }

    for (size_t i=0; i<deleteList.size(); ++i)
        remove(deleteList[i]);
}

void VolumeContainer::clear() {
    while (!empty())
        remove(first());
}

VolumeCollection* VolumeContainer::loadVolume(const std::string& filename)
    throw (tgt::FileException, std::bad_alloc) {

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
        // detect duplicates (volumes already loaded from one of the passed origins)
        VolumeCollection duplicates;
        for (size_t i=0; i<volumeCollection->size(); ++i) {
            VolumeCollection* tempCol = selectOrigin(volumeCollection->at(i)->getOrigin());
            duplicates.add(tempCol);
            delete tempCol;
        }

        // add loaded collection
        VolumeCollection::add(volumeCollection);

        // remove duplicates
        for (size_t i=0; i<duplicates.size(); i++)
            LINFO("Replacing duplicate volume " << duplicates.at(i)->getOrigin().getPath());
        remove(&duplicates);
    }

    return volumeCollection;
}

VolumeHandle* VolumeContainer::loadRawVolume(const std::string& filename, const std::string& objectModel,
    const std::string& format, const tgt::ivec3& dimensions, const tgt::vec3& spacing, int headerskip, bool bigEndian)
    throw (tgt::FileException, std::bad_alloc)
{

    ProgressBar* progress = VoreenApplication::app()->createProgressDialog();
    RawVolumeReader rawReader(progress);
    rawReader.setReadHints(dimensions, spacing, 0, objectModel, format, headerskip, bigEndian);

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
        // detect duplicates (volumes already loaded from one of the passed origins)
        VolumeCollection* duplicates = selectOrigin(handle->getOrigin());

        // add loaded collection
        VolumeCollection::add(handle);

        // remove duplicates
        for (size_t i=0; i<duplicates->size(); i++)
            LINFO("Replacing duplicate volume " << duplicates->at(i)->getOrigin().getPath());
        remove(duplicates);
        delete duplicates;
    }

    delete progress;

    return handle;
}

void VolumeContainer::release(const VolumeHandle* handle) {
    VolumeCollection::remove(handle);
}

void VolumeContainer::releaseAll() {
    VolumeCollection::clear();
}

} // namespace
