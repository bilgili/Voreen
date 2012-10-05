/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "explosionproxygeometry.h"
#include "tgt/glmath.h"
#include <vector>

namespace voreen {

ExplosionProxyGeometry::ExplosionProxyGeometry()
    : RenderProcessor()
    , canvasSize_("canvasSize", "Canvas Size", tgt::ivec2(256), tgt::ivec2(64), tgt::ivec2(10000), Processor::VALID)
    , brickingMode_("BrickingMode", "Bricking mode", Processor::INVALID_PROGRAM)
    , showBoundingBoxes_("showBoundingBoxes", "Show bounding boxes", Processor::INVALID_PROGRAM)
    , brickColor_("brickColor", "Brick color", tgt::vec4(0.0f, 0.0f, 0.0f, 1.0f))
    , selectedBrickColor_("selectedBrickColor", "Selected brick color", tgt::vec4(1.0f, 1.0f, 0.0f, 1.0f))
    , hideNonSelectedBricks_("hideNonSelectedBricks", "Hide non-selected bricks")
    , hideSelectedBricks_("hideSelectedBricks", "Hide selected bricks")
    , showAllBricks_("showAllBricks", "Show all bricks")
    , numBricksXp_("numberBricksX", "Number of bricks on X", 1, 1, 10)
    , numBricksYp_("numberBricksY", "Number of bricks on Y", 1, 1, 10)
    , numBricksZp_("numberBricksZ", "Number of bricks on Z", 1, 1, 10)
    , regularExplosionGap_("regularBrickExplosionGap", "Explosion-gap between bricks", 0.f, 0.f, 10.f)
    , manualExplosionGap_("manualBrickExplosionGap", "Explosion-gap between bricks", 0.f, 0.f, 10.f)
    , XbrickingPlane_("XbrickingPlane", "X-brickingplane", 0, 0, 100000)
    , addXbrickingPlane_("addXbrickingPlane", "Add X-brickingplane")
    , deleteLastXbrickingPlane_("deleteLastXbrickingPlane", "Delete last X-brickingplane")
    , clearXbrickingList_("clearXbrickingList", "Clear X-bricking list")
    , YbrickingPlane_("YbrickingPlane", "Y-brickingplane", 0, 0, 100000)
    , addYbrickingPlane_("addYbrickingPlane", "Add Y-brickingplane")
    , deleteLastYbrickingPlane_("deleteLastYbrickingPlane", "Delete last Y-brickingplane")
    , clearYbrickingList_("clearYbrickingList", "Clear Y-bricking list")
    , ZbrickingPlane_("ZbrickingPlane", "Z-brickingplane", 0, 0, 100000)
    , addZbrickingPlane_("addZbrickingPlane", "Add Z-brickingplane")
    , deleteLastZbrickingPlane_("deleteLastZbrickingPlane", "Delete last Z-brickingplane")
    , clearZbrickingList_("clearZbrickingList", "Clear Z-bricking list")
    , resetMode_("resetMode", "Reset mode")
    , undo_("Undo", "Undo")
    , takeForCustom_("takeForCustomMode", "Take this for custom mode")
    , translation_("translation", "Translation", tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(-10.f), tgt::vec3(10.f))
    , camera_("camera", "Camera", tgt::Camera())
    , selectingOneBrickEvent_("mouseEvent.selectingOneBrick", "Select one brick", this,
                               &ExplosionProxyGeometry::onSelectingOneBrickEvent,
                               tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::PRESSED, tgt::Event::CTRL)
    , selectingSeveralBricksEvent_("mouseEvent.selectingSeveralBricks", "Select several bricks", this,
                                    &ExplosionProxyGeometry::onSelectingSeveralBricksEvent,
                                    tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::PRESSED, tgt::Event::ALT)
    , translateSelectedBricksEvent_("mouseEvent.translateSelectedBricks", "Translate selected bricks", this,
                                     &ExplosionProxyGeometry::onTranslateSelectedBricksEvent,
                                     tgt::MouseEvent::MOUSE_BUTTON_LEFT,
                                     tgt::MouseEvent::PRESSED | tgt::MouseEvent::MOTION | tgt::MouseEvent::RELEASED,
                                     tgt::Event::Modifier(tgt::Event::CTRL | tgt::Event::ALT))
    , inportVolume_(Port::INPORT, "volumehandle.in", "Volume Input")
    , loopInport_(Port::INPORT, "loop.inport", "Loop Inport")
    , outportProxyGeometry_(Port::OUTPORT, "geometry.proxy", "Proxy Geometry")
    , outportRenderGeometry_(Port::OUTPORT, "geometry.render", "Render Geometry")
    , pickingBuffer_(Port::OUTPORT, "pickingBuffer")
{
    brickColor_.setViews(Property::COLOR);
    selectedBrickColor_.setViews(Property::COLOR);

    loopInport_.setLoopPort(true);
    addPort(inportVolume_);
    addPort(loopInport_);
    addPort(outportProxyGeometry_);
    addPort(outportRenderGeometry_);
    addPrivateRenderPort(pickingBuffer_);

    canvasSize_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::canvasSizeChanged));
    canvasSize_.setWidgetsEnabled(false);
    canvasSize_.setVisible(false);
    addProperty(canvasSize_);

    brickingMode_.addOption("regularGrid", "Regular Grid");
    brickingMode_.addOption("manualGrid", "Manual Grid");
    brickingMode_.addOption("custom", "Custom");
    addProperty(brickingMode_);
    brickingMode_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::onBrickingModeChange));
    showBoundingBoxes_.addOption("all", "All");
    showBoundingBoxes_.addOption("none", "None");
    showBoundingBoxes_.addOption("selected", "Selected");
    addProperty(showBoundingBoxes_);
    addProperty(brickColor_);
    addProperty(selectedBrickColor_);
    addProperty(hideNonSelectedBricks_);
    hideNonSelectedBricks_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, bool>
                                    (this, &ExplosionProxyGeometry::onHideBricksChange, false));
    addProperty(hideSelectedBricks_);
    hideSelectedBricks_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, bool>
                                 (this, &ExplosionProxyGeometry::onHideBricksChange, true));
    addProperty(showAllBricks_);
    showAllBricks_.onChange(CallMemberAction<ExplosionProxyGeometry>
                            (this, &ExplosionProxyGeometry::onShowAllBricksChange));


    //Properties only for regularGrid bricking mode
    addProperty(numBricksXp_);
    numBricksXp_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::doBricking));
    addProperty(numBricksYp_);
    numBricksYp_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::doBricking));
    addProperty(numBricksZp_);
    numBricksZp_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::doBricking));
    addProperty(regularExplosionGap_);
    regularExplosionGap_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::onExplosionGapChange));

    //Properties for manual and custom bricking mode
    numBricksXm_ = 1;
    numBricksYm_ = 1;
    numBricksZm_ = 1;
    addProperty(XbrickingPlane_);
    addProperty(addXbrickingPlane_);
    addXbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                (this, &ExplosionProxyGeometry::onAddBrickingPlaneChange, 'x'));
    addProperty(deleteLastXbrickingPlane_);
    deleteLastXbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                       (this, &ExplosionProxyGeometry::onDeleteLastBrickingPlaneChange, 'x'));
    addProperty(clearXbrickingList_);
    clearXbrickingList_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                 (this, &ExplosionProxyGeometry::onClearBrickingListChange, 'x'));
    addProperty(YbrickingPlane_);
    addProperty(addYbrickingPlane_);
    addYbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                (this, &ExplosionProxyGeometry::onAddBrickingPlaneChange, 'y'));
    addProperty(deleteLastYbrickingPlane_);
    deleteLastYbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                       (this, &ExplosionProxyGeometry::onDeleteLastBrickingPlaneChange, 'y'));
    addProperty(clearYbrickingList_);
    clearYbrickingList_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                 (this, &ExplosionProxyGeometry::onClearBrickingListChange, 'y'));
    addProperty(ZbrickingPlane_);
    addProperty(addZbrickingPlane_);
    addZbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                (this, &ExplosionProxyGeometry::onAddBrickingPlaneChange, 'z'));
    addProperty(deleteLastZbrickingPlane_);
    deleteLastZbrickingPlane_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                       (this, &ExplosionProxyGeometry::onDeleteLastBrickingPlaneChange, 'z'));
    addProperty(clearZbrickingList_);
    clearZbrickingList_.onChange(Call1ParMemberAction<ExplosionProxyGeometry, char>
                                 (this, &ExplosionProxyGeometry::onClearBrickingListChange, 'z'));
    addProperty(manualExplosionGap_);
    manualExplosionGap_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::onExplosionGapChange));
    addProperty(translation_);
    translation_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::onTranslationChange));


    addProperty(takeForCustom_);
    takeForCustom_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::takeForCustom));

    addProperty(undo_);
    undo_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::undo));

    addProperty(resetMode_);
    resetMode_.onChange(CallMemberAction<ExplosionProxyGeometry>(this, &ExplosionProxyGeometry::resetMode));
    addProperty(camera_);

    addEventProperty(selectingOneBrickEvent_);
    addEventProperty(selectingSeveralBricksEvent_);
    addEventProperty(translateSelectedBricksEvent_);
    isClicked_ = false;

    onBrickingModeChange();

}

