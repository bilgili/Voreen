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

#ifdef VRN_WITH_PYTHON
// Python header must be included before other system headers
#include <Python.h>
#endif

#include "voreen/qt/widgets/network/editor/propertylinkdialog.h"

#include "voreen/core/vis/properties/link/linkevaluatorbase.h"
#include "voreen/core/vis/properties/link/linkevaluatorfactory.h"
#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include "voreen/qt/widgets/network/editor/propertygraphicsitem.h"
#include "voreen/qt/widgets/network/editor/scripteditor.h"
#include <QComboBox>
#include <QErrorMessage>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#ifdef VRN_WITH_PYTHON
#include "voreen/core/vis/properties/link/linkevaluatorpython.h"
#endif

namespace voreen {

namespace {
    const QSize iconSize(15, 15);
    const QString leftArrowIcon(":/voreenve/icons/arrow-left.png");
    const QString biArrowIcon(":/voreenve/icons/arrow-leftright.png");
    const QString rightArrowIcon(":/voreenve/icons/arrow-right.png");
}

/*
 * /------------------------------------\
 * |             :  <---  :             |
 * | ...................................|
 * |  Property A :  <-->  : Property B  |
 * |....................................|
 * |             :  --->  :             |
 * |....................................|
 * |   Function  :  Combo-:-box         |
 * |....................................|
 * |                                    |
 * |....................................|
 * |                      OK    Cancel  |
 * \------------------------------------/
 */

PropertyLinkDialog::PropertyLinkDialog(QWidget* parent, PropertyGraphicsItem* sourceGraphicsItem,
                                       PropertyGraphicsItem* destGraphicsItem, PropertyLinkSelectedButton selectedButton)
    : QDialog(parent)
    , sourceGraphicsItem_(sourceGraphicsItem)
    , destinationGraphicsItem_(destGraphicsItem)
{
    createWidgets();
    setupCombobox();

    switch (selectedButton) {
    case BUTTON_TO_LEFT:
        leftArrowButton_->setChecked(true);
        break;
    case BUTTON_BIDIRECTIONAL:
        bidirectionalButton_->setChecked(true);
        break;
    case BUTTON_TO_RIGHT:
        rightArrowButton_->setChecked(true);
        break;
    }

    setWindowTitle(tr("Edit Property Link"));
    setFixedHeight(sizeHint().height());
}

void PropertyLinkDialog::createWidgets() {
    QString sourceProcName = sourceGraphicsItem_->getProcessorGraphicsItem()->getName();
    QString sourcePropName = QString::fromStdString(sourceGraphicsItem_->getProperty()->getGuiText());
    QString destinProcName = destinationGraphicsItem_->getProcessorGraphicsItem()->getName();
    QString destinPropName = QString::fromStdString(destinationGraphicsItem_->getProperty()->getGuiText());
    QString separator = QString("::");
    QString sourceName = sourceProcName.append(separator).append(sourcePropName);
    QString destinationName = destinProcName.append(separator).append(destinPropName);

    QGridLayout* layout = new QGridLayout(this);

    // setup layout
    layout->setColumnMinimumWidth(1, 75);
    layout->setColumnStretch(0, 1);
    layout->setColumnMinimumWidth(0, 150);
    layout->setColumnStretch(2, 1);
    layout->setColumnMinimumWidth(2, 150);
    layout->setRowMinimumHeight(4, 20);

    QLabel* propertyA = new QLabel(sourceName);
    layout->addWidget(propertyA, 0, 0, 3, 1, Qt::AlignLeft);  // at (0,0) with stretch (3,1)

    QLabel* propertyB = new QLabel(destinationName);
    layout->addWidget(propertyB, 0, 2, 3, 1, Qt::AlignRight);  // at (0,2) with stretch (3,1)

    leftArrowButton_ = new QPushButton(QIcon(leftArrowIcon), "");
    leftArrowButton_->setIconSize(iconSize);
    leftArrowButton_->setCheckable(true);
    leftArrowButton_->setFlat(true);
    leftArrowButton_->setToolTip(tr("Propagation from \"%1\" to \"%2\"").arg(destinationName, sourceName));
    layout->addWidget(leftArrowButton_, 0, 1, 1, 1, Qt::AlignJustify); // at (0,1)

    bidirectionalButton_ = new QPushButton(QIcon(biArrowIcon), "");
    bidirectionalButton_->setIconSize(iconSize);
    bidirectionalButton_->setCheckable(true);
    bidirectionalButton_->setFlat(true);
    bidirectionalButton_->setToolTip(tr("Bidirectional propagation"));
    layout->addWidget(bidirectionalButton_, 1, 1, 1, 1, Qt::AlignJustify); // at (1,1)

    rightArrowButton_ = new QPushButton(QIcon(rightArrowIcon), "");
    rightArrowButton_->setIconSize(iconSize);
    rightArrowButton_->setCheckable(true);
    rightArrowButton_->setFlat(true);
    rightArrowButton_->setToolTip(tr("Propagation from \"%1\" to \"%2\"").arg(sourceName, destinationName));
    layout->addWidget(rightArrowButton_, 2, 1, 1, 1, Qt::AlignJustify); // at (2,1)

    QHBoxLayout* functionLayout = new QHBoxLayout();
    QLabel* functionLabel = new QLabel(tr("Function"));
    functionLayout->addWidget(functionLabel, 0, Qt::AlignLeft);
    functionCB_ = new QComboBox;
    functionCB_->setMinimumWidth(175);
    functionLayout->addWidget(functionCB_, 0, Qt::AlignRight);
    layout->addLayout(functionLayout, 3, 0, 1, 3);

    QWidget* spacing = new QWidget;
    layout->addWidget(spacing, 4, 0, 1, 3);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton(tr("OK"));
    buttonLayout->addWidget(okButton);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout, 5, 0, 1, 3, Qt::AlignRight);

