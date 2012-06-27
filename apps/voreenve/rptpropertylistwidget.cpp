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

#include "rptpropertylistwidget.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

RptPropertyListWidget::RptPropertyListWidget(QWidget* parent)
    : QTableWidget(parent)
    , painter_(0)
    , epsilon_(0.001f)
{
    setMinimumSize(200, 200);
    horizontalHeader()->hide();
    horizontalScrollBar()->hide();
    verticalHeader()->hide();

    MsgDistr.insert(this);
}

RptPropertyListWidget::~RptPropertyListWidget() {
    MsgDistr.remove(this);
}


void RptPropertyListWidget::resizeEvent(QResizeEvent* /*event*/) {
    setColumnWidth(0,width()/2);
    setColumnWidth(1,width()/2);
    setColumnWidth(0,width() / 2 - verticalScrollBar()->width() / 2);
    setColumnWidth(1,width() / 2 - verticalScrollBar()->width() / 2);
}

void RptPropertyListWidget::setProcessor(Processor* processor) {
    propertyList_.clear();
    clear();
    vectorWidgetList_.clear();
    setRowCount(0);

	if (processor==0)
        return;

    processor_ = processor;

    insertProperties(processor_->getProperties());
}

void RptPropertyListWidget::deselectProcessor(Processor* processor) {
    if (processor == processor_){
        clear();
        reset();
        propertyList_.clear();
        vectorWidgetList_.clear();
        setRowCount(0);
    }
}


