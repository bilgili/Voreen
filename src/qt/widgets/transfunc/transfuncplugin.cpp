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


#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"

#include "voreen/qt/widgets/thresholdwidget.h"
#include "voreen/qt/widgets/transfunc/transfuncintensitygradientplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensityplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensitypetplugin.h"


namespace voreen {

TransFuncPlugin::TransFuncPlugin(QWidget* parent, MessageReceiver* msgReceiver,
                                 TransFuncProp* prop, Qt::Orientation widgetOrientation)
    : WidgetPlugin(parent, msgReceiver)
    , currentVolumeRenderer_(0)
    , prop_(prop)
    , widgetOrientation_(widgetOrientation)
    , showRendererWidget_(true)
    , msgReceiver_(msgReceiver)
    , showTresholdWidget_(true)
{
    setObjectName(tr("Transfer Function"));
    icon_ = QIcon(":/icons/transferfunc.png");

    if (prop_) {
      msgIdent_ = prop->getIdent();
        for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
            transFuncEditor_.at(i)->setShowThresholdWidget(prop_->getShowThreshold());
    }
      else
        msgIdent_ = VolumeRenderer::setTransFunc_;
}

TransFuncPlugin::~TransFuncPlugin() {
/*    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        delete transFuncEditor_.at(i);
*/
}

void TransFuncPlugin::createWidgets() {
    resize(300,300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    vrendererCombo_ = new QComboBox(0);
    if (showRendererWidget_)
        mainLayout->addWidget(vrendererCombo_);
  
    transFuncEditorTab_ = new QTabWidget(this);

    // Create the first editor at the beginning
    TransFuncEditorPlugin* editor = new TransFuncEditorPlugin(this, msgReceiver_, currentVolumeRenderer_, prop_, widgetOrientation_);
    transFuncEditor_.push_back(editor);
    editor->setShowThresholdWidget(showTresholdWidget_);
    editor->createWidgets();
    editor->createConnections();

    transFuncEditorTab_->addTab(editor , "#0");
   
    mainLayout->addWidget(transFuncEditorTab_, 0, Qt::AlignTop);

    setLayout(mainLayout);
    
    setEnabled(false);
}

void TransFuncPlugin::createConnections() {
    connect(vrendererCombo_, SIGNAL(activated(int)), this, SLOT(setProcessor(int)));
    connect(transFuncEditorTab_, SIGNAL(currentChanged(int)), this, SLOT(setTab(int)));

    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        transFuncEditor_.at(i)->createConnections();
}

void TransFuncPlugin::setShowThresholdWidget(bool show) {
   // for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
     //   transFuncEditor_.at(i)->setShowThresholdWidget(show);
    showTresholdWidget_ = show;
}

void TransFuncPlugin::setShowEditorTypeWidget(bool show) {
    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        transFuncEditor_.at(i)->setShowEditorTypeWidget(show);
}

void TransFuncPlugin::setShowRendererWidget(bool show) {
    showRendererWidget_ = show;
}

void TransFuncPlugin::setThresholdWidget(ThresholdWidget* thresholdWidget) {
    transFuncEditor_.at(transFuncEditorTab_->currentIndex())->setThresholdWidget(thresholdWidget);
}

void TransFuncPlugin::setProcessor(Processor* r) {
	// Delete the currently used processors anyway
    processors_.clear();

    if (r == 0)
        return;

    // Add this processor if it is a VolumeRenderer and has a transfer function
    VolumeRenderer* vr = dynamic_cast<VolumeRenderer*>(r);

    // r is a VolumeRaycaster or a SliceRendererBase
    if (vr && vr->getTransFunc()) {
        processors_.push_back(vr);
        setProcessor(0);
    }
}

void TransFuncPlugin::setProcessors(const std::vector<Processor*> &processors) {
	// Delete the currently used processors anyway
    processors_.clear();

	for (size_t i=0; i<processors.size(); i++) {

		Processor* p = processors[i];

		if (p == 0)
			continue;

		// Add this processor if it is a VolumeRenderer and has a transfer function
		VolumeRenderer* vr = dynamic_cast<VolumeRenderer*>(p);

		// r is a VolumeRaycaster or a SliceRendererBase
		if (vr && vr->getTransFunc()) {
			vrendererCombo_->addItem(QString::fromStdString(vr->getClassName().getName()+" ("+vr->getTag().getName()+")"), 
                QString::fromStdString(vr->getTag().getName()));
			processors_.push_back(vr);
		}
	}

	setProcessor(0);
}

void TransFuncPlugin::setEvaluator(NetworkEvaluator* eval) {
	processors_.clear();
	vrendererCombo_->clear();

    std::vector<Processor*> res = eval->getProcessors();
    for (size_t i=0; i < res.size(); ++i) {
        // Add this processor if it is a VolumeRenderer and has a transfer function
        VolumeRenderer* vr = dynamic_cast<VolumeRenderer*>(res[i]);
        if (vr && vr->getTransFunc()) {
			vrendererCombo_->addItem((res[i]->getClassName().getName() + " (" + res[i]->getTag().getName() + ")").c_str(), 
                                     res[i]->getTag().getName().c_str());
            processors_.push_back(vr);
        }
    }
    
	if (processors_.size() > 0) {
		setProcessor(0);
		//setMessageReceiver(processors_.at(0));
	}
}

std::vector<VolumeRenderer*> TransFuncPlugin::getProcessors() {
    return processors_;
}

void TransFuncPlugin::setVisibleState(bool vis) {
    setVisible(vis);
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

void TransFuncPlugin::dataSourceChanged(Volume* newDataSource) {
    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i) {
        transFuncEditor_.at(i)->dataSourceChanged(newDataSource);
    }
}

void TransFuncPlugin::updateTransferFunction() {
    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        transFuncEditor_.at(i)->updateTransferFunction();
}

void TransFuncPlugin::setThresholds(int lower, int upper) {
    if ( (processors_.size() > 0) && (currentVolumeRenderer_))
        for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
            transFuncEditor_.at(i)->setThresholds(lower, upper);

    if ( isVisible() )
        repaintCanvases();
}


void TransFuncPlugin::switchInteractionMode(bool on) {
    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        transFuncEditor_.at(i)->switchInteractionMode(on);

    if (on)
		startTracking();
	else
		stopTracking();
}

void TransFuncPlugin::setMessageReceiver(MessageReceiver* receiver) {
    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        transFuncEditor_.at(i)->setMessageReceiver(receiver);
    msgReceiver_ = receiver;
}

void TransFuncPlugin::findAndSetProcessor(Processor* r) {
	if (r == 0) {
		for (size_t i=0 ; i < processors_.size() ; ++i) {
			if (processors_.at(i) == PropertySet::getTmpPropSet()) {
				processors_.erase(processors_.begin() + i);
				vrendererCombo_->removeItem(i);
                setEnabled(false);
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
        // Add this processor if it is a VolumeRenderer and has a transfer function
        VolumeRenderer* vr = dynamic_cast<VolumeRenderer*>(r);
        if (vr && vr->getTransFunc()) {
			vrendererCombo_->addItem(QString::fromStdString(r->getClassName().getName()+" ("+r->getTag().getName()+")"), 
                QString::fromStdString(r->getTag().getName()));
            processors_.push_back((VolumeRenderer*)r);
			setProcessor(processors_.size()-1);
		}
        else
            setEnabled(false);
	}
}

void TransFuncPlugin::setProcessor(int i) {
    setEnabled(true);
	if (processors_.size() < static_cast<size_t>(i+1))
		return;
	if (processors_.at(i) == 0)
		return;
    currentVolumeRenderer_ = processors_.at(i);
    
    for (size_t j = 0 ; j < transFuncEditor_.size() ; ++j)
        transFuncEditor_.at(j)->disconnect();

    transFuncEditorTab_->clear();
    transFuncEditor_.clear();

    setMessageReceiver(currentVolumeRenderer_);


    // THIS PART DOES NOT WORK, IF THE VOLUMERENDERER HAS MORE THAN ONE TF
    
    for (int k = 0 ; /*delete this to allow for multiple TFs*/ k < 1 ; ++k) {
        TransFunc* tf = currentVolumeRenderer_->getTransFunc(k);

        if (tf == 0)
            break;
        else {
            TransFuncEditorPlugin* newPlugin = new TransFuncEditorPlugin(this, msgReceiver_, currentVolumeRenderer_, prop_, widgetOrientation_);
            transFuncEditor_.push_back(newPlugin);

            newPlugin->createWidgets();
            newPlugin->createConnections();
            newPlugin->setCurrentVolumeRenderer(currentVolumeRenderer_);
            newPlugin->setMessageReceiver(currentVolumeRenderer_);
            transFuncEditorTab_->addTab(newPlugin , "#" + QString::number(k) );
        }
    }

    target_ = vrendererCombo_->itemData(i).toString().toStdString();
	vrendererCombo_->setCurrentIndex(i);

    for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i) {
        transFuncEditor_.at(i)->setTarget(target_);
        transFuncEditor_.at(i)->getIntensityPlugin()->setTarget(target_);
        transFuncEditor_.at(i)->getIntensityGradientPlugin()->setTarget(target_);
        transFuncEditor_.at(i)->getIntensityPetPlugin()->setTarget(target_);

        TransFunc* tf = currentVolumeRenderer_->getTransFunc(0);
    
        if (tf == 0) {
        }
        else {
            TransFuncIntensity* tfik = dynamic_cast<TransFuncIntensity*>(tf);
            TransFuncIntensityGradientPrimitiveContainer* tfig = dynamic_cast<TransFuncIntensityGradientPrimitiveContainer*>(tf);

            if (tfik) {
                transFuncEditor_.at(i)->getIntensityPlugin()->setTransFunc(tfik);
                //transFuncEditor_.at(i)->setEditor(0);
            }
            else if (tfig) {
                transFuncEditor_.at(i)->getIntensityGradientPlugin()->setTransFunc(tfig);
                //transFuncEditor_.at(i)->setEditor(1);
            }
        }
        transFuncEditor_.at(i)->updateTransferFunction();
    }
}

TransFuncIntensityPlugin* TransFuncPlugin::getIntensityPlugin() const {
    return transFuncEditor_.at(transFuncEditorTab_->currentIndex())->getIntensityPlugin();
}

TransFuncIntensityGradientPlugin* TransFuncPlugin::getIntensityGradientPlugin() const {
    return transFuncEditor_.at(transFuncEditorTab_->currentIndex())->getIntensityGradientPlugin();
}

TransFuncIntensityPetPlugin* TransFuncPlugin::getIntensityPetPlugin() const {
    return transFuncEditor_.at(transFuncEditorTab_->currentIndex())->getIntensityPetPlugin();
}

void TransFuncPlugin::setEditor(int index) {
    if (index > 0)
        transFuncEditor_.at(transFuncEditorTab_->currentIndex())->setEditor(index);
    //for (size_t i = 0 ; i < transFuncEditor_.size() ; ++i)
        //transFuncEditor_.at(i)->setEditor(index);
}

void TransFuncPlugin::setTab(int index) {
    if (index > 0)
        transFuncEditor_.at(index)->updateTransferFunction();
}

//-----------------------------------------------------------------------------

TransFuncEditorPlugin::TransFuncEditorPlugin(QWidget* parent, MessageReceiver* msgReceiver,
                                             VolumeRenderer* processor, TransFuncProp* prop,
                                             Qt::Orientation widgetOrientation)
    : WidgetPlugin(parent, msgReceiver),
      currentVolumeRenderer_(processor),
      widgetOrientation_(widgetOrientation),
      prop_(prop),
      thresholdWidget_(0),
      showThresholdWidget_(true),
      showEditorTypeWidget_(true)
{}

TransFuncEditorPlugin::~TransFuncEditorPlugin() {
}

void TransFuncEditorPlugin::createWidgets() {
    QVBoxLayout* gridLayout = new QVBoxLayout(this);

    editorType_ = new QComboBox(0);
    if (showEditorTypeWidget_) 
        gridLayout->addWidget(editorType_);
    editorType_->addItem(tr("Intensity: Keys"));
    editorType_->addItem(tr("Intensity: Gradient"));
	editorType_->addItem(tr("Intensity: PET"));
    
    intensityPlugin_ = new TransFuncIntensityPlugin(this, msgReceiver_, widgetOrientation_);
    intensityPlugin_->createWidgets();
    intensityPlugin_->createConnections();
    
    intensityGradientPlugin_ = new TransFuncIntensityGradientPlugin(this, msgReceiver_, widgetOrientation_);
    intensityGradientPlugin_->createWidgets();
    intensityGradientPlugin_->createConnections();
    
	intensityPetPlugin_ = new TransFuncIntensityPetPlugin(this, msgReceiver_, widgetOrientation_);
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

    setLayout(gridLayout);
}

void TransFuncEditorPlugin::createConnections() {
    connect(editorType_, SIGNAL(activated(int)), this, SLOT(setEditor(int)));

    if (showThresholdWidget_) {
        connect(intensityPlugin_, SIGNAL(transferFunctionReset()), thresholdWidget_, SLOT(resetThresholds()));
        connect(thresholdWidget_, SIGNAL(valuesChanged(int, int)), this, SLOT(setThresholds(int, int)));
        connect(thresholdWidget_, SIGNAL(sliderPressedChanged(bool)), this, SLOT(switchInteractionMode(bool)) );
    }
}

void TransFuncEditorPlugin::dataSourceChanged(Volume* newDataSource) {
    intensityPlugin_->dataSourceChanged(newDataSource);
    intensityGradientPlugin_->dataSourceChanged(newDataSource);
    intensityGradientPlugin_->clear();
	intensityPetPlugin_->dataSourceChanged(newDataSource);

    LGL_ERROR;

    int bits = newDataSource->getBitsStored();
    switch (bits) {
    case 8:
        setScaleFactor(1.0f/255.0f);
        setSliderValues(0, 255);
        break;
    case 12:
        setScaleFactor(1.0f/4095.0f);
        setSliderValues(0, 4095);
        break;
    case 16:
        setScaleFactor(1.0f/65535.0f);
        setSliderValues(0, 65535);
        break;
    case 32:
        setScaleFactor(1.0f/255.0f);
        setSliderValues(0, 255);
        break;
    }
}

void TransFuncEditorPlugin::setEditor(int i) {
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
        TransFunc* tf = intensityPetPlugin_->getTransFunc();
        if (!tf)
            tf = intensityPlugin_->getTransFunc();
        postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf), target_);
    }
}

