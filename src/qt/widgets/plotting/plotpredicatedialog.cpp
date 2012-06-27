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

#include "voreen/qt/widgets/plotting/plotpredicatedialog.h"

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QAction>
#include <qvalidator.h>


namespace voreen {

PlotPredicateDialog::PlotPredicateDialog(
                 const std::vector<std::pair<std::string,int> >& comboBoxText,
                 const PlotBase::ColumnType valueType, const int selected,
                 const std::vector<PlotCellValue>& values, const std::vector<int>& comboboxValues,
                 QWidget* parent)
    : QDialog(parent)
    , comboBoxText_(0)
    , editVector_(0)
    , values_(0)
    , labelVector_(0)
{
    setWindowTitle(QString(tr("Select Predicates",0)));
    resize(256,256);
    setModal(true);
    valueType_ = PlotBase::EMPTY;
    choice_ = new QComboBox(this);
    QObject::connect(choice_,SIGNAL(currentIndexChanged(int)),this,SLOT(choiceChange(int)));
    mainLayout = new QGridLayout(this);
    mainLayout->addWidget(choice_,0,1,Qt::AlignCenter);
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    okButton_ = new QPushButton(tr("OK"));
    okButton_->setDefault(true);
    cancelButton_ = new QPushButton(tr("Cancel"));
    QObject::connect(buttons,SIGNAL(accepted()),this,SLOT(clickedOk()));
    QObject::connect(buttons,SIGNAL(rejected()),this,SLOT(clickedCancel()));
    buttons->addButton(okButton_, QDialogButtonBox::AcceptRole);
    buttons->addButton(cancelButton_,QDialogButtonBox::RejectRole);
    mainLayout->addWidget(buttons,9,1,Qt::AlignCenter);
    setLayout(mainLayout);
    initialize(comboBoxText,valueType,selected,values,comboboxValues);
}


PlotPredicateDialog::~PlotPredicateDialog() {
    comboBoxText_.clear();
}

void PlotPredicateDialog::initialize(const std::vector<std::pair<std::string,int> >& comboBoxText,
                                  const PlotBase::ColumnType valueType, const int selected,
                                  const std::vector<PlotCellValue>& values, const std::vector<int>& comboboxValues) {
    comboBoxText_ = comboBoxText;
    choice_->clear();
    valueType_ = valueType;
    int index = 0;
    for (size_t i = 0; i < comboBoxText_.size(); ++i) {
        choice_->addItem(QString::fromStdString(comboBoxText_.at(i).first),QVariant(comboboxValues.at(i)));
        if (selected == comboboxValues.at(i))
            index = i;
    }
    values_ = values;
    choice_->setCurrentIndex(index);
}

void PlotPredicateDialog::choiceChange(int index) {
    unsigned int x = static_cast<unsigned int>(index);
    for (size_t i = 0; i < editVector_.size(); ++i) {
        delete editVector_.at(i);
        delete labelVector_.at(i);
    }
    editVector_.clear();
    labelVector_.clear();
    if (comboBoxText_.size() > x) {
        for (int i = 0; i < comboBoxText_.at(x).second; ++i) {
            editVector_.push_back(new QLineEdit(this));
            labelVector_.push_back(new QLabel(this));
            labelVector_.at(i)->setText(QString(tr("Value: ",0)));
            if (valueType_ == PlotBase::NUMBER) {
                QDoubleValidator* dVal = new QDoubleValidator(this);
//                dVal->setLocale(QLocale(QLocale::English));
                editVector_.at(i)->setValidator(dVal);
                if (values_.at(i).isTag()) {
                    editVector_.at(i)->setText(QString(tr("",0)));
                }
                else {
                    editVector_.at(i)->setText(QString::number(values_.at(i).getValue()));
                }
            }
            else {
                if (values_.at(i).isTag()) {
                    editVector_.at(i)->setText(QString::fromStdString(values_.at(i).getTag()));
                }
                else {
                    editVector_.at(i)->setText(QString::number(values_.at(i).getValue()));
                }
            }
            mainLayout->addWidget(labelVector_.at(i),i+1,0,Qt::AlignCenter);
            mainLayout->addWidget(editVector_.at(i),i+1,1,Qt::AlignCenter);
        }
    }
}


void PlotPredicateDialog::clickedOk() {
    accept();
}

void PlotPredicateDialog::clickedCancel() {
    reject();
}


std::vector<std::string> PlotPredicateDialog::getStringValues() const {
    std::vector<std::string> result;
    for (size_t i = 0; i < editVector_.size(); ++i) {
        result.push_back(editVector_.at(i)->text().toStdString());
    }
    return result;
}

int PlotPredicateDialog::getSelected() const {
    return choice_->currentIndex();
}

int PlotPredicateDialog::getSelectedValue() const {
    return choice_->itemData(getSelected()).toInt();
}

} // namespace voreen
