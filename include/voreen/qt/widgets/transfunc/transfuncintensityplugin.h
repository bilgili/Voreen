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

#ifndef VRN_TRANSFUNCINTENSITYPLUGIN_H
#define VRN_TRANSFUNCINTENSITYPLUGIN_H

#include <list>

#include "tgt/event/eventlistener.h"

#include "transfuncgradient.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/transfunc/transfunceditor.h"

#include "voreen/qt/widgets/widgetplugin.h"

#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QStackedWidget>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>

namespace voreen {

// Forward declarations
class TransFuncProp;
class TransFuncMappingCanvas;
class TransFuncGradient;
class TransFuncIntensityKeys;
class TransferGradient;
class QColorPicker;
class QColorLuminancePicker;

/**
*   Provides a canvas and color chooser to edit TransFuncIntensity objects.
*/
class TransFuncIntensityPlugin : public WidgetPlugin,
    tgt::EventListener, public TransFuncEditor {
    Q_OBJECT

public:
    /**
    * Standard constructor.
    * \param parent The parent frame to which this is added
    * \param msgReceiver
    * \param widgetOrientation
    * \param prop
    * \param showHistogramAtDatasourceChange self-explanatory
    */
    TransFuncIntensityPlugin(QWidget* parent, MessageReceiver* msgReceiver, Qt::Orientation widgetOrientation = Qt::Horizontal,
        TransFuncProp* prop = 0, bool showHistogramAtDatasourceChange = false);

    /**
    * Standard destructor
    */
    virtual ~TransFuncIntensityPlugin();

    /**
    * Creates the other widgets and adds them to this one.
    */
    virtual void createWidgets();
    virtual void createConnections();

    ///Update the currently used datasource for histogram etc.
    void dataSourceChanged(Volume* newDataset);
    void setVisibleState(bool vis);
    
    void changeValue(TransFunc* tf);
    void setTransFunc(TransFuncIntensityKeys* tf);
    TransFunc* getTransFunc();
    void processorChanged(int bits);

    void mouseMoveEvent(tgt::MouseEvent *e);
    void mousePressEvent(tgt::MouseEvent *e);
    void mouseReleaseEvent(tgt::MouseEvent *e);
    void mouseDoubleClickEvent(tgt::MouseEvent *e);
    
public slots:
    void readFromDisc(std::string filename);
    void setStandardFunc();
    void updateTransferFunction();
    void resizeTF(int s);

    void clearCoordinatesLabel();
    void updateCoordinatesLabel(float x, float y);

    void toggleRampMode(bool on);
    void updateRampCenter(int value);
    void updateRampWidth(int value);
    void syncRampSliders(int rampCenter, int rampWidth);
    void setScaleFactor(float scale);
    void setMaxValue(unsigned int);
    void setThresholds(int lower, int upper);
    void switchInteractionMode(bool on);
    void markerColorChanged(int h, int s, int v);
	void updateMappingCanvas();

signals:
    void transferFunctionReset();

private:
    TransFuncProp* prop_;

    TransFuncMappingCanvas* transCanvas_;
    TransFuncGradient* gradient_;
    TransFuncIntensityKeys* transferFunc_;

    QComboBox* tfSize_;
    QToolButton* loadButton_;
    QToolButton* saveButton_;
    QToolButton* clearButton_;
    QToolButton* gridEnabledButton_;
    QToolButton* histogramEnabledButton_;
    QLabel*      coordinatesLabel_;
    QColorPicker* colorPicker_;
    QColorLuminancePicker *colorLumPicker_;
    QCheckBox* checkRampMode_;
    QCheckBox* checkClipThresholds_;
    QStackedWidget* stackedWidgetPickerRamp_;
    QSlider* sliderRampCenter_;
    QSlider* sliderRampWidth_;
    QSpinBox* spinRampCenter_;
    QSpinBox* spinRampWidth_;

    Qt::Orientation widgetOrientation_;

    float scaleFactor_;
    int maxValue_;
    bool showHistogramAtDatasourceChange_;

    bool rampMode_;
    bool clipThresholds_;
    int rampWidth_;
    int rampCenter_;
    
    tgt::ivec2 lastMousePos_;
    bool rampModeTracking_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCINTENSITYPLUGIN_H
