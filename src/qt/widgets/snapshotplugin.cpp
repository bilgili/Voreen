/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include <QApplication>
#include <QMessageBox>
#include <QGroupBox>

namespace voreen {

SnapshotPlugin::SnapshotPlugin(QWidget* parent, VoreenPainter* painter)
  : WidgetPlugin(parent, painter)
  , path_("")
  , painter_(painter)
{

    setObjectName(tr("Snapshot"));
    icon_ = QIcon(":/icons/snapshot.png");
	canvas_ = 0;
}

void SnapshotPlugin::createWidgets() {
    resize(300,300);

    QVBoxLayout* gridLayout = new QVBoxLayout();

    QHBoxLayout* hboxLayout = new QHBoxLayout();


    //FIXME: no antialiasing at the moment (jms)
    /*gridLayout->addWidget(*/laAntialiasing_ = new QLabel(tr("Antialiasing: 4 x 4"));//);

    /*gridLayout->addWidget(*/slAntialiasing_ = new QSlider(Qt::Horizontal, 0);//);

    slAntialiasing_->setRange(0, 4);
    slAntialiasing_->setSliderPosition(2);
    slAntialiasing_->setTickPosition(QSlider::TicksBelow);
    slAntialiasing_->setToolTip(tr("Adjust supersampling rate"));


    hboxLayout->addWidget(new QLabel(tr("Dimensions:"), 0, 0));

    hboxLayout->addStretch();

    hboxLayout->addWidget(spWidth_ = new QSpinBox(0));
    hboxLayout->addWidget(new QLabel(tr(" x "), 0, 0));
    hboxLayout->addWidget(spHeight_ = new QSpinBox(0));
    hboxLayout->addWidget(new QLabel("  "));
    hboxLayout->addWidget(buMakeSnapshot_ = new QToolButton(0));

    buMakeSnapshot_->setIcon(QIcon(":/icons/floppy.png"));
    buMakeSnapshot_->setToolTip(tr("Save snapshot as..."));

    spWidth_->setRange(32, 2048);
    spHeight_->setRange(32, 2048);
    spWidth_->setSingleStep(32);
    spHeight_->setSingleStep(32);
    spWidth_->setValue(800);
    spHeight_->setValue(600);

    QGroupBox* groupBox = new QGroupBox(tr("Snapshot"));
    groupBox->setLayout(hboxLayout);

    gridLayout->addWidget(groupBox);
    gridLayout->addStretch();

    setLayout(gridLayout);
}

void SnapshotPlugin::createConnections() {
    connect(buMakeSnapshot_, SIGNAL(clicked()), this, SLOT(makeSnapshot()));
    connect(slAntialiasing_, SIGNAL(valueChanged(int)), this, SLOT(updateAntialiasingLabel()));
}

void SnapshotPlugin::makeSnapshot() {
    QFileDialog* filedialog = new QFileDialog(this);
    filedialog->setDefaultSuffix(tr("jpg"));
    filedialog->setWindowTitle(tr("Save Snapshot"));
    filedialog->setDirectory(tr("."));
    QStringList filter;
    filter << tr("Images (*.jpg *.png)");
    filedialog->setFilters(filter);
    filedialog->setAcceptMode(QFileDialog::AcceptSave);
    if (path_ != QString(""))
        filedialog->setDirectory(path_);
    QStringList fileList;
    if (filedialog->exec()) {
        fileList = filedialog->selectedFiles();
    }
    if (fileList.empty())
        return;

    path_ = filedialog->directory().absolutePath();

    if ( !fileList.at(0).endsWith(".jpg") && !fileList.at(0).endsWith(".png") ) {
        QMessageBox* messageBox = new QMessageBox(this);
        messageBox->setWindowTitle(tr("Error saving snapshot"));
        messageBox->setText(tr("Invalid file extension: ") + QString(fileList.at(0)));
        messageBox->show();
        return;
    }

    int screenshotWidth = spWidth_->value();
    int screenshotHeight = spHeight_->value();

//merge!!
//     slAntialiasing_->value());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    painter_->renderToSnapshot(tgt::ivec2(screenshotWidth, screenshotHeight), fileList.at(0).toStdString());
    QApplication::restoreOverrideCursor();
}

/*
  bool SnapshotPlugin::snapshotProgress(float _progress) {
  SNAPSHOT_PLUGIN_DL_PROGESS->setValue((int)(_progress * 100.0));
  return true;
  }
*/

void SnapshotPlugin::updateAntialiasingLabel() {
    switch (slAntialiasing_->value()) {
    case 0:
        laAntialiasing_->setText(tr("Antialiasing: none"));
        break;
    case 1:
        laAntialiasing_->setText(tr("Antialiasing: 2 x 2"));
        break;
    case 2:
        laAntialiasing_->setText(tr("Antialiasing: 4 x 4"));
        break;
    case 3:
        laAntialiasing_->setText(tr("Antialiasing: 8 x 8"));
        break;
    case 4:
        laAntialiasing_->setText(tr("Antialiasing: 16 x 16"));
        break;
    }

}

// bucky icon: (HLw,[fg1,lpg1,[ny,0,0,WCy)
} // namespace voreen

