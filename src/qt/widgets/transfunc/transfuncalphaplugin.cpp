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

#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QSplitter>
#include <QFrame>
#include <QGroupBox>

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"
#include "voreen/qt/widgets/transfunc/transfuncgradient.h"
#include "voreen/qt/widgets/transfunc/transfuncalphaplugin.h"


namespace voreen {

TransFuncAlphaPlugin::TransFuncAlphaPlugin(QWidget* parent, MessageReceiver* msgReceiver, 
        TransFuncAlphaProp* prop, bool showHistogramAtDatasourceChange, QString text)
    : WidgetPlugin(parent, msgReceiver), 
      TemplatePlugin<TransFunc*>(), 
      prop_(prop),
      transferFunc_(0),
      intTransferFunc_(0),
      yAxisText_(text)
{
    setObjectName(tr("Transfer Function Alpha"));
    icon_ = QIcon(":/icons/transferfunc.png");

	maxValue_ = 255;

    //else case should never be used
    if (prop_) {
        msgIdent_ = prop->getIdent();
    } else {
        msgIdent_ = VolumeRenderer::setTransFunc_;
    }
}

TransFuncAlphaPlugin::~TransFuncAlphaPlugin() {
    delete transferFunc_;
}

void TransFuncAlphaPlugin::createWidgets() {
    resize(300,300);

    transferFunc_ = new TransFuncIntensityKeys(256);
    transferFunc_->createAlphaFunc();

    QBoxLayout *mainLayout = new QVBoxLayout(this);

    // Splitter between topwidget and bottomwidget
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    // Top - MappingCanvas and Gradient
    QWidget* topwidget = new QWidget(splitter);
    QVBoxLayout* gridLayout = new QVBoxLayout(topwidget);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(1);
    
    gradient_ = new TransFuncGradient(0);
    transCanvas_ = new TransFuncMappingCanvas(0, transferFunc_, gradient_, msgReceiver_, true, false, false, yAxisText_);
    
    gridLayout->addWidget(transCanvas_);

    histogramEnabledButton_ = new QToolButton(this);
    histogramEnabledButton_->setCheckable(true);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->setIcon(QIcon(":/icons/histogram.png"));
    histogramEnabledButton_->setToolTip(tr("Show data histogram"));
    gridLayout->addWidget(histogramEnabledButton_);
    
    //FIXME: workaround for qt-bug: size calculation for vboxlayout within a splitter is corrupt
    //see: http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=118893
    //bug has been fixed in 4.3.0 (joerg-stefan)
#if (QT_VERSION < 0x040300)
    QWidget* additionalSpace = new QWidget();
    additionalSpace->setMinimumHeight(13);
    gridLayout->addWidget(additionalSpace);
#endif
    gridLayout->addWidget(gradient_);

    splitter->setStretchFactor(0, 1);    // topwidget should be stretched
    splitter->setStretchFactor(1, 0);    // bottomwidget should not be stretched
    splitter->setStretchFactor(2, 1);
    
    mainLayout->addStretch();

    setLayout(mainLayout);

    //this->setStandardFunc();
}

void TransFuncAlphaPlugin::createConnections() {
    connect(transCanvas_, SIGNAL(updateCoordinates(float, float)),
            this, SLOT(updateCoordinatesLabel(float, float)));
    connect(transCanvas_, SIGNAL(clearCoordinates()),
            this, SLOT(clearCoordinatesLabel()));

    connect(histogramEnabledButton_, SIGNAL(toggled(bool)),
            transCanvas_, SLOT(toggleShowHistogram(bool)));

    connect(transCanvas_, SIGNAL(changed()), this,
            SLOT(updateTransferFunction()));
}

TransFunc* TransFuncAlphaPlugin::getTransFunc() {
    return transferFunc_;
}

void TransFuncAlphaPlugin::setVisibleState(bool vis) {
    setVisible(vis);
}

void TransFuncAlphaPlugin::changeValue(TransFunc* /*tf*/){
}

void TransFuncAlphaPlugin::readFromDisc(std::string filename) {
    transCanvas_->readFromDisc(filename.c_str());
}

void TransFuncAlphaPlugin::setStandardFunc() {
    
    transCanvas_->setStandardFunc();
    emit(transferFunctionReset());

}

void TransFuncAlphaPlugin::updateTransferFunction() {
    if(!transferFunc_)
        transferFunc_ = new TransFuncIntensityKeys(maxValue_ + 1);
//     delete transferFunc_;
    
//     gradient_->exportFunction(transferFunc_);
    transferFunc_->updateTexture();
    if (prop_) {
        postMessage(new TransFuncPtrMsg(msgIdent_, transferFunc_), prop_->getMsgDestination());
    } else {
        postMessage(new TransFuncPtrMsg(msgIdent_, transferFunc_));
    }
    if (isVisible())
        repaintCanvases();
}

void TransFuncAlphaPlugin::updateIntTransferFunction() {
    if(!transferFunc_)
        transferFunc_ = new TransFuncIntensityKeys(maxValue_ + 1);
    intTransferFunc_->updateTexture();
    postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc2_, intTransferFunc_));
    repaintCanvases();
}

void TransFuncAlphaPlugin::updateCoordinatesLabel(float x, float y) {
    static char s[1024];
    sprintf(s, "%1.2f / %1.2f", x, y);
}

void TransFuncAlphaPlugin::clearCoordinatesLabel() {
}

void TransFuncAlphaPlugin::setThresholds(int l, int u) {    
    tgtAssert(l <= u, "Lower threshold above upper threshold!");
    if (l == u) {
        if (u > 0)
            l = u-1;
        else
            u = l+1;
    }
    transCanvas_->setThreshold(l*scaleFactor_,u*scaleFactor_);
    gradient_->setThresholds(l*scaleFactor_, u*scaleFactor_);
}
    
void TransFuncAlphaPlugin::dataSourceChanged(Volume* newDataset) {
    
    if (transCanvas_ != 0) {
        transCanvas_->dataSourceChanged(newDataset);
        setStandardFunc();
    }

    int bits = newDataset->getBitsStored();
    switch (bits) {
        case 8:
            setMaxValue(255);
            setScaleFactor(1.0f/255.0f);
            break;
        case 12:
            setMaxValue(4095);
            setScaleFactor(1.0f/4095.0f);
            break;
        case 16:
            setMaxValue(65535);
            setScaleFactor(1.0f/65535.0f);
            break;
        case 32:
            setMaxValue(255);
            setScaleFactor(1.0f/255.0f);
            break;
    }
}

void TransFuncAlphaPlugin::setScaleFactor(float scale) {
    scaleFactor_ = scale;
}

void TransFuncAlphaPlugin::setMaxValue(unsigned int value) {
    maxValue_ = static_cast<int>(value);           
}

} // namespace voreen

