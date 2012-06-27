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

// ensure python support is initialized first
#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

#include "mainframe.h"

#include "tgt/gpucapabilities.h"
#include "tgt/qt/qttimer.h"

#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/exception.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/networkanalyzer.h"
#include "voreen/core/vis/trackballnavigation.h"
#include "voreen/core/vis/processors/benchmark.h"

#include "voreen/qt/aboutbox.h"
#include "voreen/qt/widgets/overviewwidget.h"
#include "voreen/qt/widgets/snapshotplugin.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/lightmaterialplugin.h"
#include "voreen/qt/widgets/stereoplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensityplugin.h"
#include "voreen/core/geometry/geometrycontainer.h"

#include <QMessageBox>

#include "tgt/init.h"

using namespace voreen;

//---------------------------------------------------------------------------

/**
 * class can be removed and functions ported to mainframe, if messageReceiver_
 * are ported to MsgDistr in the plugins/widgets
 */
class CanvasMsgProcessor : public MessageReceiver {
public:
    CanvasMsgProcessor(tgt::QtCanvas* canvas)
        : MessageReceiver("canvasMsgProc")
        , myCanvas_(canvas){ }

    void processMessage(Message* msg, const Identifier& /*dest=Message::all_*/) {
        if (msg->id_ == "canvas.scheduleRepaint")
            myCanvas_->update();
    }

private:
    tgt::QtCanvas* myCanvas_;
};

//---------------------------------------------------------------------------

const std::string VoreenMainFrame::loggerCat_("voreendev.Mainframe");


VoreenMainFrame::VoreenMainFrame(QStringList* args, QWidget *parent)
    : QMainWindow(parent)
    , MessageReceiver("mainframe")
    , cmdLineParser_(args)
    , showTexContainer_(0)
    , volumeSetContainer_(0)
    , volumeSetWidget_(0)
{
    volumeSetContainer_ = new VolumeSetContainer();
    volumeSetWidget_ = new VolumeSetWidget(volumeSetContainer_, 0, VolumeSetWidget::LEVEL_ALL, true);
    overView_ = 0;

    QApplication::setOrganizationName("Voreen");
    QApplication::setOrganizationDomain("voreen.org");
    QApplication::setApplicationName("VoreenDev");

    setMinimumSize(1024, 768);
#ifndef __APPLE__
    move(0, 0);
#endif
    setWindowTitle(tr("Voreen - Development Environment"));
    setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));

    QSettings settings;
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(1024, 768)).toSize());
    move(settings.value("pos", QPoint(0, 0)).toPoint());
    settings.endGroup();
    settings.beginGroup("Paths");
    networkPath_ = settings.value("network", "../../data/networks").toString();
    QString datasetPath = settings.value("dataset", "../../data").toString();
    settings.endGroup();

    volumeSetWidget_->setCurrentDirectory(datasetPath.toStdString());
    
    // initialize canvases
    canvas3D_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true, 0);
    canvas3D_->setUpdatesEnabled(false); // disable repaints until GUI is fully initialized
    canvas3D_->setFocusPolicy(Qt::ClickFocus);

    canvasMod_ = new CanvasModifier(canvas3D_);

    trackball_ = 0;

    canvasDock_ = new QDockWidget(tr("Views"), this);
    canvasDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    stackedWidget_ = new QStackedWidget(canvasDock_);
    canvasDock_->setWidget(stackedWidget_);
    setCentralWidget(canvasDock_);

    stackedWidget_->addWidget(canvas3D_);

    gc_ = new voreen::GeometryContainer();
}

VoreenMainFrame::~VoreenMainFrame() {
    delete orientationDialog_;

    delete volumeSetWidget_;
    delete volumeSetContainer_;

    delete transferFuncPlugin_;
    delete camera_;
    delete canvasMsgProcessor_;
    delete trackNavi_;
    delete pipelinePlugins_;
    delete painter_;
    delete canvas3D_;
	delete evaluator_;
    delete guiGen_;
    delete gc_;

//     for (size_t i = 0; i < NUM_VIEWS; ++i)
//          delete viewToolActions_[i];
}

