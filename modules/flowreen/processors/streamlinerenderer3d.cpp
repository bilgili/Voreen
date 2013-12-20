/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "modules/flowreen/utils/flowmath.h"
#include "streamlinerenderer3d.h"
#include "modules/flowreen/datastructures/volumeflow3d.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/gpucapabilities.h"

namespace voreen {

StreamlineRenderer3D::StreamlineRenderer3D()
    : RenderProcessor(),
    numStreamlinesProp_("numberStreamlines", "number of streamlines: ", 10, 1, 100000),
    styleProp_(0),
    geometrySpacingProp_("geometrySpacing", "spacing for geometry: ", 4, 1),
    geometrySizeProp_("geometrySizeProp", "geometry size: ", 1, 1),
    useAlphaBlendingProp_("useAlphaBlendingProp", "fade off distant objects: ", true),
    camProp_("camera", "Camera", tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.5f),
        tgt::vec3(0.0f, 0.0f, 0.0f), tgt::vec3(0.0f, 1.0f, 0.0f))),
    cameraHandler_(0),
    currentStyle_(STYLE_LINES),
    numStreamlines_(10),
    rebuildDisplayLists_(true),
    reinitSeedingPositions_(true),
    displayLists_(0),
    shader_(0),
    seedingPositions_(0),
    volInport_(Port::INPORT, "volumehandle.flow", "Flow Volume Input"),
    imgOutport_(Port::OUTPORT, "image.streamlines", "Streamlines Image Output", true, Processor::INVALID_RESULT, RenderPort::RENDERSIZE_RECEIVER)
{
    CallMemberAction<StreamlineRenderer3D> streamlineChange(this,
        &StreamlineRenderer3D::onStreamlineNumberChange);
    CallMemberAction<StreamlineRenderer3D> defAction(this,
        &StreamlineRenderer3D::invalidateRendering);

    IntOptionProperty& colorTableProp = const_cast<IntOptionProperty&>(colorCoding_.getColorTableProp());

    numStreamlinesProp_.onChange(streamlineChange);
    maxStreamlineLengthProp_.onChange(defAction);
    thresholdProp_.onChange(streamlineChange);
    colorTableProp.onChange(
        CallMemberAction<StreamlineRenderer3D>(this, &StreamlineRenderer3D::onColorCodingChange));

    styleProp_ = new OptionProperty<StreamlineStyle>("streamlineStyle", "streamline style: ");
    styleProp_->addOption("streamlines (lines)", "streamlines (lines)", STYLE_LINES);
    styleProp_->addOption("streamlines (tubes)", "streamlines (tubes)", STYLE_TUBES);
    styleProp_->addOption("streamlines (arrows)", "streamlines (arrows)", STYLE_ARROWS);
    styleProp_->addOption("arrow plot (equidistant)", "arrow plot (equidistant)", STYLE_ARROW_GRID);
    styleProp_->onChange(
        CallMemberAction<StreamlineRenderer3D>(this, &StreamlineRenderer3D::onStyleChange));

    geometrySpacingProp_.onChange(defAction);
    geometrySizeProp_.onChange(defAction);

    addProperty(numStreamlinesProp_);
    addProperty(maxStreamlineLengthProp_);
    addProperty(thresholdProp_);
    addProperty(styleProp_);
    addProperty(geometrySpacingProp_);
    addProperty(geometrySizeProp_);
    addProperty(colorTableProp);
    addProperty(useBoundingBoxProp_);
    addProperty(boundingBoxColorProp_);
    addProperty(useCoordinateAxisProp_);
    addProperty(useAlphaBlendingProp_);
    addProperty(camProp_);
    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera Handler", &camProp_);
    addInteractionHandler(cameraHandler_);

    addPort(volInport_);
    addPort(imgOutport_);

    onStyleChange();
    onStreamlineNumberChange();
    colorCoding_.onColorCodingModeChange();
    setPropertyVisibilities();
}

