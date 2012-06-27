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

#ifndef VRN_PROPERTY_H
#define VRN_PROPERTY_H

#include "tgt/vector.h"
#include "voreen/core/vis/identifier.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/xml/serializable.h"
#include "tinyxml/tinyxml.h"

#include <string>
#include <vector>

namespace voreen {

class Processor;
class Identifier;
class ConditionProp;
class TransFunc;

//---------------------------------------------------------------------------

//TemplatePlugin is a class that makes it possible to store a plugin in a Templateproperty
template<class T>
class TemplatePlugin {
public:
    virtual ~TemplatePlugin() {}
    inline virtual void changeValue(T value);
    inline virtual void setVisibleState(bool vis);
    inline virtual void updateFromProperty();
};

template<class T>
void TemplatePlugin<T>::setVisibleState(bool /*vis*/) {};

template<class T>
void TemplatePlugin<T>::changeValue(T /*value*/) {};

template<class T>
void TemplatePlugin<T>::updateFromProperty() {};

//---------------------------------------------------------------------------

/**
 * Base class for properties in the messaging sytem. All data sent with messages is encapsulated
 * in properties.
 */
class Property : public Serializable {
public:
    static const Identifier noneIdentifier;

    /*
        TODO better use type_info here instead Ids for types:
        typedef const std::type_info& PropType;
        typedef std::vector<PropType> PropTypes;
        (roland)
    */
    ///enumeration of available property-types
    enum types{
        NONE_PROP,
        FLOAT_PROP,
        BOOL_PROP,
		INT_PROP,
        COLOR_PROP,
        ENUM_PROP,
        BUTTON_PROP,
        FILEDIALOG_PROP,
        CONDITION_PROP,
        GROUP_PROP,
        TRANSFUNC_PROP,
        TRANSFUNC_ALPHA_PROP,
        FLOAT_VEC2_PROP,
        FLOAT_VEC3_PROP,
        FLOAT_VEC4_PROP,
        INTEGER_VEC2_PROP,
        INTEGER_VEC3_PROP,
        INTEGER_VEC4_PROP,
        STRING_PROP,
		STRINGVECTOR_PROP
    };

    /**
     *Constructor - sets standard-values
     */
    Property();

    virtual ~Property();

    /**
     * Sets the destination where the message of the property is sent to.
     */
    void setMsgDestination(Identifier dest);

    /**
     * Returns the destination-Identifier.
     */
    Identifier getMsgDestination() const;

    /**
     * Returns type of Property.
     */
    types getType() const;
    
    virtual std::string PropertyType() const = 0;

    /**
     * Returns the string that is displayed in the gui.
     */
    std::string getGuiText() const;

    /**
     * Returns true if property is conditionController.
     */
    bool isConditionController() const;

    /**
     *  Sets the property to conditionController.
     */
    void setIsConditionController(bool is);

    /**
     * Sets the ConditionProp the conditionController is associated to.
     */
    void setCondControllerProp(ConditionProp *prop);

    /**
     * Returns the values for which this conditionedProperty is visible.
     */
    const std::vector<int>& getCondValues() const;

    /**
     * Return true if the property is conditioned.
     */
    bool isConditioned() const;

    /**
     * Returns the ConditionProp the conditionController is associated to.
     */
    const std::vector<ConditionProp*>& getCondController() const;

    /**
     * Returns the Identifier the conditionedProp is associated to
     */
    Identifier getCondControllerIdent() const;

    /**
     *
     * @param condValue sets the visibilityvalue
     */
    void setConditioned(Identifier condController, int condValue);

    /**
     * @param condController Identifier of the conditionProperty
     * @param condValue sets the visibilityvalues
     */
    void setConditioned(Identifier condController, std::vector<int> condValue);

    void setVisible(bool state);

    /**
     * Returns visibility state.
     */
    bool isVisible() const;

    /**
     * Sets the property to autoChange-mode. In autochange mode the property automatically
     * handles messages dedicated to it.
     */
    void setAutoChange(bool state);

    /**
     * Returns the current state of autoChange-mode
     */
    bool getAutoChange() const;

    /**
     * Add this property to \a group.
     */
    void setGrouped(Identifier group);

    /**
     * Returns the group-Identifier
     */
    Identifier getGroup() const;

    /**
     * Returns true if property is grouped
     */
    bool isGrouped() const;

    /**
     * Returns the associated identifier of the property
     */
    Identifier getIdent() const;
    const static Identifier getIdent(TiXmlElement* propertyElem);