void RptPropertyListWidget::insertProperties(std::vector<Property*> propertyList) {
    QStringList vHeader;
    int index = 0;
    propertyList_ = propertyList;
  
    setRowCount(100);
    setColumnCount(2);
	QString bold_s("<b>");
    QString bold_e("</b>");
	QString color_s("<font color=black>");
    QString color_e("</font>");

    
    for (size_t i=0; i< propertyList.size();i++){  
        color_s = QString("<font color=black>");
		switch (propertyList.at(i)->getType()){
            case Property::INT_PROP:
                {
                IntProp* prop = static_cast<IntProp*>(propertyList.at(i));
                QSpinBox* spinBox = getIntPropertyWidget(prop->getMinValue(), prop->getMaxValue(),prop->get());
                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e)); 
                // needed for Qt 4.4.0
                spinBox->blockSignals(true);
                // workaround: Qt resets the spinbox value during insertation
                int value = spinBox->value();
                setCellWidget(index, 1, spinBox);
                spinBox->setValue(value);
                spinBox->blockSignals(false);

                index++;
             break;
                }
            case Property::FLOAT_PROP:
                {
                FloatProp* prop = static_cast<FloatProp*>(propertyList.at(i));
                QDoubleSpinBox* spinBox = getFloatPropertyWidget(prop->getMinValue(), prop->getMaxValue(),prop->get());
                setCellWidget(index, 0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e));
                // needed for Qt 4.4.0
                spinBox->blockSignals(true);
                // workaround: Qt resets the spinbox value during insertation
                double value = spinBox->value();
                setCellWidget(index, 1, spinBox);
                spinBox->setValue(value);
                spinBox->blockSignals(false);
                index++;
            break;
                }
			/*case Property::TRANSFERFUNC_PROP: 
				{
					TransFuncProp* prop = static_cast<TransFuncProp*>(propertyList.at(i));
					setCellWidget(index,1,new QLabel(prop->getGuiText().c_str()));
	               
					if (unequalEntries_.contains(i)) color_s = QString("<font color=red>");
					setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e));  
				
					index++;
            break;
				}*/
            case Property::ENUM_PROP:
                {
                EnumProp* prop = static_cast<EnumProp*>(propertyList.at(i));
                setCellWidget(index,1,getEnumPropertyWidget(prop->getStrings(),prop->get()));
                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e));

                index++;
             break;
                }
                           
            case Property::BOOL_PROP:
                {
                BoolProp* prop = static_cast<BoolProp*>(propertyList.at(i));
                setCellWidget(index, 1, getBoolPropertyWidget(prop->get()));
                setCellWidget(index, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e));  

                index++;
             break;
                }
			case Property::STRING_PROP:
                {
					
                StringProp* prop = static_cast<StringProp*>(propertyList.at(i));
				setCellWidget(index,1,getStringPropertyWidget(prop) );
				setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e)); 

				index++;
             break;
                }
			case Property::STRINGVECTOR_PROP:
                {
					
                StringVectorProp* prop = static_cast<StringVectorProp*>(propertyList.at(i));
				setCellWidget(index,1,getStringVectorPropertyWidget(prop) );
				setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_e)); 

				index++;
             break;
                }
            case Property::INTEGER_VEC2_PROP:
                {
                IntVec2Prop* prop = static_cast<IntVec2Prop*>(propertyList.at(i));
                std::vector<QWidget*> vecWid = getIVec2PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   
                vecWid.at(0)->blockSignals(true);
                setCellWidget(index,1,vecWid.at(0));
                vecWid.at(0)->blockSignals(false);

                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s));  

                vecWid.at(1)->blockSignals(true);
                setCellWidget(index+1, 1, vecWid.at(1));
                vecWid.at(1)->blockSignals(false);
                setCellWidget(index+1,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s));  

                index+=2;
            break;
                }
            case Property::FILEDIALOG_PROP:
                 {
                 FileDialogProp* prop = static_cast<FileDialogProp*>(propertyList.at(i));
                 setCellWidget(index, 0, new QLabel(bold_s + prop->getGuiText().c_str() + bold_e));
                 FileDialogPropertyWidget* fileWidget = new FileDialogPropertyWidget(this, prop);
                 setCellWidget(index, 1, fileWidget);

                 std::vector<QWidget*> vec;
                 vec.push_back(fileWidget);
                 vectorWidgetList_.push_back(vec);

                index++;
            break;
                 }
            case Property::INTEGER_VEC3_PROP:
                {
                IntVec3Prop* prop = static_cast<IntVec3Prop*>(propertyList.at(i));
                std::vector<QWidget*> vecWid = getIVec3PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   
                vecWid.at(0)->blockSignals(true);
                setCellWidget(index, 1, vecWid.at(0));
                vecWid.at(0)->blockSignals(false);

                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s ));  
                vecWid.at(1)->blockSignals(true);
                setCellWidget(index+1, 1, vecWid.at(1));
                vecWid.at(1)->blockSignals(false);

                setCellWidget(index+1, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s ));  
                vecWid.at(2)->blockSignals(true);
                setCellWidget(index+2, 1, vecWid.at(2));
                vecWid.at(2)->blockSignals(false);
                setCellWidget(index+2, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - z")+ bold_e + color_s ));  

                index+=3;
             break;
                }
            case Property::INTEGER_VEC4_PROP:
                {
                IntVec4Prop* prop = static_cast<IntVec4Prop*>(propertyList.at(i));
                std::vector<QWidget*> vecWid = getIVec4PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   

                vecWid.at(0)->blockSignals(true);
                setCellWidget(index, 1, vecWid.at(0));
                vecWid.at(0)->blockSignals(false);

                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s ));  
                vecWid.at(1)->blockSignals(true);
                setCellWidget(index+1,1,vecWid.at(1));
                setCellWidget(index+1,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s ));  
                vecWid.at(1)->blockSignals(false);

                vecWid.at(2)->blockSignals(true);
                setCellWidget(index+2,0,vecWid.at(2));
                setCellWidget(index+2,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - z")+ bold_e + color_s));  
                vecWid.at(2)->blockSignals(false);

                vecWid.at(3)->blockSignals(true);
                setCellWidget(index+3,0, vecWid.at(3));
                setCellWidget(index+3, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - w")+ bold_e + color_s ));  
                vecWid.at(3)->blockSignals(false);

                index+=4;
             break;
                }
            case Property::FLOAT_VEC2_PROP:
                {
                FloatVec2Prop* prop = static_cast<FloatVec2Prop*>(propertyList.at(i));
                std::vector<QWidget*> vecWid = getFVec2PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   

                vecWid.at(0)->blockSignals(true);
                setCellWidget(index, 1, vecWid.at(0));
                vecWid.at(0)->blockSignals(false);

                setCellWidget(index, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s ));  

                vecWid.at(1)->blockSignals(true);
                setCellWidget(index+1, 1, vecWid.at(1));
                setCellWidget(index+1, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s ));  
                vecWid.at(1)->blockSignals(false);

                index+=2;
             break;
                }
            case Property::FLOAT_VEC3_PROP:
                {
                FloatVec3Prop* prop = static_cast<FloatVec3Prop*>(propertyList.at(i));
                std::vector<QWidget*> vecWid = getFVec3PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   

                vecWid.at(0)->blockSignals(true);
                setCellWidget(index, 1, vecWid.at(0));
                vecWid.at(0)->blockSignals(false);

                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s ));  

                vecWid.at(1)->blockSignals(true);
                setCellWidget(index+1, 1, vecWid.at(1));
                vecWid.at(1)->blockSignals(false);
                setCellWidget(index+1,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s ));  

                vecWid.at(2)->blockSignals(true);
                setCellWidget(index+2, 1, vecWid.at(2));
                vecWid.at(2)->blockSignals(false);
                setCellWidget(index+2, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - z")+ bold_e + color_s ));  
                index+=3;
             break;
                }
            case Property::FLOAT_VEC4_PROP:
                {
               FloatVec4Prop* prop = static_cast<FloatVec4Prop*>(propertyList.at(i));
               std::vector<QWidget*> vecWid = getFVec4PropertyWidget(prop->get(),prop->getMinimum(),prop->getMaximum());   

               vecWid.at(0)->blockSignals(true);
               setCellWidget(index, 1, vecWid.at(0));
               vecWid.at(0)->blockSignals(false);

               setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - x")+ bold_e + color_s ));  

               vecWid.at(1)->blockSignals(true);
               setCellWidget(index+1,1,vecWid.at(1));
               vecWid.at(1)->blockSignals(false);
               setCellWidget(index+1,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - y")+ bold_e + color_s ));  

               vecWid.at(2)->blockSignals(true);
               setCellWidget(index+2,1,vecWid.at(2));
               vecWid.at(2)->blockSignals(false);
               setCellWidget(index+2,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - z")+ bold_e + color_s ));  

               vecWid.at(3)->blockSignals(true);
               setCellWidget(index+3, 1, vecWid.at(3));
               vecWid.at(3)->blockSignals(false);
               setCellWidget(index+3, 0, new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + QString(" - w") + bold_e + color_s ));  

               index+=4;
            break;
                }
            case Property::COLOR_PROP:
                {
                ColorProp* prop = static_cast<ColorProp*>(propertyList.at(i));
              
                ColorPropertyWidget* colorProp = new ColorPropertyWidget(0,prop->get());
                std::vector<QWidget*> vec;
                vec.push_back(colorProp);
                vectorWidgetList_.push_back(vec);
                setCellWidget(index,1,colorProp);
                setCellWidget(index,0,new QLabel(color_s + bold_s + QString(prop->getGuiText().c_str()) + bold_e + color_s ));  
		    
                index++; 
            break;
                }
            default:
                {}
        }
   }
   setRowCount(index);

}

