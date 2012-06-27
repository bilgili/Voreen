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

#ifndef VRN_CANVASRENDERER_H
#define VRN_CANVASRENDERER_H

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

/**
 * A CanvasRenderer is the last processor in a network. Its only purpose is to copy
 * its input to the finaltarget of texture container. Additionally the CanvasRenderer
 * is able to cache the rendering result, if no parameter in any processor in the network
 * has been changed since last rendering.
 */
class CanvasRenderer : public Processor {
public:
    CanvasRenderer();
    ~CanvasRenderer();

    virtual void process(LocalPortMapping* portMapping);

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual int initializeGL();

    virtual bool isEndProcessor() const;
    virtual bool usesCaching() const;

    /**
     * Returns the ID of the image which will be copied to the frame buffer in order
     * to enable others to access the render target from the TextureContainer where
     * the final image is held.
     */
    int getImageID() { return imageID_; }
    
protected:
    /**
     * The shader program used by this \c CanvasRenderer.
     */
    tgt::Shader* raycastPrg_;

    BoolProp useCaching_; ///< property that activates or deactivates caching in this processor

    /// Render target holding the input image within the TextureContainer.
    int imageID_;    
};

} // namespace voreen

#endif // VRN_CANVASRENDERER_H
