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

#include "voreen/qt/opennetworkfiledialog.h"
#include <QGridLayout>

namespace voreen {

OpenNetworkFileDialog::OpenNetworkFileDialog(QWidget* parent, Qt::WindowFlags flags)
        : QFileDialog(parent, flags)
{
    initialize();
}

OpenNetworkFileDialog::OpenNetworkFileDialog(QWidget* parent, const QString& caption, const QString& directory, const QString& filter)
        : QFileDialog(parent, caption, directory, filter)
{
    initialize();
}

OpenNetworkFileDialog::~OpenNetworkFileDialog() {
    //delete loadVolumeSetContainer_; will be deleted by parent
}

void OpenNetworkFileDialog::setLoadVolumeSetContainer(bool b) {
    loadVolumeSetContainer_->setChecked(b);
}

bool OpenNetworkFileDialog::loadVolumeSetContainer() {
    return loadVolumeSetContainer_->isChecked();
}

void OpenNetworkFileDialog::initialize() {
    // Add checkbox for loading with or without volumesetcontainer
    loadVolumeSetContainer_ = new QCheckBox("&Load datasets with network", this);
    QGridLayout *layout = (QGridLayout*)this->layout();
    layout->addWidget(loadVolumeSetContainer_, layout->rowCount(), 0);
    loadVolumeSetContainer_->show();
    // Filters
    QStringList filters;
    filters << tr("Voreen network files (*.vnw)")
            << tr("All files (*.*)");
    this->setFilters(filters);
}

} // namespace