void RptPropertyListWidget::widgetChanged(int) {
    exportCellValues();
}

void RptPropertyListWidget::widgetChanged(double) {
    exportCellValues();
}

void RptPropertyListWidget::stringPropButtonPushed(StringProp* prop) {
	QFileDialog fileDialog(this,
                        tr("Dataset filename"),
						QDir::currentPath(),
                        "Raw file (*.raw)");
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    if (fileDialog.exec()) {
        prop->set(fileDialog.selectedFiles().at(0).toStdString());
    }
	widgetChanged(0);
}

void RptPropertyListWidget::propertyButtonClicked(Property* prop) {
	if (prop->getType() == Property::STRINGVECTOR_PROP) {
		StringVectorProp* stringVectorProp = dynamic_cast<StringVectorProp*>(prop);
		if (stringVectorProp) {
			stringVectorPropButtonPushed(stringVectorProp);
		}
	}
    else if (prop->getType() == Property::STRING_PROP) {
		StringProp* stringProp = dynamic_cast<StringProp*>(prop);
		if (stringProp) {
			stringPropButtonPushed(stringProp);
		}
	}
}

void RptPropertyListWidget::stringVectorPropButtonPushed(StringVectorProp* prop) {
	QFileDialog fileDialog(this,
                        tr("Dataset filename"),
						QDir::currentPath(),
                        "Any file (*.*)");
	QStringList filters;
	filters << "Raw file (*.raw) "<< "Frozen CT (*.fro)" << "CT (*.fre)" << "MRI pd (*.pd)" << "MRI t1 (*.t1)"<<"MRI t2 (*.t2)" << "Any file (*.*)";
	fileDialog.setFilters(filters);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (fileDialog.exec()) {
		std::vector<std::string> files;
		for (int i=0; i<fileDialog.selectedFiles().size(); i++) {
			files.push_back(fileDialog.selectedFiles().at(i).toStdString());
		}
		prop->set(files);
    }
	widgetChanged(0);
}


