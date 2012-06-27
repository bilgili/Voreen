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

#include "voreen/qt/widgets/transfunc/transfuncplugin.h"

#include "voreen/qt/widgets/transfunc/transfunceditorintensity.h"
#include "voreen/qt/widgets/transfunc/transfunceditorintensitygradient.h"
#include "voreen/qt/widgets/transfunc/transfunceditorintensitypet.h"
#include "voreen/qt/widgets/transfunc/transfunceditorintensityramp.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"

#include <QComboBox>
#include <QMessageBox>
#include <QStackedWidget>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

namespace voreen {

TransFuncPlugin::TransFuncPlugin(Processor* proc, QWidget* parent, Qt::Orientation orientation)
    : QPropertyWidget(parent)
    , stackedWidget_(0)
    , disconnected_(false)
    , oldIndex_(-1)
{
    // search for transfer function properties in the given processor
    const std::vector<Property*> props = proc->getProperties();
    for (size_t i = 0; i < props.size(); ++i) {
        if (typeid(*props[i]) == typeid(TransFuncProp)) {
            properties_.push_back(static_cast<TransFuncProp*>(props[i]));
        }
    }

    // create widgets for all transfer function properties
    for (size_t i = 0; i < properties_.size(); ++i) {
        createEditors(i, parent, orientation);
        properties_[i]->addWidget(this);
    }
}


TransFuncPlugin::TransFuncPlugin(TransFuncProp* prop, QWidget* parent, Qt::Orientation orientation)
    : QPropertyWidget(parent)
    , stackedWidget_(0)
    , disconnected_(false)
    , oldIndex_(-1)
{
    properties_.push_back(prop);
    createEditors(0, parent, orientation);
}

void TransFuncPlugin::createEditors(int index, QWidget* parent, Qt::Orientation orientation) {
    TransFuncEditor* tfEditor;
    std::vector<TransFuncEditor*> editor;
    if (properties_[index]->isEditorEnabled(TransFuncProp::INTENSITY)) {
        tfEditor = new TransFuncEditorIntensity(properties_[index], parent, orientation);
        editor.push_back(tfEditor);
    }
    if (properties_[index]->isEditorEnabled(TransFuncProp::INTENSITY_RAMP)) {
        tfEditor = new TransFuncEditorIntensityRamp(properties_[index], parent, orientation);
        editor.push_back(tfEditor);
    }
    if (properties_[index]->isEditorEnabled(TransFuncProp::INTENSITY_PET)) {
        tfEditor = new TransFuncEditorIntensityPet(properties_[index], parent);
        editor.push_back(tfEditor);
    }
    if (properties_[index]->isEditorEnabled(TransFuncProp::INTENSITY_GRADIENT)) {
        tfEditor = new TransFuncEditorIntensityGradient(properties_[index], parent, orientation);
        editor.push_back(tfEditor);
    }

    editors_.push_back(editor);
}

TransFuncPlugin::~TransFuncPlugin() {
    editors_.clear();
    if (!disconnected_) {
        for (size_t i = 0; i < properties_.size(); ++i)
            properties_[i]->removeWidget(this);
    }
}

void TransFuncPlugin::disconnect() {
    disconnected_ = true;
}

void TransFuncPlugin::createWidgets() {
    stackedWidget_ = new QStackedWidget();
    for (size_t i = 0; i < editors_.size(); ++i) {
        QTabWidget* tab = new QTabWidget();
        for (size_t j = 0; j < editors_[i].size(); ++j) {
            editors_[i][j]->createWidgets();
            tab->addTab(editors_[i][j], editors_[i][j]->getTitle());
        }
        stackedWidget_->addWidget(tab);
        tabWidgets_.push_back(tab);
    }

    QVBoxLayout* vbox = new QVBoxLayout();

    if (editors_.size() > 1) {
        // add combobox when there is more than one transfer function property
        QComboBox* combo = new QComboBox();
        for (size_t i = 0; i < properties_.size(); ++i) {
            combo->addItem(properties_[i]->getGuiText().c_str());
        }
        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));

        vbox->addWidget(combo);
    }

    vbox->addWidget(stackedWidget_);
    setLayout(vbox);

    oldIndex_ = 0;
}

void TransFuncPlugin::comboBoxChanged(int index) {
    stackedWidget_->setCurrentIndex(index);
    oldIndex_ = tabWidgets_[index]->currentIndex();
    editors_[index][oldIndex_]->update();
}

void TransFuncPlugin::createConnections() {
    for (size_t i = 0; i < editors_.size(); ++i) {
        for (size_t j = 0; j < editors_[i].size(); ++j) {
            editors_[i][j]->createConnections();
            connect(editors_[i][j], SIGNAL(transferFunctionChanged()), this, SIGNAL(transferFunctionChanged()));
        }
        connect(tabWidgets_[i], SIGNAL(currentChanged(int)), this, SLOT(editorChanged(int)));
    }
}

void TransFuncPlugin::update() {
    if (editors_.size() > 0 && stackedWidget_) {
        int index = tabWidgets_[stackedWidget_->currentIndex()]->currentIndex();
        editors_[stackedWidget_->currentIndex()][index]->update();
    }
}

void TransFuncPlugin::editorChanged(int index) {
    if (index != oldIndex_) {
        //TODO: check for compatibility and not reset tf by default

        // set tabwidget back to previous editor
        QTabWidget* tabWidget = tabWidgets_[stackedWidget_->currentIndex()];
        tabWidget->blockSignals(true);
        tabWidget->setCurrentIndex(oldIndex_);
        tabWidget->blockSignals(false);
        QString text(tr("The Editor you selected is not compatible with the previous one.\n"));
        text.append(tr("The transfer function and the thresholds will be reset to default.\n\n"));
        text.append(tr("Do you want to proceed?"));
        int answer = QMessageBox::question(this, tr("Proceed?"), text, QMessageBox::Yes, QMessageBox::No);
        // set editor to new one when the user decides to proceed
        if (answer == QMessageBox::Yes) {
            oldIndex_ = index;

            tabWidget->blockSignals(true);
            tabWidget->setCurrentIndex(index);
            tabWidget->blockSignals(false);

            // resets transfer function and thresholds to default
            editors_[stackedWidget_->currentIndex()][index]->resetEditor();
            // updates editor to current rendered volume
            editors_[stackedWidget_->currentIndex()][index]->update();
        }
    }
}

void TransFuncPlugin::showEvent(QShowEvent* /*event*/) {
    int index = tabWidgets_[stackedWidget_->currentIndex()]->currentIndex();
    editors_[stackedWidget_->currentIndex()][index]->update();
}

} // namespace voreen
