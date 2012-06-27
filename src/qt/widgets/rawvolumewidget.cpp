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

#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpinBox>
#include <QVBoxLayout>

#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/qt/widgets/rawvolumewidget.h"


namespace voreen {

RawVolumeWidget::RawVolumeWidget(QWidget* parent, const QString& filename, std::string& objectModel, std::string& format,
                                 tgt::ivec3& dim, tgt::vec3& spacing, int& headerSkip)
    : QDialog(parent),
      objectModel_(objectModel),
      format_(format),
      dim_(dim),
      spacing_(spacing),
      headerSkip_(headerSkip)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* datatypeModelLayout = new QHBoxLayout();
    QVBoxLayout* datatypeBoxLayout = new QVBoxLayout();
    QVBoxLayout* objectModelBoxLayout = new QVBoxLayout();
    QHBoxLayout* headerSkipBoxLayout = new QHBoxLayout();

    QGridLayout* dimensionBoxLayout = new QGridLayout();
    QGridLayout* spacingBoxLayout = new QGridLayout();

    headerSkipBox_ = new QSpinBox();
    headerSkipBox_->setValue(0);

    QGroupBox* headerSkipBox = new QGroupBox(this);
    QGroupBox* datatypeBox = new QGroupBox(this);
    QGroupBox* objectModelBox = new QGroupBox(this);
    QGroupBox* dimensionBox = new QGroupBox(this);
    QGroupBox* spacingBox = new QGroupBox(this);

    QLabel* filenameLabel = new QLabel(filename);
    datatypeComboBox_ = new QComboBox();

    headerSkipBox_->setMinimumWidth(80);
    headerSkipBoxLayout->addWidget(headerSkipBox_);
    headerSkipBox->setTitle("Header skip");
    headerSkipBoxLayout->addWidget(new QLabel("bytes"));
    headerSkipBoxLayout->addStretch();
    headerSkipBox->setLayout(headerSkipBoxLayout);

    //datatypeComboBox_->addItems(generateVolumeTypeList());
    datatypeComboBox_->addItem("UCHAR");
    datatypeComboBox_->addItem("USHORT");
    datatypeComboBox_->addItem("USHORT_12");
    datatypeComboBox_->addItem("FLOAT8");
    datatypeComboBox_->addItem("FLOAT16");
    datatypeComboBox_->addItem("FLOAT");
    objectModelComboBox_ = new QComboBox();
    objectModelComboBox_->addItem("Intensity");
    objectModelComboBox_->addItem("RGBA");

    objectModelBox->setTitle("Object model");
    objectModelBox->setLayout(objectModelBoxLayout);
    objectModelBoxLayout->addWidget(objectModelComboBox_);

    datatypeBoxLayout->addWidget(datatypeComboBox_);
    datatypeBox->setLayout(datatypeBoxLayout);
    datatypeBox->setTitle("Data type");

    QLabel* xText = new QLabel();
    xText->setText("x");
    QLabel* yText = new QLabel();
    yText->setText("y");
    QLabel* zText = new QLabel();
    zText->setText("z");
    QLabel* xText1 = new QLabel();
    xText1->setText("x");
    QLabel* yText1 = new QLabel();
    yText1->setText("y");
    QLabel* zText1 = new QLabel();
    zText1->setText("z");

    xDimension_ = new QSpinBox();
    yDimension_ = new QSpinBox();
    zDimension_ = new QSpinBox();
    xDimension_->setMinimum(1);
    yDimension_->setMinimum(1);
    zDimension_->setMinimum(1);
    xDimension_->setMaximum(4096);
    yDimension_->setMaximum(4096);
    zDimension_->setMaximum(4096);
    xDimension_->setValue(128);
    yDimension_->setValue(128);
    zDimension_->setValue(128);
    dimensionBoxLayout->addWidget(xText,0,0,1,1);
    dimensionBoxLayout->addWidget(yText,0,1,1,1);
    dimensionBoxLayout->addWidget(zText,0,2,1,1);
    dimensionBoxLayout->addWidget(xDimension_,1,0,1,1);
    dimensionBoxLayout->addWidget(yDimension_,1,1,1,1);
    dimensionBoxLayout->addWidget(zDimension_,1,2,1,1);
    dimensionBox->setLayout(dimensionBoxLayout);
    dimensionBox->setTitle("Dimensions");

