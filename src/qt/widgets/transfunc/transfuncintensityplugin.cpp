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

#include "voreen/qt/widgets/transfunc/transfuncintensityplugin.h"


#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
#include "voreen/core/vis/transfunc/transfuncintensitykeys.h"

#include "tgt/gpucapabilities.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QGroupBox>

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"
#include "voreen/qt/qcolorpicker.h"
#include "voreen/qt/qcolorluminancepicker.h"

namespace voreen {

TransFuncIntensityPlugin::TransFuncIntensityPlugin(QWidget* parent, MessageReceiver* msgReceiver,
                                                   Qt::Orientation widgetOrientation,
                                                   TransFuncProp* prop, bool showHistogramAtDatasourceChange)
    : WidgetPlugin(parent, msgReceiver),
      TransFuncEditor(msgReceiver),
      prop_(prop),
      transferFunc_(0),
      rampMode_(false),
      clipThresholds_(false),
      lastMousePos_(),
      rampModeTracking_(false),
      showHistogramAtDatasourceChange_(showHistogramAtDatasourceChange),
      widgetOrientation_(widgetOrientation)
{
    setObjectName(tr("Transfer Function"));
    icon_ = QIcon(":/icons/transferfunc.png");

    if (prop_)
        id_ = prop->getIdent();
    else
        id_ = VolumeRenderer::setTransFunc_;
    
    target_ = Message::all_;
}

TransFuncIntensityPlugin::~TransFuncIntensityPlugin() {
    delete transferFunc_;
}

void TransFuncIntensityPlugin::createWidgets() {
    resize(300,300);

    QBoxLayout* mainLayout = new QVBoxLayout(this);

    // Splitter between topwidget and bottomwidget
    QSplitter* splitter = new QSplitter(widgetOrientation_, this);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    // Top - MappingCanvas and Gradient
    QWidget* topwidget = new QWidget(splitter);
    QVBoxLayout* gridLayout = new QVBoxLayout(topwidget);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(1);
    
    gradient_ = new TransFuncGradient(0);
	gradient_->getCanvas()->getGLFocus();
	transferFunc_ = new TransFuncIntensityKeys();
    transCanvas_ = new TransFuncMappingCanvas(0, transferFunc_, gradient_, msgReceiver_);
    transferFunc_->createTex(256);

    gridLayout->addWidget(transCanvas_);
    
    //workaround for qt-bug: size calculation for vboxlayout within a splitter is corrupt
    //see: http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=118893
    //bug has been fixed in 4.3.0 (joerg-stefan)
#if (QT_VERSION < 0x040300)
    QWidget* additionalSpace = new QWidget();
    additionalSpace->setMinimumHeight(13);
    gridLayout->addWidget(additionalSpace);
#endif
    gridLayout->addWidget(gradient_);

    // CheckBoxes for ramp mode and threshold clipping (not for gui generator properties)
    if (!prop_) {
        QHBoxLayout* hboxLayout = new QHBoxLayout();
        
        checkClipThresholds_ = new QCheckBox(tr("Zoom on threshold area"));
        checkClipThresholds_->setToolTip(tr("Zoom-in on the area between lower and upper thresholds"));
        checkClipThresholds_->setChecked(clipThresholds_);
        hboxLayout->addWidget(checkClipThresholds_);
        hboxLayout->addStretch();
        
        checkRampMode_ = new QCheckBox(tr("Ramp mode"));
        checkRampMode_->setToolTip(tr("Restrict transfer function to ramp function"));
        checkRampMode_->setChecked(rampMode_);
        hboxLayout->addWidget(checkRampMode_);
        hboxLayout->addSpacing(4);

        gridLayout->addLayout(hboxLayout);
    }

    // Bottom - Buttons and ColorPicker
    QWidget* bottomwidget = new QWidget(splitter);
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomwidget);

    QHBoxLayout *cLay = new QHBoxLayout();
    cLay->setMargin(0);
    
    // Buttons
    QVBoxLayout* gridHistoBox = new QVBoxLayout();
    cLay->addItem(gridHistoBox);

    gridEnabledButton_ = new QToolButton(bottomwidget);
    gridEnabledButton_->setCheckable(true);
    gridEnabledButton_->setChecked(false);
    gridEnabledButton_->setIcon(QIcon(":/icons/magnet.png"));
    gridEnabledButton_->setToolTip(tr("Snap points to grid"));
    gridHistoBox->addWidget(gridEnabledButton_);

