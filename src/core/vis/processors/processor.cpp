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

#include "voreen/core/vis/processors/processor.h"

#include "tgt/glmath.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/lightmaterial.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/processors/portmapping.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;

namespace voreen {

HasShader::HasShader()
    : needRecompileShader_(true)
{}

HasShader::~HasShader() {}

void HasShader::compileShader() {
    if (needRecompileShader_) {
        compile();
        needRecompileShader_ = false;
    }
}

void HasShader::invalidateShader() {
    needRecompileShader_ = true;
}

//---------------------------------------------------------------------------

const std::string Processor::loggerCat_("voreen.Processor");

const std::string Processor::XmlElementName_("Processor");

Processor::Processor()
    : MessageReceiver()
    , Serializable()
    , cacheable_(true)
    , useVolumeCaching_(true)
    , isCoprocessor_(false)
    , initStatus_(VRN_OK),
    portMap_()
{
}

Processor::~Processor() {
    for (size_t i = 0; i < inports_.size(); ++i)
        delete inports_[i];
    for (size_t i = 0; i < outports_.size(); ++i)
        delete outports_[i];
    for (size_t i = 0; i < coProcessorInports_.size(); ++i)
        delete coProcessorInports_[i];
    for (size_t i = 0; i < coProcessorOutports_.size(); ++i)
        delete coProcessorOutports_[i];
    for (size_t i = 0; i < privatePorts_.size(); ++i)
        delete privatePorts_[i];
}

int Processor::initializeGL() {
    return VRN_OK;
}

Message* Processor::call(Identifier /*ident*/, LocalPortMapping*) {
    return 0;
}

void Processor::addProperty(Property* prop) {
    props_.push_back(prop);
    prop->setOwner(this);
}

const Properties& Processor::getProperties() const {
    return props_;
}

bool Processor::connect(Port* outport, Port* inport) {
    if ((outport != 0) && (outport->testConnectivity(inport) == true))
        return outport->connect(inport);
    return false;
}

bool Processor::disconnect(Port* outport, Port* inport) {
    if (outport != 0)
        return (outport->disconnect(inport));
    return false;
}

bool Processor::testConnect(Port* const outport, Port* const inport) {
    if (outport != 0)
        return outport->testConnectivity(inport);
    return false;
}

Port* Processor::getPort(Identifier ident) {
    PortMap::iterator it = portMap_.find(ident);
    if (it == portMap_.end())
        return 0;
    return it->second;
}

void Processor::createInport(Identifier ident, bool allowMultipleConnections) {
    Port* inport = new GenericInPort<int>(ident, this, allowMultipleConnections);
    inports_.push_back(inport);
    portMap_.insert(std::make_pair(ident, inport));

    //inports_.push_back(new Port(ident,this,false,allowMultipleConnectios,false));
}

void Processor::createOutport(Identifier ident, bool isPersistent, Identifier inport) {
    Port* outport = new GenericOutPort<int, PortDataTexture>(ident, -1, this, true, isPersistent, false);
    outports_.push_back(outport);
    portMap_.insert(std::make_pair(ident, outport));

    /*
    Port* newOutport = new Port(ident, this, true, true, isPersistent);
    outports_.push_back(outport);
    */
    if (inport != "dummy.port.unused") {
        if (getPort(inport) != 0)
            outportToInportMap_.insert(std::pair<Port*,Port*>(outport, getPort(inport)));
        else
            LWARNING("Couldn't find the inport in Processor::createOutport(Identifier ident, Identifier ident)");
    }
}

void Processor::createCoProcessorInport(Identifier ident, bool allowMultipleConnections) {

    //Port* coInport = new Port(ident, this, false, allowMultipleConnectios, false)
    
    Port* coInport = new GenericInPort<FunctionPointer>(ident, this, allowMultipleConnections);
    coProcessorInports_.push_back(coInport);
    portMap_.insert(std::make_pair(ident, coInport));
}

void Processor::createCoProcessorOutport(Identifier ident, FunctionPointer function, 
                                         bool allowMultipleConnections)
{
    Port* coOutport = new GenericOutPort<FunctionPointer, PortDataCoProcessor>(
        ident, PortDataCoProcessor(this, function), this, allowMultipleConnections, false, false);
    coOutport->setFunctionPointer(function);    // TODO: remove when old evaluator is removed
    coProcessorOutports_.push_back(coOutport);
    portMap_.insert(std::make_pair(ident, coOutport));

    //Port* newPort = new Port(ident,this,true,allowMultipleConnectios,false);
    //newPort->setFunctionPointer(function);
    //coProcessorOutports_.push_back(newPort);
}

void Processor::createPrivatePort(Identifier ident) {
    Port* port = new GenericOutPort<int, PortDataTexture>(ident, -1, this, false, true, true);
    privatePorts_.push_back(port);
    portMap_.insert(std::make_pair(ident, port));

    //privatePorts_.push_back(new Port(ident,this,true,false,true));
}


void Processor::processMessage(Message* msg, const Identifier& dest) {
	MessageReceiver::processMessage(msg, dest);
}

void Processor::setName(const std::string& name) {
    name_ = name;
}

const std::string& Processor::getName() const {
    return name_;
}

const std::string Processor::getProcessorInfo() const {
    return "No information available";
}

const std::vector<Port*>& Processor::getInports() const {
    return inports_;
}

const std::vector<Port*>& Processor::getOutports() const {
    return outports_;
}

const std::vector<Port*>& Processor::getCoProcessorInports() const {
    return coProcessorInports_;
}

const std::vector<Port*>& Processor::getCoProcessorOutports() const {
    return coProcessorOutports_;
}

const std::vector<Port*>& Processor::getPrivatePorts() const {
    return privatePorts_;
}

bool Processor::hasPortOfCertainType(int portTypeMask) const {
    std::vector<Port*> ports = inports_;
    ports.insert(ports.end(), outports_.begin(), outports_.end());
    ports.insert(ports.end(), coProcessorInports_.begin(), coProcessorInports_.end());
    ports.insert(ports.end(), coProcessorOutports_.begin(), coProcessorOutports_.end());
    if (portTypeMask == 0)
        portTypeMask = ~0;

    for (size_t i = 0; i < ports.size(); ++i) {
        int pt = static_cast<int>(ports[i]->getType());
        if ((portTypeMask & pt) != 0)
            return true;
    }
    return false;
}

Port* Processor::getInport(Identifier type) {
    for (size_t i = 0; i < inports_.size(); ++i) {
        if (inports_.at(i)->getTypeIdentifier() == type)
            return inports_.at(i);
    }
    return 0;
}

Port* Processor::getOutport(Identifier type) {
    for (size_t i = 0; i < outports_.size(); ++i) {
        if (outports_.at(i)->getTypeIdentifier() == type)
            return outports_.at(i);
    }
    return 0;
}

void Processor::invalidate() {
    MsgDistr.postMessage(new ProcessorPointerMsg("processor.invalidated", this));
}

bool Processor::isEndProcessor() const {
    return ((outports_.size() == 0) && (coProcessorOutports_.size() == 0));
}

std::string Processor::getState() const {
    std::string state;
    for (size_t i = 0; i < props_.size(); ++i)
        state += props_[i]->toString();
    return state;
}

std::string Processor::getXmlElementName() const {
    return XmlElementName_;
}

const Identifier Processor::getClassName(TiXmlElement* processorElem) {
    if (!processorElem)
        throw XmlElementException("Can't get ClassName of Null-Pointer!");

    if (processorElem->Value() != XmlElementName_)
        throw XmlElementException("Can't get ClassName of a " + std::string(processorElem->Value())
                                  + " - need " + XmlElementName_ + "!");

    if (!processorElem->Attribute("type"))
        throw XmlAttributeException("Needed attribute 'type' missing");


    return Identifier(processorElem->Attribute("type"));
}

TiXmlElement* Processor::serializeToXml() const {
    TiXmlElement* processorElem = new TiXmlElement(XmlElementName_);

    // metadata
    TiXmlElement* metaElem = meta_.serializeToXml();
    processorElem->LinkEndChild(metaElem);

    // misc settings
    processorElem->SetAttribute("type", getClassName().getName());
    processorElem->SetAttribute("name", getName());

    // serialize the properties of the processor
    std::vector<Property*> properties = getProperties();
    for (size_t i = 0; i < properties.size(); ++i) {
        if (properties[i]->isSerializable() || ignoreIsSerializable()) {
            TiXmlElement* propElem = properties[i]->serializeToXml();
            processorElem->LinkEndChild(propElem);
        }
    }

    return processorElem;
}

TiXmlElement* Processor::serializeToXml(const std::map<Processor*,int> idMap) const {
    TiXmlElement* processorElem = serializeToXml();
    Processor* self = const_cast<Processor*>(this); // For const-correctness - can't use 'this' in Map::find
    processorElem->SetAttribute("id", idMap.find(self)->second);
    // serialize the (in)ports a.k.a. connection info
    std::vector<Port*> outports = getOutports();
    const std::vector<Port*>& coprocessoroutports = getCoProcessorOutports();
    // append coprocessoroutports to outports because we can handle them identically
    outports.insert(outports.end(), coprocessoroutports.begin(), coprocessoroutports.end());
    for (size_t i = 0; i < outports.size(); ++i) {
        // add (out)port
        TiXmlElement* outportElem = new TiXmlElement("Outport");
        outportElem->SetAttribute("type", outports[i]->getTypeIdentifier().getName());
        processorElem->LinkEndChild(outportElem);
        // add processors connected to this (out)port via one of their (in)ports
        std::vector<Port*> connectedPorts = outports[i]->getConnected();
        for (size_t j=0; j < connectedPorts.size(); ++j) {
            TiXmlElement* connectedProcessorElem = new TiXmlElement(XmlElementName_);
            connectedProcessorElem->SetAttribute("id", idMap.find(connectedPorts[j]->getProcessor())->second);
            connectedProcessorElem->SetAttribute("port", connectedPorts[j]->getTypeIdentifier().getName());
            // For inports that allow multiple connections
            // need to know the index of outport in the inports connected ports
            if (connectedPorts[j]->allowMultipleConnections())
                connectedProcessorElem->SetAttribute("order", connectedPorts[j]->getIndexOf(outports[i]));
            outportElem->LinkEndChild(connectedProcessorElem);
        }
    }

    return processorElem;
}

void Processor::updateFromXml(TiXmlElement* processorElem) {
    errors_.clear();
    // meta
    TiXmlElement* metaElem = processorElem->FirstChildElement(meta_.getXmlElementName());
    if (metaElem) {
        meta_.updateFromXml(metaElem);
        errors_.store(meta_.errors());
    }
    else
        errors_.store(XmlElementException("Metadata missing!")); // TODO better exception
    // read properties
    if (processorElem->Attribute("name"))
        setName(processorElem->Attribute("name"));

    for (TiXmlElement* propertyElem = processorElem->FirstChildElement(Property::XmlElementName_);
        propertyElem != 0;
        propertyElem = propertyElem->NextSiblingElement(Property::XmlElementName_))
    {
        for (size_t j = 0; j < props_.size(); ++j) {
            try {
                if (props_[j]->getIdent() == Property::getIdent(propertyElem) ) {
                    props_[j]->updateFromXml(propertyElem);
                    errors_.store(props_[j]->errors());
                }
            } catch (SerializerException& e) {
                errors_.store(e);
            }
        }
    }
}

std::pair<int, Processor::ConnectionMap> Processor::getMapAndUpdateFromXml(TiXmlElement* processorElem) {
    updateFromXml(processorElem);
    ConnectionSide out, in;
    ConnectionMap lcm;
    int id;
    if (processorElem->QueryIntAttribute("id", &id) != TIXML_SUCCESS) {
        errors_.store(XmlAttributeException("Required attribute 'id' of Processor missing!"));
        id = -1;
    }
    else {
        out.processorId = id;

        // read outports
        TiXmlElement* outportElem;
        for (outportElem = processorElem->FirstChildElement("Outport");
            outportElem;
            outportElem = outportElem->NextSiblingElement("Outport"))
        {
            if (!outportElem->Attribute("type")) {
                errors_.store(XmlAttributeException("Required attribute 'type' of Port missing!"));
            }
            else {
                out.portId = outportElem->Attribute("type");
                //if (outportElem->QueryIntAttribute("order", &out.order) != TIXML_SUCCESS)
                    out.order = 0; // Annotation: The order for the first Port is not really needed
                // read processors connected to outport
                TiXmlElement* connectedprocessorElem;
                for (connectedprocessorElem = outportElem->FirstChildElement(XmlElementName_);
                    connectedprocessorElem;
                    connectedprocessorElem = connectedprocessorElem->NextSiblingElement(XmlElementName_))
                {
                    try {
                        if (connectedprocessorElem->QueryIntAttribute("id", &in.processorId) != TIXML_SUCCESS)
                            throw XmlAttributeException("Required attribute id of Processor missing!");
                        if (!connectedprocessorElem->Attribute("port"))
                            throw XmlAttributeException("Required attribute port of Processor missing!");

                        if (connectedprocessorElem->QueryIntAttribute("order", &in.order) != TIXML_SUCCESS)
                            in.order = 0;
                        in.portId = connectedprocessorElem->Attribute("port");

                        // store this connection in the map
                        lcm.push_back(std::make_pair(out, in));
                    }
                    catch (SerializerException& e) {
                        errors_.store(e);
                    }
                }
            }
        }
    }

    return std::make_pair(id, lcm);
}

void Processor::addToMeta(TiXmlElement* elem) {
    meta_.addData(elem);
}

void Processor::removeFromMeta(std::string elemName) {
    meta_.removeData(elemName);
}

void Processor::clearMeta() {
    meta_.clearData();
}

TiXmlElement* Processor::getFromMeta(std::string elemName) const {
    return meta_.getData(elemName);
}

std::vector<TiXmlElement*> Processor::getAllFromMeta() const {
    return meta_.getAllData();
}

bool Processor::hasInMeta(std::string elemName) const {
    return meta_.hasData(elemName);
}

bool Processor::getIsCoprocessor() const {
    return isCoprocessor_;
}

void Processor::setIsCoprocessor(bool b) {
    isCoprocessor_ = b;
}

bool Processor::getCacheable() const {
    return cacheable_;
}

void Processor::setCacheable(bool b) {
    cacheable_ = b;
}

int Processor::getInitStatus() const {
    return initStatus_;
}

std::map<Port*,Port*> Processor::getOutportToInportMap() {
    return outportToInportMap_;
}

} // namespace voreen
