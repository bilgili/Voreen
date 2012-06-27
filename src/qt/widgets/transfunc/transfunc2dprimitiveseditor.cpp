/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/qt/widgets/transfunc/transfunc2dprimitiveseditor.h"

#include "voreen/qt/widgets/transfunc/transfunc2dprimitivespainter.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"
#include "voreen/core/datastructures/volume/volumeram.h"

#include "tgt/qt/qtcanvas.h"
#include "tgt/logmanager.h"
#include "tgt/gpucapabilities.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QGLContext>
#include <QLabel>
#include <QMessageBox>
#include <QSlider>
#include <QSplitter>
#include <QToolButton>


namespace voreen {

const std::string TransFunc2DPrimitivesEditor::loggerCat_("voreen.qt.TransFunc2DPrimitivesEditor");

TransFunc2DPrimitivesEditor::TransFunc2DPrimitivesEditor(TransFuncProperty* prop, QWidget* parent,
                                                                   Qt::Orientation orientation)
    : TransFuncEditor(prop, parent)
    , transCanvas_(0)
    , painter_(0)
    , orientation_(orientation)
    , maximumIntensity_(255)
{
    title_ = QString("Intensity Gradient (2D)");

    transFuncGradient_ = dynamic_cast<TransFunc2DPrimitives*>(property_->get());

    supported_ = GpuCaps.areFramebufferObjectsSupported();
}

TransFunc2DPrimitivesEditor::~TransFunc2DPrimitivesEditor() {
    delete painter_;
}

QLayout* TransFunc2DPrimitivesEditor::createMappingLayout() {
    QLayout* layout = new QHBoxLayout();

    if (supported_) {
        transCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true);
        transCanvas_->getGLFocus();
        //transCanvas_->setMinimumSize(200, 100);

        painter_ = new TransFunc2DPrimitivesPainter(transCanvas_);
        painter_->initialize();
        if (transFuncGradient_)
            painter_->setTransFunc(transFuncGradient_);

        transCanvas_->setPainter(painter_);

        layout->addWidget(transCanvas_);
    }
    else
        layout->addWidget(new QLabel(tr("Your graphics card does not support FramebufferObjects!")));

    return layout;
}

QLayout* TransFunc2DPrimitivesEditor::createButtonLayout() {
    QBoxLayout* buttonLayout;
    if (orientation_ == Qt::Vertical)
        buttonLayout = new QHBoxLayout();
    else
        buttonLayout = new QVBoxLayout();

    clearButton_ = new QToolButton();
    clearButton_->setIcon(QIcon(":/qt/icons/clear.png"));
    clearButton_->setToolTip(tr("Reset transfer function to default"));

    loadButton_ = new QToolButton();
    loadButton_->setIcon(QIcon(":/qt/icons/open.png"));
    loadButton_->setToolTip(tr("Load transfer function"));

    saveButton_ = new QToolButton();
    saveButton_->setIcon(QIcon(":/qt/icons/save.png"));
    saveButton_->setToolTip(tr("Save transfer function"));

    gridEnabledButton_ = new QToolButton();
    gridEnabledButton_->setCheckable(true);
    gridEnabledButton_->setChecked(false);
    gridEnabledButton_->setIcon(QIcon(":/qt/icons/grid.png"));
    gridEnabledButton_->setToolTip(tr("Show grid"));

    histogramEnabledButton_ = new QToolButton();
    histogramEnabledButton_->setCheckable(true);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->setEnabled(false);
    histogramEnabledButton_->setIcon(QIcon(":/qt/icons/histogram.png"));
    histogramEnabledButton_->setToolTip(tr("Show data histogram"));

    buttonLayout->addWidget(clearButton_);
    buttonLayout->addWidget(loadButton_);
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addSpacing(4);
    buttonLayout->addWidget(gridEnabledButton_);
    buttonLayout->addWidget(histogramEnabledButton_);

    buttonLayout->addStretch();

    return buttonLayout;
}