    static const std::string XmlElementName;
    /**
    * Returns the name of the xml element uses when serializing the object
    */
    virtual std::string getXmlElementName() const;

    /**
     * Serializes the property to XML. Derived classes should implement their own version.
     */
    virtual TiXmlElement* serializeToXml() const;
    
    /**
     * Updates the property from XML. Derived classes should implement their own version.
     */
    virtual void updateFromXml(TiXmlElement* propElem);
    
    void setOwner(Processor* processor);
    Processor* getOwner() const;

 protected:
    ///Type of this property
    Property::types type_;
    ///Identifier that is used for the messaging system
    Identifier msgIdent_;
    ///Destination for the messaging system
    Identifier destination_;
    std::string guiText_;

    //variables for group mode
    /// Is this property grouped.
    bool grouped_;
    /// Group this property is associated to.
    Identifier group_;

    //variables for autoChange mode
    bool autoChange_;

    //variables for conditioned mode
    bool conditioned_;
    std::vector<int> condValues_;
    bool isCondController_;
    std::vector<ConditionProp*> cond_;
    Identifier condControllerIdent_;
    bool visible_;

    Processor* owner_;

};

typedef std::vector<Property*> Properties;

//---------------------------------------------------------------------------

/**
 * Handles a property, that can enable/disable other properties.
 */
class ConditionProp : public Property {
public:
    /**
     *constructor
     *@param ident sets the conditionIdent
     *@param condi sets the condionController
     */
     ConditionProp(Identifier ident, Property* condi);

    virtual std::string PropertyType() const { return "ConditionProp"; }
    static std::string Type() { return "ConditionProp"; }

     /**
      * Adds a conditionedProp.
      */
     void addCondProp(Property* prop);

     /**
      * Returns the conditionController.
      */
     Property* getCondControllerProp();

     /**
      * Returns the conditionIdentifier.
      */
     Identifier getCondIdent();

     /**
      * Sets the right visibility states for the associated conditionedProps.
      */
     void conditionChanged();

     /**
      * Sets visibility state.
      * Only needed if conditionProp is conditioned itself.
      */
     void setVisible(bool state);

     /**
      * Returns associated conditionedProps.
      */
     std::vector<Property*> getCondProps();
protected:
     Property* condControllerProp_;
     void setVisState(Property* prop, bool state);
     int translateCondition(Property* prop);
     bool state_;
     Identifier condIdent_;
     std::vector<Property*> props_;
};

//--------------------------------------------------------------------------

/**
 * Handles grouped properties.
 */
class GroupProp : public Property {
public:
    GroupProp(Identifier ident, std::string guiText);
    virtual std::string PropertyType() const { return "GroupProp"; }
    static std::string Type() { return "GroupProp"; }
    Identifier getGroupIdent();
    std::vector<Property*> getGroupedProps();
    void addGroupedProp(Property* prop);
protected:
    Identifier groupIdent_;
    std::vector<Property*> props_;
};

//--------------------------------------------------------------------------

/**
 * Stores a parameter value and additional information regarding the parameter.
 *
 * If shader recompiling is needed when the parameter changed, pass a pointer
 * to the appropriate shader variable.
 * If the parameter should not be changeable, the isChangeable to false.
 */
template<class T>
class TemplateProperty : public Property {
public:
    TemplateProperty();
    TemplateProperty(const T& value);
    TemplateProperty(const T& value, bool* needRecompileShader);
    inline void init(const T& value);
    inline void init(const T& value, bool* needRecompileShader);
    inline void set(const T& value);
    inline void set(const TemplateProperty<T>& value);
    void set(TemplateMessage<T>& message);

    /**
     * Calls the updateFromProperty() method
     */
    inline void forwardChangesToPlugin();

    /**
     * Sets the callback that updates the gui, if this property has changed.
     */
    inline void setCallback(TemplatePlugin<T>* plugin);

    /**
     * Sets the visibility of this property in the gui.
     */
    inline void setVisible(bool state);
    T get() const;

