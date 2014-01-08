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

#ifndef VRN_TRANSFUNCPROPERTYWIDGETPAINTER_H
#define VRN_TRANSFUNCPROPERTYWIDGETPAINTER_H

#include "voreen/qt/voreenqtapi.h"

#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "tgt/event/mouseevent.h"
#include "tgt/vector.h"

#include <QObject>
#include <QPoint>
#include <QString>

namespace voreen {

class TransFunc;
class Histogram1D;

/***********************************************************
 * Base Slider Class                                       *
 * Provides sliders which can be moved inside a sub canvas *
 * of a given canvas.                                      *
 ***********************************************************/
class TransFuncPropertyWidgetPainterBaseSlider : public QObject {
public:
    /** Constructor */
    TransFuncPropertyWidgetPainterBaseSlider();
    /** Paints the slider. Has to be implemented by sub classes. */
    virtual void paint() = 0;
    /** Checks, if the slider is licked by the mouse at pos. Has to be implemented by sub classes. */
    virtual bool isHit(tgt::ivec2 pos) = 0;
    //--------------------
    //  getter and setter
    //--------------------
    /** Sets the current canvas size. Calls "updateSliderPosition()". */
    void setCanvasSize(tgt::ivec2 size);
    /** Sets the current sub canvas offset in pixel. Calls "updateSliderPosition()". */
    void setSubCanvasOffset(int offset);
    /** Sets the current sub canvas width in pixel. Calls "updateSliderPosition()". */
    void setSubCanvasWidth(int width);
    /** Sets the current value range of the sub canvas. Calls "updateSliderPosition()". */
    void setValueRange(tgt::vec2 range);
protected:
    /**
     * Calculates and sets the sider position according to valueRange_ and canvasSizeInPixel_.
     * Has to be implemented by sub classes.
     */
    virtual void updateSliderPosition() = 0;
    /**
     * Calculates and sets the stored value according to the slider position.
     * Has to be implemented by sub classes.
     */
    virtual void updateStoredValue() = 0;
    //--------------------
    //  member
    //--------------------
    tgt::ivec2 canvasSizeInPixel_;  ///< current canvas size
    int subCanvasOffsetInPixel_;    ///< left sub canvas offset
    int subCanvasWidthInPixel_;     ///< sub canvas width

    tgt::vec2 valueRange_;          ///< value range of the sub canvas
};

/***********************************************************
 * Gamma Slider                                            *
 ***********************************************************/
class TransFuncPropertyWidgetPainterGammaSlider : public TransFuncPropertyWidgetPainterBaseSlider {
    Q_OBJECT
public:
    /** Constructor */
    TransFuncPropertyWidgetPainterGammaSlider();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::paint */
    void paint();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::isHit */
    bool isHit(tgt::ivec2 pos);
    //--------------------
    //  getter and setter
    //--------------------
    /** Sets the slider position. Calls updateStoredValue(). */
    void setPosition(int x);
    /** Returns the x position of the gamma slider in pixel coordinates. */
    int getPosition();
    /** Sets the gamma value. Calls updateSliderPosition(). */
    void setGammaValue(float gamma);
    /** Returns the stored gamma value. */
    float getGammaValue();
protected:
    /** @see TransFuncPropertyWidgetPainterBaseSlider::updateSliderPosition */
    void updateSliderPosition();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::updateStoredValue */
    void updateStoredValue();
private:
    //--------------------
    //  member
    //--------------------
    int position_;  ///< position of the slider
    float gamma_;   ///< gamma value
signals:
    /** Signal emited by updateStoredValue. */
    void gammaChanged(float gamma);
};
/***********************************************************
 * Domain Slider                                           *
 ***********************************************************/
class TransFuncPropertyWidgetPainterDomainSlider : public TransFuncPropertyWidgetPainterBaseSlider {
    Q_OBJECT
public:
    /** Constructor */
    TransFuncPropertyWidgetPainterDomainSlider();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::paint */
    void paint();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::isHit */
    bool isHit(tgt::ivec2 pos);
    /** Checks, if left slider is hit. */
    bool isLeftHit(tgt::ivec2 pos);
    /** Checks, if right slider is hit. */
    bool isRightHit(tgt::ivec2 pos);
    //--------------------
    //  getter and setter
    //--------------------
    /** Sets the left slider position. Calls updateStoredValue(). */
    void setLeftPosition(int x);
    /** Sets the right slider position. Calls updateStoredValue(). */
    void setRightPosition(int x);
    /** Returns both slider postitions. */
    tgt::ivec2 getPosition();
    /** Sets the domainvalue. Calls updateSliderPosition(). */
    void setDomainValue(tgt::vec2 domain);
    /** returns the stored domain value. */
    tgt::vec2 getDomainValue();
protected:
    /** @see TransFuncPropertyWidgetPainterBaseSlider::updateSliderPosition */
    void updateSliderPosition();
    /** @see TransFuncPropertyWidgetPainterBaseSlider::updateStoredValue */
    void updateStoredValue();
private:
    //--------------------
    //  member
    //--------------------
    int leftPosition_;      ///< left slider position
    int rightPosition_;     ///< right slider position
    tgt::vec2 domain_;      ///< stored domain
signals:
    /** Signal emited by updateStoredValue() */
    void domainChanged(tgt::vec2 domain);
};

/**
 * The painter used in the TransFuncPropertyWidget to render the display canvas.
 */
class VRN_QT_API TransFuncPropertyWidgetPainter : public QObject, public tgt::Painter, public tgt::EventListener {
    Q_OBJECT
public:
    /**
     * @param canvas canvas that belongs to this painter
     */
    TransFuncPropertyWidgetPainter(tgt::GLCanvas* canvas);
    ~TransFuncPropertyWidgetPainter();

