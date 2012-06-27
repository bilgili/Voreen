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

#ifndef VRN_TRANSFUNCTEXTUREPAINTER_H
#define VRN_TRANSFUNCTEXTUREPAINTER_H

#include "tgt/painter.h"
#include "tgt/vector.h"

namespace voreen {

class TransFuncIntensity;

/**
 * A class for painting the texture of a transfer function onto a given canvas.
 */
class TransFuncTexturePainter : public tgt::Painter {
public:
    /**
     * Constructor
     *
     * @param canvas canvas that belongs to this painter
     */
    TransFuncTexturePainter(tgt::GLCanvas* canvas);

    /**
     * Destructor
     */
    ~TransFuncTexturePainter();

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
    void setTransFunc(TransFuncIntensity* tf);

protected:
    TransFuncIntensity* tf_; ///< the displayed transfer function
};

} // namespace voreen

#endif // VRN_TRANSFUNCTEXTUREPAINTER_H