void TransFuncEditorPlugin::setThresholdWidget(ThresholdWidget* thresholdWidget) {
    thresholdWidget_ = thresholdWidget;
}

void TransFuncEditorPlugin::setShowThresholdWidget(bool show) {
    showThresholdWidget_ = show;
}

void TransFuncEditorPlugin::setShowEditorTypeWidget(bool show) {
    showEditorTypeWidget_ = show;
}

TransFuncIntensityGradientPlugin* TransFuncEditorPlugin::getIntensityGradientPlugin() {
    return intensityGradientPlugin_;
}

TransFuncIntensityPlugin* TransFuncEditorPlugin::getIntensityPlugin() {
    return intensityPlugin_;
}

TransFuncIntensityPetPlugin* TransFuncEditorPlugin::getIntensityPetPlugin() {
    return intensityPetPlugin_;
}

TransFuncProp* TransFuncEditorPlugin::getTransFuncProp() {
    return prop_;
}

void TransFuncEditorPlugin::setSliderValues(int lowerValue, int upperValue) {
    if (thresholdWidget_) {
        thresholdWidget_->setMinValue(lowerValue);
        thresholdWidget_->setMaxValue(upperValue);
        thresholdWidget_->setValues(lowerValue, upperValue);
        setThresholds(lowerValue, upperValue);
    }
}

