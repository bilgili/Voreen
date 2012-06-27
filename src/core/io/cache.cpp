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

#include "voreen/core/io/cache.h"

#include "tgt/filesystem.h"
#include "voreen/core/application.h"
#include "voreen/core/vis/processors/processor.h"

#include <iostream>

using tgt::FileSystem;

namespace voreen {

template<typename T> const std::string Cache<T>::loggerCat_("Cache<T>");

template<typename T>
Cache<T>::Cache(const std::string& cachedObjectsClassName, const std::string& assignedPortType)
    : assignedPortType_(assignedPortType),
    cachedObjectsClassName_(cachedObjectsClassName),
    objectMap_(),
    cacheIndex_(CacheIndex::getInstance()),
    isEnabled_(true),
    cacheFolder_(VoreenApplication::app()->getCachePath())
{
}

template<typename T>
Cache<T>::~Cache() {
    typename Cache<T>::ObjectMap::iterator it = objectMap_.begin(); 
    for ( ; it != objectMap_.end(); ++it) {
        delete it->second;
        it->second = 0;
    }
    objectMap_.clear();
}

template<typename T>
T* Cache<T>::find(Processor* const processor, Port* const port, 
                  LocalPortMapping* const localPortMapping)
{
    if ((isEnabled_ == false) || (processor == 0) || (port == 0))
        return 0;

    // 1. Check whether the cached data already exist in the local object map and
    // return them if so.
    //
    std::string inportConfig = getProcessorsInportConfig(processor, localPortMapping);
    CacheIndex::IndexKey key = CacheIndex::generateCacheIndexKey(processor, port, inportConfig);
    std::string keyStr = key.first + "." + key.second;
    typename Cache<T>::ObjectMap::const_iterator it = objectMap_.find(keyStr);
    if (it != objectMap_.end()) {
        //std::cout << "find(): found object in local map!";
        cacheIndex_.incrementRefCounter(key);
        return (it->second)->object_;
    }

    // 2. If the data were not in the local object map, look up the cache index entry.
    // If the there is not even a cache index entry, the data are not cached at all
    // and 0 is returned.
    //
    const std::string filename = cacheIndex_.findFilename(processor, port, inportConfig);
    if (filename.empty() == true)
        return 0;

    // 3. Otherwise, if there is a cache index entry, load the concerned object and 
    // store it in the local object map for future lookups.
    //
    T* object = loadObject(cacheFolder_, filename);
    if (object != 0) {
        //std::cout << "find(): found cache index entry for object!";
        std::pair<typename Cache<T>::ObjectMap::iterator, bool> res = 
            objectMap_.insert(std::make_pair(keyStr, new CacheObject<T>(object, true)));
    }
    return object;
}

template<typename T>
std::vector<Port*> Cache<T>::getCacheConcernedOutports(Processor* const processor) const {
    if (processor == 0)
        return std::vector<Port*>();

    std::vector<Port*> concernedPorts;
    std::vector<Port*> outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (outports[i]->getType().getSubString(0) == assignedPortType_)
            concernedPorts.push_back(outports[i]);
    }
    return concernedPorts;
}

template<typename T>
bool Cache<T>::isCompatible(voreen::Processor* const processor) const {
    std::vector<Port*> outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (outports[i]->getType().getSubString(0) == assignedPortType_)
            return true;
    }
    return false;
}

template<typename T>
bool Cache<T>::update(Processor* const processor, LocalPortMapping* const localPortMapping) {
    if ((isEnabled_ == false) || (processor == 0))
        return false;

    std::string inportConfig = getProcessorsInportConfig(processor, localPortMapping);
    if (inportConfig.empty() == true)
        return false;

    std::vector<Port*> concernedPorts = getCacheConcernedOutports(processor);
    bool result = (! concernedPorts.empty());
    for (size_t i = 0; i < concernedPorts.size(); ++i) {
        T* object = getPortData(concernedPorts[i]->getType(), localPortMapping);
        if (object == 0)
            continue;

        // try to find an entry for that processor and port with the current 
        // configuration on the inports in the index of the cache
        //
        std::string filename = cacheIndex_.findFilename(processor, concernedPorts[i], inportConfig);
        bool requiresNewEntry = filename.empty();

        // save the object itself and add entry to the cache's index. if filename is not empty,
        // the file will be replaced. otherwise a new file will be created and the filename
        // which will be generated is retruned.
        //
        filename = saveObject(object, cacheFolder_, filename);
        result = (result && (! filename.empty()));
        std::string key;
        if ((result == true) && (requiresNewEntry == true)) {
            key = cacheIndex_.insert(processor, concernedPorts[i], cachedObjectsClassName_, 
                inportConfig, filename);
            // cleanup cache
            cleanup();
        }

        if (key.empty() == true) {
            LFATAL("update(): Insertion of cache index failed!");
            return false;
        }

        // add or replace the internal 'copy' of the object
        //
        std::pair<typename Cache<T>::ObjectMap::iterator, bool> res = 
            objectMap_.insert(std::make_pair(key, new CacheObject<T>(object, false)));

        if (res.second == false) {
            //std::cout << "update(): replacing existing data...";
            delete (res.first)->second;
            (res.first)->second = new CacheObject<T>(object, false);
        }
    }
    return result;
}

template<typename T>
std::string Cache<T>::getProcessorsInportConfig(const Processor* processor, 
                                                LocalPortMapping* const localPortMapping) const
{
    if ((processor == 0) || (localPortMapping == 0))
        return "";

    std::string inputConfig("");
    std::vector<Port*> inports = processor->getInports();
    for (size_t i = 0; i < inports.size(); ++i) {
        inputConfig += portContentToString(inports[i]->getType(), localPortMapping);
        if (i < (inports.size() - 1))
            inputConfig += ", ";
    }
    return inputConfig;
}

// private methods
//

template<typename T>
void Cache<T>::cleanup() {
    std::vector<std::pair<std::string, std::string> > dumps = cacheIndex_.cleanup();
    for (size_t i = 0; i < dumps.size(); ++i) {
        
        // 1. Erase the object from the local object map
        //
        objectMap_.erase(dumps[i].first);
        
        // 2. Delete the file from the cache's directory
        //
        std::string& file = dumps[i].second;
        //std::cout << "cleanup(): deleting file '" << file << "'...";
        FileSystem::deleteFile(file);

        // If file is a .dat file, also try to delete related .raw file
        //
        if (FileSystem::getFileExtension(file) == "dat") {
            file.replace(file.size() - 3, 3, "raw");
            FileSystem::deleteFile(file);
        }
    }
}

template class Cache<VolumeHandle>;

}   // namespace
