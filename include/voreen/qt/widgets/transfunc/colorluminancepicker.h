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

#ifndef VRN_COLORLUMINANCEPICKER_H
#define VRN_COLORLUMINANCEPICKER_H

#include <QWidget>

class QColor;
class QMouseEvent;
class QPaintEvent;

namespace voreen {

/**
 * A widget that allows the user to selected a luminance for a given color.
 * A colorbar is diplayed that shows a colorgradient with all possible luminance values.
 * Furthermore there is an arrow that shows the current luminance. The user can adjust the
 * luminance by dragging the arrow or clicking on the colorbar.
 */
class ColorLuminancePicker : public QWidget {
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param parent the parent widget
     */
    ColorLuminancePicker(QWidget* parent=0);

    /**
     * Destructor
     */
    ~ColorLuminancePicker();

public slots:
    /**
     * Slot that is called when the selected color in the colorpicker changes.
     *
     * @param h new hue value
     * @param s new saturation value
     */
    void setCol(int h, int s);

    /**
     * Slot that is called when the selected color in the colorpicker changes.
     *
     * @param c selected color
     */
    void setCol(const QColor& c);

signals:
    /**
     * This signal is emitted when the luminance was changed by the user.
     */
    void newHsv(int h, int s, int v);

    /**
     * This signal is emitted when the user changes the luminance. It will switch
     * the coarseness mode of volume rendering on and off.
     *
     * @param on should coarseness mode switched on or off?
     */
    void toggleInteractionMode(bool b);

protected:
    /**
     * Paints the luminancepicker.
     *
     * @param event the paint event
     */
    void paintEvent(QPaintEvent* event);

    /**
     * Sets the arrow that indicates the luminance to the y-position of the mouse.
     *
     * @param event the mouse event
     */
    void mouseMoveEvent(QMouseEvent* event);

    /**
     * Sets the arrow that indicates the luminance to the y-position of the mouse.
     *
     * @param event the mouse event
     */
    void mousePressEvent(QMouseEvent* event);

    /**
     * Only emits the toggleInteractionMode signal, so that coarseness mode is siwtched off.
     *
     * @param event the mouse event
     */
    void mouseReleaseEvent(QMouseEvent* event);

private:

    /**
     * This method sets the displayed color to the given values. The arrow that
     * indicates the luminance is set to a position that represents the value v.
     *
     * @param h hue value
     * @param s saturation value
     * @param v value in hsv-colorspace
     */
    void setCol(int h, int s, int v);

    /**
     * Converts the given height (y-value of mouse position) to value
     * of hsv-colorspace.
     *
     * @param y y-position of mouse
     * @return value of hsv-colorspace
     */
    int y2val(int y);

    /**
     * Converts the value of hsv-colorspace to y-position of the mouse.
     *
     * @param val value of hsv-colorspace
     * @return y-position of the mouse
     */
    int val2y(int val);

    /**
     * Sets the value of hsv-colorspace to the given value.
     *
     * @param v new value in hsv-colorspace
     */
    void setVal(int v);

    //frame and contents offset
    enum {
        foff = 0,
        coff = 1
    };

    int hue_; ///< hue that is represented by the luminancepicker.
    int sat_; ///< saturation that is represented by the luminancepicker.
    int val_; ///< value in hsv-colorspace that is represented by the luminancepicker.

    QPixmap* pix_; ///< pixmap that displays a bar with current choosen color and all possible luminance values.

};

 } // namespace voreen

#endif // VRN_COLORLUMINANCEPICKER_H