QComboBox* RptPropertyListWidget::getBoolPropertyWidget(bool startValue) {
    QComboBox* boolProp = new QComboBox();

    boolProp->insertItem(1,"true");
    boolProp->insertItem(0,"false");
    boolProp->setCurrentIndex(startValue); 

    std::vector<QWidget*> vec;
    vec.push_back(boolProp);
    vectorWidgetList_.push_back(vec);

    connect(boolProp, SIGNAL(currentIndexChanged(int)),this, SLOT(widgetChanged(int)));

    return boolProp;
}

QDialogButtonBox* RptPropertyListWidget::getStringPropertyWidget(StringProp* prop) {
	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal,0);
	QGridLayout* grid = new QGridLayout();
	buttonBox->setLayout(grid);

	RptPropertyButton* datasetButton = new RptPropertyButton("dataset");
	datasetButton->setProperty(prop);
	buttonBox->addButton(datasetButton,QDialogButtonBox::ActionRole);
	
	std::vector<QWidget*> vec;
    vec.push_back(buttonBox);
    vectorWidgetList_.push_back(vec);

	connect(datasetButton, SIGNAL(clicked()) ,datasetButton, SLOT(buttonClicked() ) );
	connect(datasetButton, SIGNAL(propertyButtonPushed(Property*)) ,this, SLOT(propertyButtonClicked(Property*) ) );

	return buttonBox;
}

QDialogButtonBox* RptPropertyListWidget::getStringVectorPropertyWidget(StringVectorProp* prop) {
	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal,0);
	QGridLayout* grid = new QGridLayout();
	buttonBox->setLayout(grid);

	RptPropertyButton* sliceButton = new RptPropertyButton("Select");
	sliceButton->setProperty(prop);
	buttonBox->addButton(sliceButton,QDialogButtonBox::ActionRole);

	std::vector<QWidget*> vec;
    vec.push_back(buttonBox);
    vectorWidgetList_.push_back(vec);

	connect(sliceButton, SIGNAL(clicked()) ,sliceButton, SLOT(buttonClicked() ) );
	connect(sliceButton, SIGNAL(propertyButtonPushed(Property*)) ,this, SLOT(propertyButtonClicked(Property*) ) );

	return buttonBox;
}

QComboBox* RptPropertyListWidget::getEnumPropertyWidget(std::vector<std::string> descr, int startValue) {
    QComboBox* enumProp = new QComboBox();

    for (size_t i=0; i< descr.size(); i++)
        enumProp->insertItem(i,descr.at(i).c_str());  

    enumProp->setCurrentIndex(startValue); 
    std::vector<QWidget*> vec;
    vec.push_back(enumProp);
    vectorWidgetList_.push_back(vec);

    connect(enumProp, SIGNAL(currentIndexChanged(int)),this, SLOT(widgetChanged(int)));

    return enumProp;
}

QSpinBox* RptPropertyListWidget::getIntPropertyWidget(int min, int max, int startValue) {
    QSpinBox* spinProp = new QSpinBox();

    spinProp->setMinimum(min);
    spinProp->setMaximum(max);
    spinProp->setSingleStep(1); 
    spinProp->setValue(startValue);

    int i = spinProp->value();

    std::vector<QWidget*> vec;
    vec.push_back(spinProp);
    vectorWidgetList_.push_back(vec);

    connect(spinProp, SIGNAL(valueChanged(int)), this, SLOT(widgetChanged(int)));

    i = spinProp->value();

    return spinProp;
}


QDoubleSpinBox* RptPropertyListWidget::getFloatPropertyWidget(float min, float max, float startValue) {
    QDoubleSpinBox* spinProp = new QDoubleSpinBox();

    spinProp->setMinimum(min);
    spinProp->setMaximum(max);

    spinProp->setSingleStep((max-min)/100.0f);
    spinProp->setValue((double)startValue);
    spinProp->setDecimals(7);

    std::vector<QWidget*> vec;
    vec.push_back(spinProp);
    vectorWidgetList_.push_back(vec);

    connect(spinProp, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));

    return spinProp;
}

