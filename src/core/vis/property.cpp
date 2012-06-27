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

#include "voreen/core/vis/property.h"


#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"


namespace voreen {

const Identifier Property::noneIdentifier("no_Ident");



Property::Property() : Serializable(), owner_(0) {
	type_ = Property::NONE_PROP;
	msgIdent_ = noneIdentifier;
	guiText_ = "empty";
    destination_ = Message::all_;
	conditioned_ = false;
	condValues_.clear();
	condValues_.push_back(0);
	isCondController_ = false;
	visible_ = true;
    autoChange_ = false;
    grouped_ = false;
}

Property::~Property() {};

void Property::setMsgDestination(Identifier dest) {
    destination_ = dest;
}

Identifier Property::getMsgDestination() const {
    return destination_;
}

void Property::setConditioned(Identifier condController, int condValue) {
    condValues_.clear();
    condValues_.push_back(condValue);
    conditioned_ = true;
    condControllerIdent_ = condController;
}

void Property::setAutoChange(bool state) {
    autoChange_ = state;
}

bool Property::getAutoChange() const {
    return autoChange_;
}

void Property::setConditioned(Identifier condController, std::vector<int> condValues) {
    condValues_.clear();
    for (unsigned int i=0; i< condValues.size(); ++i) {
        condValues_.push_back(condValues.at(i));
    }
    conditioned_ = true;
    condControllerIdent_ = condController;
}

bool Property::isConditioned() const {
	return conditioned_;
}

const std::vector<int>& Property::getCondValues() const {
    return condValues_;
}

void Property::setCondControllerProp(ConditionProp *prop) {
    cond_.push_back(prop);
}

Identifier Property::getCondControllerIdent() const {
    return condControllerIdent_;
}

bool Property::isConditionController() const {
    return isCondController_;
}

void Property::setIsConditionController(bool is) {
    isCondController_ = is;
}

void Property::setVisible(bool state) {
    visible_=state;
}

bool Property::isVisible() const {
    return visible_;
}

std::string Property::getGuiText() const {
    return guiText_;
}

Property::types Property::getType() const {
    return type_;
}

Identifier Property::getIdent() const {
    return msgIdent_;
}

const std::vector<ConditionProp*>& Property::getCondController() const {
    return cond_;
}

void Property::setGrouped(Identifier group) {
    grouped_ = true;
    group_ = group;
}

Identifier Property::getGroup() const {
    return group_;
}

bool Property::isGrouped() const {
    return grouped_;
}

const std::string Property::XmlElementName = "Property";

std::string Property::getXmlElementName() const {
    return XmlElementName;
}

const Identifier Property::getIdent(TiXmlElement* propertyElem) {
    if (!propertyElem)
        throw XmlElementException("Can't get Ident of Null-Pointer!");
    if (propertyElem->Value() != XmlElementName)
        throw XmlElementException(std::string("Cant get ClassName of a ") + propertyElem->Value() + " - need " + XmlElementName + "!");
    Identifier ident(propertyElem->Attribute("id"));
    return ident;
}

TiXmlElement* Property::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = new TiXmlElement(XmlElementName);
    
    //propElem->SetAttribute("Property_text" , getGuiText());
    propElem->SetAttribute("id", getIdent().getName());
    propElem->SetAttribute("type", PropertyType());
    if (getAutoChange())
        propElem->SetAttribute("autochange", getAutoChange() ? "true" : "false"); // Yeah - check twice because thats cool. :P
    
    return propElem;
}

void Property::updateFromXml(TiXmlElement* propElem) {
    errors_.clear();
    serializableSanityChecks(propElem);

    if (propElem->Attribute("type") != PropertyType()) {
        std::stringstream errormsg;
        errormsg << "You are trying to deserialize a " << propElem->Attribute("type") << " when you expectet a " << PropertyType() << "!";
        throw XmlAttributeException(errormsg.str());
    }
    if (propElem->Attribute("autochange"))
        setAutoChange(std::string("true").compare(propElem->Attribute("autochange")) == 0 ? true : false); // else leave default

}

void Property::setOwner(Processor* owner)
{
    owner_ = owner;
}

Processor* Property::getOwner() const
{
    return owner_;
}

/* -------------------------------------------------------------------------------------------- */

ConditionProp::ConditionProp(Identifier ident, Property* condi) {
	condIdent_ = ident;
	msgIdent_ = ident;
    if (condi->getType() == Property::CONDITION_PROP)
        condi = dynamic_cast<ConditionProp*>(condi)->getCondControllerProp();
	condControllerProp_ = condi;
	conditioned_ = true;
	condi->setCondControllerProp(this);
	condi->setIsConditionController(true);
	type_ = Property::CONDITION_PROP;
    setSerializable(false);
}

int ConditionProp::translateCondition(Property* prop) {
	switch (prop->getType()) {
		case Property::FLOAT_PROP :
			return static_cast<int>((dynamic_cast<FloatProp*>(prop))->get());
		case Property::INT_PROP :
			return (dynamic_cast<IntProp*>(prop))->get();
		case Property::BOOL_PROP :
			if ((dynamic_cast<BoolProp*>(prop))->get())
                return 1;
			else
                return 0;
		case Property::ENUM_PROP :
			return (dynamic_cast<EnumProp*>(prop))->get();
		default:
			return 0;
	}
}

Property* ConditionProp::getCondControllerProp() {
	return condControllerProp_;
}

void ConditionProp::setVisState(Property* prop, bool state) {
	switch(prop->getType()) {
        case Property::FLOAT_PROP:
            (dynamic_cast<FloatProp*>(prop))->setVisible(state);
            break;
        case Property::BOOL_PROP:
            (dynamic_cast<BoolProp*>(prop))->setVisible(state);
            break;
        case Property::BUTTON_PROP:
            (dynamic_cast<ButtonProp*>(prop))->setVisible(state);
            break;
        case Property::COLOR_PROP:
            (dynamic_cast<ColorProp*>(prop))->setVisible(state);
            break;
        case Property::ENUM_PROP:
            (dynamic_cast<EnumProp*>(prop))->setVisible(state);
            break;
        case Property::FILEDIALOG_PROP:
            (dynamic_cast<FileDialogProp*>(prop))->setVisible(state);
            break;
        case Property::INT_PROP:
            (dynamic_cast<IntProp*>(prop))->setVisible(state);
            break;
        case Property::CONDITION_PROP:
            (dynamic_cast<ConditionProp*>(prop))->setVisible(state);
            break;
        case Property::FLOAT_VEC2_PROP:
            (dynamic_cast<FloatVec2Prop*>(prop))->setVisible(state);
            break;
        case Property::FLOAT_VEC3_PROP:
            (dynamic_cast<FloatVec3Prop*>(prop))->setVisible(state);
            break;
        case Property::FLOAT_VEC4_PROP:
            (dynamic_cast<FloatVec4Prop*>(prop))->setVisible(state);
            break;
        case Property::INTEGER_VEC2_PROP:
            (dynamic_cast<IntVec2Prop*>(prop))->setVisible(state);
            break;
        case Property::INTEGER_VEC3_PROP:
            (dynamic_cast<IntVec3Prop*>(prop))->setVisible(state);
            break;
        case Property::INTEGER_VEC4_PROP:
            (dynamic_cast<IntVec4Prop*>(prop))->setVisible(state);
            break;
        default:
            break;
	}
}

void ConditionProp::setVisible(bool state) {
    visible_ = state;
    setVisState(condControllerProp_,state);
    conditionChanged();
}

void ConditionProp::conditionChanged() {
	int val = translateCondition(condControllerProp_);
	std::vector<int> vals;
	for (size_t i=0; i < props_.size(); ++i) {
		bool make_visible = false;
		vals = props_.at(i)->getCondValues();
		for (size_t j=0 ; j < vals.size(); ++j) {
            if (vals.at(j) == val && visible_)
                make_visible = true;
		}
		setVisState(props_.at(i),make_visible);
	}
}

Identifier ConditionProp::getCondIdent() {
	return condIdent_;
}

std::vector<Property*> ConditionProp::getCondProps() {
    return props_;
}

void ConditionProp::addCondProp(Property *prop) {
	props_.push_back(prop);
	bool make_visible = false;
	std::vector<int> vals = prop->getCondValues();
	for (size_t j=0 ; j < vals.size(); ++j) {
		if (translateCondition(condControllerProp_) == vals.at(j))
            make_visible = true;
	}
	setVisState(prop,make_visible);
}

/* -------------------------------------------------------------------------------------------- */

GroupProp::GroupProp(Identifier ident, std::string guiText) {
    groupIdent_ = ident;
    guiText_ = guiText;
    type_ = Property::GROUP_PROP;
    setSerializable(false);
}

