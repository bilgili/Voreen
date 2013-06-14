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

#ifndef VRN_TRANSFUNC2DPRIMITIVESPAINTER_H
#define VRN_TRANSFUNC2DPRIMITIVESPAINTER_H

#include "tgt/vector.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"

namespace tgt {
    class GLCanvas;
    class Texture;
}

#include <QObject>

namespace voreen {

class VolumeHistogramIntensityGradient;
class TransFunc2DPrimitives;
class TransFuncPrimitive;
class VolumeBase;

/**
 * Painter implementation for drawing onto a tgt::QtCanvas for editing 2D transfer functions.
 * There are 2 types of primitives that can be used to specify the transfer function, a banana
 * and a quad. Every primitive can be colored and moved. Furthermore you can change the extent
 *  and shape of the primitive by dragging the control points.
 */
class TransFunc2DPrimitivesPainter : public QObject, public tgt::Painter, public tgt::EventListener {
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param canvas the canvas that uses this painter for painting
     */
    TransFunc2DPrimitivesPainter(tgt::GLCanvas* canvas);

    /**
     * Destructor
     */
    ~TransFunc2DPrimitivesPainter();

    /**
     * Handles the event when a mouse button is pressed.
     * Determines whether a primitive is under the mouse cursor or not and selects it
     * if possible.
     *
     * @param event the mouse event with mouse coordinates
     */
    void mousePressEvent(tgt::MouseEvent* event);

    /**
     * Handles the events when the mouse is moved while a button is pressed.
     * Moves the selected primitive by the movement of mouse. Nothing happens when
     * no mousebutton is pressed or no primitive is selected.
     *
     * @param event the mouse event with mouse coordinates
     */
    void mouseMoveEvent(tgt::MouseEvent* event);

    /**
     * Handles the events when a pressed mosue button is released. It turns the coarseness
     * mode off when a primitive was moved.
     *
     * @param event the mouse event
     */
    void mouseReleaseEvent(tgt::MouseEvent* event);

    /**
     * Handles mouse doubleclick events. Opens a colordialog when the doubleclick occured
     * on a primitive.
     *
     * @param event the mouse event
     */
    void mouseDoubleClickEvent(tgt::MouseEvent* event);

    /**
     * Handles events of the mousewheel. It changes the transparency of the selected
     * primitive.
     *
     * @param event the mouse event
     */
    void wheelEvent(tgt::MouseEvent* event);

    /**
     * Updates the texture of the transfer function. Therefore the updateTexture()-method of the
     * transfer function object is called. A repaint of the volume rendering is triggered after
     * that.
     */
    void updateTF();

    /**
     * Marks the given primitive as selected. It is tested whether a control point was selected
     * or the whole primitive.
     *
     * @param p the selected primitive
     * @param mousePos position of the mouse cursor
     */
    void selectPrimitive(TransFuncPrimitive* p, tgt::vec2 mousePos = tgt::vec2(0.f));

    /**
     * Resets the selection state of the current selected primitive.
     */
    void deselectPrimitive();

    /**
     * Paints the histogram texture, the grid when enabled and the primitives the transfer function
     * consists of.
     */
    void paint();

    /**
     * Initializes the painter. The modelviewMatrix is set to identity and the projection is set to
     * orthographic.
     */
    void initialize();

    /**
     * Called when the size of the canvas changes. Adapts the projection matrix to new size.
     *
     * @param size new size of the canvas
     */
    void sizeChanged(const tgt::ivec2& size);

    /**
     * Returns the current selected primitive or 0 if no primitive is selected.
     *
     * @return the current selected primitive or 0 if no primitive is selected
     */
    const TransFuncPrimitive* getSelectedPrimitive();

    /**
     * Called when the rendered volume changes. Deletes the created histogram and the
     * histogram texture.
     *
     * @param newVolume the current rendered volume
     */
    void volumeChanged(const VolumeBase* newVolume);

    /**
     * Enables or disables display of the histogram. It creates the histogram texture if
     * there is none.
     *
     * @param v should the histogram be enabled or disabled?
     */
    void setHistogramVisible(bool v);

    /**
     * Enables or disables display of an underlying grid.
     *
     * @param v should the grid be enabled or disabled?
     */
    void toggleShowGrid(bool v);

