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

#ifndef VRN_GENERICPORT_H
#define VRN_GENERICPORT_H

#include "voreen/core/ports/port.h"
#include "voreen/core/datastructures/imagesequence.h"
#include "voreen/core/datastructures/volume/volumelist.h"

#include <sstream>

namespace tgt {
    class Texture;
}

namespace voreen {

/**
 * @brief Template port class to store points to type T.
 *
 * The data is always stored in the outport, inports fetch data from connected outports.
 */
template<typename T>
class GenericPort : public Port {
public:
    GenericPort(PortDirection direction, const std::string& id, const std::string& guiName = "", bool allowMultipleConnections = false,
                         Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);
    virtual ~GenericPort();

    virtual void forwardData() const;

    /**
     * Set data stored in this port. Can only be called on outports.
     * @param takeOwnership If true, the data will be deleted by the port.
     */
    virtual void setData(const T* data, bool takeOwnership = true);

    /// Return the data stored in this port (if this is an outport) or the data of the first connected outport (if this is an inport).
    virtual const T* getData() const;
    /// Returns a non-const pointer to the data. Can only be used on outports.
    virtual T* getWritableData();

    /**
     * Returns whether GenericPort::getData() returns 0 or not,
     * therefore indicating if there is any data at the port.
     */
    virtual bool hasData() const;

    /// Return the data stored in this port (if this is an outport) or the data of all the connected outports (if this is an inport).
    virtual std::vector<const T*> getAllData() const;

    std::vector<const GenericPort<T>* > getConnected() const;

    /**
     * Returns true, if the port is connected and
     * contains a data object.
     */
    virtual bool isReady() const;

    virtual void clear();
protected:
    const T* portData_;
    bool ownsData_;
};

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API GenericPort<VolumeList>;
#endif

class VRN_CORE_API VolumeListPort : public GenericPort<VolumeList> {

public:

    VolumeListPort(PortDirection direction, const std::string& id, const std::string& guiName = "", bool allowMultipleConnections = false, Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT)
    : GenericPort<VolumeList>(direction, id, guiName, allowMultipleConnections, invalidationLevel) {}

    virtual std::string getClassName() const {return "VolumeListPort";}
    virtual Port* create(PortDirection direction, const std::string& id, const std::string& guiName = "") const {return new VolumeListPort(direction,id,guiName);}
    virtual tgt::col3 getColorHint() const {
        return tgt::col3(255, 0, 255);
    }
    virtual std::string getContentDescription() const {
        std::stringstream strstr;
        strstr << Port::getContentDescription();
        if(hasData())
            strstr << std::endl << "Size of List: " << getData()->size();
        return strstr.str();
    }
    virtual std::string getContentDescriptionHTML() const {
        std::stringstream strstr;
        strstr << Port::getContentDescriptionHTML();
        if(hasData())
            strstr << "<br>" << "Size of List: " << getData()->size();
        return strstr.str();
    }
};

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API GenericPort<ImageSequence>;
#endif

class VRN_CORE_API ImageSequencePort : public GenericPort<ImageSequence> {

public:

    ImageSequencePort(PortDirection direction, const std::string& id, const std::string& guiName = "", bool allowMultipleConnections = false, Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT)
    : GenericPort<ImageSequence>(direction, id, guiName, allowMultipleConnections, invalidationLevel) {}

    virtual Port* create(PortDirection direction, const std::string& id, const std::string& guiName = "") const {return new ImageSequencePort(direction,id,guiName);}
    virtual std::string getClassName() const { return "ImageSequencePort"; }

    virtual std::string getContentDescription() const {
        std::stringstream strstr;
        strstr << Port::getContentDescription();
        if(hasData())
            strstr << std::endl << "Size of List: " << getData()->size();
        return strstr.str();
    }

    virtual std::string getContentDescriptionHTML() const {
        std::stringstream strstr;
        strstr << Port::getContentDescriptionHTML();
        if(hasData())
            strstr << "<br>" << "Size of List: " << getData()->size();
        return strstr.str();
    }

};


// ---------------------------------------- implementation ----------------------------------------

template <typename T>
GenericPort<T>::GenericPort(PortDirection direction, const std::string& id, const std::string& guiName, bool allowMultipleConnections,
                     Processor::InvalidationLevel invalidationLevel)
    : Port(direction, id, guiName, allowMultipleConnections, invalidationLevel)
    , portData_(0)
    , ownsData_(false)
{}

template <typename T>
GenericPort<T>::~GenericPort() {
    if(ownsData_)
        delete portData_;
}

template <typename T>
void GenericPort<T>::forwardData() const{
    for(std::vector<Port*>::const_iterator it = forwardPorts_.begin(); it != forwardPorts_.end(); ++it){
        dynamic_cast<GenericPort<T>*>(*it)->setData(getData(), false);
    }
}

template <typename T>
void GenericPort<T>::setData(const T* data, bool takeOwnership) {
    tgtAssert(isOutport(), "called setData on inport!");

    //delete previous data:
    if(ownsData_)
        delete portData_;

    portData_ = data;
    ownsData_ = takeOwnership;

    invalidatePort();
}

template <typename T>
const T* GenericPort<T>::getData() const {
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
T* GenericPort<T>::getWritableData() {
    if (isOutport())
        return const_cast<T*>(portData_);
    else {
        tgtAssert(false, "Tried to get non-const data from inport!");
        return 0;
    }
}

template <typename T>
bool GenericPort<T>::hasData() const {
    return (getData() != 0);
}

template <typename T>
std::vector<const T*> GenericPort<T>::getAllData() const {
    std::vector<const T*> allData;

    if (isOutport())
        allData.push_back(portData_);
    else {
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            if (connectedPorts_[i]->isOutport() == false)
                continue;
            GenericPort<T>* p = static_cast<GenericPort<T>*>(connectedPorts_[i]);
            if(p->getData())
                allData.push_back(p->getData());
        }
    }

    return allData;
}

template <typename T>
std::vector<const GenericPort<T>*> GenericPort<T>::getConnected() const {
    std::vector<const GenericPort<T>*> ports;
    for (size_t i = 0; i < connectedPorts_.size(); ++i) {
        GenericPort<T>* p = static_cast<GenericPort<T>*>(connectedPorts_[i]);

        ports.push_back(p);
    }
    return ports;
}

template <typename T>
bool GenericPort<T>::isReady() const {
    if (isOutport())
        return isConnected();
    else
        return (!getConnected().empty() && hasData() && checkConditions());
}

template <typename T>
void GenericPort<T>::clear() {
    if (isOutport()) {
        if(portData_) {
            if(ownsData_)
                delete portData_;
            portData_ = 0;
            invalidatePort();
        }
    }
    else
        LERROR("clear() called on inport");
}

} // namespace

#endif // VRN_GENERICPORT_H