Identifier GroupProp::getGroupIdent() {
    return groupIdent_;
}

std::vector<Property*> GroupProp::getGroupedProps() {
    return props_;
}

void GroupProp::addGroupedProp(Property* prop) {
    if (prop->getMsgDestination() == destination_) //connect only the correct props
        props_.push_back(prop);
}

/* -------------------------------------------------------------------------------------------- */

template<class T>
void TemplateProperty<T>::distributeChanges() {
    // FIXME: Cannot use MsgDistr as Processors are not registered - also look at EnumProp::distrChanges()
    //tgt::Singleton<MessageDistributor>::getRef().postMessage(new TemplateMessage<T>(getIdent(), get()), getMsgDestination());
    if (owner_) {
        owner_->postMessage(new TemplateMessage<T>(getIdent(), get()));
    }
}

/* -------------------------------------------------------------------------------------------- */

FloatProp::FloatProp(Identifier ident, std::string guiText, float value, float minValue,
                     float maxValue, bool instantValueChange)
    : TemplateProperty<float>(value)
    , decimals_(2)
{
	msgIdent_ = ident;
	guiText_ = guiText;
    minValue_ = minValue;
    maxValue_ = maxValue;
    instantValueChange_ = instantValueChange;
	type_ = Property::FLOAT_PROP;
}

FloatProp::FloatProp(Identifier ident, std::string guiText, bool *needRecompileShader, float value,
                     float minValue, float maxValue, bool instantValueChange)
    : TemplateProperty<float>(value, needRecompileShader)
    , decimals_(2)
{
	msgIdent_ = ident;
	guiText_ = guiText;
    minValue_ = minValue;
    maxValue_ = maxValue;
    instantValueChange_ = instantValueChange;
	type_ = Property::FLOAT_PROP;
}

float FloatProp::getMinValue() const {
    return minValue_;
}

void FloatProp::setMinValue(const float min) {
    minValue_ = min;
}

float FloatProp::getMaxValue() const {
    return maxValue_;
}

void FloatProp::setMaxValue(const float max) {
    maxValue_ = max;
}

void FloatProp::setDecimals(int decimals) {
    decimals_ = decimals;
}

int FloatProp::getDecimals() const {
    return decimals_;
}

bool FloatProp::getInstantValueChangeState() const {
    return instantValueChange_;
}

void FloatProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    float value;
    if (propElem->QueryFloatAttribute("value", &value) == TIXML_SUCCESS)
        set(value);
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* FloatProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("value", get());
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

IntProp::IntProp(Identifier ident, std::string guiText, int value, int minValue, int maxValue, bool instantValueChange) : TemplateProperty<int>(value) {
	msgIdent_ = ident;
	guiText_ = guiText;
    minValue_ = minValue;
    maxValue_ = maxValue;
    instantValueChange_ = instantValueChange;
	type_ = Property::INT_PROP;
}

IntProp::IntProp(Identifier ident, std::string guiText, bool *needRecompileShader, int value, int minValue, int maxValue, bool instantValueChange) : TemplateProperty<int>(value, needRecompileShader) {
	msgIdent_ = ident;
	guiText_ = guiText;
    minValue_ = minValue;
    maxValue_ = maxValue;
    instantValueChange_ = instantValueChange;
	type_ = Property::INT_PROP;
}

int IntProp::getMinValue() const {
    return minValue_;
}

void IntProp::setMinValue(const int minValue)
{
    minValue_ = minValue;
}

int IntProp::getMaxValue() const {
    return maxValue_;
}

void IntProp::setMaxValue(const int maxValue)
{
    maxValue_ = maxValue;
}

bool IntProp::getInstantValueChangeState() const {
    return instantValueChange_;
}

void IntProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    int value;
    if (propElem->QueryIntAttribute("value", &value) == TIXML_SUCCESS)
        set(value);
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* IntProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", get());
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

BoolProp::BoolProp(Identifier ident, std::string guiText, bool value) : TemplateProperty<bool>(value) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::BOOL_PROP;
}

BoolProp::BoolProp(Identifier ident, std::string guiText, bool *needRecompileShader, bool value) : TemplateProperty<bool>(value, needRecompileShader) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::BOOL_PROP;
}


void BoolProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value"))
        set(std::string("true").compare(propElem->Attribute("value")) == 0 ? true : false);
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* BoolProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", get() ? "true" : "false");
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

StringProp::StringProp(Identifier ident, std::string guiText, std::string value) : TemplateProperty<std::string>(value) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::STRING_PROP;
}

StringProp::StringProp(Identifier ident, std::string guiText, bool *needRecompileShader, std::string value) : TemplateProperty<std::string>(value, needRecompileShader) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::STRING_PROP;
}

void StringProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value"))
        set(propElem->Attribute("value"));
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* StringProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", get());
    return propElem;
}
/* -------------------------------------------------------------------------------------------- */

StringVectorProp::StringVectorProp(Identifier ident, std::string guiText, std::vector<std::string> value) : TemplateProperty<std::vector<std::string> >(value) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::STRINGVECTOR_PROP;
}

StringVectorProp::StringVectorProp(Identifier ident, std::string guiText, bool *needRecompileShader, std::vector<std::string> value) : TemplateProperty<std::vector<std::string> >(value, needRecompileShader) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::STRINGVECTOR_PROP;
}

/* -------------------------------------------------------------------------------------------- */

ColorProp::ColorProp(Identifier ident, std::string guiText, tgt::Color value) : TemplateProperty<tgt::vec4>(value) {
	msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::COLOR_PROP;
}

ColorProp::ColorProp(Identifier ident, std::string guiText, bool *needRecompileShader, tgt::Color value) : TemplateProperty<tgt::Color>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
	type_ = Property::COLOR_PROP;
}

void ColorProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    float r,g,b,a;
    if (propElem->QueryFloatAttribute("r", &r) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("g", &g) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("b", &b) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("a", &a) == TIXML_SUCCESS)
        set(tgt::Color(r,g,b,a));
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* ColorProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("r", get().r);
    propElem->SetDoubleAttribute("g", get().g);
    propElem->SetDoubleAttribute("b", get().b);
    propElem->SetDoubleAttribute("a", get().a);
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

EnumProp::EnumProp(Identifier ident, std::string guiText, std::vector<std::string>& value, int startindex,  bool sendStringMsg) : TemplateProperty<int>(startindex) {
    msgIdent_ = ident;
    guiText_ = guiText;
    strings_ = value;
    type_ = Property::ENUM_PROP;
    sendStringMsg_ = sendStringMsg;
}

EnumProp::EnumProp(Identifier ident, std::string guiText, std::vector<std::string>& value, bool *needRecompileShader, int startindex, bool sendStringMsg) : TemplateProperty<int>(startindex, needRecompileShader) {
    msgIdent_ = ident;
    guiText_ = guiText;
    strings_ = value;
    type_ = Property::ENUM_PROP;
    sendStringMsg_ = sendStringMsg;
}

std::vector<std::string> EnumProp::getStrings() {
    return strings_;
}

void EnumProp::setStrings(const std::vector<std::string>& strings) {
    strings_ = strings;
}

bool EnumProp::getSendStringMsg() {
    return sendStringMsg_;
}

void EnumProp::distributeChanges() {
    if (getSendStringMsg()) { 
        //FIXME: look at Property::distrChanges()
        //tgt::Singleton<MessageDistributor>::getRef().postMessage(
        //    new StringMsg(getIdent(),getStrings().at(get())), getMsgDestination()
        //); 
        if (owner_) {
            owner_->postMessage(
                new StringMsg(getIdent(),getStrings().at(get()))
            );
        }
    }
    else { // call superclass method
        TemplateProperty<int>::distributeChanges();
    }
}

void EnumProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value")) {
        for (size_t j = 0; j < strings_.size(); j++) {
            if (strings_.at(j).compare(propElem->Attribute("value")) == 0) {
                set(j); // The compiler might complain about size_t to int conversion...
                break;
            }
        }
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* EnumProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", strings_.at(get()));
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

ButtonProp::ButtonProp(voreen::Identifier ident, std::string guiText,
                       std::string buttonText, std::string iconFilename)
    : TemplateProperty<bool>()
{
    msgIdent_ = ident;
    guiText_ = guiText;
    buttonText_ = buttonText;
    iconFilename_ = iconFilename;
    type_ = Property::BUTTON_PROP;
}

std::string ButtonProp::getButtonText() {
    return buttonText_;
}

std::string ButtonProp::getIconFilename() {
    return iconFilename_;
}

