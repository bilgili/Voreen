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

#include "voreen/qt/widgets/transfunc/transfuncintensitypetplugin.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>

#include "tgt/gpucapabilities.h"

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

#include "voreen/qt/widgets/transfunc/transfuncgradient.h"
#include "voreen/qt/widgets/doublesliderwidget.h"

namespace voreen {

namespace {

inline QColor Col2QColor(const tgt::col4& color) {
    return QColor(color.r, color.g, color.b); // ignore alpha
}

inline tgt::col4 QColor2Col(const QColor& color) {
    return tgt::col4(color.red(), color.green(), color.blue(), 255); // ignore alpha
}

} // namespace

TransFuncIntensityPetPlugin::TransFuncIntensityPetPlugin(QWidget* parent, MessageReceiver* msgReceiver,
                                                         Qt::Orientation widgetOrientation)
    : WidgetPlugin(parent, msgReceiver),
      TransFuncEditor(msgReceiver),
      widgetOrientation_(widgetOrientation),
      tfGradient_(0),
      tf_(0),
      histogram_(0)
{
    setObjectName(tr("Intensity PET Transfer function"));
    icon_ = QIcon(":/icons/transferfunc.png");
    target_ = Message::all_;
    range = 1.f;
    oldThreshX = 0.f;
    lowestIntensity = 0;
    greatestIntensity = 255;
    thresholds = tgt::vec2(0.f, 1.f);
    offset = tgt::vec2(0.f, 1.f);
    mousePress = false;
}


TransFuncIntensityPetPlugin::~TransFuncIntensityPetPlugin() {
    delete tf_;
    delete tfGradient_;
    delete histogram_;
}

void TransFuncIntensityPetPlugin::createWidgets() {
    resize(500,500);

    QBoxLayout *mainLayout = new QVBoxLayout(this);

	// Splitter between topwidget and bottomwidget
    QSplitter* splitter = new QSplitter(widgetOrientation_, this);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    // Top
    QWidget* topwidget = new QWidget(splitter);
    QVBoxLayout* topLayout = new QVBoxLayout(topwidget);
    topLayout->setMargin(0);
    topLayout->setSpacing(1);

    topLayout->addWidget(loadButton_ = new QToolButton());
    loadButton_->setIcon(QIcon(":/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));

	histo_ = new HistogramPainter(this);
    gradient_ = new TransFuncGradient(this);	
    doubleSlider_ = new DoubleSlider(this);

	// add TransferGradient margin to doubleSlider_ and histo_using layouts
	QBoxLayout *histoLayout = new QVBoxLayout();
	QBoxLayout *sliderLayout = new QVBoxLayout();

	//getContentsMargins available in QT version 4.3.0 for the first time
	#if (QT_VERSION >= 0x040300)
    int left, top, right, bottom;
	mainLayout->getContentsMargins(&left, &top, &right, &bottom);
	histoLayout->setContentsMargins(left, 0, right, 0);
	sliderLayout->setContentsMargins(left, 0, right, 0);
	#endif
	
	histoLayout->addWidget(histo_);
	sliderLayout->addWidget(doubleSlider_);

	topLayout->addLayout(histoLayout);
	topLayout->addWidget(gradient_);
	topLayout->addLayout(sliderLayout);
	
    //bottom
    QWidget* bottomwidget = new QWidget(splitter);
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomwidget);
    bottomLayout->setMargin(0);

	QBoxLayout* qhBox = new QHBoxLayout();
	qhBox->addWidget(new QLabel("min"));
	minBox_ = new QSpinBox(0);
	minBox_->setMaximum(greatestIntensity);
	qhBox->addWidget(minBox_);
	qhBox->addWidget(new QLabel(" max"));
	maxBox_ = new QSpinBox(0);
	maxBox_->setMinimum(lowestIntensity);
	maxBox_->setMaximum(greatestIntensity);
	maxBox_->setValue(greatestIntensity);
	qhBox->addWidget(maxBox_);

	bottomLayout->addLayout(qhBox);

	commitPetTransGradient_ = new QPushButton("&Expand", this);    
    bottomLayout->addWidget(commitPetTransGradient_);

	resetPetTransGradient_ = new QPushButton("&Collapse", this);    
    bottomLayout->addWidget(resetPetTransGradient_);

    splitter->setStretchFactor(0, QSizePolicy::Expanding);    // topwidget should be stretched
    splitter->setStretchFactor(1, QSizePolicy::Fixed);    // bottomwidget should not be stretched
    splitter->setStretchFactor(2, QSizePolicy::Expanding);

    mainLayout->addStretch();
    setLayout(mainLayout);

    createConnections();
}

void TransFuncIntensityPetPlugin::createConnections() {
	connect(loadButton_, SIGNAL(clicked()), this, SLOT(readFromDisc()));
	connect(doubleSlider_ , SIGNAL(valuesChanged(float, float)), this, SLOT(sliderChanged(float, float)));
	connect(minBox_ , SIGNAL(valueChanged(int)), this, SLOT(minBoxChanged(int)));
	connect(maxBox_ , SIGNAL(valueChanged(int)), this, SLOT(maxBoxChanged(int)));
	connect(commitPetTransGradient_ , SIGNAL(clicked()), this, SLOT(commitGradient()));
	connect(resetPetTransGradient_ , SIGNAL(clicked()), this, SLOT(resetGradient()));
}

void TransFuncIntensityPetPlugin::minBoxChanged(int value) {
    if (fabs(static_cast<float>(value) / static_cast<float>(greatestIntensity) - thresholds.x) > 0.00001f) {
	    thresholds.x = float(value) / float(greatestIntensity);
	    histo_->setLowerThreshold(thresholds.x);
        doubleSlider_->blockSignals(true);
	    doubleSlider_->setMinValue((thresholds.x - offset.x) / (offset.y - offset.x));
        doubleSlider_->blockSignals(false);
        maxBox_->blockSignals(true);
	    maxBox_->setValue(tgt::iround((offset.x + doubleSlider_->getMaxValue()
                                       * (offset.y - offset.x))* greatestIntensity));   // needed to avoid slider collision
        maxBox_->blockSignals(false);
	    updateToThresholds();
    }
}

void TransFuncIntensityPetPlugin::maxBoxChanged(int value) {
    if (fabs(static_cast<float>(value) / static_cast<float>(greatestIntensity) - thresholds.y) > 0.00001f) {
	    thresholds.y = float(value) / float(greatestIntensity);
	    histo_->setUpperThreshold(thresholds.y);
        doubleSlider_->blockSignals(true);
	    doubleSlider_->setMaxValue((thresholds.y - offset.x) / (offset.y - offset.x));
        doubleSlider_->blockSignals(false);
        minBox_->blockSignals(true);
	    minBox_->setValue(tgt::iround((offset.x + doubleSlider_->getMinValue()
                                       * (offset.y - offset.x)) * greatestIntensity));	// needed to avoid slider collision
        minBox_->blockSignals(false);
	    updateToThresholds();
    }
}

void TransFuncIntensityPetPlugin::sliderChanged(float x, float y) {
	minBox_->setValue(tgt::iround((offset.x + x * (offset.y - offset.x)) * greatestIntensity));	
	maxBox_->setValue(tgt::iround((offset.x + y * (offset.y - offset.x))* greatestIntensity));	
}

void TransFuncIntensityPetPlugin::commitGradient() {
	if (tf_) {
		offset.x = tf_->getKey(0)->getIntensity();
		offset.y = tf_->getKey(tf_->getNumKeys()-1)->getIntensity();
	}
	doubleSlider_->setValues(0.f, 1.f);
	updateToThresholds();
}

void TransFuncIntensityPetPlugin::resetGradient() {
	if (tf_)
		doubleSlider_->setValues(tf_->getKey(0)->getIntensity(), tf_->getKey(tf_->getNumKeys()-1)->getIntensity());
	offset = tgt::vec2(0.f, 1.f);
	updateToThresholds();
}

void TransFuncIntensityPetPlugin::updateToThresholds(){
	if (tf_) {
		float recRange = 1/range;
		range = thresholds.y - thresholds.x;		
		float gradientRange = doubleSlider_->getMaxValue() - doubleSlider_->getMinValue();
		float plus = doubleSlider_->getMinValue();

		float intensity;
		for (int i=0; i < tf_->getNumKeys(); ++i){
			// renormalize intensity
			intensity = recRange * (tf_->getKey(i)->getIntensity() - oldThreshX);
			// calculate intensity values for gradient and transferfunc
			tfGradient_->getKey(i)->setIntensity(gradientRange * intensity + plus);
			tf_->getKey(i)->setIntensity(range * intensity + thresholds.x);
		}

		oldThreshX = thresholds.x;

		tf_->updateTexture();
		tfGradient_->updateTexture();

		transFuncChanged();
	}
    gradient_->update();	
}

void TransFuncIntensityPetPlugin::readFromDisc() {
	// prepare gradient
	resetGradient();
	minBox_->setValue(lowestIntensity);
	maxBox_->setValue(greatestIntensity);
	
	// load new transfer function
	if (mousePress){
		QString fileName = QFileDialog::getOpenFileName(this,
					"Choose transfer function to open",
					"../../data/transferfuncs",
					"TransferFuncs (*.tfi)",0,0);
		if (!fileName.isEmpty()) {
			fileOpen(fileName.toStdString());
		}
	}
	mousePress = !mousePress;		//FIXME: Signal "clicked" is emitted twice!
}

void TransFuncIntensityPetPlugin::fileOpen(std::string filename) {
	if (!tf_)
		tf_ = new TransFuncIntensity();
	if (!tfGradient_)
		tfGradient_ = new TransFuncIntensity(256);
		
	tf_->load(filename);
	tf_->updateTexture();
	tfGradient_->load(filename);
	tfGradient_->updateTexture();
	
	gradient_->setTransFunc(tfGradient_);	
	gradient_->update();
	transFuncChanged();
}

void TransFuncIntensityPetPlugin::dataSourceChanged(Volume* newDataset) {
	int bits = newDataset->getBitsStored();
    lowestIntensity = 0;
	switch (bits) {
    case 8:
		greatestIntensity = 255;
        break;
    case 12:
        greatestIntensity = 4095;            
        break;
    case 16:
        greatestIntensity = 65535;
        break;
    case 32:
        greatestIntensity = 255;
        break;
	}

	minBox_->setMaximum(greatestIntensity);
    minBox_->setMinimum(lowestIntensity);
    maxBox_->setMaximum(greatestIntensity);
    maxBox_->setMinimum(lowestIntensity);

    doubleSlider_->setMaxValue(greatestIntensity);
    doubleSlider_->setMinValue(lowestIntensity);

    minBox_->setValue(lowestIntensity);
	maxBox_->setValue(greatestIntensity);

    resetGradient();
	
	if (histogram_)
       delete histogram_;
    histogram_ = 0;
	histogram_ = new HistogramIntensity(newDataset, greatestIntensity);
	histo_->setHistogram(histogram_);
	histo_->repaint();
}

void TransFuncIntensityPetPlugin::setThresholds(int l, int u) {
    minBox_->setValue(l);
    maxBox_->setValue(u);
}

void TransFuncIntensityPetPlugin::getThresholds(int &l, int &u) {
    l = minBox_->value();
    u = maxBox_->value();
}

void TransFuncIntensityPetPlugin::transFuncChanged() {
    postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, getTransFunc()), target_);
    postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void TransFuncIntensityPetPlugin::setTransFunc(TransFuncIntensity* tf) {
    tf_ = tf;
    gradient_->setTransFunc(tf_);
    gradient_->update();
    transFuncChanged();
}

} //namespace voreen
