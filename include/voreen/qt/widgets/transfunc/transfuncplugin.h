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

#include "tgt/event/eventlistener.h"
#include "voreen/qt/widgets/widgetplugin.h"

// Forward declarations
class list;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QSpinBox;
class QStackedLayout;
class QStackedWidget;
class QTabWidget;
class QToolButton;

namespace voreen {

class NetworkEvaluator;
class Processor;
class TransFunc;
class TransFuncEditorPlugin;
class TransFuncIntensityGradientPlugin;
class TransFuncIntensityPlugin;
class TransFuncIntensityPetPlugin;
class TransFuncProp;
class ThresholdWidget;
class Volume;
class VolumeRenderer;

/**
* This widgets contains a combobox to choose the target raycaster and (possibly multiple)
* TransFuncEditorPlugin and is able to control them.
* The plugin will determine how many transfer functions the current processor has and therefore
* how many TransFuncEditorPlugin's have to be created. If just one is needed, there will be no
* tabbed display.
* This is done to minimize the programmers necessity to deal with the TransFuncEditorPlugins himself.
* \sa TransFuncEditorPlugin
*/
class TransFuncPlugin : public WidgetPlugin, 
    tgt::EventListener {
    Q_OBJECT

public:
    /**
    * Default constructor 
    */
    TransFuncPlugin(QWidget* parent, MessageReceiver* msgReceiver, TransFuncProp* prop = 0, Qt::Orientation widgetOrientation = Qt::Vertical);
    
    /**
     * Currently empty virtual destructor
     */
    virtual ~TransFuncPlugin();

    /**
    * In this method all the other widgets will be created and layouted
    */
    virtual void createWidgets();

    /**
    * This method creates the necessary connections to the combobox and to the
    * TransFuncEditorPlugins
    */
    virtual void createConnections();

    /**
    * Propagates the boolean value to the TransFuncEditorPlugins
    * \param show Should the ThresholdWidget be visible?
    */
    void setShowThresholdWidget(bool show);

    /**
    * Propagates the boolean value to the TransFuncEditorPlugins
    * \param show Should the EditorType combobox be visible?
    */
    void setShowEditorTypeWidget(bool show);

    /**
    * Do you want to show the renderer combobox?
    * \param show Should the Renderer combobox be visible?
    */
    void setShowRendererWidget(bool show);

    /**
    * Propagates the changed to the TransFuncEditorPlugins
    * \param newDataSource The new volume which the editors should use
    */
    void dataSourceChanged(Volume* newDataSource);
    
    /**
    * Just sets the visibility of this object
    * \param vis Should this be visible?
    */
    void setVisibleState(bool vis);

	/**
	* Sets the MessageReceiver. This is neccessary because depending on which renderer is
	* currently selected in RptGui (or in the combo box), the message must only be sent
	* to certain renderers. 
	*/
	void setMessageReceiver(MessageReceiver* receiver);

    /**
    * Sets the processor as the only processor in the corresponding vector.
    * All other processors will be removed and if r is null, the vector will be empty.
    * \param r The new processor which is to be added.
    */
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

    /**
    * Returns the internally created transfer function intensity plugin of the currently
    * selected editor.
    * \return The transfer function intensity plugin
    */
    TransFuncIntensityPlugin* getIntensityPlugin() const;

    /**
    * Returns the internally created transfer function intensity-gradient plugin of the
    * currently selected editor.
    * \return The transfer function gradient plugin
    */
    TransFuncIntensityGradientPlugin* getIntensityGradientPlugin() const;
    
    /** Returns the internally created transfer function intensity-pet plugin of the
    * currently selected editor.
    * \return The transfer function intensity-pet plugin
    */
    TransFuncIntensityPetPlugin* getIntensityPetPlugin() const;

public slots:
    /**
    * Sets the threshold widget of the currently selected TransFuncEditorPlugin to the
    * new provided one.
    * \param thresholdWidget The new thresholdWidget will should be used by the current Editor
    */
    void setThresholdWidget(ThresholdWidget* thresholdWidget);

    /**
    * Activates the processor stored in processors_ at position i and propagates him
    * to the TransFuncEditors
    * \param i The number of the processor
    */
    void setProcessor(int i);

    /**
    * Propagates the signal to the TransFuncEditors
    */
    void updateTransferFunction();
    
    /**
    * Propagates the signal to the TransFuncEditors and repaint everything (if
    * it is visible)
    * \param lower The lower bounds for the threshold
    * \param upper The upper bounds for the threshold
    */
    void setThresholds(int lower, int upper);
    
    /**
    * Starts or stops the tracking and propagates the signal to the TransFuncEditos
    * \param on Interaction mode on or off?
    */
    void switchInteractionMode(bool on);

    /**
    * Sets the appropriate editor on all TransFuncEditors
    * \param index The index of the editor which should be activated
    */
    void setEditor(int index);

    /**
    * Brings the assigned tab up to front an activates the transfer function within
    * \param index The index of the tab we want to activate
    */
    void setTab(int index);

private:
    // The tab which shows the currently accessible transfer functions
    QTabWidget* transFuncEditorTab_;
    // A combobox which shows the currently accessible processors
    QComboBox* vrendererCombo_;

    // All the accessible transfer functions
    std::vector<TransFuncEditorPlugin*> transFuncEditor_;
    // All the accessible processors
    std::vector<VolumeRenderer*> processors_;

    // The currently selected volume renderer
    VolumeRenderer* currentVolumeRenderer_;

    Identifier target_;
    TransFuncProp* prop_;
    Qt::Orientation widgetOrientation_;

    // Should the processor combobox be painted?
    bool showRendererWidget_;

    tgt::ivec2 lastMousePos_;

    MessageReceiver* msgReceiver_;

    Identifier msgIdent_;
};

//-----------------------------------------------------------------------------

/**
* This class is for editing a single transfer function. Normally it will be used in conjunction
* with a TransFuncPlugin, which creates (possibly multiple) instances of this class.
* \sa TransFuncPlugin
* \sa TransFunc
* \sa TransFuncIntensityPlugin
* \sa TransFuncIntensityGradientPlugin
* \sa TransFuncIntensityPetPlugin
*/
class TransFuncEditorPlugin : public WidgetPlugin {
    Q_OBJECT

public:
    /**
    * Default constructor. Nothing special will be done here.
    * \sa QWidget
    * \sa MessageReceiver
    * \sa VolumeRenderer
    * \sa TransFuncProp
    */
    TransFuncEditorPlugin(QWidget* parent, MessageReceiver* msgReceiver, VolumeRenderer* processor = 0, TransFuncProp* prop = 0, Qt::Orientation widgetOrientation = Qt::Horizontal);
    
