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

#include "voreen/core/plotting/plotlibrary.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/plotting/plotrow.h"
#include "voreen/core/interaction/plotpickingmanager.h"

#include "tgt/glmath.h"
#include "tgt/math.h"
#include "tgt/spline.h"
#include "tgt/quadric.h"

#include <iomanip>

namespace voreen {

namespace {
/// compares pair<plot_t, color> but only the plot_t is used. We need it to order bars
struct MergedBarSorter {
    bool operator() (std::pair<plot_t, tgt::Color> a, std::pair<plot_t, tgt::Color> b) const {
        if (a.first >= 0 || b.first >= 0)
            return (a.first < b.first);
        else
            return (b.first < a.first);
    };
} mergedBarSorter;
};

const std::string PlotLibrary::loggerCat_("voreen.plotting.PlotLibrary");

PlotLibrary::PlotLibrary()
    : labelFont_(VoreenApplication::app()->getFontPath("Vera.ttf"))
    , windowSize_(256, 256)
    , plotLabelGroup_(&labelFont_, 5, tgt::Bounds(), tgt::Color(1, 1, 1, 0.75))
    , lineLabelGroup_(&labelFont_, 6, tgt::Bounds())
    , xAxisLabelGroup_(&labelFont_, 10, tgt::Bounds())
    , axisLabelGroup_(&labelFont_, 6)
    , dimension_(TWO)
    , drawingColor_(0.0f, 0.0f, 0.0f, 1.0f)
    , fillColor_(0.0f, 0.0f, 0.0f, 0.5f)
    , fontColor_(0.0f, 0.0f, 0.0f, 1.0f)
    , highlightColor_(1.0f, 0.95f, 0.9f, 0.5f)
    , colorMap_(ColorMap::createColdHot())
    , minGlyphSize_(1.0f)
    , maxGlyphSize_(1.0f)
    , lineWidth_(1.0f)
    , barWidth_(0.5)
    , axesWidth_(1.0f)
    , fontSize_(10)
    , squeezeFactor_(1.0)
    , lineStyle_(PlotEntitySettings::CONTINUOUS)
    , glyphStyle_(PlotEntitySettings::POINT)
    , barMode_(GROUPED)
    , marginLeft_(60)
    , marginRight_(100)
    , marginBottom_(50)
    , marginTop_(50)
    , shear_(tgt::vec2(0.f,0.f))
    , lightingFlag_(false)
    , orthographicCameraFlag_(true)
    , texture_(0)
    , ppm_(NULL)
    , usePlotPickingManager_(false)
{
    labelFont_.setSize(8);
    domain_[0] = Interval<plot_t>(0, 1, false, false);
    domain_[1] = Interval<plot_t>(0, 1, false, false);
    domain_[2] = Interval<plot_t>(0, 1, false, false);
    logarithmicAxisFlags_[0] = false;
    logarithmicAxisFlags_[1] = false;
    logarithmicAxisFlags_[2] = false;
    minimumScaleStep_[0] = 40;
    minimumScaleStep_[1] = 40;
    minimumScaleStep_[2] = 40;
}

bool PlotLibrary::setOpenGLStatus() {
    // helper domains
    plot_t xl = domain_[X_AXIS].getLeft(); plot_t xr = domain_[X_AXIS].getRight();
    plot_t yl = domain_[Y_AXIS].getLeft(); plot_t yr = domain_[Y_AXIS].getRight();
    plot_t zl = domain_[Z_AXIS].getLeft(); plot_t zr = domain_[Z_AXIS].getRight();
    // for 2D plots setup orthographic projection using margins
    if (dimension_ == TWO || dimension_ == FAKETHREE) {
        //check, if the canvas is big enough
        if (marginLeft_+marginRight_>=windowSize_.x || marginTop_+marginBottom_>=windowSize_.y)
            return false;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(xl-static_cast<double>(marginLeft_)/plotToViewportScale_.x
        , xr+static_cast<double>(marginRight_)/plotToViewportScale_.x
        , yl-static_cast<double>(marginBottom_)/plotToViewportScale_.y
        , yr+static_cast<double>(marginTop_)/plotToViewportScale_.y
        , zl-1, zr+1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        if (dimension_ == TWO) {
            //set up clipping planes
            tgt::dvec2 leftBottom = convertViewportToPlotCoordinates(tgt::ivec2(marginLeft_,marginBottom_));
            tgt::dvec2 rightTop = convertViewportToPlotCoordinates(tgt::ivec2(windowSize_.x-marginRight_,windowSize_.y-marginTop_));
            GLdouble eqnLeft[4] = {1.0, 0.0, 0.0, -leftBottom.x};
            GLdouble eqnRight[4] = {-1.0, 0.0, 0.0, rightTop.x};
            GLdouble eqnBottom[4] = {0.0, 1.0, 0.0, -leftBottom.y};
            GLdouble eqnTop[4] = {0.0, -1.0, 0.0, rightTop.y};
            glClipPlane (GL_CLIP_PLANE0, eqnLeft);   glClipPlane (GL_CLIP_PLANE1, eqnRight);
            glClipPlane (GL_CLIP_PLANE2, eqnBottom); glClipPlane (GL_CLIP_PLANE3, eqnTop);
        }
        else { //dimension_ == FAKETHREE
            //translation to the center
            glTranslated((xl+xr)/2,(yl+yr)/2,(zl+zr)/2);
            //rescaling
            glScaled(1/(1+shear_.x*(zr-zl)),1/(1+shear_.y*(zr-zl)),1);
            //shearing for fake 3d effect
            tgt::mat4 m = tgt::mat4(1,0,0,0,0,1,0,0,-shear_.x*static_cast<float>(xr-xl),
                -shear_.y*static_cast<float>(yr-yl),1,0,0,0,0,1);
            tgt::multTransposeMatrix(m);
            glTranslated(-(xl+xr)/2,-(yl+yr)/2,-(zl+zr)/2);
        }
    }
    // for 3D plot setup projection using the camera
    else if (dimension_ == THREE) {
        if (lightingFlag_ == true && usePlotPickingManager_ == false) {
            GLfloat ambientlight[] = {0.4, 0.4, 0.4, 1.0f};
            GLfloat diffuselight[] = {0.7, 0.7, 0.7, 1.0f};
            GLfloat position[] = {1.f,1.f, 1.f, 1.f};
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambientlight);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuselight);
            glLightfv(GL_LIGHT0, GL_POSITION, position);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
        }
        tgt::loadMatrix(camera_.getViewMatrix());
        //plot into [-0.5,0.5]^3
        glTranslated(-0.5,-0.5,-0.5);
        glScaled(1/domain_[0].size(),1/domain_[1].size(),1/domain_[2].size());
        glTranslated(-domain_[0].getLeft(),-domain_[1].getLeft(),-domain_[2].getLeft());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (orthographicCameraFlag_)
            glOrtho(-.9,.9,-.9,.9,-2,10);
        else
            tgt::loadMatrix(camera_.getProjectionMatrix());
        GLdouble eqnLeft[4] = {1.0, 0.0, 0.0, -domain_[X_AXIS].getLeft()};
        GLdouble eqnRight[4] = {-1.0, 0.0, 0.0, domain_[X_AXIS].getRight()};
        GLdouble eqnBottom[4] = {0.0, 1.0, 0.0, -domain_[Y_AXIS].getLeft()};
        GLdouble eqnTop[4] = {0.0, -1.0, 0.0, domain_[Y_AXIS].getRight()};
        GLdouble eqnFront[4] = {0.0, 0.0, 1.0, -domain_[Z_AXIS].getLeft()};
        GLdouble eqnBack[4] = {0.0, 0.0, -1.0, domain_[Z_AXIS].getRight()};
        glClipPlane (GL_CLIP_PLANE0, eqnLeft);   glClipPlane (GL_CLIP_PLANE1, eqnRight);
        glClipPlane (GL_CLIP_PLANE2, eqnBottom); glClipPlane (GL_CLIP_PLANE3, eqnTop);
        glClipPlane (GL_CLIP_PLANE4, eqnFront);  glClipPlane (GL_CLIP_PLANE5, eqnBack);
        // we need to calculate which are the outer edges (used for selection and
        // labeling the axes)
        calculateSelectionEdges();
    }
    glMatrixMode(GL_MODELVIEW);
    //some settings are not used for drawing pickable objects
    if (!usePlotPickingManager_) {
        //enable antialiasing
        glEnable (GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);
        //clear color buffer
        glClearColor(1.0,1.0,1.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    return true;
}

void PlotLibrary::renderLine(const PlotData& data, int indexX, int indexY) const {
    //row iterator
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin();
    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    //set up some opengl settings
    glEnable (GL_CLIP_PLANE0); glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2); glEnable (GL_CLIP_PLANE3);
    bool lineIsHighlighted = data.isHighlighted(tgt::ivec2(-1,indexY));
    glLineWidth(lineWidth_);
    glPointSize(maxGlyphSize_);
    glDisable(GL_DEPTH_TEST);
    //set line style
    if (lineStyle_!= PlotEntitySettings::CONTINUOUS) {
        glEnable(GL_LINE_STIPPLE);
        if (lineStyle_ == PlotEntitySettings::DOTTED)
            glLineStipple(1, 0x0101);
        else //DASHED
            glLineStipple(1, 0x00FF);
    }
    int i = 0;
    double x = 0.0; double y = 0.0; //they are set in the loop
    //go to the first row with non null entries
    while (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
        ++it; ++i;
    }
    if (it == data.getRowsEnd() || it == --data.getRowsEnd())
        return;

    // draw the line
    double oldX = tagsInX ? i : it->getValueAt(indexX);
    double oldY = it->getValueAt(indexY);
    if (usePlotPickingManager_)
        ppm_->setGLColor(-1, indexY);
    else if (lineIsHighlighted)
        glColor4fv(highlightColor_.elem);
    else
        glColor4fv(drawingColor_.elem);
    for (++it, ++i; it != data.getRowsEnd(); ++it, ++i) {
        //we ignore rows with null entries
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
            continue;
        }
        x = tagsInX ? i : it->getValueAt(indexX);
        y = it->getValueAt(indexY);
        glBegin(GL_LINES);
            logGlVertex2d(oldX, oldY);
            logGlVertex2d(x, y);
        glEnd();
        //if x is out of the interval, we leave the loop
        if (x>= domain_[X_AXIS].getRight())
            break;
        oldX = x;
        oldY = y;
    }

    // render the points
    glPointSize(maxGlyphSize_);
    glBegin(GL_POINTS);
    for (it = data.getRowsBegin(), i = 0; it != data.getRowsEnd(); ++it, ++i) {
        //we ignore rows with null entries
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
            continue;
        }
        x = tagsInX ? i : it->getValueAt(indexX);
        y = it->getValueAt(indexY);
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, indexY);
        else if (it->getCellAt(indexY).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(drawingColor_.elem);
        logGlVertex2d(x, y);
    }
    glEnd();

    glDisable (GL_CLIP_PLANE0); glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2); glDisable (GL_CLIP_PLANE3);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LINE_STIPPLE);
}