void VoreenMainFrame::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
    MessageReceiver::processMessage(msg, dest);
    MsgDistr.processMessage(msg, dest);
    
	if (msg->id_ == "main.dataset.load")
        //fileOpen(msg->getValue<std::string>().c_str(), false);
        fileOpen(msg->getValue<std::string>().c_str());
    else if (msg->id_ == StereoPlugin::setStereoMode_)
        painter_->setStereoMode(msg->getValue<int>());
    else if (msg->id_ == StereoPlugin::setEyeDistance_)
        camera_->setEyeSeparation(msg->getValue<int>());
    else if (msg->id_ == StereoPlugin::setFocalDistance_)
        camera_->setFocalLength(static_cast<float>(msg->getValue<int>()));
}

void VoreenMainFrame::init() {

    MsgDistr.setAllowRepaints(false); // disallow all repaints until fully initialized
                                      // prevents some bad GFX driver crashes on Linux

    camera_ = new tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.75f),
		                      tgt::vec3(0.0f, 0.0f, 0.0f),
                              tgt::vec3(0.0f, 1.0f, 0.0f));

    // Or we could use an orthographic camera:
//     camera_ = new tgt::OrthographicCamera(tgt::vec3(0.0f, 0.0f, 3.75f),
//                                           tgt::vec3(0.0f, 0.0f, 0.0f),
//                                           tgt::vec3(0.0f, 1.0f, 0.0f),
//                                           -1.f, 1.f, -1.f, 1.f, -10.f, 10.f);

    
    canvas3D_->setCamera(camera_);

    tgt::EventHandler* timeHandler = new tgt::EventHandler();
    trackball_ = new tgt::Trackball(canvas3D_, false, new tgt::QtTimer(timeHandler));
    trackball_->setCenter(tgt::vec3(0.f));

    painter_ = new VoreenPainter(canvas3D_, trackball_, "mainview");

    MsgDistr.insert(painter_);

    // insert canvasMsgProcessor in MsgDistr
    canvasMsgProcessor_ = new CanvasMsgProcessor(canvas3D_);
    MsgDistr.insert(canvasMsgProcessor_);

    // set initial orientation to coronal view
    float c = 0.5f * sqrtf(2.f);
    tgt::quat q = tgt::quat(c, 0.f, 0.f, c);
    trackball_->rotate(q);

    trackNavi_ = new TrackballNavigation(trackball_, true, 0.05f, 15.f);
    timeHandler->addListenerToFront(trackNavi_);

    ShdrMgr.addPath("../../src/core/vis/glsl");
    int finalTarget = 20;

    evaluator_ = new NetworkEvaluator();
    
    tc_ = evaluator_->initTextureContainer(finalTarget);

    IDManager id1;
	id1.setTC(tc_);
    
    ProcessorFactory::getInstance()->setTextureContainer(tc_);
    evaluator_->setGeometryContainer(gc_);

    ProcessBenchmark* bench = new ProcessBenchmark();
