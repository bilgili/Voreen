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

#include "voreen/qt/widgets/property/filedialogpropertywidget.h"

#include "voreen/core/vis/properties/filedialogproperty.h"

#include <QFileDialog>
#include <QPushButton>

namespace voreen {

FileDialogPropertyWidget::FileDialogPropertyWidget(FileDialogProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
    , openFileDialogBtn_(new QPushButton)
{
    updateButtonText(prop->get());

    connect(openFileDialogBtn_, SIGNAL(clicked(void)), this, SLOT(setProperty(void)));

    addWidget(openFileDialogBtn_);

    addVisibilityControls();
}

void FileDialogPropertyWidget::setProperty() {
    if (!disconnected_) {
        const QString dialogCaption = property_->getDialogCaption().c_str();
        const QString directory = property_->getDirectory().c_str();
        const QString fileFilter = QString(property_->getFileFilter().c_str()) + ";;" + tr("All files (*)");

        QString filename;
        if (property_->getFileMode() == FileDialogProperty::OPEN_FILE) {
            filename = QFileDialog::getOpenFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);
        }
        else if (property_->getFileMode() == FileDialogProperty::SAVE_FILE) {
            filename = QFileDialog::getSaveFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);
        }
        else if (property_->getFileMode() == FileDialogProperty::DIRECTORY) {
            filename = QFileDialog::getExistingDirectory(QWidget::parentWidget(), dialogCaption, property_->get().c_str());
        }

        if (!filename.isEmpty()) {
            property_->set(filename.toStdString());
            updateButtonText(filename.toStdString());
            emit modified();
        }
    }
}

void FileDialogPropertyWidget::updateButtonText(const std::string& filename) {
    if (!filename.empty()) {
        if ((property_->getFileMode() == FileDialogProperty::OPEN_FILE) || (property_->getFileMode() == FileDialogProperty::SAVE_FILE)) {
            size_t index = filename.find_last_of('/');
            std::string endFilename = filename.substr(index + 1, filename.length());
            openFileDialogBtn_->setText(endFilename.c_str());
        }
        else if (property_->getFileMode() == FileDialogProperty::DIRECTORY) {
            std::string directory = filename;
            if (directory.length() >= 20)
                directory = "..." + directory.substr(directory.length()-20);
            openFileDialogBtn_->setText(directory.c_str());
        }
    }
    else {
        if (property_->getFileMode() == FileDialogProperty::OPEN_FILE)
             openFileDialogBtn_->setText(tr("open file"));
        else if (property_->getFileMode() == FileDialogProperty::SAVE_FILE)
             openFileDialogBtn_->setText(tr("save file"));
        else if (property_->getFileMode() == FileDialogProperty::DIRECTORY)
            openFileDialogBtn_->setText(tr("select directory"));
    }

    openFileDialogBtn_->update();
}

} // namespace
