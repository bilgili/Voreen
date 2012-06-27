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

#include "voreen/qt/widgets/compactproperty/compactstringvectorpropertywidget.h"

#include "voreen/core/vis/properties/stringproperty.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QPushButton>

namespace voreen {

CompactStringVectorPropertyWidget::CompactStringVectorPropertyWidget(StringVectorProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , property_(prop)
    , buttonBox_(new QDialogButtonBox(Qt::Horizontal,0))
    , grid_(new QGridLayout)
    , sliceButton_(new QPushButton(tr("Select")))
{
    buttonBox_->setLayout(grid_);
    buttonBox_->addButton(sliceButton_, QDialogButtonBox::ActionRole);

    connect(sliceButton_, SIGNAL(clicked(void)), sliceButton_, SLOT(setProperty(void)));
    addWidget(buttonBox_);

    addVisibilityControls();
}

void CompactStringVectorPropertyWidget::setProperty() {
    if (!disconnected_) {
        QFileDialog fileDialog(this,
                               tr("Dataset filename"),
                               QDir::currentPath(),
                               "Any file (*.*)");
        QStringList filters;
        filters << "Raw file (*.raw) "<< "Frozen CT (*.fro)" << "CT (*.fre)" << "MRI pd (*.pd)"
                << "MRI t1 (*.t1)"<<"MRI t2 (*.t2)" << "Any file (*.*)";
        fileDialog.setFilters(filters);
        fileDialog.setFileMode(QFileDialog::ExistingFiles);
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (fileDialog.exec()) {
            std::vector<std::string> files;
            for (int i=0; i<fileDialog.selectedFiles().size(); i++) {
                files.push_back(fileDialog.selectedFiles().at(i).toStdString());
            }
            property_->set(files);
            emit propertyChanged();
        }
    }
}

} // namespace