ExplosionProxyGeometry::~ExplosionProxyGeometry() {
}

Processor* ExplosionProxyGeometry::create() const {
    return new ExplosionProxyGeometry();
}

void ExplosionProxyGeometry::initialize() throw (tgt::Exception) {
    RenderProcessor::initialize();

    idManager_.setRenderTarget(pickingBuffer_.getRenderTarget());
    idManager_.initializeTarget();

    onBrickingModeChange();
}

void ExplosionProxyGeometry::deinitialize() throw (tgt::Exception) {
    outportProxyGeometry_.setData(0);
    outportRenderGeometry_.setData(0);

    RenderProcessor::deinitialize();
}

bool ExplosionProxyGeometry::isReady() const {
    return (inportVolume_.isReady() && (outportProxyGeometry_.isConnected() || outportRenderGeometry_.isConnected()));
}

void ExplosionProxyGeometry::beforeProcess() {
    RenderProcessor::beforeProcess();

    if (inportVolume_.hasChanged() && (loopInport_.getLoopIteration() == 0)) {
        if (brickingMode_.isSelected("regularGrid")) {
            regularBricklist_.clear();
            loopInport_.setNumLoopIterations(numBricksXp_.get() * numBricksYp_.get() * numBricksZp_.get());
            updatePlaneWidgets();
        }
        else {
            resetMode();
        }
    }
}

void ExplosionProxyGeometry::process() {
    tgtAssert(inportVolume_.getData()->getRepresentation<VolumeRAM>(), "no volume");

    int iteration = loopInport_.getLoopIteration();

    /////////////////////////////Bricking process beginning/////////////////////////////////////
    MeshListGeometry* proxyGeometry = new MeshListGeometry();
    MeshListGeometry* renderGeometry = new MeshListGeometry();

    //regularGrid bricking
    if (brickingMode_.get() == "regularGrid") {
        if (iteration == 0) {

            if (regularBricklist_.empty())
                createRegularOrManualBricklist(loopInport_.getNumLoopIterations());

            outputBricklist_ = sortBricklist(regularBricklist_);
        }

        proxyGeometry->addMesh(MeshGeometry::createCube(
             outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
             outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
             outputBricklist_.at(iteration).texLlf,
             outputBricklist_.at(iteration).texUrb));

        if (showBoundingBoxes_.get() == "all" || showBoundingBoxes_.get() == "selected") {
            renderGeometry->addMesh(MeshGeometry::createCube(
                outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).texLlf,
                outputBricklist_.at(iteration).texUrb,
                brickColor_.get().xyz(),
                brickColor_.get().xyz()));
        }
    }
    //manualGrid bricking
    else if (brickingMode_.get() == "manualGrid") {
        if (iteration == 0) {

            if (manualBricklist_.empty())
                doBricking();

            outputBricklist_ = sortBricklist(manualBricklist_);
        }

        proxyGeometry->addMesh(MeshGeometry::createCube(
            outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
            outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
            outputBricklist_.at(iteration).texLlf,
            outputBricklist_.at(iteration).texUrb));

        if (showBoundingBoxes_.get() == "all" || showBoundingBoxes_.get() == "selected") {
            renderGeometry->addMesh(MeshGeometry::createCube(
                outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).texLlf,
                outputBricklist_.at(iteration).texUrb,
                brickColor_.get().xyz(),
                brickColor_.get().xyz()));
        }
    }
    //custom bricking
    else{
        if (iteration == 0) {
            if (customBricklist_.empty())
                doBricking();

            outputBricklist_ = sortBricklist(customBricklist_);
        }

        registerForSelecting(customBricklist_);

        if (outputBricklist_.at(iteration).selected) {
            if (showBoundingBoxes_.get() == "all" || showBoundingBoxes_.get() == "selected") {
                renderGeometry->addMesh(MeshGeometry::createCube(
                    outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                    outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                    outputBricklist_.at(iteration).texLlf,
                    outputBricklist_.at(iteration).texUrb,
                    selectedBrickColor_.get().xyz(),
                    selectedBrickColor_.get().xyz()));

            }
            proxyGeometry->addMesh(MeshGeometry::createCube(
                outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                outputBricklist_.at(iteration).texLlf,
                outputBricklist_.at(iteration).texUrb));
        }
        else {
            if (!outputBricklist_.at(iteration).hidden) {
                if (showBoundingBoxes_.get() == "all") {
                    renderGeometry->addMesh(MeshGeometry::createCube(
                        outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                        outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                        outputBricklist_.at(iteration).texLlf,
                        outputBricklist_.at(iteration).texUrb,
                        brickColor_.get().xyz(),
                        brickColor_.get().xyz()));
                }
                proxyGeometry->addMesh(MeshGeometry::createCube(
                    outputBricklist_.at(iteration).coordLlf + outputBricklist_.at(iteration).translation,
                    outputBricklist_.at(iteration).coordUrb + outputBricklist_.at(iteration).translation,
                    outputBricklist_.at(iteration).texLlf,
                    outputBricklist_.at(iteration).texUrb));
            }
        }

    }

    outportProxyGeometry_.setData(proxyGeometry);
    outportRenderGeometry_.setData(renderGeometry);
    LGL_ERROR;
}

