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

#include "voreen/qt/widgets/property/fontpropertywidget.h"

#include "voreen/core/voreenapplication.h"

#include "voreen/core/properties/fontproperty.h"

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QTabWidget>
#include <QWidget>
#include <QLCDNumber>
#include <QColorGroup>
#include <QLabel>

namespace voreen {

FontPropertyWidget::FontPropertyWidget(FontProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
{
    groupBox_ = new QGroupBox(this);
    addWidget(groupBox_);

    QVBoxLayout* tgtFontLayout = new QVBoxLayout(groupBox_);
    tgtFontLayout->setContentsMargins(2,2,2,2);

    tgtFontType_ = new QComboBox();
    tgtFontType_->addItem("Bitmap", (int)tgt::Font::BitmapFont);
    tgtFontType_->addItem("Buffer", (int)tgt::Font::BufferFont);
    tgtFontType_->addItem("Extrude", (int)tgt::Font::ExtrudeFont);
    tgtFontType_->addItem("Outline", (int)tgt::Font::OutlineFont);
    tgtFontType_->addItem("Pixmap", (int)tgt::Font::PixmapFont);
    tgtFontType_->addItem("Polygon", (int)tgt::Font::PolygonFont);
    tgtFontType_->addItem("Texture", (int)tgt::Font::TextureFont);
    tgtFontLayout->addWidget(tgtFontType_);

    tgtFontName_ = new QComboBox();
    tgtFontName_->addItem("VeraMono", "VeraMono.ttf");
    tgtFontName_->addItem("Vera", "Vera.ttf");
    tgtFontLayout->addWidget(tgtFontName_);

    QLabel* fontSizeLabel = new QLabel("Fontsize:");
    tgtFontLayout->addWidget(fontSizeLabel);

    QWidget* tgtFontSize = new QWidget();
    QHBoxLayout* tgtFontSizeLayout = new QHBoxLayout(tgtFontSize);
    tgtFontSizeLayout->setContentsMargins(0,0,0,0);
    tgtFontSizeSlider_ = new QSlider(Qt::Horizontal);
    tgtFontSizeLayout->addWidget(tgtFontSizeSlider_);
    QLCDNumber* tgtFontSizeLCD = new QLCDNumber(2);
    tgtFontSizeLayout->addWidget(tgtFontSizeLCD);
    tgtFontLayout->addWidget(tgtFontSize);

    QLabel* lineWidthLabel = new QLabel("Line width:");
    tgtFontLayout->addWidget(lineWidthLabel);

    QWidget* tgtLineWidth = new QWidget();
    QHBoxLayout* tgtLineWidthLayout = new QHBoxLayout(tgtLineWidth);
    tgtLineWidthLayout->setContentsMargins(0,0,0,0);
    tgtLineWidthSlider_ = new QSlider(Qt::Horizontal);
    tgtLineWidthLayout->addWidget(tgtLineWidthSlider_);
    QLCDNumber* tgtLineWidthLCD = new QLCDNumber(3);
    tgtLineWidthLayout->addWidget(tgtLineWidthLCD);
    tgtFontLayout->addWidget(tgtLineWidth);

    tgtTextAlign_ = new QComboBox();
    tgtTextAlign_->addItem("Left", (int)tgt::Font::Left);
    tgtTextAlign_->addItem("Center", (int)tgt::Font::Center);
    tgtTextAlign_->addItem("Right", (int)tgt::Font::Right);
    tgtFontLayout->addWidget(tgtTextAlign_);

    tgtVerticalTextAlign_ = new QComboBox();
    tgtVerticalTextAlign_->addItem("Top", (int)tgt::Font::Top);
    tgtVerticalTextAlign_->addItem("Middle", (int)tgt::Font::Middle);
    tgtVerticalTextAlign_->addItem("Bottom", (int)tgt::Font::Bottom);
    tgtFontLayout->addWidget(tgtVerticalTextAlign_);

    tgtFontSizeLCD->setSegmentStyle(QLCDNumber::Filled);
    tgtLineWidthLCD->setSegmentStyle(QLCDNumber::Filled);

    tgtFontSizeSlider_->setMinimum(6);
    tgtFontSizeSlider_->setMaximum(36);

    tgtLineWidthSlider_->setMinimum(10);
    tgtLineWidthSlider_->setMaximum(999);

    connect(tgtFontType_,        SIGNAL(activated(int)),     this,            SLOT(updateProperty()));
    connect(tgtFontName_,        SIGNAL(activated(int)),     this,            SLOT(updateProperty()));
    connect(tgtFontSizeSlider_,  SIGNAL(valueChanged(int)),  this,            SLOT(updateProperty()));
    connect(tgtFontSizeSlider_,  SIGNAL(valueChanged(int)),  tgtFontSizeLCD,  SLOT(display(int)));
    connect(tgtLineWidthSlider_, SIGNAL(valueChanged(int)),  this,            SLOT(updateProperty()));
    connect(tgtLineWidthSlider_, SIGNAL(valueChanged(int)),  tgtLineWidthLCD, SLOT(display(int)));

    connect(tgtTextAlign_,        SIGNAL(currentIndexChanged(int)),     this,            SLOT(updateProperty()));
    connect(tgtVerticalTextAlign_,SIGNAL(currentIndexChanged(int)),     this,            SLOT(updateProperty()));

    updateFromPropertySlot();
    addVisibilityControls();
}

void FontPropertyWidget::updateFromPropertySlot() {
    int fontType = (int)property_->get()->getFontType();
    for(int i=0; i<tgtFontType_->count(); i++)
        if(fontType == tgtFontType_->itemData(i).toInt())
            tgtFontType_->setCurrentIndex(i);

    std::string fontName = property_->get()->getFontName();
    if(fontName.length() > 0)
        fontName = fontName.substr(fontName.find_last_of("/\\"));
    for(int i=0; i<tgtFontName_->count(); i++)
        if(fontName.compare(tgtFontName_->itemData(i).toString().toStdString()) == 0)
            tgtFontName_->setCurrentIndex(i);

    tgtFontSizeSlider_->setValue(property_->get()->getSize());

    tgtLineWidthSlider_->setValue(property_->get()->getLineWidth());

    int textAlign = (int)property_->get()->getTextAlignment();
    for(int i=0; i<tgtTextAlign_->count(); i++)
        if(textAlign == tgtTextAlign_->itemData(i).toInt())
            tgtTextAlign_->setCurrentIndex(i);

    int veticalTextAlign = (int)property_->get()->getVerticalTextAlignment();
    for(int i=0; i<tgtVerticalTextAlign_->count(); i++)
        if(veticalTextAlign == tgtVerticalTextAlign_->itemData(i).toInt())
            tgtVerticalTextAlign_->setCurrentIndex(i);
}

void FontPropertyWidget::updateProperty() {
    property_->get()->setSize(tgtFontSizeSlider_->value());
    property_->get()->setFontType((tgt::Font::FontType)tgtFontType_->itemData(tgtFontType_->currentIndex()).toInt());
    property_->get()->setFontName(VoreenApplication::app()->getFontPath(tgtFontName_->itemData(tgtFontName_->currentIndex()).toString().toStdString()));
    property_->get()->setLineWidth(tgtLineWidthSlider_->value());
    property_->get()->setTextAlignment((tgt::Font::TextAlignment)tgtTextAlign_->itemData(tgtTextAlign_->currentIndex()).toInt());
    property_->get()->setVerticalTextAlignment((tgt::Font::VerticalTextAlignment)tgtVerticalTextAlign_->itemData(tgtVerticalTextAlign_->currentIndex()).toInt());
    property_->invalidate();
}

} // namespace voreen
