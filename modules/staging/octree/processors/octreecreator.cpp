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

#include "octreecreator.h"

#include "../datastructures/volumeoctree.h"
#include "../datastructures/octreebrickpoolmanager.h"
#include "../datastructures/octreebrickpoolmanagerdisk.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresizepoweroftwo.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorconvert.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"
#include "voreen/core/datastructures/volume/volumeminmax.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/utils/hashing.h"

#include "tgt/vector.h"
#include "tgt/tgt_math.h"
#include "tgt/stopwatch.h"
#include "tgt/filesystem.h"

#include <queue>
#include <stack>

using tgt::ivec2;
using tgt::ivec3;
using tgt::ivec4;
using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::svec3;

namespace voreen {

const std::string OctreeCreator::loggerCat_("voreen.OctreeCreator");

OctreeCreator::OctreeCreator()
    : VolumeProcessor()
    , volumeInport_(Port::INPORT, "volumeInput", "Volume Input")
    , volumeOutport_(Port::OUTPORT, "volumeOutport", "Volume Output")
    , brickDimensions_("brickDimensions", "Brick Dimensions", 16, 2, 128)
    , homogeneityThreshold_("homogeneityThreshold", "Homogeneity Threshold", 0.005f, 0.f, 0.1f)
    , brickPoolManager_("brickPoolManager", "Brick Pool Manager")
    , singleBufferMemorySize_("singleBufferMemorySize", "Single Buffer Size (MB)", 64, 1, 1024)
    , maxRamUsage_("maxRamUsage", "Max RAM Usage (MB)", 1024, 1, 16384)
    , useCaching_("useCaching", "Use Disk Cache", true, VALID)
    , octreeCachePath_("octreeBasePath", "Cache Path", "", "", "", FileDialogProperty::DIRECTORY, VALID)
    , clearCache_("clearCache", "Clear Cache", VALID)
    , regenerateOctreeButton_("generateOctree", "Generate Octree")
    , regenerateOctree_(false)
{
    addPort(volumeInport_);
    addPort(volumeOutport_);

    addProperty(regenerateOctreeButton_);
    regenerateOctreeButton_.onChange(CallMemberAction<OctreeCreator>(this, &OctreeCreator::regenerateOctree));

    addProperty(brickDimensions_);
    homogeneityThreshold_.setNumDecimals(3);
    homogeneityThreshold_.setStepping(0.001f);
    addProperty(homogeneityThreshold_);
    brickDimensions_.setGroupID("octree");
    homogeneityThreshold_.setGroupID("octree");
    setPropertyGroupGuiName("octree", "Octree");

    brickPoolManager_.addOption("brickPoolManagerRAM",  "RAM");
    brickPoolManager_.addOption("brickPoolManagerDisk", "Disk");
    addProperty(brickPoolManager_);
    brickPoolManager_.onChange(CallMemberAction<OctreeCreator>(this, &OctreeCreator::updatePropertyConfiguration));
    addProperty(singleBufferMemorySize_);
    addProperty(maxRamUsage_);
    brickPoolManager_.setGroupID("brickPoolManager");
    singleBufferMemorySize_.setGroupID("brickPoolManager");
    maxRamUsage_.setGroupID("brickPoolManager");
    maxRamUsage_.setWidgetsEnabled(false);
    setPropertyGroupGuiName("brickPoolManager", "Brick Pool");

    addProperty(useCaching_);
    addProperty(octreeCachePath_);
    addProperty(clearCache_);
    clearCache_.onChange(CallMemberAction<OctreeCreator>(this, &OctreeCreator::clearCache));
    octreeCachePath_.setGroupID("disk-management");
    useCaching_.setGroupID("disk-management");
    clearCache_.setGroupID("disk-management");
    setPropertyGroupGuiName("disk-management", "Disk Management");

}

OctreeCreator::~OctreeCreator() {

}

Processor* OctreeCreator::create() const {
    return new OctreeCreator();
}

void OctreeCreator::initialize() throw (VoreenException) {
    VolumeProcessor::initialize();
}

void OctreeCreator::deinitialize() throw (VoreenException) {
    volumeOutport_.clear();
    VolumeProcessor::deinitialize();
}

void OctreeCreator::process() {
    if (!volumeInport_.hasChanged() && !regenerateOctree_)
        return;

    updatePropertyConfiguration();

    volumeOutport_.clear();

    VolumeBase* inputVolume = const_cast<VolumeBase*>(volumeInport_.getData());
    if (!inputVolume)
        return;

    // remove existing octree representation
    inputVolume->removeRepresentation<VolumeOctreeBase>();
    brickDimensions_.setMaxValue((int)inputVolume->getDimensions().x);

    // if caching enabled, try to restore octree from cache
    VolumeOctreeBase* octree = 0;
    if (useCaching_.get() && VoreenApplication::app()->useCaching() && !regenerateOctree_) {
        octree = restoreOctreeFromCache();
    }
    // if octree has not been restored from cache, generate it and store it to cache
    if (!octree) {
        octree = generateOctree();
    }
    regenerateOctree_ = false;

    if (octree) {
        inputVolume->addRepresentation(octree);
        volumeOutport_.setData(inputVolume, false);
    }

    setProgress(1.f);
}

VolumeOctreeBase* OctreeCreator::generateOctree() {
    // retrieve input RAM volume
    const VolumeBase* inputVolume = volumeInport_.getData();
    if (!inputVolume) {
        LERROR("Unable to create octree: no input volume");
        return 0;
    }
    if (!inputVolume->getRepresentation<VolumeRAM>()) {
        LERROR("Unable to create octree: VolumeRAM not available");
        return 0;
    }

    // resize input volume to pot-dimensions, if necessary
    const VolumeBase* uint16tVolume = 0;
    if (dynamic_cast<const VolumeRAM_UInt16*>(inputVolume->getRepresentation<VolumeRAM>())) {
        uint16tVolume = inputVolume;
    }
    else {
        LINFO("Converting input volume to UInt16");
        VolumeOperatorConvert convertOp;
        Volume* uint16Volume = convertOp.apply<uint16_t>(inputVolume);
        if (!uint16Volume) {
            LERROR("Failed to convert input volume to UInt16");
            return 0;
        }
        uint16tVolume = uint16Volume;
    }
    tgtAssert(dynamic_cast<const VolumeRAM_UInt16*>(uint16tVolume->getRepresentation<VolumeRAM>()),
        "input volume not properly converted to UInt16");

    // select brick pool manager
    OctreeBrickPoolManagerBase* brickPoolManager = 0;
    if (brickPoolManager_.isSelected("brickPoolManagerRAM"))
        brickPoolManager = new OctreeBrickPoolManagerRAM(singleBufferMemorySize_.get() << 20);
    else if (brickPoolManager_.isSelected("brickPoolManagerDisk")) {
        std::string brickPoolPath = getOctreeStoragePath();
        if (!tgt::FileSystem::dirExists(brickPoolPath))
            tgt::FileSystem::createDirectoryRecursive(brickPoolPath);
        // TODO
        brickPoolManager = new OctreeBrickPoolManagerDiskLimitedRam(
            static_cast<size_t>(singleBufferMemorySize_.get()) << 20, static_cast<size_t>(maxRamUsage_.get()) << 20,
            false, brickPoolPath, "");
    }
    else {
        LERROR("Unknown brick pool manager: " << brickPoolManager_.get());
        return 0;
    }

    // create octree
    VolumeOctreeBase* octree = 0;
    try {
        octree = new VolumeOctree(uint16tVolume, brickDimensions_.get(), homogeneityThreshold_.get(), brickPoolManager, progressBar_);
    }
    catch (std::exception& e) {
        LERROR("Failed to create octree: " << e.what());
        delete brickPoolManager;
        brickPoolManager = 0;
    }

    // delete converted input volume, if created
    if (inputVolume != uint16tVolume) {
        delete uint16tVolume;
        uint16tVolume = 0;
    }

    // add minmax derived data to inputVolume volume, if not present, and assign inputVolume to octree
    if (!inputVolume->hasDerivedData<VolumeMinMax>()) {
        float minNorm = octree->getRootNode()->getMinValue() / 65535.f;
        float maxNorm = octree->getRootNode()->getMaxValue() / 65535.f;
        tgtAssert(minNorm <= maxNorm, "invalid min/max values");
        float min = inputVolume->getRealWorldMapping().normalizedToRealWorld(minNorm);
        float max = inputVolume->getRealWorldMapping().normalizedToRealWorld(maxNorm);
        const_cast<VolumeBase*>(inputVolume)->addDerivedData(new VolumeMinMax(min, max, minNorm, maxNorm));
    }

    // store octree to cache, if enabled
    if (octree && useCaching_.get() && VoreenApplication::app()->useCaching()) {
        storeOctreeToCache(octree);
    }

    return octree;
}

void OctreeCreator::storeOctreeToCache(const VolumeOctreeBase* octree) const {
    tgtAssert(octree, "null pointer passed");

    // detect/create base cache path
    const std::string cachePath = getOctreeStoragePath();
    if (!tgt::FileSystem::dirExists(cachePath) && !tgt::FileSystem::createDirectoryRecursive(cachePath)) {
        LWARNING("Failed to create cache directory: " << cachePath);
        return;
    }
    else {
        //tgt::FileSystem::clearDirectory(cachePath);
    }

    // serialize octree
    std::string octreePath = tgt::FileSystem::cleanupPath(cachePath + "/octree.xml");
    XmlSerializer serializer(octreePath);
    try {
        serializer.serialize("Octree", octree);
    }
    catch (SerializationException& e) {
        LWARNING("Failed to serialize octree: " << e.what());
        return;
    }

    // write serialization to stream
    std::ostringstream textStream;
    try {
        serializer.write(textStream);
        if (textStream.fail()) {
            LWARNING("Failed to write octree serialization to string stream");
            return;
        }
    }
    catch (std::exception& e) {
        LWARNING("Failed to write octree serialization to string stream: " << e.what());
        return;
    }

    // now we have a valid string stream containing the serialized octree
    // => open output file and write it to the file
    std::fstream fileStream(octreePath.c_str(), std::ios_base::out);
    if (fileStream.fail()) {
        LWARNING("Failed to open file '" << octreePath << "' for writing.");
        return;
    }

    try {
        fileStream << textStream.str();
    }
    catch (std::exception& e) {
        LWARNING("Failed to write serialization data stream to file '" << octreePath << "': " << std::string(e.what()));
    }
    fileStream.close();

    LINFO("Serialized octree to file: " << octreePath);
}

VolumeOctreeBase* OctreeCreator::restoreOctreeFromCache() const {
    const std::string octreeFile = tgt::FileSystem::cleanupPath(getOctreeStoragePath() + "/octree.xml");
    if (!tgt::FileSystem::fileExists(octreeFile))
        return 0;

    tgt::Stopwatch stopWatch;
    stopWatch.start();

    // open file for reading
    std::fstream fileStream(octreeFile.c_str(), std::ios_base::in);
    if (fileStream.fail()) {
        LWARNING("Failed to open cached octree file '" << octreeFile << "' for reading.");
        return 0;
    }

    // read data stream into deserializer
    XmlDeserializer d(octreeFile);
    try {
        d.read(fileStream);
    }
    catch (std::exception& e) {
        LWARNING("Failed to read serialization data stream from cached octree file '" + octreeFile + "': " + e.what());
        return 0;
    }

    // deserialize octree from data stream
    VolumeOctreeBase* octree = 0;
    try {
        d.deserialize("Octree", octree);
        tgtAssert(octree, "null pointer after deserialization");
        tgtAssert(octree->getRootNode(), "deserialized octree has no root node");
        LINFO("Restored cached octree from file: " << octreeFile << " (" << stopWatch.getRuntime() << " msec)");

        // add minmax derived data to source volume, if not present, and assign source vol to octree
        VolumeBase* sourceVolume = const_cast<VolumeBase*>(volumeInport_.getData());
        tgtAssert(sourceVolume, "no source volume");
        if (!sourceVolume->hasDerivedData<VolumeMinMax>()) {
            float minNorm = octree->getRootNode()->getMinValue() / 65535.f;
            float maxNorm = octree->getRootNode()->getMaxValue() / 65535.f;
            tgtAssert(minNorm <= maxNorm, "invalid min/max values");
            float min = sourceVolume->getRealWorldMapping().normalizedToRealWorld(minNorm);
            float max = sourceVolume->getRealWorldMapping().normalizedToRealWorld(maxNorm);
            sourceVolume->addDerivedData(new VolumeMinMax(min, max, minNorm, maxNorm));
        }

#ifdef VRN_VOLUMEOCTREE_DEBUG
        octree->logDescription();
#endif
    }
    catch (std::exception& e) {
        LWARNING("Failed to restore cached octree from file '" + octreeFile + "': " + e.what());
        delete octree;
        octree = 0;
    }

    return octree;
}

void OctreeCreator::clearCache() {
    std::string cachePath = getOctreeStoragePath();

    // delete all subdirectories of parent path that have an octree.xml in it
    cachePath = tgt::FileSystem::parentDir(cachePath);
    std::vector<std::string> subdirs = tgt::FileSystem::listSubDirectories(cachePath);
    for (size_t i=0; i<subdirs.size(); i++) {
        std::string subdirAbs = tgt::FileSystem::cleanupPath(cachePath + "/" + subdirs.at(i));
        if (tgt::FileSystem::fileExists(subdirAbs + "/octree.xml")) {
            if (!tgt::FileSystem::deleteDirectoryRecursive(subdirAbs))
                LWARNING("Failed to delete directory: " << subdirAbs);
        }
    }
}

std::string OctreeCreator::getOctreeStoragePath() const {
    std::string cacheBasePath = octreeCachePath_.get();
    if (cacheBasePath == "" || !tgt::FileSystem::dirExists(cacheBasePath))
        cacheBasePath = VoreenApplication::app()->getCachePath("OctreeCreator");

    return tgt::FileSystem::cleanupPath(cacheBasePath + "/" + getConfigurationHash());
}

std::string OctreeCreator::getConfigurationHash() const {
    // compute property string
    std::map<std::string, const Property*> propertyMap;
    propertyMap[brickDimensions_.getID()] = &brickDimensions_;
    propertyMap[homogeneityThreshold_.getID()] = &homogeneityThreshold_;
    propertyMap[brickPoolManager_.getID()] = &brickPoolManager_;
    propertyMap[singleBufferMemorySize_.getID()] = &singleBufferMemorySize_;
    propertyMap[maxRamUsage_.getID()] = &maxRamUsage_;

    XmlSerializer s;
    const bool usePointerContentSerialization = s.getUsePointerContentSerialization();
    s.setUsePointerContentSerialization(true);
    try {
        s.serialize("Properties", propertyMap, "Property", "name");
    }
    catch (SerializationException& e) {
        LWARNING(e.what());
    }
    s.setUsePointerContentSerialization(usePointerContentSerialization);
    std::stringstream propertyConfig;
    s.write(propertyConfig);
    std::string propertyHash = VoreenHash::getHash(propertyConfig.str());

    // get input volume hash
    std::string volumeHash = volumeInport_.getData() ? volumeInport_.getData()->getHash() : "novolume";

    // concatenate property and volume hash
    return propertyHash + "-" + volumeHash;
}

void OctreeCreator::regenerateOctree() {
    regenerateOctree_ = true;
    invalidate();
}

void OctreeCreator::updatePropertyConfiguration() {
    maxRamUsage_.setWidgetsEnabled(brickPoolManager_.isSelected("brickPoolManagerDisk"));
}

}   // namespace