    /**
    * Default destructor. Nothing special will be done here.
    */
    virtual ~TransFuncEditorPlugin();

    /**
    * In this method all the other widgets will be created and layouted
    */
    void createWidgets();

    /**
    * This method creates the connection to the editor chooser
    */
    void createConnections();

    /**
    * Changes the volume data used by this class.
    * Informs the intensity plugin, gradient plugin and the pet plugin about the new volume
    * \param newDataSource The new volume we want to use from now on
    * \sa Volume
    */
    void dataSourceChanged(Volume* newDataset);

    /**
    * Returns the gradient plugin
    * \return The gradient plugin used by this instance.
    * \sa TransFuncIntensityGradientPlugin
    */
    TransFuncIntensityGradientPlugin* getIntensityGradientPlugin();
    
    /**
    * Returns the intensity plugin
    * \return The intensity plugin used by this instance.
    * \sa TransFuncIntensityPlugin
    */
    TransFuncIntensityPlugin* getIntensityPlugin();
    
    /**
    * Returns the pet plugin
    * \return The pet plugin used by this instance.
    * \sa TransFuncIntensityPetPlugin
    */
    TransFuncIntensityPetPlugin* getIntensityPetPlugin();
    
    /**
    * Returns the transfer function property
    * \return The transfer function property
    * \sa TransFuncProp
    */
    TransFuncProp* getTransFuncProp();
    
