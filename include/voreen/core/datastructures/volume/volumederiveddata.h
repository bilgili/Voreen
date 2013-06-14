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

#ifndef VRN_VOLUMEDERIVEDDATA_H
#define VRN_VOLUMEDERIVEDDATA_H

#include "voreen/core/voreenobject.h"
#include "tgt/assert.h"

#include <boost/thread.hpp>

namespace voreen {

class VolumeBase;

/**
 * Marker interface for data that is derived from a volume dataset,
 * such as histograms or preview images.
 *
 * @note each concrete subclass must provide a default constructor
 * @note each concrete subclass must be handled by a SerializableFactory
 *
 * @see DerivedDataFactory
 * @see VoreenModule::registerSerializerFactory
 */
class VRN_CORE_API VolumeDerivedData : public VoreenSerializableObject {
public:
    VolumeDerivedData();
    virtual ~VolumeDerivedData() {}

    /**
     * Virtual constructor.
     * Check for boo
     *
     * @param handle the volume to derive from
     * @return the constructed derived data object, or null if construction is not possible
     */
    virtual VolumeDerivedData* createFrom(const VolumeBase* handle) const = 0;

    virtual void serialize(XmlSerializer& s) const = 0;
    virtual void deserialize(XmlDeserializer& s) = 0;
};

/// Thread calculating a VolumeDerivedData, started by a Volume when calling getDerivedDataThreaded<T>().
class VolumeDerivedDataThreadBase {
public:
    VolumeDerivedDataThreadBase() : result_(0), volume_(0) {}

    virtual ~VolumeDerivedDataThreadBase() {
        if(isRunning()) {
            interrupt();
            join();
        }
    }

    void startThread(const VolumeBase* vb) {
        if(isRunning()) {
            tgtAssert(false, "Thread is already running!\n");
            return;
        }

        volume_ = vb;
        workerThread_ = boost::thread(&VolumeDerivedDataThreadBase::run, this);
    }

    void join() {
        workerThread_.join();
    }

    void interrupt() {
        workerThread_.interrupt();
    }

    bool isRunning() {
        if(!workerThread_.timed_join(boost::posix_time::seconds(0)))
            return true;
        else
            return false;
    }

    VolumeDerivedData* getResult() {
        resultMutex_.lock();
        VolumeDerivedData* tmp = result_;
        resultMutex_.unlock();
        return tmp;
    }

protected:
    virtual void run() = 0;

    VolumeDerivedData* result_;
    boost::mutex resultMutex_;

    const VolumeBase* volume_;

    boost::thread workerThread_;
};


} // namespace voreen

#endif