void ExplosionProxyGeometry::onBrickingModeChange() {

    if (brickingMode_.get() == "regularGrid") {

        loopInport_.setNumLoopIterations(numBricksXp_.get() * numBricksYp_.get() * numBricksZp_.get());
        //showBoundingBoxes_.set("all");
        selectedBrickColor_.setVisible(false);
        hideNonSelectedBricks_.setVisible(false);
        hideSelectedBricks_.setVisible(false);
        showAllBricks_.setVisible(false);
        numBricksXp_.setVisible(true);
        numBricksYp_.setVisible(true);
        numBricksZp_.setVisible(true);
        XbrickingPlane_.setVisible(false);
        addXbrickingPlane_.setVisible(false);
        deleteLastXbrickingPlane_.setVisible(false);
        clearXbrickingList_.setVisible(false);
        YbrickingPlane_.setVisible(false);
        addYbrickingPlane_.setVisible(false);
        deleteLastYbrickingPlane_.setVisible(false);
        clearYbrickingList_.setVisible(false);
        ZbrickingPlane_.setVisible(false);
        addZbrickingPlane_.setVisible(false);
        deleteLastZbrickingPlane_.setVisible(false);
        clearZbrickingList_.setVisible(false);
        regularExplosionGap_.setVisible(true);
        manualExplosionGap_.setVisible(false);
        translation_.setVisible(false);
        takeForCustom_.setVisible(true);
        undo_.setVisible(false);
        resetMode_.setVisible(true);
        if (inportVolume_.hasData())
            updatePlaneWidgets();
    }
    else if (brickingMode_.get() == "manualGrid") {
        doBricking();
        showBoundingBoxes_.set("all");
        selectedBrickColor_.setVisible(false);
        hideNonSelectedBricks_.setVisible(false);
        hideSelectedBricks_.setVisible(false);
        showAllBricks_.setVisible(false);
        numBricksXp_.setVisible(false);
        numBricksYp_.setVisible(false);
        numBricksZp_.setVisible(false);
        XbrickingPlane_.setVisible(true);
        addXbrickingPlane_.setVisible(true);
        deleteLastXbrickingPlane_.setVisible(true);
        clearXbrickingList_.setVisible(true);
        YbrickingPlane_.setVisible(true);
        addYbrickingPlane_.setVisible(true);
        deleteLastYbrickingPlane_.setVisible(true);
        clearYbrickingList_.setVisible(true);
        ZbrickingPlane_.setVisible(true);
        addZbrickingPlane_.setVisible(true);
        deleteLastZbrickingPlane_.setVisible(true);
        clearZbrickingList_.setVisible(true);
        regularExplosionGap_.setVisible(false);
        manualExplosionGap_.setVisible(true);
        translation_.setVisible(false);
        if (selectedBricks_.size() == 1)
            translation_.set(selectedBricks_.at(0).translation);
        else if (selectedBricks_.size() > 1)
            translation_.set(findMiddle(selectedBricks_));
        else
            translation_.set(tgt::vec3(0.f, 0.f, 0.f));

        takeForCustom_.setVisible(true);
        undo_.setVisible(false);
        resetMode_.setVisible(true);
        if (inportVolume_.hasData())
            updatePlaneWidgets();
    }
    else{
        doBricking();
        showBoundingBoxes_.set("all");
        selectedBrickColor_.setVisible(true);
        hideNonSelectedBricks_.setVisible(true);
        hideSelectedBricks_.setVisible(true);
        showAllBricks_.setVisible(true);
        numBricksXp_.setVisible(false);
        numBricksYp_.setVisible(false);
        numBricksZp_.setVisible(false);
        XbrickingPlane_.setVisible(true);
        addXbrickingPlane_.setVisible(true);
        deleteLastXbrickingPlane_.setVisible(false);
        clearXbrickingList_.setVisible(false);
        YbrickingPlane_.setVisible(true);
        addYbrickingPlane_.setVisible(true);
        deleteLastYbrickingPlane_.setVisible(false);
        clearYbrickingList_.setVisible(false);
        ZbrickingPlane_.setVisible(true);
        addZbrickingPlane_.setVisible(true);
        deleteLastZbrickingPlane_.setVisible(false);
        clearZbrickingList_.setVisible(false);
        regularExplosionGap_.setVisible(false);
        manualExplosionGap_.setVisible(false);
        translation_.setVisible(true);
        takeForCustom_.setVisible(false);
        undo_.setVisible(true);
        resetMode_.setVisible(true);
        if (inportVolume_.hasData())
            updatePlaneWidgets();
    }
}

void ExplosionProxyGeometry::onHideBricksChange(bool selected) {
    size_t i;

    if (selected)
        selectedBricks_.clear();

    for (i = 0; i < customBricklist_.size(); ++i) {
        if (customBricklist_.at(i).selected) {
            if (selected) {
                customBricklist_.at(i).selected = false;
                customBricklist_.at(i).hidden = true;
            }
        }
        else if (!customBricklist_.at(i).hidden) {
            if (selected) {
                customBricklist_.at(i).selected = true;
                selectedBricks_.insert(selectedBricks_.begin(), customBricklist_.at(i));
            }
            else{
                customBricklist_.at(i).hidden = true;
            }
        }
    }

    updatePlaneWidgets();
    invalidate();
}

void ExplosionProxyGeometry::onShowAllBricksChange() {
    size_t i;

    for (i = 0; i < customBricklist_.size(); ++i) {
        customBricklist_.at(i).hidden = false;
        if (selectedBricks_.empty())
            customBricklist_.at(i).selected = true;
    }

    if (selectedBricks_.empty())
        selectedBricks_ = customBricklist_;

    updatePlaneWidgets();
    invalidate();
}

void ExplosionProxyGeometry::doBricking() {
    if (!inportVolume_.getData())
        return;

    const VolumeBase* volume = inportVolume_.getData();

    tgt::vec3 volumeSize = volume->getCubeSize();;
    tgt::vec3 offsetLlf;
    tgt::vec3 offsetUrb;
    tgt::vec3 texLlf;
    tgt::vec3 texUrb;

    if (brickingMode_.isSelected("regularGrid")) {
        createRegularOrManualBricklist(numBricksXp_.get() * numBricksYp_.get() * numBricksZp_.get());
        loopInport_.setNumLoopIterations(numBricksXp_.get() * numBricksYp_.get() * numBricksZp_.get());
    }
    else if (brickingMode_.isSelected("manualGrid")) {
        offsetLlf = tgt::vec3(0.f, 0.f, 0.f);
        offsetUrb = tgt::vec3(volumeSize.x, volumeSize.y, -volumeSize.z);
        texLlf = tgt::vec3(0.f, 0.f, 1.f);
        texUrb = tgt::vec3(1.f, 1.f, 0.f);

        if (xPlanescoord_.empty()) {
            xPlanescoord_.insert(xPlanescoord_.begin(), offsetLlf.x);
            xPlanescoord_.insert(xPlanescoord_.end(), offsetUrb.x);
            xPlanestex_.insert(xPlanestex_.begin(), texLlf.x);
            xPlanestex_.insert(xPlanestex_.end(), texUrb.x);
        }

        if (yPlanescoord_.empty()) {
            yPlanescoord_.insert(yPlanescoord_.begin(), offsetLlf.y);
            yPlanescoord_.insert(yPlanescoord_.end(), offsetUrb.y);
            yPlanestex_.insert(yPlanestex_.begin(), texLlf.y);
            yPlanestex_.insert(yPlanestex_.end(), texUrb.y);
        }

        if (zPlanescoord_.empty()) {
            zPlanescoord_.insert(zPlanescoord_.begin(), offsetLlf.z);
            zPlanescoord_.insert(zPlanescoord_.end(), offsetUrb.z);
            zPlanestex_.insert(zPlanestex_.begin(), texLlf.z);
            zPlanestex_.insert(zPlanestex_.end(), texUrb.z);
        }

        createRegularOrManualBricklist(static_cast<int>(numBricksXm_ * numBricksYm_ * numBricksZm_));
        loopInport_.setNumLoopIterations(static_cast<int>(numBricksXm_ * numBricksYm_ * numBricksZm_));
    }
    else{
        if (customBricklist_.empty())
            createCustomBricklist(' ', 0, 0);

        loopInport_.setNumLoopIterations(static_cast<int>(customBricklist_.size()));
    }
}

void ExplosionProxyGeometry::onAddBrickingPlaneChange(char axis) {
    if (axis == 'x')
        addBrickingPlane('x', numBricksXm_, xPlanescoord_, xPlanestex_, xCoords_);

    else if (axis == 'y')
        addBrickingPlane('y', numBricksYm_, yPlanescoord_, yPlanestex_, yCoords_);

    else
        addBrickingPlane('z', numBricksZm_, zPlanescoord_, zPlanestex_, zCoords_);
}

