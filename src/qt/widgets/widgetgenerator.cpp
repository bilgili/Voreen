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

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/networkanalyzer.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <sstream>

#include "voreen/qt/widgets/transfunc/transfuncplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncalphaplugin.h"
#include "voreen/qt/widgets/widgetgenerator.h"
#include "voreen/qt/widgets/widgetgeneratorplugins.h"

using std::vector;
namespace voreen {

WidgetGenerator::WidgetGenerator(QWidget *parent, MessageReceiver *msgreceiver)
    : WidgetPlugin(parent, msgreceiver)
    , parWidget_(parent)
    , msgReceiver_(msgreceiver)
{
    setObjectName(tr("Visualization Setup"));
    icon_ = QIcon(":/icons/options.png");
}

WidgetGenerator::~WidgetGenerator() {
    for (size_t i = 0; i < overlayPropertyPlugin_list_.size(); ++i) 
        delete overlayPropertyPlugin_list_[i];

    for (size_t i = 0; i < processorPropertyPlugin_list_.size(); ++i) 
        delete processorPropertyPlugin_list_[i];

    ProcessorHeaderWidgets_.clear();
    frames_.clear();
    overlays_.clear();
}

void WidgetGenerator::createWidgets(){
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    ProcessorPropsList* processorPropsList_tmp;
    OverlayPropsList* overlayPropsList_tmp;
    PropsWidgetList* propsWidgetList_tmp;

    // iterate through pipeline
    for (size_t j = 0; j < processor_list.size() ; ++j){
        //get vector of properties from actual processor
        const Properties& props_tmp = processor_list.at(j)->getProperties();
        // save processor (needed for loading and saving)
        processorPropsList_tmp = new ProcessorPropsList();
        processorPropsList_tmp->processor= processor_list.at(j);
        processorPropsList_tmp->state = false;

        // add processor name with checkbox and expansion button
		if (props_tmp.size() > 0) {
            ProcessorHeaderWidget* actPlug = new ProcessorHeaderWidget(parWidget_, processorPropsList_tmp, msgReceiver_, this, props_tmp.size() > 0);
            vboxLayout->addWidget(actPlug);
            ProcessorHeaderWidgets_.push_back(actPlug);
            
            // iterate through all properties of the processor
            for (size_t i = 0; i < props_tmp.size(); ++i){
                QHBoxLayout* hb = new QHBoxLayout();
                //special handling for conditional- and group-properties needed
                if (props_tmp.at(i)->getType() != Property::CONDITION_PROP && !props_tmp.at(i)->isConditioned()
                    && !props_tmp.at(i)->isConditionController() && !props_tmp.at(i)->isGrouped()
                    && props_tmp.at(i)->getType() != Property::GROUP_PROP)
                {
                    propsWidgetList_tmp = makeWidget(props_tmp.at(i),hb);
                    processorPropsList_tmp->propWidg_list_.push_back(propsWidgetList_tmp);
                }
                else {
                    if (props_tmp.at(i)->getType() == Property::GROUP_PROP){
                        makeGroupedWidgets(dynamic_cast<GroupProp*>(props_tmp.at(i)), hb, processorPropsList_tmp);
                    }
                    if (props_tmp.at(i)->isConditionController()){
                        makeConditionedWidgets(props_tmp.at(i), hb, processorPropsList_tmp);
                    }
                }
                vboxLayout->addItem(hb);
            }
        }
        //save widget/property for loading and saving
        processorPropertyPlugin_list_.push_back(processorPropsList_tmp);
    }

    //get properties from active overlays
    for(size_t i = 0; i < overlays_.size(); ++i) {
        const Properties& props_tmp = overlays_.at(i)->getProperties();

        overlayPropsList_tmp = new OverlayPropsList();
        overlayPropsList_tmp->overlay = overlays_.at(i);
        overlayPropsList_tmp->state = false;

        //add checkbox for enabling/disabling the overlay
        if (props_tmp.size() > 0) {
            vboxLayout->addWidget(new OverlayProcessorHeaderWidget(parWidget_, overlayPropsList_tmp, msgReceiver_, this, props_tmp.size() > 0));
        }

        for(unsigned int j=0; j<props_tmp.size();++j){
            QHBoxLayout* hb = new QHBoxLayout();
            //special handling for conditional- and groupproperties needed
            if (props_tmp.at(j)->getType() != Property::CONDITION_PROP && !props_tmp.at(j)->isConditioned()
                && !props_tmp.at(j)->isConditionController() && !props_tmp.at(j)->isGrouped()
                && props_tmp.at(j)->getType() != Property::GROUP_PROP){
                propsWidgetList_tmp = makeWidget(props_tmp.at(j),hb);
                overlayPropsList_tmp->propWidg_list_.push_back(propsWidgetList_tmp);
            }
            else {
                if (props_tmp.at(j)->getType() == Property::GROUP_PROP){
                    makeGroupedWidgets(dynamic_cast<GroupProp*>(props_tmp.at(j)), hb, overlayPropsList_tmp);
                }
                if (props_tmp.at(j)->isConditionController()){
                    makeConditionedWidgets(props_tmp.at(j), hb, overlayPropsList_tmp);
                }
            }
            vboxLayout->addItem(hb);
        }
        overlayPropertyPlugin_list_.push_back(overlayPropsList_tmp);
    }

    vboxLayout->addStretch();

    // textbox for naming the visualization setup
	QWidget* fileNameWidget = new QWidget();
	QHBoxLayout* hboxLayout0 = new QHBoxLayout(fileNameWidget);
	hboxLayout0->setSpacing(1);
	hboxLayout0->setMargin(0);
    hboxLayout0->addWidget(new QLabel(tr("Config Name")));
    configName_ = new QLineEdit(this);
    hboxLayout0->addWidget(configName_);
    QPushButton* loadButton = new QPushButton();
    loadButton->setIcon(QIcon(":/icons/open.png"));
    hboxLayout0->addWidget(loadButton);
    connect(loadButton, SIGNAL(clicked(void)), this, SLOT(loadWidgetSettings()));
    QPushButton* saveButton = new QPushButton();
    saveButton->setIcon(QIcon(":/icons/floppy.png"));
    hboxLayout0->addWidget(saveButton);
    connect (saveButton, SIGNAL(clicked(void)), this, SLOT(saveWidgetSettings()));
    vboxLayout->addWidget(fileNameWidget);

    setLayout(vboxLayout);

    stateChanged();
}

void WidgetGenerator::makeGroupedWidgets(GroupProp* prop, QHBoxLayout* hb, ProcessorPropsList* propsWidgetLists){
    GroupFrame* frame = new GroupFrame(prop, propsWidgetLists, msgReceiver_);
    frame->setTitle(tr(prop->getGuiText().c_str()));
    hb->addWidget(frame);

    QVBoxLayout* newVBox = new QVBoxLayout();
    frame->setLayout(newVBox);

    PropsWidgetList* listItem_tmp;

    frames_.push_back(frame);
    std::vector<Property*> propList = prop->getGroupedProps();
    for (unsigned int j=0; j<propList.size(); ++j){
        QHBoxLayout* newHBox = new QHBoxLayout();
        listItem_tmp = makeWidget(propList.at(j),newHBox);
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        newVBox->addLayout(newHBox);
    }
}

void WidgetGenerator::makeGroupedWidgets(GroupProp* prop, QHBoxLayout* hb, OverlayPropsList* propsWidgetLists){
    GroupFrame* frame = new GroupFrame(prop, propsWidgetLists, msgReceiver_);
    frame->setTitle(tr(prop->getGuiText().c_str()));
    hb->addWidget(frame);

    QVBoxLayout* newVBox = new QVBoxLayout();
    frame->setLayout(newVBox);

    PropsWidgetList* listItem_tmp;

    frames_.push_back(frame);
    std::vector<Property*> propList = prop->getGroupedProps();
    for (unsigned int j=0; j<propList.size(); ++j){
        QHBoxLayout* newHBox = new QHBoxLayout();
        listItem_tmp = makeWidget(propList.at(j),newHBox);
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        newVBox->addLayout(newHBox);
    }
}

void WidgetGenerator::makeConditionedWidgets(Property* prop, QHBoxLayout* hb, ProcessorPropsList* propsWidgetLists){
    GroupFrame* frame = new GroupFrame(prop, propsWidgetLists, msgReceiver_);
    frame->setTitle(tr(prop->getGuiText().c_str()));
    hb->addWidget(frame);

    QVBoxLayout* newVBox = new QVBoxLayout();
    PropsWidgetList* listItem_tmp;

    if(prop->getType() == Property::BOOL_PROP){
        frame->setCheckable(true);
        frame->setChecked((dynamic_cast<BoolProp*>(prop))->get());
        // FIXME this is not deleted
        listItem_tmp = new PropsWidgetList();
        listItem_tmp->prop = prop;
        listItem_tmp->widget = frame;
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        frames_.push_back(frame);
    }
    else{
        QHBoxLayout* hb_tmp = new QHBoxLayout();
        newVBox->addLayout(hb_tmp);
        listItem_tmp = makeWidget(prop,hb_tmp, frame);
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        frames_.push_back(frame);
    }

    std::vector<ConditionProp*> condProps = prop->getCondController();
    for (unsigned int i=0; i<condProps.size(); ++i){
        std::vector<Property*> propList = condProps.at(i)->getCondProps();
        for (unsigned int j=0; j<propList.size(); ++j){
            if (propList.at(j)->getType() != Property::CONDITION_PROP){
                QHBoxLayout* newHBox = new QHBoxLayout();
                if (propList.at(j)->isConditionController()){
                    makeConditionedWidgets(propList.at(j), newHBox, propsWidgetLists);
                } else {
                    listItem_tmp = makeWidget(propList.at(j),newHBox);
                    propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
                }
                newVBox->addLayout(newHBox);
            }
        }
    }
    frame->setLayout(newVBox);
}

void WidgetGenerator::makeConditionedWidgets(Property* prop, QHBoxLayout* hb, OverlayPropsList* propsWidgetLists){
    GroupFrame* frame = new GroupFrame(prop, propsWidgetLists, msgReceiver_);
    frame->setTitle(tr(prop->getGuiText().c_str()));
    hb->addWidget(frame);

    QVBoxLayout* newVBox = new QVBoxLayout();
    PropsWidgetList* listItem_tmp;

    if(prop->getType() == Property::BOOL_PROP){
        frame->setCheckable(true);
        frame->setChecked((dynamic_cast<BoolProp*>(prop))->get());
        listItem_tmp = new PropsWidgetList();
        listItem_tmp->prop = prop;
        listItem_tmp->widget = frame;
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        frames_.push_back(frame);
    }
    else{
        QHBoxLayout* hb_tmp = new QHBoxLayout();
        newVBox->addLayout(hb_tmp);
        listItem_tmp = makeWidget(prop,hb_tmp, frame);
        propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
        frames_.push_back(frame);
    }

    std::vector<ConditionProp*> condProps = prop->getCondController();
    for (unsigned int i=0; i<condProps.size(); ++i){
        std::vector<Property*> propList = condProps.at(i)->getCondProps();
        for (unsigned int j=0; j<propList.size(); ++j){
            if (propList.at(j)->getType() != Property::CONDITION_PROP){
                QHBoxLayout* newHBox = new QHBoxLayout();
                if (propList.at(j)->isConditionController()){
                    makeConditionedWidgets(propList.at(j), newHBox, propsWidgetLists);
                } else {
                    listItem_tmp = makeWidget(propList.at(j),newHBox);
                    propsWidgetLists->propWidg_list_.push_back(listItem_tmp);
                }
                newVBox->addLayout(newHBox);
            }
        }
    }
    frame->setLayout(newVBox);
}

WidgetGenerator::PropsWidgetList* WidgetGenerator::makeWidget(Property* prop, QHBoxLayout* hb, QGroupBox* frame){
    // FIXME this is not deleted
    PropsWidgetList* propertyPluginPair_tmp = new PropsWidgetList();
    //depending on type of property the appropriate case is selected
    //and so the cast in the case is always working
    switch (prop->getType()){
        case Property::FLOAT_PROP : {
            FloatProp* prop1 = dynamic_cast<FloatProp*>(prop);
            AGFloatWidget* fplg1 = new AGFloatWidget(parWidget_,msgReceiver_, prop1);
            if (frame) {
                fplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = fplg1;
            hb->addWidget(fplg1);
            prop1->setCallback(fplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                fplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::INT_PROP : {
			IntProp* prop1 = dynamic_cast<IntProp*>(prop);
            AGIntWidget* iplg1 = new AGIntWidget(parWidget_,msgReceiver_, prop1);
            if (frame){
                iplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = iplg1;
            hb->addWidget(iplg1);
            prop1->setCallback(iplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                iplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::BOOL_PROP : {
            BoolProp* prop1 = dynamic_cast<BoolProp*>(prop);
            AGBoolWidget* bplg1 = new AGBoolWidget(parWidget_, msgReceiver_,prop1);
            if (frame){
                bplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = bplg1;
            hb->addWidget(bplg1);
            prop1->setCallback(bplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                bplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::COLOR_PROP : {
            ColorProp* prop1 = dynamic_cast<ColorProp*>(prop);
            AGColorWidget* cplg1 = new AGColorWidget(parWidget_,msgReceiver_, prop1);
            if (frame){
                cplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = cplg1;
            hb->addWidget(cplg1);
            prop1->setCallback(cplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                cplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::ENUM_PROP : {
            EnumProp* prop1 = dynamic_cast<EnumProp*>(prop);
            AGEnumWidget* enplg1 = new AGEnumWidget(parWidget_,msgReceiver_, prop1);
            if (frame){
                enplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = enplg1;
            prop1->setCallback(enplg1);
            hb->addWidget(enplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                enplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::BUTTON_PROP : {
            ButtonProp* prop1 = dynamic_cast<ButtonProp*>(prop);
            AGButtonWidget* bplg1 = new AGButtonWidget(parWidget_, msgReceiver_, prop1);
            if (frame){
                bplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = bplg1;
            // no setCallback
            hb->addWidget(bplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                bplg1->setVisibleState(prop1->isVisible());
            break;
        }
        case Property::FILEDIALOG_PROP : {
            FileDialogProp* prop1 = dynamic_cast<FileDialogProp*>(prop);
            AGFileDialogWidget* fdplg1 = new AGFileDialogWidget(parWidget_, msgReceiver_, prop1);
            if (frame){
                fdplg1->setFrameControler(frame);
            }
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = fdplg1;
            // no setCallback
            hb->addWidget(fdplg1);
            if (prop1->isConditioned() && !prop1->isVisible())
                fdplg1->setVisibleState(prop1->isVisible());
            break;
        }
		case Property::INTEGER_VEC2_PROP : {
			IntVec2Prop* prop1 = dynamic_cast<IntVec2Prop*>(prop);
			AGIntVec2Widget* ivec2plug = new AGIntVec2Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                ivec2plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = ivec2plug;
			hb->addWidget(ivec2plug);
			prop1->setCallback(ivec2plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                ivec2plug->setVisibleState(prop1->isVisible());
            break;
		}
		case Property::INTEGER_VEC3_PROP : {
			IntVec3Prop* prop1 = dynamic_cast<IntVec3Prop*>(prop);
			AGIntVec3Widget* ivec3plug = new AGIntVec3Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                ivec3plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = ivec3plug;
			hb->addWidget(ivec3plug);
			prop1->setCallback(ivec3plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                ivec3plug->setVisibleState(prop1->isVisible());
            break;
		}
		case Property::INTEGER_VEC4_PROP : {
			IntVec4Prop* prop1 = dynamic_cast<IntVec4Prop*>(prop);
			AGIntVec4Widget* ivec4plug = new AGIntVec4Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                ivec4plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = ivec4plug;
			hb->addWidget(ivec4plug);
			prop1->setCallback(ivec4plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                ivec4plug->setVisibleState(prop1->isVisible());
            break;
		}
   		case Property::FLOAT_VEC2_PROP : {
			FloatVec2Prop* prop1 = dynamic_cast<FloatVec2Prop*>(prop);
			AGFloatVec2Widget* fvec2plug = new AGFloatVec2Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                fvec2plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = fvec2plug;
			hb->addWidget(fvec2plug);
			prop1->setCallback(fvec2plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                fvec2plug->setVisibleState(prop1->isVisible());
            break;
		}
		case Property::FLOAT_VEC3_PROP : {
			FloatVec3Prop* prop1 = dynamic_cast<FloatVec3Prop*>(prop);
			AGFloatVec3Widget* fvec3plug = new AGFloatVec3Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                fvec3plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = fvec3plug;
			hb->addWidget(fvec3plug);
			prop1->setCallback(fvec3plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                fvec3plug->setVisibleState(prop1->isVisible());
            break;
		}
		case Property::FLOAT_VEC4_PROP : {
			FloatVec4Prop* prop1 = dynamic_cast<FloatVec4Prop*>(prop);
			AGFloatVec4Widget* fvec4plug = new AGFloatVec4Widget(parWidget_, msgReceiver_, prop1);
            if (frame){
                fvec4plug->setFrameControler(frame);
            }
			propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = fvec4plug;
			hb->addWidget(fvec4plug);
			prop1->setCallback(fvec4plug);
            if (prop1->isConditioned() && !prop1->isVisible())
                fvec4plug->setVisibleState(prop1->isVisible());
            break;
		}
        case  Property::TRANSFUNC_PROP : {
//             TransFuncProp* prop1 = dynamic_cast<TransFuncProp*>(prop);
//             TransFuncPlugin* tfpl1 = new TransFuncPlugin(parWidget_, msgReceiver_, prop1);
//             tfpl1->createWidgets();
//             tfpl1->createConnections();
//             propertyPluginPair_tmp->prop = prop1;
//             propertyPluginPair_tmp->widget = tfpl1;
//             hb->addWidget(tfpl1);
//             prop1->setCallback(tfpl1);
//             if (prop1->isConditioned() && !prop1->isVisible())
//                 tfpl1->setVisibleState(prop1->isVisible());
//             tfpl1->updateTransferFunction(); // so that processor get a valid TF
            break;
        }
        case  Property::TRANSFUNC_ALPHA_PROP : {
            TransFuncAlphaProp* prop1 = dynamic_cast<TransFuncAlphaProp*>(prop);
            TransFuncAlphaPlugin* tfpl1 = new TransFuncAlphaPlugin(parWidget_, msgReceiver_, prop1,false, QString((prop1->getYAxisText()).c_str()));
            tfpl1->createWidgets();
            tfpl1->createConnections();
            propertyPluginPair_tmp->prop = prop1;
            propertyPluginPair_tmp->widget = tfpl1;
            hb->addWidget(tfpl1);
            //prop1->setCallback(tfpl1);
            if (prop1->isConditioned() && !prop1->isVisible())
                tfpl1->setVisibleState(prop1->isVisible());
            tfpl1->updateTransferFunction(); // so that processor get a valid TF
            break;
        }

        default: break;
    }
    return propertyPluginPair_tmp;
}


void WidgetGenerator::loadPropsFromXML(vector< PropsWidgetList* > propsPlugins_tmp, TiXmlElement* xmlParent) {
    std::string property_text;
    std::string conditionIdentifier_text;
    std::string propText;
	TiXmlElement* pElem;

    pElem = xmlParent->FirstChildElement();
    for ( ; pElem; pElem=pElem->NextSiblingElement()) {

        property_text = pElem->Attribute("Property_text");
        for (size_t j = 0; j < propsPlugins_tmp.size(); j++) {
            if ( propsPlugins_tmp.at(j)->prop->getGuiText() == property_text ) {
                // Found a property in the current pipeline which can get
                // some attributes from xml file

                int property_type;
                pElem->QueryIntAttribute("Property_type", &property_type);

                // first set autochange false
                propsPlugins_tmp.at(j)->prop->setAutoChange(false);

                // set autochange
                int autoChange_tmp;
                pElem->QueryIntAttribute("Autochange", &autoChange_tmp);

                switch (property_type) {
                    case Property::FLOAT_PROP : {
                        try {
                            AGFloatWidget* fplg1 = dynamic_cast<AGFloatWidget*>(propsPlugins_tmp.at(j)->widget);
                            float max;
                            float min;
                            float value;
                            pElem->QueryFloatAttribute("Max_value", &max);
                            pElem->QueryFloatAttribute("Min_value", &min);
                            pElem->QueryFloatAttribute("Value", &value);
                            fplg1->initWidgetFromLoadedSettings(min, max, value);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::INT_PROP : {
                        try {
                            AGIntWidget* iplg1 = dynamic_cast<AGIntWidget*>(propsPlugins_tmp.at(j)->widget);
                            int max;
                            int min;
                            int value;
                            pElem->QueryIntAttribute("Max_value", &max);
                            pElem->QueryIntAttribute("Min_value", &min);
                            pElem->QueryIntAttribute("Value", &value);
                            iplg1->initWidgetFromLoadedSettings(min, max, value);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::BOOL_PROP : {
                        try {
                            pElem->QueryIntAttribute("Property_type", &property_type);
                            // first check if the property is a condition controller
                            // cause then it is a frame widget with checkbox
                            // otherwise normal bool plugin
                            if (propsPlugins_tmp.at(j)->prop->isConditionController()) {
                                GroupFrame* group = dynamic_cast<GroupFrame*>(propsPlugins_tmp.at(j)->widget);
                                int value_tmp;
                                pElem->QueryIntAttribute("Value", &value_tmp);
                                group->initWidgetFromLoadedSettings(value_tmp == 1);
                            }
                            else {
                                AGBoolWidget* bplg1 = dynamic_cast<AGBoolWidget*>(propsPlugins_tmp.at(j)->widget);
                                int value_tmp;
                                pElem->QueryIntAttribute("Value", &value_tmp);
                                bplg1->initWidgetFromLoadedSettings(value_tmp == 1);
                            }
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::COLOR_PROP : {
                        try {
                            AGColorWidget* colorplug = dynamic_cast<AGColorWidget*>(propsPlugins_tmp.at(j)->widget);
                            float color_r, color_g, color_b, color_a;
                            pElem->QueryFloatAttribute("Color_r", &color_r);
                            pElem->QueryFloatAttribute("Color_g", &color_g);
                            pElem->QueryFloatAttribute("Color_b", &color_b);
                            pElem->QueryFloatAttribute("Color_a", &color_a);
                            colorplug->initWidgetFromLoadedSettings(tgt::Color(color_r,color_g,color_b,color_a));
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::ENUM_PROP : {
                        try {
                            AGEnumWidget* eplg1 = dynamic_cast<AGEnumWidget*>(propsPlugins_tmp.at(j)->widget);
                            int value;
                            pElem->QueryIntAttribute("Value", &value);
                            eplg1->initWidgetFromLoadedSettings(value);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::TRANSFUNC_PROP : {
//                         try{
//                             TransFuncPlugin* tflg1 = dynamic_cast<TransFuncPlugin*>(propsPlugins_tmp.at(j)->widget);
//                             TransFuncIntensityKeys* tf = tflg1->getTransferFunc();
//
//                             //iterate through all markers
//                             //first set conditions for the current item if there're some
//                             if (!pElem->NoChildren()) {
//                                 cElem = pElem->FirstChildElement();
//                                 for(; cElem; cElem=cElem->NextSiblingElement()) {
//                                     //first get the color
//                                     float value_tmp;
//                                     int int_tmp;
//                                     tgt::col4 color_tmp;
//                                     cElem->QueryIntAttribute("Color_r", &int_tmp);
//                                     color_tmp.r = (uint8_t) int_tmp;
//                                     cElem->QueryIntAttribute("Color_g", &int_tmp);
//                                     color_tmp.g = (uint8_t) int_tmp;
//                                     cElem->QueryIntAttribute("Color_b", &int_tmp);
//                                     color_tmp.b = (uint8_t) int_tmp;
//                                     cElem->QueryIntAttribute("Color_a", &int_tmp);
//                                     color_tmp.a = (uint8_t) int_tmp;
//
//                                     cElem->QueryFloatAttribute("Source_value", &value_tmp);
//                                     TransFuncMappingKey* myKey = new TransFuncMappingKey(value_tmp, color_tmp);
// //                                     myKey->setSourceValue(value_tmp);
//                                     cElem->QueryFloatAttribute("Dest_left_value", &value_tmp);
//                                     myKey->setAlphaL(value_tmp);
//                                     cElem->QueryIntAttribute("isSplit", &int_tmp);
//                                     if (int_tmp == 1) {
//                                         myKey->setSplit(true);
//
//                                         cElem->QueryFloatAttribute("Dest_right_value", &value_tmp);
//                                         myKey->setAlphaR(value_tmp);
//                                         cElem->QueryIntAttribute("Color_alt_r", &int_tmp);
//                                         color_tmp.r = (uint8_t) int_tmp;
//                                         cElem->QueryIntAttribute("Color_alt_g", &int_tmp);
//                                         color_tmp.g = (uint8_t) int_tmp;
//                                         cElem->QueryIntAttribute("Color_alt_b", &int_tmp);
//                                         color_tmp.b = (uint8_t) int_tmp;
// //                                         cElem->QueryIntAttribute("Color_alt_a", &int_tmp);
// //                                         color_tmp.a = (uint8_t) int_tmp;
//                                         myKey->setColorR(color_tmp);
// 								    } else {
//                                         myKey->setSplit(false);
// 								    }
//                                     // push key to vector
//                                     tf->addKey(myKey);
//                                 } // for( pElem; pElem; pElem=pElem->NextSiblingElement())
//                                 //renderPropertyPlugin_list_[i].second.at(j).first->setConditioned(conditionIdentifier_text, cond_tmp);
//                             } // if (!pElem->NoChildren())
//                         } catch (const std::bad_cast& /*ex*/) {
//                             QMessageBox::warning(this, "Error",
//                                 "A dynamic-cast was not successfull.");
//                         }
                        break;
                    }
                    case Property::INTEGER_VEC2_PROP : {
                        try {
                            AGIntVec2Widget* ivpl1 = dynamic_cast<AGIntVec2Widget*>(propsPlugins_tmp.at(j)->widget);
                            int value_tmp;
                            tgt::ivec2 vector;
                            tgt::ivec2 min;
                            tgt::ivec2 max;
                            pElem->QueryIntAttribute("Vector_x", &value_tmp);
                            vector.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_y", &value_tmp);
                            vector.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_x", &value_tmp);
                            max.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_y", &value_tmp);
                            max.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_x", &value_tmp);
                            min.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_y", &value_tmp);
                            min.y = (uint8_t) value_tmp;
                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::INTEGER_VEC3_PROP : {
                        try {
                            AGIntVec3Widget* ivpl1 = dynamic_cast<AGIntVec3Widget*>(propsPlugins_tmp.at(j)->widget);
                            int value_tmp;
                            tgt::ivec3 vector;
                            tgt::ivec3 min;
                            tgt::ivec3 max;
                            pElem->QueryIntAttribute("Vector_x", &value_tmp);
                            vector.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_y", &value_tmp);
                            vector.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_z", &value_tmp);
                            vector.z = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_x", &value_tmp);
                            max.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_y", &value_tmp);
                            max.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_z", &value_tmp);
                            max.z = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_x", &value_tmp);
                            min.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_y", &value_tmp);
                            min.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_z", &value_tmp);
                            min.z = (uint8_t) value_tmp;
                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::INTEGER_VEC4_PROP : {
                        try {
                            AGIntVec4Widget* ivpl1 = dynamic_cast<AGIntVec4Widget*>(propsPlugins_tmp.at(j)->widget);
                            int value_tmp;
                            tgt::ivec4 vector;
                            tgt::ivec4 min;
                            tgt::ivec4 max;
                            pElem->QueryIntAttribute("Vector_x", &value_tmp);
                            vector.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_y", &value_tmp);
                            vector.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_z", &value_tmp);
                            vector.z = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Vector_w", &value_tmp);
                            vector.w = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_x", &value_tmp);
                            max.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_y", &value_tmp);
                            max.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_z", &value_tmp);
                            max.z = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Max_value_w", &value_tmp);
                            max.w = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_x", &value_tmp);
                            min.x = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_y", &value_tmp);
                            min.y = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_z", &value_tmp);
                            min.z = (uint8_t) value_tmp;
                            pElem->QueryIntAttribute("Min_value_w", &value_tmp);
                            min.w = (uint8_t) value_tmp;
                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::FLOAT_VEC2_PROP : {
                        try {
                            AGFloatVec2Widget* ivpl1 = dynamic_cast<AGFloatVec2Widget*>(propsPlugins_tmp.at(j)->widget);
                            tgt::vec2 vector;
                            tgt::vec2 min;
                            tgt::vec2 max;
                            pElem->QueryFloatAttribute("Vector_x", &(vector.x));
                            pElem->QueryFloatAttribute("Vector_y", &(vector.y));
                            pElem->QueryFloatAttribute("Max_value_x", &(max.x));
                            pElem->QueryFloatAttribute("Max_value_y", &(max.y));
                            pElem->QueryFloatAttribute("Min_value_x", &(min.x));
                            pElem->QueryFloatAttribute("Min_value_y", &(min.y));

                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::FLOAT_VEC3_PROP : {
                        try {
                            AGFloatVec3Widget* ivpl1 = dynamic_cast<AGFloatVec3Widget*>(propsPlugins_tmp.at(j)->widget);
                            tgt::vec3 vector;
                            tgt::vec3 min;
                            tgt::vec3 max;
                            pElem->QueryFloatAttribute("Vector_x", &(vector.x));
                            pElem->QueryFloatAttribute("Vector_y", &(vector.y));
                            pElem->QueryFloatAttribute("Vector_z", &(vector.z));
                            pElem->QueryFloatAttribute("Max_value_x", &(max.x));
                            pElem->QueryFloatAttribute("Max_value_y", &(max.y));
                            pElem->QueryFloatAttribute("Max_value_z", &(max.z));
                            pElem->QueryFloatAttribute("Min_value_x", &(min.x));
                            pElem->QueryFloatAttribute("Min_value_y", &(min.y));
                            pElem->QueryFloatAttribute("Min_value_z", &(min.z));

                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                    case Property::FLOAT_VEC4_PROP : {
                        try {
                            AGFloatVec4Widget* ivpl1 = dynamic_cast<AGFloatVec4Widget*>(propsPlugins_tmp.at(j)->widget);
                            tgt::vec4 vector;
                            tgt::vec4 min;
                            tgt::vec4 max;
                            pElem->QueryFloatAttribute("Vector_x", &(vector.x));
                            pElem->QueryFloatAttribute("Vector_y", &(vector.y));
                            pElem->QueryFloatAttribute("Vector_z", &(vector.z));
                            pElem->QueryFloatAttribute("Vector_w", &(vector.w));
                            pElem->QueryFloatAttribute("Max_value_x", &(max.x));
                            pElem->QueryFloatAttribute("Max_value_y", &(max.y));
                            pElem->QueryFloatAttribute("Max_value_z", &(max.z));
                            pElem->QueryFloatAttribute("Max_value_w", &(max.w));
                            pElem->QueryFloatAttribute("Min_value_x", &(min.x));
                            pElem->QueryFloatAttribute("Min_value_y", &(min.y));
                            pElem->QueryFloatAttribute("Min_value_z", &(min.z));
                            pElem->QueryFloatAttribute("Min_value_w", &(min.w));
                            
                            ivpl1->initWidgetFromLoadedSettings(min, max, vector);
                        } catch (const std::bad_cast& /*ex*/) {
                            QMessageBox::warning(this, "Error",
                                "A dynamic-cast was not successfull.");
                        }
                        break;
                    }
                }
            }
        }
    }
}

void WidgetGenerator::loadWidgetSettings() {
    //open xml file
    QFileDialog *fd = new QFileDialog(this);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setFilter(tr("Widget configuration (*.xml)"));
    fd->setViewMode(QFileDialog::List);
    fd->setWindowTitle(tr("Load configuration from file"));
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    QDir dir = fd->directory();
    fd->setDirectory(dir.absolutePath());

    QStringList fileNames;
    if (fd->exec())
        fileNames = fd->selectedFiles();
    else
        return;

    std::string filename = fileNames.value(0).toStdString();

    TiXmlDocument doc(filename);

    if (!doc.LoadFile()) return;

    TiXmlHandle hDoc(&doc);
	TiXmlElement* rElem;
	//TiXmlElement* pElem; //unreferenced ??
    TiXmlHandle hRoot(0);

    rElem=hDoc.FirstChildElement().Element();
	// get the xml root in this case "processor-settings"
	if (!rElem)
        return;

	// save this for later
	hRoot=TiXmlHandle(rElem);

    // iterate through all possible processor
    // there should be at least one processor

    // test if the current config name matches the config name you are attempted to load
    // first read config name from xml

    rElem=hRoot.Element();

    if (QString(configName_->text()).toStdString() != rElem->Attribute("Config_name")) {
        switch(QMessageBox::question(this,"Voreen",
            tr("The current configuration name does not match the configuration name \n"
            "you are attempting to load, proceed anyway?"),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No)) {

            case QMessageBox::No:
                return;
            default:
                break;
        }
    }
    configName_->setText(rElem->Attribute("Config_name"));

    std::string processor_name;
    std::string overlay_name;
    std::stringstream myStream;

    // look for settings in xml (processor-settings, overlay-settings)

    // look for processor-settings in xml
    rElem=hRoot.FirstChildElement().FirstChildElement().Element();

    for(; rElem; rElem=rElem->NextSiblingElement()) {

        // is there a property?
        if (!rElem->NoChildren()) {

            processor_name = rElem->Attribute("Processor_name");

            // look for a processor with processor_name in processorproperty_list
            for(size_t i = 0; i < processorPropertyPlugin_list_.size(); i++) {
                if( processorPropertyPlugin_list_.at(i)->processor->getName() == processor_name ) {
                    // found a processor which fits processor_name
                    // iterate through all properties of the current processor and set attributes
                    int int_tmp;
                    rElem->QueryIntAttribute("State", &int_tmp);
                    if (int_tmp == 1)
                        processorPropertyPlugin_list_.at(i)->state = true;
                    else
                        processorPropertyPlugin_list_.at(i)->state = false;

                    loadPropsFromXML(processorPropertyPlugin_list_.at(i)->propWidg_list_, rElem);

                    break;
                }
            }
        }
    }

    // look for overlay-settings in xml
    rElem = hRoot.FirstChildElement().Element()->NextSiblingElement()->FirstChildElement();

    for(; rElem; rElem=rElem->NextSiblingElement()) {

        // is there a property?
        if (!rElem->NoChildren()) {

            overlay_name = rElem->Attribute("Overlay_name");

            // look for a processor with overlay_name in processorproperty_list
            for (size_t i = 0; i < overlayPropertyPlugin_list_.size(); i++) {
                if ( overlayPropertyPlugin_list_.at(i)->overlay->getName() == overlay_name ) {
                    // found a processor which fits processor_name
                    // iterate through all properties of the current processor and set attributes
                    int int_tmp;
                    rElem->QueryIntAttribute("State", &int_tmp);
                    if (int_tmp == 1)
                        overlayPropertyPlugin_list_.at(i)->state = true;
                    else
                        overlayPropertyPlugin_list_.at(i)->state = false;

                    loadPropsFromXML(overlayPropertyPlugin_list_.at(i)->propWidg_list_, rElem);

                    break;
                }
            }
        }
    }

    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    stateChanged();
    QMessageBox::information(this,"Voreen", "Configuration has been loaded successfully!");
}

void WidgetGenerator::savePropsToXML(vector< PropsWidgetList* > propsPlugins_tmp, size_t /*j*/, TiXmlElement* xmlParent) {
    //FIXME: wrong value is saved when property is not on autochange
    //       and message send by widget is not caught in the class where property is defined
    // d_kirs04 - That's because Processors are not registered with the Message-Receiver

    std::stringstream myStream;
    std::stringstream myStream_cond;
    std::stringstream myStream_marker;
    //iterate through all properties of the processor

    for (size_t i = 0; i < propsPlugins_tmp.size(); ++i) {
        //iterate through all properties and save them to xml
        myStream.str("");
        myStream << i;
        TiXmlElement *xmlProperty = new TiXmlElement("Property" + myStream.str());
        xmlParent->LinkEndChild(xmlProperty);

        xmlProperty->SetAttribute("Property_text" , propsPlugins_tmp.at(i)->prop->getGuiText());
        xmlProperty->SetAttribute("Grouped", propsPlugins_tmp.at(i)->prop->getGroup().getName());
        xmlProperty->SetAttribute("Autochange", propsPlugins_tmp.at(i)->prop->getAutoChange());

        switch (propsPlugins_tmp.at(i)->prop->getType()) {
            case Property::FLOAT_PROP : {
                FloatProp* prop1 = dynamic_cast<FloatProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_PROP);
                xmlProperty->SetDoubleAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Max_value" ,(int)prop1->getMaxValue());
                xmlProperty->SetAttribute("Min_value" ,(int)prop1->getMinValue());
                break;
            }
            case Property::INT_PROP : {
				IntProp* prop1 = dynamic_cast<IntProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::INT_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Max_value" ,prop1->getMaxValue());
                xmlProperty->SetAttribute("Min_value" ,prop1->getMinValue());
                break;
            }
            case Property::BOOL_PROP : {
                BoolProp* prop1 = dynamic_cast<BoolProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::BOOL_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                break;
            }
            case Property::COLOR_PROP : {
				ColorProp* prop1 = dynamic_cast<ColorProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::COLOR_PROP);
                xmlProperty->SetDoubleAttribute("Color_r", prop1->get().r);
                xmlProperty->SetDoubleAttribute("Color_g", prop1->get().g);
                xmlProperty->SetDoubleAttribute("Color_b", prop1->get().b);
                xmlProperty->SetDoubleAttribute("Color_a", prop1->get().a);
                break;
            }
            case Property::ENUM_PROP : {
				EnumProp* prop1 = dynamic_cast<EnumProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::ENUM_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                break;
            }
            case Property::BUTTON_PROP : {
                xmlProperty->SetAttribute("Property_type", Property::BUTTON_PROP);
                break;
            }
            case Property::FILEDIALOG_PROP : {
				FileDialogProp* prop1 = dynamic_cast<FileDialogProp*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::FILEDIALOG_PROP);
                xmlProperty->SetAttribute("Value", prop1->get());
                xmlProperty->SetAttribute("Dialog_caption", prop1->getDialogCaption());
                xmlProperty->SetAttribute("Dialog_directory", prop1->getDirectory());
                xmlProperty->SetAttribute("Dialog_filefilter", prop1->getFileFilter());
                break;
            }
            case Property::TRANSFUNC_PROP : {
//                 TransFuncPlugin* transfer = dynamic_cast<TransFuncPlugin*>(renderPropertyPlugin_list_[j]->propWidg_list_[i]->widget);
//
//                 xmlProperty->SetAttribute("Property_type", Property::TRANSFERFUNC_PROP);
//                 TransFuncIntensityKeys* tf = transfer->getTransferFunc();
//
//                 // iterate through all markers
//                 for (size_t m = 0; m < tf->getNumKeys(); ++m) {
//                     // prepare xml
//                     myStream_marker.str("");
//                     myStream_marker << m;
//                     TiXmlElement *xmlMarker = new TiXmlElement("Marker" + myStream_marker.str());
//                     //save markers to xml
//                     xmlMarker->SetDoubleAttribute("Source_value", tf->getKey(m)->getIntensity());
//                     xmlMarker->SetDoubleAttribute("Dest_left_value", tf->getKey(m)->getAlphaL());
//                     xmlMarker->SetAttribute("isSplit", tf->getKey(m)->isSplit());
//                     xmlMarker->SetAttribute("Color_r", tf->getKey(m)->getColorL().r);
//                     xmlMarker->SetAttribute("Color_g", tf->getKey(m)->getColorL().g);
//                     xmlMarker->SetAttribute("Color_b", tf->getKey(m)->getColorL().b);
//                     xmlMarker->SetAttribute("Color_a", tf->getKey(m)->getColorL().a);
//                     if (tf->getKey(m)->isSplit()) {
//                         xmlMarker->SetAttribute("Color_alt_r", tf->getKey(m)->getColorR().r);
//                         xmlMarker->SetAttribute("Color_alt_g", tf->getKey(m)->getColorR().g);
//                         xmlMarker->SetAttribute("Color_alt_b", tf->getKey(m)->getColorR().b);
//                         xmlMarker->SetAttribute("Color_alt_a", tf->getKey(m)->getColorR().a);
//                         xmlMarker->SetDoubleAttribute("Dest_right_value", tf->getKey(m)->getAlphaR());
//                     }else{
//                         xmlMarker->SetAttribute("Color_alt_r", 0);
//                         xmlMarker->SetAttribute("Color_alt_g", 0);
//                         xmlMarker->SetAttribute("Color_alt_b", 0);
//                         xmlMarker->SetAttribute("Color_alt_a", 0);
//                         xmlMarker->SetDoubleAttribute("Dest_right_value", 0);
//                     }
//
//                     xmlProperty->LinkEndChild(xmlMarker);
//                 } // for (size_t m=0; m<transfer->getNumKeys(); ++m)
                break;
            }
            case Property::INTEGER_VEC2_PROP : {
                IntVec2Prop* prop1 = dynamic_cast<IntVec2Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC2_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                break;
            }
            case Property::INTEGER_VEC3_PROP : {
                IntVec3Prop* prop1 = dynamic_cast<IntVec3Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC3_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Max_value_z" ,prop1->getMaximum().z);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                xmlProperty->SetAttribute("Min_value_z" ,prop1->getMinimum().z);
                break;
            }
            case Property::INTEGER_VEC4_PROP : {
                IntVec4Prop* prop1 = dynamic_cast<IntVec4Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::INTEGER_VEC4_PROP);
                xmlProperty->SetAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetAttribute("Vector_w", prop1->get().w);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Max_value_z" ,prop1->getMaximum().z);
                xmlProperty->SetAttribute("Max_value_w" ,prop1->getMaximum().w);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                xmlProperty->SetAttribute("Min_value_z" ,prop1->getMinimum().z);
                xmlProperty->SetAttribute("Min_value_w" ,prop1->getMinimum().w);
                break;
            }
            case Property::FLOAT_VEC2_PROP : {
                FloatVec2Prop* prop1 = dynamic_cast<FloatVec2Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC2_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                break;
            }
            case Property::FLOAT_VEC3_PROP : {
                FloatVec3Prop* prop1 = dynamic_cast<FloatVec3Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC3_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetDoubleAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Max_value_z" ,prop1->getMaximum().z);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                xmlProperty->SetAttribute("Min_value_z" ,prop1->getMinimum().z);
                break;
            }
            case Property::FLOAT_VEC4_PROP : {
                FloatVec4Prop* prop1 = dynamic_cast<FloatVec4Prop*>(propsPlugins_tmp.at(i)->prop);
                xmlProperty->SetAttribute("Property_type", Property::FLOAT_VEC4_PROP);
                xmlProperty->SetDoubleAttribute("Vector_x", prop1->get().x);
                xmlProperty->SetDoubleAttribute("Vector_y", prop1->get().y);
                xmlProperty->SetDoubleAttribute("Vector_z", prop1->get().z);
                xmlProperty->SetDoubleAttribute("Vector_w", prop1->get().w);
                xmlProperty->SetAttribute("Max_value_x" ,prop1->getMaximum().x);
                xmlProperty->SetAttribute("Max_value_y" ,prop1->getMaximum().y);
                xmlProperty->SetAttribute("Max_value_z" ,prop1->getMaximum().z);
                xmlProperty->SetAttribute("Max_value_w" ,prop1->getMaximum().w);
                xmlProperty->SetAttribute("Min_value_x" ,prop1->getMinimum().x);
                xmlProperty->SetAttribute("Min_value_y" ,prop1->getMinimum().y);
                xmlProperty->SetAttribute("Min_value_z" ,prop1->getMinimum().z);
                xmlProperty->SetAttribute("Min_value_w" ,prop1->getMinimum().w);
                break;
            }
            default:
                break;
        }
    }
}

void WidgetGenerator::saveWidgetSettings() {
    // first check if there's a name for the current config
    if (configName_->text() == "") {
        QMessageBox::information(this, "Voreen", tr("Please enter a name for the current configuration"));
        return;
    }

    QFileDialog *fd = new QFileDialog(this);
    fd->setFileMode(QFileDialog::AnyFile);
    fd->setFilter(tr("Widget configuration (*.xml)"));
    fd->setViewMode(QFileDialog::List);
    fd->selectFile(configName_->text());
    fd->setWindowTitle(tr("Save configuration to file"));
    fd->setAcceptMode(QFileDialog::AcceptSave);
    fd->setDefaultSuffix(tr("xml"));
    QDir dir = fd->directory();
    fd->setDirectory(dir.absolutePath());

    QStringList fileNames;
    if (fd->exec())
        fileNames = fd->selectedFiles();
    else
        return;

    std::string filename = fileNames.value(0).toStdString();
    std::stringstream myStream;
    vector< PropsWidgetList* > propsPlugins_tmp;

    //init xml file
    TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild(decl);

    // save configuration name
    TiXmlElement *root = new TiXmlElement("Voreen-Config");
    // versioning to ensure downardly compatibility
    //HISTORY:
    // 1.0.0 - Basic functionality
    // 1.0.1 - added attribute "Autochange" and attribute "Grouped"
    // 1.0.2 - complete redesign of xml-root structure
    root->SetAttribute("Config_version", "1.0.2");
    root->SetAttribute("Config_name", QString(configName_->text()).toStdString());
    doc.LinkEndChild(root);

    TiXmlElement *rootProcessor = new TiXmlElement("Processor-settings");
    root->LinkEndChild(rootProcessor);

    //iterate through all processors from processorPropertyPlugin_list_
    for(size_t j = 0; j < processorPropertyPlugin_list_.size() ; ++j){
        //get vector of properties and plugins from actual processor
        propsPlugins_tmp = processorPropertyPlugin_list_[j]->propWidg_list_;

        myStream.str("");
        myStream << j;
        TiXmlElement *xmlProcessor = new TiXmlElement("Processor" + myStream.str());
        rootProcessor->LinkEndChild(xmlProcessor);
        xmlProcessor->SetAttribute("Processor_name", processorPropertyPlugin_list_[j]->processor->getName());
        xmlProcessor->SetAttribute("State", processorPropertyPlugin_list_[j]->state);

        if (propsPlugins_tmp.size() > 0) {
            savePropsToXML(propsPlugins_tmp, j, xmlProcessor);
        }

    }

    TiXmlElement *rootOverlay = new TiXmlElement("Overlay-settings");
    root->LinkEndChild(rootOverlay);

   //iterate through all overlays from overlayPropertyPlugin_list_
    for(size_t j = 0; j < overlayPropertyPlugin_list_.size() ; ++j){
        //get vector of properties and plugins from actual overlay
        propsPlugins_tmp = overlayPropertyPlugin_list_[j]->propWidg_list_;

        myStream.str("");
        myStream << j;
        TiXmlElement *xmlOverlay = new TiXmlElement("Overlay" + myStream.str());
        rootOverlay->LinkEndChild(xmlOverlay);
        xmlOverlay->SetAttribute("Overlay_name", overlayPropertyPlugin_list_[j]->overlay->getName());
        xmlOverlay->SetAttribute("State", overlayPropertyPlugin_list_[j]->state);

        if (propsPlugins_tmp.size() > 0) {
            savePropsToXML(propsPlugins_tmp, j, xmlOverlay);
        }

    }

    //save xml file
    doc.SaveFile(filename);

    QMessageBox::information(this,"Voreen",
            tr("Configuration saved successfully!"));
}

void WidgetGenerator::createAllWidgets(std::vector<Processor*> processors, OverlayManager* mgr){
	//connect all grouped and conditioned Props to their owners
	overlays_ = mgr->getOverlays();
	NetworkAnalyzer::connectOverlayProps(overlays_);

    processor_list = processors;
}

void WidgetGenerator::stateChanged(){
    //set the visible-state of all properties in the frames and the overlays
    //depending on the visibility of the widget and the property
    for (unsigned int i=0; i < frames_.size(); ++i){
        if (frames_.at(i)->list_)
            frames_.at(i)->setVisible(frames_.at(i)->list_->state && frames_.at(i)->prop_->isVisible());
        if (frames_.at(i)->listOverlay_)
            frames_.at(i)->setVisible(frames_.at(i)->listOverlay_->state && frames_.at(i)->prop_->isVisible());
    }
    //set visible-state of all other properties
    for (unsigned int i=0; i < processorPropertyPlugin_list_.size(); ++i){
        std::vector<PropsWidgetList*> vec_tmp = processorPropertyPlugin_list_.at(i)->propWidg_list_;
        for (unsigned int j=0; j < vec_tmp.size(); ++j) {
            if (vec_tmp.at(j)) {
                if (vec_tmp.at(j)->widget)
                    vec_tmp.at(j)->widget->setVisible(processorPropertyPlugin_list_.at(i)->state && vec_tmp.at(j)->prop->isVisible());
            }
        }
    }
    for (unsigned int i=0; i < overlayPropertyPlugin_list_.size(); ++i) {
        std::vector<PropsWidgetList*> vec_tmp = overlayPropertyPlugin_list_.at(i)->propWidg_list_;
        for (unsigned int j=0; j < vec_tmp.size(); ++j){
            vec_tmp.at(j)->widget->setVisible(overlayPropertyPlugin_list_.at(i)->state && vec_tmp.at(j)->prop->isVisible());
        }
    }
    for (size_t i = 0; i < ProcessorHeaderWidgets_.size(); ++i) {
        ProcessorHeaderWidgets_[i]->updateState();
    }
}

std::vector<TransFuncPlugin*> WidgetGenerator::getTransFuncPlugins() {
    std::vector<TransFuncPlugin*> back;

    // iterate through all props and return vector of transfer-funcs
    // first through processors
    for (size_t i=0; i < processorPropertyPlugin_list_.size(); ++i) {
        // iterate through all properties of processors
        std::vector<PropsWidgetList*> vec_tmp = processorPropertyPlugin_list_.at(i)->propWidg_list_;
        for (size_t j=0; j < vec_tmp.size(); ++j) {
            if (vec_tmp.at(j)->prop->getType()==Property::TRANSFUNC_PROP) {
                TransFuncPlugin* tfpl_tmp = dynamic_cast<TransFuncPlugin*>(vec_tmp.at(j)->widget);
                back.push_back(tfpl_tmp);
            }
        }
    }
    return back;
}

std::vector<TransFuncAlphaPlugin*> WidgetGenerator::getTransFuncAlphaPlugins() {
    std::vector<TransFuncAlphaPlugin*> back;

    // iterate through all props and return vector of transfer-funcs
    // first through processor
    for (size_t i=0; i < processorPropertyPlugin_list_.size(); ++i) {
        // iterate through all properties of processor
        std::vector<PropsWidgetList*> vec_tmp = processorPropertyPlugin_list_.at(i)->propWidg_list_;
        for (size_t j=0; j < vec_tmp.size(); ++j) {
            if (vec_tmp.at(j)->prop && vec_tmp.at(j)->prop->getType()==Property::TRANSFUNC_ALPHA_PROP) {
                TransFuncAlphaPlugin* tfpl_tmp = dynamic_cast<TransFuncAlphaPlugin*>(vec_tmp.at(j)->widget);
                back.push_back(tfpl_tmp);
            }
        }
    }
    return back;
}

//---------------------------------------------------------------------

GroupFrame::GroupFrame(Property* prop, WidgetGenerator::ProcessorPropsList* list, MessageReceiver* msgReceiver)
    : QGroupBox()
{
    prop_ = prop;
    list_ = list;
    listOverlay_ = 0;
    msgIdent_ = prop->getIdent();
    msgReceiver_ = msgReceiver;
    setObjectName(QString::fromUtf8("Frame-Plugin"));
    connect( this , SIGNAL( toggled(bool) ) , this , SLOT( changeChecked(bool) ));
}

GroupFrame::GroupFrame(Property* prop, WidgetGenerator::OverlayPropsList* list, MessageReceiver* msgReceiver) 
    : QGroupBox()
{
    prop_ = prop;
    listOverlay_ = list;
    list_ = 0;
    msgIdent_ = prop->getIdent();
    msgReceiver_ = msgReceiver;
    setObjectName(QString::fromUtf8("Frame-Plugin"));
    connect( this , SIGNAL( toggled(bool) ) , this , SLOT( changeChecked(bool) ));
}

void GroupFrame::changeChecked(bool val) {
    BoolProp* prop_tmp = dynamic_cast<BoolProp*>(prop_);
    if ((prop_tmp) && (isVisible())){
        if (prop_tmp->getAutoChange())
            prop_tmp->set(val);
        else
            msgReceiver_->postMessage(new BoolMsg(msgIdent_, val), prop_->getMsgDestination());
        
        MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    }
}

void GroupFrame::initWidgetFromLoadedSettings(bool value) {
    blockSignals(true);
    setChecked(value);
    blockSignals(false);
    BoolProp* prop_tmp = dynamic_cast<BoolProp*>(prop_);
    if ((prop_tmp) && (isVisible())) {
        if (prop_tmp->getAutoChange())
            prop_tmp->set(value);
        else
            msgReceiver_->postMessage(new BoolMsg(msgIdent_, value), prop_->getMsgDestination());
    }
}

void GroupFrame::setVisibleState(bool vis) {
    setVisible(vis);
}

void GroupFrame::changeValue(bool val) {
    if (!isVisible()) {
        disconnect( this , SIGNAL( toggled(bool) ) , this , SLOT( changeChecked(bool) ));
        setChecked(val);
        connect( this , SIGNAL( toggled(bool) ) , this , SLOT( changeChecked(bool) ));
    }
    else
        setChecked(val);

}

} // namespace voreen

