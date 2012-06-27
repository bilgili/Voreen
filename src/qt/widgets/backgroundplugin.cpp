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

#include "voreen/qt/widgets/backgroundplugin.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <QFrame>
#include <QGroupBox>
#include <QComboBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QColorDialog>
#include <QPushButton>
#include <QColor>
#include <QMessageBox>

#include "voreen/core/vis/processors/image/background.h"

using tgt::Color;

namespace voreen {

BackgroundPlugin::BackgroundPlugin(QWidget* parent, MessageReceiver* msgReceiver)
  : WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Background"));
    icon_ = QIcon(":/icons/background.png");

    activeLayouts_ = ALL_LAYOUTS;

    initParameters();

    // this indicates, that the widget has not yet been initialized
    groupBox_ = 0;

}

BackgroundPlugin::~BackgroundPlugin()
{}

void BackgroundPlugin::setIsSliceRenderer(bool isSlice) {
    // renderer type change not allowed after initialization
    if (groupBox_ != 0)
        return;

    isSliceRenderer_ = isSlice;

    initGL();
}

void BackgroundPlugin::enableBackgroundLayouts(int bitString) {
    activeLayouts_ |= bitString;
}

void BackgroundPlugin::disableBackgroundLayouts(int bitString) {
    activeLayouts_ &= ~bitString;
}