std::vector<QWidget*> RptPropertyListWidget::getIVec2PropertyWidget(tgt::ivec2 val,tgt::ivec2 min,tgt::ivec2 max) {

    QSpinBox* value1 = new QSpinBox();
    QSpinBox* value2 = new QSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(1); 
    value2->setValue(val.y);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);

    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value2, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));

    return vec;
}

std::vector<QWidget*> RptPropertyListWidget::getIVec3PropertyWidget(tgt::ivec3 val,tgt::ivec3 min,tgt::ivec3 max) {

    QSpinBox* value1 = new QSpinBox();
    QSpinBox* value2 = new QSpinBox();
    QSpinBox* value3 = new QSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(1); 
    value2->setValue(val.y);

    value3->setMinimum(min.z);
    value3->setMaximum(max.z);
    value3->setSingleStep(1); 
    value3->setValue(val.z);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);
    vec.push_back(value3);
    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value2, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value3, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));

    return vec;
}

std::vector<QWidget*> RptPropertyListWidget::getIVec4PropertyWidget(tgt::ivec4 val,tgt::ivec4 min,tgt::ivec4 max) {

    QSpinBox* value1 = new QSpinBox();
    QSpinBox* value2 = new QSpinBox();
    QSpinBox* value3 = new QSpinBox();
    QSpinBox* value4 = new QSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(1); 
    value2->setValue(val.y);

    value3->setMinimum(min.z);
    value3->setMaximum(max.z);
    value3->setSingleStep(1); 
    value3->setValue(val.z);

    value4->setMinimum(min.w);
    value4->setMaximum(max.w);
    value4->setSingleStep(1); 
    value4->setValue(val.w);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);
    vec.push_back(value3);
    vec.push_back(value4);
    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value2, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value3, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));
    connect(value4, SIGNAL(valueChanged(int)),this, SLOT(widgetChanged(int)));

    return vec;
}


std::vector<QWidget*> RptPropertyListWidget::getFVec2PropertyWidget(tgt::vec2 val,tgt::vec2 min,tgt::vec2 max) {
   
    QDoubleSpinBox* value1 = new QDoubleSpinBox();
    QDoubleSpinBox* value2 = new QDoubleSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(0.1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(0.1); 
    value2->setValue(val.y);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);
    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value2, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));

    return vec;
}

std::vector<QWidget*> RptPropertyListWidget::getFVec3PropertyWidget(tgt::vec3 val,tgt::vec3 min,tgt::vec3 max) {
   
    QDoubleSpinBox* value1 = new QDoubleSpinBox();
    QDoubleSpinBox* value2 = new QDoubleSpinBox();
    QDoubleSpinBox* value3 = new QDoubleSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(0.1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(0.1); 
    value2->setValue(val.y);

    value3->setMinimum(min.z);
    value3->setMaximum(max.z);
    value3->setSingleStep(0.1); 
    value3->setValue(val.z);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);
    vec.push_back(value3);
    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value2, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value3, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));

    return vec;
}

std::vector<QWidget*> RptPropertyListWidget::getFVec4PropertyWidget(tgt::vec4 val,tgt::vec4 min,tgt::vec4 max) {
   
    QDoubleSpinBox* value1 = new QDoubleSpinBox();
    QDoubleSpinBox* value2 = new QDoubleSpinBox();
    QDoubleSpinBox* value3 = new QDoubleSpinBox();
    QDoubleSpinBox* value4 = new QDoubleSpinBox();

    value1->setMinimum(min.x);
    value1->setMaximum(max.x);
    value1->setSingleStep(0.1); 
    value1->setValue(val.x);

    value2->setMinimum(min.y);
    value2->setMaximum(max.y);
    value2->setSingleStep(0.1); 
    value2->setValue(val.y);

    value3->setMinimum(min.z);
    value3->setMaximum(max.z);
    value3->setSingleStep(0.1); 
    value3->setValue(val.z);

    value4->setMinimum(min.w);
    value4->setMaximum(max.w);
    value4->setSingleStep(0.1); 
    value4->setValue(val.w);

    std::vector<QWidget*> vec;
    vec.push_back(value1);
    vec.push_back(value2);
    vec.push_back(value3);
    vec.push_back(value4);
    vectorWidgetList_.push_back(vec);

    connect(value1, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value2, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value3, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));
    connect(value4, SIGNAL(valueChanged(double)),this, SLOT(widgetChanged(double)));

    return vec;
}

