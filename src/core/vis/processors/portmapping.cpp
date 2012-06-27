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

#include "voreen/core/vis/processors/portmapping.h"


#include "voreen/core/vis/processors/processor.h"


namespace voreen {

PortMapping::PortMapping(std::map<Port*,std::vector<PortData*> > portMap) {
	portMap_=portMap;
}

LocalPortMapping* PortMapping::createLocalPortMapping(Processor* processor) {
	return new LocalPortMapping(this, processor);
}

int PortMapping::getTarget(Processor* processor, Identifier ident, int pos) throw (std::exception) {
	Port* p = processor->getPort(ident);
	if (!p)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> targets = portMap_[p];

	if (static_cast<int>(targets.size()) <= pos)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	PortDataTexture* pdt = dynamic_cast<PortDataTexture*>(targets.at(pos));

	if (!pdt)
        throw VoreenException("The data mapped to this is port is not a TextureContainer target.");

	return pdt->getData();
}

std::vector<int> PortMapping::getAllTargets(Processor* processor, Identifier ident) throw(std::exception) {
	Port* p = processor->getPort(ident);
	if (p==0)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> portData = portMap_[p];

	if (portData.size() < 1)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	std::vector<int> targets;
	PortDataTexture* pdt;

	for (size_t i=0; i < portData.size(); ++i) {
		pdt = dynamic_cast<PortDataTexture*>(portData.at(i));

		if (!pdt)
            throw VoreenException("The data mapped to this is port is not a TextureContainer target.");
		
		targets.push_back(pdt->getData());
	}

	return targets;
}

int PortMapping::getGeometryNumber(Processor* processor, Identifier ident, int pos) throw (std::exception) {
	Port* p = processor->getPort(ident);
	if (p==0)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> portData = portMap_[p];

	if (static_cast<int>(portData.size()) <= pos)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	PortDataGeometry* pdg = dynamic_cast<PortDataGeometry*>(portData.at(pos));

	if (!pdg)
        throw VoreenException("The data mapped to this is port is not a geometry number.");

	return pdg->getData();
}

std::vector<int> PortMapping::getAllGeometryNumbers(Processor* processor, Identifier ident) throw(std::exception) {
	Port* p = processor->getPort(ident);
	if (p==0)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> portData = portMap_[p];

	if (portData.size() < 1)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	std::vector<int> targets;
	PortDataGeometry* pdg;

	for (size_t i=0; i < portData.size(); ++i) {
		pdg= dynamic_cast<PortDataGeometry*>(portData.at(i));

		if (!pdg)
            throw VoreenException("The data mapped to this is port is not a volume number.");
		
		targets.push_back(pdg->getData());
	}
	return targets;
}

PortDataCoProcessor* PortMapping::getCoProcessorData(Processor* processor, Identifier ident, int pos) throw(std::exception) {
	Port* p = processor->getPort(ident);
	if (p==0)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> portData = portMap_[p];

	if (static_cast<int>(portData.size()) < pos)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");

	PortDataCoProcessor* pdcp = dynamic_cast<PortDataCoProcessor*>(portData.at(pos));

	if (!pdcp)
        throw VoreenException("The data mapped to this is port is not a CoProcessor.");

	return pdcp;
}

std::vector<PortDataCoProcessor*> PortMapping::getAllCoProcessorData(Processor* processor, Identifier ident) throw(std::exception) {
	Port* p = processor->getPort(ident);
	if (p==0)
        throw VoreenException("No port with the given identifier '" + ident.getName() + "' found.");

	std::vector<PortData*> portData = portMap_[p];

	if (portData.size() < 1)
		throw VoreenException("No data was mapped for that port: '" + ident.getName() + "'");
	
	std::vector<PortDataCoProcessor*> result;
	PortDataCoProcessor* pdcp;

	for (size_t i=0; i<portData.size(); ++i) {
		pdcp = dynamic_cast<PortDataCoProcessor*>(portData.at(i));

		if (!pdcp)
            throw VoreenException("The data mapped to this is port is not a CoProcessor.");

		result.push_back(pdcp);

	}
	return result;
}

//---------------------------------------------------------------------------

LocalPortMapping::LocalPortMapping(PortMapping* portMapping, Processor* processor)
    : portMapping_(portMapping)
    , processor_(processor)
{
}

int LocalPortMapping::getTarget(Identifier ident, int pos) throw (std::exception) {
	return portMapping_->getTarget(processor_, ident, pos);
}

std::vector<int> LocalPortMapping::getAllTargets(Identifier ident) throw(std::exception) {
	return portMapping_->getAllTargets(processor_, ident);
}

int LocalPortMapping::getGeometryNumber(Identifier ident, int pos) throw (std::exception) {
	return portMapping_->getGeometryNumber(processor_, ident, pos);
}

std::vector<int> LocalPortMapping::getAllGeometryNumbers(Identifier ident) throw(std::exception) {
	return portMapping_->getAllGeometryNumbers(processor_, ident);
}

PortDataCoProcessor* LocalPortMapping::getCoProcessorData(Identifier ident, int pos) throw(std::exception) {
	return portMapping_->getCoProcessorData(processor_, ident, pos);
}

std::vector<PortDataCoProcessor*> LocalPortMapping::getAllCoProcessorData(Identifier ident) throw(std::exception) {
	return portMapping_->getAllCoProcessorData(processor_, ident);
}

LocalPortMapping* LocalPortMapping::createLocalPortMapping(Processor *processor) throw(std::exception)  {
	return new LocalPortMapping(portMapping_, processor);
}

} //namespace voreen
