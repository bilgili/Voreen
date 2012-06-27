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

#ifndef VRN_TRANSFUNCPLUGIN_H
#define VRN_TRANSFUNCPLUGIN_H

#include <list>

#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QSlider>
#include <QSpinBox>

#include "tgt/event/eventlistener.h"

#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/qt/widgets/widgetplugin.h"

#include "voreen/qt/widgets/thresholdwidget.h"

namespace voreen {

class TransFuncIntensityGradientPlugin;
class TransFuncIntensityPlugin;
class TransFuncIntensityPetPlugin;
class TransFuncProp;

/**
*   This widgets contains a combobox to choose the target raycaster and TF type,
*   TF editors (as stacked widget) und a thresholdwidget.
*/
class TransFuncPlugin : public WidgetPlugin, 
    tgt::EventListener {
    Q_OBJECT

public:
    TransFuncPlugin(QWidget* parent, MessageReceiver* msgReceiver, TransFuncProp* prop = 0, Qt::Orientation widgetOrientation = Qt::Vertical);
    virtual ~TransFuncPlugin();

    void dataSourceChanged(Volume* newDataset);
    void setVisibleState(bool vis);
    void changeValue(TransFunc* tf);
    void setShowThresholdWidget(bool show);
    void setShowEditorTypeWidget(bool show);
    void setShowRendererWidget(bool show);
    virtual void createWidgets();
    virtual void createConnections();

	/**
	* Sets the MessageReceiver. This is neccessary because depending on which renderer is
	* currently selected in RptGui (or in the combo box), the message must only be sent
	* to certain renderers. 
	*/
	void setMessageReceiver(MessageReceiver* receiver);

    void setProcessor(Processor* r);
	
	/**
	* Checks if the given processor is already in the processors_ vector, and if so,
	* sets it to the active processor. If it's not in the vector, nothing is done.
	*/
	void findAndSetProcessor(Processor* r);

	/**
	* Puts all processors in the evaluator into the processors_ vector and sets the
	* first one to active.
	*/
	void setEvaluator(NetworkEvaluator* eval);

	/**
	* Removes a processor from the processors_ vector. This is neccessary because with the new 
	* render system we cannot be sure that all processors always exist. At the moment the 
	* destructor of the processor class posts a message that is catched in RptMainWindow,
	* and this function is called. This is very bad, but we can't inherite from Messagereceiver
	* here, because the postMessage function is already defined in WidgetPlugin. (s_rade02)
	*/
	void removeProcessor(Processor* processor);

    /// Returns the internally created transfer function intensity plugin.
    TransFuncIntensityPlugin* getIntensityPlugin() const;
    /// Returns the internally created transfer function intensity-gradient plugin.
    TransFuncIntensityGradientPlugin* getIntensityGradientPlugin() const;
    /// Returns the internally created transfer function intensity-pet plugin.
    TransFuncIntensityPetPlugin* getIntensityPetPlugin() const;

	void setEnabled(bool b);

public slots:
    void updateTransferFunction();
    void setSliderValues(int lowerValue, int upperValue);
    void setThresholds(int l, int u);
    void switchInteractionMode(bool on);
    void setThresholdWidget(ThresholdWidget* thresholdWidget) {thresholdWidget_=thresholdWidget;}
    void setEditor(int i);
    void setProcessor(int i);

private:
    Identifier msgIdent_;
    TransFuncProp* prop_;

    TransFuncIntensityGradientPlugin* intensityGradientPlugin_;
    TransFuncIntensityPlugin* intensityPlugin_;
	TransFuncIntensityPetPlugin* intensityPetPlugin_;

    QComboBox* vrendererCombo_;
    VolumeRenderer* currentVolumeRenderer_;

    QStackedWidget* editors_;
    QComboBox*  editorType_;

    Qt::Orientation widgetOrientation_;

    ThresholdWidget* thresholdWidget_;
    float scaleFactor_;
    int maxValue_;
    
    Identifier target_;

    bool showThresholdWidget_;
    bool showRendererWidget_;
    bool showEditorTypeWidget_;

    tgt::ivec2 lastMousePos_;
    
    std::vector<VolumeRenderer*> processors_;

	bool enabled_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCPLUGIN_H
