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

#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/processornetwork.h"

#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

namespace voreen {

NetworkSerializer::NetworkSerializer() {}

NetworkSerializer::~NetworkSerializer() {}

ProcessorNetwork* NetworkSerializer::readNetworkFromFile( std::string filename ) throw (SerializerException)
{
    //Prepare the TiXmlDoxument
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file!");
    // Still a bit messy in here...
    TiXmlHandle documentHandle(&doc);

    TiXmlElement* element;
    TiXmlHandle rootHandle(0);

    element = documentHandle.FirstChildElement().Element();
    if (!element)
        throw SerializerException("Did not find root element!");

    std::string s = element->Value();

    rootHandle=TiXmlHandle(element);

    TiXmlNode* node=&doc;

    TiXmlNode* versionFinder = node;            //just for finding out the version, not needed afterwards
    int version = findVersion(versionFinder);

    ProcessorNetwork* net = new ProcessorNetwork();

    switch (version) {
    case 2:
        net->updateFromXml(doc.RootElement());
        break;
    default:
        throw SerializerException("Unsupported or unknown network version!");
    }
    return net;
}

int NetworkSerializer::readVersionFromFile(std::string filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializerException("Could not load network file " + filename);
    int version = findVersion(&doc); // FIXME this is not safe
    return version;
}

int NetworkSerializer::findVersion(TiXmlNode* node) {
    if (node->Type() != TiXmlNode::ELEMENT) {
        TiXmlNode* pChild;
        for (pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) {
            int result=findVersion(pChild);
            if (result!=0)
                return result;
        }
    }
    else {
        TiXmlElement* element=node->ToElement();
        if (element->FirstAttribute() ) {
            TiXmlAttribute* attribute=element->FirstAttribute();
            std::string s = attribute->Name();
            if (s == "Version" || s == "version")
                return attribute->IntValue();
        }
    }
    return 0;

}

void NetworkSerializer::serializeToXml(const ProcessorNetwork* network, const std::string& filename)
    throw (SerializerException)
{
    TiXmlDocument doc;
    TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(declNode);
    // Serialize the network and add it as root element
    TiXmlElement* networkElem = network->serializeToXml();
    doc.LinkEndChild(networkElem);

    if (!doc.SaveFile(filename))
        throw SerializerException("Could not write network file " + filename);
}

// static methods
//

size_t NetworkSerializer::connectEventListenersToHandler(tgt::EventHandler* const eventHandler,
                                                         const std::vector<Processor*>& processors,
                                                         const bool addToFront)
{
    if (eventHandler == 0)
        return 0;

    size_t counter = 0;
    for (size_t i = 0; i < processors.size(); ++i) {
        tgt::EventListener* listener = dynamic_cast<tgt::EventListener*>(processors[i]);
        if (listener != 0) {
            if (addToFront == true)
                eventHandler->addListenerToFront(listener);
            else
                eventHandler->addListenerToBack(listener);
            ++counter;
        }
    }
    return counter;
}

size_t NetworkSerializer::removeEventListenersFromHandler(tgt::EventHandler* const eventHandler,
                                                          const std::vector<Processor*>& processors)
{
    if (eventHandler == 0)
        return 0;

    size_t counter = 0;
    for (size_t i = 0; i < processors.size(); ++i) {
        tgt::EventListener* listener = dynamic_cast<tgt::EventListener*>(processors[i]);
        if (listener != 0) {
            eventHandler->removeListener(listener);
            ++counter;
        }
    }
    return counter;
}

} // namespace voreen