void ExplosionProxyGeometry::addBrickingPlane(char axis, size_t& numBricks, std::vector<float>& planescoord,
                                             std::vector<float>& planestex, std::stack<float>& coords) {

    const VolumeBase* volume = inportVolume_.getData();
    tgt::vec3 volumeSize = volume->getCubeSize();
    tgt::ivec3 numSlices = volume->getDimensions();
    tgt::vec3 texLlf(0, 0, 1);
    tgt::vec3 texUrb(1, 1, 0);
    size_t i;

    float tex;
    float offset;
    float offsetLlf;
    float offsetUrb;

    if (axis == 'x') {
        tex = XbrickingPlane_.get() / static_cast<tgt::vec3::ElemType>(numSlices.x);
        offset = -volumeSize.x * (texLlf.x - tex);
        if (brickingMode_.get() == "manualGrid") {
            offsetLlf = 0.f;
            offsetUrb = volumeSize.x;
        }
        else if (!selectedBricks_.empty()) {
            createCustomBricklist('x', offset, tex);
            loopInport_.setNumLoopIterations(static_cast<int>(customBricklist_.size()));
        }
    }
    else if (axis == 'y') {
        tex = YbrickingPlane_.get() / static_cast<tgt::vec3::ElemType>(numSlices.y);
        offset = -volumeSize.y * (texLlf.y - tex);
        if (brickingMode_.get() == "manualGrid") {
            offsetLlf = 0.f;
            offsetUrb = volumeSize.y;
        }
        else if (!selectedBricks_.empty()) {
            createCustomBricklist('y', offset, tex);
            loopInport_.setNumLoopIterations(static_cast<int>(customBricklist_.size()));
        }
    }
    else if (axis == 'z') {
        tex = ZbrickingPlane_.get() / static_cast<tgt::vec3::ElemType>(numSlices.z);
        offset = -volumeSize.z * (texLlf.z - tex);
        if (brickingMode_.get() == "manualGrid") {
            offsetLlf = 0.f;
            offsetUrb = -volumeSize.z;
        }
        else if (!selectedBricks_.empty()) {
            createCustomBricklist('z', offset, tex);
            loopInport_.setNumLoopIterations(static_cast<int>(customBricklist_.size()));
        }
    }

    if (brickingMode_.get() == "manualGrid") {
        if (planescoord.empty()) {
            planescoord.insert(planescoord.begin(), offset);
            planescoord.insert(planescoord.begin(), offsetLlf);
            planescoord.insert(planescoord.end(), offsetUrb);
            planestex.insert(planestex.begin(), tex);
            planestex.insert(planestex.begin(), texLlf.x);
            planestex.insert(planestex.end(), texUrb.x);
        }
        else{
            i=1;
            while (numBricks > (planescoord.size() - 2) && i < numBricks) {
                if (axis == 'x' || axis == 'y') {
                    if (planescoord.at(i) > offset) {
                        planescoord.insert(planescoord.begin() + i, offset);
                        planestex.insert(planestex.begin() + i, tex);
                    }
                    else{
                        ++i;
                    }
                }
                else if (axis == 'z') {
                    if (planescoord.at(i) < offset) {
                        planescoord.insert(planescoord.begin() + i, offset);
                        planestex.insert(planestex.begin() + i, tex);
                    }
                    else{
                        ++i;
                    }
                }
            }
            if (numBricks > (planescoord.size() - 2)) {
                planescoord.insert(planescoord.end() - 1, offset);
                planestex.insert(planestex.end() - 1, tex);
            }
        }
        coords.push(offset);
        numBricks = numBricks + 1;
        doBricking();
    }
}

void ExplosionProxyGeometry::onDeleteLastBrickingPlaneChange(char axis) {
    if (axis == 'x')
        deleteLastBrickingPlane(numBricksXm_, xPlanescoord_, xPlanestex_, xCoords_);

    else if (axis == 'y')
        deleteLastBrickingPlane(numBricksYm_, yPlanescoord_, yPlanestex_, yCoords_);

    else if (axis == 'z')
        deleteLastBrickingPlane(numBricksZm_, zPlanescoord_, zPlanestex_, zCoords_);
}

void ExplosionProxyGeometry::deleteLastBrickingPlane(size_t& numBricks, std::vector<float>& planescoord,
                                                    std::vector<float>& planestex, std::stack<float>& coords) {

    if (!coords.empty()) {
        float del = coords.top();
        coords.pop();
        bool deleted = false;
        int i = 0;

        while (!deleted) {
            if (planescoord.at(i) == del) {
                planescoord.erase(planescoord.begin() + i);
                planestex.erase(planestex.begin() + i);
                deleted = true;
            }
            else{
                ++i;
            }
        }

        numBricks = numBricks - 1;
        doBricking();
    }
}

void ExplosionProxyGeometry::onClearBrickingListChange(char axis) {
    clearBrickingList(axis);
    doBricking();
}

void ExplosionProxyGeometry::clearBrickingList(char axis) {

    if (axis == 'x') {
        numBricksXm_ = 1;
        xPlanescoord_.clear();
        xPlanestex_.clear();
        while (!xCoords_.empty())
            xCoords_.pop();
    }
    else if (axis == 'y') {
        numBricksYm_ = 1;
        yPlanescoord_.clear();
        yPlanestex_.clear();
        while (!yCoords_.empty())
            yCoords_.pop();
    }
    else{
        numBricksZm_ = 1;
        zPlanescoord_.clear();
        zPlanestex_.clear();
        while (!zCoords_.empty())
            zCoords_.pop();
    }

}

void ExplosionProxyGeometry::onExplosionGapChange() {
    if (inportVolume_.hasData()) {
        createRegularOrManualBricklist(loopInport_.getNumLoopIterations());
    }
}

void ExplosionProxyGeometry::onTranslationChange() {
    size_t i;

    if (selectedBricks_.size() == 1) {
        customBricklist_.at(selectedBricks_.at(0).brickId).translation = translation_.get();
        selectedBricks_.at(0).translation = translation_.get();
    }
    else if (selectedBricks_.size() > 1) {
        tgt::vec3 middle = findMiddle(selectedBricks_);
        tgt::vec3 distance = translation_.get() - middle;
        for (i = 0; i < selectedBricks_.size(); ++i) {
            customBricklist_.at(selectedBricks_.at(i).brickId).translation += distance;
            selectedBricks_.at(i).translation += distance;
        }
    }
    invalidate();
}

void ExplosionProxyGeometry::takeForCustom() {
    size_t i;

    if (brickingMode_.get() == "regularGrid") {
        brickingMode_.set("custom");
        resetMode();
        customBricklist_ = regularBricklist_;
    }
    else if (brickingMode_.get() == "manualGrid") {
        brickingMode_.set("custom");
        resetMode();
        customBricklist_ = manualBricklist_;
    }

    for (i = 0; i < customBricklist_.size(); ++i) {
        customBricklist_.at(i).brickId = static_cast<int>(i);

        if (customBricklist_.at(i).selected)
            selectedBricks_.insert(selectedBricks_.begin(), customBricklist_.at(i));
    }

    registerForSelecting(customBricklist_);
    onBrickingModeChange();

}

void ExplosionProxyGeometry::undo() {
    if (!customBricklistStack_.empty()) {
        customBricklist_.clear();
        customBricklist_ = customBricklistStack_.top();
        customBricklistStack_.pop();
        selectedBricks_.clear();

        size_t i;
        for (i = 0; i < customBricklist_.size(); ++i) {
            if (customBricklist_.at(i).selected)
                selectedBricks_.insert(selectedBricks_.begin(), customBricklist_.at(i));
        }

        if (selectedBricks_.empty())
            translation_.set(tgt::vec3(0.f, 0.f, 0.f));

        else if (selectedBricks_.size() == 1)
            translation_.set(selectedBricks_.at(0).translation);

        else
            translation_.set(findMiddle(selectedBricks_));

        registerForSelecting(customBricklist_);
        updatePlaneWidgets();
        loopInport_.setNumLoopIterations(static_cast<int>(customBricklist_.size()));
        invalidate();
    }
}

void ExplosionProxyGeometry::resetMode() {

    if (brickingMode_.isSelected("regularGrid")) {
        numBricksXp_.set(1);
        numBricksYp_.set(1);
        numBricksZp_.set(1);
        regularExplosionGap_.set(0);
        regularBricklist_.clear();
        loopInport_.setNumLoopIterations(numBricksXp_.get() * numBricksYp_.get() * numBricksZp_.get());
        updatePlaneWidgets();
    }
    else if (brickingMode_.isSelected("manualGrid")) {
        clearBrickingList('x');
        clearBrickingList('y');
        clearBrickingList('z');
        doBricking();
        loopInport_.setNumLoopIterations(1);
        updatePlaneWidgets();
        manualExplosionGap_.set(0);
    }
    else if (brickingMode_.isSelected("custom")) {
        while (!customBricklistStack_.empty())
            customBricklistStack_.pop();

        customBricklist_.clear();
        selectedBricks_.clear();
        translation_.set(tgt::vec3(0.f, 0.f, 0.f));
        updatePlaneWidgets();
        doBricking();
    }
    else {
        LERROR("Unknown bricking mode: " << brickingMode_.get());
    }

}

