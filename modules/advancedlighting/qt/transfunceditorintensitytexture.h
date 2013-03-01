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

#ifndef VRN_TRANSFUNCEDITORINTENSITYTEXTURE_H
#define VRN_TRANSFUNCEDITORINTENSITYTEXTURE_H

#include "voreen/qt/widgets/transfunc/transfunceditorintensity.h"
#include "tgt/texture.h"

namespace voreen {

class TransFuncEditorIntensityTexture : public TransFuncEditorIntensity {
    Q_OBJECT

public:
    /**
     * Standard constructor.
     *
     * @param prop the transfer function property
     * @param parent the parent frame to which this is added
     * @param orientation orientation of the widget, e.g. position of the color and luminancepicker
     */
    TransFuncEditorIntensityTexture(TransFuncProperty* prop, QWidget* parent = 0, Qt::Orientation orientation = Qt::Horizontal);

    /**
     * Creates the connections for all control elements.
     */
    void createConnections();

    Property* getProperty() {
        return property_;
    }

public slots:

    //void updateTransferFunction
    void causeVolumeRenderingRepaint();

protected:

    virtual QLayout* createButtonLayout();

    /**
     * Creates the layout with mapping canvas and threshold control elements.
     *
     * @return the layout with mapping canvas and threshold control elements
     */
    virtual QLayout* createMappingLayout();
};

} // namespace voreen

#endif // VRN_TRANSFUNCEDITORINTENSITY_H
