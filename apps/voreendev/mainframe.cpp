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
#include "ui_aboutbox.h"

#include "tgt/gpucapabilities.h"
#include "tgt/qt/qttimer.h"

#ifdef WIN32
    #include "voreen/core/opengl/gpucapabilitieswindows.h"
#endif

#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/volumeserializer.h"

#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/processor.h"
#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/processors/render/slicerenderer.h"
#endif
#include "voreen/core/vis/exception.h"
#include "voreen/core/vis/processors/image/labeling.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/networkanalyzer.h"
#include "voreen/core/vis/trackballnavigation.h"

#include "voreen/qt/datasetserver.h"
#include "voreen/qt/widgets/overviewwidget.h"
#include "voreen/qt/widgets/snapshotplugin.h"
#include "voreen/qt/widgets/flybyplugin.h"
#include "voreen/qt/widgets/lightmaterialplugin.h"
#include "voreen/qt/widgets/backgroundplugin.h"
#include "voreen/qt/widgets/adddialog.h"
#include "voreen/qt/widgets/stereoplugin.h"
#include "voreen/qt/widgets/transfunc/transfuncintensityplugin.h"

#include "voreen/core/geometry/geometrycontainer.h"

#include <QMessageBox>

#include "tgt/init.h"
#include "iosystem.h"

using namespace voreen;

// maximize undocked widget with 'F' key
class MyQDockWidget : public QDockWidget {
public:
    MyQDockWidget (const QString & title, QWidget * parent = 0, Qt::WFlags flags = 0)
        : QDockWidget(title, parent, flags) {}
protected:
    virtual void keyPressEvent(QKeyEvent* e) {
        if (e->key() == Qt::Key_F) {
            if (isMaximized())
                showNormal();
            else
                showMaximized();
        }
        QDockWidget::keyPressEvent(e);
    }
};

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
    , infoPlugin_(0)
    , showTexContainer_(0)
    , processor_(0)
    , ioSystem_( new IOSystem(this) )
    , volumeSerializerPopulator_( ioSystem_->getObserver() )
    , volumeSetContainer_(0)
    , volumeSetWidget_(0)
{
    volumeSetContainer_ = new VolumeSetContainer();
    volumeSetWidget_ = new VolumeSetWidget(volumeSetContainer_, 0, VolumeSetWidget::LEVEL_ALL, Qt::Dialog);
   
    overView_ = 0;

#ifdef VRN_WITH_DCMTK
    dicomDirDialog_ = 0;
#endif
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

    // initialize canvases
    canvas3D_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true, 0);
    canvas3D_->setUpdatesEnabled(false); // disable repaints until GUI is fully initialized
    canvas3D_->setFocusPolicy(Qt::ClickFocus);

    trackball_ = 0;

    canvasDock_ = new MyQDockWidget(tr("Views"), this);
    canvasDock_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    stackedWidget_ = new QStackedWidget(canvasDock_);
    canvasDock_->setWidget(stackedWidget_);
    setCentralWidget(canvasDock_);

    stackedWidget_->addWidget(canvas3D_);

    // initialize object needed for volume management
    volumeSerializer_ = volumeSerializerPopulator_.getVolumeSerializer();

    fileDialogDir_ = QDir(datasetPath);

    MsgDistr.setCurrentViewId(Identifier("mainview"));

#ifdef VRN_WITH_DATASETSERVER
    datasetServer_ = new DatasetServer(this);
#endif // VRN_WITH_DATASETSERVER

    gc_ = new voreen::GeometryContainer();
}

VoreenMainFrame::~VoreenMainFrame() {
    delete orientationDialog_;

    delete volumeSetWidget_;
    delete volumeSetContainer_;

    delete transferFuncPlugin_;
    delete ioSystem_;
    delete camera_;
    delete canvasMsgProcessor_;
    delete trackNavi_;
    delete pipelinePlugins_;
    delete painter_;
    delete canvas3D_;
	delete evaluator_;
    delete guiGen_;
    delete gc_;

#ifdef VRN_WITH_DATASETSERVER
    delete datasetServer_;
#endif // VRN_WITH_DATASETSERVER

    for (size_t i = 0; i < NUM_VIEWS; ++i)
         delete viewToolActions_[i];
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
        camera_->setFocalLength((float)msg->getValue<int>());
}