void ExplosionProxyGeometry::createRegularOrManualBricklist(int numIterations) {

    if (!inportVolume_.hasData()) {
        LWARNING("No volume!");
        return;
    }

    const VolumeBase* volume = inportVolume_.getData();
    tgt::vec3 volumeSize = volume->getCubeSize();
    tgt::ivec3 numSlices = volume->getDimensions();
    tgt::vec3 sliceWidth = getSliceWidth(volume);
    tgt::vec3 coordLlf = -(volumeSize / static_cast<tgt::vec3::ElemType>(2));
    tgt::vec3 coordUrb = (volumeSize / static_cast<tgt::vec3::ElemType>(2));

    std::swap(coordLlf.z, coordUrb.z);
    const tgt::vec3 texLlf(0, 0, 1);
    const tgt::vec3 texUrb(1, 1, 0);

    Brick current;
    int xOffset;
    int yOffset;
    int zOffset;
    int it;

    if (brickingMode_.get() == "regularGrid") {
        regularBricklist_.clear();

        float xBricks = static_cast<float>(numBricksXp_.get());
        float yBricks = static_cast<float>(numBricksYp_.get());
        float zBricks = static_cast<float>(numBricksZp_.get());
        float gx = -1.f * (xBricks - 1.f);
        float gy = -1.f * (yBricks - 1.f);
        float gz = -1.f * (zBricks - 1.f);
        float hx = volumeSize.x / xBricks;
        float hy = volumeSize.y / yBricks;
        float hz = volumeSize.z / zBricks;
        tgt::vec3 startcUrb = coordUrb - tgt::vec3((xBricks - 1.f) * hx,(yBricks - 1.f) * hy, -(zBricks - 1.f) * hz);
        tgt::vec3 starttUrb = texUrb - tgt::vec3((xBricks - 1.f) / xBricks,(yBricks - 1.f) / yBricks, -1.f * (zBricks - 1.f) / zBricks);


        for (it = 0; it < numIterations; ++it) {
            xOffset = it / (numBricksYp_.get() * numBricksZp_.get());
            yOffset = (it / numBricksZp_.get()) % numBricksYp_.get();
            zOffset = it % numBricksZp_.get();

            current.coordLlf = coordLlf + tgt::vec3(static_cast<float>(xOffset) * hx, static_cast<float>(yOffset) * hy,
                                                    -static_cast<float>(zOffset) * hz);
            current.coordUrb = startcUrb + tgt::vec3(static_cast<float>(xOffset) * hx, static_cast<float>(yOffset) * hy,
                                                     -static_cast<float>(zOffset) * hz);
            current.texLlf = texLlf + tgt::vec3(static_cast<float>(xOffset) / xBricks, static_cast<float>(yOffset) / yBricks,
                                                -static_cast<float>(zOffset) / zBricks);
            current.texUrb = starttUrb+tgt::vec3(static_cast<float>(xOffset)/xBricks,static_cast<float>(yOffset)/yBricks,
                                                 -static_cast<float>(zOffset)/zBricks);
            current.sliceIndexLlf = tgt::ivec3(static_cast<int>(xOffset*hx / sliceWidth.x), static_cast<int>(yOffset * hy / sliceWidth.y),
                                               numSlices.z - static_cast<int>(zOffset * hz / sliceWidth.z));
            current.sliceIndexUrb = tgt::ivec3(static_cast<int>((current.coordUrb.x - coordLlf.x) / sliceWidth.x),
                                               static_cast<int>((current.coordUrb.y - coordLlf.y) / sliceWidth.y),
                                               numSlices.z + static_cast<int>((current.coordUrb.z - coordLlf.z) / sliceWidth.z));
            current.translation = tgt::vec3((gx * regularExplosionGap_.get() / 2) + static_cast<float>(xOffset) * regularExplosionGap_.get(),
                                            (gy * regularExplosionGap_.get() / 2) + static_cast<float>(yOffset) * regularExplosionGap_.get(),
                                            -((gz * regularExplosionGap_.get() / 2) + static_cast<float>(zOffset) * regularExplosionGap_.get()));
            current.brickId = -1;
            current.selected = true;
            current.hidden = false;
            regularBricklist_.insert(regularBricklist_.end(), current);
        }
    }
    else if (brickingMode_.get() == "manualGrid") {
        manualBricklist_.clear();
        tgt::vec3 startExplosionGap = -1.f * tgt::vec3((numBricksXm_ - 1) * manualExplosionGap_.get() / 2,
                                              (numBricksYm_ - 1) * manualExplosionGap_.get() / 2,
                                              -1.f * (numBricksZm_ - 1) * manualExplosionGap_.get() / 2);

        for (it = 0; it < numIterations; ++it) {
            xOffset = it / static_cast<int>(numBricksYm_ * numBricksZm_);
            yOffset = (it / static_cast<int>(numBricksZm_)) % numBricksYm_;
            zOffset = it % numBricksZm_;

            current.coordLlf = coordLlf + tgt::vec3(xPlanescoord_.at(xOffset),yPlanescoord_.at(yOffset),(zPlanescoord_.at(zOffset)));
            current.coordUrb = coordLlf + tgt::vec3(xPlanescoord_.at(xOffset + 1),yPlanescoord_.at(yOffset + 1),
                                                    (zPlanescoord_.at(zOffset + 1)));
            current.texLlf = tgt::vec3(xPlanestex_.at(xOffset),yPlanestex_.at(yOffset),zPlanestex_.at(zOffset));
            current.texUrb = tgt::vec3(xPlanestex_.at(xOffset + 1),yPlanestex_.at(yOffset + 1),zPlanestex_.at(zOffset + 1));
            current.sliceIndexLlf = tgt::ivec3(static_cast<int>(xPlanescoord_.at(xOffset) / sliceWidth.x),
                                               static_cast<int>(yPlanescoord_.at(yOffset) / sliceWidth.y),
                                               numSlices.z + static_cast<int>(zPlanescoord_.at(zOffset) / sliceWidth.z));
            current.sliceIndexUrb = tgt::ivec3(static_cast<int>(xPlanescoord_.at(xOffset + 1) / sliceWidth.x),
                                               static_cast<int>(yPlanescoord_.at(yOffset + 1) / sliceWidth.y),
                                               numSlices.z+static_cast<int>(zPlanescoord_.at(zOffset + 1) / sliceWidth.z));
            current.translation = startExplosionGap +  tgt::vec3(static_cast<float>(xOffset) * manualExplosionGap_.get(),
                                                        static_cast<float>(yOffset) * manualExplosionGap_.get(),
                                                        -static_cast<float>(zOffset) * manualExplosionGap_.get());
            current.brickId = -1;
            current.selected = true;
            current.hidden = false;
            manualBricklist_.insert(manualBricklist_.end(), current);
        }
    }
}