     //connect signals and slots
    connect(functionCB_, SIGNAL(activated(int)), this, SLOT(functionComboBoxChange()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(createPropertyLink()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(leftArrowButton_, SIGNAL(clicked(bool)), this, SLOT(setLeftArrowButtonChecked()));
    connect(rightArrowButton_, SIGNAL(clicked(bool)), this, SLOT(setRightArrowButtonChecked()));
    connect(bidirectionalButton_, SIGNAL(clicked(bool)), this, SLOT(setBidirectionalButtonChecked()));

    connect(this, SIGNAL(closeDialog()), this, SLOT(accept()));
}

void PropertyLinkDialog::setupCombobox() {
    std::list<std::string> availableFunctions = LinkEvaluatorFactory::getInstance()->listFunctionNames();
    foreach (std::string function, availableFunctions)
        functionCB_->addItem(QString::fromStdString(function));

#ifdef VRN_WITH_PYTHON
    functionCB_->addItem(tr("new function"));
#endif

    int index = functionCB_->findText("id");
    functionCB_->setCurrentIndex(index);
}

void PropertyLinkDialog::createPropertyLink(){
    LinkEvaluatorBase* linkEvaluatorBase = 0;
    try {
        std::string function = functionCB_->currentText().toStdString();
        linkEvaluatorBase = LinkEvaluatorFactory::getInstance()->createLinkEvaluator(function);

        if (leftArrowButton_->isChecked() || bidirectionalButton_->isChecked())
            emit create(destinationGraphicsItem_, sourceGraphicsItem_, linkEvaluatorBase);

        if (rightArrowButton_->isChecked() || bidirectionalButton_->isChecked())
            emit create(sourceGraphicsItem_, destinationGraphicsItem_, linkEvaluatorBase);

        emit accept();
    } catch (VoreenException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
    }
}

// Slot for changes of functionComboBox:
void PropertyLinkDialog::functionComboBoxChange() {
    // if "new function" is choosed start the script editor
    if (functionCB_->currentIndex() == functionCB_->findText(tr("new function"))) {
        ScriptEditor* scriptEditor = new ScriptEditor(this);
        connect(scriptEditor, SIGNAL(emitFunction(QString, QString)), this, SLOT(addFunction(QString, QString)));
        scriptEditor->show();
    }
 }

#ifdef VRN_WITH_PYTHON
void PropertyLinkDialog::addFunction(const QString& functionName, const QString& functionBody) {
    if (functionName != "") {
        try {
            LinkEvaluatorFactory::getInstance()->registerLinkEvaluatorPython(functionName.toStdString(),
                                                                             functionBody.toStdString(), false);
            // generate a new function list for the combobox
            functionCB_->clear();
            setupCombobox();
            functionCB_->setCurrentIndex(functionCB_->count() - 2);
        } catch (VoreenException e) {
            functionCB_->setCurrentIndex(0);

            // show the exception
            QErrorMessage* errorMessageDialog = new QErrorMessage(this);
            errorMessageDialog->showMessage(e.what());
        }
    }
    else {
        functionCB_->setCurrentIndex(0);
    }
}
#endif // VRN_WITH_PYTHON

void PropertyLinkDialog::setLeftArrowButtonChecked() {
    bidirectionalButton_->setChecked(false);
    rightArrowButton_->setChecked(false);
}

void PropertyLinkDialog::setRightArrowButtonChecked() {
    bidirectionalButton_->setChecked(false);
    leftArrowButton_->setChecked(false);
}

void PropertyLinkDialog::setBidirectionalButtonChecked() {
    leftArrowButton_->setChecked(false);
    rightArrowButton_->setChecked(false);
}

} // namespace