void VoreenMainFrame::init() {

    #ifdef WIN32
        LDEBUG("Reading Windows specific graphics driver information ...");
        GpuCapabilitiesWindows* gpuCapsWin = GpuCapabilitiesWindows::getInstance();
        GpuCapabilitiesWindows::GraphicsDriverInformation driverInfo = gpuCapsWin->getGraphicsDriverInformation();
        LINFO("Graphics Driver Version: " << driverInfo.driverVersion.versionString);
        LINFO("Graphics Driver Date:    " << driverInfo.driverDate);
        #ifdef VRN_WITH_WMI
            LINFO("Graphics Memory Size:    " << gpuCapsWin->getVideoRamSize() << " MB");
        #else
            LINFO("Graphics Memory Size:    " << "<no WMI support>");
        #endif
    #endif

    MsgDistr.setAllowRepaints(false); // disallow all repaints until fully initialized
                                      // prevents some bad GFX driver crashes on Linux

    camera_ = new tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.75f),
		                      tgt::vec3(0.0f, 0.0f, 0.0f),
                              tgt::vec3(0.0f, 1.0f, 0.0f));
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
    ShdrMgr.setPath("../../src/core/vis/glsl");
    int finalTarget = 20;
    //FIXME: the following lines (till 320) are really ugly
    //       it is redundant code in every app
    //       the initialisation of textureContainer should happen somewhere else (cdoer)
    //	if (!tc_) {
	tc_ = TextureContainer::createTextureContainer(finalTarget + 1);//, false, TextureContainer::VRN_TEXTURE_CONTAINER_RTT);
    tc_->setFinalTarget(20);
    //FIXME: if initialization goes fail and tc_ is 0
    //nullpointer exception in line 309
    //maybe throw exception (cdoer)
    if (tc_->initializeGL() == false) {
        delete tc_;
        tc_ = 0;
        //return std::vector<Renderer*>;
    }

    //id1_.setTC(tc_);

#ifndef __APPLE__
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#else
    // FIXME: support for depth textures on Apple ;)
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        //    TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#endif

    for (int i=0; i < finalTarget; i++) {
        tc_->initializeTarget(i, renderTargetType);
    }
    tc_->initializeTarget(finalTarget, TextureContainer::VRN_FRAMEBUFFER);
    tc_->setFinalTarget(finalTarget);
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,false),"evaluator");

	ProcessorFactory::getInstance()->setTextureContainer(tc_);

    evaluator_ = new NetworkEvaluator();
	evaluator_->setTextureContainer(tc_);
    evaluator_->setGeometryContainer(gc_);

	IDManager id1;
	id1.setTC(tc_);
	//evaluator_->postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,false),"evaluator");
	
    //FIXME: where is tested whether the given network works on current graphics system? (cdoer)
	networkSerializer_ = new NetworkSerializer();
   
    ProcessorNetwork net;
    try {
        net = networkSerializer_->readNetworkFromFile(cmdLineParser_.getNetworkName());
    }
    catch (SerializerException) { // Makes more sense to not catch at all...
        LERROR("Failed to read initial network from file '" << cmdLineParser_.getNetworkName() << "'");
        exit(1);
    }
    
	std::vector<Processor*> processors = net.processors;
    for (size_t i=0; i < processors.size(); i++) {
		processors.at(i)->setCamera(camera_);
    }
	evaluator_->setProcessors(processors);
	MsgDistr.insert(evaluator_);
	evaluator_->analyze();
	painter_->setEvaluator(evaluator_);

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

    QDockWidget* pipelineDock = new QDockWidget(this);
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

    transferFuncPlugin_ = new TransFuncPlugin(toolDock, this);
    transferFuncPlugin_->createWidgets();
    transferFuncPlugin_->createConnections();
    //transferFuncPlugin_->setRenderer(processor_);
	transferFuncPlugin_->setEvaluator(evaluator_);
    toolDock->setWidget(transferFuncPlugin_);
    addDockWidget(Qt::LeftDockWidgetArea, toolDock);

    // FIXME: transferFuncPlugin has an own canvas, that ist current after the initialization
    // this leads to problems with texture id generation after initialisation of
    // TransFuncIntensityGradientPlugin (see there line 43)
    canvas3D_->makeCurrent();

    // initialize information plugin widget
    infoDock_ = new QDockWidget(tr("Dataset Information"), this);
    infoPlugin_ = new InformationPlugin(infoDock_, painter_);
    infoPlugin_->setMaximumHeight(130);
    infoDock_->setWidget(infoPlugin_);
    infoDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, infoDock_);

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

    segmentationDialog_ = new PluginDialog(new SegmentationPlugin(0, this), this);
    addDockWidget(Qt::RightDockWidgetArea, segmentationDialog_);

    LightMaterialPlugin* lightMaterialPlugin = new LightMaterialPlugin(this, this);
    lightMaterialPlugin->disableFeatures(LightMaterialPlugin::MATERIAL_EMISSION /* |
        LightMaterialPlugin::MATERIAL_AMBIENT |
        LightMaterialPlugin::MATERIAL_DIFFUSE |
        LightMaterialPlugin::MATERIAL_USE_OPENGL_MATERIAL */ );
    lightMaterialDialog_ = new PluginDialog(lightMaterialPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, lightMaterialDialog_);

    SnapshotPlugin* snapshotPlugin = new SnapshotPlugin(this, painter_);
    snapshotDialog_ = new PluginDialog(snapshotPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, snapshotDialog_);

    //RPTMERGE, FlybyPlugin expects a renderer
    FlybyPlugin *flybyPlugin = new FlybyPlugin(this, painter_,
        0/*painter_->getRenderer()*/, trackball_, canvas3D_->winId());
    animationDialog_ = new PluginDialog(flybyPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, animationDialog_);

    BackgroundPlugin* backgroundPlugin = new BackgroundPlugin(this, this);
    backgroundPlugin->disableBackgroundLayouts(BackgroundPlugin::RADIAL | BackgroundPlugin::CLOUD | BackgroundPlugin::TEXTURE);
    backgroundPlugin->setIsSliceRenderer(cmdLineParser_.getNetworkName() == "slice.vnw");
    backgroundDialog_ = new PluginDialog(backgroundPlugin, this, true);
    addDockWidget(Qt::RightDockWidgetArea, backgroundDialog_);

    canvas3D_->setPainter(painter_);
