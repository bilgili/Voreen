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

#include "voreen/modules/base/processors/plotting/plotprocessor.h"

namespace voreen {

PlotProcessor::PlotProcessor(PlotEntitySettings::Entity entity, bool threeDimensional)
    : ImageProcessor()
    , inport_(Port::INPORT, "inport")
    , outport_(Port::OUTPORT, "outport", false, Processor::INVALID_RESULT, GL_RGBA)
    , pickingBuffer_(Port::OUTPORT, "pickingBuffer", false, Processor::INVALID_RESULT, GL_RGBA)
    , dataProp_("plotDataTable", "Plot Data Table", NULL)
    , plotEntitiesProp_("plotEntitiesProperty", "Entity Selection", entity)
    , plotLabel_("plotLabel", "Label", "", INVALID_RESULT)
    , zoomPlaneColor_("zoomPlaneColor", "Zoom Plane Color", tgt::vec4(.4f, .4f, .4f, 0.4f))
    , enablePicking_("enablePicking", "Enable Picking", true)
    , renderMousePosition_("renderMousePosition", "Render Mouse Position", true)
    , marginLeft_("marginLeft", "Left Margin", 60, 0, 200)
    , marginRight_("marginRight", "Right Margin", 100, 0, 200)
    , marginBottom_("marginBottom", "Bottom Margin", 50, 0, 200)
    , marginTop_("marginTop", "Top Margin", 50, 0, 200)
    , discreteStep_("descreteStep", "Number of Steps", 50, 1, 500)
    , zoomProp_("zoomProperty", "Zoom", false)
    , plotLib_()
    , data_(0,0)
    , leftMousePressed_(false)
    , mouseInsideCanvas_(false)
    , zoomEdgeIndex_(-1)
    , inportHasPlotFunction_(false)
    , threeDimensional_(threeDimensional)
{

    //create event properties
    mouseEventClickLeft_ = new EventProperty<PlotProcessor>("mouseClickLeft", "Mouse Click Left",
        this, &PlotProcessor::leftClickEvent, tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::CLICK);
    mouseEventClickLeftCtrl_ = new EventProperty<PlotProcessor>("mouseClickLeftCtrl",
        "Mouse Click Left with CTRL modifier", this,&PlotProcessor::leftClickEvent,
        tgt::MouseEvent::MOUSE_BUTTON_LEFT, tgt::MouseEvent::CLICK, tgt::Event::CTRL);
    eventZoomOut_ = new EventProperty<PlotProcessor>("zoomOut",
        "Zoom Out", this, &PlotProcessor::zoomOut,
        tgt::MouseEvent::MOUSE_BUTTON_RIGHT, tgt::MouseEvent::CLICK | tgt::MouseEvent::DOUBLECLICK);
    mouseEventMoveLeft_ = new EventProperty<PlotProcessor>("mouseMoveLeft",
        "Mouse Move Left", this, &PlotProcessor::leftMoveEvent, tgt::MouseEvent::MOUSE_BUTTON_LEFT,
        tgt::MouseEvent::MOTION);
    mouseEventMoveRight_ = new EventProperty<PlotProcessor>("mouseMoveRight",
        "Mouse Move Right", this, &PlotProcessor::rightMoveEvent,
        tgt::MouseEvent::MOUSE_BUTTON_RIGHT, tgt::MouseEvent::MOTION);
    mousePositionUpdateEvent_ = new EventProperty<PlotProcessor>("mousePositionUpdate", "Update Mouse Position", this,
        &PlotProcessor::updateMousePosition, tgt::MouseEvent::MOUSE_BUTTON_NONE,
        tgt::MouseEvent::MOTION, tgt::MouseEvent::MODIFIER_NONE);
    mouseEventEnterExit_ = new EventProperty<PlotProcessor>("enterExitEvent", "Enter/Exit Event",
        this, &PlotProcessor::enterExitEvent, tgt::MouseEvent::MOUSE_BUTTON_NONE,
        tgt::MouseEvent::ENTER_EXIT, tgt::MouseEvent::MODIFIER_NONE);

    // set up onChange calls
    plotEntitiesProp_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::toggleProperties));
    // it is important to calculate the domains after the properties are toggled / updated
    plotEntitiesProp_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::calcDomains));
    plotEntitiesProp_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::entitiesPropChanged));
    discreteStep_.setVisible(false);
    discreteStep_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::selectDataFromFunction));

    zoomProp_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::selectDataFromFunction));
    zoomProp_.onChange(CallMemberAction<PlotProcessor>(this, &PlotProcessor::zoomPropChanged));
    zoomProp_.setShowThirdDimension(threeDimensional_);

    zoomPlaneColor_.setViews(Property::COLOR);

    // set up ports
    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(pickingBuffer_);
}

