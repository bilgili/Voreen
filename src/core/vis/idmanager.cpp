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

#include "voreen/core/vis/idmanager.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/processor.h"

#include "tgt/logmanager.h"

namespace voreen {

IDManager::IDManager() :
      rt_(0)
       , currentID_(0,0,1)
{
}

void IDManager::initializeTarget() {
    if (!rt_)
        return;

    rt_->initialize(GL_RGB);
}

tgt::col3 IDManager::registerObject(void* obj) {
    increaseID();
    colorToID_[currentID_] = obj;
    IDToColor_[obj] = currentID_;
    return currentID_;
}

void IDManager::deregisterObject(void* obj) {
    tgt::col3 col = getColorFromObject(obj);
    colorToID_.erase(col);
    IDToColor_.erase(obj);
}

void IDManager::clearRegisteredObjects() {
    colorToID_.clear();
    IDToColor_.clear();
    currentID_ = tgt::col3(0,0,1);
}

tgt::col3 IDManager::getColorFromObject(void* obj) {
    if (obj == 0)
        return tgt::col3(0,0,0);

    if (isRegistered(obj))
        return IDToColor_[obj];
    else
        return tgt::col3(0,0,0);
}

void IDManager::setGLColor(void* obj) {
    tgt::col3 col = getColorFromObject(obj);
    glColor3ub(col.x, col.y, col.z);
}

void* IDManager::getObjectFromColor(tgt::col3 col) {
    if (col == tgt::col3(0,0,0))
        return 0;

    if (isRegistered(col))
        return colorToID_[col];
    else
        return 0;
}

void* IDManager::getObjectAtPos(tgt::ivec2 pos) {
    return getObjectFromColor(getColorAtPos(pos));
}

tgt::col3 IDManager::getColorAtPos(tgt::ivec2 pos) {
    rt_->activateTarget();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    tgt::col3 pixels;
    glReadPixels(pos.x, pos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixels);
    return pixels;
}

bool IDManager::isHit(tgt::ivec2 pos, void* obj) {
    return (getObjectAtPos(pos) == obj);
}

void IDManager::activateTarget(std::string debugLabel) {
    if (rt_) {
        rt_->activateTarget();
        rt_->increaseNumUpdates();
        rt_->setDebugLabel("ID target" + (debugLabel.empty() ? "" : " (" + debugLabel + ")"));
    }
    else
        LERRORC("voreen.idmanager", "No RenderTarget set!");
}

void IDManager::clearTarget() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void IDManager::setRenderTarget(RenderTarget* rt) {
    rt_ = rt;
}

RenderTarget* IDManager::getRenderTarget() {
    return rt_;
}

void IDManager::increaseID() {
    if (currentID_.b == 255) {
        currentID_.b = 0;
        if (currentID_.g == 255) {
            currentID_.g = 0;
            if (currentID_.r == 255) {
                LERRORC("voreen.idmanager", "Out of ids...");
            }
            else
                currentID_.r++;

        }
        else
            currentID_.g++;
    }
    else
        currentID_.b++;
}

bool IDManager::isRegistered(void* obj) {
    if(IDToColor_.find(obj) != IDToColor_.end())
        return true;
    else
        return false;
}

bool IDManager::isRegistered(tgt::col3 col) {
    if (colorToID_.find(col) != colorToID_.end())
        return true;
    else
        return false;
}

} // namespace voreen