    xSpacing_ = new QDoubleSpinBox();
    ySpacing_ = new QDoubleSpinBox();
    zSpacing_ = new QDoubleSpinBox();
    xSpacing_->setLocale(QLocale::English);
    ySpacing_->setLocale(QLocale::English);
    zSpacing_->setLocale(QLocale::English);
    xSpacing_->setDecimals(6);
    ySpacing_->setDecimals(6);
    zSpacing_->setDecimals(6);
    xSpacing_->setSingleStep(0.1);
    ySpacing_->setSingleStep(0.1);
    zSpacing_->setSingleStep(0.1);
    xSpacing_->setMinimum(0.000001);
    ySpacing_->setMinimum(0.000001);
    zSpacing_->setMinimum(0.000001);
    xSpacing_->setMaximum(100.0);
    ySpacing_->setMaximum(100.0);
    zSpacing_->setMaximum(100.0);
    xSpacing_->setMaximumWidth(150);
    ySpacing_->setMaximumWidth(150);
    zSpacing_->setMaximumWidth(150);
    xSpacing_->setValue(1.0);
    ySpacing_->setValue(1.0);
    zSpacing_->setValue(1.0);

    spacingBoxLayout->addWidget(xText1,0,0,1,1);
    spacingBoxLayout->addWidget(yText1,0,1,1,1);
    spacingBoxLayout->addWidget(zText1,0,2,1,1);
    spacingBoxLayout->addWidget(xSpacing_,1,0,1,1);
    spacingBoxLayout->addWidget(ySpacing_,1,1,1,1);
    spacingBoxLayout->addWidget(zSpacing_,1,2,1,1);

    spacingBox->setLayout(spacingBoxLayout);
    spacingBox->setTitle("Spacing");

    mainLayout->addWidget(filenameLabel);
    mainLayout->addWidget(headerSkipBox);

    datatypeModelLayout->addWidget(datatypeBox);
    datatypeModelLayout->addWidget(objectModelBox);
    mainLayout->addLayout(datatypeModelLayout);

    mainLayout->addWidget(dimensionBox);
    mainLayout->addWidget(spacingBox);

    connect(headerSkipBox_, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
    connect(xDimension_, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
    connect(yDimension_, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));
    connect(zDimension_, SIGNAL(valueChanged(int)), this, SLOT(updateValues()));

    connect(xSpacing_, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
    connect(ySpacing_, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
    connect(zSpacing_, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));

    connect(datatypeComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValues()));
    QPushButton* loadButton = new QPushButton();
    loadButton->setText("Load volume");
    mainLayout->addWidget(loadButton);

    connect(loadButton, SIGNAL(clicked()), this, SLOT(accept()) );
    setWindowTitle(QString::fromStdString("Raw Volume Information"));

    setFixedSize(sizeHint());

    updateValues();
}

void RawVolumeWidget::resizeEvent(QResizeEvent*) {
}

void RawVolumeWidget::updateValues() {
    //*objectModel =
    if(objectModelComboBox_->itemText(objectModelComboBox_->currentIndex()).toStdString() == "RGBA") {
        objectModel_ = "RGBA";
    }
    else {
        objectModel_ = "I";
    }
    headerSkip_ = headerSkipBox_->value();
    format_ = datatypeComboBox_->itemText(datatypeComboBox_->currentIndex()).toStdString();
    dim_.x = xDimension_->value();
    dim_.y = yDimension_->value();
    dim_.z = zDimension_->value();
    spacing_.x = xSpacing_->value();
    spacing_.y = ySpacing_->value();
    spacing_.z = zSpacing_->value();
}

} //namespace voreen
