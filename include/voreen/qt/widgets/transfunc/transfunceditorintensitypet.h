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

#ifndef VRN_TRANSFUNCEDITORINTENSITYPET_H
#define VRN_TRANSFUNCEDITORINTENSITYPET_H

#include "voreen/qt/widgets/transfunc/transfunceditor.h"

#include "tgt/vector.h"

#include <vector>

namespace tgt {
    class QtCanvas;
}

class QPushButton;
class QSpinBox;
class QToolButton;

namespace voreen {

class DoubleSlider;
class HistogramPainter;
class TransFuncIntensity;
class TransFuncTexturePainter;

/**
 * TransFuncEditorIntensityPet is an editor widget for transfer functions with focus on PET data.
 * It provides a widget which displays the histogram of the associated dataset, two gradients that
 * display the transferfunction and control elements to adjust the upper and lower threshold.
 * Furthermore you can zoom into threshold area by pressing expand. You can undo that with a press
 * on collapse.
 */
class TransFuncEditorIntensityPet : public TransFuncEditor {
    Q_OBJECT

public:
    /**
     * Constructor - inits member variables to default and creates histogrampainter
     *
     * @param prop the transfer function property
     * @param parent parent widget
     */
    TransFuncEditorIntensityPet(TransFuncProperty* prop, QWidget* parent = 0);

    /**
     * Destructor - deletes transfer function for expanded gradient
     */
    ~TransFuncEditorIntensityPet();

    /**
     * Creates the layout for the widget. The widget consists of a Histogrampainter,
     * 2 gradients, a doubleSlider, 2 spinboxes and 4 pushbuttons. They are arranged in a
     * vertical layout.
     */
    void createWidgets();

    /**
     * Connects all control elements with apropriate slots.
     */
    void createConnections();

    /**
     * Tests whether the volume associated with the transFuncProperty has changed and updates
     * the editor to the new volume when necessary.
     */
    void update();

    /**
     * Updates the histogram and minimum and maximum threshold values. Additionally the volume
     * is passed to the mapping canvas where the histogram is calculated.
     * The transfer function and current thresholds remain the same.
     */
    void volumeChanged();

    /**
     * Stores the current set thresholds in given parameters.
     *
     * @param l the value of lower threshold is copied into this parameter
     * @param u the value of upper threshold is copied into this parameter
     */
    void getThresholds(int& l, int& u);

    /**
     * Tests whether the value inside the transFuncProp is the same as transfuncIntensity_.
     * A new transfer function is created if they do not match. The transfer function is reset if
     * they match. Furthermore the thresholds are reset.
     */
    void resetEditor();

public slots:
    /**
     * Opens a fileDialog and loads the selected file. The texture canvases are updated
     * according to the new transfer function.
     */
    void loadTransferFunction();

    /**
     * Opens a filedialog and saves the transfer function to the desired file.
     */
    void saveTransferFunction();

    /**
     * Marks the texture of the transfer function as invalid.
     */
    void updateTransferFunction();

    /**
     * Slot for press on reset button. Calls resetTransferFunction() and resetThresholds().
     */
    void resetButtonClicked();

    /**
     * Slot that is called from doubleSlider when the user dragged the doubleslider.
     *
     * @param lower new lower threshold
     * @param upper new upper threshold
     */
    void sliderChanged(float lower, float upper);

    /**
     * Slot that is called when the user changes the value in lowerThreshold spinbox.
     *
     * @param value new lower threshold
     */
    void minBoxChanged(int value);

    /**
     * Slot that is called when the user changes the value in upperThreshold spinbox.
     *
     * @param value new upper threshold
     */
    void maxBoxChanged(int value);

    /**
     * Slot that is called when the user presses the expand button.
     * The current range_ is saved and the doubleSlider is reset.
     */
    void expandGradient();

    /**
     * Slot that is called when the user presses the collapse button.
     * The range before the last expand is restored. The doubleSlider is also set to the
     * position before expand.
     */
    void collapseGradient();

    /**
     * Notifies the transfer function property about a change in the transfer function
     * and emits a signal for repaint of the volume rendering.
     */
    void causeVolumeRenderingRepaint();

private:
    /**
     * Calls repaint for double slider, both gradients and the histogrampainter.
     */
    void repaintAll();

    /**
     * Resets transfer function to default.
     */
    void resetTransferFunction();

    /**
     * Applies the current set thresholds to the transfer function.
     */
    void applyThreshold();

    /**
     * Resets the threshold to default values. The control elements are updated as well.
     */
    void resetThresholds();

    /**
     * Reads the thresholds from the transfer function and assigns them to the widgets.
     */
    void restoreThresholds();

    /**
     * Tests whether the transfer function is correct for this editor. Therefor is tested whether
     * the alpha value of all keys of the transfer function is set to 255.
     *
     * @return true when alpha value of all keys is set to 255, false otherwise
     */
    bool transferFunctionCorrect();

    /**
     * Sets the alpha value of all keys of the transfer function to 255.
     */
    void rectifyTransferFunction();

    QToolButton* loadButton_;      ///< button for loading a transfer function
    QToolButton* saveButton_;      ///< button for saving a transfer function
    QToolButton* resetButton_;     ///< button for resetting transfer function to default
    QToolButton* repaintButton_;   ///< button for forcing a repaint of the volume rendering
    QPushButton* expandButton_;    ///< button for expanding the lower transfer function gradient
    QPushButton* collapseButton_;  ///< button for collapsing the lower transfer function gradient
    QSpinBox* lowerThresholdSpin_; ///< spinbox for adjusting the lower threshold
    QSpinBox* upperThresholdSpin_; ///< spinbox for adjusting the upper threshold

    DoubleSlider* doubleSlider_;                ///< slider for adjusting the transfer function
    tgt::QtCanvas* completeTextureCanvas_;      ///< canvas in which the texture of the transfer function is displayed
    TransFuncTexturePainter* completeTexturePainter_; ///< painter for texture display of transfer function
    tgt::QtCanvas* expandedTextureCanvas_;      ///< canvas in which the expanded transfer function is displayed
    TransFuncTexturePainter* expandedTexturePainter_; ///< painter for texture display of expanded transfer function
    TransFuncIntensity* transferFuncIntensity_; ///< transfer function that is applied to the dataset
    TransFuncIntensity* transferFuncGradient_;  ///< transfer function for display in expanded gradient

    HistogramPainter* histogramPainter_; ///< painter that draws the histogram

    int maximumIntensity_;   ///< maximum intensity that can occur in the volume dataset
    tgt::vec2 oldThreshold_; ///< threshold that was used in applyThreshold() for calculation of intensities

    std::vector<tgt::ivec2> ranges_; ///< vector with all ranges that were set by expanding
    tgt::ivec2 currentRange_; ///< current range of thresholds that can be set by doubleslider or spinboxes

    static const std::string loggerCat_; ///< the logger category
};

} // namespace voreen

#endif // VRN_TRANSFUNCEDITORINTENSITYPET_H
