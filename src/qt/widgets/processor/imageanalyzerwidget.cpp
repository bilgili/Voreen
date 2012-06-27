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

#include "voreen/qt/widgets/processor/imageanalyzerwidget.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QGridLayout>
#include <QMainWindow>
#include <QLabel>
#include <QCheckBox>


namespace voreen {

const std::string ImageAnalyzerWidget::loggerCat_("voreen.ImageAnalyzerWidget");

ImageAnalyzerWidget::ImageAnalyzerWidget(QWidget* parent, ImageAnalyzer* imageAnalyzer)
    : QProcessorWidget(imageAnalyzer, parent)
    , labelVector_(0)
    , checkboxVector_(0)
{
    tgtAssert(imageAnalyzer, "No ImageAnalyzer processor");

    setWindowTitle(QString::fromStdString(imageAnalyzer->getName()));
    resize(256, 256);
}

ImageAnalyzerWidget::~ImageAnalyzerWidget() {
}

void ImageAnalyzerWidget::initialize() {

    QProcessorWidget::initialize();

    ImageAnalyzer* imageAnalyzer = dynamic_cast<ImageAnalyzer*>(processor_);
    tgtAssert(imageAnalyzer, "Imageinformation expected");

    QGridLayout* layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    const std::vector<Property*>& properties = imageAnalyzer->getProperties();
    QLabel* label1 = new QLabel("Description",this);
    QCheckBox* checkbox;
    QFont font1 = label1->font();
    font1.setBold(true);
    font1.setUnderline(true);
    label1->setFont(font1);
    layout->addWidget(label1,1,1);
    label1 = new QLabel("Value",this);
    label1->setFont(font1);
    layout->addWidget(label1,1,2);
    label1 = new QLabel("   ",this);
    layout->addWidget(label1,8,1);
    label1 = new QLabel("   ",this);
    layout->addWidget(label1,9,1);

    label1 = new QLabel("Description",this);
    label1->setFont(font1);
    layout->addWidget(label1,10,1);
    label1 = new QLabel("Min Value",this);
    label1->setFont(font1);
    layout->addWidget(label1,10,2);
    label1 = new QLabel("Max Value",this);
    label1->setFont(font1);
    layout->addWidget(label1,10,3);
    label1 = new QLabel("Median",this);
    label1->setFont(font1);
    layout->addWidget(label1,10,4);
    label1 = new QLabel("Add to Histogram",this);
    label1->setFont(font1);
    layout->addWidget(label1,10,5);
    size_t j = 0;
    size_t j2 = 0;
    for (size_t i=0; i < properties.size(); ++i){
        if (dynamic_cast<FloatProperty*>(properties[i])){
            FloatProperty* fProperty = dynamic_cast<FloatProperty*>(properties[i]);
            if (j % 3 == 0) {
                label1 = new QLabel(fProperty->getGuiName().substr(4,fProperty->getGuiName().size()).c_str(),this);
                layout->addWidget(label1,11+j/3,1);
            }
            label1 = new QLabel(QString::number(fProperty->get()),this);
            layout->addWidget(label1,11+j/3,j%3 + 2);
            labelVector_.push_back(label1);
            ++j;
        }
        else if (dynamic_cast<IntProperty*>(properties[i])){
            IntProperty* iProperty = dynamic_cast<IntProperty*>(properties[i]);
            label1 = new QLabel(iProperty->getGuiName().c_str(),this);
            layout->addWidget(label1,i+1,1);
            label1 = new QLabel(QString::number(iProperty->get()),this);
            layout->addWidget(label1,i+1,2);
            labelVector_.push_back(label1);
        }
        else if ((i > 5) && (dynamic_cast<BoolProperty*>(properties[i]))){
            BoolProperty* bProperty = dynamic_cast<BoolProperty*>(properties[i]);
            checkbox = new QCheckBox(" ",this);
            checkbox->setTristate(false);
            if (bProperty) {
                checkbox->setChecked(true);
            }
            QObject::connect(checkbox,SIGNAL(stateChanged(int)),this,SLOT(checkBoxChange(int)));
            layout->addWidget(checkbox,11+j2,5);
            checkboxVector_.push_back(std::pair<QCheckBox*,int>(checkbox,i));
            ++j2;
        }
    }

    setLayout(layout);

    show();

    initialized_ = true;
}

void ImageAnalyzerWidget::checkBoxChange(int state) {
    ImageAnalyzer* imageAnalyzer = dynamic_cast<ImageAnalyzer*>(processor_);
    for (size_t j = 0; j < checkboxVector_.size(); ++j) {
        if (sender() == checkboxVector_.at(j).first) {
            BoolProperty* bProperty = dynamic_cast<BoolProperty*>(imageAnalyzer->getProperties()[checkboxVector_.at(j).second]);
            if ((bProperty->get() == 0 && state != 0) || (bProperty->get() != 0 && state == 0))
            bProperty->set(state);
        }
    }
}

void ImageAnalyzerWidget::updateFromProcessor(){
    ImageAnalyzer* imageAnalyzer = dynamic_cast<ImageAnalyzer*>(processor_);
    tgtAssert(imageAnalyzer, "Imageinformation expected");

    const std::vector<Property*>& properties = imageAnalyzer->getProperties();
    int j = 0;
    int j2 = 0;
    QLabel* label;
    for (size_t i=0; i < properties.size(); ++i){
        if (dynamic_cast<FloatProperty*>(properties[i])){
            FloatProperty* fProperty = dynamic_cast<FloatProperty*>(properties[i]);
            label = dynamic_cast<QLabel*>(labelVector_.at(j));
            label->setText(QString::number(fProperty->get()));
            ++j;
        }
        else if (dynamic_cast<IntProperty*>(properties[i])) {
            IntProperty* iProperty = dynamic_cast<IntProperty*>(properties[i]);
            label = dynamic_cast<QLabel*>(labelVector_.at(j));
            label->setText(QString::number(iProperty->get()));
            ++j;
        }
        else if ((i > 5) && (dynamic_cast<BoolProperty*>(properties[i]))){
            BoolProperty* bProperty = dynamic_cast<BoolProperty*>(properties[i]);
            checkboxVector_.at(j2).first->setChecked(bProperty->get()*2);
            ++j2;
        }
    }
}

} //namespace voreen

