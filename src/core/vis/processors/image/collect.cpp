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

#include "voreen/core/vis/processors/image/collect.h"

namespace voreen {

Collect::Collect(std::string shaderFilename)
    : ImageProcessor(shaderFilename)
    , inport_(Port::INPORT, "image.inputs", true)
    , outport_(Port::OUTPORT, "image.output")
    , tmp1Port_(Port::OUTPORT, "image.temp1", true)
    , tmp2Port_(Port::OUTPORT, "image.temp2", true)
{
    addPort(inport_);
    addPort(outport_);
    addPort(tmp1Port_);
    addPort(tmp2Port_);
}

const std::string Collect::getProcessorInfo() const {
    // TODO write me (ab)
    return "No information available.";
}

void Collect::process() {
    if (!outport_.isReady())
        return;
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    glDepthFunc(GL_ALWAYS);
    processIterative();
    glDepthFunc(GL_LESS);
}

void Collect::processIterative() {

    // processor will be removed soon

    /*std::vector<RenderTarget*> sources = inport_.getAllData();
    if (sources.size() <= 1 || !outport_.getData())
        return;

    int tempTarget1 = getTarget("image.temp1");
    int tempTarget2 = getTarget("image.temp2");
    int dest = outport_.getData()->textureID_;
    int currentTarget = tempTarget1;
    int source0, source1;
    source0 = sources.at(0)->textureID_;
    for (size_t i=1; i<sources.size(); i++) {
        if (!sources.at(i))
            return;

        source1 = sources.at(i)->textureID_;
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
    } */
}

} // voreen namespace
