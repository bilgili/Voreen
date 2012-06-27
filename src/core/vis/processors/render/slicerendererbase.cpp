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

#include <math.h>
#include <limits>
#include <algorithm>
#include <sstream>

#include "tgt/tgt_gl.h"
#include "tgt/gpucapabilities.h"

#include "tgt/plane.h"
#include "tgt/glmath.h"

#include "voreen/core/vis/processors/render/slicerendererbase.h"
#include "voreen/core/volume/modality.h"

using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::ivec2;
using tgt::ivec3;

namespace voreen {

const std::string SliceRendererBase::transFuncTexUnit_("transFuncTexUnit");
const std::string SliceRendererBase::volTexUnit_("volTexUnit");

SliceRendererBase::SliceRendererBase()
    : VolumeRenderer(),
    transferFunc_("transferFunction", "Transfer function"),
    transferFuncShader_(0),
    outport_(Port::OUTPORT, "image.outport"),
    inport_(Port::INPORT, "volumehandle.volumehandle")
{
    std::vector<std::string> units;
    units.push_back(transFuncTexUnit_);
    units.push_back(volTexUnit_);
    tm_.registerUnits(units);

    addProperty(transferFunc_);

    addPort(outport_);
    addPort(inport_);
}

SliceRendererBase::~SliceRendererBase() {
    if (transferFuncShader_)
        ShdrMgr.dispose(transferFuncShader_);
}

void SliceRendererBase::initialize() throw (VoreenException) {
    VolumeRenderer::initialize();
}

/*
void SliceRendererBase::init() {
    if ( GpuCaps.areShadersSupported() )
        setupShader();
    else if (GpuCaps.areSharedPalettedTexturesSupported())
        glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    // else -> no support for transfer functions
}

void SliceRendererBase::deinit() {
    if ( GpuCaps.areShadersSupported() )
        tgt::Shader::deactivate();
    else if ( GpuCaps.areSharedPalettedTexturesSupported() )
        glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
    // else -> no support for transfer functions
}
*/

// protected methods
//

bool SliceRendererBase::ready() const {

    if (!inport_.isReady())
        return false;

    // we need at least one texture
    VolumeGL* volumeGL = inport_.getData()->getVolumeGL();
    if (volumeGL->getNumTextures() == 0)
        return false;

    bool ready = false;

    // transferfunction and transferfuncshader are needed, if shaders are supported
    if (GpuCaps.areShadersSupported()) {
        ready = (transferFuncShader_ != 0);
    // transferfunction is needed and texture's format has to be color-index, if we use shared palette textures
    } else {
        if (GpuCaps.areSharedPalettedTexturesSupported()) {
            ready = (volumeGL->getTexture()->getFormat() == GL_COLOR_INDEX);
    // without shaders / palette textures, the texture's format has to be RGBA or luminance-alpha
        } else {
            ready = ((volumeGL->getTexture()->getFormat() == GL_LUMINANCE_ALPHA)
                    || (volumeGL->getTexture()->getFormat() == GL_RGBA));
        }
    }

    if (!ready)
        LWARNING("SliceRendererBase not ready to render!");

    return ready;
}


bool SliceRendererBase::setupShader() {
    // if we don't have hardware support -> return
    if (!GpuCaps.areShadersSupported())
        return false;

    if (!transferFuncShader_)
        transferFuncShader_ = ShdrMgr.load("sr_transfunc", buildShaderHeader(), false);

    // activate the shader if everything went fine and set the needed uniforms
    if (transferFuncShader_) {
        transferFuncShader_->activate();
        updateShaderUniforms();
    }

    LGL_ERROR;
    return true;
}

std::string SliceRendererBase::buildShaderHeader() {
    std::string header = VolumeRenderer::generateHeader();

    header += transferFunc_.get()->getShaderDefines();

    std::ostringstream oss;
    oss << "#define VOL_TEX " << tm_.getTexUnit(volTexUnit_) << std::endl;
    header += oss.str();
    return header;
}

bool SliceRendererBase::rebuildShader() {
    // do nothing if there is no shader at the moment
    if (!transferFuncShader_)
        return false;

    transferFuncShader_->setHeaders(buildShaderHeader(), false);
    return transferFuncShader_->rebuild();
}

void SliceRendererBase::updateShaderUniforms() {
    if (!transferFuncShader_ || !transferFuncShader_->isActivated())
        return;

    transferFuncShader_->setUniform("volumeDataset_", tm_.getTexUnit(volTexUnit_));
    transferFuncShader_->setUniform("transferFunc_", tm_.getTexUnit(transFuncTexUnit_));
}

void SliceRendererBase::deactivateShader() {
    if (transferFuncShader_ && transferFuncShader_->isActivated())
        transferFuncShader_->deactivate();
}

}   // namespace
