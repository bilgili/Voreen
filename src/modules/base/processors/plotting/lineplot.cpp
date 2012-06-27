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

#include "voreen/modules/base/processors/plotting/lineplot.h"

namespace voreen {

const std::string LinePlot::loggerCat_("voreen.LinePlot");

LinePlot::LinePlot()
    : PlotProcessor(PlotEntitySettings::LINE, false)
    , lineWidth_("lineWidth", "Line Width", 2.f, 1.f, 5.f)
    , pointSize_("pointSize", "Point Size", 2.f, 1.f, 9.f)
    , axesWidth_("axesWidth", "Axes Width", 1.f, 1.f, 5.f)
    , xLabel_("xLabel", "x Axis Label", "x axis")
    , yLabel_("yLabel", "y Axis Label", "y axis")
    , xScaleStep_("xScaleStep", "Minimum x Scale Step (pixels)", 40, 32, 256)
    , yScaleStep_("yScaleStep", "Minimum y Scale Step (pixels)", 40, 32, 256)
    , logXAxis_("logXAxes", "Logarithmic x Axis", false)
    , logYAxis_("logYAxes", "Logarithmic y Axis", false)
    , renderLineLabel_("renderLineLabel", "Line Labels", false)
    , renderAxes_("renderAxes", "Render Axes", true)
    , renderScales_("renderScales", "Render Scales", false)
    , renderXHelperLines_("renderXHelperLines", "Show x Helper Lines", true)
    , renderYHelperLines_("renderYHelperLines", "Show y Helper Lines", false)
    , fixZoom_("fixZoom", "Fix Zoom", false)
    , regenDataList_(true)
    , regenPickingList_(true)
    , dataList_(0)
    , pickingList_(0)
{
    plotEntitiesProp_.setGuiName("Line Selection");
    addProperty(dataProp_);
    addProperty(plotEntitiesProp_);
    addProperty(plotLabel_);
    addProperty(xLabel_);
    addProperty(yLabel_);
    addProperty(renderAxes_);
    addProperty(renderScales_);
    addProperty(renderXHelperLines_);
    addProperty(renderYHelperLines_);
    addProperty(enablePicking_);
    addProperty(logXAxis_);
    addProperty(logYAxis_);
    addProperty(renderLineLabel_);
    addProperty(renderMousePosition_);
    addProperty(lineWidth_);
    addProperty(pointSize_);
    addProperty(axesWidth_);
    addProperty(xScaleStep_);
    addProperty(yScaleStep_);
    addProperty(marginLeft_);
    addProperty(marginRight_);
    addProperty(marginBottom_);
    addProperty(marginTop_);
    addProperty(discreteStep_);
    addProperty(fixZoom_);
    addProperty(zoomProp_);
    addProperty(zoomPlaneColor_);

    addEventProperty(mouseEventClickLeft_);
    addEventProperty(mouseEventClickLeftCtrl_);
    addEventProperty(eventZoomOut_);
    addEventProperty(mouseEventMoveLeft_);
    addEventProperty(mouseEventMoveRight_);
    addEventProperty(mousePositionUpdateEvent_);
    addEventProperty(mouseEventEnterExit_);

    //if one of the following properties is changed we handle it like plot entities property is changed
    logXAxis_.onChange       (CallMemberAction<LinePlot>(this, &LinePlot::entitiesPropChanged));
    logYAxis_.onChange       (CallMemberAction<LinePlot>(this, &LinePlot::entitiesPropChanged));
    renderLineLabel_.onChange(CallMemberAction<LinePlot>(this, &LinePlot::entitiesPropChanged));
    lineWidth_.onChange      (CallMemberAction<LinePlot>(this, &LinePlot::entitiesPropChanged));
    pointSize_.onChange      (CallMemberAction<LinePlot>(this, &LinePlot::entitiesPropChanged));
}

Processor* LinePlot::create() const {
    return new LinePlot();
}

std::string LinePlot::getProcessorInfo() const {
    return std::string("This processor is able to plot lines and candlesticks.");
}

void LinePlot::initialize() throw (VoreenException) {
    PlotProcessor::initialize();
    dataList_ = glGenLists(1);
    pickingList_ = glGenLists(1);
}

void LinePlot::deinitialize() throw (VoreenException) {
    if (dataList_) {
        glDeleteLists(dataList_, 1);
        dataList_ = 0;
    }
    if (pickingList_) {
        glDeleteLists(pickingList_, 1);
        pickingList_ = 0;
    }

    PlotProcessor::deinitialize();
}

void LinePlot::leftClickEvent(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    if (e->action() == tgt::MouseEvent::PRESSED) {
        mousePressedPosition_ = mousePosition_;
        leftMousePressed_ = true;
    }
    else if (e->action() == tgt::MouseEvent::RELEASED) {
        leftMousePressed_ = false;
        if (!(e->modifiers() & tgt::Event::CTRL))
            zoomIn2D(mousePressedPosition_, mousePosition_);
        if (pickingBuffer_.hasRenderTarget() && tgt::length(mousePosition_ - mousePressedPosition_) < 4) {
            // handle PlotCell selection
            tgt::ivec2 cell = plotPickingManager_.getCellAtPos(tgt::ivec2(e->x(), e->viewport().y-e->y()));
            data_.toggleHighlight(cell, e->modifiers() & tgt::Event::CTRL);
            regenDataList_ = true;
            regenPickingList_ = true;
        }
    }
    e->accept();
    invalidate(INVALID_RESULT);
}

void LinePlot::render() {
    outport_.activateTarget();
    plotLib_.setUsePlotPickingManager(false);
    setPlotStatus();
    plotLib_.resetLineLabels();
    plotLib_.resetPlotLabels();
    if (plotLib_.setOpenGLStatus()) {
        renderAxes();
        if (regenDataList_) {
            glNewList(dataList_, GL_COMPILE);
                renderData();
            glEndList();
            regenDataList_ = false;
        }
        glCallList(dataList_);
        plotLib_.renderLineLabels();
        plotLib_.renderPlotLabels();
        renderPlotLabel();
        renderMousePosition();
    }
    plotLib_.resetOpenGLStatus();
    renderSelectedRegion(mousePressedPosition_, mousePosition_);
    outport_.deactivateTarget();
    plotPickingManager_.activateTarget();
    plotPickingManager_.clearTarget();
    if (enablePicking_.get()) {
        plotLib_.setUsePlotPickingManager(true);
        if (plotLib_.setOpenGLStatus() && regenPickingList_) {
            glNewList(pickingList_, GL_COMPILE);
                renderData();
            glEndList();
            regenPickingList_ = false;
        }
        glCallList(pickingList_);
        plotLib_.resetOpenGLStatus();
    }
    plotPickingManager_.deactivateTarget();
}

void LinePlot::setPlotStatus() {
    plotLib_.setWindowSize(outport_.getSize());
    plotLib_.setLogarithmicAxis(logXAxis_.get(), PlotLibrary::X_AXIS);
    plotLib_.setLogarithmicAxis(logYAxis_.get(), PlotLibrary::Y_AXIS);
    plotLib_.setRenderDataLabelFlag(renderLineLabel_.get());
    plotLib_.setAxesWidth(axesWidth_.get());
    plotLib_.setDrawingColor(tgt::Color::black);
    plotLib_.setLineWidth(lineWidth_.get());
    plotLib_.setMaxGlyphSize(pointSize_.get());
    plotLib_.setMarginBottom(marginBottom_.get());
    plotLib_.setMarginTop(marginTop_.get());
    plotLib_.setMarginLeft(marginLeft_.get());
    plotLib_.setMarginRight(marginRight_.get());
    plotLib_.setMinimumScaleStep(xScaleStep_.get(), PlotLibrary::X_AXIS);
    plotLib_.setMinimumScaleStep(yScaleStep_.get(), PlotLibrary::Y_AXIS);
    plotLib_.setDomain(zoomProp_.top().xZoom_, PlotLibrary::X_AXIS);
    plotLib_.setDomain(zoomProp_.top().yZoom_, PlotLibrary::Y_AXIS);
}

void LinePlot::renderData() {
    // set font size for line labels
    plotLib_.setFontSize(12);
    // iterate through plot entity settings
    std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
    for (; it < plotEntitiesProp_.get().end(); ++it) {
        // handle candle stick
        if (it->getCandleStickFlag()) {
            plotLib_.setDrawingColor(it->getFirstColor());
            plotLib_.setFillColor(it->getSecondColor());
            plotLib_.renderCandlesticks(data_, plotEntitiesProp_.getXColumnIndex(), it->getStickTopColumnIndex(),
                it->getStickBottomColumnIndex(), it->getCandleTopColumnIndex(), it->getCandleBottomColumnIndex());
        }
        // handle line
        else {
            // render errorline / errorbar
            if (it->getOptionalColumnIndex() != -1) {
                plotLib_.setDrawingColor(it->getSecondColor());
                if (it->getErrorbarFlag())
                    plotLib_.renderErrorbars(data_, plotEntitiesProp_.getXColumnIndex(),
                            it->getMainColumnIndex(), it->getOptionalColumnIndex());
                else {
                    tgt::Color c = it->getSecondColor();
                    c.a = 0.5;
                    plotLib_.setFillColor(c);
                    if (it->getSplineFlag())
                        plotLib_.renderErrorspline(data_, plotEntitiesProp_.getXColumnIndex(),
                                it->getMainColumnIndex(), it->getOptionalColumnIndex());
                    else
                        plotLib_.renderErrorline(data_, plotEntitiesProp_.getXColumnIndex(),
                                it->getMainColumnIndex(), it->getOptionalColumnIndex());
                }
            }
            plotLib_.setDrawingColor(it->getFirstColor());
            plotLib_.setFontColor(it->getFirstColor());
            plotLib_.setLineStyle(it->getLineStyle());
            if (it->getSplineFlag())
                plotLib_.renderSpline(data_, plotEntitiesProp_.getXColumnIndex(), it->getMainColumnIndex());
            else
                plotLib_.renderLine(data_, plotEntitiesProp_.getXColumnIndex(), it->getMainColumnIndex());
        }
    }
    LGL_ERROR;
}

void LinePlot::renderAxes() {
    if (renderAxes_.get()) {
        plotLib_.setDrawingColor(tgt::Color::black);
        plotLib_.renderAxes();
        plotLib_.setDrawingColor(tgt::Color(0, 0, 0, .5f));
        if (renderScales_.get()) {
            plotLib_.setFontSize(10);
            plotLib_.setFontColor(tgt::Color::black);
            if (data_.getColumnType(plotEntitiesProp_.getXColumnIndex()) == PlotBase::STRING)
                plotLib_.renderAxisLabelScales(data_, plotEntitiesProp_.getXColumnIndex(), renderXHelperLines_.get());
            else
                plotLib_.renderAxisScales(PlotLibrary::X_AXIS, renderXHelperLines_.get());
            plotLib_.renderAxisScales(PlotLibrary::Y_AXIS, renderYHelperLines_.get());
            plotLib_.setFontSize(12);
            plotLib_.renderAxisLabel(PlotLibrary::X_AXIS, xLabel_.get());
            plotLib_.renderAxisLabel(PlotLibrary::Y_AXIS, yLabel_.get());
        }
    }
}

void LinePlot::readFromInport() {
    // create local copy of data and assign it to property
    if (dynamic_cast<PlotData*>(inport_.getData())) {
        data_ = *dynamic_cast<PlotData*>(inport_.getData());
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        plotPickingManager_.setColumnCount(data_.getColumnCount());
        if (zoomProp_.isDeserialized_)
            zoomProp_.isDeserialized_ = false;
        inportHasPlotFunction_ = false;
        discreteStep_.setVisible(false);
        fixZoom_.setVisible(true);
    }
    else if (dynamic_cast<PlotFunction*>(inport_.getData())) {
        //check if it is a function R->R
        if (inport_.getData()->getKeyColumnCount() == 1)  {
            function_ = *dynamic_cast<PlotFunction*>(inport_.getData());
            inportHasPlotFunction_ = true;
            //if a function is set, the zoom must be fixed i.e. totally controlled by the property
            //(we do -not- want to call calcDomains for functions)
            fixZoom_.set(true);
            fixZoom_.setVisible(false);
            selectDataFromFunction();
            discreteStep_.setVisible(true);
        }
    }
    else {
        data_ = PlotData(0,0);
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
    }
}

void LinePlot::calcDomains() {
    if (plotEntitiesProp_.dataValid() && !fixZoom_.get()) {
        Interval<plot_t> xDomain = data_.getInterval(plotEntitiesProp_.getXColumnIndex());
        Interval<plot_t> yDomain = Interval<plot_t>();
        std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
        for (; it < plotEntitiesProp_.get().end(); ++it) {
            if (!it->getCandleStickFlag()) {
                Interval<plot_t> lineDomain = data_.getInterval(it->getMainColumnIndex());
                if (it->getOptionalColumnIndex() != -1) {
                    Interval<plot_t> errorDomain = data_.getInterval(it->getOptionalColumnIndex());
                    plot_t error = std::max(abs(errorDomain.getLeft()), abs(errorDomain.getRight()));
                    lineDomain = Interval<plot_t>(lineDomain.getLeft()-error, lineDomain.getRight()+error);
                }
                yDomain = yDomain.unionWith(lineDomain);
            }
            else {
                yDomain = yDomain.unionWith(data_.getInterval(it->getStickTopColumnIndex()));
                yDomain = yDomain.unionWith(data_.getInterval(it->getStickBottomColumnIndex()));
                yDomain = yDomain.unionWith(data_.getInterval(it->getCandleTopColumnIndex()));
                yDomain = yDomain.unionWith(data_.getInterval(it->getCandleBottomColumnIndex()));
            }
        }
        yDomain = yDomain.enlarge(1.1);
        //if there is a new domain
        if (! zoomProp_.isDeserialized_) {
            zoomProp_.clear();
            zoomProp_.push(PlotZoomState(xDomain, yDomain));
        }
    }
}

void LinePlot::zoomPropChanged() {
    regenDataList_ = true;
    regenPickingList_ = true;
}

void LinePlot::entitiesPropChanged() {
    regenDataList_ = true;
    regenPickingList_ = true;
}
void LinePlot::toggleProperties() {
    if (plotEntitiesProp_.dataValid()) {
        if (data_.getColumnType(plotEntitiesProp_.getXColumnIndex()) == PlotBase::STRING) {
            xScaleStep_.setVisible(false);
            logXAxis_.set(false);
            logXAxis_.setVisible(false);
        }
        else {
            xScaleStep_.setVisible(true);
            logXAxis_.setVisible(true);
        }
    }
}


} // namespace voreen
