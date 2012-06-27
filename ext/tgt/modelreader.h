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

#ifndef TGT_MODELREADER_H
#define TGT_MODELREADER_H

#include <string>

#include "tgt/config.h"
#include "tgt/model.h"

namespace tgt {

class ModelReader {
protected:
    static const std::string loggerCat_;
    
    std::vector<std::string> extensions_;
    std::string name_;

public:
    ModelReader() {};
    virtual ~ModelReader() {}
    
    virtual const std::vector<std::string>& getEndings() const { return extensions_; }
    virtual std::string getName() const { return name_; }

    virtual Model* loadModel(const std::string& filename ) = 0;
};

}  // namespace tgt

#endif // TGT_MODELREADER_H