PlotProcessor::~PlotProcessor() {
    delete mouseEventClickLeft_;
    delete mouseEventClickLeftCtrl_;
    delete eventZoomOut_;
    delete mouseEventMoveLeft_;
    delete mouseEventMoveRight_;
    delete mousePositionUpdateEvent_;
    delete mouseEventEnterExit_;
}

void PlotProcessor::initialize() throw (VoreenException) {
    ImageProcessor::initialize();
    plotPickingManager_.setRenderTarget(pickingBuffer_.getRenderTarget());
    plotPickingManager_.initializeTarget();
    plotLib_.setPlotPickingManager(&plotPickingManager_);
}

void PlotProcessor::leftClickEvent(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    if (e->action() == tgt::MouseEvent::PRESSED) {
        mousePressedPosition_ = mousePosition_;
        leftMousePressed_ = true;
    }
    else
        leftMousePressed_ = false;
    invalidate();
    e->accept();
}

void PlotProcessor::zoomOut(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    if (e->action() & tgt::MouseEvent::DOUBLECLICK) {
        while (zoomProp_.size() > 1)
            zoomProp_.pop();
    }
    else if (e->action() & tgt::MouseEvent::PRESSED) {
        mousePressedPosition_ = mousePosition_;
        if (tgt::length(mousePressedPosition_ - mousePosition_) < 4 && zoomProp_.size() > 1)
            zoomProp_.pop();
    }
    invalidate();
    e->accept();
}

void PlotProcessor::leftMoveEvent(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    invalidate();
    e->accept();
}

void PlotProcessor::rightMoveEvent(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    invalidate();
    e->accept();
}

void PlotProcessor::updateMousePosition(tgt::MouseEvent* e) {
    mousePosition_ = tgt::ivec2(e->x(),e->viewport().y-e->y());
    invalidate();
    e->accept();
}

void PlotProcessor::enterExitEvent(tgt::MouseEvent* e) {
    mouseInsideCanvas_ = e->action() == tgt::MouseEvent::ENTER;
    invalidate();
    e->accept();
}

void PlotProcessor::process() {
    if (inport_.isReady() && inport_.hasChanged())
        readFromInport();
    if (plotEntitiesProp_.dataValid())
        render();
}

void PlotProcessor::selectDataFromFunction() {
    if (inportHasPlotFunction_) {
        if (!zoomProp_.get().empty() && zoomProp_.top().xZoom_.size() > 0
                && (!threeDimensional_ || zoomProp_.top().yZoom_.size() > 0 ))  {
            std::vector<Interval<plot_t> > zoom;
            zoom.push_back(zoomProp_.top().xZoom_);
            if (threeDimensional_)
                zoom.push_back(zoomProp_.top().yZoom_);
            std::vector<plot_t> step;
            step.push_back(zoomProp_.top().xZoom_.size()/discreteStep_.get());
            if (threeDimensional_)
                step.push_back(zoomProp_.top().yZoom_.size()/discreteStep_.get());
            // select the data
            data_ = PlotData(0,0);
            function_.select(zoom, step,data_);
            dataProp_.set(&data_);
            plotEntitiesProp_.setPlotData(&data_);
            plotPickingManager_.setColumnCount(data_.getColumnCount());
            if (zoomProp_.isDeserialized_)
                zoomProp_.isDeserialized_ = false;
        }
    }
}