//     bench->addProcessorName("SimpleRaycaster");
//     bench->addProcessorName("SingleVolumeRaycaster");
    evaluator_->addProcessWrapper(bench);
	
    //FIXME: where is tested whether the given network works on current graphics system? (cdoer)
	networkSerializer_ = new NetworkSerializer();
   
    ProcessorNetwork net;
    try {
        net = networkSerializer_->readNetworkFromFile(cmdLineParser_.getNetworkName());
    }
    catch (SerializerException) { // Makes more sense to not catch at all...
        LERROR("Failed to read initial network from file '"
               << cmdLineParser_.getNetworkName() << "'");
        exit(1);
    }
    
	std::vector<Processor*> processors = net.processors;
    for (size_t i=0; i < processors.size(); i++)
		processors[i]->setCamera(camera_);
    
	evaluator_->setProcessors(processors);
	MsgDistr.insert(evaluator_);
	evaluator_->analyze();

    // need to show error later, when GUI is ready
    bool evaluateSuccessful = painter_->setEvaluator(evaluator_);

    //read conditioned and grouped Properties
	std::pair<std::vector<ConditionProp*>, std::vector<GroupProp*> > props = NetworkAnalyzer::findCondProps(processors);
	NetworkAnalyzer::markCondProps(processors, props.first, props.second);

    canvas3D_->getEventHandler()->addListenerToBack(trackNavi_);
    MsgDistr.insert(trackNavi_);

    if (cmdLineParser_.getMaximized())
        setWindowState(windowState() | Qt::WindowMaximized);

    // initialize widget plugins and arrange inside toolbox
    QDockWidget* toolDock = new QDockWidget(tr("Transfer Function"), this);
    toolDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    toolDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    QDockWidget* pipelineDock = new QDockWidget(tr("Visualization Setup"), this);
    pipelineDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    
    pipelineBox_ = new QToolBox(pipelineDock);
    pipelinePlugins_ = new QVector<WidgetPlugin*>();
    guiGen_ = new WidgetGenerator(0, this);
	guiGen_->createAllWidgets(evaluator_->getProcessors(), painter_->getOverlayMgr());
    pipelinePlugins_->append(guiGen_);
    for (int i=0; i < pipelinePlugins_->size(); i++) {
        WidgetPlugin* curPlugin = pipelinePlugins_->at(i);
        curPlugin->createWidgets();
        curPlugin->createConnections();
        pipelineBox_->addItem(curPlugin, curPlugin->getIcon(), curPlugin->objectName());
    }
	pipelineDock->setWidget(pipelineBox_);
	pipelineDock->setMinimumWidth(300);
    addDockWidget(Qt::RightDockWidgetArea, pipelineDock);
    //tfAlphas_ = guigen_->getTransferAlphaPlugins();

    transferFuncPlugin_ = new TransFuncPlugin(toolDock, painter_);
    transferFuncPlugin_->createWidgets();
    transferFuncPlugin_->createConnections();
	transferFuncPlugin_->setEvaluator(evaluator_);
    toolDock->setWidget(transferFuncPlugin_);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    // FIXME: transferFuncPlugin has its own canvas, that is current after the initialization
    // this leads to problems with texture id generation after initialization of
    // TransFuncIntensityGradientPlugin (see there line 43)
    canvas3D_->makeCurrent();

    // multi view creation
    //multiView_  = new MultiViewWidget(factory, rendererType, camera_,
    //    (tgt::EventListener*)(transferFuncPlugin_->getIntensityPlugin()), stackedWidget_);
    //// set color processing mode to 'no shading' for multiview 3D renderer
    //MsgDistr.postMessage( new StringMsg(FancyRaycaster::setShadingMode_, "No Shading"), "mv1" );
    //// disable clipping for multiview 3D view
    //MsgDistr.postMessage( new BoolMsg(ProxyGeometry::setUseClipping_, false), "mv1" );
    //stackedWidget_->addWidget(multiView_);

    // overview
    //overView_ = new OverViewWidget((tgt::EventListener*)(transferFuncPlugin_->getIntensityPlugin()), stackedWidget_);
    //stackedWidget_->addWidget(overView_);

    OrientationPlugin* orientationPlugin = new OrientationPlugin(this, this, canvas3D_, trackball_, tc_);
    trackNavi_->addReceiver((MessageReceiver*)orientationPlugin);
	//orientationPlugin->disableFeatures(OrientationPlugin::ORIENTATION_OVERLAY);
    orientationDialog_ = new PluginDialog(orientationPlugin);
    addDockWidget(Qt::RightDockWidgetArea, orientationDialog_);

    LightMaterialPlugin* lightMaterialPlugin = new LightMaterialPlugin(this, this);
    // only the light source luminances and the material shininess are presented to the user
    lightMaterialPlugin->disableFeatures(LightMaterialPlugin::ALL_FEATURES);
    lightMaterialPlugin->enableFeatures(LightMaterialPlugin::LIGHT_AMBIENT | 
                                        LightMaterialPlugin::LIGHT |
                                        LightMaterialPlugin::LIGHT_DIFFUSE |
                                        LightMaterialPlugin::LIGHT_SPECULAR |
                                        LightMaterialPlugin::MATERIAL |
                                        LightMaterialPlugin::MATERIAL_SPECULAR );
    lightMaterialDialog_ = new PluginDialog(lightMaterialPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, lightMaterialDialog_);

    SnapshotPlugin* snapshotPlugin = new SnapshotPlugin(this, painter_);
    snapshotDialog_ = new PluginDialog(snapshotPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, snapshotDialog_);

    AnimationPlugin *animationPlugin = new AnimationPlugin(this, camera_, canvas3D_);
    animationDialog_ = new PluginDialog(animationPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, animationDialog_);

