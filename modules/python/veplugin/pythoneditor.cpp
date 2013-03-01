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

// include Python stuff first
#include "../pythonmodule.h"
#include "pythoneditor.h"

#include "tgt/filesystem.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/voreenapplication.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopServices>

namespace voreen {

const std::string PythonEditor::loggerCat_ = "voreen.Python.PythonEditor";

PythonEditor::PythonEditor(QWidget* parent)
    : VoreenVEPlugin(parent)
    , script_(0)
    , highlighter_(0)
    , fontSize_(9)
    , scriptOwner_(false)
{}

PythonEditor::~PythonEditor() {
}

void PythonEditor::setWorkspace(Workspace* workspace) {
    VoreenVEPlugin::setWorkspace(workspace);
}

void PythonEditor::createWidgets() {
    runBt_ = new QToolButton();
    runBt_->setIcon(QIcon(":/modules/python/python.png"));
    runBt_->setIconSize(QSize(24, 24));
    runBt_->setShortcut(QKeySequence("Ctrl+R"));
    runBt_->setToolTip("Run Script (Ctrl+R)");
    newBt_ = new QToolButton();
    newBt_->setIcon(QIcon(":/modules/python/python_2.png"));
    newBt_->setIconSize(QSize(24, 24));
    newBt_->setToolTip("New Script");
    openBt_ = new QToolButton();
    openBt_->setIcon(QIcon(":/qt/icons/open.png"));
    openBt_->setIconSize(QSize(24, 24));
    openBt_->setToolTip("Load Script");
    saveBt_ = new QToolButton();
    saveBt_->setIcon(QIcon(":/qt/icons/save.png"));
    saveBt_->setIconSize(QSize(24, 24));
    saveBt_->setToolTip("Save Script");
    saveAsBt_ = new QToolButton();
    saveAsBt_->setIcon(QIcon(":/qt/icons/saveas.png"));
    saveAsBt_->setIconSize(QSize(24, 24));
    saveAsBt_->setToolTip("Save Script As");

    increaseFontSizeBt_ = new QToolButton();
    increaseFontSizeBt_->setIcon(QIcon(":/qt/icons/viewmag+.png"));
    increaseFontSizeBt_->setIconSize(QSize(24, 24));
    increaseFontSizeBt_->setToolTip("Increase Font Size");
    decreaseFontSizeBt_ = new QToolButton();
    decreaseFontSizeBt_->setIcon(QIcon(":/qt/icons/viewmag_.png"));
    decreaseFontSizeBt_->setIconSize(QSize(24, 24));
    decreaseFontSizeBt_->setToolTip("Decrease Font Size");

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(runBt_);
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    hbox->addWidget(sep);
    hbox->addWidget(newBt_);
    hbox->addWidget(openBt_);
    hbox->addWidget(saveBt_);
    hbox->addWidget(saveAsBt_);
    hbox->addStretch();
    hbox->addWidget(increaseFontSizeBt_);
    hbox->addWidget(decreaseFontSizeBt_);
    QWidget* toolButtonBar = new QWidget();
    toolButtonBar->setLayout(hbox);

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(fontSize_);

    codeEdit_ = new CodeEdit();
    codeEdit_->setFont(font);
    highlighter_ = new PythonHighlighter(codeEdit_->document());

    compilerLogWidget_ = new QTextEdit();
    compilerLogWidget_->setFont(font);
    compilerLogWidget_->setReadOnly(true);
    compilerLogWidget_->setFixedHeight(150);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(toolButtonBar);
    vbox->addWidget(codeEdit_);
    vbox->addWidget(compilerLogWidget_);
    setLayout(vbox);

    connect(runBt_, SIGNAL(clicked()), this, SLOT(runScript()));
    connect(newBt_, SIGNAL(clicked()), this, SLOT(newScript()));
    connect(openBt_, SIGNAL(clicked()), this, SLOT(openScript()));
    connect(saveBt_, SIGNAL(clicked()), this, SLOT(saveScript()));
    connect(saveAsBt_, SIGNAL(clicked()), this, SLOT(saveScriptAs()));
    connect(increaseFontSizeBt_, SIGNAL(clicked()), this, SLOT(increaseFontSize()));
    connect(decreaseFontSizeBt_, SIGNAL(clicked()), this, SLOT(decreaseFontSize()));
    connect(codeEdit_, SIGNAL(textChanged()), this, SIGNAL(modified()));

    setMinimumSize(300, 400);
}

void PythonEditor::initialize() throw (tgt::Exception) {
    VoreenVEPlugin::initialize();

    if (PythonModule::getInstance()) {
        PythonModule::getInstance()->addOutputListener(this);
        newScript();
    }
    else
        LWARNING("Python module class not instantiated");

    updateGuiState();
}

void PythonEditor::deinitialize() throw (tgt::Exception) {
    clearScript();
    if (PythonModule::getInstance()) {
        PythonModule::getInstance()->removeOutputListener(this);
    }
    delete highlighter_;

    VoreenVEPlugin::deinitialize();
}

void PythonEditor::runScript() {
    if (!script_)
        return;

    compilerLogWidget_->setTextColor(Qt::black);
    compilerLogWidget_->clear();

    // retrieve script source from code editor and assign it to script object
    QString source = codeEdit_->toPlainText();
    source = source.replace("\t", "    ");   //< replace tabs with four spaces
    script_->setSource(source.toStdString());

    if (script_->compile(false)) {
        compilerLogWidget_->setPlainText("Running script...");
        setEnabled(false);
        codeEdit_->updateHighlight();
        qApp->setOverrideCursor(Qt::WaitCursor);
        qApp->processEvents();

        bool success = script_->run(false);

        qApp->restoreOverrideCursor();
        setEnabled(true);
        codeEdit_->updateHighlight();
        codeEdit_->setFocus();
        qApp->processEvents();

        if (success) {
            compilerLogWidget_->append("\nfinished.");
            //compilerLogWidget_->setPlainText(compilerLogWidget_->toPlainText() + QString("finished."));
        }
        else {
            compilerLogWidget_->setTextColor(Qt::red);
            compilerLogWidget_->setPlainText("Runtime Error:\n" + QString::fromStdString(script_->getLog()));
            codeEdit_->moveCursorToPosition(script_->getErrorLine() - 1);
        }
    }
    else { // compilation failed
        compilerLogWidget_->setTextColor(Qt::red);
        compilerLogWidget_->setPlainText("Compile Error:\n" + QString::fromStdString(script_->getLog()));
        codeEdit_->moveCursorToPosition(script_->getErrorLine() - 1, script_->getErrorCol() - 1);
    }
}

void PythonEditor::newScript() {
    clearScript();
    if (PythonModule::getInstance() && PythonModule::getInstance()->isInitialized()) {
        script_ = PythonModule::getInstance()->loadScript("template.py", false);
        if (script_) {
            QString source = QString::fromStdString(script_->getSource());
            source = source.replace("\t", "    ");  //< replace tabs with four spaces
            PythonModule::getInstance()->dispose(script_);

            script_ = new PythonScript();
            script_->setSource(source.toStdString());
            codeEdit_->setPlainText(source);
            scriptOwner_ = true;
        }
    }
    else {
        QMessageBox::critical(this, tr("Python Error"), tr("Failed to create Python script: module not initialized"));
    }
    updateGuiState();
}

void PythonEditor::openScript() {

    if (!PythonModule::getInstance()) {
        LERROR("PythonModule not instantiated");
        return;
    }

    //create filter with supported file formats
    QString filter = "Python Script (*.py)";
    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        clearScript();

        script_ = PythonModule::getInstance()->loadScript(fileName.toStdString(), false);
        if (!script_) {
            QMessageBox::critical(this, tr("Python Error"), tr("Python script '%1' could not be loaded.").arg(fileName));
        }
        else {
            // retrieve script source from script object and assign it to code editor
            QString source = QString::fromStdString(script_->getSource());
            source = source.replace("\t", "    ");  //< replace tabs with four spaces
            codeEdit_->setPlainText(source);
            scriptOwner_ = false;
        }
        updateGuiState();
    }
}

