/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreen/qt/widgets/property/transfuncpropertywidget.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/transfunc/transfuncplugin.h"
#include "voreen/qt/widgets/transfunc/doubleslider.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/qt/widgets/transfunc/transfunc1dhistogrampainter.h"
#include "voreen/qt/widgets/customlabel.h"

#include <QToolButton>
#include <QMenu>
#include "tgt/qt/qtcanvas.h"
#include "tgt/filesystem.h"

namespace voreen {

TransFuncPropertyWidget::TransFuncPropertyWidget(TransFuncProperty* prop, QWidget* parent)
    : QPropertyWidgetWithEditorWindow(prop, parent, false)
    , plugin_(0)
    , property_(prop)
    , texturePainter_(0)
    , doubleSlider_(0)
    , ignoreSlideUpdates_(false)
    , viewInitialized_(false)
    , editBt_(new QToolButton())
    , useAlphaBt_(new QToolButton())
    , templateBt_(new QToolButton())
    , windowBt_(new QToolButton())
    , zoomInBt_(new QToolButton())
    , zoomOutBt_(new QToolButton())
    , zoomResetBt_(new QToolButton())
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignLeft);
    mainLayout->setContentsMargins(0, 2, 0, 0);
    layout_->addLayout(mainLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignLeft);
    buttonLayout->setContentsMargins(0, 0, 4, 0);
    buttonLayout->setSpacing(4);
    buttonLayout->setMargin(0);
    mainLayout->addLayout(buttonLayout);

    TransFunc1DKeys* transferFuncIntensity_ = dynamic_cast<TransFunc1DKeys*>(property_->get());

    textureCanvas_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true);
    texturePainter_ = new TransFunc1DHistogramPainter(textureCanvas_);
    texturePainter_->initialize();
    texturePainter_->setTransFunc(transferFuncIntensity_);
    textureCanvas_->setPainter(texturePainter_, false);
    textureCanvas_->setFixedHeight(15);
    textureCanvas_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    mainLayout->addWidget(textureCanvas_);

    doubleSlider_ = new DoubleSlider();
    doubleSlider_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    doubleSlider_->showToolTip(true);
    //doubleSlider_->setOffsets(12, 27);
    mainLayout->addWidget(doubleSlider_);

    editBt_->setIcon(QPixmap(":/qt/icons/mapping-function.png"));
    editBt_->setToolTip("Editor");

    useAlphaBt_->setIcon(QPixmap(":/qt/icons/alpha.png"));
    useAlphaBt_->setToolTip("Use Alpha Channel");
    useAlphaBt_->setCheckable(true);
    useAlphaBt_->setChecked(true);
    if(prop && prop->get())
        useAlphaBt_->setChecked(!prop->get()->getIgnoreAlpha());

    loadTemplateMenu_ = new QMenu();
    templateBt_->setIcon(QPixmap(":/qt/icons/colorize.png"));
    templateBt_->setToolTip("Color Map Presets");
    templateBt_->setMenu(loadTemplateMenu_);
    templateBt_->setPopupMode(QToolButton::InstantPopup);
    connect(loadTemplateMenu_, SIGNAL(aboutToShow()), this, SLOT(populateLoadTemplateMenu()));
    connect(loadTemplateMenu_, SIGNAL(triggered(QAction*)), this, SLOT(loadTemplate(QAction*)));

    loadWindowMenu_ = new QMenu();
    windowBt_->setIcon(QPixmap(":/qt/icons/histogram_fit.png"));
    windowBt_->setToolTip("Window Fitting/Presets");
    windowBt_->setMenu(loadWindowMenu_);
    windowBt_->setPopupMode(QToolButton::InstantPopup);
    connect(loadWindowMenu_, SIGNAL(aboutToShow()), this, SLOT(populateLoadWindowMenu()));
    connect(loadWindowMenu_, SIGNAL(triggered(QAction*)), this, SLOT(loadWindow(QAction*)));

    zoomInBt_->setIcon(QPixmap(":/qt/icons/viewmag+.png"));
    zoomInBt_->setToolTip("Zoom In");
    zoomOutBt_->setIcon(QPixmap(":/qt/icons/viewmag_.png"));
    zoomOutBt_->setToolTip("Zoom Out");
    zoomResetBt_->setIcon(QPixmap(":/qt/icons/viewmag.png"));
    zoomResetBt_->setToolTip("Reset Zoom");

    tgtAssert(prop, "No property passed");

    if (!prop->getLazyEditorInstantiation() || editorVisibleOnStartup())
        createEditorWindow(Qt::RightDockWidgetArea);

    buttonLayout->addWidget(QPropertyWidgetWithEditorWindow::getNameLabel(), 1);
    buttonLayout->addStretch();
    buttonLayout->addWidget(zoomInBt_, 1);
    buttonLayout->addWidget(zoomOutBt_, 1);
    buttonLayout->addWidget(zoomResetBt_, 1);
    buttonLayout->addStretch();
    buttonLayout->addWidget(windowBt_, 1);
    buttonLayout->addWidget(templateBt_, 1);
    buttonLayout->addWidget(useAlphaBt_, 1);
    buttonLayout->addWidget(editBt_, 1);

    connect(editBt_, SIGNAL(clicked()), this, SLOT(setProperty()));
    connect(editBt_, SIGNAL(clicked()), this, SIGNAL(widgetChanged()));
    connect(useAlphaBt_, SIGNAL(toggled(bool)), this, SLOT(setUseAlpha(bool)));
    connect(zoomInBt_, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(zoomOutBt_, SIGNAL(clicked()), this, SLOT(zoomOut()));
    connect(zoomResetBt_, SIGNAL(clicked()), this, SLOT(resetZoom()));
    connect(doubleSlider_, SIGNAL(valuesChanged(float, float)), this, SLOT(slidersMoved(float, float)));
    connect(doubleSlider_, SIGNAL(toggleInteractionMode(bool)), this, SLOT(toggleInteractionMode(bool)));

    addVisibilityControls();
    QFontInfo fontInfo(font());
    editBt_->setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));
}

