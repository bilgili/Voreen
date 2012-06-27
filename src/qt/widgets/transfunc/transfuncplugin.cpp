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

#include "voreen/qt/widgets/transfunc/transfuncplugin.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QSplitter>
#include <QFrame>
#include <QGroupBox>
#include <QTabWidget>

#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"

#include "voreen/qt/widgets/transfunc/transfuncintensitygradientplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensityplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensitypetplugin.h"


namespace voreen {


TransFuncPlugin::TransFuncPlugin(QWidget* parent, MessageReceiver* msgReceiver, TransFuncProp* prop, Qt::Orientation widgetOrientation)
    : WidgetPlugin(parent, msgReceiver),
      prop_(prop),
      showThresholdWidget_(true),
      showRendererWidget_(true),
      showEditorTypeWidget_(true),
	  enabled_(false),
      thresholdWidget_(0),
      currentVolumeRenderer_(0),
      widgetOrientation_(widgetOrientation)
{
    setObjectName(tr("Transfer Function"));
    icon_ = QIcon(":/icons/transferfunc.png");

    if (prop_) {
        msgIdent_ = prop->getIdent();
        showThresholdWidget_ = prop_->getShowThreshold();
    } else {
        msgIdent_ = VolumeRenderer::setTransFunc_;
    }
}

TransFuncPlugin::~TransFuncPlugin() {

}

void TransFuncPlugin::setShowThresholdWidget(bool show) {
    showThresholdWidget_ = show;
}

void TransFuncPlugin::setShowEditorTypeWidget(bool show) {
    showEditorTypeWidget_ = show;
}

void TransFuncPlugin::setShowRendererWidget(bool show) {
    showRendererWidget_ = show;
}

void TransFuncPlugin::createWidgets() {
    resize(300,300);

    QBoxLayout *mainLayout = new QVBoxLayout(this);

    // Splitter between topwidget and bottomwidget
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    // Top - MappingCanvas and Gradient
    QWidget* topwidget = new QWidget(splitter);
    QVBoxLayout* gridLayout = new QVBoxLayout(topwidget);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(1);

    vrendererCombo_ = new QComboBox(0);
    if (showRendererWidget_)
        gridLayout->addWidget(vrendererCombo_);
    
    editorType_ = new QComboBox(0);
    if (showEditorTypeWidget_) 
        gridLayout->addWidget(editorType_);
    editorType_->addItem(tr("Intensity: Keys"));
    editorType_->addItem(tr("Intensity: Gradient"));
	editorType_->addItem(tr("Intensity: PET"));
    
    intensityPlugin_ = new TransFuncIntensityPlugin(0, msgReceiver_, widgetOrientation_);
    intensityPlugin_->createWidgets();
    intensityPlugin_->createConnections();
    
    intensityGradientPlugin_ = new TransFuncIntensityGradientPlugin(0, msgReceiver_, widgetOrientation_);
    intensityGradientPlugin_->createWidgets();
    intensityGradientPlugin_->createConnections();
    
	intensityPetPlugin_ = new TransFuncIntensityPetPlugin(0, msgReceiver_, widgetOrientation_);
    intensityPetPlugin_->createWidgets();
    intensityPetPlugin_->createConnections();

    
    editors_ = new QStackedWidget();
    editors_->addWidget(intensityPlugin_);
    editors_->addWidget(intensityGradientPlugin_);
	editors_->addWidget(intensityPetPlugin_);

    editors_->setContentsMargins(-8, -8, -8, -8);

    gridLayout->addWidget(editors_);
    
    gridLayout->addSpacing(10);
    
    if (showThresholdWidget_) {
        thresholdWidget_ = new ThresholdWidget(this);
        gridLayout->addWidget(thresholdWidget_);
    }

    gridLayout->addStretch();
    gridLayout->addStretch();

    setLayout(mainLayout);
}

void TransFuncPlugin::createConnections() {
    connect(editorType_, SIGNAL(activated(int)), this, SLOT(setEditor(int)));
    connect(vrendererCombo_, SIGNAL(activated(int)), this, SLOT(setProcessor(int)));

    if (showThresholdWidget_) {
        connect(intensityPlugin_, SIGNAL(transferFunctionReset()), thresholdWidget_, SLOT(resetThresholds()));
        connect(thresholdWidget_, SIGNAL(valuesChanged(int, int)), this, SLOT(setThresholds(int, int)));
        connect(thresholdWidget_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(switchInteractionMode(bool)) );
    }
}

void TransFuncPlugin::setProcessor(Processor* r) {
	processors_.clear();
    VolumeRaycaster* vrc = dynamic_cast<VolumeRaycaster*> (r);
    SliceRendererBase* srb = dynamic_cast<SliceRendererBase*> (r);
    if ( (r != 0) && (vrc || srb) ) {
       processors_.push_back((VolumeRenderer*)r);
	   enabled_ = true;
	   setProcessor(0);
    }
}

void TransFuncPlugin::setEvaluator(NetworkEvaluator* eval) {
	processors_.clear();
	vrendererCombo_->clear();
	std::vector<Processor*> res = eval->getProcessors();
    for (size_t i=0 ; i < res.size() ; ++i) {
        VolumeRaycaster* vrc = dynamic_cast<VolumeRaycaster*>(res[i]);
        SliceRendererBase* srb = dynamic_cast<SliceRendererBase*>(res[i]);
        if (vrc || srb) {
			vrendererCombo_->addItem(QString::fromStdString(res[i]->getClassName().getName()+" ("+res[i]->getTag().getName()+")"), 
                QString::fromStdString(res[i]->getTag().getName()));
            processors_.push_back((VolumeRenderer*)res[i]);
        }
    }
	if (processors_.size() > 0) {
		enabled_ = true;
		setProcessor(0);
		setMessageReceiver(processors_.at(0));
	}
}

void TransFuncPlugin::setEnabled(bool b) {
	enabled_ = b;
}

void TransFuncPlugin::setVisibleState(bool vis) {
    setVisible(vis);
}

void TransFuncPlugin::changeValue(TransFunc* /*tf*/) {

}

void TransFuncPlugin::removeProcessor(Processor* processor) {
	int position = -1;
	for (size_t i = 0 ; i < processors_.size() ; ++i) {
		if (processors_.at(i) == processor) {
			processors_.erase(processors_.begin() + i);
			position = i;
			break;
		}
	}
	if (position != -1)
		vrendererCombo_->removeItem(position);
}

void TransFuncPlugin::dataSourceChanged(Volume* newDataset) {
    intensityPlugin_->dataSourceChanged(newDataset);
    intensityGradientPlugin_->dataSourceChanged(newDataset);
    intensityGradientPlugin_->clear();
	intensityPetPlugin_->dataSourceChanged(newDataset);

    LGL_ERROR;

    int bits = newDataset->getBitsStored();
    switch (bits) {
        case 8:
            scaleFactor_ = 1.0f/255.0f;
            setSliderValues(0, 255);
            break;
        case 12:
            scaleFactor_ = 1.0f/4095.0f;
            setSliderValues(0, 4095);
            break;
        case 16:
            scaleFactor_ = 1.0f/65535.0f;
            setSliderValues(0, 65535);
            break;
        case 32:
            scaleFactor_ = 1.0f/255.0f;
            setSliderValues(0, 255);
            break;
    }
}

void TransFuncPlugin::setSliderValues(int lowerValue, int upperValue) {
    if (thresholdWidget_) {
        thresholdWidget_->setMinValue(lowerValue);
        thresholdWidget_->setMaxValue(upperValue);
        thresholdWidget_->setValues(lowerValue, upperValue);
        setThresholds(lowerValue, upperValue);
    }
}

void TransFuncPlugin::updateTransferFunction() {
    //re-send the TF msg:
    setEditor(editors_->currentIndex());
}


void TransFuncPlugin::setThresholds(int lower, int upper) {
        if (processors_.size() > 0){
            if (currentVolumeRenderer_) {
                if (prop_) {       
                    currentVolumeRenderer_->postMessage(
                        new FloatMsg(VolumeRenderer::setLowerThreshold_,lower*scaleFactor_),prop_->getMsgDestination());
                    currentVolumeRenderer_->postMessage(
                        new FloatMsg(VolumeRenderer::setUpperThreshold_,upper*scaleFactor_),prop_->getMsgDestination());
                } else {
                    currentVolumeRenderer_->postMessage(
                        new FloatMsg(VolumeRenderer::setLowerThreshold_,lower*scaleFactor_),target_);
                    currentVolumeRenderer_->postMessage(
                        new FloatMsg(VolumeRenderer::setUpperThreshold_,upper*scaleFactor_),target_);
                }
           }
    }

    intensityPlugin_->setThresholds(lower,upper);
    intensityGradientPlugin_->setThresholds(lower,upper);
    if (isVisible())
        repaintCanvases();
}


void TransFuncPlugin::switchInteractionMode(bool on) {
    if (on)
		startTracking();
	else
		stopTracking();
}

void TransFuncPlugin::setEditor(int i) {
    editors_->setCurrentIndex(i);

    if (i==0)
        postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, intensityPlugin_->getTransFunc()), target_);
    else if (i==1) {
        TransFunc* tf = intensityGradientPlugin_->getTransFunc();
        if (!tf)
            tf = intensityPlugin_->getTransFunc();
        postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf), target_);
    }
	 else if (i==2) {
//        TransFunc* tf = intensityPetPlugin_->getTransFunc();
//        if (!tf)
            TransFunc* tf = intensityPlugin_->getTransFunc();
        postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf), target_);
    }
}

