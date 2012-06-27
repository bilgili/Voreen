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

#include "voreen/qt/widgets/network/editor/scripteditor.h"

#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/qt/widgets/tablesstextedit.h"
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QTextStream>
#include <QVBoxLayout>

namespace {
    QSize buttonSize = QSize(25,25);
}

namespace voreen {

/*
 * /--------------------------------------\
 * | New Open Save SaveAs         OK  Exit|
 * |--------------------------------------|
 * | Textfield with Script template       |
 * |                                      |
 * |                                      |
 * |                                      |
 * |                                      |
 * |                                      |
 * \--------------------------------------/
 */

ScriptEditor::ScriptEditor(QWidget* parent)
    : QDialog(parent)
{
    setMinimumWidth(500);
    setMinimumHeight(200);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowModality(Qt::ApplicationModal);

    createWidgets();
    newSlot();
    setWindowTitle();
}

void ScriptEditor::createWidgets() {
    QStyle* style = QApplication::style();

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* newButton = new QPushButton(QIcon(":/voreenve/icons/document-new_32x32.png"), "");
    newButton->setIconSize(buttonSize);
    buttonLayout->addWidget(newButton);

    QPushButton* openButton = new QPushButton(style->standardIcon(QStyle::SP_DialogOpenButton), "");
    openButton->setIconSize(buttonSize);
    buttonLayout->addWidget(openButton);

    QPushButton* saveButton = new QPushButton(style->standardIcon(QStyle::SP_DialogSaveButton), "");
    saveButton->setIconSize(buttonSize);
    buttonLayout->addWidget(saveButton);

    buttonLayout->addSpacing(50);

    QPushButton* okButton = new QPushButton(style->standardIcon(QStyle::SP_DialogApplyButton), "");
    okButton->setIconSize(buttonSize);
    buttonLayout->addWidget(okButton);

    QPushButton* closeButton = new QPushButton(style->standardIcon(QStyle::SP_DialogCloseButton), "");
    closeButton->setIconSize(buttonSize);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    textEdit_ = new TablessTextEdit;
    layout->addWidget(textEdit_);

    connect(newButton, SIGNAL(pressed()), this, SLOT(newSlot()));
    connect(openButton, SIGNAL(pressed()), this, SLOT(openSlot()));
    connect(saveButton, SIGNAL(pressed()), this, SLOT(saveSlot()));
    connect(okButton, SIGNAL(pressed()), this, SLOT(okSlot()));
    connect(closeButton, SIGNAL(pressed()), this, SLOT(closeSlot()));
    connect(textEdit_->document(), SIGNAL(contentsChanged()), this, SLOT(setWindowTitle()));
}

void ScriptEditor::newSlot() {
    if (shouldSave()) {
        textEdit_->document()->clear();

        textEdit_->setPlainText("def scriptName(oldSourceValue, newSourceValue, oldDestinationValue):\n    return newSourceValue");
        textEdit_->document()->setModified(false);
        setWindowTitle();
    }
}

void ScriptEditor::openSlot() {
    if (shouldSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, tr("Application"),
                        tr("Cannot read file %1:\n%2.")
                        .arg(fileName)
                        .arg(file.errorString()));
                return;
            }

            QTextStream in(&file);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            textEdit_->setPlainText(in.readAll());
            QApplication::restoreOverrideCursor();
            setWindowTitle();
        }
    }
}

bool ScriptEditor::saveSlot() {
    QString scriptName = getScriptName() + ".py";
    QString pwd = QString::fromStdString(VoreenApplication::app()->getScriptPath(scriptName.toStdString()));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save script"), pwd);
    if (fileName.isEmpty())
        return false;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot write file %1:\n%2.")
                .arg(fileName)
                .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit_->toPlainText();
    QApplication::restoreOverrideCursor();
    textEdit_->document()->setModified(false);
    setWindowTitle();

    return true;
}

void ScriptEditor::okSlot() {
    if (shouldSave()) {
        QString scriptName = getScriptName();
        if (scriptName.isEmpty())
            QMessageBox::information(this, tr("Missing/wrong function name"), tr("The script must start with the definition of the main function.\nThe name of the function is used as the name of the script"));
        else {
            QString scriptBody = textEdit_->document()->toPlainText();
            emit emitFunction(scriptName, scriptBody);
            close();
        }
    }
}

void ScriptEditor::closeSlot() {
    if (shouldSave()) {
        close();
    }
}

void ScriptEditor::setWindowTitle() {
    QString scriptName = getScriptName();
    QDialog::setWindowTitle(tr("%1[*] - Script Editor").arg(scriptName));
    setWindowModified(textEdit_->document()->isModified());
}

bool ScriptEditor::shouldSave() {
    if (textEdit_->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                tr("The document has been modified.\n"
                    "Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveSlot();
        } else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

QString ScriptEditor::getScriptName() const {
    QString result;
    QString script = textEdit_->document()->toPlainText();

    if (script.startsWith("def ")) {
        int braceIndex = script.indexOf("(");
        if (braceIndex != -1) {
            script = script.left(braceIndex);
            script = script.simplified();
            script = script.remove("def ");

            if (!script.contains(" "))
                return script;
        }
    }

    return "";
}

} // namespace
