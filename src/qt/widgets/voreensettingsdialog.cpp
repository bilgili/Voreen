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

#include "voreen/qt/widgets/voreensettingsdialog.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/properties/property.h"
#include "voreen/qt/widgets/customlabel.h"
#include "voreen/qt/widgets/property/propertyownerwidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>

namespace voreen {

VoreenSettingsDialog::VoreenSettingsDialog(QWidget* parent)
    : QDialog(parent, Qt::Tool | Qt::Window)
{
    setWindowTitle(tr("Voreen Settings Editor"));

    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    // scroll area
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);

    // container widget for PropertyOwnerWidgets
    QWidget* containerWidget = new QWidget();
    scrollArea->setWidget(containerWidget);
    QVBoxLayout* widgetLayout = new QVBoxLayout();
    containerWidget->setLayout(widgetLayout);

    // application widget
    PropertyOwnerWidget* appPow = new PropertyOwnerWidget(this, VoreenApplication::app(), "Application Settings", true, false);
    widgetLayout->addWidget(appPow);

    // module widgets
    VoreenApplication* va = VoreenApplication::app();
    const std::vector<VoreenModule*>& modules = va->getModules();
    for(size_t i=0; i<modules.size(); i++) {
        if(!modules[i]->getProperties().empty()) {
            widgetLayout->addSpacerItem(new QSpacerItem(1, 8));
            PropertyOwnerWidget* modulePow = new PropertyOwnerWidget(this, modules[i], "Module: " + modules[i]->getGuiName(), true, false);
            widgetLayout->addWidget(modulePow);
        }
    }
    widgetLayout->addStretch();

    // button row
    mainLayout->addSpacerItem(new QSpacerItem(1, 4));
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    resetButton_ = new QPushButton("Reset");
    buttonLayout->addWidget(resetButton_);
    buttonLayout->addStretch();
    closeButton_ = new QPushButton("Close");
    buttonLayout->addWidget(closeButton_);
    mainLayout->addLayout(buttonLayout);

    connect(resetButton_, SIGNAL(clicked()), this, SLOT(resetSettings()));
    connect(closeButton_, SIGNAL(clicked()), this, SIGNAL(closeSettings()));
}

void VoreenSettingsDialog::resetSettings() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QApplication::tr("Reset Settings"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(QApplication::tr("This will reset all application and module settings to default values."));
    msgBox.setInformativeText(QApplication::tr("Do you want to proceed?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok) {
        VoreenApplication::app()->resetAllProperties();
        const std::vector<VoreenModule*>& modules = VoreenApplication::app()->getModules();
        for(size_t i=0; i<modules.size(); i++)
            modules[i]->resetAllProperties();
    }
}

} // namespace
