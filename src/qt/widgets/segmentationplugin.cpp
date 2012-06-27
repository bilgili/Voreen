/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/segmentationplugin.h"

#include "voreen/core/vis/processors/render/volumeraycaster.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"
#include "voreen/qt/widgets/thresholdwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

namespace voreen {

using tgt::ivec2;

SegmentationPlugin::SegmentationPlugin(QWidget* parent, NetworkEvaluator* evaluator)
    : WidgetPlugin(parent, 0),
      evaluator_(evaluator),
      transFuncProp_("Segmentation Plugin TF", "Transfer Function"),
      intensityEditor_(0)
{
    setObjectName(tr("Segmentation"));
    icon_ = QIcon(":/icons/segmentation.png");

    transFuncProp_.disableEditor(TransFuncProp::Editors(TransFuncProp::ALL & ~TransFuncProp::INTENSITY));
    transFuncProp_.onChange(CallMemberAction<SegmentationPlugin>(this, &SegmentationPlugin::transFuncChanged));
    MsgDistr.insert(this);

    createWidgets();
    createConnections();

    MsgDistr.postMessage(new TemplateMessage<tgt::EventListener*>(VoreenPainter::addEventListener_, this));
}

SegmentationPlugin::~SegmentationPlugin() {
    MsgDistr.remove(this);
}

bool SegmentationPlugin::usable(const std::vector<Processor*>& processors) {
    for (size_t i=0; i < processors.size(); i++) {
        if (dynamic_cast<SegmentationRaycaster*>(processors[i]))
            return true;
        if (dynamic_cast<RegionGrowingProcessor*>(processors[i]))
            return true;
    }
    return false;
}

void SegmentationPlugin::createWidgets() {

    QVBoxLayout* mainLayout = new QVBoxLayout();

    //
    // Visualization Box
    //
    renderingBox_ = new QGroupBox(tr("Rendering"));

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QHBoxLayout* hboxLayout = new QHBoxLayout;

    // apply segmentation
    checkApplySegmentation_ = new QCheckBox(tr("Apply Segmentation"));
    hboxLayout->addWidget(checkApplySegmentation_);
    hboxLayout->addSpacing(20);

    // current segment
    spinCurrentSegment_ = new QSpinBox;
    spinCurrentSegment_->setRange(0, 1024);
    hboxLayout->addWidget(new QLabel(tr("Current Segment: ")));
    hboxLayout->addWidget(spinCurrentSegment_);
    hboxLayout->addSpacing(20);

    // segment visible
    checkSegmentVisible_ = new QCheckBox(tr("Segment Visible"));
    hboxLayout->addWidget(checkSegmentVisible_);
    hboxLayout->addStretch();

    vboxLayout->addItem(hboxLayout);

    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vboxLayout->addWidget(line);

    // tf editor
    intensityEditor_ = new TransFuncEditorIntensity(&transFuncProp_);
    intensityEditor_->createWidgets();
    intensityEditor_->createConnections();
    vboxLayout->addWidget(intensityEditor_);

    renderingBox_->setLayout(vboxLayout);
    mainLayout->addWidget(renderingBox_);

    //
    // Region Growing Box
    //
    regionGrowingBox_ = new QGroupBox(tr("Region Growing"));
    vboxLayout = new QVBoxLayout;
    hboxLayout = new QHBoxLayout;

    // mark seed button, output segment
    hboxLayout->addWidget(new QLabel(tr("Output Segment: ")));
    outputSegment_ = new QSpinBox;
    outputSegment_->setRange(0, 255);
    outputSegment_->setValue(1);
    hboxLayout->addWidget(outputSegment_);
    hboxLayout->addSpacing(10);
    markSeedButton_ = new QPushButton(tr(" Mark Seed "));
    markSeedButton_->setCheckable(true);
    hboxLayout->addWidget(markSeedButton_);
    hboxLayout->addSpacing(10);
    undoButton_ = new QPushButton(tr(" Undo Last "));
    hboxLayout->addWidget(undoButton_);
    hboxLayout->addSpacing(10);
    clearSegmentButton_ = new QPushButton(tr(" Clear Segment "));
    hboxLayout->addWidget(clearSegmentButton_);
    hboxLayout->addStretch();
    vboxLayout->addLayout(hboxLayout);
    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vboxLayout->addWidget(line);

    hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(new QLabel("Strictness: "));
    spinStrictness_ = new QDoubleSpinBox();
    spinStrictness_->setValue(0.8);
    spinStrictness_->setRange(0.0, 65000.0);
    spinStrictness_->setSingleStep(0.05);
    hboxLayout->addWidget(spinStrictness_);
    hboxLayout->addSpacing(10);
    spinMaxSeedDist_ = new QSpinBox();
    spinMaxSeedDist_->setValue(0);
    spinMaxSeedDist_->setRange(0,999);
    hboxLayout->addWidget(new QLabel(tr("Max distance to seed:" )));
    hboxLayout->addWidget(spinMaxSeedDist_);
    hboxLayout->addSpacing(10);
    checkApplyThresholds_ = new QCheckBox(tr("Apply Thresholds"));
    checkApplyThresholds_->setChecked(true);
    hboxLayout->addWidget(checkApplyThresholds_);
    hboxLayout->addStretch();
    vboxLayout->addLayout(hboxLayout);

    hboxLayout = new QHBoxLayout;
    comboCostFunction_ = new QComboBox();
    comboCostFunction_->addItem(tr("Intensity"));
    comboCostFunction_->addItem(tr("Gradient Magnitude"));
    comboCostFunction_->addItem(tr("Weighted"));
    hboxLayout->addWidget(new QLabel(tr("Cost Function: ")));
    hboxLayout->addWidget(comboCostFunction_);
    hboxLayout->addSpacing(15);
    checkAdaptive_ = new QCheckBox(tr("Adaptive Growing Criteria"));
    hboxLayout->addWidget(checkAdaptive_);
    hboxLayout->addStretch();
    vboxLayout->addLayout(hboxLayout);


    // clear / save buttons
    line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    vboxLayout->addWidget(line);
    hboxLayout = new QHBoxLayout;

    clearSegmentationButton_ = new QPushButton(tr(" Clear Segmentation "));
    hboxLayout->addWidget(clearSegmentationButton_);
    hboxLayout->addSpacing(5);
    saveSegmentationButton_ = new QPushButton(tr(" Save Segmentation "));
    hboxLayout->addWidget(saveSegmentationButton_);
    hboxLayout->addSpacing(5);
    loadSegmentationButton_ = new QPushButton(tr(" Load Segmentation "));
    loadSegmentationButton_->setEnabled(false);
    hboxLayout->addWidget(loadSegmentationButton_);
    hboxLayout->addStretch();
    vboxLayout->addItem(hboxLayout);

    // threshold widget
    /*thresholdWidget_ = new ThresholdWidget(this);
    thresholdWidget_->setMinValue(0);
    thresholdWidget_->setMaxValue(0xFFFF);
    thresholdWidget_->setValues(0, 0xFFFF);
    vboxLayout->addWidget(thresholdWidget_);*/

    regionGrowingBox_->setLayout(vboxLayout);
    mainLayout->addWidget(regionGrowingBox_);

    // finish layout
    mainLayout->addStretch();
    setLayout(mainLayout);
}

void SegmentationPlugin::createConnections() {
    // connections
    connect(checkApplySegmentation_, SIGNAL(toggled(bool)), this, SLOT(toggleApplySegmentation(bool)));
    connect(spinCurrentSegment_, SIGNAL(valueChanged(int)), this, SLOT(setCurrentSegment(int)));
    connect(checkSegmentVisible_, SIGNAL(toggled(bool)), this, SLOT(toggleSegmentVisible(bool)));

    connect(clearSegmentButton_, SIGNAL(clicked()), this, SLOT(clearSegment()));
    connect(undoButton_, SIGNAL(clicked()), this, SLOT(undoSegment()));
    connect(markSeedButton_, SIGNAL(clicked(bool)), this, SLOT(setSeed(bool)));

    connect(saveSegmentationButton_, SIGNAL(clicked()), this, SLOT(saveSegmentation()));
    connect(clearSegmentationButton_, SIGNAL(clicked()), this, SLOT(clearSegmentation()));

    //connect(thresholdWidget_, SIGNAL(valuesChanged(int, int)), this, SLOT(setThresholds(int, int)));
}

void SegmentationPlugin::processMessage(Message* msg, const Identifier& /*dest*/) {
    if (msg->id_ == "evaluatorUpdated") {
    }
}

void SegmentationPlugin::toggleApplySegmentation(bool useSegmentation) {

    if (!getSegmentationRaycaster())
        return;

    getSegmentationRaycaster()->getApplySegmentationProp().set(useSegmentation);
    synchronizeTransFuncs(PULL);
    checkSegmentVisible_->setChecked(getSegmentationRaycaster()->getSegmentVisibleProp().get());

    repaintCanvases();

}

void SegmentationPlugin::setCurrentSegment(int segment) {
    if (getSegmentationRaycaster()) {
        getSegmentationRaycaster()->getCurrentSegmentProp().set(segment);
        synchronizeTransFuncs(PULL);
        checkSegmentVisible_->setChecked(getSegmentationRaycaster()->getSegmentVisibleProp().get());
    }
}

void SegmentationPlugin::toggleSegmentVisible(bool visible) {
    if (getSegmentationRaycaster() && getSegmentationRaycaster()->getApplySegmentationProp().get()) {
        getSegmentationRaycaster()->getSegmentVisibleProp().set(visible);
        repaintCanvases();
    }
}

void SegmentationPlugin::undoSegment() {

    if (getRegionGrowingProcessor()) {
        getRegionGrowingProcessor()->undoLastGrowing();
        repaintCanvases();
    }
}

void SegmentationPlugin::clearSegment() {

    if (getRegionGrowingProcessor()) {
        getRegionGrowingProcessor()->clearSegment(outputSegment_->value());
        repaintCanvases();
    }

}

void SegmentationPlugin::setSeed(bool checked) {

    if (!getRegionGrowingProcessor()) {
        if (checked)
            markSeedButton_->setChecked(false);
        return;
    }

}

void SegmentationPlugin::mousePressEvent(tgt::MouseEvent* e) {

    e->ignore();

    if (e->button() != tgt::MouseEvent::MOUSE_BUTTON_LEFT ||
        !markSeedButton_->isChecked() ||
        !getRegionGrowingProcessor()) {
        return;
    }

    if (!evaluator_) {
        LERRORC("voreen.qt.SegmentationPlugin", "No network evaluator");
        return;
    }

    if (!evaluator_->getTextureContainer()) {
        LERRORC("voreen.qt.SegmentationPlugin", "No texture container");
        return;
    }

    e->accept();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // propagate local settings to region growing processor
    propagateRegionGrowingParams();

    // propagate picking coords to RegionGrowing processor and start growing
    ivec2 tcDims = evaluator_->getTextureContainer()->getSize();
    getRegionGrowingProcessor()->startGrowing(ivec2(e->coord().x, tcDims.y - e->coord().y), outputSegment_->value());
    QApplication::restoreOverrideCursor();

    repaintCanvases();
}

void SegmentationPlugin::setThresholds(int /*lower*/, int /*upper*/) {

   /* WidgetPlugin::postMessage(new FloatMsg(VolumeRenderer::setLowerThreshold_,
                             (float)lower / thresholdWidget_->getMaxValue()));
    WidgetPlugin::postMessage(new FloatMsg(VolumeRenderer::setUpperThreshold_,
                             (float)upper / thresholdWidget_->getMaxValue()));
    repaintCanvases(); */
}

void SegmentationPlugin::saveSegmentation() {

    if (!getRegionGrowingProcessor())
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Segmentation"),
                                                    QDir::currentPath() + "/../../data/segtest.dat",
                                                    tr("Volumes (*.dat)"));
    if (!fileName.isEmpty())
        getRegionGrowingProcessor()->saveSegmentation(fileName.toStdString());
}

