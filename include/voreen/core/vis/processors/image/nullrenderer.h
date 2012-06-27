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

#ifndef VRN_NULLRENDERER_H
#define VRN_NULLRENDERER_H

#include "voreen/core/vis/processors/renderprocessor.h"

namespace voreen {

/**
 * A NullRenderer is the last processor in a network. Its only purpose is to terminate a
 * network and to keep the id of the rendered image from the texture container.
 * This processor does neither create any output nor does it render to the frame buffer!
 *
 * @author  Dirk Feldmann, October 2008
 */
class NullRenderer : public RenderProcessor {
public:
    NullRenderer();
    ~NullRenderer();
    virtual void process(LocalPortMapping* portMapping);

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    /**
     * Returns the ID of the input image in order to enable others to access the render target
     * from the TextureContainer.
     */
    int getImageID() { return imageID_; }

    virtual bool isEndProcessor() const;

protected:
    /// Render target holding the input image within the TextureContainer.
    int imageID_;
};

} // namespace voreen

#endif // VRN_NULLRENDERER_H
