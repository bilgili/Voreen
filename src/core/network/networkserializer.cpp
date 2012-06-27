/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/network/networkserializer.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkconverter.h"

#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"

#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

NetworkSerializer::SerializationResource::SerializationResource(ProcessorNetwork* network,
                                                                XmlSerializerBase* serializer,
                                                                const std::string& filename,
                                                                const std::ios_base::openmode openMode)
    : network_(network)
    , serializer_(serializer)
    , stream_(filename.c_str(), openMode)
{
    serializer_->setUseAttributes(true);
}

NetworkSerializer::SerializationResource::~SerializationResource() {
    stream_.close();

    // It would be better to directly access network_ as a friend here, but MinGW doesn't like
    // accessing inner classes of friends.
    if (network_)
        network_->setErrors(serializer_->getErrors());
}

std::fstream& NetworkSerializer::SerializationResource::getStream() {
    return stream_;
}

ProcessorNetwork* NetworkSerializer::readNetworkFromFile(std::string filename)
    throw (SerializationException)
{
    ProcessorNetwork* net = 0;

    XmlDeserializer d(filename);
    SerializationResource resource(net, &d, filename, std::ios_base::in);

    d.read(resource.getStream(), this);
    d.deserialize("ProcessorNetwork", net);

    return net;
}

void NetworkSerializer::writeNetworkToFile(const ProcessorNetwork* network, const std::string& filename)
    throw (SerializationException)
{
    XmlSerializer s(filename);
    SerializationResource resource(const_cast<ProcessorNetwork*>(network), &s, filename, std::ios_base::out);

    s.serialize("ProcessorNetwork", network);
    s.write(resource.getStream());
}

void NetworkSerializer::process(TiXmlDocument& document) {
    TiXmlElement* processorNetworkNode = findProcessorNetworkNode(document.RootElement());
    int version = findVersion(processorNetworkNode);

    switch (version) {
        case -1:
            throw XmlSerializationFormatException("Could not determine network format version!");

        // Fall through to the newest version.
        // Omit break statements in all case blocks except the last.
        case 4:
            NetworkConverter4to5().convert(processorNetworkNode);
        case 5:
            NetworkConverter5to6().convert(processorNetworkNode);
        case 6:
            NetworkConverter6to7().convert(processorNetworkNode);
        case 7:
            break;

        default:
            std::stringstream msg;
            msg << "Unsupported network format version '" << version << "'.";
            throw XmlSerializationFormatException(msg.str());
    }
}

int NetworkSerializer::readVersionFromFile(std::string filename)
    throw (SerializationException)
{
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        throw SerializationException("Could not load network file " + filename);
    int version = findVersion(&doc);
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

TiXmlElement* NetworkSerializer::findProcessorNetworkNode(TiXmlElement* node) {
    const std::string PROCESSORNETWORKNODE = "ProcessorNetwork";

    if (node->ValueStr() == PROCESSORNETWORKNODE)
        return node;

    TiXmlElement* processorNetworkNode = 0;
    for (TiXmlElement* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement())
        if ((processorNetworkNode = findProcessorNetworkNode(child)) != 0)
            return processorNetworkNode;

    return 0;
}

} // namespace voreen
