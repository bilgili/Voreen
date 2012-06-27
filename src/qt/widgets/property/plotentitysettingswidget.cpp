/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/property/plotentitysettingswidget.h"
#include "voreen/core/voreenapplication.h"

#include <QLabel>
#include <QGridLayout>
#include <QTabWidget>
#include <QColorDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>

namespace voreen {

PlotEntitySettingsWidget::PlotEntitySettingsWidget(PlotEntitiesProperty* prop)
    : property_(prop) {
    setObjectName("PlotEntitySettingsWidget");
}

PlotEntitySettingsWidget::~PlotEntitySettingsWidget() {
}

void PlotEntitySettingsWidget::createWidgets() {
    switch (property_->getEntities()) {
        case PlotEntitySettings::LINE: {
            tabWidget_ = new QTabWidget(this);
            QWidget* lineTab = new QWidget(this);
            QWidget* candleStickTab = new QWidget(this);
            QGridLayout* gridLayout = new QGridLayout(lineTab);
            QGridLayout* gridLayout2 = new QGridLayout(candleStickTab);
            //column combo boxes
            cbMainColumn_ = new QComboBox(); cbCandleTopColumn_ = new QComboBox();
            cbCandleBottomColumn_ = new QComboBox(); cbStickTopColumn_ = new QComboBox();
            cbStickBottomColumn_ = new QComboBox(); cbOptionalColumn_ = new QComboBox();
            connect(cbMainColumn_, SIGNAL(activated(int)), this, SLOT(cbMainColumnChanged(int)));
            connect(cbCandleTopColumn_, SIGNAL(activated(int)), this, SLOT(cbCandleTopColumnChanged(int)));
            connect(cbCandleBottomColumn_, SIGNAL(activated(int)), this, SLOT(cbCandleBottomColumnChanged(int)));
            connect(cbStickTopColumn_, SIGNAL(activated(int)), this, SLOT(cbStickTopColumnChanged(int)));
            connect(cbOptionalColumn_, SIGNAL(activated(int)), this, SLOT(cbOptionalColumnChanged(int)));
            connect(cbStickBottomColumn_, SIGNAL(activated(int)), this, SLOT(cbStickBottomColumnChanged(int)));

            cbStyle_ = new QComboBox();
            cbStyle_->addItem("Continuous"); cbStyle_->addItem("Dotted");  cbStyle_->addItem("Dashed");
            connect(cbStyle_, SIGNAL(activated(int)), this, SLOT(cbStyleChanged(int)));
            chbSpline_ = new QCheckBox();
            connect(chbSpline_, SIGNAL(stateChanged(int)), this, SLOT(chbSplineChanged()));
            chbErrorbar_ = new QCheckBox();
            connect(chbErrorbar_, SIGNAL(stateChanged(int)), this, SLOT(chbErrorbarChanged()));
            //colors
            firstColor_ = toQColor(property_->getColorMap().getColorAtIndex(0));
            cclrFirst_ = new ClickableColorLabel("");
            connect(cclrFirst_, SIGNAL(clicked(void)), this, SLOT(firstColorDialog(void)));
            cclrFirst_->setColor(firstColor_);
            secondColor_ = toQColor(property_->getColorMap().getColorAtIndex(1));
            cclrSecond_ = new ClickableColorLabel("");
            connect(cclrSecond_, SIGNAL(clicked(void)), this, SLOT(secondColorDialog(void)));
            cclrSecond_->setColor(secondColor_);
            cclrThird_ = new ClickableColorLabel("");
            connect(cclrThird_, SIGNAL(clicked(void)), this, SLOT(firstColorDialog(void)));
            cclrThird_->setColor(firstColor_);
            cclrFourth_ = new ClickableColorLabel("");
            connect(cclrFourth_, SIGNAL(clicked(void)), this, SLOT(secondColorDialog(void)));
            cclrFourth_->setColor(secondColor_);
            //add widgets
            gridLayout->addWidget(new QLabel(tr("Line")), 0, 0);
            gridLayout->addWidget(cbMainColumn_, 0, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Color")), 1, 0);
            gridLayout->addWidget(cclrFirst_, 1, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Style")), 2, 0);
            gridLayout->addWidget(cbStyle_, 2, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Error")), 3, 0);
            gridLayout->addWidget(cbOptionalColumn_, 3, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Error Color")), 4, 0);
            gridLayout->addWidget(cclrSecond_, 4, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Errorbar")), 5, 0);
            gridLayout->addWidget(chbErrorbar_, 5, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Spline")), 6, 0);
            gridLayout->addWidget(chbSpline_, 6, 1, 1, 2);

            gridLayout2->addWidget(new QLabel(tr("Candle Top")), 0, 0);
            gridLayout2->addWidget(cbCandleTopColumn_, 0, 1, 1, 2);
            gridLayout2->addWidget(new QLabel(tr("Candle Bottom")), 1, 0);
            gridLayout2->addWidget(cbCandleBottomColumn_, 1, 1, 1, 2);
            gridLayout2->addWidget(new QLabel(tr("Stick Top")), 2, 0);
            gridLayout2->addWidget(cbStickTopColumn_, 2, 1, 1, 2);
            gridLayout2->addWidget(new QLabel(tr("Stick Bottom")), 3, 0);
            gridLayout2->addWidget(cbStickBottomColumn_, 3, 1, 1, 2);
            gridLayout2->addWidget(new QLabel(tr("Stick Color")), 4, 0);
            gridLayout2->addWidget(cclrThird_, 4, 1, 1, 2);
            gridLayout2->addWidget(new QLabel(tr("Candle Color")), 5, 0);
            gridLayout2->addWidget(cclrFourth_, 5, 1, 1, 2);

            tabWidget_->addTab(lineTab, tr("Line"));
            tabWidget_->addTab(candleStickTab, tr("Candlestick"));
            connect(tabWidget_, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
            QVBoxLayout* boxLayout = new QVBoxLayout();
            boxLayout->addWidget(tabWidget_);
            setLayout(boxLayout);
            setWindowTitle(tr("Line settings"));
            break;
            }
        case PlotEntitySettings::BAR: {
            QGridLayout* gridLayout = new QGridLayout();

            gridLayout->addWidget(new QLabel(tr("Bars")), 0, 0);
            cbMainColumn_ = new QComboBox();
            gridLayout->addWidget(cbMainColumn_, 0, 1, 1, 2);
            connect(cbMainColumn_, SIGNAL(activated(int)), this, SLOT(cbMainColumnChanged(int)));

            gridLayout->addWidget(new QLabel(tr("Color")), 1, 0);
            firstColor_ = toQColor(property_->getColorMap().getColorAtIndex(0));
            cclrFirst_ = new ClickableColorLabel("");
            connect(cclrFirst_, SIGNAL(clicked(void)), this, SLOT(firstColorDialog(void)));
            cclrFirst_->setColor(firstColor_);
            gridLayout->addWidget(cclrFirst_, 1, 1, 1, 2);

            setLayout(gridLayout);
            setWindowTitle(tr("Bar settings"));
            break;
            }
        case PlotEntitySettings::SURFACE: {
            QGridLayout* gridLayout = new QGridLayout();

            gridLayout->addWidget(new QLabel(tr("Surface")), 0, 0);
            cbMainColumn_ = new QComboBox();
            gridLayout->addWidget(cbMainColumn_, 0, 1, 1, 2);
            connect(cbMainColumn_, SIGNAL(activated(int)), this, SLOT(cbMainColumnChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Color")), 1, 0);
            firstColor_ = toQColor(property_->getColorMap().getColorAtIndex(0));
            cclrFirst_ = new ClickableColorLabel("");
            connect(cclrFirst_, SIGNAL(clicked(void)), this, SLOT(firstColorDialog(void)));
            cclrFirst_->setColor(firstColor_);
            gridLayout->addWidget(cclrFirst_, 1, 1, 1, 2);
            cbColorMap_ = new QComboBox();
            std::vector<std::string> colorMapLabels = ColorMap::getColorMapLabels();
            for (size_t i = 0; i < colorMapLabels.size(); ++i)
                cbColorMap_->addItem(QString::fromStdString(colorMapLabels.at(i)));
            gridLayout->addWidget(cbColorMap_, 1, 1, 1, 2);
            cbColorMap_->setVisible(false);
            connect(cbColorMap_, SIGNAL(activated(int)), this, SLOT(cbColorMapChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Wire Color")), 2, 0);
            secondColor_ = toQColor(tgt::Color::black);
            cclrSecond_ = new ClickableColorLabel("");
            connect(cclrSecond_, SIGNAL(clicked(void)), this, SLOT(secondColorDialog(void)));
            cclrSecond_->setColor(secondColor_);
            gridLayout->addWidget(cclrSecond_, 2, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Wire Only")), 3, 0);
            chbWireOnly_ = new QCheckBox();
            gridLayout->addWidget(chbWireOnly_, 3, 1, 1, 2);
            connect(chbWireOnly_, SIGNAL(stateChanged(int)), this, SLOT(chbWireOnlyChanged()));
            gridLayout->addWidget(new QLabel(tr("Height Map")), 4, 0);
            chbHeightMap_ = new QCheckBox();
            gridLayout->addWidget(chbHeightMap_, 4, 1, 1, 2);
            connect(chbHeightMap_, SIGNAL(stateChanged(int)), this, SLOT(chbHeightMapChanged()));
            gridLayout->addWidget(new QLabel(tr("Color Data")), 5, 0);
            cbOptionalColumn_ = new QComboBox();
            gridLayout->addWidget(cbOptionalColumn_, 5, 1, 1, 2);
            connect(cbOptionalColumn_, SIGNAL(activated(int)), this, SLOT(cbOptionalColumnChanged(int)));
            setLayout(gridLayout);
            break;
            }
        case PlotEntitySettings::SCATTER: {
            QGridLayout* gridLayout = new QGridLayout();
            gridLayout->addWidget(new QLabel(tr("Data")), 0, 0);
            cbMainColumn_ = new QComboBox();
            gridLayout->addWidget(cbMainColumn_, 0, 1, 1, 2);
            connect(cbMainColumn_, SIGNAL(activated(int)), this, SLOT(cbMainColumnChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Color")), 1, 0);
            firstColor_ = toQColor(property_->getColorMap().getColorAtIndex(0));
            cclrFirst_ = new ClickableColorLabel("");
            connect(cclrFirst_, SIGNAL(clicked(void)), this, SLOT(firstColorDialog(void)));
            cclrFirst_->setColor(firstColor_);
            gridLayout->addWidget(cclrFirst_, 1, 1, 1, 2);
            cbColorMap_ = new QComboBox();
            std::vector<std::string> colorMapLabels = ColorMap::getColorMapLabels();
            for (size_t i = 0; i < colorMapLabels.size(); ++i)
                cbColorMap_->addItem(QString::fromStdString(colorMapLabels.at(i)));
            gridLayout->addWidget(cbColorMap_, 1, 1, 1, 2);
            cbColorMap_->setVisible(false);
            connect(cbColorMap_, SIGNAL(activated(int)), this, SLOT(cbColorMapChanged(int)));
            cbStyle_ = new QComboBox();
            cbStyle_->addItem("Point"); cbStyle_->addItem("Circle"); cbStyle_->addItem("Triangle");  cbStyle_->addItem("Quad");
            connect(cbStyle_, SIGNAL(activated(int)), this, SLOT(cbStyleChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Glyph")), 2, 0);
            gridLayout->addWidget(cbStyle_, 2, 1, 1, 2);
            gridLayout->addWidget(new QLabel(tr("Glyph Size")), 3, 0);
            sbMinGlyphSize_ = new QDoubleSpinBox();
            sbMaxGlyphSize_ = new QDoubleSpinBox();
            sbMinGlyphSize_->setDecimals(2);
            sbMaxGlyphSize_->setDecimals(2);
            gridLayout->addWidget(sbMinGlyphSize_, 3, 1, 1, 1);
            gridLayout->addWidget(sbMaxGlyphSize_, 3, 2, 1, 1);
            sbMinGlyphSize_->setVisible(false);
            connect(sbMinGlyphSize_, SIGNAL(valueChanged(double)), this, SLOT(sbMinGlyphSizeChanged(double)));
            connect(sbMaxGlyphSize_, SIGNAL(valueChanged(double)), this, SLOT(sbMaxGlyphSizeChanged(double)));
            gridLayout->addWidget(new QLabel(tr("Color Data")), 4, 0);
            cbOptionalColumn_ = new QComboBox();
            gridLayout->addWidget(cbOptionalColumn_, 4, 1, 1, 2);
            connect(cbOptionalColumn_, SIGNAL(activated(int)), this, SLOT(cbOptionalColumnChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Size Data")), 5, 0);
            cbSecondOptionalColumn_ = new QComboBox();
            gridLayout->addWidget(cbSecondOptionalColumn_, 5, 1, 1, 2);
            connect(cbSecondOptionalColumn_, SIGNAL(activated(int)), this,
                SLOT(cbSecondOptionalColumnChanged(int)));
            gridLayout->addWidget(new QLabel(tr("Load Icon")), 6, 0);
            btOpenFileDialog_ = new QPushButton();
            connect(btOpenFileDialog_, SIGNAL(clicked(void)), this, SLOT(btOpenFileDialogClicked(void)));
            gridLayout->addWidget(btOpenFileDialog_, 6, 1, 1, 2);
            setLayout(gridLayout);
            setWindowTitle(tr("Scatter settings"));
            break;
            }
        default:
            break;
    }
}

void PlotEntitySettingsWidget::reCreateWidgets() {
    //this method is called even before there is a plotdata, so we have to check it
    if (!property_->dataValid())
        return;
    switch (entitySettings_.getEntity()) {
        case PlotEntitySettings::LINE:
            //comboboxes
            fillComboBox(cbMainColumn_);
            fillComboBox(cbCandleTopColumn_);
            fillComboBox(cbCandleBottomColumn_);
            fillComboBox(cbStickTopColumn_);
            fillComboBox(cbStickBottomColumn_);
            fillComboBox(cbOptionalColumn_, true);
            break;
        case PlotEntitySettings::BAR:
            fillComboBox(cbMainColumn_);
            break;
        case PlotEntitySettings::SURFACE:
            fillComboBox(cbMainColumn_);
            fillComboBox(cbOptionalColumn_, true);
            break;
        case PlotEntitySettings::SCATTER:
            fillComboBox(cbMainColumn_);
            fillComboBox(cbOptionalColumn_, true);
            fillComboBox(cbSecondOptionalColumn_, true);
            break;
        default :
            break;
    }
}

void PlotEntitySettingsWidget::updateWidgets() {
    //maybe entity settings is changed
    if (static_cast<size_t>(index_) < property_->get().size())
        entitySettings_ = property_->get().at(index_);
    reCreateWidgets();
    //this method is called even before there is a plot data, so we have to check it
    if (!property_->dataValid())
        return;
    blockSignals(true);
    switch (entitySettings_.getEntity()) {
        case PlotEntitySettings::LINE:
            cbStyle_->setCurrentIndex(static_cast<int>(entitySettings_.getLineStyle()));
            firstColor_ = toQColor(entitySettings_.getFirstColor());
            cclrFirst_->setColor(firstColor_);
            secondColor_ = toQColor(entitySettings_.getSecondColor());
            cclrSecond_->setColor(secondColor_);
            cclrThird_->setColor(firstColor_);
            cclrFourth_->setColor(secondColor_);
            cbMainColumn_->setCurrentIndex(cbMainColumn_->findData(entitySettings_.getMainColumnIndex()));
            cbOptionalColumn_->setCurrentIndex(
                cbOptionalColumn_->findData(entitySettings_.getOptionalColumnIndex()));
            cbCandleTopColumn_->setCurrentIndex(
                cbCandleTopColumn_->findData(entitySettings_.getCandleTopColumnIndex()));
            cbCandleBottomColumn_->setCurrentIndex(
                cbCandleBottomColumn_->findData(entitySettings_.getCandleBottomColumnIndex()));
            cbStickTopColumn_->setCurrentIndex(
                cbStickTopColumn_->findData(entitySettings_.getStickTopColumnIndex()));
            cbStickBottomColumn_->setCurrentIndex(
                cbStickBottomColumn_->findData(entitySettings_.getStickBottomColumnIndex()));
            chbSpline_->setChecked(entitySettings_.getSplineFlag());
            chbErrorbar_->setChecked(entitySettings_.getErrorbarFlag());
            entitySettings_.getCandleStickFlag() ? tabWidget_->setCurrentIndex(1)
                                                 : tabWidget_->setCurrentIndex(0);
            break;
        case PlotEntitySettings::BAR:
            cbMainColumn_->setCurrentIndex(cbMainColumn_->findData(entitySettings_.getMainColumnIndex()));
            firstColor_ = toQColor(entitySettings_.getFirstColor());
            cclrFirst_->setColor(firstColor_);
            break;
        case PlotEntitySettings::SURFACE:
            cbMainColumn_->setCurrentIndex(cbMainColumn_->findData(entitySettings_.getMainColumnIndex()));
            cbOptionalColumn_->setCurrentIndex(
                cbOptionalColumn_->findData(entitySettings_.getOptionalColumnIndex()));
            firstColor_ = toQColor(entitySettings_.getFirstColor());
            cclrFirst_->setColor(firstColor_);
            secondColor_ = toQColor(entitySettings_.getSecondColor());
            cclrSecond_->setColor(secondColor_);
            chbWireOnly_->setChecked(entitySettings_.getWireOnlyFlag());
            chbHeightMap_->setChecked(entitySettings_.getHeightmapFlag());
            cclrFirst_->setVisible((entitySettings_.getOptionalColumnIndex() == -1));
            cbColorMap_->setVisible((entitySettings_.getOptionalColumnIndex() != -1));
            break;
        case PlotEntitySettings::SCATTER:
            cbMainColumn_->setCurrentIndex(cbMainColumn_->findData(entitySettings_.getMainColumnIndex()));
            cbOptionalColumn_->setCurrentIndex(
                cbOptionalColumn_->findData(entitySettings_.getOptionalColumnIndex()));
            cbSecondOptionalColumn_->setCurrentIndex(
                cbSecondOptionalColumn_->findData(entitySettings_.getSecondOptionalColumnIndex()));
            firstColor_ = toQColor(entitySettings_.getFirstColor());
            cclrFirst_->setColor(firstColor_);
            sbMinGlyphSize_->setValue(static_cast<double>(entitySettings_.getMinGlyphSize()));
            sbMaxGlyphSize_->setValue(static_cast<double>(entitySettings_.getMaxGlyphSize()));
            cbColorMap_->setCurrentIndex(cbColorMap_->findText(
                QString::fromStdString(entitySettings_.getColorMap().toString())));
            cclrFirst_->setVisible((entitySettings_.getOptionalColumnIndex() == -1));
            cbColorMap_->setVisible((entitySettings_.getOptionalColumnIndex() != -1));
            cbStyle_->setCurrentIndex(static_cast<int>(entitySettings_.getGlyphStyle()));
            sbMinGlyphSize_->setVisible((entitySettings_.getSecondOptionalColumnIndex() != -1));
            updateBtOpenFileDialogText(entitySettings_.getTexturePath());
            break;
        default :
            break;
    }
    blockSignals(false);
}

void PlotEntitySettingsWidget::fillComboBox(QComboBox* cb, bool emptyEntry) {
    cb->clear();
    if (emptyEntry)
        cb->addItem(tr(""), QVariant(-1));
    PlotData* data = property_->getPlotData();
    for (int i = 0; i < data->getColumnCount(); ++i) {
        if (data->getColumnType(i) == PlotBase::NUMBER        //number
            && property_->getXColumnIndex() != i                         //not current x axis
            && (entitySettings_.getEntity() == PlotEntitySettings::LINE ||
                entitySettings_.getEntity() == PlotEntitySettings::BAR ||
                property_->getYColumnIndex() != i))                       //not current y axis
            cb->addItem(QString::fromStdString(data->getColumnLabel(i)), QVariant(i));
    }
    cb->setCurrentIndex(0);
}

tgt::Color PlotEntitySettingsWidget::toTgtColor(QColor color) {
    return tgt::Color(color.redF(),color.greenF(),color.blueF(), color.alphaF());
}

QColor PlotEntitySettingsWidget::toQColor(tgt::Color color) {
    return QColor(static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
                  static_cast<int>(color.b * 255), static_cast<int>(color.a * 255));
}


void PlotEntitySettingsWidget::setEntity(PlotEntitySettings entitySettings, int index) {
    tgtAssert(property_->getEntities() == entitySettings.getEntity(),
        "PlotEntitySettingsWidget::setEntity: entities do not fit");
    index_ = index;
    entitySettings_ = entitySettings;
    updateWidgets();
}

void PlotEntitySettingsWidget::updateProperty() {
    property_->setPlotEntitySettings(entitySettings_, index_);
}

void PlotEntitySettingsWidget::firstColorDialog() {
    #if (QT_VERSION >= 0x040500)
        QColor col = QColorDialog::getColor(firstColor_, this, "Color", QColorDialog::ShowAlphaChannel);
    if(col.isValid())
        firstColor_ = col;
    #else   //if qt is < version 4.5 there is no way to set the alpha value to its correct value
    firstColor_.setRgba(QColorDialog::getRgba(firstColor_.rgba()));
    #endif
    cclrFirst_->setColor(firstColor_);
    //third ccclr only exist in line mode
    if (entitySettings_.getEntity() == PlotEntitySettings::LINE)
        cclrThird_->setColor(firstColor_);
    entitySettings_.setFirstColor(toTgtColor(firstColor_));
    entitySettings_.setUseTextureFlag(false);
    updateProperty();
}

void PlotEntitySettingsWidget::secondColorDialog() {
    #if (QT_VERSION >= 0x040500)
        QColor col = QColorDialog::getColor(secondColor_, this, "Color", QColorDialog::ShowAlphaChannel);
    if(col.isValid())
        secondColor_ = col;
    #else   //if qt is < version 4.5 there is no way to set the alpha value to its correct value
    secondColor_.setRgba(QColorDialog::getRgba(secondColor_.rgba()));
    #endif
    cclrSecond_->setColor(secondColor_);
    if (entitySettings_.getEntity() == PlotEntitySettings::LINE)
        cclrFourth_->setColor(secondColor_);
    entitySettings_.setSecondColor(toTgtColor(secondColor_));
    updateProperty();
}

void PlotEntitySettingsWidget::cbMainColumnChanged(int index) {
    entitySettings_.setMainColumnIndex(cbMainColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbCandleTopColumnChanged(int index) {
    entitySettings_.setCandleTopColumnIndex(cbCandleTopColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbCandleBottomColumnChanged(int index) {
    entitySettings_.setCandleBottomColumnIndex(cbCandleBottomColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbStickTopColumnChanged(int index) {
    entitySettings_.setStickTopColumnIndex(cbStickTopColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbStickBottomColumnChanged(int index) {
    entitySettings_.setStickBottomColumnIndex(cbStickBottomColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbOptionalColumnChanged(int index) {
    entitySettings_.setOptionalColumnIndex(cbOptionalColumn_->itemData(index).toInt());
    entitySettings_.setUseTextureFlag(false);
    updateProperty();
}

void PlotEntitySettingsWidget::cbSecondOptionalColumnChanged(int index) {
    entitySettings_.setSecondOptionalColumnIndex(cbSecondOptionalColumn_->itemData(index).toInt());
    updateProperty();
}

void PlotEntitySettingsWidget::cbStyleChanged(int index) {
    if (entitySettings_.getEntity() == PlotEntitySettings::LINE)
        entitySettings_.setLineStyle(static_cast<PlotEntitySettings::LineStyle>(index));
    else //if SCATTER
        entitySettings_.setGlyphStyle(static_cast<PlotEntitySettings::GlyphStyle>(index));
    updateProperty();
}

void PlotEntitySettingsWidget::chbSplineChanged() {
    entitySettings_.setSplineFlag(chbSpline_->isChecked());
    updateProperty();
}

void PlotEntitySettingsWidget::chbErrorbarChanged() {
    entitySettings_.setErrorbarFlag(chbErrorbar_->isChecked());
    updateProperty();
}

void PlotEntitySettingsWidget::tabChanged(int index) {
    index == 0 ? entitySettings_.setCandleStickFlag(false) : entitySettings_.setCandleStickFlag(true);
    updateProperty();
}

void PlotEntitySettingsWidget::chbWireOnlyChanged() {
    entitySettings_.setWireOnlyFlag(chbWireOnly_->isChecked());
    updateProperty();
}

void PlotEntitySettingsWidget::chbHeightMapChanged() {
    entitySettings_.setHeightmapFlag(chbHeightMap_->isChecked());
    updateProperty();
}

void PlotEntitySettingsWidget::cbColorMapChanged(int index) {
    entitySettings_.setColorMap(ColorMap::createColorMap(index));
    entitySettings_.setUseTextureFlag(false);
    updateProperty();
}

void PlotEntitySettingsWidget::sbMinGlyphSizeChanged(double value) {
    entitySettings_.setMinGlyphSize(static_cast<float>(value));
    updateProperty();
}

void PlotEntitySettingsWidget::sbMaxGlyphSizeChanged(double value) {
    entitySettings_.setMaxGlyphSize(static_cast<float>(value));
    updateProperty();
}

void PlotEntitySettingsWidget::btOpenFileDialogClicked() {
    QString directory;
    // use directory of current property value if any, default directory otherwise
    if (!entitySettings_.getTexturePath().empty())
        directory = QString::fromStdString(entitySettings_.getTexturePath());
    else
        directory = QString::fromStdString(VoreenApplication::app()->getTexturePath());

    const QString fileFilter = tr("*.jpg;*.png;*.bmp;;All files (*)");
    QString filename = QFileDialog::getOpenFileName(QWidget::parentWidget(),
        tr("Select File"), directory, fileFilter);
    if (!filename.isEmpty()) {
        entitySettings_.setTexturePath(filename.toStdString());
        entitySettings_.setUseTextureFlag(true);
        updateBtOpenFileDialogText(entitySettings_.getTexturePath());
    }
    updateProperty();
}

void PlotEntitySettingsWidget::updateBtOpenFileDialogText(const std::string& text) {
    if (!text.empty()) {
        size_t index = text.find_last_of('/');
        if (index == text.npos)
            index = text.find_last_of('\\');
        std::string endFilename = text;
        if (index != text.npos)
            endFilename = text.substr(index + 1, text.length());
        btOpenFileDialog_->setText(QString::fromStdString(endFilename));
    }
    else {
        btOpenFileDialog_->setText(tr("Select File"));
    }
    btOpenFileDialog_->update();
}

} // namespace