StreamlineRenderer3D::~StreamlineRenderer3D() {
    if (displayLists_ != 0)
        glDeleteLists(displayLists_, static_cast<GLsizei>(numStreamlines_));

    delete styleProp_;
    delete [] seedingPositions_;

    if ((shader_ != 0) && (shader_->isActivated() == true))
        shader_->deactivate();
    if(shader_)
        ShdrMgr.dispose(shader_);

    delete cameraHandler_;
}

void StreamlineRenderer3D::process() {

    if (volInport_.isReady() == false)
        return;

    bool changed = volInport_.hasChanged();
    currentVolume_ = volInport_.getData();

    // check whether the current Volume belongs to flow data
    //
    const VolumeFlow3D* volFlow = dynamic_cast<const VolumeFlow3D*>(currentVolume_->getRepresentation<VolumeRAM>());
    if (volFlow == 0) {
        LERROR("process(): volume data appear to not contain flow data!");
        return;
    }

    glPushAttrib(GL_TEXTURE_BIT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const VolumeTexture* const tex = currentVolume_->getRepresentation<VolumeGL>()->getTexture();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, tex->getId());

    if (changed == true)
        rebuildDisplayLists_ = true;

    flowDimensions_ = volFlow->getDimensions();
    if (reinitSeedingPositions_ == true)
        initSeedingPositions();

    // set the texture target assigned to the outport as the current render target
    //
    imgOutport_.activateTarget("StreamlineRenderer3D::process()");

    const Flow3D& flow3D = volFlow->getFlow3D();
    tgt::vec2 thresholds(flow3D.maxMagnitude_ * (thresholdProp_.get() / 100.0f));

    if (rebuildDisplayLists_ == true) {
        if (currentStyle_ == STYLE_ARROW_GRID)
            buildDisplayListsArrowGrid(flow3D);
        else
            buildDisplayLists(flow3D, thresholds);
    }

    // important: save current camera state before using the processor's camera or
    // successive processors will use those settings!
    //
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();

    camProp_.look(imgOutport_.getSize());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (useCoordinateAxisProp_.get() == true)
        renderCoordinateAxis();

    if (useBoundingBoxProp_.get() == true)
        renderBoundingBox(boundingBoxColorProp_.get());

    setupShader(volFlow->getFlow3D(), thresholds);

    if ((shader_ != 0) && (shader_->isActivated() == true)) {
        shader_->setUniform("lightDir_", camProp_.get().getLook());
        shader_->setUniform("camPos_", camProp_.get().getPosition());

        if (useAlphaBlendingProp_.get() == true) {
            shader_->setUniform("clippingPlanes_", tgt::vec2(camProp_.get().getNearDist(),
                camProp_.get().getFarDist()));
        } else
            shader_->setUniform("clippingPlanes_", tgt::vec2(0.0f));
    }

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    for (GLuint i = 0; i < numStreamlines_; ++i)
        glCallList(displayLists_ + i);

    glDisable(GL_CULL_FACE);

    if (shader_ != 0)
        shader_->deactivate();
    glPopAttrib();
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    imgOutport_.deactivateTarget();
}

void StreamlineRenderer3D::initialize() throw (tgt::Exception) {
    FlowreenProcessor::init();
    RenderProcessor::initialize();
}

// private methods
//

