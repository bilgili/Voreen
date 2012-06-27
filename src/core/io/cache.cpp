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

// Setup the general ability of caching
//
bool CacheBase::cachingEnabled_(true);

CacheBase::CacheBase(const std::string& cachedObjectsClassName, const std::type_info& assignedPortType)
    : assignedPortType_(assignedPortType),
    cachedObjectsClassName_(cachedObjectsClassName),
    isEnabled_(true)
{
}

const std::type_info& CacheBase::getAssignedPortType() const {
    return assignedPortType_;
}

std::vector<Port*> CacheBase::getCacheConcernedOutports(Processor* const processor) const {
    if (processor == 0)
        return std::vector<Port*>();

    std::vector<Port*> concernedPorts;
    const std::vector<Port*>& outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (typeid(outports[i]) == assignedPortType_)
            concernedPorts.push_back(outports[i]);
    }
    return concernedPorts;
}

const std::string& CacheBase::getCachedObjectsClassName() const  {
    return cachedObjectsClassName_;
}

std::string CacheBase::getProcessorsInportConfig(const Processor* processor) const
{
    if (processor == 0)
        return "";

    std::string inputConfig("");
    const std::vector<Port*>& inports = processor->getInports();
    for (size_t i = 0; i < inports.size(); ++i) {
        inputConfig += portContentToString(inports[i]);
        if (i < (inports.size() - 1))
            inputConfig += ", ";
    }
    return inputConfig;
}

bool CacheBase::isCompatible(voreen::Processor* const processor) const {
    const std::vector<Port*>& outports = processor->getOutports();
    for (size_t i = 0; i < outports.size(); ++i) {
        if (typeid(outports[i]) == assignedPortType_)
            return true;
    }
    return false;
}

bool CacheBase::isEnabled() const {
    return (isEnabled_ && cachingEnabled_);
}

void CacheBase::setEnabled(const bool enable) {
    isEnabled_ = enable;
}

// ============================================================================



template class Cache<VolumeHandle*>;
//template class Cache<int>;

}   // namespace
