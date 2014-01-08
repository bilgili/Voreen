/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/datastructures/volume/volumedecorator.h"

#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volumehash.h"
#include "voreen/core/utils/hashing.h"

using std::string;
using tgt::vec3;
using tgt::mat4;

namespace voreen {

VolumeDecoratorIdentity::VolumeDecoratorIdentity(const VolumeBase* vhb) : base_(vhb) {
    tgtAssert(vhb, "null pointer passed as VolumeBase");
    vhb->addObserver(this);
}

//-------------------------------------------------------------------------------------------------

VolumeDecoratorReplace::VolumeDecoratorReplace(const VolumeBase* vhb,
    const std::string& key, MetaDataBase* value, bool keepDerivedData)
    : VolumeDecoratorIdentity(vhb)
    , key_(key)
    , value_(value)
{
    tgtAssert(key != "", "empty key passed");
    tgtAssert(value, "null pointer passed as value");

    // create volume hash by concatenating hash of underlying volume with the replace item
    if (vhb->hasDerivedData<VolumeHash>()) {
        VolumeHash* newHash = new VolumeHash();
        newHash->setHash(VoreenHash::getHash(vhb->getHash() + "-" + value->toString()));
        addDerivedData(newHash);
    }

    // copy over derived data (TODO: handle other derived data types)
    if (keepDerivedData) {
        if (vhb->hasDerivedData<VolumeHistogramIntensity>())
            addDerivedData(new VolumeHistogramIntensity(*vhb->getDerivedData<VolumeHistogramIntensity>()));

        if (vhb->hasDerivedData<VolumeMinMax>())
            addDerivedData(new VolumeMinMax(*vhb->getDerivedData<VolumeMinMax>()));
    }

}

std::vector<std::string> VolumeDecoratorReplace::getMetaDataKeys() const {
    std::vector<std::string> keys = base_->getMetaDataKeys();

    if(!base_->hasMetaData(key_))
        keys.push_back(key_);

    return keys;
}

const MetaDataBase* VolumeDecoratorReplace::getMetaData(const std::string& key) const {
    if(key == key_)
        return value_;
    else
        return base_->getMetaData(key);
}

bool VolumeDecoratorReplace::hasMetaData(const std::string& key) const {
    if(key == key_)
        return true;
    else
        return base_->hasMetaData(key);
}

MetaDataBase* VolumeDecoratorReplace::getValue() const {
    return value_;
}

void VolumeDecoratorReplace::setValue(MetaDataBase* value) {
    delete value_;
    value_ = value;
}

} // namespace
