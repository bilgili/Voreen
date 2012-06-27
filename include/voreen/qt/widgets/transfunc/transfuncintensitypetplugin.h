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

#ifndef VRN_TRANSFUNCINTENSITYPETPLUGIN_H
#define VRN_TRANSFUNCINTENSITYPETPLUGIN_H

#include "tgt/vector.h"
#include "tgt/math.h"

#include "voreen/core/volume/histogram.h"
#include "voreen/core/vis/transfunc/transfunceditor.h"

#include "voreen/qt/widgets/widgetplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncgradient.h"
#include "voreen/qt/widgets/doublesliderwidget.h"
#include "voreen/qt/widgets/histogrampainterwidget.h"

#include <QString>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QtGui>


namespace voreen {

class TransFuncGradient;
class TransFuncIntensityKeys;

/*
 * TransFuncIntensityPETPlugin is an editor widget for transfer functions with focus on PET data.
 */
class TransFuncIntensityPetPlugin : public WidgetPlugin, public TransFuncEditor {
    Q_OBJECT

public:
    TransFuncIntensityPetPlugin(QWidget* parent, MessageReceiver* msgReceiver, Qt::Orientation widgetOrientation = Qt::Vertical);
	virtual ~TransFuncIntensityPetPlugin();

    virtual void createWidgets();
    virtual void createConnections();

    void dataSourceChanged(Volume* newDataset);
    TransFunc* getTransFunc() { return tf_; }
    void setThresholds(int l, int u);
    void getThresholds(int &l, int &u);
    void transFuncChanged();
    void setTransFunc(TransFuncIntensityKeys* tf);
	void updateToThresholds();

public slots:
	void fileOpen(std::string filename);
    ///open file dialog and load a transferfunction from xml file
    void readFromDisc();
	void minBoxChanged(int);
	void maxBoxChanged(int);
	void sliderChanged(float x, float y);
	void commitGradient();
	void resetGradient();

private:
    Identifier msgIdent_;

    QToolButton* loadButton_;
    QPushButton* commitPetTransGradient_;
    QPushButton* resetPetTransGradient_;
    QSpinBox* minBox_;
    QSpinBox* maxBox_;

    Qt::Orientation widgetOrientation_;

    DoubleSlider* doubleSlider_;
    TransFuncGradient* gradient_;
    TransFuncIntensityKeys* tfGradient_;    //transfunc for gradient
    TransFuncIntensityKeys* tf_;            //transfunc for volume

    HistogramIntensity* histogram_;
    HistogramPainter* histo_;

    tgt::vec2 offset;                       //transfunc position of gradient range
    tgt::vec2 thresholds;                   //'global' transfunc position of sliders
    float range;
    float oldThreshX;
    int lowestIntensity;
    int greatestIntensity;
    bool mousePress;
};

} // namespace voreen

#endif //VRN_TRANSFUNCINTENSITYPETPLUGIN_H