void PlotLibrary::renderSpline(const PlotData& data, int indexX, int indexY) const {
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin(); // row iterator
    tgt::Spline spline;
    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    bool lineIsHighlighted = data.isHighlighted(tgt::ivec2(-1,indexY));
    glLineWidth(lineWidth_);
    glDisable(GL_DEPTH_TEST);
    glEnable (GL_CLIP_PLANE0); glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2); glEnable (GL_CLIP_PLANE3);
    if (lineStyle_!= PlotEntitySettings::CONTINUOUS) { //set line style
        glEnable(GL_LINE_STIPPLE);
        if (lineStyle_ == PlotEntitySettings::DOTTED)
            glLineStipple(1, 0x0101);
        else //DASHED
            glLineStipple(1, 0x00FF);
    }
    int i = 0;
    //go to the first row with non null entrys
    while (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
        ++it; ++i;
    }
    if (it == data.getRowsEnd())
        return;

    // add the control points
    double x = tagsInX ? i : it->getValueAt(indexX);
    double y = it->getValueAt(indexY);
    spline.addControlPoint(tgt::dvec3(x,y,0));
    for (; it != data.getRowsEnd(); ++it, ++i) {
        //we ignore rows with null entries
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
            continue;
        }
        x = tagsInX ? i : it->getValueAt(indexX);
        y = it->getValueAt(indexY);
        spline.addControlPoint(tgt::dvec3(x,y,0));
    }
    spline.addControlPoint(tgt::dvec3(x,y,0));

    //render spline, possibly with log coordinates
    GLfloat step = 1.f /spline.getStepCount();
    if (usePlotPickingManager_)
        ppm_->setGLColor(-1, indexY);
    else if (lineIsHighlighted)
        glColor4fv(highlightColor_.elem);
    else
        glColor4fv(drawingColor_.elem);
    glBegin(GL_LINE_STRIP);
    for (GLfloat p = 0.f; p < 1.f; p+=step) {
        x = spline.getPoint(p).x;
        y = spline.getPoint(p).y;
        logGlVertex2d(x, y);
        if (x>= domain_[X_AXIS].getRight())
            break;
    }
    glEnd();

    // render the points
    glPointSize(maxGlyphSize_);
    glBegin(GL_POINTS);
    for (it = data.getRowsBegin(), i = 0; it != data.getRowsEnd(); ++it, ++i) {
        //we ignore rows with null entries
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
            continue;
        }
        x = tagsInX ? i : it->getValueAt(indexX);
        y = it->getValueAt(indexY);
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, indexY);
        else if (it->getCellAt(indexY).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(drawingColor_.elem);
        logGlVertex2d(x, y);
    }
    glEnd();

    glDisable (GL_CLIP_PLANE0); glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2); glDisable (GL_CLIP_PLANE3);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LINE_STIPPLE);
}

void PlotLibrary::renderErrorline(const PlotData& data, int indexX, int indexY, int indexError) const {
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin();
    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    glDisable(GL_DEPTH_TEST);
    glEnable (GL_CLIP_PLANE0); glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2); glEnable (GL_CLIP_PLANE3);

    int i = 0;
    //go to the first row with non null entrys
    while (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
        ++it; ++i;
    }
    if (it == data.getRowsEnd())
        return;

    double x             = tagsInX ? i : it->getValueAt(indexX);
    double errorTop      = it->getValueAt(indexY) + std::abs(it->getValueAt(indexError));
    double errorBottom   = it->getValueAt(indexY) - std::abs(it->getValueAt(indexError));
    if (usePlotPickingManager_)
        ppm_->setGLColor(-1, indexY);
    else
        glColor4fv(fillColor_.elem);
    // draw the errorline
    ++i;
    glBegin(GL_QUAD_STRIP);
        for (it = (++it); it != data.getRowsEnd(); ++it, ++i) {
            if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull())
                continue;
            logGlVertex2d(x, errorBottom);
            logGlVertex2d(x, errorTop);

            x = tagsInX ? i : it->getValueAt(indexX);
            errorTop = it->getValueAt(indexY) + std::abs(it->getValueAt(indexError));
            errorBottom = it->getValueAt(indexY) - std::abs(it->getValueAt(indexError));
        }
        logGlVertex2d(x, errorBottom);
        logGlVertex2d(x, errorTop);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glDisable (GL_CLIP_PLANE0);  glDisable (GL_CLIP_PLANE1);
    glDisable (GL_CLIP_PLANE2);  glDisable (GL_CLIP_PLANE3);
}

void PlotLibrary::renderErrorspline(const PlotData& data, int indexX, int indexY, int indexError) const {
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin();
    tgt::Spline splineTop, splineBottom;

    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    if (usePlotPickingManager_)
        ppm_->setGLColor(-1, indexY);
    else
        glColor4fv(fillColor_.elem);
    glDisable(GL_DEPTH_TEST);

    glEnable (GL_CLIP_PLANE0); glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2); glEnable (GL_CLIP_PLANE3);
    int i = 0;
    //go to the first row with non null entrys
    while (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
        ++it; ++i;
    }
    if (it == data.getRowsEnd())
        return;

    //fill the splines

    double x             = tagsInX ? i : it->getValueAt(indexX);
    double errorTop      = it->getValueAt(indexY) + std::abs(it->getValueAt(indexError));
    double errorBottom   = it->getValueAt(indexY) - std::abs(it->getValueAt(indexError));
    splineTop.addControlPoint(tgt::dvec3(x,errorTop,0));
    splineBottom.addControlPoint(tgt::dvec3(x,errorBottom,0));
    for (; it != data.getRowsEnd(); ++it, ++i) {
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull())
            continue;

        x = tagsInX ? i : it->getValueAt(indexX);
        if (it->getCellAt(indexError).isNull()){
            errorTop = it->getValueAt(indexY);
            errorBottom = it->getValueAt(indexY);
        } else {
            errorTop = it->getValueAt(indexY) + std::abs(it->getValueAt(indexError));
            errorBottom = it->getValueAt(indexY) - std::abs(it->getValueAt(indexError));
        }
        splineTop.addControlPoint(tgt::dvec3(x,errorTop,0));
        splineBottom.addControlPoint(tgt::dvec3(x,errorBottom,0));
    }
    splineTop.addControlPoint(tgt::dvec3(x,errorTop,0));
    splineBottom.addControlPoint(tgt::dvec3(x,errorBottom,0));

    //draw the thick spline
    GLfloat step = 1.f /splineTop.getStepCount();
    glBegin(GL_QUAD_STRIP);
    for (GLfloat p = 0.f; p < 1.f; p+=step) {
        logGlVertex2d(splineTop.getPoint(p).x, splineTop.getPoint(p).y);
        logGlVertex2d(splineBottom.getPoint(p).x, splineBottom.getPoint(p).y);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CLIP_PLANE0);  glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);  glDisable(GL_CLIP_PLANE3);
}

void PlotLibrary::renderErrorbars(const PlotData& data, int indexX, int indexY, int indexError) const{
    std::vector<PlotRowValue>::const_iterator it;
    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    glLineWidth(lineWidth_);
    glDisable(GL_DEPTH_TEST);
    glEnable (GL_CLIP_PLANE2);
    glEnable (GL_CLIP_PLANE3);

    float radius = static_cast<float>(domain_[X_AXIS].size()/(4.f*data.getRowsCount()));
    float aspectRatio = static_cast<float>(windowSize_.x)/static_cast<float>(windowSize_.y)*
        static_cast<float>(domain_[Y_AXIS].size() / domain_[X_AXIS].size());

    int i = 0;
    // draw the errorbars
    for (it = data.getRowsBegin(); it != data.getRowsEnd(); ++it, ++i) {
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull() || it->getCellAt(indexError).isNull()) {
            continue;
        }
        double x = tagsInX ? i : it->getValueAt(indexX);
        if (!domain_[X_AXIS].contains(x))
            continue;

        if (usePlotPickingManager_)
            ppm_->setGLColor(i, indexError);
        else if (it->getCellAt(indexError).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(drawingColor_.elem);

        double y = it->getValueAt(indexY);
        double yTop = it->getValueAt(indexY) + it->getValueAt(indexError);
        double yBottom = it->getValueAt(indexY) - it->getValueAt(indexError);
        x = logarithmicAxisFlags_[X_AXIS] ? convertToLogCoordinates(x, X_AXIS) : x;
        y = logarithmicAxisFlags_[Y_AXIS] ? convertToLogCoordinates(y, Y_AXIS) : y;
        yTop = logarithmicAxisFlags_[Y_AXIS] ? convertToLogCoordinates(yTop, Y_AXIS) : yTop;
        yBottom = logarithmicAxisFlags_[Y_AXIS] ? convertToLogCoordinates(yBottom, Y_AXIS) : yBottom;
        glBegin(GL_LINES);
            glVertex2d(x, yBottom);
            glVertex2d(x, yTop);
        glEnd();
        glBegin(GL_LINES);
            glVertex2d(x-radius, yTop);
            glVertex2d(x+radius, yTop);
        glEnd();
        glBegin(GL_LINES);
            glVertex2d(x-radius, yBottom);
            glVertex2d(x+radius, yBottom);
        glEnd();

        tgt::Ellipse midpoint(tgt::dvec3(x, y, domain_[2].getLeft()), radius/2,
                              (radius*aspectRatio)/2, tgt::dvec3(0, 0, 1), tgt::dvec3(1, 0, 0 ), 32);
        midpoint.render();
    }
    glEnable(GL_DEPTH_TEST);
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
}

