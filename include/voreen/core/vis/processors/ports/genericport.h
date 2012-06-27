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

#ifndef VRN_GENERICPORT_H
#define VRN_GENERICPORT_H

#include "voreen/core/vis/processors/ports/port.h"

namespace voreen {

/**
 * @brief Template port class to store points to type T.
 *
 * The data is always stored in the outport, inports fetch data from connected outports.
 */
template<typename T>
class GenericPort : public Port {
public:
    explicit GenericPort(PortDirection direction, const std::string& name, bool allowMultipleConnections = false,
                         Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual ~GenericPort();

    virtual void setData(T* data);

    virtual T* getData() const;

    /// Returns whether GenericPort#getData() returns 0 or not (therefor indicating if there is any data at the port)
    virtual bool hasData() const;

    virtual std::vector<T*> getAllData() const;

    std::vector< GenericPort<T>* > getConnected() const;

    /**
     * Returns true, if the port is connected and
     * contains a data object.
     */
    virtual bool isReady() const;

protected:
    T* portData_;
};

class VolumeCollection;
typedef GenericPort<VolumeCollection> VolumeCollectionPort;

class Geometry;
typedef GenericPort<Geometry> GeometryPort;

// ---------------------------------------- implementation ----------------------------------------

template <typename T>
GenericPort<T>::GenericPort(PortDirection direction, const std::string& name, bool allowMultipleConnections,
                     Processor::InvalidationLevel invalidationLevel)
    : Port(name, direction, allowMultipleConnections, invalidationLevel)
    , portData_(0)
{}

template <typename T>
GenericPort<T>::~GenericPort() {}

template <typename T>
void GenericPort<T>::setData(T* data) {
    tgtAssert(isOutport(), "called setData on inport!");

    if (portData_ != data) {
        portData_ = data;
        invalidate();
    }
}

template <typename T>
T* GenericPort<T>::getData() const {
    if (isOutport())
        return portData_;
    else {
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            if (!connectedPorts_[i]->isOutport())
                continue;

            GenericPort<T>* p = static_cast< GenericPort<T>* >(connectedPorts_[i]);
            if (p)
                return p->getData();
        }
    }
    return 0;
}

template <typename T>
bool GenericPort<T>::hasData() const {
    return (getData() != 0);
}

template <typename T>
std::vector<T*> GenericPort<T>::getAllData() const {
    if (isOutport()) {
        std::vector<T*> allData;
        allData.push_back(portData_);
        return allData;
    }
    else {
        std::vector<T*> allData;
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            if (connectedPorts_[i]->isOutport() == false)
                continue;
            GenericPort<T>* p = static_cast<GenericPort<T>*>(connectedPorts_[i]);
            allData.push_back(p->getData());
        }
        return allData;
    }
}

template <typename T>
std::vector<GenericPort<T>*> GenericPort<T>::getConnected() const {
    std::vector<GenericPort<T>*> ports;
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        GenericPort<T>* p = static_cast<GenericPort<T>*>(connectedPorts_[i]);

        ports.push_back(p);
    }
    return ports;
}

template <typename T>
bool GenericPort<T>::isReady() const {
    if(isOutport())
        return isConnected();
    else
        return (!getConnected().empty() && hasData());
}

} // namespace

#endif // VRN_GENERICPORT_H