#ifdef VRN_MODULE_DEFORMATION
    DeformationPlugin* deformplugin = new DeformationPlugin(0, this, canvas3D_, painter_);
    deformationDialog_ = new PluginDialog(deformplugin, this);
    addDockWidget(Qt::RightDockWidgetArea, deformationDialog_);
#endif
    ////pickingPlugin
    //pickingPlugin_ = new PickingPlugin(this,this);
    //pickingDialog_ = new PluginDialog(pickingPlugin_, this, false);


    //if (rendererType == RendererFactory::VRN_PICKINGRAYCASTING){
    //    pickingPlugin_->setTextureContainer(tc_);
    //    canvas3D_->getEventHandler()->addListenerToFront((tgt::EventListener*)pickingPlugin_);
    //    addDockWidget(Qt::RightDockWidgetArea, pickingDialog_);
    //}

    canvas3D_->getEventHandler()->addListenerToFront( (tgt::EventListener*)(transferFuncPlugin_) );

    createActions();
    createMenus();
    QToolBar* fileToolBar = new QToolBar(tr("File Toolbar"), this);
    QToolBar* toolsToolBar = new QToolBar(tr("Utilities Toolbar"), this);
    QToolBar* optionsToolBar = new QToolBar(tr("Options Toolbar"), this);
//     QToolBar* viewToolBar = new QToolBar(tr("View Toolbar"), this);

    fileToolBar->addAction(openAct_);

    toolsToolBar->addAction(snapshotAction_);
    toolsToolBar->addAction(animationAction_);

    optionsToolBar->addAction(orientationAction_);
    optionsToolBar->addAction(lightMaterialAction_);
#ifdef VRN_MODULE_DEFORMATION
    toolsToolBar->addAction(deformationAction_);
#endif
    toolsToolBar->addAction(scriptAction_);

    toolsToolBar->addAction(rebuildShadersAction_);
    toolsToolBar->addAction(textureContainerAction_);

//     QActionGroup* ag = new QActionGroup(this);
//     for (int i = 0; i < NUM_VIEWS; ++i) {
//         viewToolActions_[i]->setData(i);
//         viewToolActions_[i]->setCheckable(true);
//         ag->addAction(viewToolActions_[i]);
//         viewToolBar->addAction(viewToolActions_[i]);
//     }
//     ag->setExclusive(true);
//     viewToolActions_[0]->setChecked(true);

//     connect(viewToolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(selectView(QAction*)));

    addToolBar(fileToolBar);
    addToolBar(toolsToolBar);
    addToolBar(optionsToolBar);
//     addToolBar(viewToolBar);
    statusBar();

    //FIXME: special handling needed for canvas3D_, seems not to be in MsgDistr... b0rk b0rk
    canvas3D_->getGLFocus();
    lightMaterialPlugin->initGLState();

    MsgDistr.setAllowRepaints(true); // all initialized, allow repaints via messages

    transferFuncPlugin_->updateTransferFunction();

    if (!cmdLineParser_.getTransFuncFileName().isEmpty()) {
        QString fn = cmdLineParser_.getTransFuncFileName();
        if (fn.endsWith(".tf") || fn.endsWith(".TF"))
            transferFuncPlugin_->getIntensityPlugin()->readFromDisc(fn.toStdString());
        else if (fn.endsWith(".tfi") || fn.endsWith(".TFI")) {
            TransFuncIntensity* tf = new TransFuncIntensity();
            tf->load(fn.toStdString());
            transferFuncPlugin_->getIntensityPlugin()->setTransFunc(tf);
        }
        else {
            LERROR(fn.toStdString());
            LERROR("Only .tf and .tfi transfer-function-formats are supported for now.");
        }
    }

    canvas3D_->startTimer(10);
    canvas3D_->setUpdatesEnabled(true); // everything is initialized, now allow repaints

    // Load dataset specified as program arguments
    if (!cmdLineParser_.getFileName().isEmpty()) {

        if (!cmdLineParser_.getSegmentationFileName().isEmpty()) {
            // segmentation: first load the segmentation and force the correct modality,
            // afterwards load the main file
            VolumeSet* volset = volumeSetWidget_
                ->loadVolumeSet(cmdLineParser_.getSegmentationFileName().toStdString());
            volset->forceModality(Modality::MODALITY_SEGMENTATION);
            std::vector<std::string> v;
            v.push_back(cmdLineParser_.getFileName().toStdString());
            volumeSetWidget_->addVolumeSeries(v, volset);
            postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);            
        } else {
            // just a simple file
            fileOpen(cmdLineParser_.getFileName());
        }
    }

    if (cmdLineParser_.getCanvasWidth() > 0 && cmdLineParser_.getCanvasHeight() > 0) {
        LINFO("Canvas size set to "
              << cmdLineParser_.getCanvasWidth() << "x"
              << cmdLineParser_.getCanvasHeight());
        qApp->processEvents();            // to make sure previous layout work is finished
                                          // before resize
        canvas3D_->resize(cmdLineParser_.getCanvasWidth(), cmdLineParser_.getCanvasHeight());
    }

    // Set painter for canvas only after all widget layout is complete, to prevent several
    // calls to sizeChanged() with repaints and costly resize of all RTs
    qApp->processEvents();            // to make sure layout work is finished
    
    canvas3D_->setPainter(painter_);
    canvas3D_->repaint();

