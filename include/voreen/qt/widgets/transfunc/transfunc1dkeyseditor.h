/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_TRANSFUNC1DKEYSEDITOR_H
#define VRN_TRANSFUNC1DKEYSEDITOR_H

#include "voreen/qt/widgets/transfunc/transfunceditor.h"

namespace tgt {
    class QtCanvas;
}

class QCheckBox;
class QComboBox;
class QLabel;
class QLayout;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QToolButton;
class QPushButton;
class QMenu;

namespace voreen {

class DoubleSlider;
class ColorLuminancePicker;
class ColorPicker;
class TransFunc1DKeys;
class TransFuncMappingCanvas;
class TransFunc1DKeysPainter;
class Histogram1D;

/**
 * TransFuncEditorIntensity is an editor widget for an intensity transfer function.
 * It provides a mapping canvas which displays the keys of the transfer function and the histogram
 * of the associated dataset, a canvas that shows the texture of the transfer function and control
 * elements to adjust the upper and lower thresholds. Furthermore you can change the color of a key
 * with a color and luminancepicker.
 */
class TransFunc1DKeysEditor : public TransFuncEditor {
    Q_OBJECT

public:
    /**
     * Standard constructor.
     *
     * @param prop the transfer function property
     * @param parent the parent frame to which this is added
     */
    TransFunc1DKeysEditor(TransFuncProperty* prop, QWidget* parent = 0);

    /**
     * Standard destructor
     */
    ~TransFunc1DKeysEditor();

    /**
     * Assigns the passed transfer function property.
     */
    void setTransFuncProp(TransFuncProperty* prop);

    /**
     * Returns the currently assigned transfer function property.
     */
    const TransFuncProperty* getTransFuncProp() const;

    /**
     * Creates the whole layout of this widget.
     */
    void createWidgets();

    /**
     * Creates the connections for all control elements.
     */
    void createConnections();

    /**
     * Tests whether the volume associated with the transFuncProperty has changed and updates
     * the editor to the new volume when necessary.
     */
    void updateFromProperty();

    /**
     * Tests whether the value inside the transFuncProp is the same as transfuncIntensity_.
     * A new transfer function is created if they do not match. The transfer function is reset if
     * they match. Furthermore the thresholds are reset.
     */
    void resetEditor();

public slots:
    /**
     * Slot for press on reset button. Calls resetThresholds(), resetTransferFunction(),
     * repaints the control elements, notifies the property about the change and emits
     * a repaint for the volume rendering.
     */
    void clearButtonClicked();

    /**
     * Tells the transfer function that the texture is invalid and emits repaint signal for the
     * volume rendering.
     */
    void updateTransferFunction();

    /**
     * Slot for a click on load button. Opens a fileDialog and loads the selected
     * file. The gradient widget and the mapping canvas are updated with new transfer function.
     */
    void loadTransferFunction();

    /**
     * Slot for a click on save button. Opens a fileDialog and saves the transfer function
     * to the desired file.
     */
    void saveTransferFunction();

    /// Transform the currently loaded TF into a ramp.
    void makeRamp();

    /// Inverts the currently loaded TF.
    void invertMap();

    /**
     * This Function is called if one of the mapping spinboxes have changed.
     */
    void mappingChanged();

    /**
     * This Function is called if the lower mapping spinbox has changed.
     * Also mappingChanged() is called.
     */
    void lowerMappingChanged(double value);

    /**
     * This Function is called if the upper mapping spinbox has changed.
     * Also mappingChanged() is called.
     */
    void upperMappingChanged(double value);

    void domainFittingStrategyChanged(int index);

    /**
     * Resets the transfer function to default.
     */
    void resetTransferFunction();

    /**
     * Applies the current set thresholds to the transfer function.
     */
    void applyThreshold(bool fromSlider);
    void applySliderThreshold() {applyThreshold(true);}
    void applySpinThreshold() {applyThreshold(false);}

    /**
     * Applies the current gamma to the transfer function.
     */
    void gammaChanged(double gamma);

    /**
     * Sets the alpha mode of the transfer function.
     */
    void setAlpha(QAction* action);