void StreamlineRenderer3D::buildDisplayLists(const Flow3D& flow, const tgt::vec2& thresholds) {
    if (rebuildDisplayLists_ == false)
        return;

    const tgt::vec3 dim = static_cast<tgt::vec3>(flow.dimensions_);
    const float integrationLength = maxStreamlineLengthProp_.get();
    const float stepwidth = 0.5f;

    if (displayLists_ != 0)
        glDeleteLists(displayLists_, static_cast<GLsizei>(numStreamlines_));

    displayLists_ = glGenLists(static_cast<GLsizei>(numStreamlines_));
    GLuint i = 0;
    size_t numTries = 0;
    const size_t maxNumTries = numStreamlines_ * 5; // HACK: tries per streamline
    for (; ((i < numStreamlines_) && (numTries < maxNumTries)); ++numTries) {
        const tgt::vec3& r0 = seedingPositions_[i];

        // in case of flow at r0 being zero or with its magnitude not fitting
        // into the range defined by thresholds, the random position leads to no
        // useful streamline so that another position has to be taken
        //
        std::deque<tgt::vec3> streamlineDeque =
            FlowMath::computeStreamlineRungeKutta(flow, r0, integrationLength, stepwidth, 0, thresholds);

        if (streamlineDeque.size() <= 1) {
            seedingPositions_[i] = reseedPosition(dim, i);
            continue;
        }

        std::vector<tgt::vec3> streamline = FlowMath::dequeToVector(streamlineDeque);

        glNewList(displayLists_ + i, GL_COMPILE);
        //glColor4fv(fancyColors_[i % NUM_COLORS].elem);

        switch (currentStyle_) {
            case STYLE_LINES:
                renderStreamlineLines(streamline, flow);
                break;
            case STYLE_TUBES:
                renderStreamlineTubes(streamline);
                break;
            case STYLE_ARROWS:
                renderStreamlineArrows(streamline);
                break;
            default:
                break;
        }

        glEndList();
        streamline.clear();
        ++i;
    }   // for (i

    if (numTries >= maxNumTries) {
        LINFO("Only " << i << " streamlines could be created from valid random seeding positions. \
Giving up after " << numTries << " tries.\n");
    }

    rebuildDisplayLists_ = false;
}

void StreamlineRenderer3D::buildDisplayListsArrowGrid(const Flow3D& flow) {
    if (rebuildDisplayLists_ == false)
        return;

    tgt::vec3 dim = static_cast<tgt::vec3>(flow.dimensions_);
    const size_t arrowScaling = static_cast<size_t>(geometrySizeProp_.get());

    int spacing = geometrySpacingProp_.get();
    if (spacing > tgt::min(flow.dimensions_))
        spacing = tgt::min(flow.dimensions_);
    else if(spacing <= 0)
        spacing = 1;

    tgt::ivec3 grid = flow.dimensions_ / spacing;   // the dimensions of the grid
    numStreamlines_ = grid.z + 1;  // use a display list for each slice coplanar to XY-plane

    if (displayLists_ != 0)
        glDeleteLists(displayLists_, static_cast<GLsizei>(numStreamlines_));
    displayLists_ = glGenLists(static_cast<GLsizei>(numStreamlines_));

    for (GLuint z = 0; z <= numStreamlines_; ++z) {
        glNewList(displayLists_ + z, GL_COMPILE);

        for (int y = 0; y <= grid.y; ++y) {
            for (int x = 0; x <= grid.x; ++x) {
                tgt::ivec3 r(x * spacing, y * spacing, z * spacing);
                if (flow.isInsideBoundings(r) == false)
                    continue;

                tgt::vec3 v = flow.lookupFlow(r);
                if (v == tgt::vec3::zero)
                    continue;

                tgt::vec3 texCoord(r.x / dim.x, r.y / dim.y, r.z / dim.z);
                glMultiTexCoord3fv(GL_TEXTURE0, texCoord.elem);
                tgt::mat4 trafo = flow.getTransformationMatrix(r, static_cast<float>(arrowScaling));
                renderArrow(trafo);
            }   // for (x
        }   // for (y

        glEndList();
    }   // for (z
    rebuildDisplayLists_ = false;
}

void StreamlineRenderer3D::initSeedingPositions() {
    if (reinitSeedingPositions_ == false)
        return;

    delete [] seedingPositions_;
    seedingPositions_ = new tgt::vec3[numStreamlines_];
    for (size_t i = 0; i < numStreamlines_; ++i)
        seedingPositions_[i] = (FlowMath::uniformRandomVec3() * static_cast<tgt::vec3>(flowDimensions_));

    reinitSeedingPositions_ = false;
}

void StreamlineRenderer3D::invalidateRendering() {
    rebuildDisplayLists_ = true;
}

