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

#include "voreen/modules/base/processors/plotting/graphplot.h"
#include "voreen/core/plotting/plotrow.h"


namespace voreen {

const std::string GraphPlot::loggerCat_("voreen.GraphPlot");

GraphPlot::GraphPlot()
    : PlotProcessor(PlotEntitySettings::NONE, false)
    , connectionPort_(Port::INPORT, "connectionPort")
    , spring_("spring", "Spring", 1, 0, 32)
    , charge_("charge", "Charge", 1, 0, 32)
    , damping_("damping", "Damping", .5, 0, 1)
    , threshold_("threshold", "Equilibrium Threshold", .5, 0, 10)
    , applyForces_("applyForces", "Apply Forces", Processor::INVALID_RESULT)
    , applyForcesStep_("applyForcesStep", "Apply Forces Once", Processor::INVALID_RESULT)
{
    plotEntitiesProp_.setGuiName("No Data");
    addPort(connectionPort_);
    addProperty(spring_);
    addProperty(charge_);
    addProperty(damping_);
    addProperty(threshold_);
    addProperty(applyForces_);
    addProperty(applyForcesStep_);

    applyForces_.onChange(CallMemberAction<GraphPlot>(this, &GraphPlot::applyForces));
    applyForcesStep_.onChange(CallMemberAction<GraphPlot>(this, &GraphPlot::applyForcesStep));
}

std::string GraphPlot::getProcessorInfo() const {
    return std::string("This processor is renders simple 2D graphs and applies force-directed layout algorithms to them.");
}

void GraphPlot::convertPlotDataToNodeGraph(const PlotData* nodes, const PlotData* connections) {
    nodeGraph_.clearNodes();
    if (nodes == 0 || connections == 0)
        return;

    const int idCol    = 0;
    const int xCol     = 1;
    const int yCol     = 2;
    const int massCol  = 3;
    const int conn1Col = 0;
    const int conn2Col = 1;

    // first convert nodes
    for (std::vector<PlotRowValue>::const_iterator it = nodes->getRowsBegin(); it != nodes->getRowsEnd(); ++it) {
        nodeGraph_.addNode(static_cast<int>(it->getValueAt(idCol)), it->getValueAt(xCol), it->getValueAt(yCol), it->getValueAt(massCol));
    }

    // then convert connections
    for (std::vector<PlotRowValue>::const_iterator it = connections->getRowsBegin(); it != connections->getRowsEnd(); ++it) {
        nodeGraph_.connectNodes(static_cast<int>(it->getValueAt(conn1Col)), static_cast<int>(it->getValueAt(conn2Col)));
    }
}

void GraphPlot::applyForces() {
    double threshold = threshold_.get() / damping_.get();
    int step = 0;
    double maxVelocity = 0;
    do {
        nodeGraph_.applyForceDirectedLayoutStep(spring_.get(), charge_.get(), damping_.get());

        maxVelocity = 0;
        for (std::map<int, NodeGraphNode*>::const_iterator firstIt = nodeGraph_.getNodesBegin(); firstIt != nodeGraph_.getNodesEnd(); ++firstIt) {
            maxVelocity = std::max(maxVelocity, tgt::length(firstIt->second->velocity_));
        }

        process();
    } while (step <= 512 && maxVelocity > threshold);
}

void GraphPlot::applyForcesStep() {
    nodeGraph_.applyForceDirectedLayoutStep(spring_.get(), charge_.get(), damping_.get());
}

void GraphPlot::process() {
    if (inport_.isReady() && connectionPort_.isReady() && (inport_.hasChanged() || connectionPort_.hasChanged()))
        readFromInport();
    render();
}


void GraphPlot::render() {
    outport_.activateTarget();
    plotLib_.setUsePlotPickingManager(false);
    calcDomains();
    setPlotStatus();
    if (plotLib_.setOpenGLStatus()) {
        renderData();
    }
    plotLib_.resetOpenGLStatus();
    outport_.deactivateTarget();
/*    plotPickingManager_.activateTarget();
    plotPickingManager_.clearTarget();
    if (enablePicking_.get()) {
        plotLib_.setUsePlotPickingManager(true);
        if (plotLib_.setOpenGLStatus())
            renderData();
        plotLib_.resetOpenGLStatus();
    }
    plotPickingManager_.deactivateTarget();*/
}

void GraphPlot::setPlotStatus() {
    plotLib_.setDimension(PlotLibrary::TWO);
    plotLib_.setWindowSize(outport_.getSize());
    plotLib_.setDrawingColor(tgt::Color::black);
}

void GraphPlot::renderData() {
    plotLib_.setDrawingColor(tgt::Color::red);
    plotLib_.setMinGlyphSize(10);
    plotLib_.setMaxGlyphSize(15);

    plotLib_.renderNodeGraph(nodeGraph_);
    LGL_ERROR;
}

void GraphPlot::renderAxes() {

}

void GraphPlot::readFromInport() {
    // create local copy of data and assign it to property
    PlotData* nodeData = dynamic_cast<PlotData*>(inport_.getData());
    PlotData* connectionData = dynamic_cast<PlotData*>(connectionPort_.getData());

    if (nodeData && connectionData) {
        data_ = *nodeData;
        if (!data_.sorted())
            data_.sortRows();
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);

        convertPlotDataToNodeGraph(nodeData, connectionData);
    }
    else {
        LWARNINGC("GraphPlot", "GraphPlot can only handle PlotData objects");
        data_ = PlotData(0,0);
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        selectionProp_.setPlotData(&data_);
        plotPickingManager_.setColumnCount(data_.getColumnCount());
    }
}

void GraphPlot::calcDomains() {
    plotLib_.setDomain(nodeGraph_.getXInterval(), PlotLibrary::X_AXIS);
    plotLib_.setDomain(nodeGraph_.getYInterval(), PlotLibrary::Y_AXIS);
}

void GraphPlot::toggleProperties() {
}

void GraphPlot::createPlotLabels() {
}

} // namespace voreen
