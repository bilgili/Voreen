/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_POSTPROCESSOR_H
#define VRN_POSTPROCESSOR_H

#include <vector>
#include <string>

#include "tgt/shadermanager.h"
#include "tgt/quadric.h"
#include "tgt/types.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/processor.h"

namespace voreen {

/**
 * Manage the individual post processing filters.
 *
 * Only a skeleton at the moment.
 */
class PostProcessor : public Processor {
public:

    /**
     * The Constructor.
     *
     * @param camera The camera from wich we will get information about the current modelview-matrix.
     * @param tc The TextureContainer that will be used to manage TextureUnits for all render-to-texture work done by the PostProcessing.
     */
	PostProcessor();
    virtual const Identifier getClassName() const {return "PostProcessor.PostProcessor";}
    virtual Processor* create() {return new PostProcessor();} // Sollte das pure virtual / abstract sein?
    virtual ~PostProcessor();

    /**
     * Initializes all necessary openGL options.
     *
     * @return The return state tells us if everything went ok.
     */
    virtual int initializeGL();

    /**
     * Performs the actual postprocessing.
     */
    virtual void process(LocalPortMapping* portMapping);

protected:

    virtual void analyzeDepthBuffer(int source); ///< Read back depth buffer and determine min and max depth value.

    FloatProp minDepth_; ///< Control the minimum depth value.
    FloatProp maxDepth_; ///< Control the maximum depth value.
};

} // namespace voreen

#endif //VRN_POSTPROCESSING_H