void StreamlineRenderer3D::onColorCodingChange() {
    if (shader_ == 0)
        return;

    shader_->deactivate();
    shader_->setHeaders(generateShaderHeader());
    shader_->rebuild();
}

void StreamlineRenderer3D::onStreamlineNumberChange() {
    numStreamlines_ = static_cast<size_t>(numStreamlinesProp_.get());
    rebuildDisplayLists_ = true;
    reinitSeedingPositions_ = true;
}

void StreamlineRenderer3D::onStyleChange() {
    // delete old seeding positions on grid, if
    // the former rendering style was "grid"
    //
    if (currentStyle_ == STYLE_ARROW_GRID)
        onStreamlineNumberChange();

    currentStyle_ = styleProp_->getValue();
    rebuildDisplayLists_ = true;
    setPropertyVisibilities();
}

void StreamlineRenderer3D::renderStreamlineArrows(const std::vector<tgt::vec3>& streamline) const {
    const size_t arrowScaling = static_cast<size_t>(geometrySizeProp_.get());
    const size_t steps = static_cast<size_t>(geometrySpacingProp_.get()) * arrowScaling;

    tgt::vec3 texCoord(0.0f);
    for (size_t n = 0; n < streamline.size(); n += steps ) {
        texCoord = streamline[n] / static_cast<tgt::vec3>(flowDimensions_);
        glMultiTexCoord3fv(GL_TEXTURE0, texCoord.elem);

        tgt::mat4 trafo = FlowMath::getTransformationMatrix(streamline, n, static_cast<float>(arrowScaling));
        renderArrow(trafo);
    }
}

void StreamlineRenderer3D::renderStreamlineLines(const std::vector<tgt::vec3>& streamline,
                                                 const Flow3D& flow) const
{
    const size_t stepwidth = static_cast<size_t>(geometrySpacingProp_.get());
    const float lineSize = static_cast<float>(geometrySizeProp_.get());

    glLineWidth(lineSize);
    glBegin(GL_LINE_STRIP);
        for (size_t n = 0; n < streamline.size(); n += stepwidth) {
            tgt::vec3 texCoord(streamline[n].x / flowDimensions_.x,
                streamline[n].y / flowDimensions_.y,
                streamline[n].z / flowDimensions_.z);

            glMultiTexCoord3fv(GL_TEXTURE0, texCoord.elem);
            glNormal3fv(FlowMath::normalize(flow.lookupFlow(streamline[n])).elem);
            glVertex3fv(mapToFlowBoundingBox(streamline[n]).elem);
        }
    glEnd();
    glLineWidth(1.0f);
}

void StreamlineRenderer3D::renderStreamlineTubes(const std::vector<tgt::vec3>& streamline) const {
    const size_t stepwidth = static_cast<size_t>(geometrySpacingProp_.get());
    const float tubeRadius = geometrySizeProp_.get() / 10.0f;

    tgt::mat4 trans = FlowMath::getTransformationMatrix(streamline, 0, tubeRadius);
    std::vector<tgt::vec3> circleInit = getTransformedCircle(trans);
    std::vector<tgt::vec3>& circle1 = circleInit;
    tgt::vec3 texCoord(0.0f);

    for (size_t n = 1; n < streamline.size(); n += stepwidth) {
        trans = FlowMath::getTransformationMatrix(streamline, n, tubeRadius);
        std::vector<tgt::vec3> circle2 = getTransformedCircle(trans);

        glBegin(GL_QUAD_STRIP);
        for (size_t k = 0; k <= circle1.size(); ++k) {
            const tgt::vec3& r1 = circle1[k % circle1.size()];
            const tgt::vec3& r2 = circle2[k % circle2.size()];
            const tgt::vec3& r4 = circle1[(k + 1) % circle1.size()];

            tgt::vec3 normal = FlowMath::normalize( tgt::cross((r2 - r1), (r4 - r1)) );
            glNormal3fv(normal.elem);

            texCoord.x = streamline[n - 1].x / flowDimensions_.x;
            texCoord.y = streamline[n - 1].y / flowDimensions_.y;
            texCoord.z = streamline[n - 1].z / flowDimensions_.z;
            glMultiTexCoord3fv(GL_TEXTURE0, texCoord.elem);
            glVertex3fv(mapToFlowBoundingBox(r1).elem);

            texCoord.x = streamline[n].x / flowDimensions_.x;
            texCoord.y = streamline[n].y / flowDimensions_.y;
            texCoord.z = streamline[n].z / flowDimensions_.z;
            glMultiTexCoord3fv(GL_TEXTURE0, texCoord.elem);
            glVertex3fv(mapToFlowBoundingBox(r2).elem);
        }   // for (k
        glEnd();

        circle1.clear();
        circle1 = circle2;
    }   // for (n
    circle1.clear();
}

