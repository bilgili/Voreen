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

#ifndef VRN_TRANSFUNCMAPPINGCANVASRAMP_H
#define VRN_TRANSFUNCMAPPINGCANVASRAMP_H

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"

class QMouseEvent;

namespace voreen {

/**
 * A widget that provides a canvas to edit an intensity transfer function.
 * In contrast to TransFuncMappingCanvas the transfer function is restricted to a ramp.
 * That means that you can not insert new keys, you can not split keys and you can not
 * move the keys arbitrary.
 */
class TransFuncMappingCanvasRamp : public TransFuncMappingCanvas {
    Q_OBJECT
public:
    /**
     * Constructor - inits member variables to default
     *
     * @param parent the parent widget
     * @param tf the transfer function that is displayed in this widget
     * @param noColor when true the color of a key can not be changed
     * @param xAxisText caption of the x axis
     * @param yAxisText caption of the y axis
     */
    TransFuncMappingCanvasRamp(QWidget* parent, TransFunc1DKeys* tf, bool noColor = false,
                           QString xAxisText = tr("intensity"),
                           QString yAxisText = tr("opacity"));

    /**
     * Destructor
     */
    ~TransFuncMappingCanvasRamp();

    /**
     * This method turns coarseness mode on, opens the context menu when right
     * mousebutton was pressed and tests whether the mouse cursor is at a key.
     *
     * @param event the mouse event
     */
    void mousePressEvent(QMouseEvent* event);

    /**
     * This method turns coarseness mode off and hides the tooltip.
     *
     * @param event the mouse event
     */
    void mouseReleaseEvent(QMouseEvent* event);

    /**
     * Repositions the selected key. Nothing happens when no key is selected.
     *
     * @param event the mouse event
     */
    void mouseMoveEvent(QMouseEvent* event);

    /**
     * This method diplays the context menu at the given mouse position
     * for the case of a keyselection.
     *
     * @param event the mouse event
     */
    void showKeyContextMenu(QMouseEvent* event);

    /**
     * Stores the current ramp parameters in given variables.
     *
     * @param rampCenter the current ramp center will be stored in this variable
     * @param rampWidth the current ramp width will be stored in this variable
     */
    void getRampParameter(float &rampCenter, float &rampWidth);

    /**
     * Sets the paramters of the ramp to the given values. The keys of the transfer function
     * are calculated according to new parameters.
     *
     * @param center center of the ramp
     * @param width width of the ramp
     */
    void setRampParameter(float center, float width);

    /**
     * Calculates the keys of the transfer function from ramp center and ramp width.
     */
    void calcKeysFromRampParameter();

    /**
     * Calculates the ramp parameters, e.g. center and width, from the keys of the transfer function.
     */
    void calcRampParameterFromKeys();

private:
    float rampCenter_; ///< center of the ramp
    float rampWidth_;  ///< width of the ramp
};

} // namespace voreen

#endif // VRN_TRANSFUNCMAPPINGCANVASRAMP_H
