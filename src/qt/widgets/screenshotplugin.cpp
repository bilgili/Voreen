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

#include "voreen/qt/widgets/screenshotplugin.h"

#include "voreen/core/voreenapplication.h"
#include "modules/core/processors/output/canvasrenderer.h" //< core module is always available

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
#include <QDesktopServices>

namespace voreen {

ScreenshotPlugin::ScreenshotPlugin(QWidget* parent, CanvasRenderer* canvasRenderer)
    : QWidget(parent),
      canvasRenderer_(canvasRenderer)
{
    if (canvasRenderer_)
        setWindowTitle(tr("Screenshot: ") + QString::fromStdString(canvasRenderer_->getGuiName()));

    setObjectName(tr("Screenshot"));

    resolutions_.push_back("512x512");
    resolutions_.push_back("800x600");
    resolutions_.push_back("1024x768");
    resolutions_.push_back("1024x1024");
    resolutions_.push_back("1280x1024");
    resolutions_.push_back("1600x1200");
    resolutions_.push_back("1920x1080");
    resolutions_.push_back("1920x1200");
    resolutions_.push_back("2048x2048");
    if (canvasRenderer_)
        resolutions_.push_back("canvas size");
    resolutions_.push_back("user-defined");

    setWindowFlags(Qt::Tool);


    QVBoxLayout* vboxLayout = new QVBoxLayout();
    QGridLayout* gridLayout = new QGridLayout();

    gridLayout->addWidget(new QLabel(tr("Preset:")), 0, 0);
    gridLayout->addWidget(new QLabel(tr("Resolution:")), 1, 0);

    sizeCombo_ = new QComboBox();
    QStringList resolution;
    for (size_t i = 0; i < resolutions_.size(); ++i)
        resolution << resolutions_[i];

    sizeCombo_->addItems(resolution);
    if(canvasRenderer_)
        sizeCombo_->setCurrentIndex(static_cast<int>(resolutions_.size()) - 2); // default to canvas size
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

    buMakeScreenshot_ = new QToolButton();
    buMakeScreenshot_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buMakeScreenshot_->setText(tr(" Save snapshot as..."));
    buMakeScreenshot_->setIcon(QIcon(":/qt/icons/saveas.png"));
    buMakeScreenshot_->setToolTip(tr("Save snapshot as..."));
    buMakeScreenshot_->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred));
    vboxLayout->addWidget(buMakeScreenshot_);

    vboxLayout->addStretch();

    setLayout(vboxLayout);

    // initialize widget state
    sizeComboChanged(sizeCombo_->currentIndex());


    connect(buMakeScreenshot_, SIGNAL(clicked()), this, SLOT(takeScreenshot()));
    connect(spWidth_, SIGNAL(valueChanged(int)), this, SLOT(widthSpinChanged(int)));
    connect(spHeight_, SIGNAL(valueChanged(int)), this, SLOT(heightSpinChanged(int)));
    connect(sizeCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeComboChanged(int)));
}

ScreenshotPlugin::~ScreenshotPlugin() {
}

void ScreenshotPlugin::widthSpinChanged(int /*value*/) {
    // currently nothing todo
}

void ScreenshotPlugin::heightSpinChanged(int /*value*/) {
    // currently nothing todo
}

void ScreenshotPlugin::sizeComboChanged(int index) {
    // if user-defined
    if (index == sizeCombo_->count() - 1) {
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
    if (canvasRenderer_ && (index == sizeCombo_->count() - 2)) {
        tgt::ivec2 size = canvasRenderer_->getCanvas()->getSize();
        spWidth_->setValue(size.x);
        spHeight_->setValue(size.y);
    }
    else {
        // create int from string and set Spinboxes to that values
        if(!sizeCombo_->currentText().contains("native")) {
            QString curText = sizeCombo_->currentText();
            int xIndex = curText.indexOf("x");
            int width = curText.left(xIndex).toInt();
            int height = curText.right(curText.size()-xIndex-1).toInt();
            spWidth_->setValue(width);
            spHeight_->setValue(height);
        }
    }

    spWidth_->blockSignals(false);
    spHeight_->blockSignals(false);
}

void ScreenshotPlugin::takeScreenshot() {
    QFileDialog filedialog(this);
    filedialog.setWindowTitle(tr("Save Screenshot"));
    if(path_.isEmpty())
        filedialog.setDirectory(VoreenApplication::app()->getUserDataPath("screenshots").c_str());
    else
        filedialog.setDirectory(path_);
    filedialog.setDefaultSuffix(tr("png"));

    QStringList filter;
    filter << tr("PNG image (*.png)");
    filter << tr("JPEG image (*.jpg)");
    filter << tr("Windows Bitmap (*.bmp)");
    filter << tr("TIFF image (*.tif)");
    filedialog.setFilters(filter);
    filedialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath("screenshots").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    filedialog.setSidebarUrls(urls);

    struct tm* Tm;
    time_t currentTime = time(NULL);
    Tm = localtime(&currentTime);
    std::stringstream timestamp;
    timestamp << "snapshot " << (Tm->tm_year+1900) << "-" << (Tm->tm_mon+1) << "-" << Tm->tm_mday << "-" << Tm->tm_hour << "-" << Tm->tm_min << "-" << Tm->tm_sec;
    timestamp << ".png";
    filedialog.selectFile(tr(timestamp.str().c_str()));

    QStringList fileList;
    if (filedialog.exec())
        fileList = filedialog.selectedFiles();
    if (fileList.empty())
        return;

    QString file = fileList.at(0);
    path_ = filedialog.directory().absolutePath();

    if (!file.endsWith(".jpg", Qt::CaseInsensitive) && !file.endsWith(".png", Qt::CaseInsensitive) &&
        !file.endsWith(".tif", Qt::CaseInsensitive) && !file.endsWith(".bmp", Qt::CaseInsensitive)) {
        QString text = tr("Image file could not be saved.\n");
        int index = file.lastIndexOf(".");
        if ((index == -1) || (index+1 == fileList[0].size()))
            text += tr("No file extension specified.");
        else
            text += tr("Invalid file extension: ") + file.right(file.size() - index - 1);

        QMessageBox::critical(this, tr("Error saving snapshot"), text);
        return;
    }

    saveScreenshot(file, spWidth_->value(), spHeight_->value());
}

void ScreenshotPlugin::saveScreenshot(const QString& filename, int width, int height) {
    if (!canvasRenderer_)
        return;

    tgt::ivec2 size(width, height);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qApp->processEvents();
    try {
        bool success = canvasRenderer_->renderToImage(filename.toStdString(), size);
        QApplication::restoreOverrideCursor();
        if (!success) {
            QMessageBox::warning(this, tr("Error saving snapshot"),
                                 tr("Screenshot could not be saved:\n%1").arg(
                                     QString::fromStdString(canvasRenderer_->getRenderToImageError())));
        }
    }
    catch (const std::exception& e) {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("Error saving snapshot"),
                             tr("Screenshot could not be saved:\n%1").arg(e.what()));
    }
}

void ScreenshotPlugin::updateFromProcessor() {
    if (!canvasRenderer_)
        return;

    // if canvas-size
    if (sizeCombo_->currentIndex() == sizeCombo_->count()-2) {
        tgt::ivec2 size = canvasRenderer_->getCanvas()->getSize();
        spWidth_->setValue(size.x);
        spHeight_->setValue(size.y);
    }

}

} // namespace voreen