TiXmlElement* ButtonProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

FileDialogProp::FileDialogProp(voreen::Identifier ident, std::string guiText,
                               std::string /*dialogCaption*/, std::string /*directory*/,
                               std::string /*fileFilter*/)
    : TemplateProperty<std::string>()
{
    msgIdent_ = ident;
	guiText_ = guiText;
    type_ = Property::FILEDIALOG_PROP;
}

std::string FileDialogProp::getDialogCaption() const {
    return dialogCaption_;
}

std::string FileDialogProp::getDirectory() const {
    return directory_;
}

std::string FileDialogProp::getFileFilter() const {
    return fileFilter_;
}

TiXmlElement* FileDialogProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", get());
    propElem->SetAttribute("caption", getDialogCaption());
    propElem->SetAttribute("directory", getDirectory());
    propElem->SetAttribute("filefilter", getFileFilter());
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

TransFuncAlphaProp::TransFuncAlphaProp(voreen::Identifier ident, std::string guiText, TransFunc* tf, const std::string& yAxisText)
    : TemplateProperty<TransFunc*>(tf)
    , yAxisText_(yAxisText)
{
    msgIdent_ = ident;
	guiText_ = guiText;
    type_ = Property::TRANSFUNC_ALPHA_PROP;
}

TransFuncAlphaProp::TransFuncAlphaProp(voreen::Identifier ident, std::string guiText, bool* needRecompileShader, TransFunc* tf, const std::string& yAxisText)
    : TemplateProperty<TransFunc*>(tf, needRecompileShader)
    , yAxisText_(yAxisText)
{
    msgIdent_ = ident;
	guiText_ = guiText;
    type_ = Property::TRANSFUNC_ALPHA_PROP;
}

/* -------------------------------------------------------------------------------------------- */

TransFuncProp::TransFuncProp(voreen::Identifier ident, std::string guiText, TransFunc* tf, bool showThreshold) : TemplateProperty<TransFunc*>(tf) {
    msgIdent_ = ident;
	guiText_ = guiText;
    showThreshold_ = showThreshold;
    type_ = Property::TRANSFUNC_PROP;
}

TransFuncProp::TransFuncProp(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, TransFunc* tf, bool showThreshold) : TemplateProperty<TransFunc*>(tf, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    showThreshold_ = showThreshold;
    type_ = Property::TRANSFUNC_PROP;
}

bool TransFuncProp::getShowThreshold() {
    return showThreshold_;
}

void TransFuncProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    TransFuncIntensity* tf = new TransFuncIntensity();
    tf->clearKeys();

    //iterate through all markers
    TiXmlElement* markerElem;
    for (markerElem = propElem->FirstChildElement("Marker");
    markerElem;
    markerElem = markerElem->NextSiblingElement("Marker"))
    {
        //first get the color
        float value, dest;
        tgt::col4 color;
        tgt::ivec4 tmp;
        if (markerElem->QueryFloatAttribute("source", &value) == TIXML_SUCCESS &&
            markerElem->QueryFloatAttribute("dest", &dest) == TIXML_SUCCESS &&
            markerElem->QueryIntAttribute("r", &tmp.r) == TIXML_SUCCESS &&
            markerElem->QueryIntAttribute("g", &tmp.g) == TIXML_SUCCESS &&
            markerElem->QueryIntAttribute("b", &tmp.b) == TIXML_SUCCESS &&
            markerElem->QueryIntAttribute("a", &tmp.a) == TIXML_SUCCESS)
        {
            color.r = (uint8_t) tmp.r;
            color.g = (uint8_t) tmp.g;
            color.b = (uint8_t) tmp.b;
            color.a = (uint8_t) tmp.a;
            TransFuncMappingKey* myKey = new TransFuncMappingKey(value, color);
            myKey->setAlphaL(dest);
            if (markerElem->QueryFloatAttribute("splitdest", &dest) == TIXML_SUCCESS &&
                markerElem->QueryIntAttribute("splitr", &tmp.r) == TIXML_SUCCESS &&
                markerElem->QueryIntAttribute("splitg", &tmp.g) == TIXML_SUCCESS &&
                markerElem->QueryIntAttribute("splitb", &tmp.b) == TIXML_SUCCESS &&
                markerElem->QueryIntAttribute("splita", &tmp.a) == TIXML_SUCCESS)
            {
                myKey->setSplit(true);
                color.r = (uint8_t) tmp.r;
                color.g = (uint8_t) tmp.g;
                color.b = (uint8_t) tmp.b;
                color.a = (uint8_t) tmp.a;
                myKey->setColorR(color);
                myKey->setAlphaR(dest);
            } else {
                myKey->setSplit(false);
            }
            tf->addKey(myKey);
        }
        else
            errors_.store(XmlElementException("A Key in a TransFunc is messed up"));
    } // for ( pElem; pElem; pElem=pElem->NextSiblingElement())
    tf->updateTexture();
    set(tf);
    // processor->postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf) );
    // Hope this does the same... 
    distributeChanges();
}

