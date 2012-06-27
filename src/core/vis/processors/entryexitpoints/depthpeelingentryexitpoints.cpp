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

#include "voreen/core/vis/processors/entryexitpoints/depthpeelingentryexitpoints.h"
#include "tgt/glmath.h"
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>

#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/virtualclipping.h"
#endif
#include "voreen/core/vis/voreenpainter.h"

using tgt::vec3;
using tgt::mat4;

namespace voreen {

const std::string DepthPeelingEntryExitPoints::entryPointsTexUnit_      = "entryPointsTexUnit";
const std::string DepthPeelingEntryExitPoints::entryPointsDepthTexUnit_ = "entryPointsDepthTexUnit";
const std::string DepthPeelingEntryExitPoints::exitPointsTexUnit_       = "exitPointsTexUnit";

const std::string DepthPeelingEntryExitPoints::loggerCat_("voreen.DepthPeelingEntryExitPoints");

/*
    constructor
*/
DepthPeelingEntryExitPoints::DepthPeelingEntryExitPoints()
    : VolumeRenderer()
    , shaderProgram_(0)
    , camera_("camera", "Camera", new tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , biasProp_("biasProp", "Bias",0,0,0.5,true)
    , pgPort_(Port::INPORT, "coprocessor.proxygeometry")
    , outPort_(Port::OUTPORT, "coprocessor.depthpeeling")
    , outport1_(Port::OUTPORT, "image.layer1")
    , outport2_(Port::OUTPORT, "image.layer2")
    , outport3_(Port::OUTPORT, "image.layer3")
    , outport4_(Port::OUTPORT, "image.layer4")
    , outport5_(Port::OUTPORT, "image.layer5")
    , outport6_(Port::OUTPORT, "image.layer6")
    , initialImagePort_(Port::OUTPORT, "image.initial")
{

    // Register textures in order to use them in the process method
    std::vector<std::string> units;
    units.push_back(entryPointsTexUnit_);
    units.push_back(entryPointsDepthTexUnit_);
    units.push_back(exitPointsTexUnit_);
    tm_.registerUnits(units);

    addPrivateRenderPort(&initialImagePort_);

    // Co-Processor for rendering proxy geomtry calls
    addPort(outPort_);
    addPort(pgPort_);

    // For the maximum of three volumes, there can be up to 6 different Outputs
    addPort(outport1_);
    addPort(outport2_);
    addPort(outport3_);
    addPort(outport4_);
    addPort(outport5_);
    addPort(outport6_);

    layersNumber=0;
    addProperty(camera_);
    addProperty(biasProp_);
}

DepthPeelingEntryExitPoints::~DepthPeelingEntryExitPoints() {
    if (shaderProgram_)
        ShdrMgr.dispose(shaderProgram_);
}

void DepthPeelingEntryExitPoints::initialize() throw (VoreenException) {

    VolumeRenderer::initialize();

    shaderProgram_ = ShdrMgr.load("eep_depth", generateHeader(), false);

    if(!shaderProgram_) {
        LERROR("Failed to load shader!");
        initialized_ = false;
        throw VoreenException(getClassName() + ": Failed to load shader!");
    }

    initialized_ = true;
}

int DepthPeelingEntryExitPoints::getNumLayers() {
    return layersNumber;
}

std::vector<int> DepthPeelingEntryExitPoints::getMap() {
    return volumeMap_;
}

std::string DepthPeelingEntryExitPoints::itos(int i) {     // convert int to string
        std::stringstream s;
        s << i;
        return s.str();
}

int DepthPeelingEntryExitPoints::lookup(int objectid) {
    for (size_t c = 0; c < volumeMap_.size(); ++c) {
        if (volumeMap_.at(c) == objectid)
            return c;
    }

    return -1;
}

void DepthPeelingEntryExitPoints::createMap(std::vector<ProxyGeometry*> portData) {
    int objectID;
    ProxyGeometry* pdcp;
    for (size_t i = 0; i < portData.size(); ++i) {
        pdcp = portData.at(i);
        //objectID = pdcp->getObjectID();
        volumeMap_.push_back(objectID);
    }
}

void DepthPeelingEntryExitPoints::process() {

    // Todo: port merge

    /*std::vector<ProxyGeometry*> portData = pgPort_.getConnectedProcessors();
    int objectID1;
    ProxyGeometry* pdcp1;
    volumeMap_.clear();

    ///This is for mapping the object ID of each volume to a unique ID
    for (size_t i=0; i<portData.size() && i<3; i++) {
        pdcp1 = portData.at(i);
        objectID1=pdcp1->getObjectID();
        volumeMap_.push_back(objectID1);
    }

    /// saving the output targets in a vector (this could be enhanced by a multioutput port)
    std::vector<int> targets;
    targets.push_back(getTarget("image.layer1") );
    targets.push_back(getTarget("image.layer2") );
    targets.push_back(getTarget("image.layer3") );
    targets.push_back(getTarget("image.layer4") );
    targets.push_back(getTarget("image.layer5") );
    targets.push_back(getTarget("image.layer6") );
    targets.push_back(getTarget("image.initial") );

    /// Initializing some opengl features
    glClearDepth(0.0);
    tc_->setActiveTarget(getTarget("image.initial"), "initial");
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    LGL_ERROR;

    GLint samples=1;/// variable for storing the samples generated by the occlusion test
    int id;/// the variable that is passed to the shader to be included in the alpha value
    int objectID;
    ProxyGeometry* pdcp;
    int count=0;
    std::vector<int> tempMap;
    bool firstLayer_ = true;
    // Bias value modifying
    float bias;
    bias = biasProp_.get() / 0.05f;
    bias = pow(10, bias);

    // Computing the entry/exit textures on several passes
    for (count = 0; samples != 0; ++count) {
        glBeginQuery(GL_SAMPLES_PASSED, (count+1)); // Start the occlusion query
        LGL_ERROR;

        tc_->setActiveTarget(targets.at(count) , "layer"+itos(count));
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        LGL_ERROR;

        shaderProgram_->activate();

        // For the first pass, the previous texture is initialized with zeros
        if (count == 0) {
            glActiveTexture(tm_.getGLTexUnit(entryPointsDepthTexUnit_));
            LGL_ERROR;
            glBindTexture(tc_->getGLDepthTexTarget(getTarget("image.initial")), tc_->getGLDepthTexID(getTarget("image.initial")));
            LGL_ERROR;

            glActiveTexture(tm_.getGLTexUnit(exitPointsTexUnit_));
            LGL_ERROR;
            glBindTexture(tc_->getGLTexTarget(getTarget("image.initial")), tc_->getGLTexID(getTarget("image.initial")));
            LGL_ERROR;

            shaderProgram_->setUniform("previous_", tm_.getTexUnit(entryPointsDepthTexUnit_));
            shaderProgram_->setUniform("previous1_", tm_.getTexUnit(exitPointsTexUnit_));
        }
        else {
            // bind previous params which correspond to the last generated layer
            glActiveTexture(tm_.getGLTexUnit(exitPointsTexUnit_));
            glBindTexture(tc_->getGLTexTarget(targets.at(count-1)), tc_->getGLTexID(targets.at(count-1)));
            LGL_ERROR;

            glActiveTexture(tm_.getGLTexUnit(entryPointsDepthTexUnit_));
            glBindTexture(tc_->getGLDepthTexTarget(targets.at(count-1)), tc_->getGLDepthTexID(targets.at(count-1)));
            LGL_ERROR;

            shaderProgram_->setUniform("previous_", tm_.getTexUnit(entryPointsDepthTexUnit_));
            shaderProgram_->setUniform("previous1_", tm_.getTexUnit(exitPointsTexUnit_));
        }


        shaderProgram_->setUniform("bias_",bias);
        // set modelview and projection matrices
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        tgt::loadMatrix(camera_.get()->getProjectionMatrix());
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        tgt::loadMatrix(camera_.get()->getViewMatrix());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < portData.size(); ++i) { /// render all PGs while performing Depth Peeling
            pdcp = portData.at(i);
            setGlobalShaderParameters(shaderProgram_, camera_.get());
            objectID=pdcp->getObjectID();
            id=lookup(objectID);
            shaderProgram_->setUniform("id_",float(id));
            shaderProgram_->setUniform("firstLayer_",firstLayer_);
            glCullFace(GL_FRONT);
            pdcp->render();
            glCullFace(GL_BACK);
            pdcp->render();
        }

        shaderProgram_->deactivate();
        glFlush();
        glEndQuery(GL_SAMPLES_PASSED); // end the occlusion query
        glGetQueryObjectiv((count+1),GL_QUERY_RESULT,&samples);// get the result of the occlusion query
        firstLayer_=false;

        // restore modelview and projection matrices
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
      }

    LGL_ERROR;
    layersNumber=count-1;
    glActiveTexture(tm_.getGLTexUnitFromInt(0)); */
}

} // namespace voreen