    /**
     * Distributes the templateproperty's value by using the message system
     */
    inline void distributeChanges();

protected:
    bool affectNeedRecompileShader_;
    bool* needRecompileShader_;
    TemplatePlugin<T>* plugin_;
    T value_;
};

//---------------------------------------------------------------------------

template<class T>
TemplateProperty<T>::TemplateProperty()
  : affectNeedRecompileShader_(false)
  , needRecompileShader_(0)
  , plugin_(0)
{
}

template<class T>
TemplateProperty<T>::TemplateProperty(const T& value)
  : affectNeedRecompileShader_(false)
  , needRecompileShader_(0)
  , plugin_(0)
  , value_(value)
{
}

template<class T>
void TemplateProperty<T>::forwardChangesToPlugin() {
    if (plugin_)
        plugin_->updateFromProperty();
};

template<class T>
TemplateProperty<T>::TemplateProperty(const T& value, bool* needRecompileShader) 
  : affectNeedRecompileShader_(true)
  , needRecompileShader_(needRecompileShader)
  , plugin_(0)
  , value_(value)
{
}

template<class T>
void TemplateProperty<T>::setVisible(bool state) {
    visible_ = state;
    if (plugin_)
        plugin_->setVisibleState(state);
}

template<class T>
void TemplateProperty<T>::init(const T& value) {
    //isChangeable_ = true;
    affectNeedRecompileShader_ = false;
    needRecompileShader_ = 0;
    value_ = value;
}

template<class T>
void TemplateProperty<T>::init(const T& value, bool* needRecompileShader) {
    //isChangeable_ = true;
    affectNeedRecompileShader_ = true;
    needRecompileShader_ = needRecompileShader;
    value_ = value;
}

template<class T>
void TemplateProperty<T>::set(const T& value) {
    if (value_ != value) {
        value_ = value;
        if (plugin_)
            plugin_->changeValue(value_);
        if (affectNeedRecompileShader_)
            *needRecompileShader_ = true;
        if (isCondController_) {
            for (size_t i=0; i < cond_.size(); ++i) {
                cond_.at(i)->conditionChanged();
            }
        }
//            if (owner_)
        //    owner_->invalidate();
    }
}

template<class T>
void TemplateProperty<T>::set(const TemplateProperty<T>& value) {
    value_ = value.value_;
    if (plugin_)
        plugin_->changeValue(value_);
    if (isCondController_) {
        for (size_t i=0; i < cond_.size(); ++i) {
            cond_.at(i)->conditionChanged();
        }
    }
}

template<class T>
void TemplateProperty<T>::set(TemplateMessage<T>& message) {
    set(message.getValue());
}

template<class T>
void TemplateProperty<T>::setCallback(TemplatePlugin<T>* plug) {
    plugin_ = plug;
}

template<class T>
T TemplateProperty<T>::get() const {
    return value_;
}

//------------------------------------------------------------------------------

class FloatProp : public TemplateProperty<float> {
public:
    FloatProp(Identifier ident, std::string guiText, float value = 0.f, float minValue = 0.f, float maxValue = 1.f, bool instantValueChange = false);
    FloatProp(Identifier ident, std::string guiText, bool* needRecompileShader, float value = 0.f, float minValue = 0.f, float maxValue = 1.f, bool instantValueChange = false);

    virtual std::string PropertyType() const { return "FloatProp"; }
    static std::string Type() { return "FloatProp"; }

    float getMinValue() const;
    void setMinValue(const float min);
    float getMaxValue() const;
    void setMaxValue(const float max);
    void setDecimals(int decimals);
    int getDecimals() const;
    bool getInstantValueChangeState() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;

protected:
    float minValue_;
    float maxValue_;
    int decimals_;
    bool instantValueChange_;
};

//------------------------------------------------------------------------------

class IntProp : public TemplateProperty<int> {
public:
    IntProp(Identifier ident, std::string guiText, int value = 0, int minValue = 0, int maxValue = 100, bool instantValueChange = false);
    IntProp(Identifier ident, std::string guiText, bool* needRecompileShader, int value = 0, int minValue = 0, int maxValue = 100, bool instantValueChange = false);

    virtual std::string PropertyType() const { return "IntProp"; }
    static std::string Type() { return "IntProp"; }

    int getMinValue() const;
    int getMaxValue() const;
    bool getInstantValueChangeState() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;

    void setMinValue(const int minValue);
    void setMaxValue(const int maxValue);

protected:
    int minValue_;
    int maxValue_;
    bool instantValueChange_;
};

//------------------------------------------------------------------------------

class BoolProp : public TemplateProperty<bool> {
public:
    BoolProp(Identifier ident, std::string guiText, bool value = false);
    BoolProp(Identifier ident, std::string guiText, bool* needRecompileShader, bool value = false);

    virtual std::string PropertyType() const { return "BoolProp"; }
    static std::string Type() { return "BoolProp"; }

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
};

//------------------------------------------------------------------------------

class StringProp : public TemplateProperty<std::string> {
public:
	StringProp(Identifier ident, std::string guiText, std::string value = "");
	StringProp(Identifier ident, std::string guiText, bool* needRecompileShader, std::string value = "");