    histogramEnabledButton_ = new QToolButton(bottomwidget);
    histogramEnabledButton_->setCheckable(true);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->setIcon(QIcon(":/icons/histogram.png"));
    histogramEnabledButton_->setToolTip(tr("Show data histogram"));
    gridHistoBox->addWidget(histogramEnabledButton_);

    clearButton_ = new QToolButton(bottomwidget);
    clearButton_->setIcon(QIcon(":/icons/eraser.png"));
    clearButton_->setToolTip(tr("Reset to default transfer function"));
    gridHistoBox->addWidget(clearButton_);

    gridHistoBox->addStretch();
    gridHistoBox->addWidget(loadButton_ = new QToolButton(bottomwidget));
    loadButton_->setIcon(QIcon(":/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));
    gridHistoBox->addWidget(saveButton_ = new QToolButton(bottomwidget));
    saveButton_->setIcon(QIcon(":/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    // ColorPicker
    QWidget* pickerFrame = new QWidget(this);
    pickerFrame->setContentsMargins(-8,-8,-8,-8);
    QHBoxLayout* pickerLayout = new QHBoxLayout();

    colorPicker_ = new QColorPicker(bottomwidget);
    colorPicker_->setFrameStyle(QFrame::Panel + QFrame::Sunken);
    pickerLayout->addWidget(colorPicker_);

    colorLumPicker_ = new QColorLuminancePicker(bottomwidget);
    colorLumPicker_->setFixedWidth(20);
    pickerLayout->addWidget(colorLumPicker_);
    
    pickerFrame->setLayout(pickerLayout);
    
    // Ramp settings
    QGroupBox* rampBox = new QGroupBox(tr("Ramp Parameters"));

    QGridLayout* rampLayout = new QGridLayout();
    rampLayout->setColumnStretch(0, 0);
    rampLayout->setColumnStretch(1, 1);
    rampLayout->setColumnStretch(2, 0);
    rampLayout->setRowStretch(0, 0);
    rampLayout->setRowStretch(1, 0);
    rampLayout->setRowStretch(2, 1);
    
    rampLayout->addWidget(new QLabel(tr("Center")), 0, 0);
    sliderRampCenter_ = new QSlider(Qt::Horizontal);
    sliderRampCenter_->setMaximum(0);
    spinRampCenter_ = new QSpinBox();
    spinRampCenter_->setMaximum(0);
    rampLayout->addWidget(sliderRampCenter_, 0, 1);
    rampLayout->addWidget(spinRampCenter_, 0, 2);
    
    rampLayout->addWidget(new QLabel(tr("Width")), 1, 0);
    sliderRampWidth_ = new QSlider(Qt::Horizontal);
    sliderRampWidth_->setMaximum(0);
    spinRampWidth_ = new QSpinBox();
    spinRampWidth_->setMaximum(0);
    rampLayout->addWidget(sliderRampWidth_, 1, 1);
    rampLayout->addWidget(spinRampWidth_, 1, 2);
    
    QWidget* spacer = new QWidget();
    rampLayout->addWidget(spacer, 2, 0, 1, 3);

    rampBox->setLayout(rampLayout);

    stackedWidgetPickerRamp_ = new QStackedWidget();
    stackedWidgetPickerRamp_->addWidget(pickerFrame);
    stackedWidgetPickerRamp_->addWidget(rampBox);

    cLay->addWidget(stackedWidgetPickerRamp_);
    
    tfSize_ = new QComboBox();
    QHBoxLayout* hboxL = new QHBoxLayout();
    
    if (GpuCaps.areSharedPalettedTexturesSupported())
        tfSize_->addItem("256", 256);
    else {
        int maxsize = GpuCaps.getMaxTextureSize();
        int minsize = 8;
        
        int k = minsize;
        while (k < maxsize) {
            k <<= 1;
            tfSize_->addItem(QString::number(k), k);
        }
    }
    
    int cursize = 256;
    if ( (transferFunc_) && (transferFunc_->getTexture()) )
        cursize = transferFunc_->getTexture()->getWidth();

    int item = tfSize_->findData(cursize);
    tfSize_->setCurrentIndex(item);
    
    hboxL->addSpacing(2);
    hboxL->addWidget(new QLabel(tr("Resolution:")));
    hboxL->addWidget(tfSize_);
    hboxL->addSpacing(2);
    bottomLayout->addSpacing(2);
    bottomLayout->addLayout(hboxL);

    bottomLayout->addLayout(cLay);

    coordinatesLabel_ = 0;

	splitter->setStretchFactor(0, QSizePolicy::Expanding);    // topwidget should be stretched
	splitter->setStretchFactor(1, QSizePolicy::Fixed);    // bottomwidget should not be stretched
	splitter->setStretchFactor(2, QSizePolicy::Expanding);
    
    mainLayout->addStretch();

    setLayout(mainLayout);

    transCanvas_->setStandardFunc();
}

void TransFuncIntensityPlugin::createConnections() {
    // Buttons
    connect(loadButton_, SIGNAL(clicked()), transCanvas_, SLOT(readFromDisc()));
    connect(saveButton_, SIGNAL(clicked()), transCanvas_, SLOT(saveToDisc()));
    connect(clearButton_, SIGNAL(clicked()), this, SLOT(setStandardFunc()));

    connect(gridEnabledButton_, SIGNAL(toggled(bool)),
            transCanvas_, SLOT(toggleGridSnap(bool)));
    connect(histogramEnabledButton_, SIGNAL(toggled(bool)),
            transCanvas_, SLOT(toggleShowHistogram(bool)));
    connect(transCanvas_, SIGNAL(updateCoordinates(float, float)),
            this, SLOT(updateCoordinatesLabel(float, float)));
    connect(transCanvas_, SIGNAL(clearCoordinates()),
            this, SLOT(clearCoordinatesLabel()));

    connect(transCanvas_, SIGNAL(changed()), this,
            SLOT(updateTransferFunction()));

    connect(tfSize_, SIGNAL(activated(int)), this, SLOT(resizeTF(int)));

    if (!prop_) {
        connect(checkRampMode_, SIGNAL(toggled(bool)), 
            this, SLOT(toggleRampMode(bool)));
        connect(sliderRampCenter_, SIGNAL(valueChanged(int)),
            this, SLOT(updateRampCenter(int)));
        connect(spinRampCenter_, SIGNAL(valueChanged(int)),
            this, SLOT(updateRampCenter(int)));
        connect(spinRampWidth_, SIGNAL(valueChanged(int)),
            this, SLOT(updateRampWidth(int)));
        connect(sliderRampWidth_, SIGNAL(valueChanged(int)),
            this, SLOT(updateRampWidth(int)));
        
        connect(sliderRampCenter_,  SIGNAL(sliderPressed()), this,
            SLOT(startTracking()));
        connect(sliderRampWidth_,  SIGNAL(sliderPressed()), this,
            SLOT(startTracking()));
        connect(sliderRampCenter_,  SIGNAL(sliderReleased()), this,
            SLOT(stopTracking()));
        connect(sliderRampWidth_,  SIGNAL(sliderReleased()), this,
            SLOT(stopTracking()));

        connect(checkClipThresholds_, SIGNAL(toggled(bool)),
            transCanvas_, SLOT(toggleClipThresholds(bool)));
        connect(checkClipThresholds_, SIGNAL(toggled(bool)),
            gradient_, SLOT(toggleClipThresholds(bool)));
    }

    // color picker
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorPicker_, SLOT(setCol(const QColor)));
    connect(transCanvas_, SIGNAL(colorChanged(const QColor&)),
            colorLumPicker_, SLOT(setCol(const QColor)));
    connect(colorPicker_, SIGNAL(newCol(int,int)),
            colorLumPicker_, SLOT(setCol(int,int)));
    connect(colorLumPicker_, SIGNAL(newHsv(int,int,int)),
            this, SLOT(markerColorChanged(int,int,int)));

}

void TransFuncIntensityPlugin::dataSourceChanged(Volume* newDataset) {
   
    if (transCanvas_ != 0) {
        transCanvas_->dataSourceChanged(newDataset);
        setStandardFunc();
    }

    histogramEnabledButton_->setChecked(showHistogramAtDatasourceChange_);
    transCanvas_->toggleShowHistogram(showHistogramAtDatasourceChange_);

    int bits = newDataset->getBitsStored();
    switch (bits) {
        case 8:
            setMaxValue(255);
            setScaleFactor(1.0f/255.0f);
            transferFunc_->createTex(256);
            updateTransferFunction();
            break;
        case 12:
            if (GpuCaps.getMaxTextureSize() >= 1024) {
                transferFunc_->createTex(1024);
                updateTransferFunction();
            }
            setMaxValue(4095);
            setScaleFactor(1.0f/4095.0f);
            break;
        case 16:
            if (GpuCaps.getMaxTextureSize() >= 1024) {
                transferFunc_->createTex(1024);
                updateTransferFunction();
            }
            setMaxValue(65535);
            setScaleFactor(1.0f/65535.0f);
            break;
        case 32:
            transferFunc_->createTex(256);
            updateTransferFunction();
            setMaxValue(255);
            setScaleFactor(1.0f/255.0f);
            break;
    }
}

void TransFuncIntensityPlugin::setVisibleState(bool vis) {
    setVisible(vis);
}

void TransFuncIntensityPlugin::changeValue(TransFunc* /*tf*/){

}

void TransFuncIntensityPlugin::setTransFunc(TransFuncIntensityKeys* tf) {
    transferFunc_ = tf;
    transCanvas_->setTransFunc(tf);
    
    int cursize = 256;
    if ( (transferFunc_) && (transferFunc_->getTexture()) )
        cursize = transferFunc_->getTexture()->getWidth();

    int item = tfSize_->findData(cursize);
    tfSize_->setCurrentIndex(item);
    
    //updateTransferFunction();
}

TransFunc* TransFuncIntensityPlugin::getTransFunc() {
    return transferFunc_;
}

void TransFuncIntensityPlugin::processorChanged(int bits){
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

void TransFuncIntensityPlugin::mouseMoveEvent(tgt::MouseEvent *e) {
    e->ignore();
    
    if (!rampMode_)
        return;

    if ( isVisible() && (e->modifiers() & tgt::MouseEvent::ALT) && (rampModeTracking_) ) {
        tgt::ivec2 shift = e->coord() - lastMousePos_;
        lastMousePos_ = e->coord();
            
        float center, width;
        transCanvas_->getRampParams(center, width);
        center = std::max(std::min(center - shift.y / 400.f, 1.f), 0.f);
        width = std::max(std::min(width + shift.x / 400.f, 1.f), 0.f);
        transCanvas_->setRampParams(center, width);
        syncRampSliders(int(center*maxValue_+0.5f), int(width*maxValue_+0.5f));
            
        e->accept();
    }
}

void TransFuncIntensityPlugin::mousePressEvent(tgt::MouseEvent *e) {
    e->ignore();

    if  ( isVisible() &&
        ( e->button() & tgt::MouseEvent::MOUSE_BUTTON_RIGHT) &&
        ( e->modifiers() & tgt::MouseEvent::ALT)
        ) {
            e->accept();
            if (rampMode_) {
                lastMousePos_ = e->coord();
                rampModeTracking_ = true;
                startTracking();
            }
    }
}

void TransFuncIntensityPlugin::mouseReleaseEvent(tgt::MouseEvent *e) {
    e->ignore();

    if ( isVisible() && (e->button() & tgt::MouseEvent::MOUSE_BUTTON_RIGHT) && rampModeTracking_ ) {
        e->accept();
        if (rampModeTracking_) {
            rampModeTracking_ = false;
            stopTracking();
        }
    }
}

void TransFuncIntensityPlugin::mouseDoubleClickEvent(tgt::MouseEvent *e) {
    e->ignore();

    if ( isVisible() && (e->button() & tgt::MouseEvent::MOUSE_BUTTON_RIGHT) && 
        (e->modifiers() & tgt::MouseEvent::ALT) ) {
        
        e->accept();
        if (rampMode_)
            setStandardFunc();   
    }
}

void TransFuncIntensityPlugin::readFromDisc(std::string filename) {
    transCanvas_->readFromDisc(filename.c_str());
}

void TransFuncIntensityPlugin::setStandardFunc() {
    transCanvas_->setStandardFunc();
    emit(transferFunctionReset());
}

void TransFuncIntensityPlugin::updateTransferFunction() {
    if (!transferFunc_)
        transferFunc_ = new TransFuncIntensityKeys();
    
    if (rampMode_) {
        float rampCenter, rampWidth;
        transCanvas_->getRampParams(rampCenter, rampWidth);
        int center = int(rampCenter*maxValue_+0.5f);
        int width = int(rampWidth*maxValue_+0.5f);
        syncRampSliders(center, width);
    }
    
    transferFunc_->updateTexture();
    if (prop_) {
        postMessage(new TransFuncPtrMsg(id_, transferFunc_), prop_->getMsgDestination());
        if (isVisible())
            repaintCanvases();
    } else
        sendTFMessage();
    
    gradient_->setTransFunc(transferFunc_);
    gradient_->update();
}

void TransFuncIntensityPlugin::resizeTF(int s) {
    QVariant var = tfSize_->itemData(s);
    int newsize = var.toInt();
    transferFunc_->createTex(newsize);
    updateTransferFunction();
}

void TransFuncIntensityPlugin::clearCoordinatesLabel() {
    if (coordinatesLabel_)
        coordinatesLabel_->setText(QString(""));
}

void TransFuncIntensityPlugin::updateCoordinatesLabel(float x, float y) {
    static char s[1024];
    sprintf(s, "%1.2f / %1.2f", x, y);
    if (coordinatesLabel_)
        coordinatesLabel_->setText(QString(s));
}

void TransFuncIntensityPlugin::toggleRampMode(bool on) {
    rampMode_ = on;
    transCanvas_->setRampMode(rampMode_);
    if (rampMode_)
        stackedWidgetPickerRamp_->setCurrentIndex(1);
    else
        stackedWidgetPickerRamp_->setCurrentIndex(0);

    updateTransferFunction();

}

void TransFuncIntensityPlugin::updateRampCenter(int center) {
    int width = spinRampWidth_->value();
    float centerf = float(center) / maxValue_;
    float widthf = float(width) / maxValue_;
    transCanvas_->setRampParams(centerf, widthf);
    syncRampSliders(center, width);

}
  
void TransFuncIntensityPlugin::updateRampWidth(int width) {
    int center = spinRampCenter_->value();
    float centerf = float(center) / maxValue_;
    float widthf = float(width) / maxValue_;
    transCanvas_->setRampParams(centerf, widthf);
    syncRampSliders(center, width);

}

void TransFuncIntensityPlugin::syncRampSliders(int rampCenter, int rampWidth) {
    sliderRampCenter_->blockSignals(true);
    sliderRampWidth_->blockSignals(true);
    spinRampCenter_->blockSignals(true);
    spinRampWidth_->blockSignals(true);
    sliderRampCenter_->setValue(rampCenter);
    spinRampCenter_->setValue(rampCenter);
    sliderRampWidth_->setValue(rampWidth);
    spinRampWidth_->setValue(rampWidth);
    sliderRampCenter_->blockSignals(false);
    sliderRampWidth_->blockSignals(false);
    spinRampCenter_->blockSignals(false);
    spinRampWidth_->blockSignals(false);
            
}

void TransFuncIntensityPlugin::setScaleFactor(float scale) {
    scaleFactor_ = scale;
}

void TransFuncIntensityPlugin::setMaxValue(unsigned int value) {
    maxValue_ = static_cast<int>(value);
    
    // synchronize ramp widget
    float center, width;
    transCanvas_->getRampParams(center, width);
    syncRampSliders(center*maxValue_, width*maxValue_);
    sliderRampCenter_->setMaximum(maxValue_);
    sliderRampWidth_->setMaximum(maxValue_);
    spinRampCenter_->setMaximum(maxValue_);
    spinRampWidth_->setMaximum(maxValue_);
}   

void TransFuncIntensityPlugin::setThresholds(int lower, int upper) {
    tgtAssert(lower <= upper, "Lower threshold above upper threshold!");
    if (lower == upper) {
        if (upper > 0)
            lower = upper-1;
        else
            upper = lower+1;
    }
    transCanvas_->setThreshold(lower*scaleFactor_,upper*scaleFactor_);
    gradient_->setThresholds(lower*scaleFactor_, upper*scaleFactor_);
}

void TransFuncIntensityPlugin::switchInteractionMode(bool on) {
    if (on)
		startTracking();
	else
		stopTracking();
}

void TransFuncIntensityPlugin::markerColorChanged(int h, int s, int v) {
    transCanvas_->changeCurrentColor(QColor::fromHsv(h, s, v));
}

void TransFuncIntensityPlugin::updateMappingCanvas() {
    transCanvas_->repaint(); 
}

} // namespace voreen