void TransFuncEditorPlugin::updateTransferFunction() {
    //re-send the TF msg:
    setEditor(editors_->currentIndex());
}

void TransFuncEditorPlugin::switchInteractionMode(bool on) {
    if (on)
		startTracking();
	else
		stopTracking();
}

void TransFuncEditorPlugin::setThresholds(int lower, int upper) {
    if (currentVolumeRenderer_) {
        if (prop_) {       
            currentVolumeRenderer_->postMessage(
                new FloatMsg(VolumeRenderer::setLowerThreshold_,lower*scaleFactor_),prop_->getMsgDestination());
            currentVolumeRenderer_->postMessage(
                new FloatMsg(VolumeRenderer::setUpperThreshold_,upper*scaleFactor_),prop_->getMsgDestination());
        }
        else {
            currentVolumeRenderer_->postMessage(
                new FloatMsg(VolumeRenderer::setLowerThreshold_,lower*scaleFactor_),target_);
            currentVolumeRenderer_->postMessage(
                new FloatMsg(VolumeRenderer::setUpperThreshold_,upper*scaleFactor_),target_);
        }

        intensityPlugin_->setThresholds(lower, upper);
        intensityGradientPlugin_->setThresholds(lower, upper);

        if ( isVisible() )
            repaintCanvases();
    }
}