TiXmlElement* TransFuncProp::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    TransFuncIntensity* tf = dynamic_cast<TransFuncIntensity*>(get());
    if (tf) {
        // iterate through all markers
        for (int m = 0; m < tf->getNumKeys(); ++m) {
            // prepare xml
            TiXmlElement *xmlMarker = new TiXmlElement("Marker");
            //save markers to xml
            xmlMarker->SetDoubleAttribute("source", tf->getKey(m)->getIntensity());
            xmlMarker->SetDoubleAttribute("dest", tf->getKey(m)->getAlphaL());
            xmlMarker->SetAttribute("r", tf->getKey(m)->getColorL().r);
            xmlMarker->SetAttribute("g", tf->getKey(m)->getColorL().g);
            xmlMarker->SetAttribute("b", tf->getKey(m)->getColorL().b);
            xmlMarker->SetAttribute("a", tf->getKey(m)->getColorL().a);
            if (tf->getKey(m)->isSplit()) {
                xmlMarker->SetDoubleAttribute("splitdest", tf->getKey(m)->getAlphaR());
                xmlMarker->SetAttribute("splitr", tf->getKey(m)->getColorR().r);
                xmlMarker->SetAttribute("splitg", tf->getKey(m)->getColorR().g);
                xmlMarker->SetAttribute("splitb", tf->getKey(m)->getColorR().b);
                xmlMarker->SetAttribute("splita", tf->getKey(m)->getColorR().a);
            }
            propElem->LinkEndChild(xmlMarker);
        } // for (size_t m=0; m<transfer->getNumKeys(); ++m)
    }

    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

FloatVec2Prop::FloatVec2Prop(voreen::Identifier ident, std::string guiText, tgt::vec2 value, tgt::vec2 minimum, tgt::vec2 maximum) : TemplateProperty<tgt::vec2>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC2_PROP;
}

FloatVec2Prop::FloatVec2Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::vec2 value, tgt::vec2 minimum, tgt::vec2 maximum) : TemplateProperty<tgt::vec2>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC2_PROP;
}

tgt::vec2 FloatVec2Prop::getMinimum() const {
    return minimum_;
}

tgt::vec2 FloatVec2Prop::getMaximum() const {
    return maximum_;
}

void FloatVec2Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::vec2 vector;
    if (propElem->QueryFloatAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("y", &vector.y) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* FloatVec2Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("x", get().x);
    propElem->SetDoubleAttribute("y", get().y);
    return propElem;
}
/* -------------------------------------------------------------------------------------------- */

FloatVec3Prop::FloatVec3Prop(voreen::Identifier ident, std::string guiText, tgt::vec3 value, tgt::vec3 minimum, tgt::vec3 maximum) : TemplateProperty<tgt::vec3>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC3_PROP;
}

FloatVec3Prop::FloatVec3Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::vec3 value, tgt::vec3 minimum, tgt::vec3 maximum) : TemplateProperty<tgt::vec3>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC3_PROP;
}

tgt::vec3 FloatVec3Prop::getMinimum() const {
    return minimum_;
}

tgt::vec3 FloatVec3Prop::getMaximum() const {
    return maximum_;
}

void FloatVec3Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::vec3 vector;
    if (propElem->QueryFloatAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("y", &vector.y) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("z", &vector.z) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* FloatVec3Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("x", get().x);
    propElem->SetDoubleAttribute("y", get().y);
    propElem->SetDoubleAttribute("z", get().z);
    return propElem;
}
/* -------------------------------------------------------------------------------------------- */

FloatVec4Prop::FloatVec4Prop(voreen::Identifier ident, std::string guiText, tgt::vec4 value, tgt::vec4 minimum, tgt::vec4 maximum) : TemplateProperty<tgt::vec4>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC4_PROP;
}