void PlotLibrary::renderCandlesticks(const PlotData& data, int indexX, int stickTop,
                                     int stickBottom, int candleTop, int candleBottom) const {
    std::vector<PlotRowValue>::const_iterator it;

    // check if only values or only tags in given cells
    bool tagsInX = (data.getColumnType(indexX) == PlotBase::STRING);
    // openGL settings
    glDisable(GL_DEPTH_TEST);
    glEnable (GL_CLIP_PLANE0); glEnable (GL_CLIP_PLANE1);
    glEnable (GL_CLIP_PLANE2); glEnable (GL_CLIP_PLANE3);
    glLineWidth(lineWidth_);

    float width = static_cast<float>(domain_[X_AXIS].size()/(4.f*data.getRowsCount()));

    // draw the candlestick
    int i=0;
    double yStickTop     = 0.0;
    double yStickBottom  = 0.0;
    double yCandleTop    = 0.0;
    double yCandleBottom = 0.0;

    for (it = data.getRowsBegin(); it != data.getRowsEnd(); ++it, ++i) {
        if (it->getCellAt(indexX).isNull() || it->getCellAt(stickTop).isNull() || it->getCellAt(stickBottom).isNull()
            || it->getCellAt(candleTop).isNull() || it->getCellAt(candleBottom).isNull()) {
            continue;
        }
        double x = tagsInX ? i : it->getValueAt(indexX);
        yStickTop     = it->getValueAt(stickTop);
        yStickBottom  = it->getValueAt(stickBottom);
        yCandleTop    = it->getValueAt(candleTop);
        yCandleBottom = it->getValueAt(candleBottom);

        // we divide the stick and the candle in top and bottom half
        // draw stick
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, stickTop);
        else if (it->getCellAt(stickTop).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(drawingColor_.elem);
        glBegin(GL_LINES);
            logGlVertex2d(x, yStickTop);
            logGlVertex2d(x, (yStickTop+yStickBottom)/2.0);
        glEnd();
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, stickBottom);
        else if (it->getCellAt(stickBottom).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(drawingColor_.elem);
        glBegin(GL_LINES);
            logGlVertex2d(x, (yStickTop+yStickBottom)/2.0);
            logGlVertex2d(x, yStickBottom);
        glEnd();
        //draw candle
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, candleTop);
        else if (it->getCellAt(candleTop).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(fillColor_.elem);
        glBegin(GL_POLYGON);
            logGlVertex2d(x-width, yCandleTop);
            logGlVertex2d(x-width, (yCandleBottom+yCandleTop)/2.0);
            logGlVertex2d(x+width, (yCandleBottom+yCandleTop)/2.0);
            logGlVertex2d(x+width, yCandleTop);
        glEnd();
        if (usePlotPickingManager_)
            ppm_->setGLColor(i, candleBottom);
        else if (it->getCellAt(candleBottom).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else
            glColor4fv(fillColor_.elem);
        glBegin(GL_POLYGON);
            logGlVertex2d(x-width, (yCandleBottom+yCandleTop)/2.0);
            logGlVertex2d(x-width, yCandleBottom);
            logGlVertex2d(x+width, yCandleBottom);
            logGlVertex2d(x+width, (yCandleBottom+yCandleTop)/2.0);
        glEnd();
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CLIP_PLANE0); glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2); glDisable(GL_CLIP_PLANE3);
}

void PlotLibrary::renderSurface(const PlotData& data, const std::vector<int>& triangleVertexIndices, bool wire,
                                int indexX, int indexY, int indexZ, int indexCM)  const {
    // security check: if count of edge indices is not a multiple of 3 abort
    if (triangleVertexIndices.size() % 3 != 0)
        return;

    glEnable(GL_CLIP_PLANE0); glEnable(GL_CLIP_PLANE1);
    glEnable(GL_CLIP_PLANE2); glEnable(GL_CLIP_PLANE3);
    glEnable(GL_CLIP_PLANE4); glEnable(GL_CLIP_PLANE5);

    Interval<plot_t> colInterval(0, 0);

    if (wire)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0, 1.0);
    }
    if ( indexCM != -1 ) {
        colInterval = data.getInterval(indexCM);
        if (colInterval.size() == 0)
            indexCM = -1;
    }
    if ( indexCM == -1)
        glColor4fv(drawingColor_.elem);

    glLineWidth(lineWidth_);
    // draw the triangles
    std::set<int> renderedHighlights; // we want to render each plot label // highlight only once
    for (std::vector<int>::const_iterator it = triangleVertexIndices.begin(); it < triangleVertexIndices.end(); it += 3) {
        // render plot picking data?
        if (usePlotPickingManager_) {
            // to write out the PlotCell information to the ppm, we subdivide our triangle into three
            // quads, and render each with the according encoded color
            const PlotRowValue& rowA = data.getRow(*it);
            const PlotRowValue& rowB = data.getRow(*(it+1));
            const PlotRowValue& rowC = data.getRow(*(it+2));

            tgt::dvec3 a(rowA.getValueAt(indexX), rowA.getValueAt(indexY), rowA.getValueAt(indexZ));
            tgt::dvec3 b(rowB.getValueAt(indexX), rowB.getValueAt(indexY), rowB.getValueAt(indexZ));
            tgt::dvec3 c(rowC.getValueAt(indexX), rowC.getValueAt(indexY), rowC.getValueAt(indexZ));

            // 3 linear interpolations
            tgt::dvec3 ab = a + 0.5*(b-a);
            tgt::dvec3 bc = b + 0.5*(c-b);
            tgt::dvec3 ca = c + 0.5*(a-c);

            tgt::dvec3 circumcenter = (a + b + c)*(1.0/3.0);

            ppm_->setGLColor(*it, indexZ);
            glBegin(GL_POLYGON);
                glVertex3dv(a.elem);
                glVertex3dv(ab.elem);
                glVertex3dv(circumcenter.elem);
                glVertex3dv(ca.elem);
            glEnd();

            ppm_->setGLColor(*(it+1), indexZ);
            glBegin(GL_POLYGON);
                glVertex3dv(b.elem);
                glVertex3dv(bc.elem);
                glVertex3dv(circumcenter.elem);
                glVertex3dv(ab.elem);
            glEnd();

            ppm_->setGLColor(*(it+2), indexZ);
            glBegin(GL_POLYGON);
                glVertex3dv(c.elem);
                glVertex3dv(ca.elem);
                glVertex3dv(circumcenter.elem);
                glVertex3dv(bc.elem);
            glEnd();

        }
        // else render plot
        else {
            // render triangle first
            glBegin(GL_POLYGON);
            for (int i=0; i<3; ++i) {
                const PlotRowValue& row = data.getRow(*(it+i));

                if (indexCM != -1 ) {
                    float c = static_cast<float>((row.getValueAt(indexCM) - colInterval.getLeft()) / colInterval.size());
                    tgt::Color cc = colorMap_.getColorAtPosition(c);
                    glColor4fv(cc.elem);
                }
                if (!wire && row.getCellAt(indexZ).isHighlighted()) {
                    glColor4fv(highlightColor_.elem);
                    glVertex3d(row.getValueAt(indexX), row.getValueAt(indexY), row.getValueAt(indexZ));
                    glColor4fv(drawingColor_.elem);
                }
                else
                    glVertex3d(row.getValueAt(indexX), row.getValueAt(indexY), row.getValueAt(indexZ));
            }
            glEnd();
        }
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_CLIP_PLANE0); glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2); glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4); glDisable(GL_CLIP_PLANE5);
}

void PlotLibrary::renderHeightmap(const voreen::PlotData& data, const std::vector< std::list< tgt::dvec2 > >& voronoiRegions,
                                  bool wire, int indexZ, int indexCM) const {
    Interval<plot_t> colInterval(0, 0);
    plot_t yMin = data.getInterval(2).getLeft();

    if (wire)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    else {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0, 1.0);
    }
    if ( indexCM != -1 ) {
        colInterval = data.getInterval(indexCM);
        if (colInterval.size() == 0)
            indexCM = -1;
    }
    glLineWidth(lineWidth_);

    int row = 0;
    std::vector< std::list< tgt::dvec2 > >::const_iterator rit = voronoiRegions.begin();
    std::vector<PlotRowValue>::const_iterator pit = data.getRowsBegin();
    for (; rit < voronoiRegions.end(); ++rit, ++pit, ++row){
        if (rit->empty())
            continue;

        if (usePlotPickingManager_)
            ppm_->setGLColor(row, indexZ);
        else if (!wire && pit->getCellAt(indexZ).isHighlighted())
            glColor4fv(highlightColor_.elem);
        else if (indexCM != -1 ) {
            float c = static_cast<float>((pit->getValueAt(indexCM) - colInterval.getLeft()) / colInterval.size());
            glColor4fv(colorMap_.getColorAtPosition(c).elem);
        }
        else
            glColor4fv(drawingColor_.elem);

        plot_t height = pit->getValueAt(indexZ);
        //clip it:
        if (height > domain_[Z_AXIS].getRight())
            height = domain_[Z_AXIS].getRight();
        if (yMin < domain_[Z_AXIS].getLeft())
            yMin = domain_[Z_AXIS].getLeft();
        if (height > yMin) {
            // render the sides of the pillar
            glBegin(GL_QUAD_STRIP);
            for (std::list< tgt::dvec2 >::const_iterator eit = rit->begin(); eit != rit->end(); ++eit) {
                glVertex3d(eit->x, eit->y, height);
                glVertex3d(eit->x, eit->y, yMin);
            }
            glVertex3d(rit->begin()->x, rit->begin()->y, height);
            glVertex3d(rit->begin()->x, rit->begin()->y, yMin);
            glEnd();
            // render the top of the pillar
            glBegin(GL_POLYGON);
            for (std::list< tgt::dvec2 >::const_iterator eit = rit->begin(); eit != rit->end(); ++eit) {
                glVertex3d(eit->x, eit->y, height);
            }
            glEnd();
            // render the bottom of the pillar
            glBegin(GL_POLYGON);
            for (std::list< tgt::dvec2 >::const_iterator eit = rit->begin(); eit != rit->end(); ++eit) {
                glVertex3d(eit->x, eit->y, yMin);
            }
            glEnd();
        }
    }
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void PlotLibrary::renderBars(const PlotData& data, std::vector<int> indicesY) const {
    std::vector<PlotRowValue>::const_iterator rowIt = data.getRowsBegin();
    //stores y values and indices of a merged bar group, the indices are used to get the right color
    std::vector<std::pair<plot_t, tgt::Color> > mergedBars;
    //stores last y value, used for stacked bars
    plot_t lastY;
    tgt::Color c;
    //rowcounter
    plot_t row = 0;
    for (; rowIt < data.getRowsEnd(); ++rowIt) {
        lastY = 0;
        mergedBars.clear();
        // we do not use the iterator because we also iterate through the colormap
        for (size_t i = 0; i < indicesY.size(); ++i) {
            if (rowIt->getCellAt(indicesY.at(i)).isHighlighted())
                c = highlightColor_;
            else
                c = colorMap_.getColorAtIndex(i);
            if (barMode_ == STACKED) {
                if (rowIt->getCellAt(indicesY.at(i)).isNull() )
                    continue;
                plot_t newY = lastY+rowIt->getValueAt(indicesY.at(i));
                //negative stacked bars do not make any sense and are ignored
                if (newY < lastY)
                    continue;
                if (usePlotPickingManager_)
                    ppm_->setGLColor(static_cast<int>(row), indicesY.at(i));
                renderSingleBar(row-barWidth_/2.0,row+barWidth_/2.0, lastY, newY, c);
                lastY = newY;
            }
            else if (barMode_ == GROUPED) {
                if (rowIt->getCellAt(indicesY.at(i)).isNull())
                    continue;
                double singleBarWidth = barWidth_/(1.0*static_cast<double>(indicesY.size()));
                if (usePlotPickingManager_)
                    ppm_->setGLColor(static_cast<int>(row), indicesY.at(i));
                renderSingleBar(row-barWidth_/2.0+static_cast<double>(i)*singleBarWidth,
                    row-barWidth_/2.0+static_cast<double>(i+1)*singleBarWidth, 0, rowIt->getValueAt(indicesY.at(i)), c);
            }

            else { // MERGED
                // we can't skip null entries, so we set them 0
                if (usePlotPickingManager_)
                    c = ppm_->convertColor(ppm_->getColorFromCell(static_cast<int>(row), indicesY.at(i)));
                if (rowIt->getCellAt(indicesY.at(i)).isNull())
                    mergedBars.push_back(std::pair<plot_t, tgt::Color>(0, c));
                // push the y value and the color index
                mergedBars.push_back(std::pair<plot_t, tgt::Color>(rowIt->getValueAt(indicesY.at(i)), c));
            }
        }
        if (barMode_ == MERGED) {
            // the values are stored in bars, but not yet drawn
            std::sort(mergedBars.begin(), mergedBars.end(), mergedBarSorter);
            std::vector<std::pair<plot_t, tgt::Color> >::const_iterator it;
            double squeeze = 1.0;
            for (it = mergedBars.begin(); it < mergedBars.end(); ++it) {
                glColor4fv(it->second.elem);
                renderSingleBar(row-barWidth_/2.0,row+barWidth_/2.0, 0, it->first, it->second, squeeze);
                squeeze = squeezeFactor_*squeeze;
            }
        }
        ++row;
    }
}

