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

#ifndef XMLSERIALIZABLE_H
#define XMLSERIALIZABLE_H
#include "tinyxml/tinyxml.h"
#include "voreen/core/vis/identifier.h"

#include <map>
#include <list>
#include <string>

namespace voreen{

//class XmlSerializable;
//class XmlSerializableRegister;

/**
 * A basic xml format:
 *
 * <classIdentifier_ serialId="NUMBER" attribut1="some Value" />
 *
 */

/**
 * Basic interface for serializing a class to/from an TiXmlElement
 * object.
 */
class XmlSerializable{
public:
    enum Exceptions{
        WRONG_IDENTIFIER        //TiXmlElement has wrong classIdentifier, so
                                //we can't deserialize
        , POINTER_NOT_AVAILABLE //can't recreate all pointers. (maybe
                                //the object is not deserialized yet?)
        , ATTRIBUT_ERROR        //An attribut could not be initialized
        , NO_SERIAL_ID
        , NO_SUCH_CLASS
        , NODE_ERROR
    };

    /**
     * Default constructor.
     *
     * @param init. If true - serialId_ will get the next available
     * number and -1 in other case (for later call of deserialize).
     */
    XmlSerializable(bool init = true);

    /**
     * Default destructor. It does not do a lot at this time.
     */
    virtual ~XmlSerializable() {
    }

    /**
     * get serial id of this object. If the object was created by
     * XmlSerializable(false) and was not deserialized yet, the value
     * will be -1.
     */
    int getSerialId() const;

    /**
     * serialize to an TiXmlElement. Pointer to other serialized
     * Elements should be replaced with serialId's. Every child-class
     * has to implement its own version of this function, but the
     * startpoint for TiXmlElement should be generated with this
     * version.
     */
    virtual TiXmlElement* serialize() const;

    /**
     * Every child-class should implement this method, but should run
     * this version first. If this function has been called and did
     * not throw any exceptions, so you can be sure, the param xml is
     * right for you class and the serialId_ is already restored.
     *
     * @param xml - a xml data for this object.
     * @throw WRONG_IDENTIFIER if classIdentifier is wrong
     * @throw ATTRIBUT_ERROR  if an attribut could not be initialized
     */
    virtual void deserialize(const TiXmlElement* xml)
        throw (XmlSerializable::Exceptions);

    /**
     * Init Pointers. You should get all needed Pointers from
     * getPointer function and throw
     * XmlSerializable::POINTER_NOT_AVAILABLE if it's not possible to
     * init all pointers
     */
    virtual void initPointers() throw(XmlSerializable::Exceptions) {}

    /**
     * Get pointer to an serializable object with given id.
     * @param serialId - id of searched object.
     * @return pointer to object or 0, if such an object does not exist.
     */
    static XmlSerializable* getPointer(int serialId);

    /**
     * This function should return an unique Identifier of its
     * class.
     * Implement this method preferably as follows in derived classes:
     * <code>
     * const Identifier& getClassIdentifier() const {
     *       static const Identifier identifier_("MyClassName");
     *       return identifier_;
     * }
     * </code>
     */
    virtual const Identifier& getClassIdentifier() const = 0;

    /**
     * Factory design pattern method: implement on derived
     * classes as follows:
     * <code>
     * virtual XmlSerializable getNewInstance() const {
     *   return new MyClass();
     * }
     * </code>
     */
//virtual XmlSerializable* getNewInstance() const = 0;

    /**
     * Every child class can create a static instance of SubType of
     * XmlSerializableRegister and pass it to this function for
     * providing full automaticaly deserializing.
     */
    static void registerNewClass(const Identifier& classID, XmlSerializable* (*creatorMethod)(void));
    //static void registerNewClass(XmlSerializableRegister* newClass);

    /**
     * You can create a new class of your need just by passing
     * TiXmlElement to this function. If such a class is registered
     * you'll get a new Instance of your class, that is alreade
     * deserialized. If no class with such identifier is registered
     * here, NO_SUCH_CLASS will be thrown.
     */
    static XmlSerializable* getClassFromXml(const TiXmlElement* xml)
        throw (XmlSerializable::Exceptions);
    /**
     * You can create a new class of your need just by passing
     * Identifier to this function. In contrast to getClassFromXml()
     * this method doesn't deserialize
     */
    static XmlSerializable* getClassFromIdentifier(const Identifier id)
        throw (XmlSerializable::Exceptions);
    /**
     * read/write bool attributs. bool attribut are saved in text-form
     * and have values like "true" and "false"
     */
    static void writeBoolAttribute(std::string attributeName, bool value, TiXmlElement* xml);
    static bool readBoolAttribute(std::string attributeName, const TiXmlElement* xml)
        throw (XmlSerializable::Exceptions);

protected:
    /**
     * serial id .
     */
    int serialId_;

private:
    static int idCounter_;
    /**
    * Replace static members with static functions because of
    * static initialization order fiasco look at
    * http://www.parashift.com/c++-faq-lite/ctors.html
    */
    static std::list<XmlSerializable*>* getAllObjects();
    //static std::list<XmlSerializableRegister*>* getAllClasses();

    typedef XmlSerializable* (*creatorMethod_)(void);
    typedef std::map<Identifier, creatorMethod_> ClassMap;
    static ClassMap& getAllClasses();
};

//----------------------------------------------------------------------------

class XmlSerializableRegister {
public:
    XmlSerializableRegister(const Identifier& identifier,
        XmlSerializable* (*creatorMethod)(void));
    const Identifier& getClassIdentifier() const;
private:
    Identifier identifier_;
};

/*
class XmlSerializableRegister {
public:
    XmlSerializableRegister(const Identifier& identifier);
    virtual ~XmlSerializableRegister() {}
    virtual XmlSerializable* getNewInstance() const = 0;
    const Identifier& getClassIdentifier() const;

private:
    Identifier identifier_;
};
*/
}   // namespace

#endif