FloatVec4Prop::FloatVec4Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::vec4 value, tgt::vec4 minimum, tgt::vec4 maximum) : TemplateProperty<tgt::vec4>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::FLOAT_VEC4_PROP;
}

tgt::vec4 FloatVec4Prop::getMinimum() const {
    return minimum_;
}

tgt::vec4 FloatVec4Prop::getMaximum() const {
    return maximum_;
}

void FloatVec4Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::vec4 vector;
    if (propElem->QueryFloatAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("y", &vector.y) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("z", &vector.z) == TIXML_SUCCESS &&
        propElem->QueryFloatAttribute("z", &vector.w) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* FloatVec4Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetDoubleAttribute("x", get().x);
    propElem->SetDoubleAttribute("y", get().y);
    propElem->SetDoubleAttribute("z", get().z);
    propElem->SetDoubleAttribute("w", get().w);
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

IntVec2Prop::IntVec2Prop(voreen::Identifier ident, std::string guiText, tgt::ivec2 value, tgt::ivec2 minimum, tgt::ivec2 maximum) : TemplateProperty<tgt::ivec2>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC2_PROP;
}

IntVec2Prop::IntVec2Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::ivec2 value, tgt::ivec2 minimum, tgt::ivec2 maximum) : TemplateProperty<tgt::ivec2>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC2_PROP;
}

tgt::ivec2 IntVec2Prop::getMinimum() const {
    return minimum_;
}

tgt::ivec2 IntVec2Prop::getMaximum() const {
    return maximum_;
}

void IntVec2Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::ivec2 vector;
    if (propElem->QueryIntAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("y", &vector.y) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* IntVec2Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("x", get().x);
    propElem->SetAttribute("y", get().y);
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

IntVec3Prop::IntVec3Prop(voreen::Identifier ident, std::string guiText, tgt::ivec3 value, tgt::ivec3 minimum, tgt::ivec3 maximum) : TemplateProperty<tgt::ivec3>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC3_PROP;
}

IntVec3Prop::IntVec3Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::ivec3 value, tgt::ivec3 minimum, tgt::ivec3 maximum) : TemplateProperty<tgt::ivec3>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC3_PROP;
}

tgt::ivec3 IntVec3Prop::getMinimum() const {
    return minimum_;
}

tgt::ivec3 IntVec3Prop::getMaximum() const {
    return maximum_;
}

void IntVec3Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::ivec3 vector;
    if (propElem->QueryIntAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("y", &vector.y) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("z", &vector.z) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* IntVec3Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("x", get().x);
    propElem->SetAttribute("y", get().y);
    propElem->SetAttribute("z", get().z);
    return propElem;
}

/* -------------------------------------------------------------------------------------------- */

IntVec4Prop::IntVec4Prop(voreen::Identifier ident, std::string guiText, tgt::ivec4 value, tgt::ivec4 minimum, tgt::ivec4 maximum) : TemplateProperty<tgt::ivec4>(value) {
    msgIdent_ = ident;
	guiText_ = guiText;
	minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC4_PROP;
}

IntVec4Prop::IntVec4Prop(voreen::Identifier ident, std::string guiText, bool *needRecompileShader, tgt::ivec4 value, tgt::ivec4 minimum, tgt::ivec4 maximum) : TemplateProperty<tgt::ivec4>(value, needRecompileShader) {
    msgIdent_ = ident;
	guiText_ = guiText;
    minimum_ = minimum;
	maximum_ = maximum;
    type_ = Property::INTEGER_VEC4_PROP;
}

tgt::ivec4 IntVec4Prop::getMinimum() const {
    return minimum_;
}

tgt::ivec4 IntVec4Prop::getMaximum() const {
    return maximum_;
}

void IntVec4Prop::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);

    tgt::ivec4 vector;
    if (propElem->QueryIntAttribute("x", &vector.x) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("y", &vector.y) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("z", &vector.z) == TIXML_SUCCESS &&
        propElem->QueryIntAttribute("z", &vector.w) == TIXML_SUCCESS) {
        set(vector);
    }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
    distributeChanges();
}

TiXmlElement* IntVec4Prop::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("x", get().x);
    propElem->SetAttribute("y", get().y);
    propElem->SetAttribute("z", get().z);
    propElem->SetAttribute("w", get().w);
    return propElem;
}

} //namespace Voreen