    /**
     * Paints the texture of the transfer function. A checkerboard is displayed
     * in the background.
     */
    void paint();

    /**
     * Initializes the painter, e.g. the projection and modelview matrix are set.
     */
    void initialize();

    /**
     * This method is called whenever the size of the widget changes.
     * It sets the viewport to the new size and updates the projection matrix.
     *
     * @param size new size of the widget
     */
    void sizeChanged(const tgt::ivec2& size);

    //--------------------
    //  getter and setter
    //--------------------
    /**
     * Sets the transfer function, which will be displayed.
     * Also controls the zoom level. If min = 1.f and max = 0.f the zoom will be reseted.
     */
    void setTransFunc(TransFunc* tf, float minDomainValue = 1.f, float maxDomainValue = 0.f);
    /** Sets the histogram, which will be displayed. */
    void setHistogram(const Histogram1D* histogram);
private:
    /** Help function used in paint. */
    void drawCheckBoard();
    /** Help function used in paint. */
    void drawTransferFunction();
    /** Help function used in paint. */
    void drawHistogram();
    /** Help function used in paint. */
    void drawThreshold(); //not used

    //--------------------
    //  mouse events
    //--------------------
    /** @see tgt::EventListener::mousePressEvent */
    void mousePressEvent(tgt::MouseEvent* e);
    /** @see tgt::EventListener::mouseMoveEvent */
    void mouseMoveEvent(tgt::MouseEvent* e);
    /** @see tgt::EventListener::mouseReleaseEvent */
    void mouseReleaseEvent(tgt::MouseEvent* e);
    /** @see tgt::EventListener::mouseDoubleClickEvent */
    void mouseDoubleClickEvent(tgt::MouseEvent* e);
    /** emits showInfoToolTip. Uses pressedSlider_ to create right tooltip. */
    void createInfoToolTip(QPoint mousePos);
protected:
    /** Enum used to communicate between mouse events. */
    enum MoveSlider {
        NO_SLIDER,
        GAMMA_SLIDER,
        DOMAIN_LEFT_SLIDER,
        DOMAIN_RIGHT_SLIDER,
        DOMAIN_BOTH_SLIDER
    } pressedSlider_;               ///< stores, which slider was clicked during mouse press.
    int mousePressedPosition_;      ///< position of the mouse press event. Used to move both sliders.
    int domainLeftPressedPosition_;        ///< position of the left domain slider during press
    int domainRightPressedPosition_;       ///< position of the right domain slider during press

protected slots:
    /** Slot connected to the gamma slider signal. Emits changed(). */
    void gammaSlot(float gamma);
    /** Slot connected to the domain slider signal. Emits changed(). */
    void domainSlot(tgt::vec2 domain);

signals:
    void changedGamma(); ///< signal which is emited, if the gamma value has been changed
    void changedDomain(); ///< signal which is emited, if the domain sliders have changed
    void interaction(bool inter); //toggles interaction mode

    void showInfoToolTip(QPoint pos, QString tip); ///< signal to show the info tooltip on slider changes
    void hideInfoToolTip(); ///< signal to hide the info tooltip

    //--------------------
    //  zoom functions
    //--------------------
public:
    /** Zooms in by chnaging the minimal and maximal domain values. */
    void zoomIn();
    /** Zooms in by chnaging the minimal and maximal domain values. */
    void zoomOut();
    /** Fits min/maxDomainValue to tf_ domain. */
    void resetZoom();

    /** Returns the minimal domain value. It is the most left possible position of the domain slider. */
    float getMinDomainValue() const;
    /** Returns the minimal domain value. It is the most left possible position of the domain slider. */
    float getMaxDomainValue() const;

private:
    //----------------
    //  Member
    //----------------
        //basic member
    TransFunc* tf_;                 ///< the displayed transfer function
    const Histogram1D* histogram_;  ///< the displayed histogram

        //slider
    TransFuncPropertyWidgetPainterGammaSlider* gammaSlider_; ///< slider to modify the gamma value
    TransFuncPropertyWidgetPainterDomainSlider* domainSlider_; ///< slider to modify the domain values

        //helper
    bool isInitialized_;            ///< painter must be initialized by calling initialize()

    bool renderHistogram_;          ///< if true, the histogram will be displayed
    bool renderGammaSlider_;        ///< if true, the gamma slider will be displayed
    bool renderDomainSlider_;       ///< if true, the domain slider will be displayed

    bool logarithmic_;              ///< if true, the histogram scale will be logarithmic

    float minDomainValue_;          ///< most left value of the texture canvas
    float maxDomainValue_;          ///< most right value of the texture canvas

};

} // namespace voreen

#endif // VRN_TRANSFUNCPROPERTYWIDGETPAINTER_H