void BackgroundPlugin::createWidgets() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    QVBoxLayout *subLayout;
    QHBoxLayout *hboxLayout;

    groupBox_ = new QGroupBox(tr("Background"));

    if (isSliceRenderer_) {
        QGridLayout *gridLayout = new QGridLayout();

        subLayout = new QVBoxLayout();
        gridLayout = new QGridLayout();
        gridLayout->setColumnStretch(0,2);
        gridLayout->setColumnStretch(1,1);

        buttonBackgroundColor_ = new QPushButton();
        buttonBackgroundColor_->setText(tr("Color ..."));
        buttonBackgroundColor_->setMinimumWidth(100);
        hboxLayout = new QHBoxLayout();
        hboxLayout->addWidget(buttonBackgroundColor_);
        hboxLayout->addStretch();
        gridLayout->addLayout(hboxLayout, 0, 0);

        colorLabelBackground_ = new QLabel("");
        colorLabelBackground_->setAutoFillBackground(true);
        QPalette palette = colorLabelBackground_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundColor_.r*255),
            static_cast<int>(backgroundColor_.g*255),
            static_cast<int>(backgroundColor_.b*255),
            255                                   )
            );
        colorLabelBackground_->setPalette(palette);
        colorLabelBackground_->setBackgroundRole(palette.Window);
        colorLabelBackground_->setMinimumWidth(60);
        gridLayout->addWidget(colorLabelBackground_, 0, 1);

        subLayout->addLayout(gridLayout);
        groupBox_->setLayout(subLayout);
    }
    else {
        // Background for raycasting (with Background)
        QGridLayout *gridLayout = new QGridLayout();

        gridLayout->setColumnStretch(0,2);
        gridLayout->setColumnStretch(1,1);

        // Background layout
        layoutLabel_ = new QLabel(tr(" Background layout:"));
        gridLayout->addWidget(layoutLabel_, 0, 0);

        backgroundLayoutCombo_ = new QComboBox();
        int comboID = 1;
        if (activeLayouts_ & MONOCHROME) {
            backgroundLayoutCombo_->addItem(tr("Plain"));
            if (backgroundLayout_ == MONOCHROME)
                comboID = backgroundLayoutCombo_->count();
        }
        if (activeLayouts_ & GRADIENT) {
            backgroundLayoutCombo_->addItem(tr("Gradient"));
            if (backgroundLayout_ == GRADIENT)
                comboID = backgroundLayoutCombo_->count();
        }
        if (activeLayouts_ & RADIAL) {
            backgroundLayoutCombo_->addItem(tr("Radial"));
            if (backgroundLayout_ == RADIAL)
                comboID = backgroundLayoutCombo_->count();
        }
        if (activeLayouts_ & CLOUD) {
            backgroundLayoutCombo_->addItem(tr("Cloud"));
            if (backgroundLayout_ == CLOUD)
                comboID = backgroundLayoutCombo_->count();
        }
        if (activeLayouts_ & TEXTURE) {
            backgroundLayoutCombo_->addItem(tr("Texture"));
            if (backgroundLayout_ == TEXTURE)
                comboID = backgroundLayoutCombo_->count();
        }
        backgroundLayoutCombo_->setCurrentIndex(comboID-1);
        gridLayout->addWidget(backgroundLayoutCombo_, 0, 1);

        // Separator
        QFrame* separator = new QFrame();
        separator->setFrameStyle(QFrame::HLine);
        gridLayout->addWidget(separator, 1, 0, 1, 0);

        // Background single color
        buttonBackgroundColor_ = new QPushButton();
        buttonBackgroundColor_->setText(tr("Color ..."));
        buttonBackgroundColor_->setMinimumWidth(100);
        hboxLayout = new QHBoxLayout();
        hboxLayout->addWidget(buttonBackgroundColor_);
        hboxLayout->addStretch();
        gridLayout->addLayout(hboxLayout, 2, 0);

        colorLabelBackground_ = new QLabel("");
        colorLabelBackground_->setAutoFillBackground(true);
        QPalette palette = colorLabelBackground_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundColor_.r*255),
            static_cast<int>(backgroundColor_.g*255),
            static_cast<int>(backgroundColor_.b*255),
            255                                   )
            );
        colorLabelBackground_->setPalette(palette);
        colorLabelBackground_->setBackgroundRole(palette.Window);
        gridLayout->addWidget(colorLabelBackground_, 2, 1);

        // Background first / second color
        buttonBackgroundFirstColor_ = new QPushButton(tr("First color ..."));
        colorLabelBackgroundFirst_ = new QLabel("");
        colorLabelBackgroundFirst_->setAutoFillBackground(true);
        hboxLayout = new QHBoxLayout();
        hboxLayout->addWidget(buttonBackgroundFirstColor_);
        hboxLayout->addStretch();
        gridLayout->addLayout(hboxLayout, 3, 0);
        palette = colorLabelBackgroundFirst_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundFirstColor_.r*255),
            static_cast<int>(backgroundFirstColor_.g*255),
            static_cast<int>(backgroundFirstColor_.b*255),
            255                                   )
            );
        colorLabelBackgroundFirst_->setPalette(palette);
        colorLabelBackgroundFirst_->setBackgroundRole(palette.Window);
        gridLayout->addWidget(colorLabelBackgroundFirst_, 3, 1);

        buttonBackgroundSecondColor_ = new QPushButton(tr("Second color ..."));
        colorLabelBackgroundSecond_ = new QLabel("");
        colorLabelBackgroundSecond_->setAutoFillBackground(true);
        hboxLayout = new QHBoxLayout();
        hboxLayout->addWidget(buttonBackgroundSecondColor_);
        hboxLayout->addStretch();
        gridLayout->addLayout(hboxLayout, 4, 0);
        palette = colorLabelBackgroundSecond_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundSecondColor_.r*255),
            static_cast<int>(backgroundSecondColor_.g*255),
            static_cast<int>(backgroundSecondColor_.b*255),
            255                                   )
            );
        colorLabelBackgroundSecond_->setPalette(palette);
        colorLabelBackgroundSecond_->setBackgroundRole(palette.Window);
        gridLayout->addWidget(colorLabelBackgroundSecond_, 4, 1);

        // synchronize sizes of background color buttons
        buttonBackgroundFirstColor_->adjustSize();
        buttonBackgroundSecondColor_->adjustSize();
        int minWidth = buttonBackgroundFirstColor_->width();
        minWidth = std::max(minWidth, buttonBackgroundSecondColor_->width());
        buttonBackgroundFirstColor_->setMinimumWidth(minWidth+10);
        buttonBackgroundSecondColor_->setMinimumWidth(minWidth+10);

        // Gradient angle
        gradientLayoutLabel_ = new QLabel(tr(" Gradient angle:"));
        gridLayout->addWidget(gradientLayoutLabel_, 5, 0);
        backgroundAngleSpin_ = new QSpinBox();
        backgroundAngleSpin_->setRange(0, 360);
        backgroundAngleSpin_->setSingleStep(10);
        backgroundAngleSpin_->setValue(backgroundAngle_);
        gridLayout->addWidget(backgroundAngleSpin_, 5, 1);
        groupBox_->setLayout(gridLayout);

        // Background tile
        tileLabel_ = new QLabel(tr(" Background tiling:"));
        gridLayout->addWidget(tileLabel_, 6, 0);
        spinBoxTile_ = new QSpinBox();
        spinBoxTile_->setRange(1,100);
        spinBoxTile_->setSingleStep(1);
        spinBoxTile_->setValue(backgroundTile_);
        gridLayout->addWidget(spinBoxTile_, 6, 1);
    }

    mainLayout->addWidget(groupBox_);
    mainLayout->addStretch();

    setLayout(mainLayout);

    setWidgetState();
}

void BackgroundPlugin::createConnections() {
    connect(buttonBackgroundColor_, SIGNAL(clicked()), this, SLOT(selectBackgroundColor()));

    if (!isSliceRenderer_) {
        connect(buttonBackgroundFirstColor_, SIGNAL(clicked()), this, SLOT(selectBackgroundFirstColor()));
        connect(buttonBackgroundSecondColor_, SIGNAL(clicked()), this, SLOT(selectBackgroundSecondColor()));
        connect(backgroundLayoutCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(setBackgroundLayout(int)));
        connect(backgroundAngleSpin_, SIGNAL(valueChanged(int)), this, SLOT(setBackgroundAngle(int)));
        connect(spinBoxTile_, SIGNAL(valueChanged(int)), this, SLOT(setBackgroundTile(int)));
    }
}