void ExplosionProxyGeometry::createCustomBricklist(char addPlaneAxis, float addPlaneCoordOffset, float addPlaneTex) {
    if (!inportVolume_.hasData()) {
        LWARNING("No volume!");
        return;
    }

    const VolumeBase* volume = inportVolume_.getData();
    tgt::vec3 volumeSize = volume->getCubeSize();
    tgt::ivec3 numSlices = volume->getDimensions();
    tgt::vec3 sliceWidth = getSliceWidth(volume);
    tgt::vec3 coordLlf = -(volumeSize / static_cast<tgt::vec3::ElemType>(2));
    tgt::vec3 coordUrb = (volumeSize / static_cast<tgt::vec3::ElemType>(2));

    std::swap(coordLlf.z, coordUrb.z);
    const tgt::vec3 texLlf(0, 0, 1);
    const tgt::vec3 texUrb(1, 1, 0);

    Brick current;
    std::vector<Brick> newBricks;
    size_t it;
    size_t i;

    if (customBricklist_.empty()) {
        current.coordLlf = coordLlf;
        current.coordUrb = coordUrb;
        current.texLlf = texLlf;
        current.texUrb = texUrb;
        current.sliceIndexLlf = tgt::ivec3(0, 0, numSlices.z);
        current.sliceIndexUrb = tgt::ivec3(numSlices.x, numSlices.y, 0);
        current.translation = tgt::vec3(0,0,0);
        current.brickId = 0;
        current.selected = true;
        current.hidden = false;
        customBricklist_.insert(customBricklist_.begin(), current);
    }
    else{
        for (i = 0; i < selectedBricks_.size(); ++i) {
            newBricks.clear();
            if (addPlaneAxis == 'x') {
                if (selectedBricks_.at(i).coordLlf.x < (coordLlf.x + addPlaneCoordOffset) &&
                    selectedBricks_.at(i).coordUrb.x > (coordLlf.x + addPlaneCoordOffset)) {

                    customBricklist_.erase(customBricklist_.begin() + selectedBricks_.at(i).brickId);

                    current.coordLlf = selectedBricks_.at(i).coordLlf;
                    current.coordUrb = tgt::vec3(coordLlf.x + addPlaneCoordOffset, selectedBricks_.at(i).coordUrb.y,
                                                 selectedBricks_.at(i).coordUrb.z);
                    current.texLlf = selectedBricks_.at(i).texLlf;
                    current.texUrb = tgt::vec3(addPlaneTex, selectedBricks_.at(i).texUrb.y, selectedBricks_.at(i).texUrb.z);
                    current.sliceIndexLlf = selectedBricks_.at(i).sliceIndexLlf;
                    current.sliceIndexUrb = tgt::ivec3(static_cast<int>(addPlaneCoordOffset / sliceWidth.x),
                                                       selectedBricks_.at(i).sliceIndexUrb.y,
                                                       selectedBricks_.at(i).sliceIndexUrb.z);
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    current.coordLlf = tgt::vec3(coordLlf.x + addPlaneCoordOffset, selectedBricks_.at(i).coordLlf.y,
                                                 selectedBricks_.at(i).coordLlf.z);
                    current.coordUrb = selectedBricks_.at(i).coordUrb;
                    current.texLlf = tgt::vec3(addPlaneTex, selectedBricks_.at(i).texLlf.y, selectedBricks_.at(i).texLlf.z);
                    current.texUrb = selectedBricks_.at(i).texUrb;
                    current.sliceIndexLlf = tgt::ivec3(static_cast<int>(addPlaneCoordOffset / sliceWidth.x),
                                                       selectedBricks_.at(i).sliceIndexLlf.y,
                                                       selectedBricks_.at(i).sliceIndexLlf.z);
                    current.sliceIndexUrb = selectedBricks_.at(i).sliceIndexUrb;
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    for (it = 0; it < newBricks.size(); ++it) {
                        customBricklist_.insert(customBricklist_.begin() + selectedBricks_.at(i).brickId, newBricks.at(it));
                    }
                }

            }
            else if (addPlaneAxis == 'y') {
                if (selectedBricks_.at(i).coordLlf.y < (coordLlf.y + addPlaneCoordOffset) &&
                    selectedBricks_.at(i).coordUrb.y > (coordLlf.y + addPlaneCoordOffset)) {

                    customBricklist_.erase(customBricklist_.begin() + selectedBricks_.at(i).brickId);

                    current.coordLlf = selectedBricks_.at(i).coordLlf;
                    current.coordUrb = tgt::vec3(selectedBricks_.at(i).coordUrb.x, coordLlf.y + addPlaneCoordOffset,
                                                 selectedBricks_.at(i).coordUrb.z);
                    current.texLlf = selectedBricks_.at(i).texLlf;
                    current.texUrb = tgt::vec3(selectedBricks_.at(i).texUrb.x, addPlaneTex, selectedBricks_.at(i).texUrb.z);
                    current.sliceIndexLlf = selectedBricks_.at(i).sliceIndexLlf;
                    current.sliceIndexUrb = tgt::ivec3(selectedBricks_.at(i).sliceIndexUrb.x,
                                                       static_cast<int>(addPlaneCoordOffset / sliceWidth.y),
                                                       selectedBricks_.at(i).sliceIndexUrb.z);
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    current.coordLlf = tgt::vec3(selectedBricks_.at(i).coordLlf.x, coordLlf.y + addPlaneCoordOffset,
                                                 selectedBricks_.at(i).coordLlf.z);
                    current.coordUrb = selectedBricks_.at(i).coordUrb;
                    current.texLlf = tgt::vec3(selectedBricks_.at(i).texLlf.x, addPlaneTex, selectedBricks_.at(i).texLlf.z);
                    current.texUrb = selectedBricks_.at(i).texUrb;
                    current.sliceIndexLlf = tgt::ivec3(selectedBricks_.at(i).sliceIndexLlf.x,
                                                       static_cast<int>(addPlaneCoordOffset / sliceWidth.y),
                                                       selectedBricks_.at(i).sliceIndexLlf.z);
                    current.sliceIndexUrb = selectedBricks_.at(i).sliceIndexUrb;
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    for (it = 0; it < newBricks.size(); ++it) {
                        customBricklist_.insert(customBricklist_.begin() + selectedBricks_.at(i).brickId, newBricks.at(it));
                    }
                }
            }
            else{
                if (selectedBricks_.at(i).coordLlf.z > (coordLlf.z + addPlaneCoordOffset) &&
                    selectedBricks_.at(i).coordUrb.z < (coordLlf.z + addPlaneCoordOffset)) {

                    customBricklist_.erase(customBricklist_.begin() + selectedBricks_.at(i).brickId);

                    current.coordLlf = selectedBricks_.at(i).coordLlf;
                    current.coordUrb = tgt::vec3(selectedBricks_.at(i).coordUrb.x, selectedBricks_.at(i).coordUrb.y,
                                                 coordLlf.z + addPlaneCoordOffset);
                    current.texLlf = selectedBricks_.at(i).texLlf;
                    current.texUrb = tgt::vec3(selectedBricks_.at(i).texUrb.x, selectedBricks_.at(i).texUrb.y,  addPlaneTex);
                    current.sliceIndexLlf = selectedBricks_.at(i).sliceIndexLlf;
                    current.sliceIndexUrb = tgt::ivec3(selectedBricks_.at(i).sliceIndexUrb.x, selectedBricks_.at(i).sliceIndexUrb.y,
                                                       numSlices.z + static_cast<int>(addPlaneCoordOffset / sliceWidth.z));
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    current.coordLlf = tgt::vec3(selectedBricks_.at(i).coordLlf.x, selectedBricks_.at(i).coordLlf.y,
                                                 coordLlf.z + addPlaneCoordOffset);
                    current.coordUrb = selectedBricks_.at(i).coordUrb;
                    current.texLlf = tgt::vec3(selectedBricks_.at(i).texLlf.x, selectedBricks_.at(i).texLlf.y, addPlaneTex);
                    current.texUrb = selectedBricks_.at(i).texUrb;
                    current.sliceIndexLlf = tgt::ivec3(selectedBricks_.at(i).sliceIndexLlf.x, selectedBricks_.at(i).sliceIndexLlf.y,
                                                       numSlices.z + static_cast<int>(addPlaneCoordOffset / sliceWidth.y));
                    current.sliceIndexUrb = selectedBricks_.at(i).sliceIndexUrb;
                    current.translation = selectedBricks_.at(i).translation;
                    current.brickId = -1;
                    current.selected = true;
                    current.hidden = false;
                    newBricks.insert(newBricks.end(), current);

                    for (it = 0; it < newBricks.size(); ++it) {
                        customBricklist_.insert(customBricklist_.begin() + selectedBricks_.at(i).brickId, newBricks.at(it));
                    }
                }
            }
        }
    }

    registerForSelecting(customBricklist_);
    selectedBricks_.clear();

    for (i = 0; i < customBricklist_.size(); ++i) {
        customBricklist_.at(i).brickId=static_cast<int>(i);

        if (customBricklist_.at(i).selected)
            selectedBricks_.insert(selectedBricks_.begin(), customBricklist_.at(i));
    }

    customBricklistStack_.push(customBricklist_);
}

