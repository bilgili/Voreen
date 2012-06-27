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

#include "voreen/core/volume/volumecontainer.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "tgt/vector.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/volume/volume.h"

#ifndef VRN_NO_OPENGL
    #include "voreen/core/volume/volumegl.h"
#endif

namespace voreen {

//------------------------------------------------------------------------------
//  VolumeContainer::Entry
//------------------------------------------------------------------------------

VolumeContainer::Entry::Entry(Volume* volume,
                              const std::string& name,
                              const Modality &modality,
                              float time /*= 0.0*/,
#ifndef VRN_NO_OPENGL
                              bool generateVolumeGL, VolumeGL* volumeGL
#else
                              bool /*generateVolumeGL*/
#endif

)
  : volume_(volume),
#ifndef VRN_NO_OPENGL
    volumeGL_(volumeGL),
#endif
    modality_(modality),
    time_(time),
    name_(name)
{
#ifndef VRN_NO_OPENGL
    if (generateVolumeGL && !volumeGL_)
        volumeGL_ = new VolumeGL(volume);
#endif
}

void VolumeContainer::Entry::destroy() {
    delete volume_;

#ifndef VRN_NO_OPENGL
    delete volumeGL_;
#endif
}

//------------------------------------------------------------------------------
//  VolumeContainer
//------------------------------------------------------------------------------

/*
 * constructors and destructor
 */


VolumeContainer::VolumeContainer()
    : loggerStr_("voreen.voreen.VolumeContainer") {}

VolumeContainer::~VolumeContainer() {
    // release memory
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
        iter->second.destroy();
}

 /*
  * methods
  */

void VolumeContainer::add(Volume* volume,
                          const std::string& filename,
                          const Modality& modality,
                          float time,
                          bool generateVolumeGL
#ifndef VRN_NO_OPENGL
                         , VolumeGL* volumeGL
#endif
) {

    if (time < 0.0) {
        if (!entries_.empty()) {
            // check whether we have here a new max element or just a new modality for the max element
            Entries::reverse_iterator maxIter = entries_.rbegin(); // get an iterator to the last element
            float maxTime = maxIter->first;
            time = maxTime;  // default: new modality for max element
            while (maxIter != entries_.rend() && maxIter->first == maxTime) {
                if (maxIter->second.modality_ == modality) {
                    time = maxTime + 1.f; // modality already there -> new max element
                    break;
                }
                maxIter++;
            }
        }
        else
            time = 0.0; // hence we have a new first element
    }

    // create and insert the element into the container
    Entry entry(volume, filename, modality, time, generateVolumeGL
#ifndef VRN_NO_OPENGL
        , volumeGL
#endif
        );

    entries_.insert(std::make_pair(time, entry));

#ifndef VRN_NO_OPENGL
    // FIXME: deprecated
    volumeDatasets_.push_back(entry.volumeGL_);
#endif // VRN_NO_OPENGL
}

void VolumeContainer::merge(VolumeContainer* vc) {
    // get latest timestamp;
    float latestTimeStamp = -1.f;
    if (entries_.begin() != entries_.end())
        latestTimeStamp = entries_.begin()->first;
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->first > latestTimeStamp)
            latestTimeStamp = iter->first;
    }
    latestTimeStamp += 1.f; // "next free" timestamp
    for (Entries::iterator iter = vc->entries_.begin(); iter != vc->entries_.end(); ++iter) {
        Entry& entry = iter->second;

        // insert in this multimap
        add(entry.volume_, entry.name_, entry.modality_, latestTimeStamp, false
#ifndef VRN_NO_OPENGL
                , entry.volumeGL_
#endif
        );
        latestTimeStamp += 1.f;
#ifndef VRN_NO_OPENGL
        // FIXME: deprecated
        volumeDatasets_.push_back(entry.volumeGL_);
#endif
    }
    vc->clear();
}

void VolumeContainer::remove(const Modality& mod) {
    //FIXME: may cause infinite loop
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
        if (iter->second.modality_ == mod)
            entries_.erase(iter);
}

void VolumeContainer::clear() {
#ifndef VRN_NO_OPENGL
    volumeDatasets_.clear();
#endif
    entries_.clear();
}

void VolumeContainer::deleteVolumes() {
        
    // delete Volumes
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
        iter->second.destroy();
    
    // remove Volumes / VolumeGLs from container
    clear();

}

