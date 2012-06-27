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

#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/plotting/plotbase.h"
#include "voreen/core/plotting/plotselection.h"

namespace voreen {

const std::string XmlDeserializer::loggerCat_ = "voreen.XmlDeserializer";

XmlDeserializer::XmlDeserializer(std::string documentPath)
    : XmlSerializerBase()
    , documentPath_(documentPath)
{
}

XmlDeserializer::~XmlDeserializer() {
    for (UnresolvedReferenceMapType::iterator mapIt = unresolvedReferenceMap_.begin();
        mapIt != unresolvedReferenceMap_.end(); ++mapIt)
    {
        for (ReferenceResolverListType::iterator listIt = mapIt->second.begin();
            listIt != mapIt->second.end(); ++listIt)
        {
            delete *listIt;
        }
    }
}

std::string XmlDeserializer::getDocumentPath() const {
    return documentPath_;
}

void XmlDeserializer::deserialize(const std::string& key, bool& data)
    throw (SerializationException)
{
    std::string boolValue;
    deserializeSimpleTypes(key, boolValue);

    std::transform(boolValue.begin(), boolValue.end(), boolValue.begin(), tolower);

    if (boolValue == "true")
        data = true;
    else if (boolValue == "false")
        data = false;
    else
        raise(XmlSerializationFormatException("XML node with key '" + key + "' contains unknown bool value."));
}

void XmlDeserializer::deserialize(const std::string& key, char& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, signed char& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, unsigned char& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, signed short& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, unsigned short& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, signed int& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, unsigned int& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, signed long& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, unsigned long& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, float& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, double& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, long double& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, std::string& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::vec2& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::vec3& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::vec4& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::dvec2& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::dvec3& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::dvec4& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}


void XmlDeserializer::deserialize(const std::string& key, tgt::ivec2& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::ivec3& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::ivec4& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::col3& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data, true);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::col4& data)
    throw (SerializationException)
{
    deserializeTgtVector(key, data, true);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::mat2& data)
    throw (SerializationException)
{
    tgt::vec2 row0, row1;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    data = tgt::mat2(row0, row1);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::mat3& data)
    throw (SerializationException)
{
    tgt::vec3 row0, row1, row2;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    deserializeTgtVector(key+".row2", row2);
    data = tgt::mat3(row0, row1, row2);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::mat4& data)
    throw (SerializationException)
{
    tgt::vec4 row0, row1, row2, row3;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    deserializeTgtVector(key+".row2", row2);
    deserializeTgtVector(key+".row3", row3);
    data = tgt::mat4(row0, row1, row2, row3);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::Matrix2d& data)
    throw (SerializationException)
{
    tgt::dvec2 row0, row1;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    data = tgt::Matrix2d(row0, row1);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::Matrix3d& data)
    throw (SerializationException)
{
    tgt::dvec3 row0, row1, row2;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    deserializeTgtVector(key+".row2", row2);
    data = tgt::Matrix3d(row0, row1, row2);
}

void XmlDeserializer::deserialize(const std::string& key, tgt::Matrix4d& data)
    throw (SerializationException)
{
    tgt::dvec4 row0, row1, row2, row3;
    deserializeTgtVector(key+".row0", row0);
    deserializeTgtVector(key+".row1", row1);
    deserializeTgtVector(key+".row2", row2);
    deserializeTgtVector(key+".row3", row3);
    data = tgt::Matrix4d(row0, row1, row2, row3);
}

void XmlDeserializer::deserialize(const std::string& key, PlotCellValue& data)
    throw (SerializationException)
{
    TemporaryNodeChanger nodeChanger(*this, getNextXmlElement(key));

    // first aquire flags
    bool isHighlighted, isTag, isValue;
    plot_t value;
    std::string tag;
    deserializeSimpleTypes("isValue", isValue);
    deserializeSimpleTypes("isTag", isTag);
    deserializeSimpleTypes("isHighlighted", isHighlighted);

    // now call the according constructors
    if (isValue) {
        deserializeSimpleTypes("value", value);
        data = PlotCellValue(value);
    }
    else if (isTag) {
        deserializeSimpleTypes("tag", tag);
        data = PlotCellValue(tag);
    }
    else { // neither value nor tag
        data = PlotCellValue();
    }

    // finally set highlighted flag
    data.setHighlighted(isHighlighted);
}