void PythonEditor::saveScript() {
    tgtAssert(codeEdit_, "No code editor");

    if (!script_) {
        LERROR("No script");
        return;
    }

    if (script_->getFilename().empty()) {
        LERROR("Script has no filename");
    }
    else {
        if (!saveScriptInternal(QString::fromStdString(script_->getFilename()), codeEdit_->toPlainText())) {
            QString message = tr("Failed to save script to file '%1'").arg(QString::fromStdString(script_->getFilename()));
            QMessageBox::critical(this, tr("Python Error"), message);
        }
    }

    updateGuiState();
}

void PythonEditor::saveScriptAs() {

    if (!PythonModule::getInstance()) {
        LERROR("PythonModule not instantiated");
        return;
    }

    if (!script_) {
        LERROR("No script");
        return;
    }

    //create filter with supported file formats
    QStringList filter;
    filter << "Python Script (*.py)";

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        if (saveScriptInternal(fileName, codeEdit_->toPlainText())) {
            clearScript();
            script_ = PythonModule::getInstance()->loadScript(fileName.toStdString(), false);
            scriptOwner_ = false;
            if (script_) {
                codeEdit_->setPlainText(QString::fromStdString(script_->getSource()));
            }
            else {
                QMessageBox::critical(this, tr("Python Error"), tr("Saved script '%1' could not be loaded.").arg(fileName));
            }
        }
        else {
            QString message = tr("Failed to save script to file '%1'").arg(fileName);
            QMessageBox::critical(this, tr("Python Error"), message);
        }
    }

    updateGuiState();
}