#ifdef VRN_MODULE_DEFORMATION
    // for deform
    DeformationPlugin* deformplugin = new DeformationPlugin(0, this, canvas3D_);
    deformationDialog_ = new PluginDialog(deformplugin, this);
    addDockWidget(Qt::RightDockWidgetArea, deformationDialog_);
    if (cmdLineParser_.getNetworkName() == "deform.vnw") {
       deformplugin->makeCutView(MsgDistr);
       deformationDialog_->show();
    }
#endif
    ////pickingPlugin
    //pickingPlugin_ = new PickingPlugin(this,this);
    //pickingDialog_ = new PluginDialog(pickingPlugin_, this, false);


    //if (rendererType == RendererFactory::VRN_PICKINGRAYCASTING){
    //    pickingPlugin_->setTextureContainer(tc_);
    //    canvas3D_->getEventHandler()->addListenerToFront((tgt::EventListener*)pickingPlugin_);
    //    addDockWidget(Qt::RightDockWidgetArea, pickingDialog_);
    //}

    canvas3D_->getEventHandler()->addListenerToFront( (tgt::EventListener*)(transferFuncPlugin_->getIntensityPlugin()) );

    createActions();
    createMenus();
    QToolBar* fileToolBar = new QToolBar(tr("File Toolbar"), this);
    QToolBar* toolsToolBar = new QToolBar(tr("Utilities Toolbar"), this);
    QToolBar* optionsToolBar = new QToolBar(tr("Options Toolbar"), this);
    QToolBar* viewToolBar = new QToolBar(tr("View Toolbar"), this);

    fileToolBar->addAction(openAct_);

    toolsToolBar->addAction(infoAction_);
    toolsToolBar->addAction(snapshotAction_);
    /*toolsToolBar->addAction(animationAction_);
    toolsToolBar->addAction(segmentationAction_); */

    optionsToolBar->addAction(orientationAction_);
    optionsToolBar->addAction(lightMaterialAction_);
    optionsToolBar->addAction(backgroundAction_);
#ifdef VRN_MODULE_DEFORMATION
    toolsToolBar->addAction(deformationAction_);
#endif
    toolsToolBar->addAction(scriptAction_);

    toolsToolBar->addAction(rebuildShadersAction_);
    toolsToolBar->addAction(textureContainerAction_);

    QActionGroup* ag = new QActionGroup(this);
    for (int i = 0; i < NUM_VIEWS; ++i) {
        viewToolActions_[i]->setData(i);
        viewToolActions_[i]->setCheckable(true);
        ag->addAction(viewToolActions_[i]);
        viewToolBar->addAction(viewToolActions_[i]);
    }
    ag->setExclusive(true);
    viewToolActions_[0]->setChecked(true);

    connect(viewToolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(selectView(QAction*)));

    addToolBar(fileToolBar);
    addToolBar(toolsToolBar);
    addToolBar(optionsToolBar);
    addToolBar(viewToolBar);
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
            TransFuncIntensityKeys* tf = new TransFuncIntensityKeys();
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

    // Set painter for canvas only after all widget layout is complete, to prevent several
    // calls to sizeChanged() with repaints and costly resize of all RTs
    qApp->processEvents();            // to make sure layout work is finished

    // Load datasets specified as program arguments
    if (!cmdLineParser_.getFileName().isEmpty())
        //fileOpen(cmdLineParser_.getFileName(), false);
        fileOpen(cmdLineParser_.getFileName());
    if (!cmdLineParser_.getSegmentationFileName().isEmpty())
        //fileOpen(cmdLineParser_.getSegmentationFileName(), true, Modality::MODALITY_SEGMENTATION);
        fileOpen(cmdLineParser_.getSegmentationFileName(), Modality::MODALITY_SEGMENTATION);

    canvas3D_->setPainter(painter_);
    canvas3D_->repaint();
}