void XmlDeserializer::deserialize(const std::string& key, PlotSelectionEntry& data)
    throw (SerializationException)
{
    TemporaryNodeChanger nodeChanger(*this, getNextXmlElement(key));

    deserialize("selection", data.selection_);
    deserializeSimpleTypes("highlight", data.highlight_);
    deserializeSimpleTypes("renderLabel", data.renderLabel_);
    deserializeSimpleTypes("zoomTo", data.zoomTo_);
}

void XmlDeserializer::deserialize(const std::string& key, Serializable& data)
    throw (SerializationException)
{
    deserializeSimpleTypes(key, data);
}

TiXmlElement* XmlDeserializer::getNextXmlElement(const std::string& key)
    throw (SerializationException)
{
    TiXmlElement* element = node_->FirstChildElement(key);
    while (element) {
        // Was node not visited before?
        if (visitedNodes_.find(element) == visitedNodes_.end())
        {
            visitedNodes_.insert(element);
            return element;
        }

        element = element->NextSiblingElement(key);
    }

    raise(XmlSerializationNoSuchDataException("No further XML node with key '" + key + "' found."));
    return 0;
}

void XmlDeserializer::findUnresolvableReferences(
    TiXmlElement* node,
    ReferenceIdSetType& references,
    ReferenceIdSetType& resolvableReferences) const
{
    const std::string* id = node->Attribute(XmlSerializationConstants::IDATTRIBUTE);
    const std::string* ref = node->Attribute(XmlSerializationConstants::REFERENCEATTRIBUTE);

    if (id)
        resolvableReferences.insert(*id);

    if (ref)
        references.insert(*ref);

    for (TiXmlElement* child = node->FirstChildElement(); child != 0; child = child->NextSiblingElement())
        findUnresolvableReferences(child, references, resolvableReferences);
}

XmlDeserializer::ReferenceIdListType XmlDeserializer::findUnresolvableReferences() const
{
    ReferenceIdListType unresolvableReferences;

    ReferenceIdSetType references;
    ReferenceIdSetType resolvableReferences;
    findUnresolvableReferences(node_->ToElement(), references, resolvableReferences);

    for (ReferenceIdSetType::const_iterator it = references.begin(); it != references.end(); ++it)
        if (resolvableReferences.find(*it) == resolvableReferences.end())
            unresolvableReferences.push_back(*it);

    return unresolvableReferences;
}

void XmlDeserializer::read(std::istream& stream, XmlProcessor* xmlProcessor)
    throw (SerializationException)
{
    // Read input stream...
    std::stringbuf buffer;
    do
    {
        // Use 0 character instead of '\n' to minimize the number of get-calls...
        stream.get(buffer, 0);
    } while (stream.good() && !stream.eof()
        && (buffer.sputc(stream.get()) != std::stringbuf::traits_type::eof()));

    // Parse input...
    document_.Parse(buffer.str().c_str());

    TiXmlElement* root = document_.RootElement();

    // Is there no root element?
    if (!root)
        raise(XmlSerializationFormatException(std::string("No root node found.")));

    // Has root node incorrect name?
    if (root->ValueStr() != XmlSerializationConstants::ROOTNODE) {
        raise(XmlSerializationFormatException("XML root node name is '" + root->ValueStr()
            + "' instead of '" + XmlSerializationConstants::ROOTNODE + "'."));
    }

    const std::string* version = root->Attribute(XmlSerializationConstants::VERSIONATTRIBUTE);
    // Is serialization version not set?
    if (!version)
        raise(XmlSerializationFormatException("XML root node has no version attribute."));
    // Does XmlSerializer and XmlDeserializer version not match the XML document version?
    if (*version != XmlSerializationConstants::VERSION) {
        raise(XmlSerializationVersionMismatchException("XML document has version " + *version
            + " instead of " + XmlSerializationConstants::VERSION + "."));
    }

    node_ = root;

    // Apply preprocessor, if a XML preprocessor is given...
    if (xmlProcessor)
        xmlProcessor->process(document_);

    std::vector<std::string> unresolvableReferences = findUnresolvableReferences();
    if (!unresolvableReferences.empty()) {
        std::stringstream idStream;
        for (ReferenceIdListType::iterator it = unresolvableReferences.begin(); it != unresolvableReferences.end(); ++it)
            idStream << (idStream.str().empty() ? "" : ", ") << "'" << *it << "'";

        raise(XmlSerializationReferenceResolvingException(
            "XML document contains the following unresolvable references: " + idStream.str() + "."));
    }
}

void XmlDeserializer::freePointer(void* pointer) {
    for (IdAddressMapType::iterator it = idAddressMap_.begin(); it != idAddressMap_.end(); ++it)
        if (it->second == pointer)
            it->second = 0;
}

} // namespace
