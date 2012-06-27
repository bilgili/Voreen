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

#include <qslider.h>
#include <qlcdnumber.h>
#include <QMessageBox>
#include <QFileDialog>

#include "voreen/qt/widgets/clippingplugin.h"

namespace voreen {

ClippingPlugin::ClippingPlugin(QWidget* parent, MessageReceiver* msgReceiver)
: WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Clipping Planes"));
    icon_ = QIcon(":/icons/clipping.png");
}

void ClippingPlugin::createWidgets() {
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    clipXWidget_ = new ClipperWidget("X");
    vboxLayout->addWidget(clipXWidget_);
    clipYWidget_ = new ClipperWidget("Y");
    vboxLayout->addWidget(clipYWidget_);
    clipZWidget_ = new ClipperWidget("Z");
    vboxLayout->addWidget(clipZWidget_);

    cropButton_ = new QPushButton(tr("Save cropped dataset"), this);
    vboxLayout->addWidget(cropButton_);

    vboxLayout->addStretch();
    setLayout(vboxLayout);
}

void ClippingPlugin::createConnections() {
    connect(clipXWidget_, SIGNAL(sliderValueChanged()), this, SLOT(updateXClippingPlanes()));
    connect(clipYWidget_, SIGNAL(sliderValueChanged()), this, SLOT(updateYClippingPlanes()));
    connect(clipZWidget_, SIGNAL(sliderValueChanged()), this, SLOT(updateZClippingPlanes()));
    connect(clipXWidget_, SIGNAL(sliderPressed(bool)), this, SLOT(sliderPressedChanged(bool)));
    connect(clipYWidget_, SIGNAL(sliderPressed(bool)), this, SLOT(sliderPressedChanged(bool)));
    connect(clipZWidget_, SIGNAL(sliderPressed(bool)), this, SLOT(sliderPressedChanged(bool)));

    connect(cropButton_, SIGNAL(clicked()), this, SLOT(cropDataset()));
}


void ClippingPlugin::updateXClippingPlanes() {
    postMessage(new IntMsg(ProxyGeometry::setLeftClipPlane_, clipXWidget_->get1stSliderValue()));
    postMessage(new IntMsg(ProxyGeometry::setRightClipPlane_, clipXWidget_->get2ndSliderValue()));
    repaintCanvas();
}

void ClippingPlugin::updateYClippingPlanes() {
    postMessage(new IntMsg(ProxyGeometry::setTopClipPlane_, clipYWidget_->get1stSliderValue()));
    postMessage(new IntMsg(ProxyGeometry::setBottomClipPlane_, clipYWidget_->get2ndSliderValue()));
    repaintCanvas();
}

void ClippingPlugin::updateZClippingPlanes() {
    postMessage(new IntMsg(ProxyGeometry::setFrontClipPlane_, clipZWidget_->get1stSliderValue()));
    postMessage(new IntMsg(ProxyGeometry::setBackClipPlane_, clipZWidget_->get2ndSliderValue()));
    repaintCanvas();
}


void ClippingPlugin::sliderPressedChanged(bool pressed) {
    if (pressed)
        startTracking();
    else
        stopTracking();
}

void ClippingPlugin::cropDataset() {
//merge!!
/*
    Volume* volset = canvas_->getRenderer()->getVolumeContainer()->get();
    if (volset) {
        tgt::ivec3 dim = volset->getDimensions();
        tgt::ivec3 start, size;
        start.x = int(clipXWidget_->get1stSliderValue() / 2.0 * dim.x);
        size.x = int(((1.0 - (clipXWidget_->get2ndSliderValue() / 2.0)) * dim.x) - start.x);

        //Y is switched?
        start.y = int(clipYWidget_->get2ndSliderValue() / 2.0 * dim.y);
        size.y = int(((1.0 - (clipYWidget_->get1stSliderValue() / 2.0)) * dim.y) - start.y);

        start.z = int(clipZWidget_->get1stSliderValue() / 2.0 * dim.z);
        size.z = int(((1.0 - (clipZWidget_->get2ndSliderValue() / 2.0)) * dim.z) - start.z);

        std::cout << "Cropping: " << start << size << std::endl;
        Volume* volsetCropped = volset->createSubset(start, size);
        QString s = QFileDialog::getSaveFileName(
                        this,
                        "Choose a filename to save under",
                        "../../data/",
                        "");
        if (!volsetCropped->saveToDisc(s.toStdString()))
            QMessageBox::information(this, "Voreen",
                                         "Error saving file!\n");
        delete volsetCropped;
    }
    else {
        QMessageBox::information(this, "Voreen",
                                        "No Datasets available.\n");
    }
*/
}

} // namespace voreen