void PlotLibrary::renderNodeGraph(const PlotData& nodeData, const PlotData& connectionData, int indexX, int indexY, int indexDx, int indexDy) const {
    std::vector<PlotRowValue>::const_iterator it;

    // render nodes
    plot_t glyphSize = (maxGlyphSize_ + minGlyphSize_)/2;
    int i = 0;
    for (it = nodeData.getRowsBegin(); it != nodeData.getRowsEnd(); ++it, ++i) {
        // render node
        glColor4fv(drawingColor_.elem);
        renderGlyph(it->getValueAt(indexX), it->getValueAt(indexY), 0, glyphSize);

        // render force vector
        glColor4fv(fillColor_.elem);
        glBegin(GL_LINE_STRIP);
            logGlVertex2d(it->getValueAt(indexX), it->getValueAt(indexY));
            logGlVertex2d(it->getValueAt(indexX) + it->getValueAt(indexDx), it->getValueAt(indexY) + it->getValueAt(indexDy));
        glEnd();

        // render node label
        std::stringstream ss;
        ss << i;
        renderLabel(tgt::dvec3(it->getValueAt(indexX), it->getValueAt(indexY), 0), SmartLabel::CENTERED, ss.str(), false, 0);
    }

    // render connections
    glLineWidth(lineWidth_);
    glColor4fv(drawingColor_.elem);
    for (it = connectionData.getRowsBegin(); it != connectionData.getRowsEnd(); ++it) {
        const PlotRowValue& first = nodeData.getRow(static_cast<int>(it->getValueAt(0)));
        const PlotRowValue& second = nodeData.getRow(static_cast<int>(it->getValueAt(1)));
        glBegin(GL_LINE_STRIP);
            logGlVertex2d(first.getValueAt(indexX), first.getValueAt(indexY));
            logGlVertex2d(second.getValueAt(indexX), second.getValueAt(indexY));
        glEnd();
    }
}

void PlotLibrary::renderColorMapLegend(const PlotData& data, int column, int number) const {
    // ColorMaps with less than 2 colors may not exist
    if (colorMap_.getColorCount() < 2)
        return;

    Interval<plot_t>   interval = data.getInterval(column);
    const std::string& label    = data.getColumnLabel(column);

    // switch to viewport coordinates
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // render legend
    int colorcount      = colorMap_.getColorCount();
    const double width  = 96;
    const double height = 16;
    const double xStart = windowSize_.x - width - 8;
    const double yStart = windowSize_.y - 8 - (number*32);
    double stepWidth    = width / (colorcount - 1);

    // color map
    glColor4fv(colorMap_.getColorAtIndex(0).elem);
    for (int i = 0; i < colorcount-1; ++i) {
        glBegin(GL_POLYGON);
            glVertex2d(xStart + (i*stepWidth), yStart);
            glVertex2d(xStart + (i*stepWidth), yStart - height);
            glColor4fv(colorMap_.getColorAtIndex(i + 1).elem);
            glVertex2d(xStart + ((i+1) * stepWidth), yStart - height);
            glVertex2d(xStart + ((i+1) * stepWidth), yStart);
        glEnd();
    }

    // bounding box
    glColor4fv(drawingColor_.elem);
    glLineWidth(lineWidth_);
    glBegin(GL_LINE_STRIP);
        glVertex2d(xStart, yStart);
        glVertex2d(xStart, yStart - height);
        glVertex2d(xStart + width, yStart - height);
        glVertex2d(xStart + width, yStart);
        glVertex2d(xStart, yStart);
    glEnd();

    // labels
    SmartLabelGroupBase::renderSingleLabel(&labelFont_, label, tgt::dvec3(xStart, yStart - (height/2), 0),
            fontColor_, fontSize_, SmartLabel::MIDDLERIGHT, 4);

    std::stringstream ss;
    ss << std::setprecision(4) << interval.getLeft();
    SmartLabelGroupBase::renderSingleLabel(&labelFont_, ss.str(), tgt::dvec3(xStart, yStart - height, 0),
            fontColor_, fontSize_, SmartLabel::BOTTOMCENTERED, 4);

    ss.str("");
    ss.clear();
    ss << std::setprecision(4) << interval.getRight();
    SmartLabelGroupBase::renderSingleLabel(&labelFont_, ss.str(), tgt::dvec3(xStart + width, yStart - height, 0),
            fontColor_, fontSize_, SmartLabel::BOTTOMCENTERED, 4);

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;
}

void PlotLibrary::renderScatter(const PlotData& data, int indexX, int indexY, int indexZ, int indexCM, int indexSize) const {
    if (lightingFlag_ == true && usePlotPickingManager_ == false && glyphStyle_ != PlotEntitySettings::POINT) {
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
    }
    Interval<plot_t> colInterval(0, 0);
    if ( indexCM != -1 ) {
        colInterval = data.getInterval(indexCM);
        if (colInterval.size() == 0)
            indexCM = -1;
    }
    plot_t x, y, z;
    plot_t size = maxGlyphSize_;
    // if there is size data, we interpolate it to [minGlyphSize_,maxGlyphSize_]
    Interval<plot_t> sizeInterval(0, 0);
    if ( indexSize != -1 ) {
        sizeInterval = data.getInterval(indexSize);
        //we can only use an interval with positive size
        if (sizeInterval.size() == 0)
            indexSize = -1;
    }
    //row iterator
    int i = 0;
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin();

    for (; it != data.getRowsEnd(); ++it, ++i) {
        //we ignore rows with null entries
        if (it->getCellAt(indexX).isNull() || it->getCellAt(indexY).isNull()) {
            continue;
        }
        x = it->getValueAt(indexX); y = it->getValueAt(indexY); z = (indexZ == -1 ? 0 : it->getValueAt(indexZ));
        //check if the point is inside the domains
        if (domain_[X_AXIS].contains(x) && domain_[Y_AXIS].contains(y) && (dimension_ == TWO || domain_[Z_AXIS].contains(z))) {
            // set color
            if (usePlotPickingManager_)
                ppm_->setGLColor(i,indexZ == -1 ? indexY : indexZ);
            else if ((indexZ != -1 && it->getCellAt(indexZ).isHighlighted())
                    || (indexZ == -1 && it->getCellAt(indexY).isHighlighted()))
                glColor4fv(highlightColor_.elem);
            else if (indexCM != -1 ) {
                float c = static_cast<float>((it->getValueAt(indexCM) - colInterval.getLeft()) / colInterval.size());
                tgt::Color cc = colorMap_.getColorAtPosition(c);
                glColor4fv(cc.elem);
            }
            else
                glColor4fv(drawingColor_.elem);
            // set size
            if (indexSize != -1 ) {
                size = minGlyphSize_ + (maxGlyphSize_ - minGlyphSize_) *
                            (it->getValueAt(indexSize) - sizeInterval.getLeft()) / sizeInterval.size();
            }
            renderGlyph(x, y, z, size);
        }
    }
    if (lightingFlag_ == true && usePlotPickingManager_ == false && glyphStyle_ != PlotEntitySettings::POINT) {
        glDisable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
    }
}

void PlotLibrary::renderAxes() const {
    // axes
    if (dimension_ == TWO)
        glDisable(GL_DEPTH_TEST);

    glLineWidth(axesWidth_);
    glColor4fv(drawingColor_.elem);

    plot_t xl = domain_[0].getLeft();    plot_t xr = domain_[0].getRight();
    plot_t yl = domain_[1].getLeft();    plot_t yr = domain_[1].getRight();
    plot_t zl = domain_[2].getLeft();    plot_t zr = domain_[2].getRight();
    //x and y axes
    glBegin(GL_LINE_STRIP);
        glVertex3d(xl, yr, zl);
        glVertex3d(xl, yl, zl);
        glVertex3d(xr, yl, zl);
    glEnd();

    if (dimension_ == TWO) {
    //draw arrows with viewport coordinates
        int arrowSize = 5; // in pixel
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glBegin(GL_LINES);
            glVertex2i(windowSize_.x-marginRight_, marginBottom_);
            glVertex2i(windowSize_.x-marginRight_+4*arrowSize, marginBottom_);
            glVertex2i(windowSize_.x-marginRight_+2*arrowSize, marginBottom_+arrowSize);
            glVertex2i(windowSize_.x-marginRight_+4*arrowSize, marginBottom_);
            glVertex2i(windowSize_.x-marginRight_+2*arrowSize, marginBottom_-arrowSize);
            glVertex2i(windowSize_.x-marginRight_+4*arrowSize, marginBottom_);

            glVertex2i(marginLeft_, windowSize_.y-marginTop_);
            glVertex2i(marginLeft_, windowSize_.y-marginTop_+4*arrowSize);
            glVertex2i(marginLeft_ + arrowSize, windowSize_.y-marginTop_+2*arrowSize);
            glVertex2i(marginLeft_, windowSize_.y-marginTop_+4*arrowSize);
            glVertex2i(marginLeft_ - arrowSize, windowSize_.y-marginTop_+2*arrowSize);
            glVertex2i(marginLeft_, windowSize_.y-marginTop_+4*arrowSize);
        glEnd();
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
    else if (dimension_ == THREE) {
    //draw cube mesh
        glBegin(GL_LINE_STRIP);
            glVertex3d(xr, yl, zl); glVertex3d(xr, yr, zl);
            glVertex3d(xl, yr, zl); glVertex3d(xl, yr, zr);
            glVertex3d(xr, yr, zr); glVertex3d(xr, yr, zl);
        glEnd();
        glBegin(GL_LINE_STRIP);
            glVertex3d(xr, yl, zl); glVertex3d(xr, yl, zr);
            glVertex3d(xl, yl, zr); glVertex3d(xl, yl, zl);
        glEnd();
        glBegin(GL_LINES);
            glVertex3d(xr, yl, zr); glVertex3d(xr, yr, zr);
            glVertex3d(xl, yl, zr); glVertex3d(xl, yr, zr);
        glEnd();
    }
    else if (dimension_ == FAKETHREE) {
        //draw back
        glBegin(GL_LINE_LOOP);
            glVertex3d(xl, yr, zl); glVertex3d(xl, yl, zl);
            glVertex3d(xr, yl, zl); glVertex3d(xr, yr, zl);
        glEnd();

        //draw bottom
        glBegin(GL_LINES);
            glVertex3d(xl, yl, zl); glVertex3d(xl, yl, zr);
            glVertex3d(xr, yl, zr); glVertex3d(xr, yl, zl);
        glEnd();

        //draw left
         glBegin(GL_LINE_LOOP);
            glVertex3d(xl, yr, zr); glVertex3d(xl, yl, zr);
            glVertex3d(xl, yl, zl); glVertex3d(xl, yr, zl);
        glEnd();

        //draw zero
        glBegin(GL_LINES);
            glVertex3d(xl, 0, zl); glVertex3d(xl, 0, zr);
            glVertex3d(xr, 0, zr); glVertex3d(xr, 0, zl);
        glEnd();

        //the front is always above the plot
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_LINES);
            glVertex3d(xl, 0, zr);
            glVertex3d(xr, 0, zr);
        glEnd();
        glBegin(GL_LINES);
            glVertex3d(xl, yl, zr);
            glVertex3d(xr, yl, zr);
        glEnd();
    }
    glEnable(GL_DEPTH_TEST);
}

