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

#include "voreen/qt/widgets/shaderplugin.h"

#include "tgt/shadermanager.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

namespace voreen {

ShaderPlugin::ShaderPlugin(ShaderProperty* prop, QWidget* parent)
    : QWidget(parent)
    , property_(prop)
{
}

ShaderPlugin::~ShaderPlugin() {
    delete highlighterFrag_;
    delete highlighterVert_;
    delete highlighterGeom_;
}

void ShaderPlugin::createWidgets() {
    updateBt_ = new QToolButton();
    updateBt_->setIcon(QIcon(":/qt/icons/rebuild.png"));
    updateBt_->setIconSize(QSize(24, 24));
    updateBt_->setShortcut(QKeySequence("Ctrl+R"));
    updateBt_->setToolTip("Update shader (Ctrl+R)");
    undoBt_ = new QToolButton();
    undoBt_->setIcon(QIcon(":/qt/icons/revert.png"));
    undoBt_->setIconSize(QSize(24, 24));
    undoBt_->setToolTip("Revert to last loaded source");
    fullUndoBt_ = new QToolButton();
    fullUndoBt_->setIcon(QIcon(":/qt/icons/revert.png"));
    fullUndoBt_->setIconSize(QSize(24, 24));
    fullUndoBt_->setToolTip("Revert to original source");
    openBt_ = new QToolButton();
    openBt_->setIcon(QIcon(":/qt/icons/open.png"));
    openBt_->setIconSize(QSize(24, 24));
    openBt_->setToolTip("Load shader");
    saveBt_ = new QToolButton();
    saveBt_->setIcon(QIcon(":/qt/icons/save.png"));
    saveBt_->setIconSize(QSize(24, 24));
    saveBt_->setToolTip("Export shader");
    fontSizeBox_ = new QSpinBox();
    fontSizeBox_->setMinimum(6);
    fontSizeBox_->setMaximum(24);
    fontSizeBox_->setValue(9);
    fontSizeBox_->setToolTip("Choose font size");

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(updateBt_);
    hbox->addWidget(undoBt_);
    hbox->addWidget(fullUndoBt_);
    hbox->addWidget(openBt_);
    hbox->addWidget(saveBt_);
    hbox->addWidget(fontSizeBox_);
    hbox->addStretch();
    QWidget* toolButtonBar = new QWidget();
    toolButtonBar->setLayout(hbox);

    codeEditFrag_ = new CodeEdit();
    codeEditVert_ = new CodeEdit();
    codeEditGeom_ = new CodeEdit();
    highlighterFrag_ = new GLSLHighlighter(codeEditFrag_->document());
    highlighterVert_ = new GLSLHighlighter(codeEditVert_->document());
    highlighterGeom_ = new GLSLHighlighter(codeEditGeom_->document());

    compilerLogWidget_ = new QTextEdit();
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    compilerLogWidget_->setFont(font);
    compilerLogWidget_->setReadOnly(true);
    compilerLogWidget_->setFixedHeight(150);

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(toolButtonBar);

    tabWidget_ = new QTabWidget(this);
    vbox->addWidget(tabWidget_);

    QWidget* fragTab = new QWidget();
    QWidget* vertTab = new QWidget();
    QWidget* geomTab = new QWidget();

    tabWidget_->addTab(fragTab, tr("Fragment Shader"));
    tabWidget_->addTab(vertTab, tr("Vertex Shader"));
    tabWidget_->addTab(geomTab, tr("Geometry Shader"));

    QVBoxLayout* fragLayout = new QVBoxLayout(fragTab);
    QVBoxLayout* vertLayout = new QVBoxLayout(vertTab);
    QVBoxLayout* geomLayout = new QVBoxLayout(geomTab);

    fragTab->setLayout(fragLayout);
    vertTab->setLayout(vertLayout);
    geomTab->setLayout(geomLayout);
    fragLayout->addWidget(codeEditFrag_);
    vertLayout->addWidget(codeEditVert_);
    geomLayout->addWidget(codeEditGeom_);
    //vbox->addWidget(codeEdit_);

    vbox->addWidget(compilerLogWidget_);
    setLayout(vbox);

    updateFromProperty();
}

void ShaderPlugin::createConnections() {
    connect(undoBt_, SIGNAL(clicked()), this, SLOT(undoShader()));
    connect(fullUndoBt_, SIGNAL(clicked()), this, SLOT(fullUndoShader()));
    connect(openBt_, SIGNAL(clicked()), this, SLOT(openShader()));
    connect(saveBt_, SIGNAL(clicked()), this, SLOT(saveShader()));
    connect(updateBt_, SIGNAL(clicked()), this, SLOT(setProperty()));
    connect(codeEditFrag_, SIGNAL(textChanged()), this, SIGNAL(modified()));
    connect(codeEditVert_, SIGNAL(textChanged()), this, SIGNAL(modified()));
    connect(codeEditGeom_, SIGNAL(textChanged()), this, SIGNAL(modified()));
    connect(fontSizeBox_, SIGNAL(valueChanged(int)), this, SLOT(changeFontSize()));
}

void ShaderPlugin::changeFontSize() {
    codeEditFrag_->updateFontSize(fontSizeBox_->value());
    codeEditVert_->updateFontSize(fontSizeBox_->value());
    codeEditGeom_->updateFontSize(fontSizeBox_->value());
}

const QString ShaderPlugin::getOpenFileName(QString filter) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a shader to open"));
    fileDialog.setDirectory(VoreenApplication::app()->getBasePath().c_str());
    fileDialog.setFilter(filter);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        return fileDialog.selectedFiles()[0];
    }

    return QString();
}

