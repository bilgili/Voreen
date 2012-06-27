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

#ifndef VRN_PLOTPROCESSOR_H
#define VRN_PLOTPROCESSOR_H

#include "voreen/core/processors/imageprocessor.h"
#include "voreen/core/ports/allports.h"

#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/plotdataproperty.h"
#include "voreen/core/properties/plotentitiesproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/plotselectionproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/boolproperty.h"

#include "voreen/core/plotting/plotfunction.h"
#include "voreen/core/plotting/plotdata.h"
#include "voreen/core/plotting/plotlibrary.h"

#include "voreen/core/interaction/plotpickingmanager.h"

namespace voreen {

class PlotProcessor : public ImageProcessor {

public:
    PlotProcessor(PlotEntitySettings::Entity entity, bool threeDimensional);
    virtual ~PlotProcessor();

    virtual std::string getCategory() const { return "Plotting"; }

protected:
    // virtual methods
    virtual void process();
    virtual void initialize() throw (VoreenException);

    /// renders the plot
    virtual void render() = 0;
    /// renders the data
    virtual void renderData() = 0;
    /// renders axes including scales
    virtual void renderAxes() = 0;
    /// set the status of the plotlib
    virtual void setPlotStatus() = 0;
    /// reads plotdata/plotfunction from inport
    virtual void readFromInport() = 0;
    /// calculates the domains from the data
    virtual void calcDomains() = 0;
    /// toggles visibility of some properties
    virtual void toggleProperties();
    /// is called if the zoom property has changed
    virtual void zoomPropChanged();
    /// is called if the entities property has changed
    virtual void entitiesPropChanged();

    // functions called by mouse events
    virtual void leftClickEvent(tgt::MouseEvent* e);
    virtual void zoomOut(tgt::MouseEvent* e);
    virtual void leftMoveEvent(tgt::MouseEvent* e);
    virtual void rightMoveEvent(tgt::MouseEvent* e);
    virtual void updateMousePosition(tgt::MouseEvent* e);
    virtual void enterExitEvent(tgt::MouseEvent* e);

    // non virtual methods
    /// renders the region selected by the mouse (2d)
    void renderSelectedRegion(tgt::ivec2 first, tgt::ivec2 second);
    /// renders the plot label
    void renderPlotLabel();
    /// clips a pair of positions to plotbounds (margins)
    std::pair<tgt::ivec2, tgt::ivec2> clipToPlotBounds(tgt::ivec2 first, tgt::ivec2 second);
    /// render the zoom clipping planes (3d)
    void renderZoomClippingPlanes();
    /// selects a plotdata from plotfunction
    void selectDataFromFunction();
    /// starts 3d zooming, returns true iff we are actually in zooming mode (zoomEdgeIndex_ != -1)
    bool startZooming3D(tgt::dvec2 startPosition);
    /// calculated zoomEnd_, returns true, if we are in zooming plane selection mode
    bool calculateZoomEnd(tgt::dvec2 mousePressPosition);
    /// zoom into region specified by zoomEdgeIndex, zoomStart_ and zoomEnd_
    void zoomIn3D();
    /// zooms into specified rectangle (2d)
    void zoomIn2D(tgt::ivec2 first, tgt::ivec2 second);
    /// renders the position of the mouse
    void renderMousePosition();

    PlotPort inport_;                       ///< inport serve a PlotData or PlotFunction
    RenderPort outport_;                    ///< outport where rendered plot goes
    RenderPort pickingBuffer_;              ///< buffer used by plotPickingManager_

    PlotPickingManager plotPickingManager_; ///< picking manager for current PlotData

    // interaction handlers
    EventProperty<PlotProcessor>* mouseEventClickLeft_;
    EventProperty<PlotProcessor>* mouseEventClickLeftCtrl_;
    EventProperty<PlotProcessor>* eventZoomOut_;
    EventProperty<PlotProcessor>* mouseEventMoveLeft_;
    EventProperty<PlotProcessor>* mouseEventMoveRight_;
    EventProperty<PlotProcessor>* mousePositionUpdateEvent_;
    EventProperty<PlotProcessor>* mouseEventEnterExit_;

    // general properties
    PlotDataProperty dataProp_;             ///< Offering a Data Table
    PlotEntitiesProperty plotEntitiesProp_; ///< Selection of Plot Entities
    StringProperty plotLabel_;              ///< Label of the Plot
    FloatVec4Property zoomPlaneColor_;      ///< color of the zoom planes
    BoolProperty enablePicking_;            ///< enable object picking
    BoolProperty renderMousePosition_;      ///< enable render mouse position

    // Margin
    IntProperty marginLeft_;
    IntProperty marginRight_;
    IntProperty marginBottom_;
    IntProperty marginTop_;

    IntProperty discreteStep_;          /// number of steps used for discretization of functions

    PlotSelectionProperty zoomProp_;         ///< handles zooming states

    PlotLibrary plotLib_;               ///< our library offering the render functions
    PlotData data_;                     ///< data table
    PlotFunction function_;             ///< function which can be evaluated to create a PlotData

    tgt::ivec2 mousePressedPosition_;   ///< viewport position of last MouseDown event
    tgt::ivec2 mousePosition_;          ///< current viewport mouse position if on canvas
    bool leftMousePressed_;             ///< flag if a MouseDown event was registered but no MouseReleased event yet
    bool mouseInsideCanvas_;

    PlotLibrary::ZoomEdge zoomEdge_; //< encapsulates information about the region in which we zoom
    int zoomEdgeIndex_;          ///< only for 3D plots: specifies the axis of zoom clipping planes, -1 if there are currently no zoom clipping planes.
    plot_t zoomStart_;           ///< only for 3D plots: plot coordinates for the first zoom clipping plane
    plot_t zoomEnd_;             ///< only for 3D plots: plot coordinates for the second zoom clipping plane

    bool inportHasPlotFunction_; ///< true iff the inport serves a PlotFunction which has the right keycolumncount (dimension-1)

    bool threeDimensional_;      ///< flag whether the plot is threedimensional
};

}   //namespace

#endif // VRN_PLOTPROCESSOR_H
