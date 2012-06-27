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

#include "voreen/modules/base/processors/plotting/surfaceplot.h"

#include "voreen/core/interaction/plotcamerainteractionhandler.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/plotting/plotrow.h"

namespace voreen {

const std::string SurfacePlot::loggerCat_("voreen.SurfacePlot");

SurfacePlot::SurfacePlot()
    : PlotProcessor(PlotEntitySettings::SURFACE, true)
    , omitDelaunayTriangulation_(0)
    , delaunay_(NULL)
    , dataList_(0)
    , pickingList_(0)
{
    plotEntitiesProp_.setGuiName("Surface Data");
    addProperty(zLabel_);
    addProperty(selectionPlaneColor_);
    addProperty(renderLegend_);
    addProperty(discreteStep_);
    addProperty(orthographicCamera_);
    addProperty(renderMousePosition_);
    addProperty(renderXHelperLines_);
    addProperty(renderYHelperLines_);
    addProperty(renderZHelperLines_);
    addProperty(xScaleStep_);
    addProperty(yScaleStep_);
    addProperty(zScaleStep_);
    addProperty(camera_);

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
    orthographicCamera_.onChange(CallMemberAction<SurfacePlot>(this, &SurfacePlot::toggleProperties));
    selectionProp_.onChange(CallMemberAction<SurfacePlot>(this, &SurfacePlot::domainChanged));
}

std::string SurfacePlot::getProcessorInfo() const {
    return std::string("This processor is able to create 3D surfaceplots and heightmaps.");
}

Processor* SurfacePlot::create() const {
    return new SurfacePlot();
}

void SurfacePlot::initialize() throw (VoreenException) {
    PlotProcessor::initialize();
    dataList_ = glGenLists(1);
    pickingList_ = glGenLists(1);
}

void SurfacePlot::deinitialize() throw (VoreenException) {
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

void SurfacePlot::domainChanged() {
    // force recalculation of voronoi regions and display lists
    currentIndexX_ = -1;
    generateDelaunay();
    regenDisplayLists();
}

void SurfacePlot::generateDelaunay() {
    if (omitDelaunayTriangulation_  == 0 && plotEntitiesProp_.dataValid()) {
        if (plotEntitiesProp_.getXColumnIndex() != currentIndexX_ ||plotEntitiesProp_.getYColumnIndex() != currentIndexY_) {
            currentIndexX_ = plotEntitiesProp_.getXColumnIndex();
            currentIndexY_ = plotEntitiesProp_.getYColumnIndex();

#ifdef VRN_MODULE_TRIANGLE
            delete delaunay_;

            std::vector<tpp::Delaunay::Point> points;
            for (std::vector<PlotRowValue>::const_iterator it = data_.getRowsBegin(); it < data_.getRowsEnd(); ++it) {
                points.push_back(tpp::Delaunay::Point(it->getValueAt(plotEntitiesProp_.getXColumnIndex()), it->getValueAt(plotEntitiesProp_.getYColumnIndex())));
            }

            delaunay_ = new tpp::Delaunay(points);
            std::string s = "vziQ";
            delaunay_->Triangulate(s);
            voronoiRegions_.clear();
            delaunay_->generateVoronoiRegions(data_, voronoiRegions_);

            triangleEdgeIndices_.clear();
            triangleEdgeIndices_.reserve(delaunay_->ntriangles() * 3);
            for (tpp::Delaunay::fIterator fit = delaunay_->fbegin(); fit != delaunay_->fend(); ++fit) {
                triangleEdgeIndices_.push_back(delaunay_->Org(fit));
                triangleEdgeIndices_.push_back(delaunay_->Dest(fit));
                triangleEdgeIndices_.push_back(delaunay_->Apex(fit));
            }

#else
            // we assume we have an uniform rectliniar grid, calculate the number of rows per column:
            int numRowsPerColumn = 1;
            plot_t xValue = data_.getRowsBegin()->getValueAt(currentIndexX_);
            for (std::vector<PlotRowValue>::const_iterator it = ++data_.getRowsBegin(); it < data_.getRowsEnd() && xValue == it->getValueAt(currentIndexX_); ++it) {
                ++numRowsPerColumn;
            }

            // generate triangles
            triangleEdgeIndices_.clear();
            int numCols = data_.getRowsCount() / numRowsPerColumn;
            for (int col = 0; col < numCols - 1; ++col) {
                for (int row = 0; row < numRowsPerColumn - 1; ++row) {
                    triangleEdgeIndices_.push_back(col*numRowsPerColumn + row);
                    triangleEdgeIndices_.push_back(col*numRowsPerColumn + row + 1);
                    triangleEdgeIndices_.push_back((col+1)*numRowsPerColumn + row);

                    triangleEdgeIndices_.push_back(col*numRowsPerColumn + row + 1);
                    triangleEdgeIndices_.push_back((col+1)*numRowsPerColumn + row + 1);
                    triangleEdgeIndices_.push_back((col+1)*numRowsPerColumn + row);
                }
            }

            // generate voronoi regions
            voronoiRegions_.clear();
            voronoiRegions_.resize(data_.getRowsCount());
            for (int col = 0; col < numCols; ++col) {
                for (int row = 0; row < numRowsPerColumn; ++row) {
                    std::list< std::pair<plot_t, plot_t> > region;

                    int indexCurrent = col*numRowsPerColumn + row;
                    int indexBelow = col*numRowsPerColumn + row + 1;
                    int indexAbove = col*numRowsPerColumn + row - 1;
                    int indexLeft = (col-1)*numRowsPerColumn + row;
                    int indexRight = (col+1)*numRowsPerColumn + row;

                    const PlotRowValue& current = data_.getRow(indexCurrent);
                    const PlotRowValue& below   = (row < numRowsPerColumn-1) ? data_.getRow(indexBelow) : current;
                    const PlotRowValue& above   = (row > 0) ? data_.getRow(indexAbove) : current;
                    const PlotRowValue& left    = (col > 0) ? data_.getRow(indexLeft) : current;
                    const PlotRowValue& right   = (col < numCols-1) ? data_.getRow(indexRight) : current;

                    voronoiRegions_[indexCurrent].push_back(tgt::dvec2(right.getValueAt(currentIndexX_) + 0.5*(current.getValueAt(currentIndexX_) - right.getValueAt(currentIndexX_)),
                                                                       above.getValueAt(currentIndexY_) + 0.5*(current.getValueAt(currentIndexY_) - above.getValueAt(currentIndexY_))));

                    voronoiRegions_[indexCurrent].push_back(tgt::dvec2(left.getValueAt(currentIndexX_) + 0.5*(current.getValueAt(currentIndexX_) - left.getValueAt(currentIndexX_)),
                                                                       above.getValueAt(currentIndexY_) + 0.5*(current.getValueAt(currentIndexY_) - above.getValueAt(currentIndexY_))));

                    voronoiRegions_[indexCurrent].push_back(tgt::dvec2(left.getValueAt(currentIndexX_) + 0.5*(current.getValueAt(currentIndexX_) - left.getValueAt(currentIndexX_)),
                                                                       below.getValueAt(currentIndexY_) + 0.5*(current.getValueAt(currentIndexY_) - below.getValueAt(currentIndexY_))));

                    voronoiRegions_[indexCurrent].push_back(tgt::dvec2(right.getValueAt(currentIndexX_) + 0.5*(current.getValueAt(currentIndexX_) - right.getValueAt(currentIndexX_)),
                                                                       below.getValueAt(currentIndexY_) + 0.5*(current.getValueAt(currentIndexY_) - below.getValueAt(currentIndexY_))));
                }
            }
#endif

        clipVoronoiRegionsToZoom();
        }
    }
}

void SurfacePlot::clipVoronoiRegionsToZoom() {
    // we define the four clipping edges we need for clipping
    const Interval<plot_t>& xInterval = selectionProp_.getZoom().xZoom_;
    const Interval<plot_t>& yInterval = selectionProp_.getZoom().yZoom_;
    std::pair<tgt::dvec2, tgt::dvec2> clipEdges[] = {
        std::make_pair( tgt::dvec2(xInterval.getRight(), yInterval.getRight()), tgt::dvec2(xInterval.getLeft(), yInterval.getRight()) ),
        std::make_pair( tgt::dvec2(xInterval.getLeft(), yInterval.getRight()), tgt::dvec2(xInterval.getLeft(), yInterval.getLeft()) ),
        std::make_pair( tgt::dvec2(xInterval.getLeft(), yInterval.getLeft()), tgt::dvec2(xInterval.getRight(), yInterval.getLeft()) ),
        std::make_pair( tgt::dvec2(xInterval.getRight(), yInterval.getLeft()), tgt::dvec2(xInterval.getRight(), yInterval.getRight()) )
    };

    // some vertices might be out of the mesh bounding box, so we will do some clipping with Sutherland-Hodgman
    // cycle through clipping edges
    for (std::vector< std::list<tgt::dvec2> >::iterator poly = voronoiRegions_.begin(); poly < voronoiRegions_.end(); ++poly) {
        for (int e=0; e<4; ++e) {
            // check for empty polygon
            if (poly->empty())
                break;

            tgt::dvec2 s = poly->back(); // get last vertex
            for (std::list<tgt::dvec2>::iterator vit = poly->begin(); vit != poly->end(); /* do nothing here, as we advance the iterator within the loop */) {
                tgt::dvec2 p = *vit;
                if (insideMeshBounds(p, clipEdges[e])) {
                    if (!insideMeshBounds(s, clipEdges[e])){ // case 4
                        tgt::dvec2 t = intersect(s, p, clipEdges[e]);
                        poly->insert(vit, t); // insert t before p, leave p as is
                        ++vit;
                    }
                    else {
                        ++vit; // leave p as is
                    }
                    s = p;
                }
                else {
                    if (insideMeshBounds(s, clipEdges[e])) { // case 2
                        tgt::dvec2 t = intersect(s, p, clipEdges[e]);
                        s = p;
                        *vit = t; // replace p with t;
                        ++vit;
                    }
                    else  { // case 3
                        vit = poly->erase(vit); // remove p from poly
                        s = p;
                    }
                }
            }
        }
    }
}

bool SurfacePlot::insideMeshBounds(const tgt::dvec2& v, const std::pair<tgt::dvec2, tgt::dvec2>& e) {
    // assume: e is edge of a clip rectangle
    if(e.second[0] > e.first[0]) { // bottom
        if(v[1] >= e.first[1])
            return true;
    }
    else if(e.second[0] < e.first[0]) { // top
        if(v[1] <= e.first[1])
            return true;
    }
    else if(e.second[1] > e.first[1]) { // right
        if(v[0] <= e.second[0])
            return true;
    }
    else if(e.second[1] < e.first[1]) {// left
        if(v[0] >= e.first[0])
            return true;
    }
    return false;
}

tgt::dvec2 SurfacePlot::intersect(const tgt::dvec2& p, const tgt::dvec2& s, const std::pair<tgt::dvec2, tgt::dvec2>& e) {
    // horizontal edge
    if (e.first[1] == e.second[1])
        return tgt::dvec2(p[0] + (e.first[1] - p[1]) * (s[0] - p[0]) / (s[1] - p[1]), e.first[1]);
    // vertical edge
    else
        return tgt::dvec2(e.first[0], p[1] + (e.first[0] - p[0]) * (s[1] - p[1]) / (s[0] - p[0]));
}

void SurfacePlot::render() {
    plotLib_.setUsePlotPickingManager(false);
    outport_.activateTarget();
    setPlotStatus();
    plotLib_.setOpenGLStatus();
    renderAxes();
    if (regenDataList_) {
        glNewList(dataList_, GL_COMPILE);
            renderData();
        glEndList();
        regenDataList_ = false;
    }
    glCallList(dataList_);
    createPlotLabels();
    renderPlotLabel();
    renderSelectionPlanes();
    plotLib_.renderPlotLabels();
    renderMousePosition();
    renderLegends();
    plotLib_.resetOpenGLStatus();
    outport_.deactivateTarget();
    plotPickingManager_.activateTarget();
    plotPickingManager_.clearTarget();
    if (enablePicking_.get()) {
        plotLib_.setUsePlotPickingManager(true);
        plotLib_.setOpenGLStatus();
        if (regenPickingList_) {
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

void SurfacePlot::setPlotStatus() {
    plotLib_.setDimension(PlotLibrary::THREE);
    plotLib_.setWindowSize(outport_.getSize());
    plotLib_.setAxesWidth(axesWidth_.get());
    plotLib_.setCamera(camera_.get());
    plotLib_.setDomain(selectionProp_.getZoom().xZoom_, PlotLibrary::X_AXIS);
    plotLib_.setDomain(selectionProp_.getZoom().yZoom_, PlotLibrary::Y_AXIS);
    plotLib_.setDomain(selectionProp_.getZoom().zZoom_, PlotLibrary::Z_AXIS);
    plotLib_.setMinimumScaleStep(xScaleStep_.get(), PlotLibrary::X_AXIS);
    plotLib_.setMinimumScaleStep(yScaleStep_.get(), PlotLibrary::Y_AXIS);
    plotLib_.setMinimumScaleStep(zScaleStep_.get(), PlotLibrary::Z_AXIS);
    plotLib_.setOrthographicCameraFlag(orthographicCamera_.get());
}

void SurfacePlot::renderData() {
    //render Surfaces
    plotLib_.setLineWidth(1);
    plotLib_.setHighlightColor(highlightColor_.get());
    std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
    for (; it < plotEntitiesProp_.get().end(); ++it) {
        if (it->getOptionalColumnIndex() != -1) //use colormap
            plotLib_.setColorMap(it->getColorMap());
        else
            plotLib_.setDrawingColor(it->getFirstColor());
        //render fill
        if (!it->getWireOnlyFlag()) {
            if (it->getHeightmapFlag())
                plotLib_.renderHeightmap(data_, voronoiRegions_, false, it->getMainColumnIndex(),
                                         it->getOptionalColumnIndex());
            else
                plotLib_.renderSurface(data_, triangleEdgeIndices_, false, plotEntitiesProp_.getXColumnIndex(),
                                       plotEntitiesProp_.getYColumnIndex(), it->getMainColumnIndex(),
                                       it->getOptionalColumnIndex() );
        }
        //render wire
        plotLib_.setDrawingColor(it->getSecondColor());
        if (it->getHeightmapFlag())
            plotLib_.renderHeightmap(data_, voronoiRegions_, true, it->getMainColumnIndex());
        else
            plotLib_.renderSurface(data_, triangleEdgeIndices_, true, plotEntitiesProp_.getXColumnIndex(),
                                   plotEntitiesProp_.getYColumnIndex(), it->getMainColumnIndex());
    }
}

void SurfacePlot::renderAxes() {
    if (renderAxes_.get()) {
        plotLib_.setDrawingColor(tgt::Color::black);
        plotLib_.renderAxes();
        if (renderScales_.get()) {
            plotLib_.setFontColor(tgt::Color::black);
            plotLib_.setFontSize(10);
            plotLib_.renderAxisScales(PlotLibrary::X_AXIS, renderXHelperLines_.get(), xLabel_.get());
            plotLib_.renderAxisScales(PlotLibrary::Y_AXIS, renderYHelperLines_.get(), yLabel_.get());
            plotLib_.renderAxisScales(PlotLibrary::Z_AXIS, renderZHelperLines_.get(), zLabel_.get());
        }
    }
}

void SurfacePlot::readFromInport() {
    if (dynamic_cast<PlotData*>(inport_.getData())) {
        ++omitDelaunayTriangulation_;
        data_ = *dynamic_cast<PlotData*>(inport_.getData());
#ifdef VRN_MODULE_TRIANGLE
#else
        if (!data_.sorted())
            data_.sortRows();
#endif
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        selectionProp_.setPlotData(&data_);
        inportHasPlotFunction_ = false;
        discreteStep_.setVisible(false);

        // enforce re-triangulation
        --omitDelaunayTriangulation_;
        currentIndexX_ = -1;
        currentIndexY_ = -1;
        generateDelaunay();
        plotPickingManager_.setColumnCount(data_.getColumnCount());
    }
    else if (dynamic_cast<PlotFunction*>(inport_.getData()) && inport_.getData()->getKeyColumnCount() == 2) {
        ++omitDelaunayTriangulation_;
        function_ = *dynamic_cast<PlotFunction*>(inport_.getData());
        inportHasPlotFunction_ = true;
        selectDataFromFunction();
        discreteStep_.setVisible(true);

        // enforce re-triangulation
        --omitDelaunayTriangulation_;
        currentIndexX_ = -1;
        currentIndexY_ = -1;
        generateDelaunay();
    }
    else {
        LWARNINGC("SurfacePlot", "SurfacePlot can only handle PlotData objects and PlotFunction objects with two key column");
        ++omitDelaunayTriangulation_;
        data_ = PlotData(0,0);
        dataProp_.set(&data_);
        plotEntitiesProp_.setPlotData(&data_);
        selectionProp_.setPlotData(&data_);
        plotPickingManager_.setColumnCount(data_.getColumnCount());
        voronoiRegions_.clear();
        triangleEdgeIndices_.clear();
        --omitDelaunayTriangulation_;
    }
}

void SurfacePlot::calcDomains() {
    if (plotEntitiesProp_.dataValid() && !inportHasPlotFunction_) {
        Interval<plot_t> xDomain = data_.getInterval(plotEntitiesProp_.getXColumnIndex());
        Interval<plot_t> yDomain = data_.getInterval(plotEntitiesProp_.getYColumnIndex());
        Interval<plot_t> zDomain;
        std::vector<PlotEntitySettings>::const_iterator it = plotEntitiesProp_.get().begin();
        for (; it < plotEntitiesProp_.get().end(); ++it)
            zDomain.unionWith(data_.getInterval(it->getMainColumnIndex()));
        selectionProp_.setBaseZoomState(PlotZoomState(xDomain, yDomain, zDomain));
    }
}

void SurfacePlot::toggleProperties() {
    plotCameraHandler_->setVisible(orthographicCamera_.get());
    plotCameraHandler_->setEnabled(orthographicCamera_.get());
    cameraHandler_->setVisible(!orthographicCamera_.get());
    cameraHandler_->setEnabled(!orthographicCamera_.get());
    if (orthographicCamera_.get())
        camera_.set(tgt::Camera(tgt::normalize(camera_.get().getPosition()), tgt::vec3(0,0,0), tgt::vec3(0,0,1)));
}

void SurfacePlot::selectDataFromFunction() {
    if (inportHasPlotFunction_) {
        ++omitDelaunayTriangulation_;
        PlotProcessor::selectDataFromFunction();
        --omitDelaunayTriangulation_;

        // enforce re-triangulation
        currentIndexX_ = -1;
        currentIndexY_ = -1;
        generateDelaunay();
    }
}

} // namespace voreen
