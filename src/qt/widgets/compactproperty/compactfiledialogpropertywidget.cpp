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

#include "voreen/qt/widgets/compactproperty/compactfiledialogpropertywidget.h"

#include "voreen/core/vis/properties/filedialogproperty.h"

#include <QFileDialog>
#include <QPushButton>

namespace voreen {

CompactFileDialogPropertyWidget::CompactFileDialogPropertyWidget(FileDialogProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , openFileDialogBtn_(new QPushButton)
{
    updateButtonText(prop->get());

    connect(openFileDialogBtn_, SIGNAL(clicked(void)), this, SLOT(setProperty(void)));

    addWidget(openFileDialogBtn_);

    addVisibilityControls();
}

void CompactFileDialogPropertyWidget::setProperty() {
    if (!disconnected_) {
        const std::string& dialogCaption = property_->getDialogCaption();
        const std::string& directory = property_->getDirectory();
        const std::string& fileFilter = property_->getFileFilter();

        std::string filename = QFileDialog::getOpenFileName(QWidget::parentWidget(), dialogCaption.c_str(),
            directory.c_str(), fileFilter.c_str()).toStdString();
        property_->set(filename);
        updateButtonText(filename);
        emit propertyChanged();
    }
}

void CompactFileDialogPropertyWidget::updateButtonText(const std::string& filename) {
    if (!filename.empty()) {
        size_t index = filename.find_last_of('/');
        std::string endFilename = filename.substr(index + 1, filename.length());
        openFileDialogBtn_->setText(endFilename.c_str());
    } else
        openFileDialogBtn_->setText(tr("open file"));

    openFileDialogBtn_->update();
}

} // namespace