void TransFuncPropertyWidget::updateFromPropertySlot() {
    if (plugin_)
        plugin_->updateFromProperty();

    const VolumeBase* vb = property_->getVolumeHandle();
    texturePainter_->setTransFunc(property_->get());

    // We want the view to reset when the TF is deserialized.
    // To achieve this we set the widget as not initialized as long as only the standard function has been set
    const TransFunc* tf = property_->get();
    if(!viewInitialized_) {
        if(tf) {
            const TransFunc1DKeys* tf1d = dynamic_cast<const TransFunc1DKeys*>(tf);
            if(tf1d) {
                if(!tf1d->isStandardFunc())
                    viewInitialized_ = true;
            }
            else
                viewInitialized_ = true;
        }

        if(viewInitialized_)
            resetZoom();
    }

    clearObserveds();
    if(vb) {
        vb->addObserver(this);
        doubleSlider_->setUnit(vb->getRealWorldMapping().getUnit());

        if(vb->hasDerivedData<VolumeHistogramIntensity>())
            texturePainter_->setHistogram(vb->getDerivedData<VolumeHistogramIntensity>());
        else
            vb->getDerivedDataThreaded<VolumeHistogramIntensity>();
    }
    else {
        texturePainter_->setHistogram(0);
    }

    if(tf) {
        useAlphaBt_->blockSignals(true);
        useAlphaBt_->setChecked(!tf->getIgnoreAlpha());
        useAlphaBt_->blockSignals(false);
    }

    textureCanvas_->update();
    adaptSliderToZoom();
}

void TransFuncPropertyWidget::zoomIn() {
    texturePainter_->zoomIn();
    textureCanvas_->update();
    adaptSliderToZoom();
}

