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

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/processor.h"

namespace voreen {

IDManager::IDManagerContents::IDManagerContents() {
    currentID_B_ = 10;
    currentID_G_ = 10;
    currentID_R_ = 10;
    textureTarget_ = -1;
    isTC_ = false;
    newRenderingPass_ = false;
}

IDManager::IDManagerContents * IDManager::content_ = 0;

IDManager::IDManager() {
    if (content_ == 0)
        content_ = new IDManager::IDManagerContents();
}

void IDManager::setTC(TextureContainer *tc) {
    content_->tc_ = tc;
    content_->isTC_ = tc ? true : false;
}

void IDManager::signalizeNewRenderingPass() {
    content_->newRenderingPass_ = true;
}

bool IDManager::isClicked(Identifier ident, int x, int y) {
    tgt::vec3 myID = getIDatPos(x,y);
    int validI = -1;
    for (size_t i = 0; i < content_->picks_.size(); ++i) {
        double delta = tgt::distance(myID, content_->picks_[i].vec());
        if (delta <= (1.f/255.f)) {
            validI = i;
            i = content_->picks_.size();
        }
    }
    if (validI == -1)
        return false;
    if (content_->picks_[validI].name_ == ident.getName())
        return true;
    else
        return false;
}

tgt::vec3 IDManager::getIDatPos(int x, int y) const {
    if (content_->isTC_) {
        tgt::vec3 returnedID;

        float* buffer = content_->tc_->getTargetAsFloats(content_->textureTarget_, x, y);

        returnedID.x = buffer[0];
        returnedID.y = buffer[1];
        returnedID.z = buffer[2];
        delete[] buffer;
        return returnedID;
    }
    else
        return tgt::vec3::zero;
}

std::string IDManager::getNameAtPos(int x, int y) const {
    if ( (content_->isTC_ == false) )
        return "";

    int flippedY = content_->tc_->getSize().y - y;
    flippedY = (flippedY >= 0) ? flippedY : 0;

    tgt::vec3 myID = getIDatPos(x, flippedY);
    for (size_t i = 0; i < content_->picks_.size(); ++i) {
        double delta = tgt::distance(myID, content_->picks_[i].vec());
        if (delta <= (1.0f / 255.0f))
            return content_->picks_[i].name_;
    }
    return "";
}

void IDManager::addNewPickObj(Identifier identIN) {
    bool isOkay = true;
    content_->currentID_B_ += 10;
    if (content_->currentID_B_ >= 255) {
        content_->currentID_B_ = 1;
        content_->currentID_G_ += 10;
    }
    if (content_->currentID_G_ >= 255) {
        content_->currentID_G_ = 1;
        content_->currentID_R_ += 10;
    }
    if (content_->currentID_R_ >= 255) {
        isOkay = false;
        //logger exception - no further picking objects
    }
    tgt::vec3 returnedID = tgt::vec3(content_->currentID_R_/255.f, content_->currentID_G_/255.f, content_->currentID_B_/255.f);
    IDF ident = IDF(identIN.getName());
    ident.x_ = returnedID.x;
    ident.y_ = returnedID.y;
    ident.z_ = returnedID.z;
    content_->picks_.push_back(ident);
}


//clearBuffers
void IDManager::clearTextureTarget() {
    LGL_ERROR;
    if (content_->isTC_ && content_->textureTarget_ > -1) {
        content_->oldRT_ = content_->tc_->getActiveTarget();
        LGL_ERROR;
        content_->tc_->setActiveTarget(content_->textureTarget_, "IDManager::initNewRenderering()");
        LGL_ERROR;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        LGL_ERROR;
        content_->tc_->setActiveTarget(content_->oldRT_);
    }
    else {
        //FIXME
        //RPTMERGE: allocTarget shouldnt be called, we somehow have to get a portMapping here
        //content_->renderBufferID_ = content_->tc_->allocTarget(Processor::ttId_, "IDManager::initNewRendering");
        //content_->tc_->setPersistent(content_->renderBufferID_, true);
    }
}

//use this function carefully !!!
// TODO: more comments why to use it carefully? (ab)
void IDManager::clearIDs() {
    content_->currentID_B_ = 0;
    content_->currentID_G_ = 0;
    content_->currentID_R_ = 0;
}

// enable the correct buffers
void IDManager::startBufferRendering(Identifier identIN) {
    int foundIDF = -1;
    for (size_t i=0; i<content_->picks_.size(); ++i) {
        if (content_->picks_[i].name_ == identIN) {
            foundIDF = i;
            break;
        }
    }
    if (content_->newRenderingPass_) {
        content_->newRenderingPass_ = false;
        clearTextureTarget();
    }
    if (content_->isTC_) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        content_->oldRT_ = content_->tc_->getActiveTarget();
        content_->tc_->setActiveTarget(content_->textureTarget_);
        glDisable(GL_LIGHTING);
        glColor3f(content_->picks_[foundIDF].x_,
                  content_->picks_[foundIDF].y_,
                  content_->picks_[foundIDF].z_);
    }
}

// disable the correct buffers
void IDManager::stopBufferRendering() {
   if (content_->isTC_) {
       content_->tc_->setActiveTarget(content_->oldRT_);
       glPopAttrib();
   }
}

} // namespace voreen
