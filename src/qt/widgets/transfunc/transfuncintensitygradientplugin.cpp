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

#include "voreen/qt/widgets/transfunc/transfuncintensitygradientplugin.h"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QColorDialog>
#include <QLabel>
#include <QToolButton>
#include <QSlider>
#include <QCheckBox>

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/transfunc/transfuncpainter.h"
#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"


namespace voreen {

TransFuncIntensityGradientPlugin::TransFuncIntensityGradientPlugin(QWidget* parent, MessageReceiver* msgReceiver, Qt::Orientation widgetOrientation)
    : WidgetPlugin(parent, msgReceiver),
    TransFuncEditor(msgReceiver),
    widgetOrientation_(widgetOrientation)
{
    setObjectName(tr("Intensity/Gradient Transfer function"));
    icon_ = QIcon(":/icons/transferfunc.png");

    transCanvas_ = new tgt::QtCanvas("test", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, this, true, 0, false);

    target_ = Message::all_;
}


void TransFuncIntensityGradientPlugin::createWidgets() {
    resize(500,500);

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

    tgt::EventHandler* eh = new tgt::EventHandler();
    transCanvas_->setMinimumHeight(100);
    painter_ = new TransFuncPainter(transCanvas_, this);
    transCanvas_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    transCanvas_->setPainter(painter_);
    painter_->initialize();
    if (!painter_->initOk()) {
        delete painter_;
        delete eh;
        delete transCanvas_;
        painter_ = 0;
        gridLayout->addWidget(new QLabel("No FBO support!", this));
        return;
    }

    eh->addListenerToBack(new TransFuncEditorListener(painter_));
    transCanvas_->setEventHandler(eh);

    gridLayout->addWidget(transCanvas_);

    QHBoxLayout* gridHistoBox = new QHBoxLayout();

    gridHistoBox->addWidget(loadButton_ = new QToolButton());
    loadButton_->setIcon(QIcon(":/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));
    gridHistoBox->addWidget(saveButton_ = new QToolButton());
    saveButton_->setIcon(QIcon(":/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    gridEnabledButton_ = new QToolButton();
    gridEnabledButton_->setCheckable(true);
    gridEnabledButton_->setChecked(false);
    gridEnabledButton_->setIcon(QIcon(":/icons/grid.png"));
    gridEnabledButton_->setToolTip(tr("Show grid"));
    gridHistoBox->addWidget(gridEnabledButton_);

    histogramEnabledButton_ = new QToolButton();
    histogramEnabledButton_->setCheckable(true);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->setIcon(QIcon(":/icons/histogram.png"));
    histogramEnabledButton_->setToolTip(tr("Show data histogram"));
    gridHistoBox->addWidget(histogramEnabledButton_);
//     histogramEnabledButton_->setEnabled(false);

    clearButton_ = new QToolButton();
    clearButton_->setIcon(QIcon(":/icons/clear.png"));
    clearButton_->setToolTip(tr("Reset to default transfer function"));
    gridHistoBox->addSpacing(7);
    gridHistoBox->addWidget(clearButton_);

    quadButton_ = new QToolButton();
    quadButton_->setIcon(QIcon(":/icons/quad.png"));
    quadButton_->setToolTip(tr("Add a quad"));
    gridHistoBox->addWidget(quadButton_);

    bananaButton_ = new QToolButton();
    bananaButton_->setIcon(QIcon(":/icons/banana.png"));
    bananaButton_->setToolTip(tr("Add a banana"));
    gridHistoBox->addWidget(bananaButton_);

    deleteButton_ = new QToolButton();
    deleteButton_->setIcon(QIcon(":/icons/eraser.png"));
    deleteButton_->setToolTip(tr("Delete selected primitive"));
    gridHistoBox->addWidget(deleteButton_);

    colorButton_ = new QToolButton();
    colorButton_->setIcon(QIcon(":/icons/colorize.png"));
    colorButton_->setToolTip(tr("Change the color of the selected primitive"));
    gridHistoBox->addWidget(colorButton_);

    gridLayout->addLayout(gridHistoBox);

    //     gridHistoBox->addSpacing(7);
    //bottom
    QWidget* bottomwidget = new QWidget(splitter);
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomwidget);
    bottomLayout->setMargin(0);

    bottomLayout->addWidget(new QLabel(tr("Histogram Brightness:"), bottomwidget));
    histogramBrightness_ = new QSlider(Qt::Horizontal, bottomwidget);
    histogramBrightness_->setEnabled(false);
    histogramBrightness_->setMinimum(10);
    histogramBrightness_->setMaximum(1000);
    bottomLayout->addWidget(histogramBrightness_);
    
    histogramLog_ = new QCheckBox("Logarithmic Histogram", bottomwidget);
    histogramLog_->setEnabled(false);
    histogramLog_->setCheckState(Qt::Checked);
    bottomLayout->addWidget(histogramLog_);
    
    bottomLayout->addWidget(new QLabel(tr("Transparency:")));
    transparency_ = new QSlider(Qt::Horizontal, bottomwidget);
    transparency_->setEnabled(false);
    transparency_->setMinimum(0);
    transparency_->setMaximum(255);
    bottomLayout->addWidget(transparency_);
    
    bottomLayout->addWidget(new QLabel(tr("Fuzziness:")));
    fuzziness_ = new QSlider(Qt::Horizontal, bottomwidget);
    fuzziness_->setEnabled(false);
    fuzziness_->setMinimum(0);
    fuzziness_->setMaximum(100);
    bottomLayout->addWidget(fuzziness_);

    splitter->setStretchFactor(0, QSizePolicy::Expanding);    // topwidget should be stretched
    splitter->setStretchFactor(1, QSizePolicy::Fixed);    // bottomwidget should not be stretched
    splitter->setStretchFactor(2, QSizePolicy::Expanding);

    mainLayout->addStretch();

    setLayout(mainLayout);

//     createConnections();
}

void TransFuncIntensityGradientPlugin::createConnections() {
    if (!painter_)
        return;
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(read()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(save()));
    connect(clearButton_, SIGNAL(clicked()), this, SLOT(clear()));
    connect(quadButton_, SIGNAL(clicked()), this, SLOT(addQuad()));
    connect(bananaButton_, SIGNAL(clicked()), this, SLOT(addBanana()));
    connect(deleteButton_, SIGNAL(clicked()), this, SLOT(deletePrimitive()));
    connect(colorButton_, SIGNAL(clicked()), this, SLOT(colorize()));
    connect(histogramEnabledButton_, SIGNAL(clicked()), this, SLOT(showHistogram()));
    connect(gridEnabledButton_, SIGNAL(clicked()), this, SLOT(showGrid()));
    connect(histogramBrightness_, SIGNAL(sliderMoved(int)), this, SLOT(adjustHistogramBrightness(int)));
    connect(histogramLog_, SIGNAL(stateChanged(int)), this, SLOT(setHistogramLog(int)));
    connect(fuzziness_, SIGNAL(sliderMoved(int)), this, SLOT(adjustFuzziness(int)));
    connect(transparency_, SIGNAL(sliderMoved(int)), this, SLOT(adjustTransparency(int)));
    connect(fuzziness_, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(transparency_, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
    connect(fuzziness_, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
    connect(transparency_, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
}

void TransFuncIntensityGradientPlugin::read() {
    QString s = QFileDialog::getOpenFileName(
                    this,
                    "Choose a file",
                    "../../data/transferfuncs",
                    "Transferfunctions (*.tfig)");
    if (s != "") {
        clear();
        painter_->getTransFunc()->load(s.toStdString ());
    }

}

void TransFuncIntensityGradientPlugin::save() {
    QFileDialog* filedialog = new QFileDialog(this);
    filedialog->setDefaultSuffix(tr("tfig"));
    filedialog->setWindowTitle(tr("Choose a filename to save transfer function"));
    filedialog->setDirectory(tr("../../data/transferfuncs"));
    QStringList filters;
    filters << tr("Transfer function (*.tfig)")
            << tr("Transfer function image (*.png)");
    filedialog->setFilters(filters);
    filedialog->setAcceptMode(QFileDialog::AcceptSave);
    QStringList fileList;
    if (filedialog->exec()) {
        fileList = filedialog->selectedFiles();
        if (filedialog->selectedFilter() == filters[0])
            painter_->getTransFunc()->save(fileList.at(0).toStdString());
		else {
			transCanvas_->makeCurrent();
            painter_->getTransFunc()->savePNG(fileList.at(0).toStdString());
		}
    }
    delete filedialog;

//     QString s = QFileDialog::getSaveFileName(
//                     this,
//                     "Choose a filename to save under",
//                     "",
//                     "Transferfunctions (*.xml)");
//     if (s != "") {
//         painter_->getTransFunc()->save(s.toStdString ());
//     }
}

void TransFuncIntensityGradientPlugin::clear() {
    if (!painter_)
        return;
    painter_->clear();
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::addQuad() {
    TransFuncPrimitive* p = new TransFuncQuad(tgt::vec2(0.6,0.6), 0.3, tgt::col4(128,128,128,128));
    painter_->getTransFunc()->addPrimitive(p);
    painter_->select(p);
	painter_->changed();
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::addBanana() {
    TransFuncPrimitive* p = new TransFuncBanana(tgt::vec2(0.0,0.0), tgt::vec2(0.3,0.4), tgt::vec2(0.34,0.2), tgt::vec2(0.5,0.0),  tgt::col4(128,128,128,128));
    painter_->getTransFunc()->addPrimitive(p);
    painter_->select(p);
	painter_->changed();
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::deletePrimitive() {
    painter_->deletePrimitive();
    transCanvas_->update();
    setInteractionCoarseness(true);
    setInteractionCoarseness(false);
}

void TransFuncIntensityGradientPlugin::colorize() {
    TransFuncPrimitive* p = painter_->getSelectedPrimitive();
    if (p) {
        tgt::col4 c = p->getColor();
        bool ok;
        QColor qc;
        qc.setRgb(c.r,c.g,c.b,c.a);
        qc = QColorDialog::getRgba(qc.rgba(), &ok);
        if (ok) {
            c.r = qc.red();
            c.g = qc.green();
            c.b = qc.blue();
            c.a = qc.alpha();
            p->setColor(c);
            painter_->changed();
        }
    }
}

void TransFuncIntensityGradientPlugin::showHistogram() {
    painter_->setHistogramVisible(histogramEnabledButton_->isChecked());
    histogramBrightness_->setEnabled(histogramEnabledButton_->isChecked());
    histogramLog_->setEnabled(histogramEnabledButton_->isChecked());
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::showGrid() {
    painter_->setGridVisible(gridEnabledButton_->isChecked());
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::dataSourceChanged(Volume* newDataset) {
    if (!painter_)
        return;

    painter_->dataSourceChanged(newDataset);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->setEnabled(true);
    histogramBrightness_->setValue(100);

    int bits = newDataset->getBitsStored();
    switch (bits) {
        case 8:
            scaleFactor_ = 1.0f/255.0f;
            break;
        case 12:
            scaleFactor_ = 1.0f/4095.0f;
            break;
        case 16:
            scaleFactor_ = 1.0f/65535.0f;
            break;
        case 32:
            scaleFactor_ = 1.0f/255.0f;
            break;
    }
}

TransFunc* TransFuncIntensityGradientPlugin::getTransFunc() {
    if (!painter_)
        return 0;
    return painter_->getTransFunc(); 
}

void TransFuncIntensityGradientPlugin::adjustHistogramBrightness(int p) {
    painter_->setHistogramBrightness(p/100.0);
    painter_->updateHistogramTex();
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::setHistogramLog(int s) {
    painter_->setHistogramLog(s);
    painter_->updateHistogramTex();
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::adjustFuzziness(int p) {
    TransFuncPrimitive* pr = painter_->getSelectedPrimitive();
    if (pr) {
        pr->setFuzziness(p/100.0);
        painter_->changed();
    }
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::adjustTransparency(int p) {
    TransFuncPrimitive* pr = painter_->getSelectedPrimitive();
    if (pr) {
        tgt::col4 c = pr->getColor();
        c.a = p;
        pr->setColor(c);
        painter_->changed();
    }
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::setThresholds(int l, int u) {
    if (!painter_)
        return;
    painter_->setThresholds(l*scaleFactor_,u*scaleFactor_);
    transCanvas_->update();
}

void TransFuncIntensityGradientPlugin::transFuncChanged() {
    sendTFMessage();
}

tgt::col4 TransFuncIntensityGradientPlugin::colorChooser(tgt::col4 c, bool &changed) {
    bool ok;
    QColor qc;
    qc.setRgb(c.r,c.g,c.b,c.a);
    changed = false;
    qc = QColorDialog::getRgba(qc.rgba(), &ok);
    if (ok) {
        c.r = qc.red();
        c.g = qc.green();
        c.b = qc.blue();
        c.a = qc.alpha();
        changed = true;
        painter_->changed();
    }
    return c;
}

void TransFuncIntensityGradientPlugin::selected() {
    TransFuncPrimitive* pr = painter_->getSelectedPrimitive();
    if (pr) {
        fuzziness_->setValue(static_cast<int>(pr->getFuzziness() * 100.f));
        fuzziness_->setEnabled(true);
        transparency_->setValue(pr->getColor().a);
        transparency_->setEnabled(true);
    }
}

void TransFuncIntensityGradientPlugin::deselected() {
    fuzziness_->setValue(0);
    fuzziness_->setEnabled(false);
    transparency_->setValue(0);
    transparency_->setEnabled(false);
}
    
void TransFuncIntensityGradientPlugin::setTransFunc(TransFuncIntensityGradientPrimitiveContainer* tf) {
    painter_->setTransFunc(tf);
    update();
}

void TransFuncIntensityGradientPlugin::sliderPressed() {
    setInteractionCoarseness(true);
}

void TransFuncIntensityGradientPlugin::sliderReleased() {
    setInteractionCoarseness(false);
}

}