const QString PythonEditor::getOpenFileName(QString filter) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a Python script to open"));
    fileDialog.setDirectory(VoreenApplication::app()->getResourcePath("scripts").c_str());
    fileDialog.setFilter(filter);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("scripts").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        return fileDialog.selectedFiles()[0];
    }

    return QString();
}

const QString PythonEditor::getSaveFileName(QStringList filters) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a filename to save script"));
    fileDialog.setDirectory(VoreenApplication::app()->getUserDataPath("scripts").c_str());
    fileDialog.setFilters(filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("scripts").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    QStringList fileList;
    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        QString endingFilter = fileDialog.selectedFilter();
        int pos = endingFilter.lastIndexOf(".");
        //removes closing bracket
        endingFilter.chop(1);
        endingFilter = endingFilter.mid(pos);

        //look whether the user specified an ending
        std::string fileExtension;
        fileList = fileDialog.selectedFiles();
        size_t dotPosition = fileList[0].toStdString().rfind(".");
        if (dotPosition == std::string::npos) {
            // no ending given -> add ending of selected filter
            fileList[0].append(endingFilter);
        }
        else {
            // an ending was given -> test whether it matches the selected filter
            if (fileList[0].mid(static_cast<int>(dotPosition)) != endingFilter)
                fileList[0].append(endingFilter);
        }
        return fileList[0];
    }
    return QString();
}

bool PythonEditor::saveScriptInternal(QString filename, QString source) {

    // retrieve script source from code editor and replace tabs with four spaces
    source = source.replace("\t", "    ");

    // save script to disk
    QFile outputFile(filename);
    if (outputFile.open(QIODevice::WriteOnly)) {
        outputFile.write(source.toStdString().c_str(), source.size());
        outputFile.close();
        LINFO("Saved script to file '" << filename.toStdString() << "'");
        return true;
    }
    else {
        LERROR("Failed to open file '" << filename.toStdString() << "' for writing");
        return false;
    }

    //updateGuiState(); //cannot be reached
}

void PythonEditor::clearScript() {
    if (!script_)
        return;

    if (!PythonModule::getInstance()) {
        LERROR("PythonModule not instantiated");
        return;
    }

    if (!scriptOwner_ && PythonModule::getInstance())
        PythonModule::getInstance()->dispose(script_);
    else
        delete script_;
    script_ = 0;
    scriptOwner_ = false;

    codeEdit_->setPlainText("");
    compilerLogWidget_->setPlainText("");
}

void PythonEditor::increaseFontSize() {
    fontSize_++;
    updateFont();
}

void PythonEditor::decreaseFontSize() {
   fontSize_ = std::max(fontSize_-1, 7);
   updateFont();
}

void PythonEditor::updateFont() {
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(fontSize_);
    codeEdit_->setFont(font);
    compilerLogWidget_->setFont(font);
}

void PythonEditor::updateGuiState() {
    runBt_->setEnabled(script_);
    saveAsBt_->setEnabled(script_);

    QString title;
    if (script_ && !script_->getFilename().empty()) {
        saveBt_->setEnabled(true);
        title = "Python Script Editor - " + QString::fromStdString(script_->getFilename());
    }
    else {
        saveBt_->setEnabled(false);
        title = "Python Script Editor";
    }
    if (parentWidget() && parentWidget()->parentWidget())
        parentWidget()->parentWidget()->setWindowTitle(title);
    else
        setWindowTitle(title);
}

void PythonEditor::pyStdout(const std::string& out) {
    if (compilerLogWidget_)
        compilerLogWidget_->append(QString::fromStdString(out));
}

void PythonEditor::pyStderr(const std::string& err){
    if (compilerLogWidget_)
        compilerLogWidget_->append(QString::fromStdString(err));
}

} // namespace voreen