    /**
     * Sets the transfer function that is edited with this widget to the given one. The texture
     * of the transfer function is updated.
     *
     * @param tf the transfer function that is edited with this widget
     */
    void setTransFunc(TransFunc2DPrimitives* tf);

    void fitToDomain();

public slots:
    /**
     * Adds a quad primitive to the editor and updates the texture of the tf.
     */
    void addQuadPrimitive();

    /**
     * Adds a banana primitive to the editor and updates the texture of the tf.
     */
    void addBananaPrimitive();

    /**
     * Deletes the current selected primitive.
     */
    void deletePrimitive();

    /**
     * Opens a colordialog if a primitive is selected. The color of the primitive
     * is set to the color choosen in the dialog.
     */
    void colorizePrimitive();

    /**
     * Updates the histogramtexture to new brightness.
     *
     * @param brightness brightness that is used in creation of histogram texture
     */
    void histogramBrightnessChanged(int brightness);

    /**
     * Enables or disables the logarithmic display of the histogram. The histogram texture
     * is updatet when the state changes.
     *
     * @param state should the histogram be logarithmic or not?
     */
    void toggleHistogramLogarithmic(int state);

    /**
     * Sets the fuzziness of the current selected primitive to the given value.
     *
     * @param fuzzi the fuzziness of the current selected primitive is set to this value
     */
    void fuzzinessChanged(int fuzzi);

    /**
     * Sets the transparency of the current selected primitive to the given value.
     *
     * @param trans the transparency of the current selected primitive is set to this value
     */
    void transparencyChanged(int trans);

    /**
     * Resets the transfer function to default, i.e. all primitives are deleted.
     */
    void resetTransferFunction();

signals:
    /**
     * Signal that is emitted when the user changes the transparency of a primitive.
     * It sets the slider that represents the transparency to the new value.
     *
     * @param trans the transparency slider will be set to this value
     */
    void setTransparencySlider(int trans);

    /**
     * Signal that is emitted when primitive is deselected. The transparency and fuzziness slider
     * will be disabled and the selected primitive is set to 0.
     */
    void primitiveDeselected();

    /**
     * Signal that is emitted when a primitive was selected. It sets the selection stat of the
     * primitive to true and enables the transparency and fuzziness slider.
     */
    void primitiveSelected();

    /**
     * Starts or stops the coarseness mode.
     *
     * @param on coarseness mode on or off?
     */
    void toggleInteractionMode(bool on);

    /**
     * Signal that notifies the transfer function property about a change in transfer function
     * and triggers a repaint of the volumerendering.
     */
    void repaintSignal();

private:
    /**
     * Returns the primitive that is under the mouse cursor or 0 if there is none.
     * Therefor all primitives are painted with an id in the color and the pixels
     * are read at the clicked position.
     *
     * @param pos position of the mouse cursor
     * @return primitive that is under the mouse cursor or 0 if there is none
     */
    TransFuncPrimitive* getPrimitiveUnderMouse(tgt::vec2 pos);

    /**
     * Updates all texel in the histogram texture. The value is queried from the histogram that
     * was calculated in createHistogram() and then scaled to the interval [0,255].
     */
    void updateHistogramTexture();

    /**
     * Calculates the histogram out of the current visualized dataset.
     */
    void createHistogram();

    /**
     * Creates the histogram texture with apropriate dimensions and updates the texels.
     */
    void createHistogramTexture();

    TransFunc2DPrimitives* tf_;        ///< the transfer function that can be edited in this widget
    TransFuncPrimitive* selectedPrimitive_; ///< the current selected primitive
    bool showHistogram_;                    ///< is the histogram visible or not?
    bool showGrid_;                         ///< should a grid displayed in the widget?
    bool histogramLogarithmic_;             ///< ist the histogram logarithmic or not?
    float histogramBrightness_;             ///< brightness of the histogram
    const VolumeBase* volume_;            ///< the volume that belongs to the transfer function edited in this widget
    VolumeHistogramIntensityGradient* histogram_; ///< the histogram calculated from the visualized volume
    tgt::Texture* histogramTex_;            ///< texture created out of the histogramdata
    bool dragging_;                         ///< is the user dragging a primitive or a control point of a primitive?
    tgt::vec2 mouseCoord_;                  ///< position of the mouse in the last move event
};

} // namespace voreen

#endif // VRN_TRANSFUNC2DPRIMITIVESPAINTER_H