    /**
    * Returns the scale factor, by which the threshold will be streched
    * \return The scale factor
    */
    float getScaleFactor();

    /**
    * Returns the currently used threshold widget
    * \return The threshold widget
    * \sa Thresholdwidget
    */
    ThresholdWidget* getThresholdWidget();
    
    /**
    * Returns if the threshold widget used by this instance should be painted
    * \return Should the threshold widget be painted?
    */
    bool getShowThresholdWidget();

    /**
    * Sets the visibility of the threshold widget
    * \param show Should the threshold widget be visible?
    */
    void setShowThresholdWidget(bool show);
    
    /**
    * Sets the visibility of the editor chooser combobox
    * \param show Should the editor combobox be painted?
    */
    void setShowEditorTypeWidget(bool show);
    
    /**
    * Sets the scale factor, by which the threshold will be streched
    * \param factor The scale factor
    */
    void setScaleFactor(float factor);
    
    /**
    * Sets the message receiver used by this instance.
    * \param receiver The message receiver
    * \sa MessageReceiver
    */
    void setMessageReceiver(MessageReceiver* receiver);
    
    /**
    * Changes the renderer used by this editor
    * \param renderer The new renderer
    * \sa VolumeRenderer
    */
    void setCurrentVolumeRenderer(VolumeRenderer* renderer);
    
    // FIXME: Elaborate doxygen comment needed by someone with insight into this
    void setTarget(Identifier TransFuncProp);

public slots:
    /**
    * Sets the upper and lower boundaries for the transfer function
    * \param lower The lower boundary
    * \param upper The upper boundary
    */
    void setThresholds(int lower, int upper);
    
    /**
    * Sets the threshold widget sliders to the assigned values
    * \param lowerValue The lower value
    * \param upperValue The upper value
    */
    void setSliderValues(int lowerValue, int upperValue);
    
    /**
    * Starts or stops the tracking and propagates the signal to the TransFuncEditos
    * \param on Interaction mode on or off?
    */
    void switchInteractionMode(bool on);
    
    /**
    * Sets a new threshold widget for this editor
    * \param thresholdWidget The new threshold widget
    * \sa ThresholdWidget
    */
    void setThresholdWidget(ThresholdWidget* thresholdWidget);
    
    /**
    * Repaints the current transfer function
    */
    void updateTransferFunction();
    
    /**
    * Sets the editor combobox to the assigned value
    * \param index The number of the editor we want to use
    */
    void setEditor(int index);

private:
    // The currently used volume renderer
    VolumeRenderer* currentVolumeRenderer_;
    
    // The different editor widgets in a layout
    QStackedWidget* editors_;
    // The combobox from which we choose an editor
    QComboBox* editorType_;

    Qt::Orientation widgetOrientation_;

    // The tranfer function property we use
    TransFuncProp* prop_;

    // FIXME: Elaborate doxygen comment needed by someone with insight into this
    Identifier target_;
    
    // The gradient plugin we use in this editor
    TransFuncIntensityGradientPlugin* intensityGradientPlugin_;
    // The intensity plugin we use in this editor
    TransFuncIntensityPlugin* intensityPlugin_;
	// The pet plugin we use in this editor
    TransFuncIntensityPetPlugin* intensityPetPlugin_;    
    // The threshold widget we use in this editor
    ThresholdWidget* thresholdWidget_;
    // The scale factor, by which the threshold (originally from 0 to 1) will be streched
    float scaleFactor_;
    
    int maxValue_;

    // Should the threshold widget be painted?
    bool showThresholdWidget_;
    // Should the editor combobox be painted?
    bool showEditorTypeWidget_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCPLUGIN_H
