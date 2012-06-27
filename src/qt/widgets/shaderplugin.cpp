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
    delete highlighter_;
}

void ShaderPlugin::createWidgets() {
    updateBt_ = new QToolButton();
    updateBt_->setIcon(QIcon(":/icons/rebuild.png"));
    updateBt_->setIconSize(QSize(24, 24));
    updateBt_->setShortcut(QKeySequence("Ctrl+R"));
    updateBt_->setToolTip("Update shader");
    undoBt_ = new QToolButton();
    undoBt_->setIcon(QIcon(":/icons/revert.png"));
    undoBt_->setIconSize(QSize(24, 24));
    undoBt_->setToolTip("Revert to original source");
    openBt_ = new QToolButton();
    openBt_->setIcon(QIcon(":/icons/open.png"));
    openBt_->setIconSize(QSize(24, 24));
    openBt_->setToolTip("Load shader");
    saveBt_ = new QToolButton();
    saveBt_->setIcon(QIcon(":/icons/save.png"));
    saveBt_->setIconSize(QSize(24, 24));
    saveBt_->setToolTip("Export shader");

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(updateBt_);
    hbox->addWidget(undoBt_);
    hbox->addWidget(openBt_);
    hbox->addWidget(saveBt_);
    hbox->addStretch();
    QWidget* toolButtonBar = new QWidget();
    toolButtonBar->setLayout(hbox);

    textEdit_ = new QTextEdit();
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    textEdit_->setFont(font);
	QFontMetrics metrics(font);
	textEdit_->setTabStopWidth(metrics.width(" ")*4);
    textEdit_->setText(QString(property_->getSource().c_str()));
    highlighter_ = new GLSLHighlighter(textEdit_->document());

    compilerLogWidget_ = new QTextEdit();
    compilerLogWidget_->setFont(font);
    compilerLogWidget_->setReadOnly(true);
    compilerLogWidget_->setFixedHeight(150);

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(toolButtonBar);
    vbox->addWidget(textEdit_);
    vbox->addWidget(compilerLogWidget_);
    setLayout(vbox);
}

void ShaderPlugin::createConnections() {
    connect(undoBt_, SIGNAL(clicked()), this, SLOT(undoShader()));
    connect(openBt_, SIGNAL(clicked()), this, SLOT(openShader()));
    connect(saveBt_, SIGNAL(clicked()), this, SLOT(saveShader()));
    connect(updateBt_, SIGNAL(clicked()), this, SLOT(setProperty()));
    connect(textEdit_, SIGNAL(textChanged()), this, SIGNAL(modified()));
}

const QString ShaderPlugin::getOpenFileName(QString filter) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a shader to open"));
    fileDialog.setDirectory(VoreenApplication::app()->getShaderPath().c_str());
    fileDialog.setFilter(filter);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getShaderPath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        return fileDialog.selectedFiles()[0];
    }

    return QString();
}

const QString ShaderPlugin::getSaveFileName(QStringList filters) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a filename to save shader"));
    fileDialog.setDirectory(VoreenApplication::app()->getShaderPath().c_str());
    fileDialog.setFilters(filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getShaderPath().c_str());
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
            if (fileList[0].mid(dotPosition) != endingFilter)
                fileList[0].append(endingFilter);
        }
        return fileList[0];
    }
    return QString();
}

void ShaderPlugin::undoShader() {
    QFile inputFile(QString::fromStdString(ShdrMgr.completePath(property_->getFileName())));
    inputFile.open(QIODevice::ReadOnly);
    QTextStream in(&inputFile);
    QString content = in.readAll();
    inputFile.close();
    textEdit_->setText(content);
    setProperty();
}

void ShaderPlugin::openShader() {
    //create filter with supported file formats
    QString filter = "Shader Program (*.vert *.geom *.frag)";
    QString fileName = getOpenFileName(filter);
    if (!fileName.isEmpty()) {
        QFile inputFile(fileName);
        inputFile.open(QIODevice::ReadOnly);
        QTextStream in(&inputFile);
        QString content = in.readAll();
        inputFile.close();
        textEdit_->setText(content);
        setProperty();
    }

}

void ShaderPlugin::saveShader() {
    //create filter with supported file formats
    QStringList filter;
    filter << "Vertex Shader (*.vert)";
    filter << "Geometry Shader (*.geom)";
    filter << "Fragment Shader (*.frag)";

    QString fileName = getSaveFileName(filter);
    if (!fileName.isEmpty()) {
        //save shader to disk
        QFile outputFile(fileName);
        outputFile.open(QIODevice::WriteOnly);
        outputFile.write(textEdit_->toPlainText().toStdString().c_str(), textEdit_->toPlainText().size());
        outputFile.close();
    }
}

void ShaderPlugin::setProperty() {
    property_->setSource(textEdit_->toPlainText().toStdString());
    compilerLogWidget_->setText(QString(property_->get()->getCompilerLog().c_str()));
}

} // namespace voreen
