/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_TRANSFUNCMAPPINGCANVAS_H
#define VRN_TRANSFUNCMAPPINGCANVAS_H

#include <QString>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>

#include "tgt/vector.h"

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/histogram.h"

namespace voreen {

// Forward Declarations
class TransFuncMappingKey;
class TransFuncGradient;
class TransFuncIntensityKeys;

/**
* Editor for TransFuncIntensityKeys
* \sa QWidget
*/
class TransFuncMappingCanvas : public QWidget {
Q_OBJECT

public:
    TransFuncMappingCanvas(QWidget *parent, TransFuncIntensityKeys* tf, TransFuncGradient* gradient = 0,
                          MessageReceiver* msgReceiver = 0, bool noColor = false,
                          bool rampMode = false, bool clipThresholds = false, QString xAxisText = tr("intensity"),
                          QString yAxisText = tr("opacity"), QString transferFuncPath = tr("../../data/transferfuncs"));

    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

    virtual void paintKeys(QPainter& paint);
    virtual void insertNewKey(tgt::vec2& hit);
    virtual void handleRightClick(QMouseEvent* event);
    virtual void handleContextMenu(QMouseEvent *event);

    virtual void dataSourceChanged(Volume* newDataset);
    virtual void setThreshold(float l, float u);

    void setRampMode(bool on);
    void setRampParams(float rampCenter, float rampWidth);
    void getRampParams(float &rampCenter, float &rampWidth);
    void readFromDisc(QString fileName);
    void saveToDisc(QString fileName);
    void clean();

    virtual QSize minimumSizeHint () const;
    virtual QSize sizeHint () const;
    QSizePolicy sizePolicy () const;

    void setTransFunc(TransFuncIntensityKeys* tf);

    void setXAxisText(const std::string& text);
    void setYAxisText(const std::string& text);

signals:
    void colorChanged(const QColor& c);
    void changed();
    void updateCoordinates(float x, float y);
    void exclusiveModeChangedWithDataSource(bool enable,
                                            float sourceValue);
    void clearCoordinates();

public slots:
    void splitMergeKeys();
    void unselectKey();
    void deleteKey();
    void changeCurrentColor(const QColor& c);
    void changeCurrentColor();
    void zeroKey();
    void gradAlphaKey();
    void readFromDisc();
    void saveToDisc();
    void setStandardFunc();
    void toggleClipThresholds(bool enabled);
    virtual void toggleGridSnap(bool enabled);
    virtual void toggleShowHistogram(bool enabled);

protected slots:
    void onChanged();

protected:
    TransFuncMappingKey* getOtherKey(TransFuncMappingKey* selectedKey, bool selectedLeftPart);
    void calcKeysFromRampParams();
    void calcRampParamsFromKeys();
    virtual int hitLine(const tgt::vec2& p);
    tgt::vec2 wtos(tgt::vec2 p);
    tgt::vec2 stow(tgt::vec2 p);
    tgt::vec2 snapToGrid(const tgt::vec2 p);
    virtual void resizeEvent(QResizeEvent *event);

    enum MarkerProps {
        MARKER_NORMAL   =  0,
        MARKER_LEFT     =  1,
        MARKER_RIGHT    =  2,
        MARKER_ZEROED   =  4,
        MARKER_ALPHA    =  8,
        MARKER_SELECTED = 16
    };

    void drawMarker(QPainter& paint, const tgt::col4& color, const tgt::vec2& p,
                    int props = 0);

private:
    TransFuncGradient* gradient_;
    TransFuncIntensityKeys* tf_;
    HistogramIntensity* histogram_;
    Volume* curDataset_;

    QString xAxisText_;
    QString yAxisText_;

    QString transferFuncPath_;

    float thresholdL_;
    float thresholdU_;
    float minThresholdGap_;

    TransFuncMappingKey* selectedKey_;
    int padding_, arrowLength_, arrowWidth_, pointSize_, minCellSize_;
    float splitFactor_;
    bool selectedLeftPart_, gridSnap_, showHistogram_, dragging_;
    QString caption_;
    tgt::vec2 xRange_, yRange_;
    std::string propertyKey_;
    float defaultValue_;
    int dragLine_;
    int dragLineStartY_;
    QPoint mousePos_;
    tgt::vec2 gridSpacing_;

    QMenu contextMenu_;
    QMenu fileMenu_;
    QAction* splitMergeAction_;
    QAction* zeroAction_;
    QAction* deleteAction_;
    QAction* loadAction_;
    QAction* saveAction_;
    QAction* resetAction_;

    MessageReceiver* msgReceiver_;
    bool isChanged_;
    bool rampMode_;
    bool clipThresholds_;
    float rampCenter_;
    float rampWidth_;
    bool noColor_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCMAPPINGCANVAS_H