std::vector<ExplosionProxyGeometry::Brick> ExplosionProxyGeometry::sortBricklist(std::vector<Brick> unsortedBricklist) {
    std::vector<Brick> sortedBricklist;
    Brick current;
    Brick compare;
    std::vector<Brick> help;
    tgt::vec3 currentMiddle;
    tgt::vec3 compareMiddle;
    float currentDistance;
    float compareDistance;
    bool inserted = false;
    int it;
    int i;

    for (it = 0; it < loopInport_.getNumLoopIterations(); ++it) {
        current = unsortedBricklist.at(it);
        help.clear();
        help.insert(help.begin(), current);

        if (it == 0)
            sortedBricklist.insert(sortedBricklist.end(), current);
        else{
            inserted = false;
            currentMiddle = findMiddle(help);
            currentDistance = getCameraDistance(camera_.get().getPosition(), currentMiddle);
            i = 0;

            while (i < it && !inserted) {
                compare = sortedBricklist.at(i);
                help.clear();
                help.insert(help.begin(), compare);
                compareMiddle = findMiddle(help);
                compareDistance = getCameraDistance(camera_.get().getPosition(), compareMiddle);

                if (currentDistance > compareDistance) {
                    sortedBricklist.insert(sortedBricklist.begin() + i, current);
                    inserted = true;
                }
                if (i == (it - 1) && !inserted) {
                    sortedBricklist.insert(sortedBricklist.end(), current);
                }
                ++i;
            }
        }
    }
    return sortedBricklist;
}

tgt::vec3 ExplosionProxyGeometry::findMiddle(std::vector<Brick> bricklist) {
    float xMiddle;
    float yMiddle;
    float zMiddle;
    float xMin = bricklist.at(0).coordLlf.x + bricklist.at(0).translation.x;
    float xMax = bricklist.at(0).coordUrb.x + bricklist.at(0).translation.x;
    float yMin = bricklist.at(0).coordLlf.y + bricklist.at(0).translation.y;
    float yMax = bricklist.at(0).coordUrb.y + bricklist.at(0).translation.y;
    float zMin = bricklist.at(0).coordLlf.z + bricklist.at(0).translation.z;
    float zMax = bricklist.at(0).coordUrb.z + bricklist.at(0).translation.z;
    size_t i;


    for (i = 1; i < bricklist.size(); ++i) {
        if (xMin > bricklist.at(i).coordLlf.x + bricklist.at(i).translation.x)
            xMin = bricklist.at(i).coordLlf.x + bricklist.at(i).translation.x;

        if (xMax < bricklist.at(i).coordUrb.x + bricklist.at(i).translation.x)
            xMax = bricklist.at(i).coordUrb.x + bricklist.at(i).translation.x;

        if (yMin > bricklist.at(i).coordLlf.y + bricklist.at(i).translation.y)
            yMin = bricklist.at(i).coordLlf.y + bricklist.at(i).translation.y;

        if (yMax < bricklist.at(i).coordUrb.y + bricklist.at(i).translation.y)
            yMax = bricklist.at(i).coordUrb.y + bricklist.at(i).translation.y;

        if (zMin < bricklist.at(i).coordLlf.z + bricklist.at(i).translation.z)
            zMin = bricklist.at(i).coordLlf.z + bricklist.at(i).translation.z;

        if (zMax > bricklist.at(i).coordUrb.z + bricklist.at(i).translation.z)
            zMax = bricklist.at(i).coordUrb.z + bricklist.at(i).translation.z;
    }

    xMiddle = xMax - ((xMax - xMin) / 2.f);
    yMiddle = yMax - ((yMax - yMin) / 2.f);
    zMiddle = zMin - ((zMin - zMax) / 2.f);

    return tgt::vec3(xMiddle, yMiddle, zMiddle);
}

float ExplosionProxyGeometry::getCameraDistance(tgt::vec3 cameraposition, tgt::vec3 brickmiddle) {
    float distance = sqrt((cameraposition.x - brickmiddle.x) * (cameraposition.x - brickmiddle.x)
                            + (cameraposition.y - brickmiddle.y) * (cameraposition.y - brickmiddle.y)
                            +(cameraposition.z - brickmiddle.z) * (cameraposition.z - brickmiddle.z));
    return distance;
}

tgt::vec3 ExplosionProxyGeometry::getSliceWidth(const VolumeBase* volume) {
    tgt::vec3 sliceWidth;
    sliceWidth = volume->getCubeSize() / tgt::vec3(volume->getDimensions());
    return sliceWidth;
}

void ExplosionProxyGeometry::updatePlaneWidgets() {
    if (brickingMode_.get() == "regularGrid" || brickingMode_.get() == "manualGrid" || selectedBricks_.empty()) {
        tgt::ivec3 numSlices = inportVolume_.getData()->getRepresentation<VolumeRAM>()->getDimensions();

        XbrickingPlane_.setMinValue(0);
        XbrickingPlane_.set(0);
        XbrickingPlane_.setMaxValue(numSlices.x);
        XbrickingPlane_.updateWidgets();

        YbrickingPlane_.setMinValue(0);
        YbrickingPlane_.set(0);
        YbrickingPlane_.setMaxValue(numSlices.y);
        YbrickingPlane_.updateWidgets();

        ZbrickingPlane_.setMinValue(0);
        ZbrickingPlane_.set(0);
        ZbrickingPlane_.setMaxValue(numSlices.z);
        ZbrickingPlane_.updateWidgets();
    }
    else{
        XbrickingPlane_.setMinValue(findMinSlice(selectedBricks_, 'x'));
        XbrickingPlane_.set(XbrickingPlane_.getMinValue());
        XbrickingPlane_.setMaxValue(findMaxSlice(selectedBricks_, 'x'));
        XbrickingPlane_.updateWidgets();
        YbrickingPlane_.setMinValue(findMinSlice(selectedBricks_, 'y'));
        YbrickingPlane_.set(YbrickingPlane_.getMinValue());
        YbrickingPlane_.setMaxValue(findMaxSlice(selectedBricks_, 'y'));
        YbrickingPlane_.updateWidgets();
        ZbrickingPlane_.setMinValue(findMaxSlice(selectedBricks_, 'z'));
        ZbrickingPlane_.set(ZbrickingPlane_.getMinValue());
        ZbrickingPlane_.setMaxValue(findMinSlice(selectedBricks_, 'z'));
        ZbrickingPlane_.updateWidgets();
    }
}

int ExplosionProxyGeometry::findMinSlice(std::vector<Brick> bricklist, char axis) {
    tgt::ivec3 sliceIndex = bricklist.at(0).sliceIndexLlf;
    size_t i;

    if (axis == 'x') {
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.x > bricklist.at(i).sliceIndexLlf.x)
                    sliceIndex.x = bricklist.at(i).sliceIndexLlf.x;
        }
        return sliceIndex.x;
    }
    if (axis == 'y') {
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.y > bricklist.at(i).sliceIndexLlf.y)
                    sliceIndex.y = bricklist.at(i).sliceIndexLlf.y;
        }
        return sliceIndex.y;
    }
    else{
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.z < bricklist.at(i).sliceIndexLlf.z)
                    sliceIndex.z = bricklist.at(i).sliceIndexLlf.z;
        }
        return sliceIndex.z;
    }
}

int ExplosionProxyGeometry::findMaxSlice(std::vector<Brick> bricklist, char axis) {
    tgt::ivec3 sliceIndex = bricklist.at(0).sliceIndexUrb;
    size_t i;

    if (axis == 'x') {
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.x < bricklist.at(i).sliceIndexUrb.x)
                    sliceIndex.x = bricklist.at(i).sliceIndexUrb.x;
        }
        return sliceIndex.x;
    }
    if (axis == 'y') {
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.y < bricklist.at(i).sliceIndexUrb.y)
                    sliceIndex.y = bricklist.at(i).sliceIndexUrb.y;
        }
        return sliceIndex.y;
    }
    else{
        for (i = 1; i < bricklist.size(); ++i) {
            if (sliceIndex.z > bricklist.at(i).sliceIndexUrb.z)
                    sliceIndex.z = bricklist.at(i).sliceIndexUrb.z;
        }
        return sliceIndex.z;
    }
}