void RptPropertyListWidget::exportCellValues() {
 
 int cellNr = -1;
   
    for (size_t i = 0; i < propertyList_.size(); i++){
        switch (propertyList_.at(i)->getType()){
            case Property::INT_PROP:
                {
                cellNr++;
                IntProp* prop = dynamic_cast<IntProp*>(propertyList_.at(i));
                QSpinBox* spin = dynamic_cast<QSpinBox*>(vectorWidgetList_.at(cellNr).at(0));

                if (prop->get() != spin->value()){
                    prop->set(spin->value());
                    processor_->postMessage(new IntMsg(prop->getIdent(), spin->value())); 
                }
                break;
                }
            case Property::FLOAT_PROP:
                {
                cellNr++;
                FloatProp* prop = dynamic_cast<FloatProp*>(propertyList_.at(i));
                QDoubleSpinBox* spin = dynamic_cast<QDoubleSpinBox*>(vectorWidgetList_.at(cellNr).at(0));
                
                if ( fabs(prop->get() - spin->value() ) > epsilon_){
                    prop->set(spin->value());
                    processor_->postMessage(new FloatMsg(prop->getIdent(), spin->value()));  
                }
                break;
                }
            case Property::ENUM_PROP:
                {
                cellNr++;
                EnumProp* prop = dynamic_cast<EnumProp*>(propertyList_.at(i));
                QComboBox* comb = dynamic_cast<QComboBox*>(vectorWidgetList_.at(cellNr).at(0));   
            	
                if (prop->get() != comb->currentIndex()){
                
                    prop->set(comb->currentIndex());  
    		        if (prop->getSendStringMsg() ) {
					    processor_->postMessage(new StringMsg(prop->getIdent(),prop->getStrings().at(prop->get()))); 
				    }
				    else {
					    processor_->postMessage(new IntMsg(prop->getIdent(),comb->currentIndex()));
				    }
                }		
                break;
                }
            case Property::BOOL_PROP:
                {
                cellNr++;
                BoolProp* prop = dynamic_cast<BoolProp*>(propertyList_.at(i));
                QComboBox* comb = dynamic_cast<QComboBox*>(vectorWidgetList_.at(cellNr).at(0));  
                if (prop->get() != (comb->currentIndex()==1))  
                {
                    prop->set(comb->currentIndex()==1);
                    processor_->postMessage(new BoolMsg(prop->getIdent(), prop->get()));  
                }
                break;
                }
			case Property::STRINGVECTOR_PROP:
				{
				cellNr++;
				StringVectorProp* prop = dynamic_cast<StringVectorProp*>(propertyList_.at(i));
				
				if (prop) {
					processor_->postMessage(new StringVectorMsg(prop->getIdent(),prop->get() ));
				}
				break;
				}
			case Property::STRING_PROP:
				{
				cellNr++;
				StringProp* prop = dynamic_cast<StringProp*>(propertyList_.at(i));
				
				if (prop) {
					processor_->postMessage(new StringMsg(prop->getIdent(),prop->get() ));
				}
				break;
				}
            case Property::FLOAT_VEC2_PROP:
                {
                cellNr++;
                FloatVec2Prop* prop = dynamic_cast<FloatVec2Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QDoubleSpinBox* sp1 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(0));
                QDoubleSpinBox* sp2 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(1));
                
                if ( ( fabs(prop->get().x - sp1->value()) > epsilon_) || ( fabs(prop->get().y - sp2->value()) > epsilon_))
                {
                    prop->set(tgt::vec2(sp1->value(),sp2->value()));  
                    processor_->postMessage(new Vec2Msg(prop->getIdent(), prop->get()));  
                }
                break;
                }
            case Property::FLOAT_VEC3_PROP:
                {
                cellNr++;
                FloatVec3Prop* prop = dynamic_cast<FloatVec3Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QDoubleSpinBox* sp1 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(0));
                QDoubleSpinBox* sp2 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(1));
                QDoubleSpinBox* sp3 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(2));
               
                if ( ( fabs(prop->get().x - sp1->value()) > epsilon_) || ( fabs(prop->get().y - sp2->value()) > epsilon_)||( fabs(prop->get().z - sp3->value()) > epsilon_))
                {
                    prop->set(tgt::vec3(sp1->value(),sp2->value(),sp3->value()));
                    processor_->postMessage(new Vec3Msg(prop->getIdent(), prop->get()));  
                }
                break;
                }
            case Property::FLOAT_VEC4_PROP:
                {
                cellNr++;
                FloatVec4Prop* prop = dynamic_cast<FloatVec4Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QDoubleSpinBox* sp1 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(0));
                QDoubleSpinBox* sp2 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(1));
                QDoubleSpinBox* sp3 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(2));
                QDoubleSpinBox* sp4 = dynamic_cast<QDoubleSpinBox*>(singleWidgetVector.at(3));

                if ( ( fabs(prop->get().x - sp1->value()) > epsilon_) || ( fabs(prop->get().y - sp2->value()) > epsilon_)||( fabs(prop->get().z - sp3->value()) > epsilon_)
                   || ( fabs(prop->get().w - sp4->value()) > epsilon_))
                {
                    prop->set(tgt::vec4(sp1->value(),sp2->value(),sp3->value(),sp4->value()));
                    processor_->postMessage(new Vec4Msg(prop->getIdent(), prop->get()));  
                }
               	break;
                }
            case Property::COLOR_PROP:
                {
                cellNr++;
                ColorProp* prop = dynamic_cast<ColorProp*>(propertyList_.at(i));
                ColorPropertyWidget* colorPropW = dynamic_cast<ColorPropertyWidget*>(vectorWidgetList_.at(cellNr).at(0));
                tgt::vec4 color = colorPropW->getSelectedColor();
               

                if ( ( fabs(prop->get().r - color.r) > epsilon_) || ( fabs(prop->get().g - color.g) > epsilon_)||( fabs(prop->get().b - color.b) > epsilon_)
                   || ( fabs(prop->get().a - color.a) > epsilon_))
                {
                      prop->set(color);
                      processor_->postMessage(new ColorMsg(prop->getIdent(), prop->get()));  
                }
                break;
                }
           case Property::INTEGER_VEC2_PROP:
                {
                cellNr++;
                IntVec2Prop* prop = dynamic_cast<IntVec2Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QSpinBox* sp1 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(0));
                QSpinBox* sp2 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(1));
              
                if ( (prop->get().x != sp1->value()) || (prop->get().y != sp2->value()) )
                {
                    prop->set(tgt::ivec2(sp1->value(),sp2->value()));  
                    processor_->postMessage(new IVec2Msg(prop->getIdent(), prop->get()));  
                }
              
                break;
                }
            case Property::INTEGER_VEC3_PROP:
                {
                cellNr++;    
                IntVec3Prop* prop = dynamic_cast<IntVec3Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QSpinBox* sp1 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(0));
                QSpinBox* sp2 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(1));
                QSpinBox* sp3 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(2));
    
                if ( (prop->get().x != sp1->value()) || (prop->get().y != sp2->value()) || (prop->get().z != sp3->value()) )
                    
                {
                    prop->set(tgt::ivec3(sp1->value(),sp2->value(),sp3->value()));
                    processor_->postMessage(new IVec3Msg(prop->getIdent(), prop->get()));  
                }
                break;
                }
            case Property::FILEDIALOG_PROP:
                {
                cellNr++;
                FileDialogProp* prop = dynamic_cast<FileDialogProp*>(propertyList_.at(i));
                    processor_->postMessage(new StringMsg(prop->getIdent(), prop->get()));
                break;
                }
            case Property::INTEGER_VEC4_PROP:
                {
                cellNr++;
                IntVec4Prop* prop = dynamic_cast<IntVec4Prop*>(propertyList_.at(i));
                std::vector<QWidget*> singleWidgetVector = vectorWidgetList_.at(cellNr);
                QSpinBox* sp1 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(0));
                QSpinBox* sp2 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(1));
                QSpinBox* sp3 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(2));
                QSpinBox* sp4 = dynamic_cast<QSpinBox*>(singleWidgetVector.at(3));
                
                if ( (prop->get().x != sp1->value()) || (prop->get().y != sp2->value()) || (prop->get().z != sp3->value()) 
                   || (prop->get().w != sp4->value()))
                {
                     prop->set(tgt::ivec4(sp1->value(),sp2->value(),sp3->value(),sp4->value()));
                     processor_->postMessage(new IVec4Msg(prop->getIdent(), prop->get()));  
                }
                break;
                }
            default:
                break;
         }   
    }
	if (painter_) {
		painter_->postMessage(new BoolMsg("msg.repaint",true));
	}
}