void PlotLibrary::renderAxisScales(Axis axis, bool helperLines, const std::string& label) const {
    tgt::dvec2 step = updateScaleSteps(axis);
    glLineWidth(axesWidth_/2.f);
    glColor4fv(drawingColor_.elem);
    plot_t xl = domain_[0].getLeft();    plot_t xr = domain_[0].getRight();
    plot_t yl = domain_[1].getLeft();    plot_t yr = domain_[1].getRight();
    plot_t zl = domain_[2].getLeft();    plot_t zr = domain_[2].getRight();

    std::stringstream stream;
    if (step.x < 1) {
        int precision = static_cast<int>(ceil(log10(1.0/step.x)));
        stream << std::fixed << std::setprecision(precision);
    }

    if (dimension_ == TWO || dimension_ == FAKETHREE) {
        xAxisLabelGroup_.reset();
        xAxisLabelGroup_.setBounds(getBoundsBelowPlot());
        // if respective scaleStep is e.g. 50 and interval begin e.g. 27.43, we want to have labels like
        // 50, 100, 150, ... instead of 27.43, 77.43, 127.43, ...
        // So we do some smart rounding:
        plot_t start = domain_[axis].getLeft();
        start = ceil(start / step.x) * step.x;

        for (plot_t i = start; i <= domain_[axis].getRight(); i += step.x) {
            plot_t pos = i;//(logarithmicAxisFlags_[axis] ? convertFromLogCoordinates(i, axis) : i);
            stream.str("");
            stream.clear();
            stream << round(pos, step.y);
            if (axis == X_AXIS) {
                xAxisLabelGroup_.addLabel(stream.str(),
                                          convertPlotCoordinatesToViewport3(tgt::dvec3(i, yl, zr)),
                                          fontColor_, fontSize_, SmartLabel::TOPCENTERED);
                if (helperLines && dimension_ == TWO) {
                    glBegin(GL_LINE_STRIP);
                        logGlVertex2d(i, yl);
                        logGlVertex2d(i, yr);
                    glEnd();
                }
            }
            else if (axis == Y_AXIS) {
                renderLabel(tgt::dvec3(xl, i, zr), SmartLabel::MIDDLERIGHT, stream.str());
                if (helperLines) {
                    glBegin(GL_LINE_STRIP);
                        if (dimension_ == FAKETHREE)
                            logGlVertex3d(xl, i, zr);
                        logGlVertex3d(xl, i, zl);
                        logGlVertex3d(xr, i, zl);
                    glEnd();
                }
            }
        }
        renderSmartLabelGroup(&xAxisLabelGroup_);
    }
    else if (dimension_ == THREE) {
        // If we are inside the plot cube (or really close to it) we do not want to
        // render scales because it looks ugly it doesn't make any sense.
        if (!orthographicCameraFlag_ && tgt::distance(camera_.getPosition(), tgt::vec3(0,0,0)) < 1)
            return;

        axisLabelGroup_.reset();

        // avoid expensive copying by using iterators
        std::vector<SelectionEdge>::const_iterator minEdge = selectionEdgesX_.begin();
        std::vector<SelectionEdge>::const_iterator endEdge = selectionEdgesX_.end();
        if (axis == X_AXIS) {
            if (selectionEdgesX_.empty())
                return;
        }
        else if (axis == Y_AXIS) {
            if (selectionEdgesY_.empty())
                return;
            minEdge = selectionEdgesY_.begin();
            endEdge = selectionEdgesY_.end();
        }
        else if (axis == Z_AXIS) {
            if (selectionEdgesZ_.empty())
                return;
            minEdge = selectionEdgesZ_.begin();
            endEdge = selectionEdgesZ_.end();
        }

        // find edge with maximum length
        double length = tgt::length(minEdge->endVertex_ - minEdge->startVertex_);

        if (! orthographicCameraFlag_) {
            for (std::vector<SelectionEdge>::const_iterator it = ++minEdge; it < endEdge; ++it) {
                double val = tgt::length(it->endVertex_ - it->startVertex_);
                if (val > length) {
                    minEdge = it;
                    length = val;
                }
            }
        }

        // determine on which side the cube our axis is and with that the label alignment
        tgt::dvec2 edgeDirection = minEdge->endVertex_ - minEdge->startVertex_;
        tgt::dvec2 center(windowSize_.x/2, windowSize_.y/2);
        tgt::dvec2 ray((minEdge->startVertex_ + 0.5*edgeDirection) - center);
        ray = 1.0/tgt::length(ray) * ray;
        double angle = atan2(ray.y, ray.x);

        SmartLabel::Alignment align = SmartLabel::MIDDLELEFT;
        if (angle > tgt::PI/8 && angle <= 3.0*tgt::PI/8)
            align = SmartLabel::TOPLEFT;
        else if (angle > 3.0*tgt::PI/8 && angle <= 5.0*tgt::PI/8)
            align = SmartLabel::TOPCENTERED;
        else if (angle > 5.0*tgt::PI/8 && angle <= 7.0*tgt::PI/8)
            align = SmartLabel::TOPRIGHT;
        else if (angle < -7.0*tgt::PI/8 || angle > 7.0*tgt::PI/8)
            align = SmartLabel::MIDDLERIGHT;

        if (angle < -tgt::PI/8 && angle >= -3.0*tgt::PI/8)
            align = SmartLabel::BOTTOMLEFT;
        else if (angle < -3.0*tgt::PI/8 && angle >= -5.0*tgt::PI/8)
            align = SmartLabel::BOTTOMCENTERED;
        else if (angle < -5.0*tgt::PI/8 && angle >= -7.0*tgt::PI/8)
            align = SmartLabel::BOTTOMRIGHT;

        // render axis label first:
        if (label != "") {
            axisLabelGroup_.addLabel(label, minEdge->startVertex_ + (0.5 * edgeDirection) + (32.0 * ray), fontColor_, fontSize_, align);
        }

        // now render scales:
        // if respective scaleStep is e.g. 50 and interval begin e.g. 27.43, we want to have labels like
        // 50, 100, 150, ... instead of 27.43, 77.43, 127.43, ...
        // So we do some smart rounding:
        plot_t start = domain_[axis].getLeft();
        start = ceil(start / step.x) * step.x;
        plot_t domainSize = domain_[axis].size();

        for (plot_t i = start; i  <= domain_[axis].getRight(); i += step.x) {
            stream.str("");
            stream.clear();
            stream << round(i, step.y);
            if (minEdge->ascOrientation_)
                axisLabelGroup_.addLabel(stream.str(),
                        minEdge->startVertex_ + ((i - domain_[axis].getLeft())/domainSize)*edgeDirection,
                        fontColor_, fontSize_, align);
            else
                axisLabelGroup_.addLabel(stream.str(),
                        minEdge->endVertex_ - ((i - domain_[axis].getLeft())/domainSize)*edgeDirection,
                        fontColor_, fontSize_, align);

        }
        renderSmartLabelGroup(&axisLabelGroup_);

        // render helper lines
        if (helperLines) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            if (axis == X_AXIS) {
                for (plot_t i = start + step.x ; i <= domain_[axis].getRight(); i += step.x) {
                    glBegin(GL_QUADS);
                        glVertex3d(i - step.x, yl, zl); glVertex3d(i, yl, zl);
                        glVertex3d(i, yr, zl);          glVertex3d(i - step.x, yr, zl);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(i, yl, zr);          glVertex3d(i - step.x, yl, zr);
                        glVertex3d(i - step.x, yr, zr); glVertex3d(i, yr, zr);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(i - step.x, yl, zl); glVertex3d(i - step.x, yl, zr);
                        glVertex3d(i, yl, zr);          glVertex3d(i, yl, zl);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(i, yr, zl);          glVertex3d(i, yr, zr);
                        glVertex3d(i - step.x, yr, zr); glVertex3d(i - step.x, yr, zl);
                    glEnd();
                }
            }
            else if (axis == Y_AXIS) {
                for (plot_t i = start + step.x ; i <= domain_[axis].getRight(); i += step.x) {
                    glBegin(GL_QUADS);
                        glVertex3d(xr, i - step.x, zl); glVertex3d(xr, i, zl);
                        glVertex3d(xl, i, zl);          glVertex3d(xl, i - step.x, zl);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xr, i, zr);          glVertex3d(xr, i - step.x, zr);
                        glVertex3d(xl, i - step.x, zr); glVertex3d(xl, i, zr);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xl, i - step.x, zr); glVertex3d(xl, i - step.x, zl);
                        glVertex3d(xl, i, zl);          glVertex3d(xl, i, zr);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xr, i, zr);          glVertex3d(xr, i,  zl);
                        glVertex3d(xr ,i - step.x, zl); glVertex3d(xr, i - step.x, zr);
                    glEnd();
                }
            }
            else if (axis == Z_AXIS) {
                for (plot_t i = start + step.x ; i <= domain_[axis].getRight(); i += step.x) {
                    glBegin(GL_QUADS);
                        glVertex3d(xr, yr, i - step.x); glVertex3d(xr, yr, i);
                        glVertex3d(xl, yr, i);          glVertex3d(xl, yr, i - step.x);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xr, yl, i);          glVertex3d(xr, yl, i - step.x);
                        glVertex3d(xl, yl, i - step.x); glVertex3d(xl, yl, i);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xl, yl, i - step.x); glVertex3d(xl, yr, i - step.x);
                        glVertex3d(xl, yr, i);          glVertex3d(xl, yl, i);
                    glEnd();
                    glBegin(GL_QUADS);
                        glVertex3d(xr, yl, i);          glVertex3d(xr, yr, i);
                        glVertex3d(xr, yr, i - step.x); glVertex3d(xr, yl,i - step.x);
                    glEnd();
                }
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_CULL_FACE);
        }
    }
}