    virtual std::string PropertyType() const { return "StringProp"; }
    static std::string Type() { return "StringProp"; }

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
};

//------------------------------------------------------------------------------

class StringVectorProp : public TemplateProperty<std::vector<std::string> > {
public:
	StringVectorProp(Identifier ident, std::string guiText, std::vector<std::string> value);
	StringVectorProp(Identifier ident, std::string guiText, bool* needRecompileShader, std::vector<std::string> value);

    virtual std::string PropertyType() const { return "StringVectorProp"; }
    static std::string Type() { return "StringVectorProp"; }

};

//------------------------------------------------------------------------------

class ColorProp : public TemplateProperty<tgt::vec4> {
public:
    ColorProp(Identifier ident, std::string guiText, tgt::Color value=tgt::Color(0.0));
    ColorProp(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::Color value=tgt::Color(0.0));

    virtual std::string PropertyType() const { return "ColorProp"; }
    static std::string Type() { return "ColorProp"; }

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
};

//------------------------------------------------------------------------------

class EnumProp : public TemplateProperty<int> {
public:
    EnumProp(Identifier ident, std::string guiText, std::vector<std::string>& value, int startindex = 0, bool sendStringMsg = true);
    EnumProp(Identifier ident, std::string guiText, std::vector<std::string>& value , bool* needRecompileShader, int startindex = 0, bool sendStringMsg = true);
    std::vector<std::string> getStrings();
    void setStrings(const std::vector<std::string>& strings);
    bool getSendStringMsg();
    void distributeChanges();

    virtual std::string PropertyType() const { return "EnumProp"; }
    static std::string Type() { return "EnumProp"; }

    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    std::vector<std::string> strings_;
    bool sendStringMsg_;
};

//------------------------------------------------------------------------------

class ButtonProp : public TemplateProperty<bool> {
public:
    ButtonProp(Identifier ident, std::string guiText, std::string buttonText, std::string iconFilename = "");

    virtual std::string PropertyType() const { return "ButtonProp"; }
    static std::string Type() { return "ButtonProp"; }

    std::string getButtonText();
    std::string getIconFilename();
    TiXmlElement* serializeToXml() const;
protected:
    std::string buttonText_;
    std::string iconFilename_;
};

//------------------------------------------------------------------------------

class FileDialogProp : public TemplateProperty< std::string > {
public:
    FileDialogProp(Identifier ident, std::string guiText, std::string dialogCaption, std::string directory, std::string fileFilter);

    virtual std::string PropertyType() const { return "FileDialogProp"; }
    static std::string Type() { return "FileDialogProp"; }

    std::string getDialogCaption() const;
    std::string getDirectory() const;
    std::string getFileFilter() const;
    TiXmlElement* serializeToXml() const;
protected:
    std::string dialogCaption_;
    std::string directory_;
    std::string fileFilter_;
};

//---------------------------------------------------------------------------

//control only alpha values of the transferfunction
class TransFuncAlphaProp : public TemplateProperty<TransFunc*> {
public:
    TransFuncAlphaProp(Identifier ident, std::string guiText, TransFunc* tf, const std::string& yAxisText = "intensity");
    TransFuncAlphaProp(Identifier ident, std::string guiText, bool* needRecompileShader, TransFunc* tf, const std::string& yAxisText = "intensity");

    virtual std::string PropertyType() const { return "TransFuncAlphaProp"; }
    static std::string Type() { return "TransFuncAlphaProp"; }

    std::string getYAxisText(){return yAxisText_;};
protected:
    std::string yAxisText_;
};

//------------------------------------------------------------------------------

class TransFuncProp : public TemplateProperty<TransFunc*> {
public:
    TransFuncProp(Identifier ident, std::string guiText, TransFunc* tf, bool showThreshold = true);
    TransFuncProp(Identifier ident, std::string guiText, bool* needRecompileShader, TransFunc* tf, bool showThreshold = true);

    virtual std::string PropertyType() const { return "TransFuncProp"; }
    static std::string Type() { return "TransFuncProp"; }

    bool getShowThreshold();
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    bool showThreshold_;
};

//------------------------------------------------------------------------------

class FloatVec2Prop : public TemplateProperty<tgt::vec2> {
public:
    FloatVec2Prop(Identifier ident, std::string guiText, tgt::vec2 value, tgt::vec2 minimum_ = tgt::vec2(0.0f), tgt::vec2 maximum_ = tgt::vec2(1.0f));
    FloatVec2Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::vec2 value, tgt::vec2 minimum_ = tgt::vec2(0.0f), tgt::vec2 maximum_ = tgt::vec2(1.0f));