void PlotProcessor::toggleProperties() {/* Do nothing here. */ }
void PlotProcessor::zoomPropChanged() {/* Do nothing here. */ }
void PlotProcessor::entitiesPropChanged() {/* Do nothing here. */ }

void PlotProcessor::renderMousePosition() {
    if (renderMousePosition_.get() && mouseInsideCanvas_) {
        plotLib_.setFontColor(tgt::Color::black);
        plotLib_.setFontSize(10);
        plotLib_.renderMousePosition(mousePosition_);
    }
}

void PlotProcessor::renderSelectedRegion(tgt::ivec2 first, tgt::ivec2 second) {
    //only if the mouse is pressed, you can select a region
    if (leftMousePressed_) {
        std::pair<tgt::ivec2, tgt::ivec2> rectPosition = clipToPlotBounds(first, second);
        first = rectPosition.first;
        second = rectPosition.second;
        glDisable(GL_DEPTH_TEST);
        glEnable (GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0,outport_.getSize().x,0,outport_.getSize().y);
        tgt::Color c = zoomPlaneColor_.get();
        glColor4f(c.r,c.g,c.b,c.a);
        glBegin(GL_QUADS);
            glVertex2i(first.x,first.y);   glVertex2i(first.x,second.y);
            glVertex2i(second.x,second.y); glVertex2i(second.x,first.y);
        glEnd();
        glColor3f(0.f,0.f,0.f);
        glBegin(GL_LINE_LOOP);
            glVertex2i(first.x,first.y);   glVertex2i(first.x,second.y);
            glVertex2i(second.x,second.y); glVertex2i(second.x,first.y);
        glEnd();
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

void PlotProcessor::renderPlotLabel() {
    if (plotLabel_.get() != "") {
        plotLib_.setFontColor(tgt::Color::blue);
        plotLib_.setFontSize(18);
        plotLib_.renderLabel(plotLib_.getCenterAbovePlot(), SmartLabel::CENTERED, plotLabel_.get(), true);
    }
}

std::pair<tgt::ivec2, tgt::ivec2> PlotProcessor::clipToPlotBounds(tgt::ivec2 first, tgt::ivec2 second) {
    if (first.x < marginLeft_.get())
        first.x = 0;
    else if (first.x > outport_.getSize().x-marginRight_.get())
        first.x = outport_.getSize().x;
    if (first.y < marginBottom_.get())
        first.y = 0;
    else if (first.y > outport_.getSize().y-marginTop_.get())
        first.y = outport_.getSize().y;

    if (first.x == 0)
        second.x = outport_.getSize().x;
    else if (first.x == outport_.getSize().x)
        second.x = 0;
    if (first.y == 0)
        second.y = outport_.getSize().y;
    else if (first.y == outport_.getSize().y)
        second.y = 0;
    return std::pair<tgt::ivec2, tgt::ivec2>(first, second);
}

void PlotProcessor::renderZoomClippingPlanes() {
    tgt::Color c = zoomPlaneColor_.get();
    glColor4f(c.r,c.g,c.b,c.a);
    if (zoomEdgeIndex_ == 0) {
        glBegin(GL_POLYGON);
        glVertex3d(zoomStart_, zoomProp_.top().yZoom_.getLeft(), zoomProp_.top().zZoom_.getLeft());
        glVertex3d(zoomStart_, zoomProp_.top().yZoom_.getLeft(), zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomStart_, zoomProp_.top().yZoom_.getRight(), zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomStart_, zoomProp_.top().yZoom_.getRight(), zoomProp_.top().zZoom_.getLeft());
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(zoomEnd_, zoomProp_.top().yZoom_.getLeft(), zoomProp_.top().zZoom_.getLeft());
        glVertex3d(zoomEnd_, zoomProp_.top().yZoom_.getLeft(), zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomEnd_, zoomProp_.top().yZoom_.getRight(), zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomEnd_, zoomProp_.top().yZoom_.getRight(), zoomProp_.top().zZoom_.getLeft());
        glEnd();
    }
    if (zoomEdgeIndex_ == 1) {
        glBegin(GL_POLYGON);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomStart_, zoomProp_.top().zZoom_.getLeft());
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomStart_, zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomStart_, zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomStart_, zoomProp_.top().zZoom_.getLeft());
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomEnd_, zoomProp_.top().zZoom_.getLeft());
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomEnd_, zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomEnd_, zoomProp_.top().zZoom_.getRight());
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomEnd_, zoomProp_.top().zZoom_.getLeft());
        glEnd();
    }
    if (zoomEdgeIndex_ == 2) {
        glBegin(GL_POLYGON);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomProp_.top().yZoom_.getLeft(), zoomStart_);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomProp_.top().yZoom_.getRight(), zoomStart_);
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomProp_.top().yZoom_.getRight(), zoomStart_);
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomProp_.top().yZoom_.getLeft(), zoomStart_);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomProp_.top().yZoom_.getLeft(), zoomEnd_);
        glVertex3d(zoomProp_.top().xZoom_.getLeft(), zoomProp_.top().yZoom_.getRight(), zoomEnd_);
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomProp_.top().yZoom_.getRight(), zoomEnd_);
        glVertex3d(zoomProp_.top().xZoom_.getRight(), zoomProp_.top().yZoom_.getLeft(), zoomEnd_);
        glEnd();
    }
}