tgt::vec3 StreamlineRenderer3D::reseedPosition(const tgt::vec3& flowDimensions,
                                               const size_t validPositions)
{
    tgt::vec3 randVec = FlowMath::uniformRandomVec3();

    // Use a "die" to determine wether a completely new random position
    // will be taken or wether an exisiting one will be used.
    // When the probability for a new position is low, the seeding positions
    // seem be prone to cluster at single location.
    //
    int die = rand() % 6;
    if ((die < 3) || (validPositions <= 1) || (seedingPositions_ == 0))
        return randVec * flowDimensions;

    // if there are already random positions which lead to a vector-field value not being
    // zero or which falls within the limits defined by thresholds, take this
    // position to generate another.
    // Use the position and add some random offset to it.
    //
    size_t index = rand() % validPositions;
    randVec *= static_cast<float>((rand() % 10) + 1);

    return tgt::clamp((seedingPositions_[index] + randVec), tgt::vec3::zero, flowDimensions);
}

void StreamlineRenderer3D::setPropertyVisibilities() {
    if (currentStyle_ == STYLE_ARROW_GRID) {
        geometrySpacingProp_.setMinValue(4);
        if (geometrySpacingProp_.get() < 4)
            geometrySpacingProp_.set(4);

        maxStreamlineLengthProp_.setVisible(false);
        thresholdProp_.setVisible(false);
        numStreamlinesProp_.setVisible(false);
    } else {
        geometrySpacingProp_.setMinValue(1);
        maxStreamlineLengthProp_.setVisible(true);
        thresholdProp_.setVisible(true);
        numStreamlinesProp_.setVisible(true);
    }
}
std::string StreamlineRenderer3D::generateShaderHeader() {
    std::string header = RenderProcessor::generateHeader();
    header += colorCoding_.getShaderDefines();
    return header;
}

bool StreamlineRenderer3D::setupShader(const Flow3D& flow, const tgt::vec2& thresholds) {
    if (shader_ == 0) {
        try {
            shader_ = ShdrMgr.loadSeparate("phong.vert", "streamlinerenderer3d.frag",
                generateShaderHeader(), false);
        } catch(tgt::Exception) {
            LGL_ERROR;
            return false;
        }
    }

    // activate the shader and set the needed uniforms
    if (shader_->isActivated() == false)
        shader_->activate();

    shader_->setUniform("usePhongShading_", true);
    shader_->setUniform("ka_", 0.3f);
    shader_->setUniform("kd_", 0.7f);
    shader_->setUniform("ks_", 1.0f);
    shader_->setUniform("shininess_", 32.0f);

    shader_->setUniform("vectorField3D_", 0);
    shader_->setUniform("minValue_", flow.minValue_);
    shader_->setUniform("maxValue_", flow.maxValue_);
    shader_->setUniform("maxMagnitude_", flow.maxMagnitude_);
    shader_->setUniform("thresholds_", thresholds);

    if (currentStyle_ != STYLE_LINES)
        shader_->setUniform("useForLines_", false);
    else
        shader_->setUniform("useForLines_", true);
    return true;
}

// private static methods
//

}   // namespace