void PlotLibrary::renderAxisLabelScales(const PlotData& data, int indexLabel, bool helperLines) const {
    std::string label;
    xAxisLabelGroup_.reset();
    xAxisLabelGroup_.setBounds(getBoundsBelowPlot());
    std::vector<PlotRowValue>::const_iterator it = data.getRowsBegin();
    plot_t x = 0;
    glLineWidth(axesWidth_/2.f);
    glColor4fv(drawingColor_.elem);
    for (;it!=data.getRowsEnd();++it) {
        if (!domain_[X_AXIS].contains(x) || it->getCellAt(indexLabel).isNull()){
            x += 1;
            continue;
        }
        if (data.getColumnType(indexLabel) == PlotBase::STRING)
            label = it->getTagAt(indexLabel);
        else {
            std::ostringstream stream;
            stream << it->getValueAt(indexLabel);
            label = stream.str();
        }
        xAxisLabelGroup_.addLabel(label,
                                  convertPlotCoordinatesToViewport3(tgt::dvec3(x, domain_[1].getLeft(), domain_[2].getRight())),
                                  fontColor_, fontSize_, SmartLabel::TOPCENTERED);
        if (helperLines && dimension_ == TWO) {
            glBegin(GL_LINE_STRIP);
                logGlVertex2d(x, domain_[1].getLeft());
                logGlVertex2d(x, domain_[1].getRight());
            glEnd();
        }
        x += 1;
    }
    renderSmartLabelGroup(&xAxisLabelGroup_);
}

void PlotLibrary::renderAxisLabel(Axis axis, const std::string& label) const {
    switch (axis) {
        case X_AXIS:
            renderLabel(tgt::dvec3(domain_[0].getRight(), domain_[1].getLeft(),
                domain_[2].getRight()), SmartLabel::BOTTOMLEFT, label, false, 15);
            break;
        case Y_AXIS:
            renderLabel(tgt::dvec3(domain_[0].getLeft(), domain_[1].getRight(),
                domain_[2].getRight()), SmartLabel::TOPRIGHT, label, false, 15);
            break;
        case Z_AXIS:
            // should not be reached
            LWARNINGC("PlotLibrary", "this line PlotLibrary::renderAxisLabel should not be reached!");
            break;
    }
}

void PlotLibrary::renderLabel(tgt::vec3 pos, const SmartLabel::Alignment align, const std::string& text,
                              bool viewCoordinates, int padding) const {
    if (!viewCoordinates)
        pos = convertPlotCoordinatesToViewport3(pos);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    SmartLabelGroupBase::renderSingleLabel(&labelFont_, text, pos, fontColor_, fontSize_, align, static_cast<float>(padding));
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;
}

void PlotLibrary::renderLabel(tgt::dvec2 pos, const SmartLabel::Alignment align, const std::string& text, int padding) const {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    SmartLabelGroupBase::renderSingleLabel(&labelFont_, text, tgt::vec3((float)pos.x, (float)pos.y, 0), fontColor_,
            fontSize_, align, static_cast<float>(padding));
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;
}

void PlotLibrary::addPlotLabel(std::string text, tgt::vec3 position, tgt::Color color,
                               int size, SmartLabel::Alignment align) {
    plotLabelGroup_.addLabel(text, position, color, size, align);
}

void PlotLibrary::addLineLabel(std::string text, tgt::vec3 position, tgt::Color color,
                               int size, SmartLabel::Alignment align) {
    lineLabelGroup_.addLabel(text, position, color, size, align);
}

//
// helper functions
//

void PlotLibrary::resetLineLabels() {
    lineLabelGroup_.reset();
    lineLabelGroup_.setBounds(getBoundsRightOfPlot());
}

void PlotLibrary::renderLineLabels() {
    renderSmartLabelGroup(&lineLabelGroup_);
}

void PlotLibrary::resetPlotLabels() {
    plotLabelGroup_.reset();
    plotLabelGroup_.setBounds(getBoundsPlot());
}

void PlotLibrary::renderPlotLabels() {
    renderSmartLabelGroup(&plotLabelGroup_);
}

void PlotLibrary::renderSmartLabelGroup(SmartLabelGroupBase* smg) const {
    smg->performLayout();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    smg->render();
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    LGL_ERROR;
}

void PlotLibrary::renderMousePosition(tgt::ivec2 position) {
    if (dimension_ == TWO) {
        tgt::dvec2 plotPosition = convertViewportToPlotCoordinates(position);
        std::stringstream ss;
        ss << "x: ";
        if (domain_[X_AXIS].contains(plotPosition.x))
            ss << plotPosition.x;
        else
            ss << "-";
        ss << " y: ";
        if (domain_[Y_AXIS].contains(plotPosition.y))
            ss << plotPosition.y;
        else
            ss << "-";
        renderLabel(tgt::ivec3(0, 0, 0), SmartLabel::TOPLEFT, ss.str(), true, 5);
    }
    else if (dimension_ == THREE) {
        // check if position was outside of the cube and which axis to zoom
        tgt::dvec2 center(windowSize_.x/2, windowSize_.y/2);
        tgt::dvec2 startPosition(position.x, position.y);

        int selectionEdgeIndex = -1; // -1 means no axis found
        SelectionEdge selectionEdge;
        std::stringstream ss;

        std::vector<SelectionEdge>::const_iterator it;
        // check against x-zoom-axes
        for (it = getSelectionEdgesX().begin(); it < getSelectionEdgesX().end(); ++it) {
            if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
                && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
                && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                selectionEdge = *it;
                selectionEdgeIndex = 0;
                ss << "x-axis: ";
            }
        }
        if (selectionEdgeIndex == -1) {
            // check against y-zoom-axes
            for (it = getSelectionEdgesY().begin(); it < getSelectionEdgesY().end(); ++it) {
                if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
                    && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
                    && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                    selectionEdge = *it;
                    selectionEdgeIndex = 1;
                    ss << "y-axis: ";
                }
            }
        }
        if (selectionEdgeIndex == -1) {
            // check against z-zoom-axes
            for (it = getSelectionEdgesZ().begin(); it < getSelectionEdgesZ().end(); ++it) {
                if (PlotLibrary::rightTurn(center, startPosition, it->startVertex_)
                    && PlotLibrary::leftTurn(center, startPosition, it->endVertex_)
                    && PlotLibrary::rightTurn(it->startVertex_, it->endVertex_, startPosition)) {
                    selectionEdge = *it;
                    selectionEdgeIndex = 2;
                    ss << "z-axis: ";
                }
            }
        }
        // if selectionEdgeIndex != -1, we are outside of the cube
        if (selectionEdgeIndex != -1) {
            tgtAssert(selectionEdgeIndex >= 0 && selectionEdgeIndex <= 2,
                    "PlotLibrary::renderMousePosition(): selectionEdgeIndex must be in [0, 2]");

            // determine position of the zoom clipping planes by orthogonal projection of
            // start-click onto the zoom edge (scaled dot product)
            tgt::dvec2 edge = selectionEdge.endVertex_ - selectionEdge.startVertex_;
            tgt::dvec2 click = startPosition - selectionEdge.startVertex_;
            double dot = (edge.x*click.x + edge.y*click.y) / (tgt::length(edge) * tgt::length(edge));

            // calculate plot coordinates of zoom clipping plane
            plot_t plotPosition;
            if (selectionEdge.ascOrientation_)
                plotPosition = domain_[selectionEdgeIndex].getLeft() + (dot*domain_[selectionEdgeIndex].size());
            else
                plotPosition = domain_[selectionEdgeIndex].getRight() - (dot*domain_[selectionEdgeIndex].size());

            if (domain_[selectionEdgeIndex].contains(plotPosition))
                ss << plotPosition;
            else
                ss << "-";
        }

        renderLabel(tgt::ivec3(0, 0, 0), SmartLabel::TOPLEFT, ss.str(), true, 5);
    }
}

namespace {
    /// tiny private function decoding array indices
    inline int decode(const std::valarray<int>& indices) {
        return 4*indices[0] + 2*indices[1] + indices[2];
    }
}

void PlotLibrary::findEdges(const tgt::dvec2* vertices, const std::vector< std::valarray< int > > indices,
                            std::vector< SelectionEdge >& out, PlotLibrary::Axis axis) const {
    // for each edge
    for (int i=0; i<4; ++i) {
        // check if it is an outer edge (part of convex hull) by checking if it meets the leftTurn predicate
        // with the start points of all other edges
        if (   leftTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+1)%4])])
            && leftTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+2)%4])])
            && leftTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+3)%4])]))
            out.push_back(SelectionEdge(axis, vertices[decode(indices[i])], vertices[decode(indices[i+4])], true));

        // same with rightTurn predicate -> then the inverse edge is part of the convex hull
        if (   rightTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+1)%4])])
            && rightTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+2)%4])])
            && rightTurn(vertices[decode(indices[i])], vertices[decode(indices[i+4])], vertices[decode(indices[(i+3)%4])]))
            out.push_back(SelectionEdge(axis, vertices[decode(indices[i+4])], vertices[decode(indices[i])], false));
    }
}