void VoreenMainFrame::deinit() {
    delete overView_;

    delete processor_;

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
    openAct_ = new QAction(QIcon(":/vrn_app/icons/open.png"), tr("&Open Dataset..."), this);
    openAct_->setShortcut(tr("Ctrl+O"));
    openAct_->setStatusTip(tr("Open an existing file"));
    openAct_->setToolTip(tr("Open an existing file"));
    connect(openAct_, SIGNAL(triggered()), this, SLOT(fileOpen()));

	openNetworkFileAction_ = new QAction(tr("Open network"),this);
	openNetworkFileAction_->setStatusTip(tr("open a voreen network"));
	openNetworkFileAction_->setToolTip(tr("open a voreen network"));
	connect(openNetworkFileAction_, SIGNAL(triggered()), this, SLOT(openNetworkFile()));

    openDicomDirAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("&Open DICOMDIR Dataset..."), this);
    openDicomDirAct_->setStatusTip(tr("Open an existing DICOMDIR file"));
    openDicomDirAct_->setToolTip(tr("Open an existing DICOMDIR file"));
    connect(openDicomDirAct_, SIGNAL(triggered()), this, SLOT(fileOpenDicomDir()));

  	openDicomFilesAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("Open DICOM Slices..."), this);
  	openDicomFilesAct_->setStatusTip(tr("Open DICOM slices"));
  	openDicomFilesAct_->setToolTip(tr("Open existing DICOM slices"));
  	connect(openDicomFilesAct_, SIGNAL(triggered()), this, SLOT(fileOpenDicomFiles()));
/*
    addDatasetAct_ = new QAction(QIcon(":/vrn_app/icons/open_add.png"), tr("&Add Dataset..."), this);
    addDatasetAct_->setStatusTip(tr("Add a dataset"));
    addDatasetAct_->setToolTip(tr("Add a dataset"));
    addDatasetAct_->setDisabled(true);
    connect(addDatasetAct_, SIGNAL(triggered()), this, SLOT(fileAddDataset()));
*/
    quitAct_ = new QAction(QIcon(":/vrn_app/icons/exit.png"), tr("&Quit"), this);
    quitAct_->setShortcut(tr("Ctrl+Q"));
    quitAct_->setStatusTip(tr("Exit the application"));
    quitAct_->setToolTip(tr("Exit the application"));
    connect(quitAct_, SIGNAL(triggered()), this, SLOT(fileExit()));

    aboutAct_ = new QAction(QIcon(":/vrn_app/icons/about.png"), tr("&About"), this);
    aboutAct_->setStatusTip(tr("Show the application's About box"));
    aboutAct_->setToolTip(tr("Show the application's About box"));
    connect(aboutAct_, SIGNAL(triggered()), this, SLOT(helpAbout()));

    infoAction_ = infoDock_->toggleViewAction();
    infoAction_->setShortcut(tr("Ctrl+I"));
    infoAction_->setIcon(QIcon(":/vrn_app/icons/info.png"));
    infoAction_->setStatusTip(tr("Show information about the currently loaded dataset"));
    infoAction_->setToolTip(tr("Show dataset information"));

    orientationAction_ = orientationDialog_->createAction();
    orientationAction_->setShortcut(tr("Ctrl+C"));
    orientationAction_->setStatusTip(tr("Set camera orientation and position"));
    orientationAction_->setToolTip(tr("Set camera orientation and position"));

    snapshotAction_ = snapshotDialog_->createAction();
    snapshotAction_->setShortcut(tr("Ctrl+S"));
    snapshotAction_->setStatusTip(tr("Take a snapshot of the current rendering"));
    snapshotAction_->setToolTip(tr("Take a snapshot of the current rendering"));

    animationAction_ = animationDialog_->createAction();
    animationAction_->setShortcut(tr("Ctrl+A"));
    animationAction_->setStatusTip(tr("Create and play animation paths and record them "
                                      "as videos"));
    animationAction_->setToolTip(tr("Create animation paths and record them as videos"));

    lightMaterialAction_ = lightMaterialDialog_->createAction();
    lightMaterialAction_->setShortcut(tr("Ctrl+L"));
    lightMaterialAction_->setStatusTip(tr("Modify light and material parameters"));
    lightMaterialAction_->setToolTip(tr("Modify light and material parameters"));

    backgroundAction_ = backgroundDialog_->createAction();
    backgroundAction_->setShortcut(tr("Ctrl+B"));
    backgroundAction_->setStatusTip(tr("Modify appearance of the background"));
    backgroundAction_->setToolTip(tr("Modify appearance of the background"));