QLayout* TransFunc2DPrimitivesEditor::createPrimitivesButtonLayout() {
    QBoxLayout* buttonLayout;
    buttonLayout = new QHBoxLayout();

    quadButton_ = new QToolButton();
    quadButton_->setIcon(QIcon(":/qt/icons/quad.png"));
    quadButton_->setToolTip(tr("Add a quad"));

    bananaButton_ = new QToolButton();
    bananaButton_->setIcon(QIcon(":/qt/icons/banana.png"));
    bananaButton_->setToolTip(tr("Add a banana"));

    deleteButton_ = new QToolButton();
    deleteButton_->setIcon(QIcon(":/qt/icons/eraser.png"));
    deleteButton_->setToolTip(tr("Delete selected primitive"));

    colorButton_ = new QToolButton();
    colorButton_->setIcon(QIcon(":/qt/icons/colorize.png"));
    colorButton_->setToolTip(tr("Change the color of the selected primitive"));

    buttonLayout->addWidget(quadButton_);
    buttonLayout->addWidget(bananaButton_);
    buttonLayout->addWidget(deleteButton_);
    buttonLayout->addWidget(colorButton_);

    buttonLayout->addStretch();

    return buttonLayout;
}

QLayout* TransFunc2DPrimitivesEditor::createSliderLayout() {
    QVBoxLayout* layout = new QVBoxLayout();

    histogramBrightness_ = new QSlider(Qt::Horizontal);
    histogramBrightness_->setEnabled(false);
    histogramBrightness_->setMinimum(10);
    histogramBrightness_->setMaximum(1000);
    histogramBrightness_->setValue(100);

    histogramLog_ = new QCheckBox(tr("Logarithmic Histogram"));
    histogramLog_->setEnabled(false);
    histogramLog_->setCheckState(Qt::Checked);

    transparency_ = new QSlider(Qt::Horizontal);
    transparency_->setEnabled(false);
    transparency_->setMinimum(0);
    transparency_->setMaximum(255);

    fuzziness_ = new QSlider(Qt::Horizontal);
    fuzziness_->setEnabled(false);
    fuzziness_->setMinimum(0);
    fuzziness_->setMaximum(100);

    layout->addWidget(new QLabel(tr("Histogram Brightness:")));
    layout->addWidget(histogramBrightness_);
    layout->addWidget(histogramLog_);
    layout->addWidget(new QLabel(tr("Transparency:")));
    layout->addWidget(transparency_);
    layout->addWidget(new QLabel(tr("Fuzziness:")));
    layout->addWidget(fuzziness_);

    layout->addStretch();

    return layout;
}

void TransFunc2DPrimitivesEditor::createWidgets() {
    QWidget* mapping = new QWidget();
    QWidget* slider = new QWidget();

    QLayout* mappingLayout = createMappingLayout();
    QLayout* buttonLayout = createButtonLayout();
    QLayout* primitivesButtonLayout = createPrimitivesButtonLayout();
    QLayout* sliderLayout = createSliderLayout();

    QSplitter* splitter = new QSplitter(orientation_);
    QLayout* buttonSlider;
    if (orientation_ == Qt::Vertical) {
        buttonSlider = new QVBoxLayout();
        buttonSlider->addItem(buttonLayout);
        buttonSlider->addItem(primitivesButtonLayout);
        buttonSlider->addItem(mappingLayout);
        mapping->setLayout(buttonSlider);
        slider->setLayout(sliderLayout);
    }
    else {
        buttonSlider = new QHBoxLayout();
        buttonSlider->addItem(buttonLayout);
        buttonSlider->addItem(new QSpacerItem(5, 1));
        buttonSlider->addItem(sliderLayout);

        QVBoxLayout* vLayout = new QVBoxLayout();
        vLayout->addItem(buttonSlider);
        vLayout->addItem(primitivesButtonLayout);
        vLayout->addStretch(10);

        mapping->setLayout(mappingLayout);
        slider->setLayout(vLayout);
    }

    splitter->setChildrenCollapsible(false);
    splitter->addWidget(mapping);
    splitter->addWidget(slider);

    //mapping is more stretched then buttons and slider
    splitter->setStretchFactor(0, 5);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->addWidget(splitter);

    setLayout(mainLayout);
}

