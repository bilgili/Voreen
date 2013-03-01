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

#include "voreen/qt/widgets/transfunc/transfunceditor.h"

#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/voreenapplication.h"

#include <QFileDialog>
#include <QUrl>
#include <QDesktopServices>

namespace voreen {

TransFuncEditor::TransFuncEditor(TransFuncProperty* prop, QWidget* parent)
    : QWidget(parent)
    , property_(prop)
    , volume_(0)
{
}

TransFuncEditor::~TransFuncEditor() {
}

void TransFuncEditor::toggleInteractionMode(bool on) {
    property_->toggleInteractionMode(on, this);
}

const QString TransFuncEditor::getOpenFileName(QString filter) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a transfer function to open"));
    fileDialog.setDirectory(VoreenApplication::app()->getResourcePath("transferfuncs").c_str());
    fileDialog.setFilter(filter);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("transferfuncs").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        return fileDialog.selectedFiles()[0];
    }

    return QString();
}

const QString TransFuncEditor::getSaveFileName(QStringList filters) {
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a filename to save transfer function"));
    fileDialog.setDirectory(VoreenApplication::app()->getResourcePath("transferfuncs").c_str());
    fileDialog.setFilters(filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("transferfuncs").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
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

const QString TransFuncEditor::getTitle() {
    return title_;
}

void TransFuncEditor::setTransFuncProp(TransFuncProperty* prop) {
    property_ = prop;
}

} // namespace voreen
