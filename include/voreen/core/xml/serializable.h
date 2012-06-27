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

#ifndef VRN_SERIALIZABLE_H
#define VRN_SERIALIZABLE_H

#include <string>
#include <vector>

#include "voreen/core/vis/exception.h"
#include "tinyxml/tinyxml.h"

namespace voreen {

//-------------------------------------------------------------------------------------------------

/**
 * Generic exception specific to the NetworkSerializer. You should use derrived classed or build
 * your own.
 */
class SerializerException : public VoreenException {
public:
    SerializerException(std::string what = "") : VoreenException(what) {}
};

// TODO build an exception hierarchy that actually makes sense

class NotSerializableException : public SerializerException {
public:
    NotSerializableException(std::string what = "") : SerializerException(what) {}
};

class EmptyNetworkException : public SerializerException {
public:
    EmptyNetworkException(std::string what = "") : SerializerException(what) {}
};

class XmlException : public SerializerException {
public:
    XmlException(std::string what = "") : SerializerException(what) {}
};

class XmlElementException : public XmlException {
public:
    XmlElementException(std::string what = "") : XmlException(what) {}
};

class XmlAttributeException : public XmlException {
public:
    XmlAttributeException(std::string what = "") : XmlException(what) {}
};

//---------------------------------------------------------------------------

/**
 * @brief Collects exceptions and stores them safely.
 *
 * A class that has an ErrorCollector can, in case it catches an exception, simply store
 * it in the ErrorCollector and defer handling to a later point or delegate to another class. 
 */
class ErrorCollector {
public:
    /**
     * Stores one exception.
     * @param e exception to store
     */
    void store(std::exception e);
    
    /**
     * Stores multiple exceptions.
     * @param v vector containing the exceptions to store
     */
    void store(std::vector<std::exception> v);
    
    /**
     * Deletes all stored exceptions.
     */
    void clear();
    
    /**
     * Returns all stored exceptions.
     */
    std::vector<std::exception> errors() const;
    
    /**
     * Returns all stored exceptions and <clear>()s the ErrorCollector.
     */    
    std::vector<std::exception> pop();
    
private:
    std::vector<std::exception> errors_;
};

//---------------------------------------------------------------------------

/**
 * (Nearly an) Interface that classes should implement when they need to be serialized
 * to XML.
 *
 * Has an ErrorCollector.
 */
class Serializable {
public:
    Serializable();
    
    virtual ~Serializable();
    
    /**
     * Returns the Name of the XML element in which the implementer will be serialized
     */
    virtual std::string getXmlElementName() const = 0;
    
    /** 
     * Some instances of serializable classes might not be serializable.
     * Indicate by setting this.
     */
    void setSerializable(const bool serializable = true);

    /** 
     * Indicates wheter the object should be serialized to XML or not.
     * The serializer has to take care of regarding or disregarding this hint.
     */
    bool isSerializable() const;
    
    /**
     * Checks if <isSerializable>() and
     * @param elem is checked for the right XML element name
     */
    void serializableSanityChecks(const TiXmlElement* elem = 0) const;
    
    /**
     * Serializes the element to XML. Derived classes should implement their own version.
     */
    virtual TiXmlElement* serializeToXml() const = 0;
    
    /**
     * Updates the property from XML. Derived classes should implement their own version.
     */
    virtual void updateFromXml(TiXmlElement* elem) = 0;
    
    /**
     * Retrieves the stored errors and clears the ErrorCollector
     */
    std::vector<std::exception> errors();

    /**
     * Retrieves the stored errors without clearing the ErrorCollector
     */
    std::vector<std::exception> getErrors() const;

protected:
    ErrorCollector errors_;

private:
    bool serializable_;
};

//---------------------------------------------------------------------------

/**
 * @brief Encapsulates serializable metadata for anything.
 *
 * If you want your clients to be able to store metadata as XML subtrees in
 * your serializable class, you can use a MetaSerializer and delegate this
 * functionality to it.
 * 
 * Any metadata stored this way can be easily serialized along with
 * your class using the appropriate methods of Serializable.
 */
class MetaSerializer : public Serializable {
public:
    MetaSerializer();
    MetaSerializer(const MetaSerializer& m);
    virtual ~MetaSerializer();

    MetaSerializer& operator=(const MetaSerializer& m);
    
    virtual std::string getXmlElementName() const { return "MetaData"; }

    /**
     * Serializes MetaData to XML.
     */
    virtual TiXmlElement* serializeToXml() const;
    
    /**
     * Updates the Metadata from XML.
     */
    virtual void updateFromXml(TiXmlElement* metaElem);

    /**
     * Adds data as a child to metadata_.
     * Replaces an existing element with the same value (element name)
     */
    void addData(TiXmlElement* data);
    
    /**
     * Removes the child named elemName from metadata_
     */
    void removeData(std::string elemName);
    
    /**
     * Clears the data
     */
    void clearData();
    
    /**
     * returns the child named elemName from metadata_
     */
    TiXmlElement* getData(std::string elemName) const;
    
    /**
     * checks for a child named elemName in metadata_
     */
    bool hasData(std::string elemName) const;

private:
    TiXmlElement* metadata_;
};

} //namespace

#endif // VRN_SERIALIZABLE_H
