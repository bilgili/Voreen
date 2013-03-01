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

#ifndef VRN_TRANSFUNCMAPPINGCANVASTEXTURE_H
#define VRN_TRANSFUNCMAPPINGCANVASTEXTURE_H

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"

namespace voreen {

class TransFuncProperty;

/**
 * A widget that provides a canvas to edit the keys of an intensity transfer function.
 * The user can insert new keys by clicking at the desired location and reposition keys with holding
 * down left mouse button. Furthermore keys can be splitted, merged and deleted. The color of a key
 * can also be changed.
 */
class TransFuncMappingCanvasTexture : public TransFuncMappingCanvas {
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param parent the parent widget
     * @param tf the transfer function that is displayed in this widget
     * @param noColor when true the color of a key can not be changed
     * @param clipThresholds should the visible range be restricted to thresholds?
     * @param xAxisText caption of the x axis
     * @param yAxisText caption of the y axis
     */
    TransFuncMappingCanvasTexture(QWidget* parent, TransFuncIntensity* tf, TransFuncProperty* prop,
                           bool noColor = false, QString xAxisText = tr("intensity"), QString yAxisText = tr("opacity"));

    /**
     * Destructor
     */
    virtual ~TransFuncMappingCanvasTexture() {};

    virtual void mousePressEvent(QMouseEvent* event);
    //virtual void mouseMoveEvent(QMouseEvent* event);

public slots:

    /**
     * Opens a filedialog for choosing the texture of the current selected key.
     */
    void changeCurrentTexture();
    void disableCurrentTexture();

protected:

    virtual void showKeyContextMenu(QMouseEvent* event);

private:

    QAction* disableAction_;
    QAction* assignAction_;
    TransFuncProperty* property_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCMAPPINGCANVASTEXTURE_H
