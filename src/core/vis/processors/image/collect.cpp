
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

#include "voreen/core/vis/processors/image/collect.h"
#include "voreen/core/vis/processors/portmapping.h"

namespace voreen {

Collect::Collect(std::string shaderFilename)
    : GenericFragment(shaderFilename)
{
	createInport("image.inputs",true);
	createOutport("image.output");

	createPrivatePort("image.temp1");
	createPrivatePort("image.temp2");
}

const std::string Collect::getProcessorInfo() const {
	return "No information available.";
}

void Collect::process(LocalPortMapping*  portMapping) {
    compileShader();
    glDepthFunc(GL_ALWAYS);
    processIterative(portMapping);
    glDepthFunc(GL_LESS);
}

void Collect::processIterative(LocalPortMapping* portMapping) {
	std::vector<int> sources = portMapping->getAllTargets("image.inputs");
	if (sources.size() <= 1)
        return;
	
	int tempTarget1 = portMapping->getTarget("image.temp1");
	int tempTarget2 = portMapping->getTarget("image.temp2");
	int dest = portMapping->getTarget("image.output");
	int currentTarget = tempTarget1;
	int source0, source1;
	source0 = sources.at(0);
	for (size_t i=1; i<sources.size(); i++) {
		source1 = sources.at(i);
		if ( (i+1) == sources.size())
			tc_->setActiveTarget(dest);
		else
			tc_->setActiveTarget(currentTarget);
		
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		renderTwo(source0, source1, i);
		source0 = currentTarget;
		if (currentTarget==tempTarget1)
			currentTarget=tempTarget2;
		else if (currentTarget == tempTarget2)
			currentTarget=tempTarget1;
	}
}

} // voreen namespace