void TransFuncPlugin::setMessageReceiver(MessageReceiver* receiver) {
	msgReceiver_ = receiver;
	intensityPlugin_->setMessageReceiver(receiver);
	intensityPlugin_->setReceiver(receiver);
    intensityGradientPlugin_->setMessageReceiver(receiver);
	intensityPetPlugin_->setMessageReceiver(receiver);
}

void TransFuncPlugin::findAndSetProcessor(Processor* r) {
	if (r == 0) {
		for (size_t i=0 ; i < processors_.size() ; ++i) {
			if (processors_.at(i) == PropertySet::getTmpPropSet()) {
				processors_.erase(processors_.begin() + i);
				vrendererCombo_->removeItem(i);
				if (processors_.size() > 0) {
					vrendererCombo_->setCurrentIndex(0);
					setProcessor(0);
				}
				return;
			}
		}
	}
	bool found = false;
	int position = 0;
	for (size_t i = 0 ; i < processors_.size() ; ++i) {
		if (processors_.at(i) == r) {
			found = true;
			position = i;
			break;
		}
	}
	if (found)
		setProcessor(position);
	else {
		VolumeRaycaster* vrc = dynamic_cast<VolumeRaycaster*>(r);
        SliceRendererBase* srb = dynamic_cast<SliceRendererBase*>(r);
        if (vrc || srb) {
			vrendererCombo_->addItem(QString::fromStdString(r->getClassName().getName()+" ("+r->getTag().getName()+")"), 
                QString::fromStdString(r->getTag().getName()));
            processors_.push_back((VolumeRenderer*)r);
			setEnabled(true);
			setProcessor(processors_.size()-1);
		}
	}
}

