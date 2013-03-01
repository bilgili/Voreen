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

#ifndef VRN_TRANSFUNC1DKEYSPAINTER_H
#define VRN_TRANSFUNC1DKEYSPAINTER_H

#include "tgt/painter.h"
#include "tgt/vector.h"

namespace voreen {

class TransFunc1DKeys;

/**
 * A class for painting the texture of a transfer function onto a given canvas.
 */
class TransFunc1DKeysPainter : public tgt::Painter {
public:
    /**
     * Constructor
     *
     * @param canvas canvas that belongs to this painter
     */
    TransFunc1DKeysPainter(tgt::GLCanvas* canvas);

    /**
     * Destructor
     */
    ~TransFunc1DKeysPainter();

    /**
     * Paints the texture of the transfer function. A checkerboard is displayed
     * in the background.
     */
    void paint();

    /**
     * Initializes the painter, e.g. the projection and modelview matrix are set.
     */
    void initialize();

    /**
     * This method is called whenever the size of the widget changes.
     * It sets the viewport to the new size and updates the projection matrix.
     *
     * @param size new size of the widget
     */
    void sizeChanged(const tgt::ivec2& size);

    /**
     * Sets the displayed transfer function to the given one.
     *
     * @param tf the transfer function which texture is painted by this widget
     */
    void setTransFunc(TransFunc1DKeys* tf);

protected:
    TransFunc1DKeys* tf_; ///< the displayed transfer function
};

} // namespace voreen

#endif // VRN_TRANSFUNC1DKEYSTEXTUREPAINTER_H
