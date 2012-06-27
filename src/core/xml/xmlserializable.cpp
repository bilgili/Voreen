#include "voreen/core/xml/xmlserializable.h"

namespace voreen{
int XmlSerializable::idCounter_ = 0;

XmlSerializable::XmlSerializable(bool init) 
{
    if (init)
        serialId_ = idCounter_++;
    else
        serialId_ = -1;
    
    getAllObjects()->push_front(this);
}

int XmlSerializable::getSerialId() const {
    return serialId_;
}

TiXmlElement* XmlSerializable::serialize() const {
    TiXmlElement* result = new TiXmlElement(getClassIdentifier().getName());
    result->SetAttribute("serialId", serialId_);
    return result;
}

void XmlSerializable::deserialize(const TiXmlElement* xml) throw (XmlSerializable::Exceptions)
{
    int serialId, error;
    const char* identifier = xml->Value();
    if (identifier == 0)
        throw (XmlSerializable::WRONG_IDENTIFIER);

    if (identifier != getClassIdentifier().getName())
        throw (XmlSerializable::WRONG_IDENTIFIER);

    error = xml->QueryIntAttribute("serialId", &serialId);
    if (error != TIXML_SUCCESS)
        throw (XmlSerializable::NO_SERIAL_ID);

    serialId_ = serialId;
}

XmlSerializable* XmlSerializable::getPointer(int serialId) {
    if (serialId < 0)
        return 0;
    
    XmlSerializable* tmp;
    std::list<XmlSerializable*>::iterator it;
	std::list<XmlSerializable*>* allObjects = getAllObjects();
    for (it = allObjects->begin(); it != allObjects->end(); ++it) {
        tmp = static_cast<XmlSerializable*>(*it);
        if(serialId == tmp->getSerialId())
            return tmp;
    }
    return 0;
}

// FIXME: replace std::list for XmlSerializableRegister by using std::set in 
// order to achieve runtime of O(log(n)) instead of O(n).
//
/*
void XmlSerializable::registerNewClass(XmlSerializableRegister* newClass) {
    const Identifier& newClassIdentifier = newClass->getClassIdentifier();
    std::list<XmlSerializableRegister*>::iterator it;
	std::list<XmlSerializableRegister*>* allClasses = getAllClasses();
	for(it = allClasses->begin(); it != allClasses->end(); ++it) {
        XmlSerializableRegister* tmp = static_cast<XmlSerializableRegister*>(*it);
		const Identifier& oldClassIdentifier = tmp->getClassIdentifier();
        if (newClassIdentifier == oldClassIdentifier)
            return;
	}
    allClasses->push_front(newClass);
}
*/

void XmlSerializable::registerNewClass(const Identifier& classID, XmlSerializable* (*creatorMethod)(void)) {
    if( (classID.getName().empty() == true) || (creatorMethod == 0) )
        return;

    static ClassMap& allClasses = getAllClasses();
    //std::pair<ClassMap::iterator, bool> pr =
        allClasses.insert( std::make_pair(classID, creatorMethod) );
    //return pr.second;
}

/*
XmlSerializable* XmlSerializable::getClassFromXml(const TiXmlElement* xml)
    throw (XmlSerializable::Exceptions)
{
    std::string classIdentifier = xml->ValueStr();
    if (classIdentifier == "")
        throw (XmlSerializable::WRONG_IDENTIFIER);

    std::list<XmlSerializableRegister*>::iterator it;
    std::list<XmlSerializableRegister*>* allClasses = getAllClasses();
    for (it = allClasses->begin(); it != allClasses->end(); ++it) {
        XmlSerializableRegister* tmp = static_cast<XmlSerializableRegister*>(*it);
        if (classIdentifier == tmp->getClassIdentifier().getName()) {
            XmlSerializable* result = tmp->getNewInstance();
            try{
                result->deserialize(xml);
            } catch (XmlSerializable::Exceptions e) {
                throw e;
            }
            return result;
        }
    }
    throw (XmlSerializable::NO_SUCH_CLASS);
}*/


XmlSerializable* XmlSerializable::getClassFromXml(const TiXmlElement* xml)
    throw (XmlSerializable::Exceptions)
{
    const std::string& classIdentifier = xml->ValueStr();
    XmlSerializable* result;
    try {
        result = getClassFromIdentifier(Identifier(classIdentifier));
    } catch (XmlSerializable::Exceptions e) {
        throw e;
    }
    try {
        result->deserialize(xml);
    } catch (XmlSerializable::Exceptions e) {
        throw e;
    }
    return result;
}

XmlSerializable* XmlSerializable::getClassFromIdentifier(const Identifier id)
    throw (XmlSerializable::Exceptions)
{
    const std::string& classIdentifier = id.getName();
    if (classIdentifier.empty() == true)
        throw (XmlSerializable::WRONG_IDENTIFIER);

    ClassMap& allClasses = getAllClasses();
    ClassMap::iterator it = allClasses.find(Identifier(classIdentifier));
    if( it == allClasses.end() )
        throw (XmlSerializable::NO_SUCH_CLASS);

    if( it->second == 0 )
        throw (XmlSerializable::NO_SUCH_CLASS);

    XmlSerializable* result = (*(it->second))();
    return result;
}

void XmlSerializable::writeBoolAttribute(std::string attributeName, bool value,
                                         TiXmlElement* xml)
{
    const char* textValue = (value) ? "true" : "false";
    xml->SetAttribute(attributeName,textValue);
}

bool XmlSerializable::readBoolAttribute(std::string attributeName, 
                                        const TiXmlElement* xml)
    throw (XmlSerializable::Exceptions)
{
    const char* value = xml->Attribute(attributeName);
    if (value == 0)
        throw XmlSerializable::ATTRIBUT_ERROR;
    
    if (strcmp(value, "true") == 0)
        return true;
    else if (strcmp(value, "false") == 0)
        return false;
    else
        throw XmlSerializable::ATTRIBUT_ERROR;
}

std::list<XmlSerializable*>* XmlSerializable::getAllObjects() {
  static std::list<XmlSerializable*>* const allObjects = new std::list<XmlSerializable*>();
  return allObjects;
}

XmlSerializable::ClassMap& XmlSerializable::getAllClasses() {
    static ClassMap* const allClasses = new ClassMap();
    return *allClasses;
}

// FIXME: replace std::list for XmlSerializableRegister by using std::set in 
// order to achieve runtime of O(log(n)) instead of O(n).
//
/*
std::list<XmlSerializableRegister*>* XmlSerializable::getAllClasses() {
  static std::list<XmlSerializableRegister*>* const allClasses = new std::list<XmlSerializableRegister*>();
  return allClasses;
}
*/

//----------------------------------------------------------------------------

XmlSerializableRegister::XmlSerializableRegister(const Identifier& identifier, 
    XmlSerializable* (*creatorMethod)(void))
    : identifier_(identifier)
{
    XmlSerializable::registerNewClass(identifier, creatorMethod);
}

const Identifier& XmlSerializableRegister::getClassIdentifier() const {
    return identifier_;
}

}   // namespace