#ifdef VRN_WITH_PYTHON
    if (cmdLineParser_.getBenchmark()) {
        canvas3D_->setParent(0);
        canvas3D_->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        canvas3D_->show();
        
        tgt::Script* script = ScriptMgr.load("../../data/scripts/fps.py", false);
        if (script->compile()) {
            if (!script->run())
                QMessageBox::warning(this, "Voreen", tr("Python runtime error (see stdout)"));

        } else {
            QMessageBox::warning(this, "Voreen", tr("Python compile error (see stdout)"));
        }
        ScriptMgr.dispose(script);
    }
#endif // VRN_WITH_PYTHON

    if (!evaluateSuccessful) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Error"),
                              tr("Initialization of one or more processors failed.\n"
                                 "Please check the console or log file for error messages."),
                              QMessageBox::Ok);
    }
    
}

void VoreenMainFrame::deinit() {
    delete overView_;

    MsgDistr.remove(trackNavi_);
    MsgDistr.remove(painter_);
    MsgDistr.remove(canvasMsgProcessor_);

    if (orientationDialog_)
        ((OrientationPlugin*)orientationDialog_->getPlugin())->deinit();
}

void VoreenMainFrame::selectView(QAction* action) {
    switch(action->data().toInt()) {
        case MAIN_VIEW:
            MsgDistr.setCurrentViewId(Identifier("mainview"));
            break;
        case MULTI_VIEW:
            MsgDistr.setCurrentViewId(Identifier("mv1"));
            break;
        case OVER_VIEW:
            MsgDistr.setCurrentViewId(Identifier("ov1"));
            break;
        default:
            break;
    }
    postMessage(new Message("msg.invalidate")); // Cached version is not valid any more, as the
                                                // TextureContainer was used by other renderes
                                                // and may even have been resized.
                                                //FIXME: clear the tc_ instead.

    stackedWidget_->setCurrentIndex(action->data().toInt());
}

TextureContainer* VoreenMainFrame::getTextureContainer() {
    return tc_;
}

