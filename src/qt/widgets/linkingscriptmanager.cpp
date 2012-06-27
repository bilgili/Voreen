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

#include "voreen/qt/widgets/linkingscriptmanager.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/qt/widgets/codeedit.h"
#include "voreen/qt/widgets/pythonhighlighter.h"

#include <QGridLayout>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>

namespace {
    const int codeEditorMinimumWidth = 350;
    const int listMinimumWidth = 200;
    const int listMaximumWidth = 200;
    const QSize iconSize = QSize(20,20);
    const QString emptyScript = "def scriptName(oldSourceValue, newSourceValue, oldDestinationValue):\n    return newSourceValue";
}

namespace voreen {

LinkingScriptManager::LinkingScriptManager(QWidget* parent)
    : QWidget(parent)
    , scriptTitle_(0)
    , listWidget_(0)
    , codeEditorWidget_(0)
    , highlighter_(0)
{
    createChildren();
    initListWidget();
}

LinkingScriptManager::~LinkingScriptManager() {
    delete highlighter_;
    highlighter_ = 0;
}

void LinkingScriptManager::createChildren() {
    QGridLayout* layout = new QGridLayout(this);

    listWidget_ = new QListWidget;
    listWidget_->setMinimumWidth(listMinimumWidth);
    listWidget_->setMaximumWidth(listMaximumWidth);
    connect(listWidget_, SIGNAL(currentTextChanged(const QString&)), this, SLOT(listSelectionChanged(const QString&)));
    layout->addWidget(listWidget_, 0, 0, 2, 1);

    scriptTitle_ = new QLabel;
    layout->addWidget(scriptTitle_, 0, 1);

    codeEditorWidget_ = new CodeEdit;
    codeEditorWidget_->setMinimumWidth(codeEditorMinimumWidth);
    codeEditorWidget_->setTabStopWidth(0);
    highlighter_ = new PythonHighlighter(codeEditorWidget_->document());
    connect(codeEditorWidget_, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    layout->addWidget(codeEditorWidget_, 1, 1);

    QPushButton* confirmButton = new QPushButton(tr("Confirm"));
    connect(confirmButton, SIGNAL(clicked(bool)), this, SLOT(confirmScript()));
    layout->addWidget(confirmButton, 2, 1, Qt::AlignRight);

    QWidget* buttonContainer = new QWidget;
    QBoxLayout* buttonLayout = new QHBoxLayout(buttonContainer);
    QPushButton* newButton = new QPushButton(QIcon(":/voreenve/icons/document-new_32x32.png"), "");
    newButton->setToolTip(tr("Create new script"));
    newButton->setIconSize(iconSize);
    connect(newButton, SIGNAL(clicked(bool)), this, SLOT(newScript()));
    buttonLayout->addWidget(newButton);

    QPushButton* openButton = new QPushButton(QIcon(":/icons/open.png"), "");
    openButton->setToolTip(tr("Open script"));
    openButton->setIconSize(iconSize);
    connect(openButton, SIGNAL(clicked(bool)), this, SLOT(openScript()));
    buttonLayout->addWidget(openButton);

    QPushButton* saveButton = new QPushButton(QIcon(":/icons/save.png"), "");
    saveButton->setToolTip(tr("Save script"));
    saveButton->setIconSize(iconSize);
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(saveScript()));
    buttonLayout->addWidget(saveButton);

    layout->addWidget(buttonContainer, 2, 0);
}

void LinkingScriptManager::rebuildScriptList() {
    initListWidget();
}

void LinkingScriptManager::initListWidget() {
    listWidget_->clear();
    LinkEvaluatorFactory* factory = LinkEvaluatorFactory::getInstance();
    std::vector<std::string> list = factory->getScriptManager()->listPythonFunctionNames();

    foreach (std::string s, list)
        listWidget_->addItem(QString::fromStdString(s));

    if (listWidget_->count() > 0)
        listWidget_->setCurrentRow(0);
}

void LinkingScriptManager::newScript() {
    setScript(emptyScript);
}

void LinkingScriptManager::openScript() {
    QString scriptPath = QString::fromStdString(VoreenApplication::app()->getScriptPath());

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open script..."), QDir(scriptPath).absolutePath(), "Python scripts (*.py)");
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        QString fileContent = stream.readAll();
        setScript(fileContent);
        file.close();
    }
}

void LinkingScriptManager::saveScript() {
    QString scriptPath = QString::fromStdString(VoreenApplication::app()->getScriptPath());

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save script..."), QDir(scriptPath).absolutePath(), "Python scripts (*.py)");
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << getScript();
        file.close();
    }
}

void LinkingScriptManager::confirmScript() {
    QString name = getScriptName();
    QString script = getScript();

    if (name == "") {
        QMessageBox msgBox;
        msgBox.setText(tr("No function has been defined in the script"));
        msgBox.exec();
    } else {
        LinkEvaluatorFactory* factory = LinkEvaluatorFactory::getInstance();
        factory->registerLinkEvaluatorPython(name.toStdString(), script.toStdString(), true);
        initListWidget();
    }
}

void LinkingScriptManager::listSelectionChanged(const QString& currentText) {
    if (currentText == "")
        return;

    LinkEvaluatorFactory* factory = LinkEvaluatorFactory::getInstance();

    LinkEvaluatorBase* base = factory->createLinkEvaluator(currentText.toStdString());
    LinkEvaluatorPython* python = static_cast<LinkEvaluatorPython*>(base);
    tgtAssert(python, "LinkEvaluatorBase was no LinkEvaluatorPython");
    setScript(QString::fromStdString(python->getScript()));
}

void LinkingScriptManager::scriptTextChanged() {
    QString function = getScriptName();
    scriptTitle_->setText(function);
}

QString LinkingScriptManager::getScriptName() {
    QString script = getScript();
    QString result;

    int defIndex = script.indexOf("def ");
    if (defIndex != -1) {
        script = script.remove(0, defIndex);    // truncate the beginning of the script up until the "def "
        int braceIndex = script.indexOf("(");
        if (braceIndex != -1) {
            script = script.left(braceIndex);   // returns the text beginning with "def " and ending just before "("
            script = script.simplified();       // removes leading and trailing whitespaces
            script = script.remove("def ");
            script = script.simplified();       // just in case someone wrote "def  name("

            if (!script.contains(" "))
                return script;
        }
    }
    return "";
}

void LinkingScriptManager::setScript(QString script) {
    codeEditorWidget_->setPlainText(script);
}

QString LinkingScriptManager::getScript() {
    return codeEditorWidget_->document()->toPlainText();
}

} // namespace