void TransFuncPropertyWidget::zoomOut() {
    texturePainter_->zoomOut();
    textureCanvas_->update();
    adaptSliderToZoom();
}

void TransFuncPropertyWidget::resetZoom() {
    texturePainter_->resetZoom();
    textureCanvas_->update();
    adaptSliderToZoom();
}

void TransFuncPropertyWidget::adaptSliderToZoom() {
    float vl = texturePainter_->getViewLeft();
    float vs = texturePainter_->getViewRight() - texturePainter_->getViewLeft();
    float dl = property_->get()->getDomain().x;
    float dr = property_->get()->getDomain().y;

    float minNorm = (dl - vl) / vs;
    float maxNorm = (dr - vl) / vs;
    ignoreSlideUpdates_ = true;
    doubleSlider_->setMaxValue(maxNorm);
    doubleSlider_->setMinValue(minNorm);
    doubleSlider_->setMapping(vl, vl + vs);
    ignoreSlideUpdates_ = false;
}

void TransFuncPropertyWidget::slidersMoved(float min, float max) {
    if(!ignoreSlideUpdates_) {
        float vl = texturePainter_->getViewLeft();
        float vs = texturePainter_->getViewRight() - texturePainter_->getViewLeft();

        float minRW = vl + (min * vs);
        float maxRW = vl + (max * vs);

        property_->get()->setDomain(tgt::vec2(minRW, maxRW));
        property_->invalidate();
        textureCanvas_->update();
    }
}

void TransFuncPropertyWidget::fitToData() {
    property_->fitDomainToData();
    resetZoom();
}

void TransFuncPropertyWidget::setProperty() {
    if (!disconnected_) {
        // lazy instantiation of transfunc editor window
        if (!editorWindow_) {
            createEditorWindow(Qt::RightDockWidgetArea);
            tgtAssert(editorWindow_, "Transfunc editor not instantiated");
        }

        if (editorWindow_->isVisible()) {
            //close widget
            editorWindow_->close();
        }
        else {
            //open Widget
            editorWindow_->showNormal();
            plugin_->updateFromProperty();
        }
    }
}

void TransFuncPropertyWidget::disconnect() {
    disconnected_ = true;
    if (plugin_)
        plugin_->disconnect();
}

QWidget* TransFuncPropertyWidget::createEditorWindowWidget() {
    plugin_ = new TransFuncPlugin(property_, parentWidget(), Qt::Horizontal);
    plugin_->createWidgets();
    plugin_->createConnections();
    connect(plugin_, SIGNAL(transferFunctionChanged()), this, SIGNAL(modified()));

    return plugin_;
}

void TransFuncPropertyWidget::customizeEditorWindow() {
    editorWindow_->setAllowedAreas(Qt::RightDockWidgetArea);
    editorWindow_->setFloating(true);
}

Property* TransFuncPropertyWidget::getProperty() {
    return property_;
}

void populateMenuFromDirectory(QMenu* menu, std::string directory) {
    std::vector<std::string> subdirs = FileSys.listSubDirectories(directory, true);
    for(size_t i=0; i<subdirs.size(); i++) {
        QMenu* subMenu = menu->addMenu(QString::fromStdString(subdirs[i]));
        populateMenuFromDirectory(subMenu, directory + "/" + subdirs[i]);
    }

    std::vector<std::string> presets = FileSys.listFiles(directory, true);
    for(size_t i=0; i<presets.size(); i++) {
        QAction* propAction = new QAction(QString::fromStdString(FileSys.baseName(presets[i])), menu);
        propAction->setData(QVariant(QString::fromStdString(directory + "/" + presets[i])));
        menu->addAction(propAction);
    }
}

void TransFuncPropertyWidget::populateLoadTemplateMenu() {
    loadTemplateMenu_->clear();

    std::string applicationPresetPath = VoreenApplication::app()->getResourcePath("transferfuncs/presets");
    populateMenuFromDirectory(loadTemplateMenu_, applicationPresetPath);

    loadTemplateMenu_->addSeparator();

    std::string userPresetPath = VoreenApplication::app()->getUserDataPath("transferfuncs/presets");
    populateMenuFromDirectory(loadTemplateMenu_, userPresetPath);
}