void VoreenMainFrame::createActions() {
    openAct_ = new QAction(QIcon(":/vrn_app/icons/open.png"),
                           tr("&Open Dataset..."), this);
    openAct_->setShortcut(tr("Ctrl+O"));
    openAct_->setStatusTip(tr("Open an existing data set"));
    openAct_->setToolTip(tr("Open Dataset"));
    connect(openAct_, SIGNAL(triggered()), this, SLOT(fileOpen()));

	openNetworkFileAction_ = new QAction(tr("Open &Network..."),this);
	openNetworkFileAction_->setStatusTip(tr("Open a Voreen network file"));
	openNetworkFileAction_->setToolTip(tr("Open Network"));
	connect(openNetworkFileAction_, SIGNAL(triggered()),
            this, SLOT(openNetworkFile()));

    quitAct_ = new QAction(QIcon(":/vrn_app/icons/exit.png"), tr("&Quit"), this);
    quitAct_->setShortcut(tr("Ctrl+Q"));
    quitAct_->setStatusTip(tr("Exit the application"));
    quitAct_->setToolTip(tr("Exit the application"));
    connect(quitAct_, SIGNAL(triggered()), this, SLOT(fileExit()));

    aboutAct_ = new QAction(QIcon(":/vrn_app/icons/about.png"), tr("&About..."), this);
    aboutAct_->setStatusTip(tr("Show the application's About box"));
    aboutAct_->setToolTip(tr("Show the application's About box"));
    connect(aboutAct_, SIGNAL(triggered()), this, SLOT(helpAbout()));

    connectCanvasModAct_ = new QAction(tr("&Connect CanvasModifier"), this);
    connectCanvasModAct_->setCheckable(true);
    connect(connectCanvasModAct_, SIGNAL(toggled(bool)),
            this, SLOT(connectCanvasModifier(bool)));

    orientationAction_ = orientationDialog_->createAction();
    orientationAction_->setShortcut(tr("Ctrl+C"));
    orientationAction_->setStatusTip(tr("Set camera orientation and position"));
    orientationAction_->setToolTip(tr("Camera orientation"));

    snapshotAction_ = snapshotDialog_->createAction();
    snapshotAction_->setShortcut(tr("Ctrl+S"));
    snapshotAction_->setStatusTip(tr("Take a snapshot of the current rendering"));
    snapshotAction_->setToolTip(tr("Snapshot"));

    animationAction_ = animationDialog_->createAction();
    animationAction_->setShortcut(tr("Ctrl+A"));
    animationAction_->setStatusTip(tr("Create and play animation paths and record them "
                                      "as videos"));
    animationAction_->setToolTip(tr("Animation"));

    lightMaterialAction_ = lightMaterialDialog_->createAction();
    lightMaterialAction_->setShortcut(tr("Ctrl+L"));
    lightMaterialAction_->setStatusTip(tr("Modify light and material parameters"));
    lightMaterialAction_->setToolTip(tr("Light and material parameters"));

#ifdef VRN_MODULE_DEFORMATION
    deformationAction_ = deformationDialog_->createAction();
    deformationAction_->setShortcut(tr("Ctrl+D"));
    deformationAction_->setStatusTip(tr("Allows you to deform datasets and control how "
                                        "the result is rendered"));
    deformationAction_->setToolTip(tr("Deform datasets"));
#endif

    scriptAction_ = new QAction( QIcon("icons/python.png"), tr("Run Python Script"), this);
    scriptAction_->setShortcut(tr("F7"));
    scriptAction_->setStatusTip(tr("Select and run a python script"));
    scriptAction_->setToolTip(tr("Python script"));
    connect(scriptAction_, SIGNAL(triggered()), this, SLOT(runScript()));
    
    rebuildShadersAction_ = new QAction( QIcon("icons/rebuildshaders.png"),
                                         tr("Rebuild All Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Reloads all shaders currently loaded "
                                           "from file and rebuilds them"));
    rebuildShadersAction_->setToolTip(tr("Rebuild shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));

    textureContainerAction_ = new QAction(QIcon("icons/texcontainer.png"),
                                          tr("Show Texture Container"),  this);
    textureContainerAction_->setShortcut(tr("Ctrl+T"));
    textureContainerAction_->setCheckable(true);
    textureContainerAction_->setToolTip(tr("Show texture container"));
    connect(textureContainerAction_, SIGNAL(triggered(bool)),
            this, SLOT(showTextureContainer(bool)));

    volumeMappingAction_ = new QAction(tr("Show Mapping of Volumes"), this);
    volumeMappingAction_->setToolTip(tr("Displays the mapping of volumes"));
    volumeMappingAction_->setDisabled(true);
    connect(volumeMappingAction_, SIGNAL(triggered(bool)), this, SLOT(showVolumeMapping()));

//     viewToolActions_[MAIN_VIEW]  = new QAction(QIcon("icons/mainview.png"), tr("Main View"), 0);
//     viewToolActions_[MAIN_VIEW]->setToolTip(tr("Show main view"));
//     viewToolActions_[MAIN_VIEW]->setStatusTip(tr("Show main view"));
//     viewToolActions_[MULTI_VIEW] = new QAction(QIcon("icons/multiview.png"), tr("Multi View"), 0);
//     viewToolActions_[MULTI_VIEW]->setToolTip(tr("Show multi view"));
//     viewToolActions_[MULTI_VIEW]->setStatusTip(tr("Show multi view"));
//     viewToolActions_[OVER_VIEW]  = new QAction(QIcon("icons/overview.png"), tr("Over View"), 0);
//     viewToolActions_[OVER_VIEW]->setToolTip(tr("Show over view"));
//     viewToolActions_[OVER_VIEW]->setStatusTip(tr("Show over view"));
}

void VoreenMainFrame::showVolumeMapping() {
}

void VoreenMainFrame::connectCanvasModifier(bool connect) {
    if (connect)
        canvasMod_->connect();
    else
        canvasMod_->disconnect();
}

void VoreenMainFrame::runScript() {
#ifdef VRN_WITH_PYTHON
    QString filename = QFileDialog::getOpenFileName(this, tr("Run script"), "../../data/scripts",
                                                    "Python scripts (*.py)");
    if (!filename.isEmpty()) {
        tgt::Script* script = ScriptMgr.load(filename.toStdString(), false);
        if (script->compile()) {
            if (!script->run())
                QMessageBox::warning(this, "Voreen", tr("Python runtime error (see stdout)"));

        } else {
            QMessageBox::warning(this, "Voreen", tr("Python compile error (see stdout)"));
        }
        ScriptMgr.dispose(script);
    }
#else
    QMessageBox::warning(this, "Voreen", tr("Voreen and tgt have been compiled without "
                         "Python support\n"));
#endif // VRN_WITH_PYTHON
}

void VoreenMainFrame::showTextureContainer(bool enable) {
    if (enable) {
        if (!showTexContainer_) {
            showTexContainer_ = new ShowTexContainerWidget(canvas3D_);
            connect(showTexContainer_, SIGNAL(closing(bool)),
                    textureContainerAction_, SLOT(setChecked(bool)));
        }
        showTexContainer_->setTextureContainer(getTextureContainer());
        showTexContainer_->resize(512, 512);
        showTexContainer_->move(frameGeometry().right()-512, frameGeometry().bottom()-512);
        showTexContainer_->show();
        activateWindow();
    } else {
        if (showTexContainer_ && showTexContainer_->isVisible())
            showTexContainer_->close();
    }
}

void VoreenMainFrame::rebuildShaders() {
	if (ShdrMgr.rebuildAllShadersFromFile()) {
        LINFO("Shaders reloaded");
        evaluator_->invalidateRendering();
		canvas3D_->update();
		#ifdef WIN32
			Beep(100, 100);
		#endif
	} else {
        LWARNING("Shader reloading failed");
		#ifdef WIN32
			Beep(10000, 100);
		#endif
	}
}

void VoreenMainFrame::createMenus() {
    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(openAct_);

    fileMenu_->addSeparator();

    fileMenu_->addSeparator();
	fileMenu_->addAction(openNetworkFileAction_);
	fileMenu_->addSeparator();
    fileMenu_->addAction(quitAct_);

    toolsMenu_ = menuBar()->addMenu(tr("&Utilities"));
    toolsMenu_->addAction(snapshotAction_);

    toolsMenu_->addAction(animationAction_);
    toolsMenu_->addAction(scriptAction_);

#ifdef VRN_MODULE_DEFORMATION
    toolsMenu_->addAction(deformationAction_);
#endif
    toolsMenu_->addAction(rebuildShadersAction_);
    toolsMenu_->addAction(textureContainerAction_);
    toolsMenu_->addAction(volumeMappingAction_);

    optionsMenu_ = menuBar()->addMenu(tr("&Options"));
    optionsMenu_->addAction(orientationAction_);
    optionsMenu_->addAction(lightMaterialAction_);
    optionsMenu_->addAction(connectCanvasModAct_);

//     viewsMenu_ = menuBar()->addMenu(tr("&Views"));
//     viewsMenu_->addAction(viewToolActions_[MAIN_VIEW]);
//     viewsMenu_->addAction(viewToolActions_[MULTI_VIEW]);
//     viewsMenu_->addAction(viewToolActions_[OVER_VIEW]);

    helpMenu_ = menuBar()->addMenu(tr("&Help"));
    helpMenu_->addAction(aboutAct_);
}

void VoreenMainFrame::clearNetwork() {
    // clear containers from existing parts of
    // previously rendered networks
    gc_->clearDeleting();
    
    std::vector<voreen::Processor*>& processors = evaluator_->getProcessors();
    for (size_t i = 0; i < processors.size(); i++)
        delete processors[i];
    processors.clear();
}

void VoreenMainFrame::openNetworkFile() {
    QStringList filenames;

	QFileDialog* fd = new QFileDialog(0, tr("Choose a file to open"), networkPath_);
	QStringList filters;
	filters  << "Voreen network files (*.vnw)"
             << "All Files (*.*)";
	fd->setFilters(filters);
	fd->setViewMode(QFileDialog::Detail);

	if (fd->exec()) {
		if (fd->selectedFiles().size() > 1) {
			QMessageBox::information(0, "Voreen",
				tr("Multiple selection is not allowed for these filetypes."));
		} else {
            filenames = fd->selectedFiles();
            networkPath_ = fd->directory().path();
		}
	} else
        return;

	std::string filename = filenames.at(0).toStdString();
	if (filename.empty()) 
		return;

    // clear traces of previously loaded network
    clearNetwork();

	ProcessorNetwork net = networkSerializer_->readNetworkFromFile(filename);
	std::vector<Processor*> processors = net.processors;
	for (size_t i=0; i < processors.size(); i++) {
		processors.at(i)->setCamera(camera_);
        if (dynamic_cast<Labeling*>(processors.at(i)))
            canvas3D_->getEventHandler()
                ->addListenerToFront(static_cast<Labeling*>(processors.at(i)));
    }
    evaluator_->setProcessors(processors);

    // propagate the VolumeSetContainer to all VolumeSetSourceProcessors in the network.
    MsgDistr.postMessage(
        new voreen::VolumeSetContainerMsg(voreen::VolumeSetContainer::msgSetVolumeSetContainer_,
                                          volumeSetContainer_), "VolumeSetSourceProcessor");

    transferFuncPlugin_->setEvaluator(evaluator_);
	if (evaluator_->analyze() == 0) {
        if (!painter_->setEvaluator(evaluator_)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("Error"),
                                  tr("Initialization of one or more processors failed.\n"
                                     "Please check the console or log file for error messages."),
                                  QMessageBox::Ok);
        }
    }
    
    //pipelineBox_->removeItem(0);
    delete guiGen_;
    guiGen_ = new WidgetGenerator(0, this);
    guiGen_->createAllWidgets(evaluator_->getProcessors(), painter_->getOverlayMgr());
    WidgetPlugin* plug = (WidgetPlugin*)guiGen_;
    plug->createWidgets();
    pipelineBox_->addItem(plug, plug->getIcon(), plug->objectName());
    pipelineBox_->repaint();
}

// ===========================================================================
//
void VoreenMainFrame::fileOpen() {
    if (volumeSetWidget_ != 0) {
        volumeSetWidget_->exec(); // show as modal dialog
        transferFuncPlugin_->setEvaluator(evaluator_); //FIXME: just to activate threshold.
                                                       //joerg
    }
}

void VoreenMainFrame::fileOpen(const QString& fileName, const Modality& forceModality) {
    if (volumeSetWidget_ == 0 || volumeSetContainer_ == 0)
        return;

    VolumeSet* volset = volumeSetWidget_->loadVolumeSet(fileName.toStdString());

    if (forceModality == Modality::MODALITY_SEGMENTATION)
        postMessage(new StringMsg(Labeling::setSegmentDescriptionFile_, fileName.toStdString()));

    if (!forceModality.isUnknown()) {
        volset->forceModality(forceModality);
    } /*else {
        transferFuncPlugin_->dataSourceChanged();
        for (size_t i = 0; i < tfAlphas_.size(); ++i){
            tfAlphas_.at(i)->dataSourceChanged();
        }
    }*/

    postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

void VoreenMainFrame::fileExit() {
	showTextureContainer(false);
    close();
}

void VoreenMainFrame::closeEvent(QCloseEvent* event) {
    // store setings
    QSettings settings;
    
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
    
    settings.beginGroup("Paths");
    settings.setValue("network", networkPath_);
    settings.setValue("dataset", volumeSetWidget_->getCurrentDirectory().c_str());
    settings.endGroup();

    showTextureContainer(false);
    event->accept();
}

void VoreenMainFrame::helpAbout() {
    AboutBox about("VoreenDev", tr("User Interface"), "0.9 beta", this);
    about.exec();
}
