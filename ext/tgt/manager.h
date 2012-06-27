/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_MANAGER_H
#define TGT_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <list>
#include <iostream>

#include "tgt/config.h"
#include "tgt/types.h"
#include "tgt/assert.h"

namespace tgt {

template <class T>
class ResourceManager {
protected:
    struct Resource {
        T*          data_;
        ulong       usedBy_;
        std::string filename_;
    };

    std::map< std::string, Resource* >  resourcesByFilename_;
    std::map< T*, Resource* >           resourcesByPtr_;
    bool cacheResources_;

    //path_ deprecated - should be remove from derived classes
    std::string path_;
    std::list<std::string> pathList_;
    
    void reg(T *ptr, const std::string& filename);
    void increaseUsage(const std::string& filename);
    
    // getter - setter
    T* get(const std::string& filename) { return resourcesByFilename_[filename]->data_; }
    const T* get(const std::string& filename) const { return resourcesByFilename_[filename]->data_; }

public:
    /// A constructor
    ResourceManager(bool cacheResources = true);
    
    /// Destroy all resources in memory
    virtual ~ResourceManager();
    
    //virtual T* load(const std::string& filename) { return 0; }
    
    /// Check if resource is loaded
    bool isLoaded(const std::string& filename);
    
    /// Mark resource as unused
    virtual void dispose(T* ptr);

    // getter - setter
    void setPath(std::string path) { path_ = path; }
    std::string getPath() { return path_; }
    void addPath(std::string path);
    void removePath(std::string path);
    
    bool isCaching() { return cacheResources_; }

    std::vector<std::string> getFilenames();
};





template <class T>
void ResourceManager<T>::reg(T* ptr, const std::string& filename) {
    if(!cacheResources_ && isLoaded(filename) ) {
        Resource* r = resourcesByFilename_[filename];
        r->data_ = ptr;
        r->usedBy_++;
        resourcesByPtr_[ptr] = r;
    }
    else {
        Resource* r = new Resource();
        r->data_ = ptr;
        r->usedBy_ = 1;
        r->filename_ = filename;
        resourcesByFilename_[filename] = r;
        resourcesByPtr_[ptr] = r;
    }
}

template <class T>
void ResourceManager<T>::increaseUsage(const std::string& filename) {
    tgtAssert(cacheResources_, "increaseUsage should not be called in non-caching mode!");
    resourcesByFilename_[filename]->usedBy_++;
}

template <class T>
ResourceManager<T>::ResourceManager(bool cacheResources) {
    //this should not be changed afterwards:
    cacheResources_ = cacheResources;
    path_ = "";
}

template <class T>
ResourceManager<T>::~ResourceManager() {
    while(!resourcesByFilename_.empty()) {
#ifdef TGT_DEBUG
        std::cout << "Un-disposed Resource: " << (*resourcesByFilename_.begin()).second->filename_
                  << " in use by " << (*resourcesByFilename_.begin()).second->usedBy_ << std::endl;
#endif
        delete resourcesByFilename_.begin()->second->data_;
        delete resourcesByFilename_.begin()->second;
        resourcesByFilename_.erase(resourcesByFilename_.begin());
    }
}

template <class T>
bool ResourceManager<T>::isLoaded(const std::string& filename) {
    return( (cacheResources_) && (resourcesByFilename_.find(filename) != resourcesByFilename_.end()) );
}

template <class T>
void ResourceManager<T>::dispose(T* ptr) {
    if(ptr == 0){
        return;
    }
    if(resourcesByPtr_.find(ptr) == resourcesByPtr_.end()) {
        return;
    }
    Resource* r = resourcesByPtr_[ptr];
    r->usedBy_--;
    //check if resource is still in use:
    if(r->usedBy_ == 0) {
        std::string filename = r->filename_;
        delete r->data_;
        resourcesByFilename_.erase(filename);
        resourcesByPtr_.erase(ptr);
        delete r;
    }
    else if(!cacheResources_) {
        resourcesByPtr_.erase(ptr);
    }
    ptr = 0;
}

template <class T>
void ResourceManager<T>::addPath( std::string path ) {
    pathList_.push_front(path);
	// remove duplicates
	pathList_.sort();
	pathList_.unique();
}

template <class T>
void ResourceManager<T>::removePath( std::string path ) {
    std::list<std::string>::iterator it;
    for( it = pathList_.begin(); it != pathList_.end(); ++it ) {
        if ( *it == path )
            pathList_.erase( it );
    }   
}

template <class T>
std::vector<std::string> ResourceManager<T>::getFilenames() {
    std::vector<std::string> filenames;
    for (typename std::map<std::string, Resource*>::const_iterator iter = resourcesByFilename_.begin(); iter != resourcesByFilename_.end(); iter++)
        filenames.push_back((*iter).first);
    return filenames;
}


} // namespace tgt

#endif //TGT_MANAGER_H
