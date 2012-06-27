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

#ifndef VRN_TRANSFUNCINTENSITYGRADIENTPLUGIN_H
#define VRN_TRANSFUNCINTENSITYGRADIENTPLUGIN_H

#include "tgt/qt/qtcanvas.h"

#include "voreen/qt/widgets/widgetplugin.h"
#include "voreen/core/vis/transfunc/transfunceditor.h"
/*
#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QSlider>
#include <QCheckBox>
*/
class QToolButton;
class QSlider;
class QCheckBox;

namespace voreen {

class TransFuncPainter;
class TransFuncIntensityGradientPrimitiveContainer;

///TransFuncIntensityGradient editor WidgetPlugin.
class TransFuncIntensityGradientPlugin : public WidgetPlugin, public TransFuncEditor {
    Q_OBJECT

public:
    TransFuncIntensityGradientPlugin(QWidget* parent, MessageReceiver* msgReceiver, Qt::Orientation widgetOrientation = Qt::Horizontal);
//     virtual ~TransFuncIntensityGradientPlugin();
  
    virtual void createWidgets();
    virtual void createConnections();
    tgt::QtCanvas* getCanvas() { return transCanvas_; }
    
    void dataSourceChanged(Volume* newDataset);
    TransFunc* getTransFunc();
    void setThresholds(int l, int u);
    void transFuncChanged();
    tgt::col4 colorChooser(tgt::col4 c, bool &changed);
    
    void setTransFunc(TransFuncIntensityGradientPrimitiveContainer* tf);
    void selected();
    void deselected();

public slots:
    ///open file dialog and load a transferfunction from xml file
    void read();
    ///open file dialog and save transferfunction to xml file
    void save();
    ///Remove all primitives from transferfunction
    void clear();
    ///Add a quad primitive
    void addQuad();
    ///Add a banana primitive
    void addBanana();
    ///Delete the selected primitive
    void deletePrimitive();
    ///Select a color for the selected primitive
    void colorize();
    ///Show a Intensity-Gradient in the background.
    void showHistogram();
    ///Show a 10x10 grid in the background.
    void showGrid();
    ///Adjust the histogram brightness. Default is 100 (percent)
    void adjustHistogramBrightness(int p);
    
    ///Set histogram to logarithmic mode
    void setHistogramLog(int s);
    
    void adjustFuzziness(int p);
    
    void adjustTransparency(int p);
    
    void sliderPressed();
    void sliderReleased();

private:
    Identifier msgIdent_;

    QToolButton* loadButton_;
    QToolButton* saveButton_;
    QToolButton* clearButton_;
    QToolButton* gridEnabledButton_;
    QToolButton* histogramEnabledButton_;
    QToolButton* quadButton_;
    QToolButton* bananaButton_;
    QToolButton* deleteButton_;
    QToolButton* colorButton_;
    QSlider* histogramBrightness_;
    QCheckBox* histogramLog_;
    QSlider* fuzziness_;
    QSlider* transparency_;

    Qt::Orientation widgetOrientation_;

    tgt::QtCanvas* transCanvas_;
    TransFuncPainter* painter_;
    float scaleFactor_;
};

} // namespace voreen

#endif //VRN_TRANSFUNCINTENSITYGRADIENT_H
