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

#include "voreen/core/vis/processors/entryexitpoints/cubeentryexitpoints.h"

#include "tgt/glmath.h"

namespace voreen {


CubeEntryExitPoints::CubeEntryExitPoints()
    : EntryExitPoints()
{
    createInport("volumehandle.volumehandle");
    createCoProcessorInport("coprocessor.proxygeometry");
    createPrivatePort("image.tmp");
    createOutport("image.entrypoints");
    createOutport("image.exitpoints");
}

CubeEntryExitPoints::~CubeEntryExitPoints() {
}

const std::string CubeEntryExitPoints::getProcessorInfo() const {
    return "Calculates cuboid entry/exit points for raycasting.";
}

std::string CubeEntryExitPoints::generateHeader() {
    std::string header = EntryExitPoints::generateHeader();

    header += "// START OF PROGRAM GENERATED DEFINES\n";
    header += "// END OF PROGRAM GENERATED DEFINES\n#line 0\n";

    return header;
}

void CubeEntryExitPoints::process(LocalPortMapping* portMapping) {
    LGL_ERROR;

    EntryExitPoints::process(portMapping);
    if (VolumeHandleValidator::checkVolumeHandle(currentVolumeHandle_,
        portMapping->getVolumeHandle("volumehandle.volumehandle")) == false)
    {
        return;
    }

    int exitSource = portMapping->getTarget("image.exitpoints");
    int entrySource;
    if ((supportCameraInsideVolume_.get() && jitterEntryPoints_.get()) ||
        (!supportCameraInsideVolume_.get() && !jitterEntryPoints_.get()))
    {
        entrySource = portMapping->getTarget("image.entrypoints");
    }
    else 
        entrySource = portMapping->getTarget("image.tmp");

    PortDataCoProcessor* pg = portMapping->getCoProcessorData("coprocessor.proxygeometry");

    // set volume parameters
    std::vector<VolumeStruct> volumes;
    volumes.push_back(VolumeStruct(
        currentVolumeHandle_->getVolumeGL(),
        "",                             // we do not need the volume itself...
        "",
        "volumeParameters_")            // ... but its parameters
    );

    tc_->setActiveTarget(exitSource, "exit");

    // sometimes set to GL_CW. We don't know where and why...
    // as workaround we overwrite it here. (jms)
    glFrontFace(GL_CCW);

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tgt::loadMatrix(camera_->getViewMatrix() * transformationMatrix_);

    shaderProgram_->activate();
    setGlobalShaderParameters(shaderProgram_);
    bindVolumes(shaderProgram_, volumes);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    LGL_ERROR;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    // render back or front texture, depending on transformation matrix
    if (switchFrontAndBackFaces_)
        glCullFace(GL_BACK);
    else
        glCullFace(GL_FRONT);
    pg->call("render");

    shaderProgram_->deactivate();

    // render front texture
    // the second initialization is only necessary when using tc with rtt.
    // should we use conditional compilation for performance reasons here?
    // (jennis)
    //TODO: check if this is really necessary
    tc_->setActiveTarget(entrySource, "entry");
    //glViewport(0, 0, getSize().x, getSize().y);

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    tgt::loadMatrix(camera_->getProjectionMatrix());
    glMatrixMode(GL_MODELVIEW);
    tgt::loadMatrix(camera_->getViewMatrix() * transformationMatrix_);

    shaderProgram_->activate();
    setGlobalShaderParameters(shaderProgram_);
    bindVolumes(shaderProgram_, volumes);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    LGL_ERROR;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LGL_ERROR;

    if (switchFrontAndBackFaces_)
        glCullFace(GL_FRONT);
    else
        glCullFace(GL_BACK);

    pg->call("render");
    shaderProgram_->deactivate();

    // fill holes in entry points texture caused by near plane clipping
    if (supportCameraInsideVolume_.get())
        complementClippedEntryPoints(portMapping);

    // jittering of entry points
    if (jitterEntryPoints_.get())
        jitterEntryPoints(portMapping);

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));

    // restore modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    LGL_ERROR;
}

} // namespace voreen