#ifdef VRN_MODULE_DEFORMATION
    deformationAction_ = deformationDialog_->createAction();
    deformationAction_->setShortcut(tr("Ctrl+D"));
    deformationAction_->setStatusTip(tr("Allows you to deform datasets and control how "
                                        "the result is rendered"));
    deformationAction_->setToolTip(tr("Deform datasets"));
#endif
/*
    segmentationAction_ = segmentationDialog_->createAction();
    segmentationAction_->setShortcut(tr("Ctrl+G"));
    segmentationAction_->setStatusTip(tr("Control rendering of segmented datasets: Activate "
                                         "segmentation and select active segment."));
    segmentationAction_->setToolTip(tr("Control rendering of segmented datasets"));
*/
    scriptAction_ = new QAction( QIcon("icons/python.png"), tr("Run Python Script"), this);
    scriptAction_->setShortcut(tr("Ctrl+P"));
    scriptAction_->setStatusTip(tr("Select and run a python script"));
    scriptAction_->setToolTip(tr("Run a python script"));
    connect(scriptAction_, SIGNAL(triggered()), this, SLOT(runScript()));
    
    rebuildShadersAction_ = new QAction( QIcon("icons/rebuildshaders.png"), tr("Rebuild All Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Reloads all shaders currently loaded from file and rebuilds them"));
    rebuildShadersAction_->setToolTip(tr("Rebuilds all currently loaded shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));

    textureContainerAction_ = new QAction( QIcon("icons/texcontainer.png"), tr("Show Texture Container"),  this);
    textureContainerAction_->setShortcut(tr("Ctrl+T"));
    textureContainerAction_->setCheckable(true);
    textureContainerAction_->setToolTip(tr("Displays the texture container"));
    connect(textureContainerAction_, SIGNAL(triggered(bool)), this, SLOT(showTextureContainer(bool)));

    volumeMappingAction_ = new QAction(tr("Show Mapping of Volumes"), this);
    volumeMappingAction_->setToolTip(tr("Displays the mapping of volumes"));
    volumeMappingAction_->setDisabled(true);
    connect(volumeMappingAction_, SIGNAL(triggered(bool)), this, SLOT(showVolumeMapping()));

    viewToolActions_[MAIN_VIEW]  = new QAction(QIcon("icons/mainview.png"), tr("Main View"), 0);
    viewToolActions_[MAIN_VIEW]->setToolTip(tr("Show main view"));
    viewToolActions_[MAIN_VIEW]->setStatusTip(tr("Show main view"));
    viewToolActions_[MULTI_VIEW] = new QAction(QIcon("icons/multiview.png"), tr("Multi View"), 0);
    viewToolActions_[MULTI_VIEW]->setToolTip(tr("Show multi view"));
    viewToolActions_[MULTI_VIEW]->setStatusTip(tr("Show multi view"));
    viewToolActions_[OVER_VIEW]  = new QAction(QIcon("icons/overview.png"), tr("Over View"), 0);
    viewToolActions_[OVER_VIEW]->setToolTip(tr("Show over view"));
    viewToolActions_[OVER_VIEW]->setStatusTip(tr("Show over view"));
}

void VoreenMainFrame::showVolumeMapping() {
}



void VoreenMainFrame::runScript() {
#ifdef VRN_WITH_PYTHON
    QString filename = QFileDialog::getOpenFileName(this, tr("Run script"), "scripts",
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
            connect(showTexContainer_, SIGNAL(closing(bool)), textureContainerAction_, SLOT(setChecked(bool)));
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
        LINFO("Shaders reloaded")
		canvas3D_->update();
		#ifdef WIN32
			Beep(100,100);
		#endif
	} else {
        LWARNING("Shader reloading failed");
		#ifdef WIN32
			Beep(10000,100);
		#endif
	}
}

void VoreenMainFrame::createMenus() {
    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(openAct_);
    dicomMenu_ = fileMenu_->addMenu(tr("Open &DICOM Dataset..."));
  	dicomMenu_->addAction(openDicomDirAct_);
  	dicomMenu_->addAction(openDicomFilesAct_);
//fileMenu_->addAction(addDatasetAct_);

    fileMenu_->addSeparator();

    fileMenu_->addSeparator();
	fileMenu_->addAction(openNetworkFileAction_);
	fileMenu_->addSeparator();
    fileMenu_->addAction(quitAct_);

    toolsMenu_ = menuBar()->addMenu(tr("&Utilities"));
    toolsMenu_->addAction(infoAction_);
    /*toolsMenu_->addAction(snapshotAction_);
    toolsMenu_->addAction(animationAction_);
    toolsMenu_->addAction(scriptAction_);
    toolsMenu_->addAction(segmentationAction_);
    */
#ifdef VRN_MODULE_DEFORMATION
    toolsMenu_->addAction(deformationAction_);
#endif
    toolsMenu_->addAction(rebuildShadersAction_);
    /*toolsMenu_->addAction(textureContainerAction_);*/
    toolsMenu_->addAction(volumeMappingAction_);

    optionsMenu_ = menuBar()->addMenu(tr("&Options"));
    optionsMenu_->addAction(orientationAction_);
    optionsMenu_->addAction(lightMaterialAction_);
    optionsMenu_->addAction(backgroundAction_);

    viewsMenu_ = menuBar()->addMenu(tr("&Views"));
    viewsMenu_->addAction(viewToolActions_[MAIN_VIEW]);
    viewsMenu_->addAction(viewToolActions_[MULTI_VIEW]);
    viewsMenu_->addAction(viewToolActions_[OVER_VIEW]);

    helpMenu_ = menuBar()->addMenu(tr("&Help"));
    helpMenu_->addAction(aboutAct_);
}


// show file open dialog, handle multiple selection (used only for dicom slices)
bool VoreenMainFrame::getFileDialog(QStringList& filenames, QDir& dir)
{
    do {
        QFileDialog *fd = new QFileDialog(this, tr("Choose a Volume Dataset to Open"),
                                          dir.absolutePath());
        QStringList filters;
        filters << tr("Volume data (*.DAT *.I4D *.PVM *.RDM *.RDI *.HDR *.SW *.SEG *.TUV "
                      "*.ZIP *.TIFF *.TIF *.MAT *.HV *.NRRD *.NHDR)");
        fd->setFilters(filters);

        if (fd->exec()) {
            if (fd->selectedFiles().size() > 1) {
                QMessageBox::information(this, "Voreen",
                                         tr("Multiple selection is not allowed for these filetypes."));
            } else {
                filenames = fd->selectedFiles();
                QDir dir = fd->directory();
                fileDialogDir_.setPath(dir.absolutePath());
                return true;
            }
        } else {
            return false;
        }

    } while (true);
}

void VoreenMainFrame::clearNetwork()
{
    // clear containers from existing parts of
    // previously rendered networks
    //
    gc_->clearDeleting();
    
    std::vector<voreen::Processor*>& processors = evaluator_->getProcessors();
    for( size_t i = 0; i < processors.size(); i++ )
    {
        delete processors[i];
    }
    processors.clear();
}

void VoreenMainFrame::openNetworkFile() {
	/*std::string filename = openFileDialog("Saved voreen network files (*.vnw)",QDir::currentPath().toStdString() );*/

    QStringList filenames;

	QFileDialog* fd = new QFileDialog(0, tr("Choose a file to open"), networkPath_);
	QStringList filters;
	filters  << "Saved voreen network files (*.vnw)"
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
	} else { return ; }

	std::string filename = filenames.at(0).toStdString();

	if (filename == "") 
		return;

    // clear traces of previously loaded network
    //
    clearNetwork();

	ProcessorNetwork net = networkSerializer_->readNetworkFromFile(filename);
	std::vector<Processor*> processors = net.processors;
	for (size_t i=0; i < processors.size(); i++) {
		processors.at(i)->setCamera(camera_);
        if ( dynamic_cast<Labeling*>(processors.at(i)) )
            canvas3D_->getEventHandler()->addListenerToFront( static_cast<Labeling*>(processors.at(i)) );
    }
    evaluator_->setProcessors(processors);

    // propagate the VolumeSetContainer to all VolumeSetSourceProcessors in the network.
    //
    MsgDistr.postMessage(new voreen::VolumeSetContainerMsg(voreen::VolumeSetContainer::msgSetVolumeSetContainer_, volumeSetContainer_), "VolumeSetSourceProcessor");

    transferFuncPlugin_->setEvaluator(evaluator_);
	if (evaluator_->analyze() == 0)
    {
		painter_->setEvaluator(evaluator_);
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
        volumeSetWidget_->show();
        volumeSetWidget_->raise();
        volumeSetWidget_->activateWindow();
    }
}