void VolumeContainer::changeModality(Volume* volume, const Modality& modality) {
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->second.volume_ == volume) {
            iter->second.modality_ = modality;
            break;
        }
    }
}

void VolumeContainer::changeTime(Volume* volume, float time) {
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->second.volume_ == volume) {
            Entry entry = iter->second;
            entries_.erase(iter);
            entries_.insert(std::make_pair(time, entry));
            
            break;
        }
    }
}

/*
 * getIterator
 */

VolumeContainer::Entries::const_iterator VolumeContainer::getIterator(const Modality& modality) const {
    if (entries_.empty())
        return entries_.end();

    if (modality == Modality::MODALITY_ANY)
        return entries_.begin();

    for (Entries::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
        if (iter->second.modality_ == modality)
            return iter;

    return entries_.end();
}

VolumeContainer::Entries::const_iterator VolumeContainer::getIterator(float time, const Modality& modality) const {

    std::pair<Entries::const_iterator, Entries::const_iterator> range
        = entries_.equal_range(time);

    for (Entries::const_iterator iter = range.first; iter != range.second; ++iter) {
        if (iter->second.modality_ == modality || modality == Modality::MODALITY_ANY)
            return iter;
    }

    return entries_.end();
}

/*
 * getVolume
 */

Volume* VolumeContainer::getVolume(const Modality& modality) const {
    Entries::const_iterator iter = getIterator(modality);

    return (iter != entries_.end()) ? iter->second.volume_ : 0;
}

Volume* VolumeContainer::getVolume(float time, const Modality& modality) const {
    Entries::const_iterator iter = getIterator(time, modality);
    
    return (iter != entries_.end()) ? iter->second.volume_ : 0;
}

Volume* VolumeContainer::getVolume(int step, const Modality& modality) const {
    return getVolume(getTime(step), modality);
}

#ifndef VRN_NO_OPENGL

/*
 * getVolumeGL
 */

VolumeGL* VolumeContainer::getVolumeGL(const Modality& modality) const {
    Entries::const_iterator iter = getIterator(modality);

    return (iter != entries_.end()) ? iter->second.volumeGL_ : 0;
}

VolumeGL* VolumeContainer::getVolumeGL(float time, const Modality& modality) const {
    Entries::const_iterator iter = getIterator(time, modality);

    return (iter != entries_.end()) ? iter->second.volumeGL_ : 0;
}

VolumeGL* VolumeContainer::getVolumeGL(int step, const Modality& modality) const {
    return getVolumeGL(getTime(step), modality);
}

VolumeContainer::TextureType3D VolumeContainer::getTextureType() {

    // FIXME: this should be done in the constructor once, but
    // the VolumeContainer is instantiated before OpenGL is initialized
    if (GpuCaps.isNpotSupported()) {
        textureType3D_ = VRN_TEXTURE_3D;
    } else {
        textureType3D_ = VRN_TEXTURE_3D_POWER_OF_TWO_SCALED;
    }

    return textureType3D_;
}

#endif // VRN_NO_OPENGL

int VolumeContainer::getStep(float time) const {
    int count = 0;
    float lasttime = -1.f;

    for (Entries::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->first >= time)
            return count;

        if (iter->first != lasttime) {
            lasttime = iter->first;
            ++count;
        }
    }

    return count;
}

float VolumeContainer::getTime(int step) const {
    int count = -1;
    float lasttime = -1.f;

    for (Entries::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->second.time_ != lasttime) {
            lasttime = iter->first;
            ++count;

            if (count == step)
                return iter->first;
        }
    }

    return -1.f;
}

float VolumeContainer::maxTime() const {
    if (entries_.empty())
        return 0.f;
    else
        return entries_.rbegin()->first;
}

int VolumeContainer::steps() const {
    int count = 0;
    float lasttime = -1.f;

    for (Entries::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        if (iter->first != lasttime) {
            lasttime = iter->first;
            count++;
        }
    }
    return count;
}

void VolumeContainer::setModality(const Modality& modality) {
    for (Entries::iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
       iter->second.modality_ = modality;
}

std::string VolumeContainer::toString() const {
    std::ostringstream s;
    int i = 0;
    for (Entries::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter) {
        s << i << ". modality: '" << (*iter).second.modality_ << "'\ttime: " << (*iter).second.time_
          << "\tname: " << (*iter).second.name_ << "\n";
        i++;
    }

    return s.str();
}

VolumeContainer::Entries VolumeContainer::getEntries() {
    return entries_;
}

} // namespace voreen
