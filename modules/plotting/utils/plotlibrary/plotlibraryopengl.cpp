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

#include "plotlibraryopengl.h"
#include "../../datastructures/plotrow.h"

#include "voreen/core/voreenapplication.h"
#include "../../interaction/plotpickingmanager.h"

#include "tgt/glmath.h"
#include "tgt/tgt_math.h"
#include "tgt/spline.h"
#include "tgt/quadric.h"

#include <iomanip>

namespace voreen {

const std::string PlotLibraryOpenGl::loggerCat_("voreen.plotting.PlotLibraryOpenGl");

PlotLibraryOpenGl::PlotLibraryOpenGl()
    : PlotLibraryNoneFileBase()
    , plotLabelGroup_(&labelFont_, 5, tgt::Bounds(), tgt::Color(1, 1, 1, 0.75))
    , lineLabelGroup_(&labelFont_, 6, tgt::Bounds())
    , xAxisLabelGroup_(&labelFont_, 10, tgt::Bounds())
    , axisLabelGroup_(&labelFont_, 6)
{
}

PlotLibraryOpenGl::~PlotLibraryOpenGl() {
}

bool PlotLibraryOpenGl::setRenderStatus() {
    // helper domains
    plot_t xl = domain_[X_AXIS].getLeft(); plot_t xr = domain_[X_AXIS].getRight();
    plot_t yl = domain_[Y_AXIS].getLeft(); plot_t yr = domain_[Y_AXIS].getRight();
    plot_t zl = domain_[Z_AXIS].getLeft(); plot_t zr = domain_[Z_AXIS].getRight();
    // for 2D plots setup orthographic projection using margins
    if (dimension_ == TWO || dimension_ == FAKETHREE) {
        //check, if the canvas is big enough
        if (marginLeft_+marginRight_>=windowSize_.x || marginTop_+marginBottom_>=windowSize_.y)
            return false;
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.loadIdentity();
        glOrtho(xl-static_cast<double>(marginLeft_)/plotToViewportScale_.x
        , xr+static_cast<double>(marginRight_)/plotToViewportScale_.x
        , yl-static_cast<double>(marginBottom_)/plotToViewportScale_.y
        , yr+static_cast<double>(marginTop_)/plotToViewportScale_.y
        , zl-1, zr+1);
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.loadIdentity();
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
        MatStack.loadMatrix(camera_.getViewMatrix());
        //plot into [-0.5,0.5]^3
        glTranslated(-0.5,-0.5,-0.5);
        glScaled(1/domain_[0].size(),1/domain_[1].size(),1/domain_[2].size());
        glTranslated(-domain_[0].getLeft(),-domain_[1].getLeft(),-domain_[2].getLeft());
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.loadIdentity();
        if (orthographicCameraFlag_)
            glOrtho(-.9,.9,-.9,.9,-2,10);
        else
            MatStack.loadMatrix(camera_.getProjectionMatrix(windowSize_));
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
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    //some settings are not used for drawing pickable objects
    if (!usePlotPickingManager_) {
        //enable antialiasing
        glEnable (GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        // FIXME Polygon smoothing does not seem to be well supported by many graphics cards; it often leads to visible seams between triangles.
        // This setting will be disabled for bar plots for now (see renderSingleBar()). FL
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_POLYGON_SMOOTH);
        //clear color buffer
        glClearColor(1.0,1.0,1.0,0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    return true;
}

void PlotLibraryOpenGl::renderLine(const PlotData& data, int indexX, int indexY) {
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

void PlotLibraryOpenGl::renderSpline(const PlotData& data, int indexX, int indexY) {
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

void PlotLibraryOpenGl::renderErrorline(const PlotData& data, int indexX, int indexY, int indexError) {
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

void PlotLibraryOpenGl::renderErrorspline(const PlotData& data, int indexX, int indexY, int indexError) {
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

void PlotLibraryOpenGl::renderErrorbars(const PlotData& data, int indexX, int indexY, int indexError) {
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

void PlotLibraryOpenGl::renderCandlesticks(const PlotData& data, int indexX, int stickTop,
                                     int stickBottom, int candleTop, int candleBottom) {
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

void PlotLibraryOpenGl::renderSurface(const PlotData& data, const std::vector<int>& triangleVertexIndices, bool wire,
                                int indexX, int indexY, int indexZ, int indexCM, bool /*wireonly*/)  {
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

void PlotLibraryOpenGl::renderHeightmap(const voreen::PlotData& data, const std::vector< std::list< tgt::dvec2 > >& voronoiRegions,
                                  bool wire, int indexZ, int indexCM, bool /*wireonly*/) {
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

void PlotLibraryOpenGl::renderBars(const PlotData& data, std::vector<int> indicesY) {
    std::vector<PlotRowValue>::const_iterator rowIt = data.getRowsBegin();
    //stores y values and indices of a merged bar group, the indices are used to get the right color
    std::vector<std::pair<plot_t, tgt::Color> > mergedBars;
    //stores last y value, used for stacked bars
    plot_t lastY;
    tgt::Color c;
    glLineWidth(lineWidth_);
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
                c = colorMap_.getColorAtIndex(static_cast<int>(i));
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

void PlotLibraryOpenGl::renderNodeGraph(const PlotData& nodeData, const PlotData& connectionData, int indexX, int indexY, int indexDx, int indexDy) {
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

    std::vector<PlotCellValue> tester;
    tester.push_back(PlotCellValue(0));
    std::vector<PlotRowValue>::const_iterator firstIt, secondIt;

    // render connections
    glLineWidth(lineWidth_);
    glColor4fv(drawingColor_.elem);
    for (it = connectionData.getRowsBegin(); it != connectionData.getRowsEnd(); ++it) {
        tester[0].setValue(static_cast<int>(it->getValueAt(0)));
        firstIt = nodeData.lower_bound(tester);

        tester[0].setValue(static_cast<int>(it->getValueAt(1)));
        secondIt = nodeData.lower_bound(tester);

        if (firstIt != nodeData.getRowsEnd() && secondIt != nodeData.getRowsEnd()) {
            glBegin(GL_LINE_STRIP);
                logGlVertex2d(firstIt->getValueAt(indexX), firstIt->getValueAt(indexY));
                logGlVertex2d(secondIt->getValueAt(indexX), secondIt->getValueAt(indexY));
            glEnd();
        }
    }
}

void PlotLibraryOpenGl::renderColorMapLegend(const PlotData& data, int column, int number) {
    // ColorMaps with less than 2 colors may not exist
    if (colorMap_.getColorCount() < 2)
        return;

    Interval<plot_t>   interval = data.getInterval(column);
    const std::string& label    = data.getColumnLabel(column);

    // switch to viewport coordinates
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

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
    SmartLabelGroupBaseOpenGl::renderSingleLabel(&labelFont_, label, tgt::dvec3(xStart, yStart - (height/2), 0),
            fontColor_, fontSize_, SmartLabel::MIDDLERIGHT, 4);

    std::stringstream ss;
    ss << std::setprecision(4) << interval.getLeft();
    SmartLabelGroupBaseOpenGl::renderSingleLabel(&labelFont_, ss.str(), tgt::dvec3(xStart, yStart - height, 0),
            fontColor_, fontSize_, SmartLabel::BOTTOMCENTERED, 4);

    ss.str("");
    ss.clear();
    ss << std::setprecision(4) << interval.getRight();
    SmartLabelGroupBaseOpenGl::renderSingleLabel(&labelFont_, ss.str(), tgt::dvec3(xStart + width, yStart - height, 0),
            fontColor_, fontSize_, SmartLabel::BOTTOMCENTERED, 4);

    glEnable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();
    LGL_ERROR;
}

void PlotLibraryOpenGl::renderScatter(const PlotData& data, int indexX, int indexY, int indexZ, int indexCM, int indexSize) {
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

void PlotLibraryOpenGl::renderAxes() {
    // axes
    if (dimension_ == TWO)
        glDisable(GL_DEPTH_TEST);

    glLineWidth(axesWidth_);
    glColor4fv(drawingColor_.elem);

    plot_t xl = domain_[0].getLeft();    plot_t xr = domain_[0].getRight();
    plot_t yl = domain_[1].getLeft();    plot_t yr = domain_[1].getRight();
    plot_t zl = domain_[2].getLeft();    plot_t zr = domain_[2].getRight();

    if (! centerAxesFlag_) {
        //x and y axes
        glBegin(GL_LINE_STRIP);
            glVertex3d(xl, yr, zl);
            glVertex3d(xl, yl, zl);
            glVertex3d(xr, yl, zl);
        glEnd();
    }

    if (dimension_ == TWO) {
    //draw arrows with viewport coordinates
        int arrowSize = 5; // in pixel
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.pushMatrix();
        MatStack.loadIdentity();
        glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
        MatStack.pushMatrix();
        MatStack.loadIdentity();
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
        MatStack.popMatrix();
        MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
        MatStack.popMatrix();
        MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    }
    else if (dimension_ == THREE) {
        if (centerAxesFlag_) {
            glBegin(GL_LINES);
                glVertex3d(xl, 0, 0); glVertex3d(xr, 0, 0);
                glVertex3d(0, yl, 0); glVertex3d(0, yr, 0);
                glVertex3d(0, 0,  0); glVertex3d(0, 0, zr);
            glEnd();
        }
        else {
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

void PlotLibraryOpenGl::renderAxisScales(Axis axis, bool helperLines, const std::string& label, plot_t offset) {
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
            stream << round(pos + offset, step.y);
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
        std::vector<SelectionEdge>::const_iterator minEdge = getSelectionEdgesX().begin();
        std::vector<SelectionEdge>::const_iterator endEdge = getSelectionEdgesX().end();
        if (axis == X_AXIS) {
            if (getSelectionEdgesX().empty())
                return;
        }
        else if (axis == Y_AXIS) {
            if (getSelectionEdgesY().empty())
                return;
            minEdge = getSelectionEdgesY().begin();
            endEdge = getSelectionEdgesY().end();
        }
        else if (axis == Z_AXIS) {
            if (getSelectionEdgesZ().empty())
                return;
            minEdge = getSelectionEdgesZ().begin();
            endEdge = getSelectionEdgesZ().end();
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

void PlotLibraryOpenGl::renderAxisLabelScales(const PlotData& data, int indexLabel, bool helperLines) {
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

void PlotLibraryOpenGl::renderLabel(tgt::vec3 pos, const SmartLabel::Alignment align, const std::string& text,
                              bool viewCoordinates, int padding) {
    if (!viewCoordinates)
        pos = convertPlotCoordinatesToViewport3(pos);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    glDisable(GL_DEPTH_TEST);
    SmartLabelGroupBaseOpenGl::renderSingleLabel(&labelFont_, text, pos, fontColor_, fontSize_, align, static_cast<float>(padding));
    glEnable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();
    LGL_ERROR;
}

void PlotLibraryOpenGl::renderLabel(tgt::dvec2 pos, const SmartLabel::Alignment align, const std::string& text, int padding) {
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    glDisable(GL_DEPTH_TEST);
    SmartLabelGroupBaseOpenGl::renderSingleLabel(&labelFont_, text, tgt::vec3((float)pos.x, (float)pos.y, 0), fontColor_,
            fontSize_, align, static_cast<float>(padding));
    glEnable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();
    LGL_ERROR;
}

void PlotLibraryOpenGl::addPlotLabel(std::string text, tgt::vec3 position, tgt::Color color,
                               int size, SmartLabel::Alignment align) {
    plotLabelGroup_.addLabel(text, position, color, size, align);
}

void PlotLibraryOpenGl::addLineLabel(std::string text, tgt::vec3 position, tgt::Color color,
                               int size, SmartLabel::Alignment align) {
    lineLabelGroup_.addLabel(text, position, color, size, align);
}

//
// helper functions
//

void PlotLibraryOpenGl::resetLineLabels() {
    lineLabelGroup_.reset();
    lineLabelGroup_.setBounds(getBoundsRightOfPlot());
}

void PlotLibraryOpenGl::renderLineLabels() {
    renderSmartLabelGroup(&lineLabelGroup_);
}

void PlotLibraryOpenGl::resetPlotLabels() {
    plotLabelGroup_.reset();
    plotLabelGroup_.setBounds(getBoundsPlot());
}

void PlotLibraryOpenGl::renderPlotLabels() {
    renderSmartLabelGroup(&plotLabelGroup_);
}

void PlotLibraryOpenGl::renderSmartLabelGroup(SmartLabelGroupBase* smg) {
    smg->performLayout();

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0f, windowSize_.x, 0.f, windowSize_.y, -1.0f, 1.0f);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    glDisable(GL_DEPTH_TEST);
    smg->render();
    glEnable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();
    LGL_ERROR;
}


void PlotLibraryOpenGl::resetRenderStatus() {
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glLineWidth(1.f);
    glEnable(GL_DEPTH_TEST);

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.loadIdentity();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.loadIdentity();

    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LIGHT0);
    glDisable(GL_NORMALIZE);
}

void PlotLibraryOpenGl::logGlVertex2d(plot_t x, plot_t y) const {
    tgt::Vector2<plot_t> point = logScale2dtoLogCoordinates(x,y);
    glVertex2d(point.x, point.y);
}

void PlotLibraryOpenGl::logGlVertex3d(plot_t x, plot_t y, plot_t z) const {
    tgt::Vector3<plot_t> point = logScale3dtoLogCoordinates(x,y,z);
    glVertex3d(point.x, point.y, point.z);
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

void PlotLibraryOpenGl::renderGlyph(plot_t x, plot_t y, plot_t z, plot_t size) {
    if (glyphStyle_ == PlotEntitySettings::POINT) {
        glPointSize(static_cast<float>(size));
        glBegin(GL_POINTS);
            glVertex3d(x,y,z);
        glEnd();
        return;
    }
    MatStack.pushMatrix();
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
    MatStack.popMatrix();
    LGL_ERROR;
}

void PlotLibraryOpenGl::renderSingleBar(plot_t left, plot_t right, plot_t bottom, plot_t top, tgt::Color c, plot_t squeeze) {
    plot_t back = domain_[Z_AXIS].getLeft();
    plot_t front = domain_[Z_AXIS].getRight();
    //squeeze
    left = (left+squeeze*left+right-squeeze*right)/2.0;
    right = (left-squeeze*left+right+squeeze*right)/2.0;
    back = (back+squeeze*back+front-squeeze*front)/2.0;
    front = (back-squeeze*back+front+squeeze*front)/2.0;

    if(!usePlotPickingManager_)
        glDisable(GL_POLYGON_SMOOTH);

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
    if(!usePlotPickingManager_)
        glEnable(GL_POLYGON_SMOOTH);
}

tgt::dvec2 PlotLibraryOpenGl::convertPlotCoordinatesToViewport(const tgt::dvec3& plotCoordinates) const {
    tgt::dvec3 copy = logScale3dtoLogCoordinates(plotCoordinates);

    GLdouble x, y, z;
    GLdouble mv[16];   glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    GLdouble pr[16];   glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    gluProject(copy.x, copy.y, copy.z, mv, pr, viewport, &x, &y, &z);
    return tgt::dvec2(x, y);
}

tgt::dvec3 PlotLibraryOpenGl::convertPlotCoordinatesToViewport3(const tgt::dvec3& plotCoordinates) const {
    tgt::dvec3 copy = logScale3dtoLogCoordinates(plotCoordinates);

    GLdouble x, y, z;
    GLdouble mv[16];   glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    GLdouble pr[16];   glGetDoublev(GL_PROJECTION_MATRIX, pr);
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    gluProject(copy.x, copy.y, copy.z, mv, pr, viewport, &x, &y, &z);
    return tgt::dvec3(x, y, z);
}

tgt::dvec2 PlotLibraryOpenGl::convertViewportToPlotCoordinates(tgt::ivec2 viewCoord) const {
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


} // namespace