void PlotProcessor::zoomIn2D(tgt::ivec2 first, tgt::ivec2 second) {
    std::pair<tgt::ivec2, tgt::ivec2> clippedPos = clipToPlotBounds(first, second);
    //if a non empty area is selected and the domains exist, we zoom in
    if ( clippedPos.first.x != clippedPos.second.x && clippedPos.first.y != clippedPos.second.y
                            && !zoomProp_.get().empty()) {
        plotLib_.setOpenGLStatus();
        tgt::dvec2 firstPosPlot = plotLib_.convertViewportToPlotCoordinates(clippedPos.first);
        tgt::dvec2 secondPosPlot = plotLib_.convertViewportToPlotCoordinates(clippedPos.second);
        plotLib_.resetOpenGLStatus();
        double xLeft = (secondPosPlot.x < firstPosPlot.x ? secondPosPlot.x : firstPosPlot.x);
        double xRight = (secondPosPlot.x < firstPosPlot.x ? firstPosPlot.x : secondPosPlot.x);
        double yBottom = (secondPosPlot.y < firstPosPlot.y ? secondPosPlot.y : firstPosPlot.y);
        double yTop = (secondPosPlot.y < firstPosPlot.y ? firstPosPlot.y : secondPosPlot.y);
        Interval<plot_t> xZoom = Interval<plot_t>(xLeft,xRight);
        Interval<plot_t> yZoom = Interval<plot_t>(yBottom,yTop);
        //make sure that we do not zoom out
        xZoom = xZoom.intersectWith(zoomProp_.top().xZoom_);
        yZoom = yZoom.intersectWith(zoomProp_.top().yZoom_);
        //opengl don't like to much zooming for accuracy reasons, so we limit the zooming factor
        plot_t minZoom = 0.0001;
        if (fabs(xZoom.size()/xZoom.getLeft()) > minZoom && fabs(yZoom.size()/yZoom.getLeft()) > minZoom) {
            zoomProp_.push(PlotZoomState(xZoom, yZoom));
        }
    }
}

void PlotProcessor::zoomIn3D() {
    const PlotZoomState& current = zoomProp_.top();
    if (zoomStart_ > zoomEnd_)
        std::swap(zoomStart_, zoomEnd_);
    if (zoomEdgeIndex_ == 0) { // do x-axis zoom
        zoomProp_.push(PlotZoomState(Interval<plot_t>(zoomStart_, zoomEnd_), current.yZoom_, current.zZoom_));
    }
    else if (zoomEdgeIndex_ == 1) { // do y-axis zoom
        zoomProp_.push(PlotZoomState(current.xZoom_, Interval<plot_t>(zoomStart_, zoomEnd_), current.zZoom_));
    }
    else if (zoomEdgeIndex_ == 2) { // do z-axis zoom
        zoomProp_.push(PlotZoomState(current.xZoom_, current.yZoom_, Interval<plot_t>(zoomStart_, zoomEnd_)));
    }
    // finish zooming
    zoomEdgeIndex_ = -1;
}

