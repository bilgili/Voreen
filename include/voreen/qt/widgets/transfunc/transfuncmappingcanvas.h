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

#ifndef VRN_TRANSFUNCMAPPINGCANVAS_H
#define VRN_TRANSFUNCMAPPINGCANVAS_H

#include "tgt/vector.h"

#include "voreen/qt/voreenqtapi.h"

#include <QWidget>
#include <QMenu>

class QAction;
class QColor;
class QMouseEvent;

namespace voreen {

// Forward Declarations
class Histogram1D;
class HistogramPainter;
class TransFuncMappingKey;
class TransFunc1DKeys;

// ------------------------------------------------------------------------- //

/**
 * A widget that provides a canvas to edit the keys of an intensity transfer function.
 * The user can insert new keys by clicking at the desired location and reposition keys with holding
 * down left mouse button. Furthermore keys can be splitted, merged and deleted. The color of a key
 * can also be changed.
 */
class VRN_QT_API TransFuncMappingCanvas : public QWidget {
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param parent the parent widget
     * @param tf the transfer function that is displayed in this widget
     * @param noColor when true the color of a key can not be changed
     * @param xAxisText caption of the x axis
     * @param yAxisText caption of the y axis
     */
    TransFuncMappingCanvas(QWidget* parent, TransFunc1DKeys* tf, bool noColor = false,
                           QString xAxisText = tr("intensity"),
                           QString yAxisText = tr("opacity"));

    /**
     * Destructor
     */
    virtual ~TransFuncMappingCanvas();

    /**
     * Paints the current transfer function in a coordinate system
     * with a grid and a caption.
     *
     * @param event the paint event
     */
    virtual void paintEvent(QPaintEvent* event);

    /**
     * Coarseness mode is turned on, a new key is inserted when no key is at the mouse position
     * or the context menu is opened when right mousebutton was pressed.
     *
     * @param event the mouse event
     */
    virtual void mousePressEvent(QMouseEvent* event);

    /**
     * Switches coarseness mode off and hides the tooltip.
     *
     * @param event the mouse event
     */
    virtual void mouseReleaseEvent(QMouseEvent* event);

    /**
     * Moves the selected key to new mouse position or moves both keys at the
     * ends of the dragged line. Nothing is done when no key is selected nor a line is dragged.
     *
     * @param event the mouse event
     */
    virtual void mouseMoveEvent(QMouseEvent* event);

    /**
     * Opens a colordialog to change the color of the selected key. Nothing happens when no key
     * is selected.
     *
     * @param event the mouse event
     */
    virtual void mouseDoubleClickEvent(QMouseEvent* event);

    /**
     * Sets the cursor to vertical size cursor when the mouse is on a line of the transfer function
     * and shift was pressed.
     *
     * @param event the key event
     */
    virtual void keyPressEvent(QKeyEvent* event);

    /**
     * Unsets the cursor and deletes the selected key when del on keyboard was pressed.
     *
     * @param event the key event
     */
    virtual void keyReleaseEvent(QKeyEvent* event);

    /**
     * Sets the lower and upper threshold to the given values.
     *
     * @param l lower threshold
     * @param u upper threshold
     */
    void setThreshold(float l, float u);

    /**
     * Signal a change in the TF domain.
     */
    void domainChanged();

    /**
     * Returns the minimum size of the widget.
     *
     * @return the minimum size of the widget
     */
    virtual QSize minimumSizeHint() const;

    /**
     * Returns the preferred size of the widget.
     *
     * @return preferred size of the widget
     */
    virtual QSize sizeHint() const;

    /**
     * Returns the expanding policies of this widget in x and y direction .
     *
     * @return expanding policies of this widget
     */
    virtual QSizePolicy sizePolicy() const;

    /**
     * Sets the transfer function that can be edited with this widget.
     *
     * @param tf transfer function
     */
    void setTransFunc(TransFunc1DKeys* tf);