void RptPropertyListWidget::processMessage(Message* msg, const Identifier& /*dest*/) {
    if (msg->id_ == "set.ExportValues")
        exportCellValues();
}

//---------------------------------------------------------------------------

ColorPropertyWidget::ColorPropertyWidget(QWidget* parent, tgt::Color color)
    : QWidget(parent)
    , curColor_(color)
{

    layout_ = new QHBoxLayout();
    layout_->setMargin(0); 
    setLayout(layout_);
    colorLbl_ = new QLabel(tr(""));
    colorBt_ = new QPushButton(tr("change"));

    colorLbl_->setAutoFillBackground(true);

    QPalette newPalette = colorLbl_->palette();
    newPalette.setColor(QPalette::Window, QColor(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
                                                 static_cast<int>(color.b * 255), static_cast<int>(color.a * 255)));
    colorLbl_->setPalette(newPalette);
    colorLbl_->setBackgroundRole(newPalette.Window);

    colorBt_->setPalette(newPalette);
    colorBt_->setBackgroundRole(newPalette.Window);
    
    layout_->addWidget(colorLbl_);
    layout_->addWidget(colorBt_); 
    
    connect(colorBt_, SIGNAL(clicked(void)), this, SLOT(clickedColorBt(void)));
                 
}


void ColorPropertyWidget::clickedColorBt(){
    QColor myColor;
 
    myColor.setRgba(QColorDialog::getRgba(QColor(static_cast<int>(curColor_.r*255), static_cast<int>(curColor_.g*255), static_cast<int>(curColor_.b*255),
                                                 static_cast<int>(curColor_.a*255)).rgba()));
    if (myColor != QColor(static_cast<int>(curColor_.r*255),static_cast<int>(curColor_.g*255), static_cast<int>(curColor_.b*255), static_cast<int>(curColor_.a*255)).rgba()) {
        QPalette newPalette = colorLbl_->palette();
        newPalette.setColor(QPalette::Window, myColor );
        colorLbl_->setPalette(newPalette);
        colorLbl_->setBackgroundRole(newPalette.Window);
        curColor_ = tgt::Color(myColor.redF(),myColor.greenF(),myColor.blueF(), myColor.alphaF());
        MsgDistr.postMessage(new BoolMsg("set.ExportValues",true)); 
    }
}

