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

#ifndef VRN_COARSENESSRENDERER_H
#define VRN_COARSENESSRENDERER_H

#include "tgt/glmath.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
namespace voreen {

class CoarsenessRenderer : public CopyToScreenRenderer
{
public:

	/**
	 * Default constructor.
	 * @param camera the \c Camera object used in this pipeline.
	 * @param tc the \c TextureContainer object to use. \see TextureContainer
	 */
	CoarsenessRenderer(tgt::Camera* camera=0, TextureContainer* tc = 0);
    ~CoarsenessRenderer();

	virtual void process(LocalPortMapping* portMapping);
	virtual const Identifier getClassName() const {return "Miscellaneous.CoarsenessRenderer";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new CoarsenessRenderer();}
 

protected:
    
};

} // namespace voreen

#endif //VRN_COARSENESSRENDERER_H