    /**
     * Sets the caption of the x axis.
     *
     * @param text caption of the x axis
     */
    void setXAxisText(const std::string& text);

    /**
     * Sets the caption of the y axis.
     *
     * @param text caption of the y axis
     */
    void setYAxisText(const std::string& text);

signals:
    /**
     * Signal that is emitted when the color of the selected key changed.
     *
     * @param c new color of the selected key
     */
    void colorChanged(const QColor& c);

    /**
     * Signal that is emitted when the transfer function changed.
     */
    void changed();

    /**
     * Signal that is emitted when a transfer function should be loaded from disk.
     */
    void loadTransferFunction();

    /**
     * Signal that is emitted when the current transfer function should be saved to disk.
     */
    void saveTransferFunction();

    /**
     * Signal that is emitted when the transfer function is reset to default.
     */
    void resetTransferFunction();

    /**
     * Signal that is emitted when the user drags a key or a line.
     * It turns coarseness mode on or off.
     *
     * @param on should coarseness mode switched on or off?
     */
    void toggleInteractionMode(bool on);

public slots:
    /**
     * Splits or merges the current selected key.
     */
    void splitMergeKeys();

    /**
     * Sets the left or right part of the current selected key to zero.
     */
    void zeroKey();

    /**
     * Deletes the current selected key.
     */
    void deleteKey();

    /**
     * Resets the transfer function to default.
     */
    void resetTransferFunc();

    /**
     * Opens a colordialog for choosing the color of the current selected key.
     */
    void changeCurrentColor();

    /**
     * Changes the color of the selected key to the given value.
     *
     * @param c new color of the selected key
     */
    void changeCurrentColor(const QColor& c);

protected:
    // enum for the status of a key
    enum MarkerProps {
        MARKER_NORMAL   =  0, ///< key is not selected and not split
        MARKER_LEFT     =  1, ///< left part of a key
        MARKER_RIGHT    =  2, ///< right part of a key
        MARKER_SELECTED =  4  ///< key is selected
    };

    /**
     * Creates a new key at the given position.
     *
     * @param hit position of the key in relative coordinates
     */
    void insertNewKey(tgt::vec2& hit);

    /**
     * Returns the nearest left or the nearest right key of the given key.
     * If no key exists at left or right 0 is returned.
     *
     * @param selectedKey key which nearest neighbour is searched
     * @param selectedLeftPart should the left neighbour be returned?
     * @return key that is the left or right neighbour of the given key. If no neighbour exists 0 is returned.
     */
    TransFuncMappingKey* getOtherKey(TransFuncMappingKey* selectedKey, bool selectedLeftPart);

    /**
     * Returns the number of the key that is left to the mouse position when
     * the position lies on the line between 2 keys. Otherwise -1 is returned.
     *
     * @param p mouse position in pixel coordinates
     * @return number of the nearest left key when on the line between two keys or -1 otherwise
     */
    int hitLine(const tgt::vec2& p);

    /**
     * Paints all keys of the transfer function.
     *
     * @param paint the painter
     */
    void paintKeys(QPainter& paint);

    /**
     * Draws the marker at the keys of the transfer function.
     *
     * @param paint the painter
     * @param color the color of the key
     * @param p the position of the key in pixel coordinates
     * @param props the status of the key
     */
    void drawMarker(QPainter& paint, const tgt::col4& color, const tgt::vec2& p,
                    int props = 0);

    /**
     * Diplays the context menu at the given mouseposition
     * for the case of a keyselection.
     *
     * @param event the mouse event
     */
    void showKeyContextMenu(QMouseEvent* event);

    /**
     * Diplays the context menu at the given mouseposition
     * for the case of no keyselection.
     *
     * @param event the mouse event
     */
    void showNoKeyContextMenu(QMouseEvent* event);

    /**
     * The underlying grid is refined or coarsened according to new size.
     *
     * @param event the resize event
     */
    virtual void resizeEvent(QResizeEvent* event);

