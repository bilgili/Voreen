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

#include "voreen/core/vis/processors/image/genericfragment.h"

namespace voreen {

const Identifier GenericFragment::shadeTexUnit_ = "shadeTexUnit";
const Identifier GenericFragment::depthTexUnit_ = "depthTexUnit";

GenericFragment::GenericFragment(const std::string& shaderFilename)
    : PostProcessor(), program_(0), shaderFilename_(shaderFilename), needRecompileShader_(true)
{
    std::vector<Identifier> units;
    units.push_back(shadeTexUnit_);
    units.push_back(depthTexUnit_);
    tm_.registerUnits(units);
}

GenericFragment::~GenericFragment() {
    if (program_)
        ShdrMgr.dispose(program_);
}

int GenericFragment::initializeGL() {
    if (!shaderFilename_.empty()) {
        program_ = ShdrMgr.loadSeparate("pp_identity.vert", shaderFilename_ + ".frag", generateHeader(), false);
        if (program_) {
            needRecompileShader_ = true;
            compileShader();
        }
        initStatus_ = program_ ? VRN_OK : VRN_ERROR;
    }
    else {
        program_ = 0;
        initStatus_ = VRN_OK;
        return VRN_OK;
    }

    return initStatus_;
}

void GenericFragment::compileShader() {
    if (needRecompileShader_) {
        compile();
        needRecompileShader_ = false;
    }
}

void GenericFragment::compile() {
    if (program_)
        program_->rebuild();
}

} // voreen namespace
