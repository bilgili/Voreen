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

#ifndef VRN_VOREENMAINFRAME_H
#define VRN_VOREENMAINFRAME_H

#ifdef WIN32
#include <windows.h>
#endif

#include <list>
#include <sstream>

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenuBar>
#include <QStackedWidget>
#include <QToolBox>
#include <QDockWidget>

#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/volume/modality.h"

#include "tgt/qt/qtcanvas.h"

namespace voreen {

class CommandlineParser;
class TextureContainer;
class Volume;
class VolumeSerializer;
class VoreenPainter;

class InformationPlugin;
class WidgetPlugin;

class VoreenMainframe : public QMainWindow, public voreen::MessageReceiver {
    Q_OBJECT
public:

    VoreenMainframe(QWidget *parent, std::string name);
    virtual ~VoreenMainframe();

    virtual void init(QStringList args);
    virtual void deinit() {}
    virtual void processMessage(voreen::Message* msg, const voreen::Identifier& dest = voreen::Message::all_);

protected:

    void closeEvent(QCloseEvent *event);

    virtual void createActions();
    virtual void createMenus();
    virtual void initializeWidgets();
    virtual void initializeRenderer();

    void showDatasetInfo(voreen::Volume* dataset, QString fileName);

    bool getFileDialog(QStringList& filenames, QDir& dir);

    void fileOpen(const QStringList& fileNames, bool add = false);

    const std::string loggerCat_;
    CommandlineParser* cmdLineParser_;

    tgt::Camera* camera_;
    tgt::QtCanvas* canvas3D_;
    VoreenPainter* painter_;

    voreen::InformationPlugin* infoPlugin_;

    voreen::VolumeSerializer* volumeSerializer_;
    voreen::VolumeSerializerPopulator volumeSerializerPopulator_;
    voreen::TextureContainer* tc_;

    QStringList filters_;

    QToolBox* toolBox_;
    QDir fileDialogDir_;
    QVector<voreen::WidgetPlugin*>* widgetPlugins_;

    QMenu* fileMenu_;
    QMenu* helpMenu_;
    QMenu* toolsMenu_;
    QMenu* optionsMenu_;

    QAction* openAct_;
    QAction* exportAct_;
    QAction* exitAct_;
    QAction* aboutAct_;
    QAction* infoAction_;

protected slots:

    virtual void fileOpen();
    virtual void fileExport();
    virtual void fileExit();
    virtual void helpAbout();
};

}

#endif // VRN_VOREENMAINFRAME_H
