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

#ifndef TRANSFERFUNCALPHAPLUGIN_H
#define TRANSFERFUNCALPHAPLUGIN_H

#include <list>

#include "voreen/core/vis/property.h"
#include "voreen/qt/widgets/widgetplugin.h"

#include "tgt/event/eventlistener.h"

#include <QToolButton>

namespace voreen {

class TransFuncGradient;
class TransFuncMappingCanvas;
class TransFuncIntensityKeys;
class Volume;

/**
*   Provides a canvas and color chooser to edit TransFuncIntensity objects.
*/
class TransFuncAlphaPlugin : public WidgetPlugin,
    tgt::EventListener, TemplatePlugin<TransFunc*>{
    Q_OBJECT

public:
    TransFuncAlphaPlugin(QWidget* parent, MessageReceiver* msgReceiver, 
        TransFuncAlphaProp* prop = 0, bool showHistogramAtDatasourceChange = false, QString text = tr("intensity"));
    virtual ~TransFuncAlphaPlugin();

    ///Update the currently used datasource for histogram etc.
    void dataSourceChanged(Volume* newDataset);
    void setVisibleState(bool vis);
    void changeValue(TransFunc* tf);
    virtual void createWidgets();
    virtual void createConnections();
    
    TransFunc* getTransFunc();

public slots:
    void updateTransferFunction();
    void updateIntTransferFunction();
    void readFromDisc(std::string filename);
    void setStandardFunc();
    void clearCoordinatesLabel();
    void updateCoordinatesLabel(float x, float y);
    void setThresholds(int l, int u);
    void setScaleFactor(float scale);
    void setMaxValue(unsigned int);
    
signals:
    void transferFunctionReset();

private:
    
    Identifier msgIdent_;
    TransFuncAlphaProp* prop_;

    QString yAxisText_;

    TransFuncMappingCanvas* transCanvas_;
    TransFuncGradient* gradient_;
    TransFuncIntensityKeys* transferFunc_;

    TransFuncMappingCanvas* intTransCanvas_;
    TransFuncGradient* intGradient_;   
    TransFuncIntensityKeys* intTransferFunc_;

    QToolButton* histogramEnabledButton_;
    
    float scaleFactor_;
    int maxValue_;
    
    tgt::ivec2 lastMousePos_;
};

} // namespace voreen

#endif
