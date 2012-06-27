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

#include "voreen/core/application.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"

#include "tgt/gpucapabilities.h"

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace voreen {

SnapshotPlugin::SnapshotPlugin(QWidget* parent, CanvasRenderer* canvasRenderer)
    : WidgetPlugin(parent)
    , canvasRenderer_(canvasRenderer)
{

    tgtAssert(canvasRenderer_, "No canvas renderer");

    setWindowTitle(tr("Snapshot: ") + QString::fromStdString(canvasRenderer_->getName()));
    setObjectName(tr("Snapshot"));

    resolutions_.push_back("512x512");
    resolutions_.push_back("800x600");
    resolutions_.push_back("1024x768");
    resolutions_.push_back("1024x1024");
    resolutions_.push_back("1280x1024");
    resolutions_.push_back("1600x1200");
    resolutions_.push_back("1920x1080");
    resolutions_.push_back("1920x1200");
    resolutions_.push_back("2048x2048");
    resolutions_.push_back("canvas size");
    resolutions_.push_back("user-defined");

    setWindowFlags(Qt::Tool);
}

SnapshotPlugin::~SnapshotPlugin() {
}

void SnapshotPlugin::createWidgets() {

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel(tr("Preset:")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("Resolution:")), 1, 0);

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
    spWidth_->setRange(1, GpuCaps.getMaxTextureSize());
    spHeight_->setRange(1, GpuCaps.getMaxTextureSize());
    spWidth_->setValue(800);
    spHeight_->setValue(600);
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 1);
    gridLayout->setColumnStretch(3, 5);
    vboxLayout->addLayout(gridLayout);

    buMakeSnapshot_ = new QToolButton();
    buMakeSnapshot_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buMakeSnapshot_->setText(tr(" Save snapshot as..."));
    buMakeSnapshot_->setIcon(QIcon(":/icons/saveas.png"));
    buMakeSnapshot_->setToolTip(tr("Save snapshot as..."));
    buMakeSnapshot_->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
    vboxLayout->addWidget(buMakeSnapshot_);

    vboxLayout->addStretch();

    setLayout(vboxLayout);

    // initialize widget state
    sizeComboChanged(sizeCombo_->currentIndex());
}

void SnapshotPlugin::createConnections() {
    connect(buMakeSnapshot_, SIGNAL(clicked()), this, SLOT(takeSnapshot()));
    connect(spWidth_, SIGNAL(valueChanged(int)), this, SLOT(widthSpinChanged(int)));
    connect(spHeight_, SIGNAL(valueChanged(int)), this, SLOT(heightSpinChanged(int)));
    connect(sizeCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged(int)));
}

void SnapshotPlugin::widthSpinChanged(int /*value*/) {
    // currently nothing todo
}

void SnapshotPlugin::heightSpinChanged(int /*value*/) {
    // currently nothing todo
}

void SnapshotPlugin::sizeComboChanged(int index) {

    tgtAssert(canvasRenderer_ && canvasRenderer_->getCanvas(), "No canvas present");

    // if user-defined
    if (index == sizeCombo_->count()-1) {
        spWidth_->setEnabled(true);
        spHeight_->setEnabled(true);
        return;
    }
    else {
        spWidth_->setEnabled(false);
        spHeight_->setEnabled(false);
    }

    spWidth_->blockSignals(true);
    spHeight_->blockSignals(true);

    // if canvas-size
    if (index == sizeCombo_->count()-2) {
        tgt::ivec2 size = canvasRenderer_->getCanvas()->getSize();
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

void SnapshotPlugin::takeSnapshot() {

    QFileDialog filedialog(this);
    filedialog.setDefaultSuffix(tr("png"));
    filedialog.setWindowTitle(tr("Save Snapshot"));

    QStringList filter;
    filter << tr("PNG image (*.png)");
    filter << tr("JPEG image (*.jpg)");
    filedialog.setFilters(filter);
    filedialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getDocumentsPath().c_str());
    filedialog.setSidebarUrls(urls);

    if (path_.isEmpty())
        filedialog.setDirectory(VoreenApplication::app()->getSnapshotPath().c_str());
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
    qApp->processEvents();
    try {
        bool success = canvasRenderer_->renderToImage(fileList.at(0).toStdString(), size);
        QApplication::restoreOverrideCursor();
        if (!success) {
            QString text = tr("Snapshot could not be saved:\n%1").arg(
                QString::fromStdString(canvasRenderer_->getRenderToImageError()));
            QMessageBox::warning(this, tr("Error saving snapshot"), text);
        }
    }
    catch (const std::exception& e) {
        QApplication::restoreOverrideCursor();
        QString text = tr("Snapshot could not be saved:\n%1").arg(e.what());
        QMessageBox::warning(this, tr("Error saving snapshot"), text);
    }

}

void SnapshotPlugin::updateFromProcessor() {

    tgtAssert(canvasRenderer_ && canvasRenderer_->getCanvas(), "No canvas present");

    // if canvas-size
    if (sizeCombo_->currentIndex() == sizeCombo_->count()-2) {
        tgt::ivec2 size = canvasRenderer_->getCanvas()->getSize();
        spWidth_->setValue(size.x);
        spHeight_->setValue(size.y);
    }

}

} // namespace voreen