void VoreenMainFrame::fileOpen(const QString& fileName, const Modality& forceModality) {
    if ((volumeSetWidget_ == 0) || (volumeSetContainer_ == 0))
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

    postMessage( new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
}

/*
void VoreenMainFrame::fileOpen() {
    QStringList filenames;
    if (getFileDialog(filenames, fileDialogDir_)) {
        VolumeContainer* newVolumeContainer = loadVolumes(filenames);
        finishOpen(newVolumeContainer, filenames[0], false);
    }
    
}

//FIXME: this is only necessary because you can pass filenames as commandlinearguments
void VoreenMainFrame::fileOpen(const QString& fileName, bool add, const Modality& forceModality) {

    VolumeContainer* newVolumeContainer = loadVolumes(QStringList(fileName));

    if (!newVolumeContainer)
        return;

    if (!forceModality.isUnknown())
        newVolumeContainer->setModality(forceModality);

//finishOpen(newVolumeContainer, fileName, add);

    if (forceModality == Modality::MODALITY_SEGMENTATION)
        postMessage(new StringMsg(Labeling::setSegmentDescriptionFile_, fileName.toStdString()));

    showDatasetInfo(volumeContainer_->getVolume(), fileName);

    if (forceModality.isUnknown()){
        transferFuncPlugin_->dataSourceChanged(volumeContainer_->getVolume());
        for (size_t i = 0; i < tfAlphas_.size(); ++i){
            tfAlphas_.at(i)->dataSourceChanged(volumeContainer_->getVolume());
        }
    }

    postMessage( new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);

}

VolumeContainer* VoreenMainFrame::loadVolumes(const QStringList& fileNames) {
    if (fileNames.empty())
        return 0;

    // don't repaint while loading
    setUpdatesEnabled(false);

    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // put into a std::vector of strings
    std::vector<std::string> vFileNames(fileNames.size());
    for (int i = 0; i < fileNames.size(); ++i)
        vFileNames[i] = fileNames.at(i).toStdString();

    VolumeContainer* newVolumeContainer;
    try {
        ioSystem_->show();
        newVolumeContainer = volumeSerializer_->load(vFileNames);
        ioSystem_->hide();
    }
    catch (std::exception& e) {
        ioSystem_->hide();
        QApplication::restoreOverrideCursor();
        setUpdatesEnabled(true);
        qApp->processEvents();
        QMessageBox::warning(this, tr("Failure"), tr("Failed to load volume dataset:\n%1").arg(e.what()));
        newVolumeContainer = NULL;
    }

    QApplication::restoreOverrideCursor();
    canvas3D_->setUpdatesEnabled(true);

    return newVolumeContainer;
}

void VoreenMainFrame::fileAddDataset() {
    QStringList filenames;
    if (getFileDialog(filenames, fileDialogDir_)) {

        VolumeContainer* newVolumeContainer = loadVolumes(filenames);

        // allow repaint
        setUpdatesEnabled(true);

        if (!newVolumeContainer)
            return;

        AddDialog* dialog = new AddDialog(this, volumeContainer_, newVolumeContainer);
        int result = dialog->exec();
        if (result != 1) {
            volumeContainer_->clear();
//finishOpen(dialog->getVolumeContainer(), filenames[0], true);
        }
        delete dialog;
    }
}

void VoreenMainFrame::finishOpen(VolumeContainer* newVolcont, const QString& fileName, bool add)
{
    if (newVolcont) {
        if (!add)
        {
            volumeContainer_->clear();
            volumeSetContainer_->clear();
        }
        volumeContainer_->merge(newVolcont);
volumeSetContainer_->insertContainer(volumeContainer_);
        delete newVolcont;

postMessage(new VolumeContainerPtrMsg(Processor::setVolumeContainer_, volumeContainer_));
postMessage(new IntMsg(Processor::setCurrentDataset_, 0));
        postMessage( new Message(ProxyGeometry::resetClipPlanes_) );

        transferFuncPlugin_->updateTransferFunction();
        transferFuncPlugin_->dataSourceChanged(volumeContainer_->getVolume());

        for (size_t i = 0; i < tfAlphas_.size(); ++i){
            tfAlphas_.at(i)->dataSourceChanged(volumeContainer_->getVolume());
        }


        showDatasetInfo(volumeContainer_->getVolume(), fileName);

        // reset camera if dataset was not added
        if (!add)
            postMessage( new Message(TrackballNavigation::resetTrackball_) );

        // repaint
        postMessage( new Message(VoreenPainter::repaint_) , VoreenPainter::visibleViews_);

        //update Gui
        addDatasetAct_->setEnabled(true);
        volumeMappingAction_->setEnabled(true);
    }

    // allow repaints
    setUpdatesEnabled(true);

    // if a volume was loaded with a transformationmatrix
	// unequal identity then change camera center
*/
   /* RendererFactory::RayCastingType rendererType;
       rendererType = cmdLineParser_.getRendererType();
       if ((rendererType == RendererFactory::VRN_PETCTFUSION) ||
           (rendererType == RendererFactory::VRN_PETCTFUSIONCLIPPING)) {
           tgt::mat4 trafo = volumeContainer_->getVolume( (int) volumeContainer_->size()-1)->meta().getTransformation();
           if (!(trafo.t03 == 0 && trafo.t13 == 0 && trafo.t23 == 0))
                 camera_->setFocus(tgt::vec3(trafo.t03,trafo.t13,trafo.t23));
                 trackball_->setCenter(tgt::vec3(trafo.t03,trafo.t13,trafo.t23));
                 camera_->updateFrustum();
       }*/

//}

/*void VoreenMainFrame::finishOpen(Volume* volume, const QString& fileName) {
    postMessage( new Message(ProxyGeometry::resetClipPlanes_) );
    transferFuncPlugin_->updateTransferFunction();

    if( volume != 0 ) {
        transferFuncPlugin_->dataSourceChanged(volume);
        for (size_t i = 0; i < tfAlphas_.size(); ++i) {
            tfAlphas_.at(i)->dataSourceChanged(volume);
        }
    }

    postMessage( new Message(TrackballNavigation::resetTrackball_) );
    postMessage( new Message(VoreenPainter::repaint_) , VoreenPainter::visibleViews_);
    
    //update Gui
    addDatasetAct_->setEnabled(true);
    volumeMappingAction_->setEnabled(true);
    // allow repaints
    setUpdatesEnabled(true);
}*/

//
// ===========================================================================

void VoreenMainFrame::fileOpenDicomFiles() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getExistingDirectory(
        this,
        "Choose a Directory",
        "../../data",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (tmp != "") {
#ifdef WIN32
        if (!tmp.endsWith("/"))
            tmp += "/";
#endif
        voreen::DicomVolumeReader volumeReader;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        setUpdatesEnabled(false);
        VolumeSet* volumeSet = volumeReader.read(tmp.toStdString());
        if (volumeSet != 0) {
            //FIXME: ??  volsetContainer_->addVolumeSet(volumeSet);
            volumeSetWidget_->updateContent();
        }
        QApplication::restoreOverrideCursor();

		// dirty hack to convert DICOM to dat file
		//volumeSerializer_->save("dicom.dat", volumeContainer_->getVolume());

    }
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif // VRN_WITH_DCMTK
}

void VoreenMainFrame::fileOpenDicomDir() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getOpenFileName(
        this,
        "Choose a File to Open",
        "../../data",
        "DICOMDIR File");
    if (tmp == "")
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    setUpdatesEnabled(false);
    voreen::DicomVolumeReader volumeReader;
    std::vector<voreen::DicomSeriesInfo> series = volumeReader.listSeries(tmp.toStdString());
    QApplication::restoreOverrideCursor();

    if (series.size() > 0) {
        if(dicomDirDialog_) {
            delete  dicomDirDialog_;
        }
        dicomDirDialog_ = new DicomDirDialog();
        connect(dicomDirDialog_, SIGNAL(dicomDirFinished()), this, SLOT(dicomDirFinished()));
        dicomDirDialog_->setSeries(series, tmp.toStdString());
        dicomDirDialog_->show();
    }
    else
        QMessageBox::warning(this, "Voreen", "No DICOM series found.");
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif // VRN_WITH_DCMTK
}


