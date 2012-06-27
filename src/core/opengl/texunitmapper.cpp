/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/opengl/texunitmapper.h"
#include "tgt/gpucapabilities.h"
#include "tgt/singleton.h"

namespace voreen {

TexUnitMapper::TexUnitMapper() {
    registeredUnits_ = new std::map<Identifier, uint>();
    remap();
}

TexUnitMapper::TexUnitMapper(const Identifier& ident) {
    TexUnitMapper();
    uint mappedTo = getFreeTexUnitInt();
    texUnits_.insert(mappedTo);
    registeredUnits_->insert(std::pair<Identifier, uint>(ident,mappedTo));
}

TexUnitMapper::TexUnitMapper(const std::vector<Identifier>& idents) {
    TexUnitMapper();
    for (uint i = 0; i < idents.size(); ++i) {
        uint mappedTo = getFreeTexUnitInt();
        texUnits_.insert(mappedTo);
        registeredUnits_->insert(std::pair<Identifier, uint>(idents.at(i), mappedTo));
    }
}

TexUnitMapper::~TexUnitMapper() {
    registeredUnits_->clear();
}

void TexUnitMapper::setTexUnitFree(uint texUnit) {
    texUnits_.erase(texUnits_.find(texUnit));
    remap();
}

void TexUnitMapper::remap() {
    texUnits_.clear();
    std::map<Identifier, uint>::iterator it;
    if (registeredUnits_->size() > 0) {
        for (it = registeredUnits_->begin(); it != registeredUnits_->end(); ++it) {
            int findID = getFreeTexUnitInt();
            it->second = findID;
            texUnits_.insert(findID);
        }
    }
}

void TexUnitMapper::removeTexUnit(const Identifier& ident) {
    std::map<Identifier, uint>::iterator found = registeredUnits_->find(ident);
    if (registeredUnits_->end() != found) {
        registeredUnits_->erase(found);
        std::set<uint>::iterator foundTexUnit = texUnits_.find(found->second);
        if (texUnits_.end() != foundTexUnit)
            texUnits_.erase(foundTexUnit);
    }
    remap();
}

bool TexUnitMapper::addTexUnit(const Identifier& ident) {
    uint mappedTo = getFreeTexUnitInt();
    texUnits_.insert(mappedTo);
    registeredUnits_->insert(std::make_pair(ident, mappedTo));
    remap();
    return true;
};

void TexUnitMapper::registerUnits(const std::vector<Identifier>& idents) {
    registeredUnits_->clear();
    remap();
    for (size_t i = 0; i < idents.size(); ++i) {
        uint mappedTo = getFreeTexUnitInt();
        texUnits_.insert(mappedTo);
        registeredUnits_->insert(std::make_pair(idents.at(i), mappedTo));
    }
}

uint TexUnitMapper::getTexUnit(const Identifier& ident) {
    std::map<Identifier, uint>::iterator found = registeredUnits_->find(ident);
    if (found == registeredUnits_->end()) {
        throw texUnit_Exception("texture unit '" + ident.getName() + "' seems to be unregistered");
    } else {
        if (found->second < (uint)GpuCaps.getNumTextureUnits())
            return found->second;
        else
            throw texUnit_Exception("not enough texture units available");
    }
}

//FIXME: This is a disaster waiting to happen! either use tex unit number "i" OR "GL_TEXTURE0 +
//       i", not both!!! joerg
uint TexUnitMapper::getGLTexUnit(const Identifier& ident) {
    std::map<Identifier, uint>::iterator found = registeredUnits_->find(ident);
    if (found == registeredUnits_->end())
        throw texUnit_Exception("texture unit '" + ident.getName() + "' seems to be unregistered");
    else {
        int foundID = found->second;
        if (foundID < GpuCaps.getNumTextureUnits())
            return GL_TEXTURE0 + found->second;
        else
            throw texUnit_Exception("not enough texture units available");
    }
}
uint TexUnitMapper::getFreeTexUnit() {
    uint mappedTo = getFreeTexUnitInt();
    if (mappedTo < (uint)GpuCaps.getNumTextureUnits()) {
        return mappedTo;
    }
    else
        throw texUnit_Exception("no further free texture units available");
}

uint TexUnitMapper::getGLTexUnitFromInt(uint texUnit) {
    return GL_TEXTURE0 + texUnit;
}

uint TexUnitMapper::getFreeTexUnitInt( ) {
    uint i = 0;
    while (texUnits_.find(i) != texUnits_.end()) {
        ++i;
    }
    return i;
}

std::map<Identifier, uint>* TexUnitMapper::getCurrentSystem() {
    return registeredUnits_;
}

} // namespace