    /**
     * Helper function for calculation of pixel coordinates from relative coordinates.
     *
     * @param p relative coordinates in the interval [0,1]
     * @return pixel coordinates
     */
    tgt::vec2 wtos(tgt::vec2 p);

    /**
     * Helper function for calculation of relative coordinates from pixel coordinates.
     *
     * @param p pixel coordinates in the interval [0,width]x[0,height]
     * @return relative coordinates
     */
    tgt::vec2 stow(tgt::vec2 p);

    /**
     * Converts a tgt::col4 into a QColor.
     *
     * @param color the tgt::col4 that will be converted
     * @return color converted to QColor
     */
    QColor Col2QColor(const tgt::col4& color);

    /**
     * Converts a QColor into a tgt::col4.
     *
     * @param color the QColor that will be converted
     * @return color converted to tgt::col4
     */
    tgt::col4 QColor2Col(const QColor& color);

    /**
     * Hides the tooltip that is displayed when a key is dragged.
     */
    void hideCoordinates();

    /**
     * Displays a tooltip at position pos with given values.
     *
     * @param pos position of the tooltip
     * @param values values that are displayed in the tooltip
     */
    void updateCoordinates(QPoint pos, tgt::vec2 values);

    TransFunc1DKeys* tf_;             ///< pointer to the transfer function that is displayed
    HistogramPainter* histogramPainter_; ///< painter that draws the histogram onto this widget

    float thresholdL_; ///< lower threshold in the interval [0, 1]
    float thresholdU_; ///< upper threshold in the interval [0, 1]

    // variables for interaction
    TransFuncMappingKey* selectedKey_; ///< key that was selected by the user
    bool selectedLeftPart_;            ///< when selected key is split, was the left part selected?
    bool dragging_;                    ///< is the user dragging a key?
    int dragLine_;                     ///< number that indicates the line that was dragged using the shift modifier
    int dragLineStartY_;               ///< y position where the drag of the line started
    float dragLineAlphaLeft_;          ///< floating alpha value of the left key of the dragged line
    float dragLineAlphaRight_;         ///< floating alpha value of the right key of the dragged line
    QPoint mousePos_;                  ///< current position of the mouse

    // variables for appearance of widget
    int padding_;           ///< additional border of the widget
    int arrowLength_;       ///< length of the arrows at the end of coordinate axes
    int arrowWidth_;        ///< width of the arrows at the end of coordinate axes
    float splitFactor_;     ///< offset between splitted keys
    int pointSize_;         ///< size of a key of the transfer function
    int minCellSize_;       ///< minimum size of a grid cell
    tgt::vec2 xRange_;      ///< range in x direction
    tgt::vec2 yRange_;      ///< range in y direction
    tgt::vec2 gridSpacing_; ///< width and height of the underlying grid
    bool noColor_;          ///< when true the color of a key can not be changed

    QString xAxisText_;     ///< caption of the x axis
    QString yAxisText_;     ///< caption of the y axis

    QMenu keyContextMenu_;   ///< context menu for right mouse click when a key is selected
    QMenu noKeyContextMenu_; ///< context menu for right mouse click when no key is selected

    QAction* splitMergeAction_; ///< action for split/merge context menu entry
    QAction* zeroAction_;       ///< action for zero to right context menu entry
    QAction* deleteAction_;     ///< action for delete key context menu entry
    QAction* loadAction_;       ///< action for load transfer function context menu entry
    QAction* saveAction_;       ///< action for save transfer function context menu entry
    QAction* resetAction_;      ///< action for reset transfer function context menu entry
    QAction* yAxisLogarithmicAction_;  ///< action for determining the scale on the histogram y-axis

public slots:
    void setHistogram(const Histogram1D* histogram);

};

} // namespace voreen

#endif // VRN_TRANSFUNCMAPPINGCANVAS_H