void TransFuncPropertyWidget::loadTemplate(QAction* action) {
    QString data = action->data().toString();
    if(property_ && property_->get()) {
        tgt::vec2  oldDomain = property_->get()->getDomain();
        bool ignoreAlpha = property_->get()->getIgnoreAlpha();
        if (property_->get()->load(data.toStdString())) {
            if(property_->get()->getDomain() == tgt::vec2(0.0f, 1.0f)) {
                property_->get()->setDomain(oldDomain);
            }

            property_->get()->setIgnoreAlpha(ignoreAlpha);
            property_->invalidate();
        }
        else {
            //TODO: error
        }
    }
}

void TransFuncPropertyWidget::populateLoadWindowMenu() {
    loadWindowMenu_->clear();

    QAction* propAction = new QAction(QString::fromStdString("Fit to Data"), loadWindowMenu_);
    propAction->setData(QVariant(QString::fromStdString("fitToData")));
    loadWindowMenu_->addAction(propAction);

    loadWindowMenu_->addSeparator();

    std::string applicationPresetPath = VoreenApplication::app()->getResourcePath("transferfuncs/windows");
    populateMenuFromDirectory(loadWindowMenu_, applicationPresetPath);

    loadWindowMenu_->addSeparator();

    std::string userPresetPath = VoreenApplication::app()->getUserDataPath("transferfuncs/windows");
    populateMenuFromDirectory(loadWindowMenu_, userPresetPath);
}

void TransFuncPropertyWidget::loadWindow(QAction* action) {
    QString data = action->data().toString();
    if(property_ && property_->get()) {
        if(data == "fitToData") {
            fitToData();
        }
        else {
            tgt::File* file = FileSys.open(data.toStdString());
            if(file) {
                std::string content = file->getAsString();
                std::vector<std::string> expl = strSplit(content, '\n');
                if(expl.size() == 2) {
                    std::cout << expl[0] << expl[1];
                    float width = stof(expl[0]);
                    float level = stof(expl[1]);

                    tgt::vec2 domain = tgt::vec2(level - (width * 0.5f), level + (width * 0.5f));
                    property_->get()->setDomain(domain);
                    property_->invalidate();
                }
                else {
                    LERRORC("voreen.TransFuncPropertyWidget", "Failed to parse window preset file");
                }
            }
        }
    }
    else {
        LERRORC("voreen.TransFuncPropertyWidget", "Failed to open window preset file");
    }
}

void TransFuncPropertyWidget::showNameLabel(bool visible) {
    if (nameLabel_) {
        if (visible)
            nameLabel_->show();
        else
            nameLabel_->hide();
    }
}

CustomLabel* TransFuncPropertyWidget::getNameLabel() const {
    return 0;
}

void TransFuncPropertyWidget::toggleInteractionMode(bool on) {
    property_->toggleInteractionMode(on, this);
}

void TransFuncPropertyWidget::volumeDelete(const VolumeBase* source) {
    texturePainter_->setHistogram(0);
}

void TransFuncPropertyWidget::volumeChange(const VolumeBase* source) {
    texturePainter_->setHistogram(0);
}

void TransFuncPropertyWidget::derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData) {
    if(dynamic_cast<const VolumeHistogramIntensity*>(derivedData)) {
        texturePainter_->setHistogram(static_cast<const VolumeHistogramIntensity*>(derivedData));
        textureCanvas_->update();
    }
}

void TransFuncPropertyWidget::setUseAlpha(bool on) {
    if(property_ && property_->get()) {
        property_->get()->setIgnoreAlpha(!on);
        property_->invalidate();
        textureCanvas_->update();
    }
}

} // namespace voreen
