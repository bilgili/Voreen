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

#ifndef TGT_MODELMANAGER_H
#define TGT_MODELMANAGER_H

#include <string>
#include <vector>
#include <set>

#include "tgt/config.h"
#include "tgt/singleton.h"
#include "tgt/manager.h"
#include "tgt/modelreader.h"

namespace tgt {

class Model;

class ModelManager : public ResourceManager<Model> {
protected:
    static const std::string loggerCat_;

    std::map<std::string, ModelReader*> readers_;
    std::set<ModelReader*> readerSet_; // for easy destruction
    
    std::string getEnding(const std::string& filename) const;
    
public:
    ModelManager()
      : ResourceManager<Model>()
    {}
    virtual ~ModelManager();
    
    /**
    *   Register ModelReader for use in Manager
    */
    void registerReader(ModelReader* r);
    
    virtual Model* load(const std::string& filename);
};

}

#define ModelMgr tgt::Singleton<tgt::ModelManager>::getRef()

#endif //TGT_MODELMANAGER_H
