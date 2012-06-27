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

#ifndef VRN_PORTMAPPING_H
#define VRN_PORTMAPPING_H

#include "voreen/core/vis/identifier.h"
#include "voreen/core/vis/exception.h"
#include "voreen/core/vis/processors/port.h"

#include <map>

namespace voreen {

class PortDataVolume;
class LocalPortMapping;
class VolumeHandle;
class Processor;

class PortMapping {
public:
    PortMapping(std::map<Port*,std::vector<PortData*> > portMapping_);

    LocalPortMapping* createLocalPortMapping(Processor* processor);

    int getTarget(Processor* processor, const Identifier& ident, int pos=0) throw (VoreenException);

    std::vector<int> getAllTargets(Processor* processor, const Identifier& ident) throw (VoreenException);

    int getGeometryNumber(Processor* processor, const Identifier& ident, int pos=0) throw (VoreenException);
    std::vector<int> getAllGeometryNumbers(Processor* processor, const Identifier& ident) throw (VoreenException);

    /**
     * Generic method for obtaining data from PortDataGeneric objects:
     * no further classes are needed for port mapping anymore.
     * Was added in order to pass pointers through the network
     */
    template<class T>
    T getGenericData(Processor* processor, const Identifier& ident, int pos = 0) throw (VoreenException) {
        Port* p = getPortAndCheck(processor, ident);
        std::vector<PortData*> portData = portMap_[p];

        if (pos >= static_cast<int>(portData.size()))
            throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

        PortDataGeneric<T>* pdGeneric = dynamic_cast<PortDataGeneric<T>*>(portData[pos]);

        if (pdGeneric == 0)
            throw VoreenException("The data mapped to this is port is from a PortDataGeneric<T> object.");

        return pdGeneric->getData();
    }

    /**
     * Generic method for obtaining all data from PortDataGeneric objects:
     * no further classes are needed for port mapping anymore.
     * Was added in order to pass pointers through the network
     */
    template<class T>
    std::vector<T> getAllGenericData(Processor* processor, const Identifier& ident) throw (VoreenException) {
        Port* p = getPortAndCheck(processor, ident);
        std::vector<PortData*> portData = portMap_[p];

        if (portData.size() < 1)
            throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

        std::vector<T> data;
        PortDataGeneric<T>* pdGeneric = 0;

        for (size_t i = 0; i < portData.size(); i++) {
            pdGeneric = dynamic_cast<PortDataGeneric<T>*>(portData[i]);

            if (pdGeneric == 0)
                throw VoreenException("The data mapped to this is port is from a PortDataGeneric<T> object.");

            data.push_back(pdGeneric->getData());
        }

        return data;
    }

    PortDataCoProcessor* getCoProcessorData(Processor* processor, const Identifier& ident, int pos=0)
        throw (VoreenException);

    std::vector<PortDataCoProcessor*> getAllCoProcessorData(Processor* processor, const Identifier& ident)
        throw (VoreenException);

protected:
    Port* getPortAndCheck(Processor* processor, const Identifier& ident) throw (VoreenException);

    std::map<Port*,std::vector<PortData*> > portMap_;
};

//---------------------------------------------------------------------------

class LocalPortMapping {
public:
    LocalPortMapping(PortMapping* portMapping, Processor* processor);

    int getTarget(const Identifier& ident,int pos=0) throw (VoreenException);

    std::vector<int> getAllTargets(const Identifier& ident) throw (VoreenException);

    int getGeometryNumber(const Identifier& ident, int pos=0) throw (VoreenException);
    std::vector<int> getAllGeometryNumbers(const Identifier& ident) throw (VoreenException);

    template<class T>
    T getGenericData(const Identifier& ident, int pos = 0) throw (VoreenException) {
        return portMapping_->getGenericData<T>(processor_, ident, pos);
    }

    template<class T>
    std::vector<T> getAllGenericData(const Identifier& ident) throw (VoreenException) {
        return portMapping_->getAllGenericData<T>(processor_, ident);
    }

    /**
     * Method for convenience: it hides the mapping of VolumeHandle** on the outports
     * of VolumeSelectionProcessors by already dereferencing the mapped data and returning
     * a VolumeHandle.
     */
    VolumeHandle* getVolumeHandle(const Identifier& ident, int pos = 0) throw (VoreenException) {
        VolumeHandle** handleAddr = getGenericData<VolumeHandle**>(ident, pos);
        if (( handleAddr != 0) && (*handleAddr != 0) )
            return *handleAddr;
        else
            return 0;
    }

    /**
     * Same as method getVolumeHandle, except, that all pointers are returned and
     * packed into a std::vector
     */
    std::vector<VolumeHandle*> getAllGenericData(const Identifier& ident) throw (VoreenException) {
        const std::vector<VolumeHandle**>& handleAddr = getAllGenericData<VolumeHandle**>(ident);
        std::vector<VolumeHandle*> handles;
        for (size_t i = 0; i < handleAddr.size(); i++) {
            if (handleAddr[i] != 0 && *(handleAddr[i]) != 0)
                handles.push_back(*(handleAddr[i]));
            else
                handles.push_back(0);
        }
        return handles;
    }

    PortDataCoProcessor* getCoProcessorData(const Identifier& ident,int pos=0) throw (VoreenException);

    std::vector<PortDataCoProcessor*> getAllCoProcessorData(const Identifier& ident) throw (VoreenException);

    LocalPortMapping* createLocalPortMapping(Processor* processor) throw (VoreenException);

protected:
    PortMapping* portMapping_;
    Processor* processor_;
};

} // namespace voreen

#endif // VRN_PORTMAPPING_H
