/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/properties/link/dependancylinkevaluators.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/properties/property.h"

namespace voreen {

void DependancyLinkEvaluatorVolumeHandle::eval(Property* src, Property* dst) throw (VoreenException) {
    DependancyLinkEvaluatorBase::eval(src, dst);

    if(static_cast<VolumeHandleProperty*>(src)->get())
        static_cast<VolumeHandleProperty*>(src)->get()->addObserver(this);
}

void DependancyLinkEvaluatorVolumeHandle::propertiesChanged(Property* src, Property* dst) {
    DependancyLinkEvaluatorBase::propertiesChanged(src, dst);

    if(static_cast<VolumeHandleProperty*>(src)->get())
        static_cast<VolumeHandleProperty*>(src)->get()->addObserver(this);
}

void DependancyLinkEvaluatorVolumeHandle::volumeHandleDelete(const VolumeHandle* source) {
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");

    //We create this property just to compare its serialization against the keys:
    VolumeHandleProperty vhp("test", "Test", const_cast<VolumeHandle*>(source));
    std::string compare = serializeProperty(&vhp);

    for (size_t i = 0; i < keys_.size(); ++i) {
        if(keys_[i] == compare) {
            keys_.erase(keys_.begin() + i);
            values_.erase(values_.begin() + i);
            break;
        }
    }
    tgtAssert(keys_.size() == values_.size(), "keys and values have different number of entries");
}

void DependancyLinkEvaluatorVolumeHandle::volumeChange(const VolumeHandle*) {}

} // namespace
