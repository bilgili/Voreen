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

#include "voreen/qt/widgets/snapshotplugin.h"

#include "voreen/core/vis/voreenpainter.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

namespace voreen {

SnapshotPlugin::SnapshotPlugin(QWidget* parent, VoreenPainter* painter)
    : WidgetPlugin(parent)
    , painter_(painter)
{
    setObjectName(tr("Snapshot"));

    resolutions_.push_back("512x512");
    resolutions_.push_back("800x600");
    resolutions_.push_back("1024x768");
    resolutions_.push_back("1024x1024");
    resolutions_.push_back("1280x1024");
    resolutions_.push_back("1600x1200");
    resolutions_.push_back("canvas size");
    resolutions_.push_back("user-defined");
}

SnapshotPlugin::~SnapshotPlugin() {
}

void SnapshotPlugin::createWidgets() {
    resize(300, 300);

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel(tr("Resolution:")), 0, 0);

    sizeCombo_ = new QComboBox();
    QStringList resolution;
    for (size_t i = 0; i < resolutions_.size(); ++i)
        resolution << resolutions_[i];

    sizeCombo_->addItems(resolution);
    gridLayout->addWidget(sizeCombo_, 0, 1, 1, 3);
    
    spWidth_ = new QSpinBox();
    gridLayout->addWidget(spWidth_, 1, 1);

    gridLayout->addWidget(new QLabel("x"), 1, 2, Qt::AlignCenter);
    spHeight_ = new QSpinBox();
    gridLayout->addWidget(spHeight_, 1, 3);
    spWidth_->setRange(1, 2048);
    spHeight_->setRange(1, 2048);
    spWidth_->setValue(800);
    spHeight_->setValue(600);
    widthSpinChanged(spWidth_->value()); // to select the correct list entry
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 1);
    gridLayout->setColumnStretch(3, 5);
    vboxLayout->addLayout(gridLayout);

    buMakeSnapshot_ = new QToolButton();
    buMakeSnapshot_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buMakeSnapshot_->setText(tr("Save snapshot as..."));
    buMakeSnapshot_->setIcon(QIcon(":/icons/floppy.png"));
    buMakeSnapshot_->setToolTip(tr("Save snapshot as..."));
    vboxLayout->addWidget(buMakeSnapshot_);

    vboxLayout->addStretch();
    
    setLayout(vboxLayout);
}

void SnapshotPlugin::createConnections() {
    connect(buMakeSnapshot_, SIGNAL(clicked()), this, SLOT(makeSnapshot()));
    connect(spWidth_, SIGNAL(valueChanged(int)), this, SLOT(widthSpinChanged(int)));
    connect(spHeight_, SIGNAL(valueChanged(int)), this, SLOT(heightSpinChanged(int)));
    connect(sizeCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged(int)));
}

void SnapshotPlugin::widthSpinChanged(int value) {
    // construct string and compare with the entries in the combobox
    QString s = QString("%1x%2").arg(value).arg(spHeight_->value());
    int index = -1;
    for (size_t i = 0; i < resolutions_.size(); ++i) {
        if (resolutions_[i] == s) {
            index = i;
            break;
        }
    }

    // set combobox to found resolution, "canvas size" or "user-defined"
    sizeCombo_->blockSignals(true);
    if (index != -1)
        sizeCombo_->setCurrentIndex(index);
    else {
        if (painter_ && painter_->getCanvas() && 
            painter_->getCanvas()->getSize() == tgt::ivec2(value, spHeight_->value()))
        {
            sizeCombo_->setCurrentIndex(sizeCombo_->count()-2);
        }
        else
            sizeCombo_->setCurrentIndex(sizeCombo_->count()-1);
    }
    sizeCombo_->blockSignals(false);
}

void SnapshotPlugin::heightSpinChanged(int value) {
    // construct string and compare with the entries in the combobox
    QString s = QString("%1x%2").arg(spWidth_->value()).arg(value);
    int index = -1;
    for (size_t i = 0; i < resolutions_.size(); ++i) {
        if (resolutions_[i] == s) {
            index = i;
            break;
        }
    }

    // set combobox to found resolution or "user-defined"
    sizeCombo_->blockSignals(true);
    if (index != -1)
        sizeCombo_->setCurrentIndex(index);
    else {
        if (painter_ && painter_->getCanvas() && 
            painter_->getCanvas()->getSize() == tgt::ivec2(spWidth_->value(), value))
        {
            sizeCombo_->setCurrentIndex(sizeCombo_->count()-2);
        }
        else
            sizeCombo_->setCurrentIndex(sizeCombo_->count()-1);
    }
    sizeCombo_->blockSignals(false);
}

void SnapshotPlugin::sizeComboChanged(int index) {
    if (index == sizeCombo_->count()-1)
        return;

    spWidth_->blockSignals(true);
    spHeight_->blockSignals(true);
    if (index == sizeCombo_->count()-2) {
        tgt::ivec2 size = painter_->getCanvas()->getSize();
        spWidth_->setValue(size.x);
        spHeight_->setValue(size.y);
    }
    else {
        // create int from string and set Spinboxes to that values
        QString curText = sizeCombo_->currentText();
        int xIndex = curText.indexOf("x");
        int width = curText.left(xIndex).toInt();
        int height = curText.right(curText.size()-xIndex-1).toInt();
        spWidth_->setValue(width);
        spHeight_->setValue(height);
    }

    spWidth_->blockSignals(false);
    spHeight_->blockSignals(false);
}

void SnapshotPlugin::makeSnapshot() {
    QFileDialog filedialog(this);
    filedialog.setDefaultSuffix(tr("png"));
    filedialog.setWindowTitle(tr("Save Snapshot"));

    QStringList filter;
    filter << tr("PNG image (*.png)");
    filter << tr("JPEG image (*.jpg)");
    filedialog.setFilters(filter);
    filedialog.setAcceptMode(QFileDialog::AcceptSave);

    if (path_.isEmpty())
        filedialog.setDirectory(QDir::homePath());       
    else
        filedialog.setDirectory(path_);
    filedialog.selectFile(tr("snapshot.png"));

    QStringList fileList;
    if (filedialog.exec())
        fileList = filedialog.selectedFiles();
    if (fileList.empty())
        return;

    path_ = filedialog.directory().absolutePath();

    if (!fileList.at(0).endsWith(".jpg") && !fileList.at(0).endsWith(".png")) {
        std::string text = "Snapshot could not be saved.\n";
        int index = fileList[0].lastIndexOf(".");
        if ((index == -1) || (index+1 == fileList[0].size()))
            text += "No file extension specified.";
        else
            text += "Invalid file extension: " + fileList[0].right(fileList[0].size()-index-1).toStdString();
        
        QMessageBox::critical(this, tr("Error saving snapshot"), tr(text.c_str()));
        return;
    }

    tgt::ivec2 size(spWidth_->value(), spHeight_->value());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    try {
        if (painter_)
            painter_->renderToSnapshot(size, fileList.at(0).toStdString());
        QApplication::restoreOverrideCursor();    
    }
    catch (const std::bad_alloc&) {
        QApplication::restoreOverrideCursor();
        QString text = tr("Snapshot could not be saved.\nNot enough system memory.");
        QMessageBox::critical(this, tr("Error saving snapshot"), text);
    }
    catch (const tgt::FileException& e) {
        QApplication::restoreOverrideCursor();
        QString text = tr("Snapshot could not be saved.\n%1").arg(e.what());
        QMessageBox::critical(this, tr("Error saving snapshot"), text);
    }
}

} // namespace voreen
