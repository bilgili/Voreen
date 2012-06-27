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

#ifndef VRN_PORTMAPPING_H
#define VRN_PORTMAPPING_H


#include "voreen/core/vis/identifier.h"
#include "voreen/core/vis/processors/processor.h"


namespace voreen {

class Port;
class PortData;
class PortDataCoProcessor;
class PortDataVolume;
//class PortDataTexture;
class LocalPortMapping;
class VolumeHandle;
class Processor;

class PortMapping {
public:
    PortMapping(std::map<Port*,std::vector<PortData*> > portMapping_);
    
    LocalPortMapping* createLocalPortMapping(Processor* processor);
   
    int getTarget(Processor* processor, Identifier ident,int pos=0) throw(std::exception);
    
    std::vector<int> getAllTargets(Processor* processor, Identifier ident) throw(std::exception);
    
	int getGeometryNumber(Processor* processor, Identifier ident, int pos=0) throw(std::exception);
	std::vector<int> getAllGeometryNumbers(Processor* processor, Identifier ident) throw(std::exception);

    /**
     * Generic method for obtaining data from PortDataGeneric objects:
     * no further classes are needed for port mapping anymore.
     * Was added in order to pass pointers through the network
     */
    template<class T>
    T getGenericData(Processor* processor, Identifier ident, int pos = 0) throw( std::exception) {
        Port* p = processor->getPort(ident);
	    if (p==0)
            throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	    std::vector<PortData*> portData = portMap_[p];

	    if (static_cast<int>(portData.size()) < pos)
            throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");
        
	    PortDataGeneric<T>* pdGeneric = dynamic_cast< PortDataGeneric<T>* >(portData.at(pos));

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
    std::vector<T> getAllGenericData(Processor* processor, Identifier ident) throw(std::exception) {
        Port* p = processor->getPort(ident);
	    if (p==0)
            throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	    std::vector<PortData*> portData = portMap_[p];

	    if (portData.size() < 1)
            throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	    std::vector<T> data;
	    PortDataGeneric<T>* pdGeneric = 0;

	    for (size_t i = 0; i < portData.size(); i++) {
		    pdGeneric = dynamic_cast< PortDataGeneric<T>* >(portData.at(i));

		    if (pdGeneric == 0)
                throw VoreenException("The data mapped to this is port is from a PortDataGeneric<T> object.");
    		
            data.push_back( pdGeneric->getData() );
	    }

	    return data;
    }

    PortDataCoProcessor* getCoProcessorData(Processor* processor, Identifier ident, int pos=0) throw(std::exception);
    
    std::vector<PortDataCoProcessor*> getAllCoProcessorData(Processor* processor, Identifier ident) throw(std::exception);
    
protected:
    std::map<Port*,std::vector<PortData*> > portMap_;
};

class LocalPortMapping {
public:
    LocalPortMapping(PortMapping* portMapping, Processor* processor);
    
    int getTarget(Identifier ident,int pos=0) throw(std::exception);
    
    std::vector<int> getAllTargets(Identifier ident) throw(std::exception);
   
	int getGeometryNumber(Identifier ident, int pos=0) throw(std::exception);
	std::vector<int> getAllGeometryNumbers(Identifier ident) throw(std::exception);

    template<class T>
    T getGenericData(Identifier ident, int pos = 0) throw(std::exception) {
        return portMapping_->getGenericData<T>(processor_, ident, pos);
    }

    template<class T>
    std::vector<T> getAllGenericData(Identifier ident) throw(std::exception) {
        return portMapping_->getAllGenericData<T>(processor_, ident);
    }

    /**
     * Method for convenience: it hides the mapping of VolumeHandle** on the outports
     * of VolumeSelectionProcessors by already dereferencing the mapped data and returning
     * a VolumeHandle.
     */
    VolumeHandle* getVolumeHandle(Identifier ident, int pos = 0) throw(std::exception) {
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
    std::vector<VolumeHandle*> getAllGenericData(Identifier ident) throw(std::exception) {
        const std::vector<VolumeHandle**>& handleAddr = getAllGenericData<VolumeHandle**>(ident);
        std::vector<VolumeHandle*> handles;
        for (size_t i = 0; i < handleAddr.size(); i++) {
            if ( (handleAddr[i] != 0) && (*(handleAddr[i]) != 0) )
                handles.push_back(*(handleAddr[i]));
            else
                handles.push_back(0);
        }
        return handles;
    }

    PortDataCoProcessor* getCoProcessorData(Identifier ident,int pos=0) throw(std::exception);
    
    std::vector<PortDataCoProcessor*> getAllCoProcessorData(Identifier ident) throw(std::exception);

	LocalPortMapping* createLocalPortMapping(Processor* processor) throw(std::exception);
    
protected:
    PortMapping* portMapping_;
    Processor* processor_;
};

} // namespace voreen

#endif // VRN_PORTMAPPING_H
