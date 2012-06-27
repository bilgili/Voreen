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

#include "tgt/modelmanager.h"

#include "tgt/vector.h"
#include "tgt/logmanager.h"
//#include "tgt/texture.h"

namespace tgt {
    
const std::string ModelManager::loggerCat_("tgt.Model.Manager");
    
std::string ModelManager::getEnding(const std::string& filename) const {
    std::string ending = "";
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        ending = filename.substr(pos+1);
    }
    return ending;
}

ModelManager::~ModelManager(){
    // clean up
    for (std::set<ModelReader*>::iterator iter = readerSet_.begin(); iter != readerSet_.end(); ++iter)
        delete *iter;
}

Model* ModelManager::load(const std::string& filename) {
    // already loaded?
    if ( isLoaded(filename) ) {
        increaseUsage(filename);
        return get(filename);
    }

    Model* m = NULL;
    
    std::string ending = getEnding(filename);
    std::transform (ending.begin(), ending.end(), ending.begin(), tolower );


    if (readers_.find(ending) != readers_.end()) {
        LINFO("Found matching reader: " << readers_[ending]->getName());
        
        // try every path
        std::string completeFilename;
        std::list<std::string>::iterator iter = pathList_.begin();
        while ( iter != pathList_.end() && !m ) {
            completeFilename = (!(*iter).empty() ? *iter + '/' : "") + filename;
            // check if responsable reader is able to load filename
            m = readers_[ending]->loadModel( completeFilename );
            iter++;
        }  
        
        // try just the filename without path
        if (!m) {
            m = readers_[ending]->loadModel( filename );
        }

    }
    
    if (m){
        reg(m, filename);
    }
        
    if (!m){
        LDEBUG( "Model file not found: " + filename );
    }
    
    return m;
}

void ModelManager::registerReader(ModelReader* mr) {
    readerSet_.insert(mr);
    LDEBUG("ModuleManager: Registering reader: " << mr->getName());

    std::string formats = "";
    std::vector<std::string> knownEndings = mr->getEndings();
    std::vector<std::string>::iterator i;
    for ( i = knownEndings.begin(); i != knownEndings.end(); ++i ) {
        readers_[*i] = mr;
        formats += *i + " ";
    }
    LDEBUG("Known formats: " << formats);
}

} // namespace tgt