void ExplosionProxyGeometry::registerForSelecting(std::vector<Brick> bricklist) {
    size_t i;

    idManager_.clearRegisteredObjects();
    idManager_.activateTarget(getName());
    idManager_.clearTarget();

    for (i = 0; i < bricklist.size(); ++i) {
        idManager_.registerObject((void *)(i+1));
    }

    // set modelview and projection matrices
    glMatrixMode(GL_PROJECTION);
    tgt::loadMatrix(camera_.get().getProjectionMatrix(pickingBuffer_.getSize()));
    glMatrixMode(GL_MODELVIEW);
    tgt::loadMatrix(camera_.get().getViewMatrix());
    LGL_ERROR;

    for (i = 0; i < bricklist.size(); ++i) {
        tgt::col4 color = idManager_.getColorFromObject((void *)(i + 1));
        MeshGeometry selectingBrick = MeshGeometry::createCube(
                                                        bricklist.at(i).coordLlf + bricklist.at(i).translation,
                                                        bricklist.at(i).coordUrb + bricklist.at(i).translation,
                                                        bricklist.at(i).texLlf,
                                                        bricklist.at(i).texUrb,
                                                        tgt::vec3(color.xyz()) / 255.f,
                                                        tgt::vec3(color.xyz()) / 255.f);
        selectingBrick.render();
    }

    // restore matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    LGL_ERROR;
}

void ExplosionProxyGeometry::onSelectingOneBrickEvent(tgt::MouseEvent * e) {
    if (!pickingBuffer_.hasRenderTarget()) {
        return;
    }
    registerForSelecting(customBricklist_);

    e->accept();
    size_t i;
    tgt::ivec2 pos = tgt::ivec2(e->coord().x, canvasSize_.get().y - e->coord().y);
    const void* brick = idManager_.getObjectAtPos(pos);
    if (brick != 0) {
        int brickIndex = static_cast<int>(reinterpret_cast<size_t>(brick)) - 1;
        if  (!customBricklist_.at(brickIndex).hidden) {
            customBricklistStack_.push(customBricklist_);

            for (i = 0; i < selectedBricks_.size(); ++i) {
                customBricklist_.at(selectedBricks_.at(i).brickId).selected = false;
            }

            customBricklist_.at(brickIndex).selected = true;
            selectedBricks_.clear();
            selectedBricks_.insert(selectedBricks_.begin(), customBricklist_.at(brickIndex));
            translation_.set(customBricklist_.at(brickIndex).translation);
            updatePlaneWidgets();
            invalidate();
        }
    }
    else{
        customBricklistStack_.push(customBricklist_);
        for (i = 0; i < selectedBricks_.size(); ++i) {
            customBricklist_.at(selectedBricks_.at(i).brickId).selected = false;
        }
        selectedBricks_.clear();
        translation_.set(tgt::vec3(0.f, 0.f, 0.f));
        updatePlaneWidgets();
        invalidate();
    }
}

void ExplosionProxyGeometry::onSelectingSeveralBricksEvent(tgt::MouseEvent * e) {
    if (!pickingBuffer_.hasRenderTarget()) {
        return;
    }
    registerForSelecting(customBricklist_);

    size_t i;
    e->accept();
    tgt::ivec2 pos = tgt::ivec2(e->coord().x, canvasSize_.get().y - e->coord().y);
    const void* brick = idManager_.getObjectAtPos(pos);
    if (brick != 0) {
        int brickIndex = static_cast<int>(reinterpret_cast<size_t>(brick)) - 1;
        if (!customBricklist_.at(brickIndex).hidden) {
            customBricklistStack_.push(customBricklist_);

            if (!customBricklist_.at(brickIndex).selected) {
                customBricklist_.at(brickIndex).selected = true;
                i = 0;
                bool inserted = false;
                while(i < selectedBricks_.size() && !inserted) {
                    if (customBricklist_.at(brickIndex).brickId > selectedBricks_.at(i).brickId) {
                        selectedBricks_.insert(selectedBricks_.begin() + i, customBricklist_.at(brickIndex));
                        inserted = true;
                    }
                    else{
                    ++i;
                    }
                }
                if (!inserted) {
                    selectedBricks_.insert(selectedBricks_.end(), customBricklist_.at(brickIndex));
                }
            }
            else{
                if (selectedBricks_.size() == 1) {
                    for (i = 0; i < customBricklist_.size(); ++i) {
                        customBricklist_.at(i).selected = true;
                    }
                    selectedBricks_.clear();
                    selectedBricks_ = customBricklist_;
                }
                else{
                    customBricklist_.at(brickIndex).selected = false;
                    i = 0;
                    bool del = false;
                    while(!del) {
                        if (selectedBricks_.at(i).brickId == brickIndex) {
                            selectedBricks_.erase(selectedBricks_.begin() + i);
                            del = true;
                        }
                        else{
                        ++i;
                        }
                    }
                }
            }
            if (selectedBricks_.size() == 1)
                translation_.set(selectedBricks_.at(0).translation);

            else
                translation_.set(findMiddle(selectedBricks_));

            updatePlaneWidgets();
            invalidate();
        }
    }
    else{
        customBricklistStack_.push(customBricklist_);
        for (i = 0; i < selectedBricks_.size(); ++i) {
            customBricklist_.at(selectedBricks_.at(i).brickId).selected = false;
        }
        selectedBricks_.clear();
        translation_.set(tgt::vec3(0.f, 0.f, 0.f));
        updatePlaneWidgets();
        invalidate();
    }
}

void ExplosionProxyGeometry::onTranslateSelectedBricksEvent(tgt::MouseEvent * e) {
    if (!pickingBuffer_.hasRenderTarget()) {
        return;
    }

    tgt::ivec2 pos = tgt::ivec2(e->coord().x, canvasSize_.get().y - e->coord().y);

    if  (e->action() & tgt::MouseEvent::PRESSED) {
        const void* brick = idManager_.getObjectAtPos(pos);
            if (brick != 0) {
                customBricklistStack_.push(customBricklist_);
                int brickIndex = static_cast<int>(reinterpret_cast<size_t>(brick)) - 1;

                if (customBricklist_.at(brickIndex).selected) {
                    e->accept();
                    isClicked_ = true;
                    startCoord_.x = e->coord().x;
                    startCoord_.y = e->coord().y;
                    startPos_ = translation_.get();
                }
            }
        return;
    }

    if (e->action() & tgt::MouseEvent::MOTION) {
        if (isClicked_) {
            e->accept();

            LGL_ERROR;
            GLint deltaX, deltaY;

            GLint viewport[4];
            GLdouble modelview[16];
            GLdouble projection[16];
            GLdouble winX, winY, winZ;
            GLdouble posX, posY, posZ;

            deltaX = e->coord().x - startCoord_.x;
            deltaY = startCoord_.y - e->coord().y;

            tgt::mat4 projection_tgt = camera_.get().getProjectionMatrix(idManager_.getRenderTarget()->getSize());
            tgt::mat4 modelview_tgt = camera_.get().getViewMatrix();
            for (int i = 0; i < 4; ++i) {
                modelview[i+0]   = modelview_tgt[i].x;
                modelview[i+4]   = modelview_tgt[i].y;
                modelview[i+8]   = modelview_tgt[i].z;
                modelview[i+12]  = modelview_tgt[i].w;
                projection[i+0]  = projection_tgt[i].x;
                projection[i+4]  = projection_tgt[i].y;
                projection[i+8]  = projection_tgt[i].z;
                projection[i+12] = projection_tgt[i].w;
            }

            viewport[0] = 0;
            viewport[1] = 0;
            viewport[2] = static_cast<GLint>(idManager_.getRenderTarget()->getSize().x);
            viewport[3] = static_cast<GLint>(idManager_.getRenderTarget()->getSize().y);

            posX = startPos_.x;
            posY = startPos_.y;
            posZ = startPos_.z;

            LGL_ERROR;
            gluProject(posX, posY,posZ,modelview,projection, viewport,&winX, &winY, &winZ);

            winX = winX + deltaX;
            winY = winY + deltaY;

            LGL_ERROR;
            gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
            LGL_ERROR;

            translation_.set(tgt::vec3(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ)));

            LGL_ERROR;
            invalidate();
            LGL_ERROR;
        }
        return;
    }

    if (e->action() & tgt::MouseEvent::RELEASED) {
        if (isClicked_) {
            e->accept();
            isClicked_ = false;
            registerForSelecting(customBricklist_);
            invalidate();
        }
        return;
    }
}

void ExplosionProxyGeometry::canvasSizeChanged() {
    //RenderProcessor::portResized(0, canvasSize_.get());
}


} // namespace