void TransFunc2DPrimitivesEditor::createConnections() {
    if (!supported_)
        return;

    // buttons
    connect(loadButton_,  SIGNAL(clicked()), this, SLOT(loadTransferFunction()));
    connect(saveButton_,  SIGNAL(clicked()), this, SLOT(saveTransferFunction()));
    connect(clearButton_, SIGNAL(clicked()), painter_, SLOT(resetTransferFunction()));

    connect(gridEnabledButton_,      SIGNAL(clicked()), this, SLOT(toggleShowGrid()));
    connect(histogramEnabledButton_, SIGNAL(clicked()), this, SLOT(toggleShowHistogram()));

    connect(quadButton_,   SIGNAL(clicked()), painter_, SLOT(addQuadPrimitive()));
    connect(bananaButton_, SIGNAL(clicked()), painter_, SLOT(addBananaPrimitive()));
    connect(deleteButton_, SIGNAL(clicked()), painter_, SLOT(deletePrimitive()));
    connect(colorButton_,  SIGNAL(clicked()), painter_, SLOT(colorizePrimitive()));

    connect(histogramBrightness_, SIGNAL(sliderMoved(int)), painter_, SLOT(histogramBrightnessChanged(int)));
    connect(histogramLog_, SIGNAL(stateChanged(int)), painter_, SLOT(toggleHistogramLogarithmic(int)));

    // slider
    connect(fuzziness_, SIGNAL(valueChanged(int)), painter_, SLOT(fuzzinessChanged(int)));
    connect(transparency_, SIGNAL(valueChanged(int)), painter_, SLOT(transparencyChanged(int)));

    connect(fuzziness_, SIGNAL(sliderPressed()), this, SLOT(startTracking()));
    connect(transparency_, SIGNAL(sliderPressed()), this, SLOT(startTracking()));

    connect(fuzziness_, SIGNAL(sliderReleased()), this, SLOT(stopTracking()));
    connect(transparency_, SIGNAL(sliderReleased()), this, SLOT(stopTracking()));

    connect(painter_, SIGNAL(setTransparencySlider(int)), this, SLOT(setTransparency(int)));
    connect(painter_, SIGNAL(primitiveDeselected()), this, SLOT(primitiveDeselected()));
    connect(painter_, SIGNAL(primitiveSelected()), this, SLOT(primitiveSelected()));
    connect(painter_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));
    connect(painter_, SIGNAL(repaintSignal()), this, SLOT(repaintSignal()));

}

void TransFunc2DPrimitivesEditor::loadTransferFunction() {
    // create filter with supported file formats
    QString filter = "transfer function (";
    for (size_t i = 0; i < transFuncGradient_->getLoadFileFormats().size(); ++i) {
        std::string temp = "*." + transFuncGradient_->getLoadFileFormats()[i] + " ";
        filter.append(temp.c_str());
    }
    filter.replace(filter.length()-1, 1, ")");

    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        // deselect current selected primitive. Primitive gets deleted on successfull loading.
        painter_->deselectPrimitive();
        if (transFuncGradient_->load(fileName.toStdString())) {
            painter_->updateTF();
            transCanvas_->update();
        }
        else {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The selected transfer function could not be loaded."));
            LERROR("The selected transfer function could not be loaded. Maybe the file is corrupt.");
        }
    }
}

void TransFunc2DPrimitivesEditor::saveTransferFunction() {
    QStringList filter;
    for (size_t i = 0; i < transFuncGradient_->getSaveFileFormats().size(); ++i) {
        std::string temp = "transfer function (*." + transFuncGradient_->getSaveFileFormats()[i] + ")";
        filter << temp.c_str();
    }

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        //save transfer function to disk
        if (!transFuncGradient_->save(fileName.toStdString())) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("The transfer function could not be saved."));
            LERROR("The transfer function could not be saved. Maybe the disk is full?");
        }
    }
}

