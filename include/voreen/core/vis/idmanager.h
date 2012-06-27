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

#ifndef VRN_IDMANAGER_H
#define VRN_IDMANAGER_H

#include "tgt/vector.h"
#include "voreen/core/vis/rendertarget.h"

namespace voreen {

struct colComp {
    bool operator()(tgt::col3 c1, tgt::col3 c2) const {
        if (c1.r < c2.r)
            return true;
        else if (c1.r == c2.r) {
            if (c1.g < c2.g)
                return true;
            else if (c1.g == c2.g) {
                if (c1.b < c2.b)
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }
};

/**
 * IDManager class for picking
 */
class IDManager {
public:
    IDManager();
    void initializeTarget();

    tgt::col3 registerObject(void* obj);
    void deregisterObject(void* obj);
    void clearRegisteredObjects();
    tgt::col3 getColorFromObject(void* obj);
    void setGLColor(void* obj);
    void* getObjectFromColor(tgt::col3 col);

    void* getObjectAtPos(tgt::ivec2 pos);
    tgt::col3 getColorAtPos(tgt::ivec2 pos);

    bool isHit(tgt::ivec2 pos, void* obj);

    void activateTarget(std::string debugLabel = "");
    void clearTarget();
    void setRenderTarget(RenderTarget* rt);
    RenderTarget* getRenderTarget();

    bool isRegistered(void* obj);
    bool isRegistered(tgt::col3 col);
private:
    void increaseID();

    std::map<tgt::col3, void*, colComp> colorToID_;
    std::map<void*, tgt::col3> IDToColor_;
    RenderTarget* rt_;
    tgt::col3 currentID_;
};


} //namespace voreen

#endif