void PlotLibrary::calculateSelectionEdges() {
    if (dimension_ != THREE)
        return;

    const plot_t& xmin = domain_[X_AXIS].getLeft();
    const plot_t& xmax = domain_[X_AXIS].getRight();
    const plot_t& ymin = domain_[Y_AXIS].getLeft();
    const plot_t& ymax = domain_[Y_AXIS].getRight();
    const plot_t& zmin = domain_[Z_AXIS].getLeft();
    const plot_t& zmax = domain_[Z_AXIS].getRight();

    // ----------------------------------------------------------------------
    // REMARK:
    //
    // We start a small battle of indices here to determine the outer edges
    // for each axis. At first sight this might look a bit odd, but doing
    // it this way save LOADS of lines of code! Otherwise we would
    // have to unroll each loop and hardcode the vertices. That would be
    // even more complex and harder to maintain.
    // ----------------------------------------------------------------------

    // index bits encoding which vertex to take:
    // most significant bit encodes x-axis, middle bit encodes y-axis, least significant bit z-axis
    // 0 is minimum, 1 is maximum,
    // decoding is done by local function decode(x, y, z) and gives according index of the vertices array
    int bits[24]  = {
     // x, y, z
        0, 0, 0,    // these index bits are aligned so that for each row i
        0, 0, 1,    // the vertices decoded by row i and row (i+4) form
        0, 1, 0,    // an x-axis edge.
        0, 1, 1,    // cyclically shifting each of these rows by one transforms
        1, 0, 0,    // the index bits so that they do the same for y-axis,
        1, 0, 1,    // and z-axis
        1, 1, 0,
        1, 1, 1
    };

    // get projected coordinates of all 8 vertices and put them into an array at the
    // right position so that they can be decoded later by the index bits
    tgt::dvec2 vertices[8] = {
        convertPlotCoordinatesToViewport(tgt::dvec3(xmin, ymin, zmin)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmin, ymin, zmax)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmin, ymax, zmin)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmin, ymax, zmax)),

        convertPlotCoordinatesToViewport(tgt::dvec3(xmax, ymin, zmin)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmax, ymin, zmax)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmax, ymax, zmin)),
        convertPlotCoordinatesToViewport(tgt::dvec3(xmax, ymax, zmax))
    };

    // fill valarrays with bits, each valarray gets one row
    std::vector<std::valarray<int> > indices;
    for (int i=0; i<8; ++i) {
        indices.push_back(std::valarray<int>(bits+(3*i), 3));
    }

    // find edges fox x-axis
    selectionEdgesX_.clear();
    findEdges(vertices, indices, selectionEdgesX_, X_AXIS);

    // cyclically shift each valarray 1 step right to get bits encoding y-axis comparisions
    for (std::vector<std::valarray<int> >::iterator it = indices.begin(); it < indices.end(); ++it)
        *it = it->cshift(-1);
    selectionEdgesY_.clear();
    findEdges(vertices, indices, selectionEdgesY_, Y_AXIS);

    // cyclically shift each valarray 1 step right to get bits encoding z-axis comparisions
    for (std::vector<std::valarray<int> >::iterator it = indices.begin(); it < indices.end(); ++it)
        *it = it->cshift(-1);
    selectionEdgesZ_.clear();
    findEdges(vertices, indices, selectionEdgesZ_, Z_AXIS);
}

const std::vector<PlotLibrary::SelectionEdge>& PlotLibrary::getSelectionEdgesX() const {
    return selectionEdgesX_;
}

const std::vector<PlotLibrary::SelectionEdge>& PlotLibrary::getSelectionEdgesY() const {
    return selectionEdgesY_;
}

const std::vector<PlotLibrary::SelectionEdge>& PlotLibrary::getSelectionEdgesZ() const {
    return selectionEdgesZ_;
}

double PlotLibrary::orientation(const tgt::dvec2& a, const tgt::dvec2& b, const tgt::dvec2& c) {
    return a.x*b.y + a.y*c.x +  b.x*c.y - c.x*b.y - c.y*a.x - b.x*a.y;
}

bool PlotLibrary::leftTurn(const tgt::dvec2& a, const tgt::dvec2& b, const tgt::dvec2& c) {
    return (orientation(a, b, c) > 0);
}

bool PlotLibrary::rightTurn(const tgt::dvec2& a, const tgt::dvec2& b, const tgt::dvec2& c) {
    return (orientation(a, b, c) < 0);
}

tgt::Bounds PlotLibrary::getBoundsAbovePlot() const {
    plot_t yScale = domain_[1].size() / (windowSize_.y - marginTop_ - marginBottom_);
    return tgt::Bounds(tgt::dvec3(domain_[0].getLeft(), domain_[1].getRight(), domain_[2].getLeft()),
                       tgt::dvec3(domain_[0].getRight(), domain_[1].getRight() + yScale*marginTop_, domain_[2].getLeft()));
}

tgt::Bounds PlotLibrary::getBoundsPlot() const {
    if (dimension_ != THREE)
        return tgt::Bounds(tgt::ivec3(marginLeft_, marginBottom_, 1), tgt::ivec3(windowSize_.x-marginRight_, windowSize_.y-marginTop_, 0));
    else
        return tgt::Bounds(tgt::ivec3(0, 0, 1), tgt::ivec3(windowSize_.x, windowSize_.y, 0));
}

tgt::vec3 PlotLibrary::getCenterAbovePlot() const {
    return tgt::vec3(static_cast<float>(windowSize_.x/2), static_cast<float>(windowSize_.y-marginTop_/2), 0);
}

tgt::Bounds PlotLibrary::getBoundsBelowPlot() const {
    return tgt::Bounds(tgt::ivec3(0, 0, 0),
                       tgt::ivec3(windowSize_.x, marginBottom_, 0));
}


tgt::Bounds PlotLibrary::getBoundsRightOfPlot() const {
    return tgt::Bounds(tgt::ivec3(windowSize_.x - marginRight_, marginBottom_, 0),
                       tgt::ivec3(windowSize_.x, windowSize_.y, 0));
}

plot_t PlotLibrary::convertToLogCoordinates(plot_t value, Axis axis) const {
    //normalize the interval to [1,1+intervalLength], apply log and fit it to the original domain
    plot_t logValue = log(value-domain_[axis].getLeft()+1);
    plot_t strechedLogValue = logValue * domain_[axis].size() / log(1+domain_[axis].size());
    return strechedLogValue + domain_[axis].getLeft();
}

plot_t PlotLibrary::convertFromLogCoordinates(plot_t value, Axis axis) const {
    //inverse of convertToLogCoordinates
    plot_t strechedLogValue = value - domain_[axis].getLeft();
    plot_t logValue = strechedLogValue * log(1 + domain_[axis].size()) / domain_[axis].size();
    return exp(logValue) + domain_[axis].getLeft() - 1;
}

tgt::dvec2 PlotLibrary::convertPlotCoordinatesToViewport(const tgt::dvec3& plotCoordinates) const {
    tgt::dvec3 copy(plotCoordinates);
    if (logarithmicAxisFlags_[X_AXIS])
        copy.x = convertToLogCoordinates(plotCoordinates.x, X_AXIS);
    if (logarithmicAxisFlags_[Y_AXIS])
        copy.y = convertToLogCoordinates(plotCoordinates.y, Y_AXIS);
    if (logarithmicAxisFlags_[Z_AXIS])
        copy.z = convertToLogCoordinates(plotCoordinates.z, Z_AXIS);

    GLdouble x, y, z;
    GLdouble mv[16];   glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    GLdouble pr[16];   glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    gluProject(copy.x, copy.y, copy.z, mv, pr, viewport, &x, &y, &z);
    return tgt::dvec2(x, y);
}

tgt::dvec3 PlotLibrary::convertPlotCoordinatesToViewport3(const tgt::dvec3& plotCoordinates) const {
    tgt::dvec3 copy(plotCoordinates);
    if (logarithmicAxisFlags_[X_AXIS])
        copy.x = convertToLogCoordinates(plotCoordinates.x, X_AXIS);
    if (logarithmicAxisFlags_[Y_AXIS])
        copy.y = convertToLogCoordinates(plotCoordinates.y, Y_AXIS);
    if (logarithmicAxisFlags_[Z_AXIS])
        copy.z = convertToLogCoordinates(plotCoordinates.z, Z_AXIS);

    GLdouble x, y, z;
    GLdouble mv[16];   glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    GLdouble pr[16];   glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    gluProject(copy.x, copy.y, copy.z, mv, pr, viewport, &x, &y, &z);
    return tgt::dvec3(x, y, z);
}

tgt::dvec2 PlotLibrary::updateScaleSteps(Axis axis) const {
    if (domain_[axis].size() <= 0)
        return tgt::dvec2(0,0);

    double viewportsize = 1;
    if (dimension_ == TWO || dimension_ == FAKETHREE) {
        viewportsize = ((axis == X_AXIS) ? windowSize_.x-marginLeft_-marginRight_ : windowSize_.y-marginTop_-marginBottom_);
    }
    else if (dimension_ == THREE) {
        tgt::dvec2 ray;
        if (axis == X_AXIS) {
            tgt::dvec2 start = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getLeft(), domain_[Y_AXIS].getLeft(), domain_[Z_AXIS].getLeft()));
            tgt::dvec2 stop = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getRight(), domain_[Y_AXIS].getLeft(), domain_[Z_AXIS].getLeft()));
            ray = stop - start;
        }
        else if (axis == Y_AXIS) {
            tgt::dvec2 start = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getLeft(), domain_[Y_AXIS].getLeft(), domain_[Z_AXIS].getLeft()));
            tgt::dvec2 stop = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getLeft(), domain_[Y_AXIS].getRight(), domain_[Z_AXIS].getLeft()));
            ray = stop - start;
        }
        else if (axis == Z_AXIS) {
            tgt::dvec2 start = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getLeft(), domain_[Y_AXIS].getLeft(), domain_[Z_AXIS].getLeft()));
            tgt::dvec2 stop = convertPlotCoordinatesToViewport(tgt::dvec3(
                domain_[X_AXIS].getLeft(), domain_[Y_AXIS].getLeft(), domain_[Z_AXIS].getRight()));
            ray = stop - start;
        }

        viewportsize = tgt::length(ray);
    }
    // due to numerical erosion viewportsize can be 0 leading to infinite loops - we do not want that.
    if (viewportsize <= 0)
        viewportsize = .1;

    double maxStepCount = viewportsize / minimumScaleStep_[axis];
    double minStep = domain_[axis].size() / maxStepCount;
    double minBase = log10(minStep);
    if (minBase >= 0) {
        double exp = 0;
        double testStep = minStep;
        while (testStep > 7.5f) {
            testStep /= 10;
            ++exp;
        }
        if (testStep > 2.5f)
            return tgt::dvec2(5.0*pow(10, exp),pow(10, exp) / 100);
        else if (testStep > 1.5f)
            return tgt::dvec2(2.0*pow(10, exp),pow(10, exp) / 100);
        else
            return tgt::dvec2(1.0*pow(10, exp),pow(10, exp) / 100);
    }
    else {
        double exp = 0;
        double testStep = minStep;
        while (testStep < .75f) {
            testStep *= 10;
            --exp;
        }
        if (testStep < 1.5f)
            return tgt::dvec2(1.0*pow(10, exp),pow(10, exp) / 100);
        else if (testStep < 2.5f)
            return tgt::dvec2(2.0*pow(10, exp),pow(10, exp) / 100);
        else
            return tgt::dvec2(5.0*pow(10, exp),pow(10, exp) / 100);
    }
}

plot_t PlotLibrary::round(plot_t value, plot_t roundingParameter) const {
    return floor( value / roundingParameter + 0.5 ) * roundingParameter;
}

void PlotLibrary::resetOpenGLStatus() {
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glLineWidth(1.f);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
}

void PlotLibrary::logGlVertex2d(plot_t x, plot_t y) const {
    if (logarithmicAxisFlags_[X_AXIS])
        x = convertToLogCoordinates(x, X_AXIS);
    if (logarithmicAxisFlags_[Y_AXIS])
        y = convertToLogCoordinates(y, Y_AXIS);
    glVertex2d(x, y);
}

void PlotLibrary::logGlVertex3d(plot_t x, plot_t y, plot_t z) const {
    if (logarithmicAxisFlags_[X_AXIS])
        x = convertToLogCoordinates(x, X_AXIS);
    if (logarithmicAxisFlags_[Y_AXIS])
        y = convertToLogCoordinates(y, Y_AXIS);
    if (logarithmicAxisFlags_[Z_AXIS])
        z = convertToLogCoordinates(z, Z_AXIS);
    glVertex3d(x, y, z);
}

