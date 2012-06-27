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

#ifndef VRN_RENDERSTORE_H
#define VRN_RENDERSTORE_H

#include "voreen/core/vis/processors/renderprocessor.h"

namespace voreen {

/**
 * A RenderStore stores a copy of its input image and makes it available to other processors via a coprocessor outport.
 * The image is stored in a persistent render target and can therefore be accessed at any time, not necessarily
 * within a rendering pass.
 */
class RenderStore : public RenderProcessor {
public:
    RenderStore();
    ~RenderStore();
    virtual void process(LocalPortMapping* portMapping);

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual int initializeGL();


    /**
     * Returns the ID of the render target where the copy of the input image is stored.
     */
    int getStoredTargetID() const { return privateTargetID_; }

    /**
     * Returns the content of the stored input image at the pixel position \param pos.
     */
    tgt::vec4 getStoredTargetPixel(const tgt::ivec2 &pos) const;

    virtual Message* call(Identifier ident, LocalPortMapping* portMapping);

    /// This processor is defined as EndProcessor.
    virtual bool isEndProcessor() const;

protected:

    /// Texture units
    static const Identifier storeTexUnit_;
    static const Identifier storeDepthTexUnit_;

    /// Render target holding the stored image.
    int privateTargetID_;

    /// The shader program used by this \c RenderStore
    tgt::Shader* shaderPrg_;
};

} // namespace voreen

#endif // VRN_RENDERSTORE_H