void VoreenMainFrame::dicomDirFinished() {
#ifdef VRN_WITH_DCMTK
    voreen::DicomVolumeReader volumeReader;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    setUpdatesEnabled(false);
    VolumeSet* volumeSet = volumeReader.read(dicomDirDialog_->getFilename());
    if (volumeSet != 0) {
        //FIXME: ?? volsetContainer_->addVolumeSet(volumeSet);
        volumeSetWidget_->updateContent();
    }
    QApplication::restoreOverrideCursor();
#endif
}

/*
void VoreenMainFrame::showDatasetInfo(Volume* dataset, QString fileName) {
    if (!infoPlugin_) return;

    std::ostringstream info;
    info << tr("Filename: ").toStdString() << fileName.toStdString() << "\n";

    info << tr("Dimensions: ").toStdString() << dataset->getDimensions() << std::endl;
    info << tr("Spacing: ").toStdString() << dataset->getSpacing() << std::endl;

    info << tr("Bits stored/allocated: ").toStdString()  << dataset->getBitsStored() << "/"
         << dataset->getBitsAllocated() << "\n";

    infoPlugin_->setPlainText(QString(info.str().c_str()));
}
*/

void VoreenMainFrame::fileExit() {
	showTextureContainer(false);
    close();
}

void VoreenMainFrame::closeEvent(QCloseEvent* event) {
    // storing setings
    QSettings settings;
    settings.beginGroup("MainWindow");
        settings.setValue("size", size());
        settings.setValue("pos", pos());
    settings.endGroup();
    settings.beginGroup("Paths");
        settings.setValue("network", networkPath_);
        settings.setValue("dataset", fileDialogDir_.path());
    settings.endGroup();

    showTextureContainer(false);
    event->accept();
}

void VoreenMainFrame::helpAbout() {

    QDialog* window = new QDialog(this);
    Ui::VoreenAboutBox ui;
    ui.setupUi(window);
#ifndef WIN32
    // On Unix the windows manager should take care of this
    int posX = pos().x() + (width() - window->width()) / 2;
    int posY = pos().y() + (height() - window->height()) / 2;
    window->move(posX, posY);
#endif
    window->setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));
    setDisabled(true);
    window->setDisabled(false);
    window->exec();
    setDisabled(false);
}
