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

//TODO: load the right headers
#include "voreen/core/vis/processors/image/merge.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "tgt/assert.h"
#include "tgt/glmath.h"
#include "tgt/vector.h"

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/messagedistributor.h"

namespace voreen {

//TODO: check if all of these are needed
using tgt::vec3;
using tgt::ivec2;
using tgt::Color;
using std::vector;

//---------------------------------------------------------------------------

const Identifier Merge::shadeTexUnit1_ = "shadeTexUnit1";
const Identifier Merge::depthTexUnit1_ = "depthTexUnit1";

Merge::Merge()
    : Collect("pp_merge")
{
    tm_.addTexUnit(shadeTexUnit1_);
    tm_.addTexUnit(depthTexUnit1_);
}

const std::string Merge::getProcessorInfo() const {
	return "Merges two sources";
}

void Merge::renderTwo(int source0, int source1, int /*pass*/) {
    // bind shading and depth result from first ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit_));
    glBindTexture(tc_->getGLTexTarget(source0), tc_->getGLTexID(source0));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit_));
    glBindTexture(tc_->getGLDepthTexTarget(source0), tc_->getGLDepthTexID(source0));
    LGL_ERROR;

    // bind shading and depth result from second ray cast
    glActiveTexture(tm_.getGLTexUnit(shadeTexUnit1_));
    glBindTexture(tc_->getGLTexTarget(source1), tc_->getGLTexID(source1));
    glActiveTexture(tm_.getGLTexUnit(depthTexUnit1_));
    glBindTexture(tc_->getGLDepthTexTarget(source1), tc_->getGLDepthTexID(source1));
    LGL_ERROR;

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("shadeTex0_", (GLint) tm_.getTexUnit(shadeTexUnit_));
    program_->setUniform("depthTex0_", (GLint) tm_.getTexUnit(depthTexUnit_));
    program_->setUniform("shadeTex1_", (GLint) tm_.getTexUnit(shadeTexUnit1_));
    program_->setUniform("depthTex1_", (GLint) tm_.getTexUnit(depthTexUnit1_));
    program_->setUniform("backgroundColor_", backgroundColor_.get());

    renderQuad();

    program_->deactivate();
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    LGL_ERROR;
}


} // voreen namespace