tgt::dvec2 PlotLibrary::convertViewportToPlotCoordinates(tgt::ivec2 viewCoord) const {
    GLdouble x, y, z;
    GLdouble mv[16];   glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    GLdouble pr[16];   glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    gluUnProject(static_cast<double>(viewCoord.x), static_cast<double>(viewCoord.y), 0.0, mv, pr, viewport, &x, &y, &z);
    if (logarithmicAxisFlags_[X_AXIS])
        x = convertFromLogCoordinates(x, X_AXIS);
    if (logarithmicAxisFlags_[Y_AXIS])
        y = convertFromLogCoordinates(y, Y_AXIS);
    return tgt::dvec2(x, y);
}

namespace {
    // To avoid frequent con- and destruction of quadrics which might be very expensive
    // we instantiate here all quadrics which might be used to render glyphs locally.
    // This way each quadric exists only once during the whole lifetime of this class
    //
    // Hopefully it is ok to initialize them that early, but I haven't found any note
    // in the docs which said we need a valid OpenGL context.
    tgt::Disk        disk(0, 1, 16, 1, true, false);
    tgt::Triangle    triangle(1, true, false);
    tgt::Rect        rect(1, 1, true, false);
    tgt::Sphere      sphere(1, 12, 12, true, false);
    tgt::Tetrahedron tetrahedron(1, true, false);
    tgt::Quad        quad(1, 1, 1, true, false);
}

void PlotLibrary::renderGlyph(plot_t x, plot_t y, plot_t z, plot_t size) const {
    if (glyphStyle_ == PlotEntitySettings::POINT) {
        glPointSize(static_cast<float>(size));
        glBegin(GL_POINTS);
            glVertex3d(x,y,z);
        glEnd();
        return;
    }
    glPushMatrix();
    glTranslated(x,y,z);
    glScaled(domain_[X_AXIS].size()/500, domain_[Y_AXIS].size()/500, domain_[Z_AXIS].size()/500);
    if (texture_ != 0 && !usePlotPickingManager_) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        texture_->bind();
    }
    if (dimension_ == TWO){
        glDisable(GL_DEPTH_TEST);
        if (glyphStyle_ == PlotEntitySettings::CIRCLE){
            disk.setOuterRadius(size/2);
            if (texture_ != 0 && !usePlotPickingManager_)
                disk.setTextureEnabled(true);
            disk.render();
        }
        else if (glyphStyle_ == PlotEntitySettings::TRIANGLE){
            triangle.setSize(size);
            if (texture_ != 0 && !usePlotPickingManager_)
                triangle.setTextureEnabled(true);
            triangle.render();
        }
        else if (glyphStyle_ == PlotEntitySettings::QUAD){
            rect.setHeight(size);
            rect.setWidth(size);
            if (texture_ != 0 && !usePlotPickingManager_)
                rect.setTextureEnabled(true);
            rect.render();
        }
    }
    else if (dimension_ == THREE){
        if (glyphStyle_ == PlotEntitySettings::CIRCLE){
            sphere.setRadius(size/2);
            if (texture_ != 0 && !usePlotPickingManager_)
                sphere.setTextureEnabled(true);
            sphere.render();
        }
        else if (glyphStyle_ == PlotEntitySettings::TRIANGLE){
            tetrahedron.setSize(size);
            if (texture_ != 0 && !usePlotPickingManager_)
                tetrahedron.setTextureEnabled(true);
            tetrahedron.render();
        }
        else if (glyphStyle_ == PlotEntitySettings::QUAD){
            quad.setWidth(size);
            quad.setHeight(size);
            quad.setDepth(size);
            if (texture_ != 0 && !usePlotPickingManager_)
                quad.setTextureEnabled(true);
            quad.render();
        }
    }
    if (texture_ != 0 && !usePlotPickingManager_)
        glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    LGL_ERROR;
}

void PlotLibrary::renderSingleBar(plot_t left, plot_t right, plot_t bottom, plot_t top, tgt::Color c, plot_t squeeze) const {
    plot_t back = domain_[Z_AXIS].getLeft();
    plot_t front = domain_[Z_AXIS].getRight();
    //squeeze
    left = (left+squeeze*left+right-squeeze*right)/2.0;
    right = (left-squeeze*left+right+squeeze*right)/2.0;
    back = (back+squeeze*back+front-squeeze*front)/2.0;
    front = (back-squeeze*back+front+squeeze*front)/2.0;

    for (int i = 0; i < 2; ++i) {
        // fill color
        if (i==0) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
        }
        else { //border color
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            if (!usePlotPickingManager_)
                glColor4f(0.2f*c.r, 0.2f*c.g, 0.2f*c.b, 1.0f);
            else
                break;
        }

        //right
        if (i==0 && !usePlotPickingManager_)
            glColor4f(0.8f*c.r, 0.8f*c.g, 0.8f*c.b, 1.0f);
        glBegin(GL_QUADS);
            glVertex3d(right, bottom, front); glVertex3d(right, top, front);
            glVertex3d(right, top, back);     glVertex3d(right, bottom, back);
        glEnd();

        //back
        glBegin(GL_QUADS);
            glVertex3d(right, bottom, back);  glVertex3d(right, top, back);
            glVertex3d(left, top, back);      glVertex3d(left, bottom, back);
        glEnd();

        //top
        if (i==0 && !usePlotPickingManager_)
            glColor4f(0.9f*c.r, 0.9f*c.g, 0.9f*c.b, 1.0f);
        glBegin(GL_QUADS);
            glVertex3d(left, top, front);     glVertex3d(left, top, back);
            glVertex3d(right, top, back);     glVertex3d(right, top, front);
        glEnd();

        //bottom
        glBegin(GL_QUADS);
            glVertex3d(left, bottom, front);  glVertex3d(left, bottom, back);
            glVertex3d(right, bottom, back);  glVertex3d(right, bottom, front);
        glEnd();

        if (i==0 && !usePlotPickingManager_)
            glColor4f(c.r, c.g, c.b, 1.0f);
        //front
        glBegin(GL_QUADS);
            glVertex3d(left, bottom, front);  glVertex3d(left, top, front);
            glVertex3d(right, top, front);    glVertex3d(right, bottom, front);
        glEnd();
        //left
        glBegin(GL_QUADS);
            glVertex3d(left, bottom, front);  glVertex3d(left, top, front);
            glVertex3d(left, top, back);      glVertex3d(left, bottom, back);
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

//
// getters and setters
//

void PlotLibrary::setDimension(Dimension dim){
    dimension_ = dim;
}

void PlotLibrary::setDomain(const Interval<plot_t>& interval, Axis axis) {
    if (interval.empty()) //this does not make much sense, so we take just [0,1]
        domain_[axis] = Interval<plot_t>(0.0, 1.0, false, false);
    else if (interval.size()==0) // not empty but interval [x, x], x \in \R
        domain_[axis] = Interval<plot_t>(interval.getLeft()-0.1, interval.getRight()+0.1, false, false);
    else
        domain_[axis] = interval;
    renewPlotToViewportScale();
}

void PlotLibrary::setLogarithmicAxis(bool polar, Axis axis){
    logarithmicAxisFlags_[axis] = polar;
}

void PlotLibrary::setDrawingColor(tgt::Color color) {
    drawingColor_ = color;
}

void PlotLibrary::setFillColor(tgt::Color color) {
    fillColor_ = color;
}

void PlotLibrary::setFontColor(tgt::Color color) {
    fontColor_ = color;
}

void PlotLibrary::setHighlightColor(tgt::Color color) {
    highlightColor_ = color;
}

void PlotLibrary::setColorMap(ColorMap cm) {
    colorMap_ = cm;
}

void PlotLibrary::setLineWidth(float width) {
    if (width >= 0)
        lineWidth_ = width;
}

void PlotLibrary::setBarWidth(double width) {
    if (width >= 0 && width <= 100)
        barWidth_ = width;
}

void PlotLibrary::setAxesWidth(float width) {
    if (width >= 0)
        axesWidth_ = width;
}

void PlotLibrary::setFontSize(int size) {
    if (size > 0)
        fontSize_ = size;
}

void PlotLibrary::setLineStyle(PlotEntitySettings::LineStyle style) {
    lineStyle_ = style;
}

void PlotLibrary::setGlyphStyle(PlotEntitySettings::GlyphStyle style) {
    glyphStyle_ = style;
}


void PlotLibrary::setMarginLeft(int margin) {
    if (margin >= 0) {
        marginLeft_ = margin;
        renewPlotToViewportScale();
    }
}

void PlotLibrary::setMarginRight(int margin) {
    if (margin >= 0) {
        marginRight_ = margin;
        renewPlotToViewportScale();
    }
}

void PlotLibrary::setMarginTop(int margin) {
    if (margin >= 0) {
        marginTop_ = margin;
        renewPlotToViewportScale();
    }
}

void PlotLibrary::setMarginBottom(int margin) {
    if (margin >= 0) {
        marginBottom_ = margin;
        renewPlotToViewportScale();
    }
}

void PlotLibrary::setWindowSize(const tgt::ivec2 windowSize) {
    windowSize_ = windowSize;
    renewPlotToViewportScale();
}

void PlotLibrary::setBarGroupingMode(BarGroupingMode mode) {
    barMode_ = mode;
}

void PlotLibrary::renewPlotToViewportScale() {
    plotToViewportScale_.x = static_cast<double>(windowSize_.x-marginLeft_-marginRight_)/(domain_[0].size());
    plotToViewportScale_.y = static_cast<double>(windowSize_.y-marginBottom_-marginTop_)/(domain_[1].size());
}

void PlotLibrary::setMinimumScaleStep(int value, Axis axis) {
    if (value > 0)
        minimumScaleStep_[axis] = value;
}

void PlotLibrary::setCamera(const tgt::Camera& camera) {
    camera_ = camera;
}

void PlotLibrary::setShear(tgt::vec2 shear) {
    shear_ = shear;
}

void PlotLibrary::setSqueezeFactor(double sf) {
    squeezeFactor_ = sf;
}

void PlotLibrary::setLightingFlag(bool value) {
    lightingFlag_ = value;
}

void PlotLibrary::setOrthographicCameraFlag(bool value) {
    orthographicCameraFlag_ = value;
}

void PlotLibrary::setMinGlyphSize(float value) {
    minGlyphSize_ = value;
}

void PlotLibrary::setMaxGlyphSize(float value) {
    maxGlyphSize_ = value;
}

void PlotLibrary::setTexture(tgt::Texture* texture) {
    TexMgr.dispose(texture_);
    LGL_ERROR;
    texture_ = texture;
}


void PlotLibrary::setPlotPickingManager(PlotPickingManager* ppm) {
    ppm_ = ppm;
}

void PlotLibrary::setUsePlotPickingManager(bool value) {
    usePlotPickingManager_ = value;
}

} // namespace