void TransFuncEditorPlugin::setMessageReceiver(MessageReceiver* receiver) {
	msgReceiver_ = receiver;
	intensityPlugin_->setMessageReceiver(receiver);
	intensityPlugin_->setReceiver(receiver);
    intensityGradientPlugin_->setMessageReceiver(receiver);
    intensityGradientPlugin_->setReceiver(receiver);
	intensityPetPlugin_->setMessageReceiver(receiver);
    intensityPetPlugin_->setReceiver(receiver);
}

ThresholdWidget* TransFuncEditorPlugin::getThresholdWidget() {
    return thresholdWidget_;
}

void TransFuncEditorPlugin::setCurrentVolumeRenderer(VolumeRenderer* renderer) {
    currentVolumeRenderer_ = renderer;

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
			thresholdWidget_->setValues(static_cast<int>(lowerT * 255.f), static_cast<int>(upperT * 255.f));
	    	break;
		case 12:
			scaleFactor_ = 1.0f/4095.0f;
			//setSliderValues(0, 4095);
			thresholdWidget_->setMinValue(0);
			thresholdWidget_->setMaxValue(4095);
			intensityPlugin_->processorChanged(bits); 
		    //setThresholds(lowerT * 4095.f,upperT * 4095.f); 
			thresholdWidget_->setValues(static_cast<int>(lowerT * 4095.f), static_cast<int>(upperT * 4095.f));
	       	break;
		case 16:
			scaleFactor_ = 1.0f/65535.0f;
			//setSliderValues(0, 65535);
			thresholdWidget_->setMinValue(0);
			thresholdWidget_->setMaxValue(65535);
	        thresholdWidget_->setValues(static_cast<int>(lowerT * 65535.f), static_cast<int>(upperT * 65535.f));
	    	intensityPlugin_->processorChanged(bits); 
			break;
		case 32:
			scaleFactor_ = 1.0f/255.0f;
			//setSliderValues(0, 255);
			thresholdWidget_->setMinValue(0);
			thresholdWidget_->setMaxValue(255);
	        thresholdWidget_->setValues(static_cast<int>(lowerT * 255.f), static_cast<int>(upperT * 255.f));
			intensityPlugin_->processorChanged(bits); 
			break;
		}
    } // if ( (thresholdWidget_) && (currentVolumeRenderer_->getVolumeHandle()) )
}

void TransFuncEditorPlugin::setTarget(Identifier target) {
    target_ = target;
}

void TransFuncEditorPlugin::setScaleFactor(float factor) {
    scaleFactor_ = factor;
}

bool TransFuncEditorPlugin::getShowThresholdWidget() {
    return showThresholdWidget_;
}

} // namespace voreen
