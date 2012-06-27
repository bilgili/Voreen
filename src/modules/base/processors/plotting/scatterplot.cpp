/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/plotting/scatterplot.h"

#include "voreen/core/interaction/plotcamerainteractionhandler.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

namespace voreen {

const std::string ScatterPlot::loggerCat_("voreen.ScatterPlot");

ScatterPlot::ScatterPlot()
    : PlotProcessor(PlotEntitySettings::SCATTER, false)
    , lighting_("lighting", "Enable Lighting", false)
{
    plotEntitiesProp_.setGuiName("Scatter Data");
    addProperty(zLabel_);
    addProperty(selectionPlaneColor_);
    addProperty(renderMousePosition_);
    addProperty(renderLegend_);
    addProperty(orthographicCamera_);
    addProperty(lighting_);
    addProperty(renderXHelperLines_);
    addProperty(renderYHelperLines_);
    addProperty(renderZHelperLines_);
    addProperty(xScaleStep_);
    addProperty(yScaleStep_);
    addProperty(zScaleStep_);
    addProperty(marginLeft_);
    addProperty(marginRight_);
    addProperty(marginBottom_);
    addProperty(marginTop_);
    addProperty(camera_);

    // group properties
    lighting_.setGroupID("general");

    addEventProperty(eventHighlight_);
    addEventProperty(eventLabel_);
    addEventProperty(eventZoom_);
    addEventProperty(eventHighlightAdditive_);
    addEventProperty(eventLabelAdditive_);
    addEventProperty(eventZoomAdditive_);
    addEventProperty(mousePositionUpdateEvent_);
    addEventProperty(mouseEventEnterExit_);

    addInteractionHandler(cameraHandler_);
    addInteractionHandler(plotCameraHandler_);
    orthographicCamera_.onChange(CallMemberAction<ScatterPlot>(this, &ScatterPlot::toggleProperties));
}

std::string ScatterPlot::getProcessorInfo() const {
    return std::string("This processor is able to create 2D and 3D Scatterplots.");
}

void ScatterPlot::render() {
    outport_.activateTarget();
    plotLib_.setUsePlotPickingManager(false);
    setPlotStatus();
    if (plotLib_.setOpenGLStatus()) {
        renderAxes();
        renderData();
        renderPlotLabel();
        createPlotLabels();
        plotLib_.renderPlotLabels();
        renderMousePosition();
        renderLegends();
        if (threeDimensional_)
            renderSelectionPlanes();
    }
    plotLib_.resetOpenGLStatus();
    if (!threeDimensional_)
        renderSelectedRegion();
    outport_.deactivateTarget();
    plotPickingManager_.activateTarget();
    plotPickingManager_.clearTarget();
    if (enablePicking_.get()) {
        plotLib_.setUsePlotPickingManager(true);
        if (plotLib_.setOpenGLStatus())
            renderData();
        plotLib_.resetOpenGLStatus();
    }
    plotPickingManager_.deactivateTarget();
}

void ScatterPlot::setPlotStatus(){
    plotLib_.setWindowSize(outport_.getSize());
    plotLib_.setAxesWidth(axesWidth_.get());
    plotLib_.setCamera(camera_.get());
    plotLib_.setMinimumScaleStep(xScaleStep_.get(), PlotLibrary::X_AXIS);
    plotLib_.setMinimumScaleStep(yScaleStep_.get(), PlotLibrary::Y_AXIS);
    plotLib_.setMinimumScaleStep(zScaleStep_.get(), PlotLibrary::Z_AXIS);
    plotLib_.setMarginBottom(marginBottom_.get());
    plotLib_.setMarginTop(marginTop_.get());
    plotLib_.setMarginLeft(marginLeft_.get());
    plotLib_.setMarginRight(marginRight_.get());
    plotLib_.setDimension(threeDimensional_ ? PlotLibrary::THREE : PlotLibrary::TWO);
    plotLib_.setLightingFlag(lighting_.get());
    plotLib_.setOrthographicCameraFlag(orthographicCamera_.get());
    plotLib_.setDomain(selectionProp_.getZoom().xZoom_, PlotLibrary::X_AXIS);
    plotLib_.setDomain(selectionProp_.getZoom().yZoom_, PlotLibrary::Y_AXIS);
    plotLib_.setDomain(selectionProp_.getZoom().zZoom_, PlotLibrary::Z_AXIS);
}

void ScatterPlot::renderData() {
    plotLib_.setHighlightColor(highlightColor_.get());
    std::vector<PlotEntitySettings>::const_iterator it;
    for (it = plotEntitiesProp_.get().begin(); it < plotEntitiesProp_.get().end(); ++it) {
        plotLib_.setDrawingColor(it->getFirstColor());
        plotLib_.setColorMap(it->getColorMap());
        plotLib_.setMinGlyphSize(it->getMinGlyphSize());
        plotLib_.setMaxGlyphSize(it->getMaxGlyphSize());
        plotLib_.setGlyphStyle(it->getGlyphStyle());
        if (it->getUseTextureFlag())
            loadTextures(it->getTexturePath());
        else
            loadTextures("");
        threeDimensional_ ?
            plotLib_.renderScatter(data_, plotEntitiesProp_.getXColumnIndex(),
                                        plotEntitiesProp_.getYColumnIndex(),
                                        it->getMainColumnIndex(),
                                        it->getOptionalColumnIndex(),
                                        it->getSecondOptionalColumnIndex())
          : plotLib_.renderScatter(data_, plotEntitiesProp_.getXColumnIndex(),
                                        it->getMainColumnIndex(),
                                        -1,
                                        it->getOptionalColumnIndex(),
                                        it->getSecondOptionalColumnIndex());
    }
    LGL_ERROR;
}

void ScatterPlot::renderAxes() {
    //render axes
    if (renderAxes_.get()) {
        plotLib_.setDrawingColor(tgt::Color::black);
        plotLib_.setFontColor(tgt::Color::black);
        plotLib_.renderAxes();
        plotLib_.setFontSize(10);
        if (renderScales_.get()) {
            if (threeDimensional_) {
                plotLib_.renderAxisScales(PlotLibrary::X_AXIS, renderXHelperLines_.get(), xLabel_.get());
                plotLib_.renderAxisScales(PlotLibrary::Y_AXIS, renderYHelperLines_.get(), yLabel_.get());
                plotLib_.renderAxisScales(PlotLibrary::Z_AXIS, renderZHelperLines_.get(), zLabel_.get());
            }
            else {
                plotLib_.renderAxisScales(PlotLibrary::X_AXIS, renderXHelperLines_.get());
                plotLib_.renderAxisScales(PlotLibrary::Y_AXIS, renderYHelperLines_.get());
                plotLib_.renderAxisLabel(PlotLibrary::X_AXIS, xLabel_.get());
                plotLib_.renderAxisLabel(PlotLibrary::Y_AXIS, yLabel_.get());
            }
        }
    }
}

void ScatterPlot::readFromInport() {
    if (dynamic_cast<PlotData*>(inport_.getData())) {
        data_ = *dynamic_cast<PlotData*>(inport_.getData());
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        selectionProp_.setPlotData(&data_);
        plotPickingManager_.setColumnCount(data_.getColumnCount());
    }
    else {
        LWARNINGC("ScatterPlot", "ScatterPlot can only handle PlotData objects");
        data_ = PlotData(0,0);
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        selectionProp_.setPlotData(&data_);
        plotPickingManager_.setColumnCount(data_.getColumnCount());
    }
}

void ScatterPlot::calcDomains() {
    if (plotEntitiesProp_.dataValid()) {
        Interval<plot_t> xDomain = data_.getInterval(plotEntitiesProp_.getXColumnIndex());
        Interval<plot_t> yDomain;
        Interval<plot_t> zDomain;
        if (threeDimensional_) {
            yDomain = data_.getInterval(plotEntitiesProp_.getYColumnIndex());
            //get z domain
            std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
            for (; it < plotEntitiesProp_.get().end(); ++it)
                zDomain.unionWith(data_.getInterval(it->getMainColumnIndex()));
        }
        else {
            std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
            for (; it < plotEntitiesProp_.get().end(); ++it)
                yDomain.unionWith(data_.getInterval(it->getMainColumnIndex()));
        }
        selectionProp_.setBaseZoomState(PlotZoomState(xDomain, yDomain, zDomain));
    }
}

void ScatterPlot::toggleProperties() {
    threeDimensional_ = plotEntitiesProp_.getYColumnIndex() != -1;
    marginLeft_.setVisible(!threeDimensional_);
    marginRight_.setVisible(!threeDimensional_);
    marginBottom_.setVisible(!threeDimensional_);
    marginTop_.setVisible(!threeDimensional_);
    setPropertyGroupVisible("margins", !threeDimensional_);
    cameraHandler_->setVisible(threeDimensional_ && !orthographicCamera_.get());
    cameraHandler_->setEnabled(threeDimensional_ && !orthographicCamera_.get());
    plotCameraHandler_->setVisible(threeDimensional_ && orthographicCamera_.get());
    plotCameraHandler_->setEnabled(threeDimensional_ && orthographicCamera_.get());
    zLabel_.setVisible(threeDimensional_);
    renderZHelperLines_.setVisible(threeDimensional_);
    zScaleStep_.setVisible(threeDimensional_);
    selectionProp_.setDimension(threeDimensional_);
    orthographicCamera_.setVisible(threeDimensional_);
    lighting_.setVisible(threeDimensional_);
    lighting_.set(lighting_.get() && threeDimensional_);
    if (orthographicCamera_.get())
        camera_.set(tgt::Camera(tgt::normalize(camera_.get().getPosition()), tgt::vec3(0,0,0), tgt::vec3(0,0,1)));
}

#ifdef VRN_WITH_DEVIL
void ScatterPlot::loadTextures(std::string path) {
    // try loading textures
    if (!path.empty())
        plotLib_.setTexture(TexMgr.load(path));
    else
        plotLib_.setTexture(0);
    LGL_ERROR;
}
#else
void ScatterPlot::loadTextures(std::string /*path*/) {
}
#endif //VRN_WITH_DEVIL

} // namespace voreen