const QString ShaderPlugin::getSaveFileName(QStringList filters) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a filename to save shader"));
    fileDialog.setDirectory(VoreenApplication::app()->getUserDataPath("shaders").c_str());
    fileDialog.setFilters(filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath("shaders").c_str());
    fileDialog.setSidebarUrls(urls);

    QStringList fileList;
    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        QString endingFilter = fileDialog.selectedFilter();
        int pos = endingFilter.lastIndexOf(".");
        //removes closing bracket
        endingFilter.chop(1);
        endingFilter = endingFilter.mid(pos);

        //look whether the user specified an ending
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

void ShaderPlugin::undoShader() {
    int curTab = tabWidget_->currentIndex();
    if(curTab == 0)
        property_->resetFragmentShader();
    if(curTab == 1)
        property_->resetVertexShader();
    else
        property_->resetGeometryShader();
}

void ShaderPlugin::fullUndoShader() {
    int curTab = tabWidget_->currentIndex();
    if(curTab == 0)
        property_->resetFragmentFilename();
    if(curTab == 1)
        property_->resetVertexFilename();
    else
        property_->resetGeometryFilename();
}

void ShaderPlugin::openShader() {
    //create filter with supported file formats
    QString filter;
    int curTab = tabWidget_->currentIndex();
    if(curTab == 0)
        filter = "Fragment Shader (*.frag)";
    else if(curTab == 1)
        filter = "Vertex Shader (*.vert)";
    else
        filter = "Geometry Shader (*.geom)";

    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        if(curTab == 0)
            property_->setFragmentFilename(fileName.toStdString());
        else if(curTab == 1)
            property_->setVertexFilename(fileName.toStdString());
        else
            property_->setGeometryFilename(fileName.toStdString());
    }
}

void ShaderPlugin::saveShader() {
    //create filter with supported file formats
    QStringList filter;
    int curTab = tabWidget_->currentIndex();
    CodeEdit* curEdit;
    if(curTab == 0) {
        filter << "Fragment Shader (*.frag)";
        curEdit = codeEditFrag_;
    }
    else if(curTab == 1) {
        filter << "Vertex Shader (*.vert)";
        curEdit = codeEditVert_;
    }
    else {
        filter << "Geometry Shader (*.geom)";
        curEdit = codeEditGeom_;
    }

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        //save shader to disk
        QFile outputFile(fileName);
        outputFile.open(QIODevice::WriteOnly);
        outputFile.write(curEdit->toPlainText().toStdString().c_str(), curEdit->toPlainText().size());
        outputFile.close();
    }
}

void ShaderPlugin::setProperty() {
    property_->setFragmentSource(codeEditFrag_->toPlainText().toStdString());
    property_->setVertexSource(codeEditVert_->toPlainText().toStdString());
    property_->setGeometrySource(codeEditGeom_->toPlainText().toStdString());
    //force rebuild
    if(property_->getOwner() && property_->getOwner()->getInvalidationLevel() < Processor::INVALID_PROGRAM)
        property_->invalidate();
}

void ShaderPlugin::updateFromProperty() {
    if(codeEditFrag_->toPlainText() != QString(property_->get().fragmentSource_.c_str()))
        codeEditFrag_->setPlainText(property_->get().fragmentSource_.c_str());
    if(codeEditVert_->toPlainText() != QString(property_->get().vertexSource_.c_str()))
        codeEditVert_->setPlainText(property_->get().vertexSource_.c_str());
    if(codeEditGeom_->toPlainText() != QString(property_->get().geometrySource_.c_str()))
        codeEditGeom_->setPlainText(property_->get().geometrySource_.c_str());

    const tgt::ShaderObject* geom = property_->getGeometryObject();
    if(geom)
        compilerLogWidget_->setText(geom->getCompilerLog().c_str());
    const tgt::ShaderObject* vert = property_->getVertexObject();
    if(vert)
        compilerLogWidget_->setText(vert->getCompilerLog().c_str());
    const tgt::ShaderObject* frag = property_->getFragmentObject();
    if(frag)
        compilerLogWidget_->setText(frag->getCompilerLog().c_str());

    std::string mod = "";
    if(property_->get().fragmentModified_)
        mod = "original fragment source: ";

    std::string windowTitle = "";
    if (property_->getOwner())
        windowTitle += property_->getOwner()->getGuiName() + " - ";
    windowTitle += property_->getGuiName() + " (" + mod + property_->get().getCurrentFragmentName() + ")";
    window()->setWindowTitle(QString::fromStdString(windowTitle));
}

} // namespace voreen
