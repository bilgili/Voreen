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

#ifndef VRN_CACHE_H
#define VRN_CACHE_H

#include "voreen/core/io/cacheindex.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

class Port;
class Processor;
class LocalPortMapping;

template<typename T>
class Cache {
public:
    Cache(const std::string& cachedObjectsClassName, const std::string& assignedPortType);
    virtual ~Cache();

    T* find(Processor* const processor, Port* const outport, 
        LocalPortMapping* const localPortMapping);

    const std::string& getAssignedPortType() const { return assignedPortType_; }

    const std::string& getCachedObjectsClassName() const  { return cachedObjectsClassName_; }

    bool isEnabled() const { return isEnabled_; }

    /**
     * Returns true, if the given processor is compatbile with this Cache object,
     * false otherwise.
     */
    bool isCompatible(Processor* const processor) const;

    void setEnabled(const bool enable) { isEnabled_ = enable; }

    bool update(Processor* const processor, LocalPortMapping* const localPortMapping);   
    
    std::vector<Port*> getCacheConcernedOutports(Processor* const processor) const;

    std::string getProcessorsInportConfig(const Processor* processor, 
        LocalPortMapping* const localPortMapping) const;

protected:
    virtual T* getPortData(const Identifier& portType,
        LocalPortMapping* const localPortMapping) const = 0;

    virtual T* loadObject(const std::string& directory, const std::string& filename) const = 0;

    virtual std::string portContentToString(const Identifier& portID, 
        LocalPortMapping* const localPortMapping) const = 0;

    virtual std::string saveObject(T* const object, const std::string& directory,
        const std::string& filename) = 0;

protected:
    /**
     * Wrapper class to manage deletion of objects which have been loaded by the
     * cache and which therefore need to be deleted by the cache.
     */
    template<typename S = T>
    struct CacheObject {
        CacheObject(S* const object, const bool originsCache)
            : object_(object), originsCache_(originsCache)
        {
        }
        
        ~CacheObject() {
            if (originsCache_ == true)
                delete object_;
        }

        S* object_;
        bool originsCache_;
    };

    const std::string assignedPortType_;
    const std::string cachedObjectsClassName_;

    /**
     * The real cache: maps the key used by CacheIndexEntry to the object
     * which are currently in memory. Objects which are loaded using 
     * <code>loadObject()</code> will be inserted into this map.
     */
    typedef std::map<std::string, CacheObject<T>* > ObjectMap;
    ObjectMap objectMap_;

    CacheIndex& cacheIndex_;
    bool isEnabled_;

private:
    void cleanup();

private:
    static const std::string loggerCat_;
    const std::string cacheFolder_;
};

}   // namespace

#endif
