/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/networkevaluator.h"
#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/datvolumereader.h"
#include "voreen/core/io/quadhidacvolumereader.h"
#include "voreen/core/io/pvmvolumereader.h"
#include "voreen/core/io/zipvolumereader.h"

#ifdef VRN_WITH_DCMTK
#include "voreen/core/io/dicomvolumereader.h"
#include "voreen/qt/dicomdialog.h"
#endif

#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/volume/volumeatomic.h"

#ifndef VRN_VOLUMESETWIDGET_H
#include "voreen/qt/widgets/volumesetwidget.h"
#endif

#include "voreen/qt/widgets/widgetplugin.h"
#include "voreen/qt/widgets/canvasmodifier.h"
#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/widgets/informationplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncplugin.h"
#include "voreen/qt/widgets/plugindialog.h"
#include "voreen/qt/widgets/orientationplugin.h"
#include "voreen/qt/widgets/showtexcontainerwidget.h"
#include "voreen/qt/widgets/segmentationplugin.h"
#ifdef VRN_MODULE_DEFORMATION
#include "voreen/modules/deformation/deformationplugin.h"
#endif
#include "voreen/qt/widgets/transfunc/transfuncalphaplugin.h"
#include "voreen/qt/widgets/pickingplugin.h"
#include "voreen/qt/widgets/widgetgenerator.h"


#include "cmdlineparser.h"

namespace voreen {
    class DatasetServer;
    class MultiViewWidget;
    class TrackballNavigation;
    class OverViewWidget;
    class VolumeSerializer;
    class VolumeSerializerPopulator;
    class VoreenPainter;
    class TransFuncPlugin;
}

class CanvasMsgProcessor;
class IOSystem;
class MyQDockWidget;

class VoreenMainFrame : public QMainWindow, public voreen::MessageReceiver {
    Q_OBJECT
public:
    VoreenMainFrame(QStringList* args = 0, QWidget *parent = 0);
    ~VoreenMainFrame();

    void init();
    void deinit();
    virtual void processMessage(voreen::Message* msg, const voreen::Identifier& dest = voreen::Message::all_);

    voreen::TextureContainer* getTextureContainer();
    CmdLineParser& getCmdLineParser() { return cmdLineParser_; }

public slots:
/*
    void fileOpen(const QString& fileName, bool add = false,
                  const voreen::Modality& forceModality = voreen::Modality::MODALITY_UNKNOWN);
    void fileAddDataset();
*/
    void fileOpen(const QString& fileName, const voreen::Modality& forceModality = voreen::Modality::MODALITY_UNKNOWN);
    void dicomDirFinished();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void openNetworkFile();
    void fileOpen();
    void fileOpenDicomDir();
    void fileOpenDicomFiles();
    void fileExit();
    void helpAbout();
    void selectView(QAction* action);
    void rebuildShaders();
    void showTextureContainer(bool enable);
    void runScript();
    void showVolumeMapping();
    void connectCanvasModifier(bool connect);

private:
    enum {
        MAIN_VIEW,
        MULTI_VIEW,
        OVER_VIEW,
        NUM_VIEWS
    };

    void clearNetwork();

    void createActions();
    void createMenus();

    //void showDatasetInfo(voreen::Volume* dataset, QString fileName);

    // TODO: eliminate version returning VolumeContainer* and remove dummy bool from
    // other version.
    //
    //voreen::VolumeContainer* loadVolumes(const QStringList& fileNames);

    // TODO: eliminate version taking VolumeContainer* and remove dummy bool
    //
    //void finishOpen(voreen::VolumeContainer* newVolcont, const QString& fileName, bool add);
    //void finishOpen(voreen::Volume* volume, const QString& fileName);

    QDir fileDialogDir_;

    bool getFileDialog(QStringList& filenames, QDir& dir);

    CmdLineParser cmdLineParser_;
    MessageReceiver* msgRcv_;

    std::vector<voreen::TransFuncAlphaPlugin*> tfAlphas_;

private:

    static const std::string loggerCat_;

    QToolBox* toolBox_;

#ifdef VRN_WITH_DCMTK
    voreen::DicomDirDialog* dicomDirDialog_;
#endif

    tgt::Camera* camera_;
    tgt::Trackball* trackball_;
    tgt::QtCanvas* canvas3D_;
    voreen::CanvasModifier* canvasMod_;
	voreen::NetworkEvaluator* evaluator_;
	voreen::NetworkSerializer* networkSerializer_;

    voreen::PluginDialog* snapshotDialog_;
    voreen::PluginDialog* animationDialog_;
    voreen::PluginDialog* lightMaterialDialog_;
    voreen::PluginDialog* stereoscopyDialog_;
    voreen::PluginDialog* segmentationDialog_;
#ifdef VRN_MODULE_DEFORMATION
    voreen::PluginDialog* deformationDialog_;
#endif
    voreen::PluginDialog* pickingDialog_;
    voreen::PluginDialog* sketchDialog_;

    QDockWidget* infoDock_;

    QVector<voreen::WidgetPlugin*>* widgetPlugins_;

    voreen::InformationPlugin* infoPlugin_;
    voreen::TransFuncPlugin* transferFuncPlugin_;
    voreen::PluginDialog* orientationDialog_;
    voreen::PickingPlugin* pickingPlugin_;

    voreen::OverViewWidget* overView_;
    voreen::VoreenPainter* painter_;

    MyQDockWidget* canvasDock_;
    QStackedWidget* stackedWidget_;

    voreen::WidgetGenerator* guiGen_;

    QMenu* fileMenu_;
    QMenu* dicomMenu_;
    QMenu* helpMenu_;
    QMenu* toolsMenu_;
    QMenu* optionsMenu_;
    QMenu* viewsMenu_;

    QAction* openAct_;
    QAction* openDicomDirAct_;
  	QAction* openDicomFilesAct_;
    QAction* addDatasetAct_;
    QAction* quitAct_;
    QAction* aboutAct_;

    QAction* connectCanvasModAct_;
    QAction* infoAction_;
    QAction* orientationAction_;
    QAction* snapshotAction_;
    QAction* animationAction_;
    QAction* lightMaterialAction_;
    QAction* rebuildShadersAction_;
    QAction* textureContainerAction_;
    QAction* segmentationAction_;
#ifdef VRN_MODULE_DEFORMATION
    QAction* deformationAction_;
#endif
    QAction* scriptAction_;
    QAction* volumeMappingAction_;

    QAction* openNetworkFileAction_;

    QAction* viewToolActions_[NUM_VIEWS];

    QToolBox* pipelineBox_;

    voreen::TextureContainer* tc_;
    voreen::ShowTexContainerWidget* showTexContainer_;

    voreen::TrackballNavigation* trackNavi_;

    CanvasMsgProcessor* canvasMsgProcessor_;
    IOSystem* ioSystem_;
    voreen::VolumeSerializer* volumeSerializer_;
    voreen::VolumeSerializerPopulator volumeSerializerPopulator_;
    QVector<voreen::WidgetPlugin*>* pipelinePlugins_;

    voreen::GeometryContainer* gc_;
    voreen::VolumeSetContainer* volumeSetContainer_;
    voreen::VolumeSetWidget* volumeSetWidget_;

    QString networkPath_;

#ifdef VRN_WITH_DATASETSERVER
    voreen::DatasetServer* datasetServer_;
#endif // VRN_WITH_DATASETSERVER
};