SegmentationRaycaster* SegmentationPlugin::getSegmentationRaycaster() {

    if (!evaluator_) {
        LERRORC("voreen.qt.SegmentationPlugin", "No network evaluator.");
        return 0;
    }

    SegmentationRaycaster* segproc = 0;
    std::vector<Processor*>& procs = evaluator_->getProcessors();
    for (size_t i=0; i < procs.size(); i++) {
        if ((segproc = dynamic_cast<SegmentationRaycaster*>(procs[i])))
            break;
    }

    if (!segproc)
        QMessageBox::warning(this, tr("No SegmentationRaycaster"), tr("No SegmentationRaycaster found in the current network."));

    return segproc;
}

RegionGrowingProcessor* SegmentationPlugin::getRegionGrowingProcessor() {

    if (!evaluator_) {
        LERRORC("voreen.qt.SegmentationPlugin", "No network evaluator.");
        return 0;
    }

    RegionGrowingProcessor* regionGrowing = 0;
    std::vector<Processor*>& procs = evaluator_->getProcessors();
    for (size_t i=0; i < procs.size(); i++) {
        if ((regionGrowing = dynamic_cast<RegionGrowingProcessor*>(procs[i])))
            break;
    }

    if (!regionGrowing)
        QMessageBox::warning(this, tr("No RegionGrowing processor"), tr("No RegionGrowing processor found in the current network."));

    return regionGrowing;
}