void TransFunc2DPrimitivesEditor::toggleShowGrid() {
    painter_->toggleShowGrid(gridEnabledButton_->isChecked());
    transCanvas_->update();
}

void TransFunc2DPrimitivesEditor::toggleShowHistogram() {
    painter_->setHistogramVisible(histogramEnabledButton_->isChecked());
    histogramBrightness_->setEnabled(histogramEnabledButton_->isChecked());
    histogramLog_->setEnabled(histogramEnabledButton_->isChecked());
    transCanvas_->update();
}

void TransFunc2DPrimitivesEditor::primitiveSelected() {
    const TransFuncPrimitive* p = painter_->getSelectedPrimitive();
    fuzziness_->setValue(static_cast<int>(p->getFuzziness() * 100.f));
    fuzziness_->setEnabled(true);
    transparency_->setValue(p->getColor().a);
    transparency_->setEnabled(true);
}

void TransFunc2DPrimitivesEditor::primitiveDeselected() {
    fuzziness_->setValue(0);
    fuzziness_->setEnabled(false);
    transparency_->setValue(0);
    transparency_->setEnabled(false);
}

void TransFunc2DPrimitivesEditor::setTransparency(int trans) {
    transparency_->blockSignals(true);
    transparency_->setValue(trans);
    transparency_->blockSignals(false);
}

void TransFunc2DPrimitivesEditor::startTracking() {
    toggleInteractionMode(true);
}

void TransFunc2DPrimitivesEditor::stopTracking() {
    toggleInteractionMode(false);
}

void TransFunc2DPrimitivesEditor::updateFromProperty() {
    if (!supported_)
        return;

    const VolumeBase* newHandle = property_->getVolumeHandle();
    if (newHandle != volumeHandle_) {
        volumeHandle_ = newHandle;
        volumeChanged();
    }

    if (transFuncGradient_)
        transCanvas_->update();
    else
        resetEditor();
}

void TransFunc2DPrimitivesEditor::volumeChanged() {
    // update control elements
    histogramEnabledButton_->setEnabled(true);
    histogramEnabledButton_->blockSignals(true);
    histogramEnabledButton_->setChecked(false);
    histogramEnabledButton_->blockSignals(false);
    histogramBrightness_->setEnabled(false);
    histogramLog_->setEnabled(false);
    histogramBrightness_->blockSignals(true);
    histogramBrightness_->setValue(100);
    histogramBrightness_->blockSignals(false);

    if (volumeHandle_ && volumeHandle_->getRepresentation<VolumeRAM>()) {
        int bits = volumeHandle_->getRepresentation<VolumeRAM>()->getBitsAllocated() / volumeHandle_->getRepresentation<VolumeRAM>()->getNumChannels();
        maximumIntensity_ = static_cast<int>(pow(2.f, static_cast<float>(bits)))-1;
    }

    // propagate volume to painter where the histogram is calculated
    if (volumeHandle_)
        painter_->volumeChanged(volumeHandle_);
    else
        painter_->volumeChanged(0);
}

void TransFunc2DPrimitivesEditor::resetEditor() {
    if (!supported_)
        return;

    if (property_->get() != transFuncGradient_) {
        LDEBUG("The pointers of property and transfer function do not match."
                << "Creating new transfer function object.....");

        // Need to make the GL context of this context current, as we use it for OpenGL calls
        // with the transFuncGradient_ later on.
        transCanvas_->getGLFocus();
        transFuncGradient_ = new TransFunc2DPrimitives(maximumIntensity_ + 1);
        property_->set(transFuncGradient_);

        painter_->setTransFunc(transFuncGradient_);
    }

    // reset transfer function to default, e.g. empty tf
    painter_->resetTransferFunction();

    // update mapping widget, e.g. canvas
    transCanvas_->update();

    // cause repaint of volume rendering
    property_->notifyChange();
    emit transferFunctionChanged();
}


void TransFunc2DPrimitivesEditor::repaintSignal() {
    property_->notifyChange();

    emit transferFunctionChanged();
}

} // namespace voreen