void BackgroundPlugin::initParameters() {
    isSliceRenderer_ = false;

    backgroundColor_ = Color( 235.f, 235.f, 235.f, 0.f ) / 255.f;
    backgroundLayout_ = GRADIENT;
    backgroundFirstColor_ = Color( 255.f, 255.f, 255.f, 0.f ) / 255.f;
    backgroundSecondColor_ = Color( 204.f, 204.f, 204.f, 0.f ) / 255.f;
    backgroundAngle_ = 180;
    backgroundTile_ = 1;
}

void BackgroundPlugin::setWidgetState() {
    // return, if widget has not yet been initialized
    if (!groupBox_)
        return;

    if (isSliceRenderer_) {
        QPalette palette = colorLabelBackground_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundColor_.r*255),
            static_cast<int>(backgroundColor_.g*255),
            static_cast<int>(backgroundColor_.b*255),
            255                                   )
            );
        colorLabelBackground_->setPalette(palette);
    }
    else {
        // assuming: Raycasting with Background
        layoutLabel_->setVisible(true);
        backgroundLayoutCombo_->setVisible(true);

        gradientLayoutLabel_->setVisible(false);
        buttonBackgroundColor_->setVisible(false);
        buttonBackgroundFirstColor_->setVisible(false);
        buttonBackgroundSecondColor_->setVisible(false);
        backgroundAngleSpin_->setVisible(false);
        colorLabelBackground_->setVisible(false);
        colorLabelBackgroundFirst_->setVisible(false);
        colorLabelBackgroundSecond_->setVisible(false);
        tileLabel_->setVisible(false);
        spinBoxTile_->setVisible(false);

        if (backgroundLayout_ == MONOCHROME) {
            buttonBackgroundColor_->setVisible(true);
            colorLabelBackground_->setVisible(true);
            QPalette palette = colorLabelBackground_->palette();
            palette.setColor(QPalette::Window, QColor(
                static_cast<int>(backgroundColor_.r*255),
                static_cast<int>(backgroundColor_.g*255),
                static_cast<int>(backgroundColor_.b*255),
                255                                   )
                );
            colorLabelBackground_->setPalette(palette);

        }
        else if (backgroundLayout_ == GRADIENT || backgroundLayout_ == RADIAL) {
            buttonBackgroundFirstColor_->setVisible(true);
            colorLabelBackgroundFirst_->setVisible(true);
            buttonBackgroundSecondColor_->setVisible(true);
            colorLabelBackgroundSecond_->setVisible(true);

            QPalette palette = colorLabelBackgroundFirst_->palette();
            palette.setColor(QPalette::Window, QColor(
                static_cast<int>(backgroundFirstColor_.r*255),
                static_cast<int>(backgroundFirstColor_.g*255),
                static_cast<int>(backgroundFirstColor_.b*255),
                255                                   )
            );
            colorLabelBackgroundFirst_->setPalette(palette);

            palette = colorLabelBackgroundSecond_->palette();
            palette.setColor(QPalette::Window, QColor(
                static_cast<int>(backgroundSecondColor_.r*255),
                static_cast<int>(backgroundSecondColor_.g*255),
                static_cast<int>(backgroundSecondColor_.b*255),
                255                                   )
                );
            colorLabelBackgroundSecond_->setPalette(palette);

            if (backgroundLayout_ == GRADIENT) {
                gradientLayoutLabel_->setVisible(true);
                backgroundAngleSpin_->setVisible(true);
                backgroundAngleSpin_->setValue(backgroundAngle_);
            }

            if (backgroundLayout_ == RADIAL) {
                tileLabel_->setVisible(true);
                spinBoxTile_->setVisible(true);
                spinBoxTile_->setValue(backgroundTile_);
            }

        }
        else if (backgroundLayout_ == CLOUD ) {
            tileLabel_->setVisible(true);
            spinBoxTile_->setVisible(true);
            spinBoxTile_->setValue(backgroundTile_);
        }
    }
}