void SegmentationPlugin::transFuncChanged() {
    synchronizeTransFuncs(PUSH);
}

void SegmentationPlugin::synchronizeTransFuncs(TransFuncSyncDirection syncDir) {

    if (!getSegmentationRaycaster())
        return;

    TransFuncProp& tfProp = getSegmentationRaycaster()->getTransFunc();
    TransFuncIntensity* tfIntensitySegmentation = dynamic_cast<TransFuncIntensity*>(tfProp.get());
    TransFuncIntensity* tfIntensity = dynamic_cast<TransFuncIntensity*>(transFuncProp_.get());

    if (tfIntensity && tfIntensitySegmentation) {

        TransFuncIntensity* sourceTF;
        TransFuncIntensity* destTF;
        if (syncDir == PULL) {
            sourceTF = tfIntensitySegmentation;
            destTF = tfIntensity;
        }
        else {
            sourceTF = tfIntensity;
            destTF = tfIntensitySegmentation;
        }

        const std::vector<TransFuncMappingKey*> keys = sourceTF->getKeys();
        destTF->clearKeys();
        for (size_t i=0; i<keys.size(); i++)
            destTF->addKey( new TransFuncMappingKey(*keys[i]) );

        tgt::vec2 thresholds = sourceTF->getThresholds();
        if (syncDir == PULL) {
            intensityEditor_->thresholdChanged(thresholds.x, thresholds.y);
            intensityEditor_->update();
        }
        else {
            destTF->setThresholds(thresholds);
            tfProp.updateWidgets();
            tfProp.notifyChange();
            repaintCanvases();
        }

    }
    else {
        LWARNINGC("voreen.qt.SegmentationPlugin", "1D transfer functions expected.");
    }

}

void SegmentationPlugin::clearSegmentation() {
    if (getRegionGrowingProcessor()) {
        getRegionGrowingProcessor()->clearSegmentation();
        repaintCanvases();
    }
}

void SegmentationPlugin::propagateRegionGrowingParams() {

    if (!getRegionGrowingProcessor())
        return;

    getRegionGrowingProcessor()->getStrictnessProp().set(spinStrictness_->value());
    getRegionGrowingProcessor()->getCostFunctionProp()->set(comboCostFunction_->currentIndex());
    getRegionGrowingProcessor()->getMaxSeedDistanceProp().set(spinMaxSeedDist_->value());
    getRegionGrowingProcessor()->getAdaptiveProp().set(checkAdaptive_->isChecked());
    if (checkApplyThresholds_->isChecked()) {
        TransFuncIntensity* tfIntensity = dynamic_cast<TransFuncIntensity*>(transFuncProp_.get());
        if (tfIntensity)
            getRegionGrowingProcessor()->getThresholdProp().set(tfIntensity->getThresholds());
    }
    getRegionGrowingProcessor()->getThresholdFillingProp().set(checkApplyThresholds_->isChecked());

}

} // namespace voreen