    virtual std::string PropertyType() const { return "FloatVec2Prop"; }
    static std::string Type() { return "FloatVec2Prop"; }

    tgt::vec2 getMinimum() const;
    tgt::vec2 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::vec2 minimum_;
    tgt::vec2 maximum_;
};

//------------------------------------------------------------------------------

class FloatVec3Prop : public TemplateProperty<tgt::vec3> {
public:
    FloatVec3Prop(Identifier ident, std::string guiText, tgt::vec3 value, tgt::vec3 minimum_ = tgt::vec3(0.0f), tgt::vec3 maximum_ = tgt::vec3(1.0f));
    FloatVec3Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::vec3 value, tgt::vec3 minimum_ = tgt::vec3(0.0f), tgt::vec3 maximum_ = tgt::vec3(1.0f));

    virtual std::string PropertyType() const { return "FloatVec3Prop"; }
    static std::string Type() { return "FloatVec3Prop"; }

    tgt::vec3 getMinimum() const;
    tgt::vec3 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::vec3 minimum_;
    tgt::vec3 maximum_;
};

//------------------------------------------------------------------------------

class FloatVec4Prop : public TemplateProperty<tgt::vec4> {
public:
    FloatVec4Prop(Identifier ident, std::string guiText, tgt::vec4 value, tgt::vec4 minimum_ = tgt::vec4(0.0f), tgt::vec4 maximum_ = tgt::vec4(1.0f));
    FloatVec4Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::vec4 value, tgt::vec4 minimum_ = tgt::vec4(0.0f), tgt::vec4 maximum_ = tgt::vec4(1.0f));

    virtual std::string PropertyType() const { return "FloatVec4Prop"; }
    static std::string Type() { return "FloatVec4Prop"; }

    tgt::vec4 getMinimum() const;
    tgt::vec4 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::vec4 minimum_;
	tgt::vec4 maximum_;
};

//------------------------------------------------------------------------------

class IntVec2Prop : public TemplateProperty<tgt::ivec2> {
public:
    IntVec2Prop(Identifier ident, std::string guiText, tgt::ivec2 value, tgt::ivec2 minimum_ = tgt::ivec2(0), tgt::ivec2 maximum_ = tgt::ivec2(100));
    IntVec2Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::ivec2 value, tgt::ivec2 minimum_ = tgt::ivec2(0), tgt::ivec2 maximum_ = tgt::ivec2(100));

    virtual std::string PropertyType() const { return "IntVec2Prop"; }
    static std::string Type() { return "IntVec2Prop"; }

    tgt::ivec2 getMinimum() const;
    tgt::ivec2 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::ivec2 minimum_;
    tgt::ivec2 maximum_;
};

//------------------------------------------------------------------------------

class IntVec3Prop : public TemplateProperty<tgt::ivec3> {
public:
    IntVec3Prop(Identifier ident, std::string guiText, tgt::ivec3 value, tgt::ivec3 minimum_ = tgt::ivec3(0), tgt::ivec3 maximum_ = tgt::ivec3(100));
    IntVec3Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::ivec3 value, tgt::ivec3 minimum_ = tgt::ivec3(0), tgt::ivec3 maximum_ = tgt::ivec3(100));

    virtual std::string PropertyType() const { return "IntVec3Prop"; }
    static std::string Type() { return "IntVec3Prop"; }

    tgt::ivec3 getMinimum() const;
    tgt::ivec3 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::ivec3 minimum_;
    tgt::ivec3 maximum_;
};

//------------------------------------------------------------------------------

class IntVec4Prop : public TemplateProperty<tgt::ivec4> {
public:
    IntVec4Prop(Identifier ident, std::string guiText, tgt::ivec4 value, tgt::ivec4 minimum_ = tgt::ivec4(0), tgt::ivec4 maximum_ = tgt::ivec4(100));
    IntVec4Prop(Identifier ident, std::string guiText, bool* needRecompileShader, tgt::ivec4 value, tgt::ivec4 minimum_ = tgt::ivec4(0), tgt::ivec4 maximum_ = tgt::ivec4(100));

    virtual std::string PropertyType() const { return "IntVec4Prop"; }
    static std::string Type() { return "IntVec4Prop"; }

    tgt::ivec4 getMinimum() const;
    tgt::ivec4 getMaximum() const;
    void updateFromXml(TiXmlElement* propElem);
    TiXmlElement* serializeToXml() const;
protected:
    tgt::ivec4 minimum_;
    tgt::ivec4 maximum_;
};

} // namespace voreen

#endif // VRN_PROPERTY_H