void BackgroundPlugin::initGL() {
    std::string layout;
    if (backgroundLayout_ == MONOCHROME)
        layout = "Monochrome";
    else if (backgroundLayout_ == GRADIENT)
        layout = "Gradient";
    else if (backgroundLayout_ == RADIAL)
        layout = "Radial";
    else if (backgroundLayout_ == CLOUD)
        layout = "Cloud";
    else if (backgroundLayout_ == TEXTURE)
        layout = "Texture";

    postMessage(new ColorMsg(Background::setBackgroundColor_, backgroundColor_));
    postMessage(new ColorMsg(Background::setBackgroundFirstColor_, backgroundFirstColor_));
    postMessage(new ColorMsg(Background::setBackgroundSecondColor_, backgroundSecondColor_));
    postMessage(new StringMsg("set.backgroundModeAsString", layout));
    postMessage(new IntMsg(Background::setBackgroundAngle_, backgroundAngle_));
    postMessage(new FloatMsg("set.backgroundTile", float(backgroundTile_)));
}

void BackgroundPlugin::selectBackgroundColor() {
    QColor color = QColorDialog::getRgba( qRgba(
        static_cast<int>(255*backgroundColor_.r),
        static_cast<int>(255*backgroundColor_.g),
        static_cast<int>(255*backgroundColor_.b),
        255 ) , 0, buttonBackgroundColor_);

    if (color.isValid()) {
        backgroundColor_ = Color(color.redF(), color.greenF(), color.blueF(), 0.f);

        QPalette palette = colorLabelBackground_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundColor_.r*255),
            static_cast<int>(backgroundColor_.g*255),
            static_cast<int>(backgroundColor_.b*255),
            255                                   )
        );
        colorLabelBackground_->setPalette(palette);

        postMessage(new ColorMsg(Background::setBackgroundColor_,
            backgroundColor_));

        repaintCanvases();
    }
}

void BackgroundPlugin::selectBackgroundFirstColor(){
    QColor color = QColorDialog::getRgba( qRgba(
        static_cast<int>(255*backgroundFirstColor_.r),
        static_cast<int>(255*backgroundFirstColor_.g),
        static_cast<int>(255*backgroundFirstColor_.b),
        255 ) , 0, buttonBackgroundFirstColor_);

    if (color.isValid()) {
        backgroundFirstColor_ = Color(color.redF(), color.greenF(), color.blueF(), 0);

        QPalette palette = colorLabelBackgroundFirst_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundFirstColor_.r*255),
            static_cast<int>(backgroundFirstColor_.g*255),
            static_cast<int>(backgroundFirstColor_.b*255),
            255                                   )
        );
        colorLabelBackgroundFirst_->setPalette(palette);

        postMessage(new ColorMsg(Background::setBackgroundFirstColor_,
            backgroundFirstColor_));

        repaintCanvases();
    }
}

void BackgroundPlugin::selectBackgroundSecondColor(){
    QColor color = QColorDialog::getRgba( qRgba(
        static_cast<int>(255*backgroundSecondColor_.r),
        static_cast<int>(255*backgroundSecondColor_.g),
        static_cast<int>(255*backgroundSecondColor_.b),
        255 ) , 0, buttonBackgroundSecondColor_);

    if (color.isValid()) {
        backgroundSecondColor_ = Color(color.redF(), color.greenF(), color.blueF(), 0.f);

        QPalette palette = colorLabelBackgroundSecond_->palette();
        palette.setColor(QPalette::Window, QColor(
            static_cast<int>(backgroundSecondColor_.r*255),
            static_cast<int>(backgroundSecondColor_.g*255),
            static_cast<int>(backgroundSecondColor_.b*255),
            255                                   )
        );
        colorLabelBackgroundSecond_->setPalette(palette);

        postMessage(new ColorMsg(Background::setBackgroundSecondColor_,
            backgroundSecondColor_));

        repaintCanvases();
    }
}

void BackgroundPlugin::setBackgroundLayout(int /*value*/) {
    std::string layout;
    QString text = backgroundLayoutCombo_->currentText();
    if (text == tr("Plain")) {
        layout = "Monochrome";
        backgroundLayout_ = MONOCHROME;
        postMessage(new ColorMsg(Background::setBackgroundColor_,
            backgroundColor_));
    }
    else if (text == tr("Gradient")) {
        layout = "Gradient";
        backgroundLayout_ = GRADIENT;
    }
    else if (text == tr("Radial")) {
        layout = "Radial";
        backgroundLayout_ = RADIAL;
    }
    else if (text == tr("Cloud")) {
        layout = "Cloud";
        backgroundLayout_ = CLOUD;
    }
    else if (text == tr("Texture")) {
        layout = "Texture";
        backgroundLayout_ = TEXTURE;
    }

    setWidgetState();

    postMessage(new StringMsg("set.backgroundModeAsString", layout));
    repaintCanvases();
}

void BackgroundPlugin::setBackgroundAngle(int value) {
    backgroundAngle_ = value;
    postMessage(new IntMsg(Background::setBackgroundAngle_, value));

    repaintCanvases();
}

void BackgroundPlugin::setBackgroundTile(int value) {
    backgroundTile_ = value;
    postMessage(new FloatMsg("set.backgroundTile", float(value)));

    repaintCanvases();
}

} // namespace voreen