void  TransFuncPlugin::setProcessor(int i) {
	if (enabled_==false)
		return;
	if (processors_.size() < (size_t)i+1)
		return;
	if (processors_.at(i) == 0)
		return;
    currentVolumeRenderer_ = processors_[i];
	
	//if ( (thresholdWidget_) && (currentVolumeRenderer_->getCurrentDataset()) ) {
    if ( (thresholdWidget_) && (currentVolumeRenderer_->getVolumeHandle()) ) {
	  //int bits = currentVolumeRenderer_->getCurrentDataset()->getVolume()->getBitsStored();
      int bits = currentVolumeRenderer_->getVolumeHandle()->getVolume()->getBitsStored();
      float lowerT = currentVolumeRenderer_->getLowerThreshold();
      float upperT = currentVolumeRenderer_->getUpperThreshold();

	  switch (bits) {
			case 8:
				scaleFactor_ = 1.0f/255.0f;
				//setSliderValues(0, 255);
				thresholdWidget_->setMinValue(0);
				thresholdWidget_->setMaxValue(255);
            	intensityPlugin_->processorChanged(bits); 
			    //setThresholds(lowerT * 255.f,upperT * 255.f); 
				thresholdWidget_->setValues(lowerT * 255.f,upperT * 255.f);  
		    	break;
			case 12:
				scaleFactor_ = 1.0f/4095.0f;
				//setSliderValues(0, 4095);
				thresholdWidget_->setMinValue(0);
				thresholdWidget_->setMaxValue(4095);
				intensityPlugin_->processorChanged(bits); 
			    //setThresholds(lowerT * 4095.f,upperT * 4095.f); 
				thresholdWidget_->setValues(lowerT * 4095.f,upperT * 4095.f);  
		       	break;
			case 16:
				scaleFactor_ = 1.0f/65535.0f;
				//setSliderValues(0, 65535);
				thresholdWidget_->setMinValue(0);
				thresholdWidget_->setMaxValue(65535);
		        thresholdWidget_->setValues(lowerT * 65535.f,upperT * 65535.f); 				
		    	intensityPlugin_->processorChanged(bits); 
				break;
			case 32:
				scaleFactor_ = 1.0f/255.0f;
				//setSliderValues(0, 255);
				thresholdWidget_->setMinValue(0);
				thresholdWidget_->setMaxValue(255);
		        thresholdWidget_->setValues(lowerT * 255.f,upperT * 255.f); 				
				intensityPlugin_->processorChanged(bits); 
				break;
		}
	}


	setMessageReceiver(currentVolumeRenderer_);
    TransFunc* tf = currentVolumeRenderer_->getTransFunc();
    
    QVariant var = vrendererCombo_->itemData(i);
	vrendererCombo_->setCurrentIndex(i);
    QString str = var.toString();
    target_ = str.toStdString();
    intensityGradientPlugin_->setTarget(target_);
    intensityPlugin_->setTarget(target_);
    
    if (tf == 0) {
        //intensityPlugin_->setStandardFunc();
        //intensityGradientPlugin_->clear();
        //setEditor(0);
    }
    else {
        TransFuncIntensityKeys* tfik = dynamic_cast<TransFuncIntensityKeys*>(tf);
        TransFuncIntensityGradientPrimitiveContainer* tfig = dynamic_cast<TransFuncIntensityGradientPrimitiveContainer*>(tf);
        
        if (tfik) {
            intensityPlugin_->setTransFunc(tfik);
            setEditor(0);
        }
        else if (tfig) {
            intensityGradientPlugin_->setTransFunc(tfig);
            setEditor(1);
        }
    }
}

TransFuncIntensityPlugin* TransFuncPlugin::getIntensityPlugin() const {
    return intensityPlugin_;
}

TransFuncIntensityGradientPlugin* TransFuncPlugin::getIntensityGradientPlugin() const {
    return intensityGradientPlugin_;
}

TransFuncIntensityPetPlugin* TransFuncPlugin::getIntensityPetPlugin() const {
    return intensityPetPlugin_;
}


} // namespace voreen