bool PlotProcessor::startZooming3D(tgt::dvec2 startPosition) {
    // check if position was outside of the cube and which axis to zoom
    tgt::dvec2 center(outport_.getSize().x/2, outport_.getSize().y/2);

    zoomEdgeIndex_ = -1; // -1 means no axis found
    Interval<plot_t> interval;

    std::vector<PlotLibrary::ZoomEdge>::const_iterator it;
    // check against x-zoom-axes
    for (it = plotLib_.getZoomEdgesX().begin(); it < plotLib_.getZoomEdgesX().end(); ++it) {
        if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
            && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
            && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                zoomEdge_ = *it;
                zoomEdgeIndex_ = 0;
                interval = zoomProp_.top().xZoom_;
        }
    }
    if (zoomEdgeIndex_ == -1) {
        // check against y-zoom-axes
        for (it = plotLib_.getZoomEdgesY().begin(); it < plotLib_.getZoomEdgesY().end(); ++it) {
            if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
                && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
                && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                    zoomEdge_ = *it;
                    zoomEdgeIndex_ = 1;
                    interval = zoomProp_.top().yZoom_;
            }
        }
    }
    if (zoomEdgeIndex_ == -1) {
        // check against z-zoom-axes
        for (it = plotLib_.getZoomEdgesZ().begin(); it < plotLib_.getZoomEdgesZ().end(); ++it) {
            if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
                && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
                && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                    zoomEdge_ = *it;
                    zoomEdgeIndex_ = 2;
                    interval = zoomProp_.top().zZoom_;
            }
        }
    }
    // if zoomEdgeIndex_ != -1, we are outside of the cube
    if (zoomEdgeIndex_ != -1) {
        // determine position of the zoom clipping planes by orthogonal projection of
        // start-click onto the zoom edge (scaled dot product)
        tgt::dvec2 edge = zoomEdge_.endVertex_ - zoomEdge_.startVertex_;
        tgt::dvec2 click = startPosition - zoomEdge_.startVertex_;
        double dot = (edge.x*click.x + edge.y*click.y) / (tgt::length(edge) * tgt::length(edge));

        // calculate plot coordinates of zoom clipping plane
        if (zoomEdge_.ascOrientation_)
            zoomStart_ = interval.getLeft() + (dot*interval.size());
        else
            zoomStart_ = interval.getRight() - (dot*interval.size());

        // clamp value to interval
        zoomStart_ = interval.clampValue(zoomStart_);
        zoomEnd_ = zoomStart_;
        return true;
    }
    else
        return false; // no zoomEdge found
}

bool PlotProcessor::calculateZoomEnd(tgt::dvec2 mousePressPosition) {
    // check if we are in zooming plane selection mode
    if (zoomEdgeIndex_ != -1) {

        // get current zoom interval of selected axis
        Interval<plot_t> interval;
        if (zoomEdgeIndex_ == 0)
            interval = zoomProp_.top().xZoom_;
        else if (zoomEdgeIndex_ == 1)
            interval = zoomProp_.top().yZoom_;
        else if (zoomEdgeIndex_ == 2)
            interval = zoomProp_.top().zZoom_;

        // determine position of the zoom clipping planes by orthogonal projection of
        // start-click onto the zoom edge (scaled dot product)
        tgt::dvec2 edge = zoomEdge_.endVertex_ - zoomEdge_.startVertex_;
        tgt::dvec2 click = mousePressPosition - zoomEdge_.startVertex_;
        double dot = (edge.x*click.x + edge.y*click.y) / (tgt::length(edge) * tgt::length(edge));

        // calculate plot coordinates of zoom clipping plane
        if (zoomEdge_.ascOrientation_)
            zoomEnd_ = interval.getLeft() + (dot*interval.size());
        else
            zoomEnd_ = interval.getRight() - (dot*interval.size());

        // clamp value to interval
        zoomEnd_ = interval.clampValue(zoomEnd_);
        return true;
    }
    else
        return false;
}

} // namespace voreen