tgt::vec4 ColorPropertyWidget::getSelectedColor(){
    return tgt::vec4(curColor_.r,curColor_.g,curColor_.b,curColor_.a); 
}


//---------------------------------------------------------------------

FileDialogPropertyWidget::FileDialogPropertyWidget(QWidget* parent, FileDialogProp* prop)
    : QWidget(parent)
{

    openFileDialogBtn_ = new QPushButton(this);
    std::string filename(prop->get());
    if (filename != "") {
        size_t index = filename.find_last_of('/');
        std::string endFilename = filename.substr(index + 1, filename.length());
        openFileDialogBtn_->setText(endFilename.c_str());
    } 
    else
        openFileDialogBtn_->setText(tr("open file"));

    connect(openFileDialogBtn_, SIGNAL(clicked(void)), this, SLOT(clicked(void)));

    dialogCaption_ = prop->getDialogCaption();
    directory_ = prop->getDirectory();
    fileFilter_ = prop->getFileFilter();
    myProp_ = prop;
}

void FileDialogPropertyWidget::clicked() {
    std::string filename = QString(QFileDialog::getOpenFileName(QWidget::parentWidget(), dialogCaption_.c_str(), directory_.c_str() , fileFilter_.c_str())).toStdString();
    myProp_->set(filename);
    if (filename != "") {
        size_t index = filename.find_last_of('/');
        std::string endFilename = filename.substr(index + 1, filename.length());
        openFileDialogBtn_->setText(endFilename.c_str());
        openFileDialogBtn_->hide();
        openFileDialogBtn_->show();
    }
}

} // namespace voreen
