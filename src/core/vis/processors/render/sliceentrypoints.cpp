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

#include "voreen/core/vis/processors/render/sliceentrypoints.h"

#include "voreen/core/vis/properties/cameraproperty.h"

namespace voreen {

using tgt::vec3;
using tgt::mat4;


SliceEntryPoints::SliceEntryPoints()
    : EntryExitPoints()
{

}


SliceEntryPoints::~SliceEntryPoints() {
}

const std::string SliceEntryPoints::getProcessorInfo() const {
    return "Calculates entry points for multiple slices arranged in 3D.";
}

std::string SliceEntryPoints::generateHeader() {
    std::string header = EntryExitPoints::generateHeader();
    return header;
}

void SliceEntryPoints::process() {

    if (!inport_.isReady())
        return;

    LGL_ERROR;

    EntryExitPoints::process();

    if (!entryPort_.isReady())
        return;

    // set volume parameters
    std::vector<VolumeStruct> volumes;
    volumes.push_back(VolumeStruct(
        inport_.getData()->getVolumeGL(),
        "",                             // we do not need the volume itself...
        "",
        "volumeParameters_")            // ... but its parameters
    );

    entryPort_.activateTarget();

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_.get()->getViewMatrix());

    shaderProgram_->activate();
    setGlobalShaderParameters(shaderProgram_, camera_.get());
    bindVolumes(shaderProgram_, volumes);
    glEnable(GL_DEPTH_TEST);
    LGL_ERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    if (cpPort_.getConnectedProcessor())
        cpPort_.getConnectedProcessor()->render();

    shaderProgram_->deactivate();

    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));

    // restore modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    LGL_ERROR;
}

} // namespace voreen