    /**
     * This slot is called whenever the color of a key was changed.
     * The new color is propagated to the mapping canvas.
     *
     * @param h new hue of the key
     * @param s new saturation of the key
     * @param v new intensity of the key
     *
     */
    void markerColorChanged(int h, int s, int v);

    /**
     * Notifies the transfer function property about a change in the transfer function
     * and emits a signal for repaint of the volume rendering.
     */
    void causeVolumeRenderingRepaint();

    /**
     * Fits the domain of he TF to the given volume. The new domain is the min and max value of the volume values.
     */
    void fitDomainToData();

    // VolumeObserver methods: //TODO: REMOVE
    virtual void volumeDelete(const VolumeBase* source) {}
    virtual void volumeChange(const VolumeBase* source) {}
    virtual void derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData) {}

protected:
    void updateAlphaButton(TransFunc::AlphaMode mode);
    /**
     * Calls the repaint method for doubleslider, mapping canvas
     * and texture canvas.
     */
    void repaintAll();

    /**
     * Creates the layout with mapping canvas and threshold control elements.
     *
     * @return the layout with mapping canvas and threshold control elements
     */
    QLayout* createMappingLayout();

    /**
     * Creates the layout with colorpicker and luminancepicker.
     *
     * @return the layout with colorpicker and luminancepicker
     */
    QLayout* createColorLayout();

    /**
     * Creates the layout with load, save and reset button.
     *
     * @return the layout with load, save and reset button
     */
    QLayout* createBaseButtonLayout();

    QLayout* createDomainLayout();

    QLayout* createTFLayout();


    /// Check if the mapping makes sense, otherwise highlight widgets as warning //FIXME: not in Windows
    void checkDomainVersusData();

    /**
     * Updates the threshold slider.
     */
    void updateThresholdFromProperty();

    /**
     * Updates the mapping spinboxes after values of the boxes have been set.
     * updateThresholdFromProperty has to be called afterwards to get the threshold konsistent.
     *
     * @param fromTF if true, the values of the boxes are set to transferFuncintensity_.domain
     */
    void updateMappingSpin(bool fromTF = false);

    /// Updates the display of data bounds (volume min/max values).
    void updateDataBounds();

    TransFuncMappingCanvas* transCanvas_;       ///< mapping canvas
    TransFunc1DKeys* transferFuncIntensity_; ///< the transfer function that is editet with this widget
    tgt::QtCanvas* textureCanvas_;              ///< canvas that is used for displaying the texture of the transfer function
    TransFunc1DKeysPainter* texturePainter_;   ///< painter for texture display of transfer function

    QToolButton* loadButton_;               ///< button for loading a transfer function
    QToolButton* saveButton_;               ///< button for saving a transfer function
    QToolButton* clearButton_;              ///< button for resetting transfer function to default
    QToolButton* makeRampButton_;           ///< button to transform a TF into a ramp
    QToolButton* invertButton_;             ///< button to invert the TF
    ColorPicker* colorPicker_;              ///< picker for choosing the color of a key in transfer function
    ColorLuminancePicker* colorLumPicker_;  ///< picker for choosing the alpha value of a key
    DoubleSlider* thresholdSlider_;            ///< 2 slider for adjusting the thresholds
    QDoubleSpinBox* lowerDomainSpin_;          ///< spinbox for lower mapping value
    QDoubleSpinBox* upperDomainSpin_;          ///< spinbox for upper mapping value
    QDoubleSpinBox* lowerThresholdSpin_;          ///< spinbox for lower mapping value
    QDoubleSpinBox* upperThresholdSpin_;          ///< spinbox for upper mapping value
    QLabel* lowerData_;          ///< label of lower volume bound
    QLabel* upperData_;          ///< label of opper volume bound
    QPushButton* fitDomainToData_;
    QComboBox* domainFittingStrategy_;

    QToolButton* alphaButton_;
    QMenu* alphaMenu_;
    QDoubleSpinBox* gammaSpin_;          ///< spinbox for upper mapping value

    int maxDigits_; ///< maximal digits of domain and threshold

    const Histogram1D* histogram_; ///< histogram used for domain bounds and passed to mapping canvas
    float domainMinValue_;
    float domainMaxValue_;

    static const std::string loggerCat_; ///< the logger category
};

} // namespace voreen

#endif // VRN_TRANSFUNC1DKEYSEDITOR_H
