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

#include "voreenmainwindow.h"

#include "rptnetworkserializergui.h"
#include "rptpainterwidget.h"
#include "rptpropertylistwidget.h"
#include "workspace.h"

#include "voreen/core/geometry/geometrycontainer.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/processors/processorfactory.h"

#include "voreen/qt/aboutbox.h"
#include "voreen/qt/helpbrowser.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/widgets/orientationplugin.h"
#include "voreen/qt/widgets/segmentationplugin.h"
#include "voreen/qt/widgets/showtexcontainerwidget.h"
#include "voreen/qt/widgets/volumesetwidget.h"
#include "voreen/qt/widgets/voreentoolwindow.h"

#include "voreen/core/application.h"
#include "voreen/core/version.h"
#include "voreen/core/vis/trackballnavigation.h"

#ifdef VRN_WITH_DEVIL
#include "voreen/qt/widgets/snapshotplugin.h"
#endif

#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

namespace voreen {

namespace {

const int MAX_RECENT_FILES = 5;

// Version number of restoring state of the main window.
// Increase when incompatible changes happen.
const int WINDOW_STATE_VERSION = 10;


} // namespace

////////// VoreenVisualization /////////////////////////////////////////////////////////

VoreenVisualization::VoreenVisualization()
    : QObject()
    , rptnet_(new RptNetwork())
    , renderWidget_(0)
    , networkEditorWidget_(0)
    , volumeSetWidget_(0)
    , readOnlyWorkspace_(false)
{
    camera_ = new tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.5f), tgt::vec3(0.0f, 0.0f, 0.0f), tgt::vec3(0.0f, 1.0f, 0.0f));

    evaluator_ = new NetworkEvaluator();
    evaluator_->setCamera(camera_);
    MsgDistr.insert(evaluator_);

    geoContainer_ = new GeometryContainer();
    evaluator_->setGeometryContainer(geoContainer_);

    volsetContainer_ = new VolumeSetContainer();
}

VoreenVisualization::~VoreenVisualization() {
    delete camera_;
    delete evaluator_;
    delete geoContainer_;
    delete volsetContainer_;
    delete rptnet_;   
}

void VoreenVisualization::init() {
    renderWidget_->init(getEvaluator(), getCamera());

    // Setup the IDManager.
    // TODO: This actually just sets a static member variable... quite awkward. joerg
    IDManager id;
    id.setTC(getEvaluator()->getTextureContainer());
}

void VoreenVisualization::setRenderWidget(RptPainterWidget* renderWidget) {
    renderWidget_ = renderWidget;
}

void VoreenVisualization::setNetworkEditorWidget(RptGraphWidget* networkEditorWidget) {
    networkEditorWidget_ = networkEditorWidget;
    networkEditorWidget_->setNetwork(rptnet_);
    connect(networkEditorWidget, SIGNAL(deleteSignal()), this, SLOT(deleteFromNetwork()));
}

void VoreenVisualization::setVolumeSetWidget(VolumeSetWidget* volumeSetWidget) {
    volumeSetWidget_ = volumeSetWidget;
}

void VoreenVisualization::openNetwork(const std::string& filename, VoreenMainWindow* mainwindow)
    throw (SerializerException)
{
    RptNetwork* network = RptNetwork::load(filename);
    mainwindow->setNetwork(network);
    qApp->processEvents();
    emit(networkLoaded(network));
}

void VoreenVisualization::saveNetwork(const std::string& filename, bool reuseTCTargets)
    throw (SerializerException)
{
    rptnet_->reuseTCTargets = reuseTCTargets;
    rptnet_->volumeSetContainer = volsetContainer_;
    rptnet_->serializeVolumeSetContainer = false;
    ProcessorNetwork* procnet = RptNetworkSerializerGui::makeProcessorNetwork(rptnet_);

    try {
        NetworkSerializer().serializeToXml(procnet, filename);
    } catch (SerializerException&) {
        delete procnet;
        throw;
    }
}

void VoreenVisualization::newWorkspace() {
    readOnlyWorkspace_ = false;
}
    
void VoreenVisualization::openWorkspace(const std::string& filename, VoreenMainWindow* mainwindow)
    throw (SerializerException)
{
    VoreenWorkspace ws(0, camera_, mainwindow);

    readOnlyWorkspace_ = false;
    ws.loadFromXml(filename);
    readOnlyWorkspace_ = ws.readOnly();

    renderWidget_->getTrackballNavigation()->getTrackball()->reinitializeCamera(camera_->getPosition(),
                                                                                camera_->getFocus(),
                                                                                camera_->getUpVector());

    MsgDistr.postMessage(new CameraPtrMsg(VoreenPainter::cameraChanged_, camera_));
    MsgDistr.postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
    emit(networkLoaded(rptnet_));
}

void VoreenVisualization::saveWorkspace(const std::string& filename, bool reuseTCTargets, VoreenMainWindow* mainwindow)
    throw (SerializerException)
{
    readOnlyWorkspace_ = false;
    rptnet_->reuseTCTargets = reuseTCTargets;
    rptnet_->volumeSetContainer = volsetContainer_;
    rptnet_->serializeVolumeSetContainer = false;

    VoreenWorkspace(rptnet_, camera_, mainwindow).serializeToXml(filename);
}

void VoreenVisualization::clearScene() {
    if (rptnet_) {   
        // Remove those processors from the EventHandler which were added to it because
        // they inherit from tgt::EventListener
        std::vector<Processor*> procs;
        for (size_t i = 0; i < rptnet_->processorItems.size(); ++i)
            procs.push_back(rptnet_->processorItems[i]->getProcessor());

        // remove processors from network evaluator
        evaluator_->setProcessors(std::vector<Processor*>());
        
        NetworkSerializer::removeEventListenersFromHandler(renderWidget_->getEventHandler(), procs);
    }

    // clear containers from existing parts of
    // previously rendered networks
    geoContainer_->clearDeleting();
    networkEditorWidget_->clearScene();

    delete rptnet_;
    rptnet_ = new RptNetwork();
    networkEditorWidget_->setNetwork(rptnet_);
}

void VoreenVisualization::setNetwork(RptNetwork* network) {
    delete rptnet_;
    rptnet_ = network;

    // Use new VolumeSetContainer if there is one in the Network
    if (rptnet_->volumeSetContainer) {
        volumeSetWidget_->setVolumeSetContainer(rptnet_->volumeSetContainer);
        delete volsetContainer_;
        volsetContainer_ = rptnet_->volumeSetContainer;
    }
    
    networkEditorWidget_->setNetwork(rptnet_);

	//setReuseTargetsAction_->setChecked(rptnet_->reuseTCTargets);
	//setReuseTargets();

    // add all processors which inherit from tgt::EventListener to the canvas' EventHandler.
    std::vector<Processor*> procs;
    for (size_t i = 0; i < rptnet_->processorItems.size(); ++i)
        procs.push_back(rptnet_->processorItems[i]->getProcessor());
    NetworkSerializer::connectEventListenersToHandler(renderWidget_->getEventHandler(), procs);
}

std::vector<std::string> VoreenVisualization::getNetworkErrors() {
    return rptnet_->errors;
}

bool VoreenVisualization::evaluateNetwork() {
    // send processors to evaluator, they now can receive messages through MsgDistr
    std::vector<Processor*> processors;
    if (rptnet_)
        for (size_t i=0; i < rptnet_->processorItems.size(); i++)
            processors.push_back(rptnet_->processorItems[i]->getProcessor());

    evaluator_->setProcessors(processors);
    
    MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgUpdateVolumeSetContainer_, volsetContainer_));

    bool result = true;
    if (evaluator_->analyze() >= 0) {
        renderWidget_->getGLFocus();
        // this sets the size of the processors in the network
        evaluator_->setSize(renderWidget_->getSize());
        if (evaluator_->initializeGL() != Processor::VRN_OK)
            result = false;
        else
            renderWidget_->repaint();
    }

    MsgDistr.postMessage(new Message("evaluatorUpdated"));

    return result;
}

void VoreenVisualization::deleteFromNetwork() {
   QList<QGraphicsItem*> selectedItems = networkEditorWidget_->scene()->selectedItems();
   if (selectedItems.size() == 0)
       return;

   // sort selectedItems by their type to not delete a port/arrow-item
   // that has already been deleted indirectly with the guiitem
   // so at first kick out the ports:
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptPortItem::Type)
           selectedItems.removeAt(i--);
   }
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptPropertyPort::Type)
           selectedItems.removeAt(i--);
   }
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptTextItem::Type)
           selectedItems.removeAt(i--);
   }

    // next delete arrows
    for (int i=0; i<selectedItems.size(); i++) {
        QGraphicsItem* item = selectedItems[i];
        if (selectedItems.at(i)->type() == RptArrow::Type) {

            RptArrow* arrow = static_cast<RptArrow*>(item);
            if (arrow->getDestNode() != 0) {
                // arrow between ports or arrow between property set and guiitem
                if (arrow->getDestNode()->type() == RptPortItem::Type) {
                    static_cast<RptPortItem*>(arrow->getSourceNode())->getParent()
                        ->disconnect(static_cast<RptPortItem*>(arrow->getSourceNode()),
                                     static_cast<RptPortItem*>(arrow->getDestNode()));
                }
                else if (arrow->getSourceNode()->type() == RptPropertyPort::Type) {
                    static_cast<RptPropertySetItem*>(arrow->getSourceNode()->parentItem())
                        ->disconnectGuiItem(arrow->getDestNode());
                }
            }

            selectedItems.removeAt(i--);
        }
    }

    // eventually delete the guiitems
    for (int i = 0; i < selectedItems.size(); i++) {
        QGraphicsItem* item = selectedItems[i];
        if (item->type() == RptProcessorItem::Type) {
            RptProcessorItem* guiItem = static_cast<RptProcessorItem*>(item);
            if (!(guiItem->parentItem() && guiItem->parentItem()->type() == RptAggregationItem::Type)) {
                for (int j = (int)rptnet_->processorItems.size() - 1; j >= 0 ; j--) {
                    if (rptnet_->processorItems[j] == guiItem) {
                        std::vector<Processor*> procs;
                        procs.push_back(guiItem->getProcessor());
                        NetworkSerializer::removeEventListenersFromHandler(renderWidget_->getEventHandler(), procs);

                        evaluator_->removeProcessor(guiItem->getProcessor());
                        guiItem->disconnectAll(); // remove connected arrows
                        emit(processorDeleted(guiItem->getProcessor()));
                        delete guiItem;
                        rptnet_->processorItems.erase(rptnet_->processorItems.begin() + j);
                        break;
                    }
                }
            }
        }
    }

    // restore focus for main canvas
    // focus is lost when the 2d transfer function editor is deleted
    renderWidget_->getGLFocus();
}

bool VoreenVisualization::rebuildShaders() {
    if (ShdrMgr.rebuildAllShadersFromFile()) {
        evaluator_->invalidateRendering();
        renderWidget_->update();
        return true;
    } else {
        return false;
    }
}

////////// VoreenMdiSubWindow //////////////////////////////////////////////////////////

VoreenMdiSubWindow::VoreenMdiSubWindow(QWidget* widget, QWidget* parent, Qt::WindowFlags flags)
    : QMdiSubWindow(parent, flags)
{
    setWidget(widget);
    setAttribute(Qt::WA_DeleteOnClose, false);
}
    
// Adapted from QWidget::saveGeometry()
QByteArray VoreenMdiSubWindow::saveGeometry() const {
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_0);
    const quint32 magicNumber = 0x1234FFFF;
    quint16 majorVersion = 1;
    quint16 minorVersion = 0;
    stream << magicNumber
           << majorVersion
           << minorVersion
           << frameGeometry()
           << normalGeometry()
           << quint8(windowState() & Qt::WindowMaximized);

    return array;           
}

// Adapted from QWidget::restoreGeometry(). Ignores multiple screen handling as this introduces
// problems for MDI windows (see #65).
bool VoreenMdiSubWindow::restoreGeometry(const QByteArray& geometry) {
    if (geometry.size() < 4)
        return false;
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = 0x1234FFFF;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber)
        return false;

    const quint16 currentMajorVersion = 1;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    if (majorVersion != currentMajorVersion)
        return false;
    // (Allow all minor versions.)

    QRect restoredFrameGeometry;
    QRect restoredNormalGeometry;
    quint8 maximized;

    stream >> restoredFrameGeometry
           >> restoredNormalGeometry
           >> maximized;

    const int frameHeight = 20;
    if (!restoredFrameGeometry.isValid())
        restoredFrameGeometry = QRect(QPoint(0,0), sizeHint());

    if (!restoredNormalGeometry.isValid())
        restoredNormalGeometry = QRect(QPoint(0, frameHeight), sizeHint());

    if (maximized) {
        // set geomerty before setting the window state to make
        // sure the window is maximized to the right screen.
        setGeometry(restoredNormalGeometry);
        Qt::WindowStates ws = windowState();
        if (maximized)
            ws |= Qt::WindowMaximized;
        setWindowState(ws);
    } else {
        QPoint offset;
        setWindowState(windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));
        move(restoredFrameGeometry.topLeft() + offset);
        resize(restoredNormalGeometry.size());
    }
    return true;
}

void VoreenMdiSubWindow::closeEvent(QCloseEvent* event) {
    event->ignore();
    showMinimized();
}

////////// VoreenMainWindow ////////////////////////////////////////////////////////////

namespace {

// Add glass highlight effect to standard menubar
class FancyMenuBar : public QMenuBar {
protected:
    void paintEvent(QPaintEvent* event) {
        QMenuBar::paintEvent(event);

        // draw semi-transparent glass highlight over upper half of menubar
        QPainter painter(this);
        painter.setBrush(QColor(255, 255, 255, 76));
        painter.setPen(Qt::NoPen);       
        painter.drawRect(0, 0, rect().width(), rect().height() / 2);               
    }
};

} // namespace

VoreenMainWindow::VoreenMainWindow(const std::string& network, const std::string& dataset)
    : QMainWindow()
    , guiMode_(MODE_NONE)
    , resetSettings_(false)
{    
    setDockOptions(QMainWindow::AnimatedDocks); // disallow tabbed docks

    // initialialize the console early so it gets all the interesting messages
    consolePlugin_ = new ConsolePlugin(this);
    
    // if we have a stylesheet we want the fancy menu bar, please
    if (!qApp->styleSheet().isEmpty())
        setMenuBar(new FancyMenuBar());
    
    loadSettings();
    if (!network.empty())
        currentNetwork_ = network.c_str();
    if (!dataset.empty())
        defaultDataset_ = dataset.c_str();

    setMinimumSize(300, 200);
    setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));
    setAcceptDrops(true);
    setWindowModified(false);
    updateWindowTitle();

    vis_ = new VoreenVisualization();
    
    // The widget containing all currently loaded volumesets must be created before loading the
    // first dataset on startup.
    volumeSetWidget_ = new VolumeSetWidget(vis_->getVolumeSetContainer(), this, VolumeSetWidget::LEVEL_ALL);
    volumeSetWidget_->setCurrentDirectory(datasetPath_.toStdString());
    vis_->setVolumeSetWidget(volumeSetWidget_);

    // Create Canvas Widget before initGL() is called.
    // Disable rendering updates until initGL() is finished to prevent
    // rendering of garbage of Mac OS, happening when the widgets is
    // rendered with no painter attached.
    renderWidget_ = new RptPainterWidget(this);
    renderWidget_->setUpdatesEnabled(false); // will be enabled when fully initialized
    vis_->setRenderWidget(renderWidget_);

    mdiArea_ = new QMdiArea(this);
    mdiArea_->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
    setCentralWidget(mdiArea_);

    renderWindow_ = new VoreenMdiSubWindow(renderWidget_, this);
    mdiArea_->addSubWindow(renderWindow_, Qt::SubWindow | Qt::WindowStaysOnTopHint);
    renderWidget_->setWindowTitle(tr("Visualization"));
    
    // update canvas when volume is loaded/removed/changed
    connect(volumeSetWidget_, SIGNAL(volumeSetChanged()), renderWidget_, SLOT(update()));

    createMenus();
    createToolBars();
}

VoreenMainWindow::~VoreenMainWindow() {
    ProcessorFactory::getInstance()->destroy();

    delete propertyListWidget_; // needs to be deleted before properties and thus processors
    delete networkEditorWidget_;
    delete renderWidget_;
    delete vis_;
}

void VoreenMainWindow::init() {
    vis_->init();

    // network editor
    networkEditorWidget_ = new RptGraphWidget(this, vis_->getEvaluator());
    vis_->setNetworkEditorWidget(networkEditorWidget_);
    connect(networkEditorWidget_, SIGNAL(processorSelected(Processor*)), this, SLOT(processorSelected(Processor*)));

    networkEditorWidget_->setWindowTitle(tr("Processor Network"));
    networkEditorWindow_ = new VoreenMdiSubWindow(networkEditorWidget_, this);
    mdiArea_->addSubWindow(networkEditorWindow_);
    
    // if item is double clicked, show properties
    connect(networkEditorWidget_, SIGNAL(showPropertiesSignal()), this, SLOT(showProperties()));

    // signals indicating a change in network
    connect(networkEditorWidget_, SIGNAL(processorAdded(Identifier, QPoint)), this, SLOT(modified()));
    connect(networkEditorWidget_, SIGNAL(pasteSignal()), this, SLOT(modified()));
    
    // create tool windows now, after everything is initialized
    createToolWindows();
   
    // restore session
    loadWindowSettings();

    if (resetSettings_) {
        QMessageBox::information(this, tr("VoreenVE"), tr("Configuration reset."));
        resetSettings_ = false;
    }
    
    //
    // now the GUI is complete
    //

    if (!lastWorkspace_.isEmpty() && loadLastWorkspace_) {
        // load last workspace
        openWorkspace(lastWorkspace_);
    } else {
        if (!currentNetwork_.isEmpty()) {
            // load an initial network
            qApp->processEvents(); // workaround for initial rendering problem
            openNetwork(currentNetwork_);
        }
        else {
            // load an initial worksspace
            openWorkspace(VoreenApplication::app()->getWorkspacePath("standard.vws").c_str());
        }

        // load an initial dataset
        if (!defaultDataset_.isEmpty())
            loadDataset(defaultDataset_.toStdString(), false);
    }

    // now we can activate rendering in the widget
    renderWidget_->setUpdatesEnabled(true);
    setUpdatesEnabled(true);
}

////////// GUI setup ///////////////////////////////////////////////////////////////////

void VoreenMainWindow::createMenus() {
    menu_ = menuBar();

    //
    // File menu
    //
    fileMenu_ = menu_->addMenu(tr("&File"));

    // Workspace
    workspaceNewAction_ = new QAction(QIcon(":/icons/clear.png"), tr("New Workspace"),  this);
    workspaceNewAction_->setShortcut(QKeySequence::New);
    connect(workspaceNewAction_, SIGNAL(triggered()), this, SLOT(newWorkspace()));
    fileMenu_->addAction(workspaceNewAction_);

    workspaceOpenAction_ = new QAction(QIcon(":/vrn_app/icons/open.png"), tr("Open Workspace..."),  this);
    workspaceOpenAction_->setShortcut(QKeySequence::Open);
    connect(workspaceOpenAction_, SIGNAL(triggered()), this, SLOT(openWorkspace()));
    fileMenu_->addAction(workspaceOpenAction_);

    workspaceSaveAction_ = new QAction(QIcon(":/vrn_app/icons/save.png"), tr("Save Workspace"),  this);
    workspaceSaveAction_->setShortcut(QKeySequence::Save);
    connect(workspaceSaveAction_, SIGNAL(triggered()), this, SLOT(saveWorkspace()));
    fileMenu_->addAction(workspaceSaveAction_);
    
    workspaceSaveAsAction_ = new QAction(QIcon(":/vrn_app/icons/saveas.png"), tr("Save Workspace As..."),  this);
    connect(workspaceSaveAsAction_, SIGNAL(triggered()), this, SLOT(saveWorkspaceAs()));
    fileMenu_->addAction(workspaceSaveAsAction_);
    
    fileMenu_->addSeparator();

    // Network
    openNetworkFileAction_ = new QAction(QIcon(":/vrn_app/icons/open_network.png"), tr("Open Network..."), this);
    connect(openNetworkFileAction_, SIGNAL(triggered()), this, SLOT(openNetwork()));
    fileMenu_->addAction(openNetworkFileAction_);
    
    saveNetworkAsAction_ = new QAction(QIcon(":/vrn_app/icons/save.png"), tr("Save Network As..."), this);
    connect(saveNetworkAsAction_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
    fileMenu_->addAction(saveNetworkAsAction_);

    fileMenu_->addSeparator();

    // Dataset
    openDatasetAction_ = new QAction(QIcon(":/vrn_app/icons/open_volume.png"), tr("Open Data Set..."), this);
    openDatasetAction_->setStatusTip(tr("Open a volume data set"));
    connect(openDatasetAction_, SIGNAL(triggered()), this, SLOT(openDataset()));
    fileMenu_->addAction(openDatasetAction_);

    dicomMenu_ = fileMenu_->addMenu(tr("Open &DICOM Dataset..."));

    openDicomDirAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("&Open DICOMDIR Data Set..."), this);
    openDicomDirAct_->setStatusTip(tr("Open an existing DICOMDIR file"));
    openDicomDirAct_->setToolTip(tr("Open an existing DICOMDIR file"));
    connect(openDicomDirAct_, SIGNAL(triggered()), volumeSetWidget_, SLOT(buttonAddDICOMDirClicked()));
    dicomMenu_->addAction(openDicomDirAct_);

    openDicomFilesAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("Open DICOM Slices..."), this);
    openDicomFilesAct_->setStatusTip(tr("Open DICOM slices"));
    openDicomFilesAct_->setToolTip(tr("Open existing DICOM slices"));
    connect(openDicomFilesAct_, SIGNAL(triggered()), volumeSetWidget_, SLOT(buttonAddDICOMClicked()));
    dicomMenu_->addAction(openDicomFilesAct_);

    fileMenu_->addSeparator();

    quitAction_ = new QAction(QIcon(":/vrn_app/icons/exit.png"), tr("&Quit"), this);
    quitAction_->setShortcut(tr("Ctrl+Q"));
    quitAction_->setStatusTip(tr("Exit the application"));
    quitAction_->setToolTip(tr("Exit the application"));
    connect(quitAction_, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu_->addAction(quitAction_);
    
    fileMenu_->addSeparator();

    // Recent files
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        recentFileActs_.append(new QAction(this));        
        connect(recentFileActs_[i], SIGNAL(triggered()), this, SLOT(openRecentFile())); 
        fileMenu_->addAction(recentFileActs_[i]);
    }   
    updateRecentFiles();
    
    //
    // Edit menu
    //
//    editMenu_ = menu_->addMenu(tr("&Edit"));

    //
    // Tools menu
    //
    toolsMenu_ = menu_->addMenu(tr("&View"));

    //
    // Action menu
    //
    actionMenu_ = menu_->addMenu(tr("&Action"));
    
    evaluatorAction_ = new QAction(QIcon(":/icons/player_play-grey.png"), "Evaluate Network", this);
    evaluatorAction_->setShortcut(tr("F9"));
    connect(evaluatorAction_, SIGNAL(triggered()), this, SLOT(evaluateNetwork()));   
    actionMenu_->addAction(evaluatorAction_);

#ifdef VRN_WITH_PYTHON
    scriptAction_ = new QAction(tr("Run Python Script..."), this);
    scriptAction_->setShortcut(tr("F7"));
    scriptAction_->setStatusTip(tr("Select and run a python script"));
    scriptAction_->setToolTip(tr("Run a python script"));
    connect(scriptAction_, SIGNAL(triggered()), this, SLOT(runScript()));
    actionMenu_->addAction(scriptAction_);
#endif
    
    rebuildShadersAction_ = new QAction(QIcon(":/vrn_app/icons/reload.png"), tr("Rebuild Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Reloads all shaders currently loaded from file and rebuilds them"));
    rebuildShadersAction_->setToolTip(tr("Rebuilds all currently loaded shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));
    actionMenu_->addAction(rebuildShadersAction_);

    //
    // Options menu
    //
    optionsMenu_ = menu_->addMenu(tr("&Options"));

#ifndef VRN_SNAPSHOT
    navigationMenu_ = optionsMenu_->addMenu(tr("Select Camera Navigation..."));
    navigationGroup_ = new QActionGroup(this);
    connect(navigationGroup_, SIGNAL(triggered(QAction*)), this, SLOT(navigationChanged()));

    trackballNaviAction_ = new QAction(tr("Trackball navigation"), this);
    trackballNaviAction_->setCheckable(true);
    trackballNaviAction_->setChecked(true);
    navigationMenu_->addAction(trackballNaviAction_);
    navigationGroup_->addAction(trackballNaviAction_);

    flythroughNaviAction_ = new QAction(tr("Flythrough navigation"), this);
    flythroughNaviAction_->setCheckable(true);
    navigationMenu_->addAction(flythroughNaviAction_);
    navigationGroup_->addAction(flythroughNaviAction_);
    
    optionsMenu_->addSeparator();
#endif
    
    loadLastWorkspaceAct_ = new QAction(tr("&Load last workspace on startup"), this);
    loadLastWorkspaceAct_->setCheckable(true);
    loadLastWorkspaceAct_->setChecked(loadLastWorkspace_);
    connect(loadLastWorkspaceAct_, SIGNAL(triggered()), this, SLOT(setLoadLastWorkspace()));
    optionsMenu_->addAction(loadLastWorkspaceAct_);

    setReuseTargetsAction_ = new QAction("Reuse TC targets (needs rebuild)", this);
    setReuseTargetsAction_->setCheckable(true);
    setReuseTargetsAction_->setChecked(false);
//     connect(setReuseTargetsAction_, SIGNAL(triggered()), this, SLOT(setReuseTargets()));
//     optionsMenu_->addAction(setReuseTargetsAction_);

    //
    // Help menu
    //
    helpMenu_ = menu_->addMenu(tr("&Help"));

#ifndef VRN_SNAPSHOT
    helpFirstStepsAct_ = new QAction(QIcon(":/vrn_app/icons/wizard.png"), tr("Getting Started..."), this);
    connect(helpFirstStepsAct_, SIGNAL(triggered()), this, SLOT(helpFirstSteps()));
    helpMenu_->addAction(helpFirstStepsAct_);

    helpMenu_->addSeparator();
#endif

    aboutAction_ = new QAction(QIcon(":/vrn_app/icons/about.png"), tr("&About..."), this);
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(helpAbout()));
    helpMenu_->addAction(aboutAction_);
}

void VoreenMainWindow::createToolBars() {
    fileToolBar_ = addToolBar(tr("File"));
    fileToolBar_->setObjectName("file");
    fileToolBar_->addAction(workspaceNewAction_);
    fileToolBar_->addAction(workspaceOpenAction_);
    fileToolBar_->addAction(workspaceSaveAction_);
    fileToolBar_->addSeparator();
    fileToolBar_->addAction(openNetworkFileAction_);
    fileToolBar_->addAction(openDatasetAction_);
    fileToolBar_->addSeparator();
    fileToolBar_->addAction(evaluatorAction_);
    
    toolsToolBar_ = addToolBar(tr("Tools"));
    toolsToolBar_->setObjectName("tools");

    guiModeToolBar_ = addToolBar(tr("GUI Mode"));
    guiModeToolBar_->setObjectName("gui mode");

    modeNetworkAction_ = new QAction(QIcon(":/vrn_app/icons/network.png"), tr("Network Mode"), this);
    modeNetworkAction_->setCheckable(true);
    guiModeToolBar_->addAction(modeNetworkAction_);

	modeVisualizationAction_ = new QAction(QIcon(":/vrn_app/icons/visualization.png"), tr("Visualization Mode"), this);
	modeVisualizationAction_->setCheckable(true);
	guiModeToolBar_->addAction(modeVisualizationAction_);

    QActionGroup* guiModeGroup = new QActionGroup(this);
    guiModeGroup->addAction(modeVisualizationAction_);
    guiModeGroup->addAction(modeNetworkAction_);
    modeVisualizationAction_->setChecked(true);

    connect(guiModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(guiModeChanged()));

    processorToolsToolBar_ = addToolBar(tr("Processor Tools"));
    processorToolsToolBar_->setObjectName("processor tools");
}

VoreenToolWindow* VoreenMainWindow::addToolWindow(QAction* action, QWidget* widget, const QString& name, bool basic) {
    action->setCheckable(true);

    VoreenToolWindow* window = new VoreenToolWindow(action, this, widget, name);
    window->adjustSize(); // prevents strange sizes written to config file

    if (WidgetPlugin* plugin = dynamic_cast<WidgetPlugin*>(widget)) {
        if (!plugin->usable(std::vector<Processor*>())) // not usable without processors?
            action->setVisible(false); // will be made visible in evaluateNetwork()
        tools_ << std::make_pair(plugin, action);
    }

    if (basic) {
        toolsMenu_->addAction(action);
        toolsToolBar_->addAction(action);
    } else {
        processorToolsToolBar_->addAction(action);
    }

    window->setVisible(false);
    toolWindows_ << window;
    
    return window;
}

VoreenToolDockWindow* VoreenMainWindow::addToolDockWindow(QAction* action, QWidget* widget, const QString& name,
                                                          Qt::DockWidgetArea dockarea, bool basic)
{
    action->setCheckable(true);

    VoreenToolDockWindow* window = new VoreenToolDockWindow(action, this, widget, name);

    if (WidgetPlugin* plugin = dynamic_cast<WidgetPlugin*>(widget)) {
        if (!plugin->usable(std::vector<Processor*>())) // not usable without processors?
            action->setVisible(false); // will be made visible in evaluateNetwork()
        tools_ << std::make_pair(plugin, action);
    }

    if (basic) {
        toolsMenu_->addAction(action);
        toolsToolBar_->addAction(action);
    } else {
        processorToolsToolBar_->addAction(action);
    }

    window->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(dockarea, window);  
    window->setVisible(false);

    return window;
}
 
void VoreenMainWindow::createToolWindows() {
    // processor list
    processorListWidget_ = new RptProcessorListWidget();
    networkEditorWidget_->addAllowedWidget(processorListWidget_);
    processorListWidget_->setMinimumSize(200, 200);
    processorListAction_ = new QAction(QIcon(":/vrn_app/icons/processor.png"), tr("Processors"), this);
    processorListTool_ = addToolDockWindow(processorListAction_, processorListWidget_, "ProcessorList");

    // property list
    propertyListWidget_ = new RptPropertyListWidget(this, 0);
    connect(propertyListWidget_, SIGNAL(repaintSignal()), renderWidget_, SLOT(repaint()));
    connect(vis_, SIGNAL(networkLoaded(RptNetwork*)), propertyListWidget_, SLOT(newNetwork(RptNetwork*)));
    connect(networkEditorWidget_, SIGNAL(processorAdded(RptProcessorItem*)), propertyListWidget_, SLOT(processorAdded(RptProcessorItem*)));
    connect(vis_, SIGNAL(processorDeleted(Processor*)), propertyListWidget_, SLOT(processorDeleted(Processor*)));
    propertyListTool_ = addToolDockWindow(new QAction(QIcon(":/icons/information.png"), tr("Properties"), this),
                                          propertyListWidget_, "Properties", Qt::RightDockWidgetArea);
    
    // volumes
    addToolDockWindow(new QAction(QIcon(":/vrn_app/icons/volume.png"), tr("Volumes"), this),
                      volumeSetWidget_, "Volumes", Qt::RightDockWidgetArea);

    // console
    QAction* consoleAction = new QAction(QIcon(":/icons/console.png"), tr("Debug Console"), this);
    consoleAction->setShortcut(tr("Ctrl+D"));
    VoreenToolWindow* console = addToolWindow(consoleAction, consolePlugin_, "Console");
    console->resize(700, 300);

    // texture container
    ShowTexContainerWidget* texContainerWidget = new ShowTexContainerWidget(renderWidget_);
    texContainerWidget->setTextureContainer(vis_->getEvaluator()->getTextureContainer());
    texContainerWidget->setMinimumSize(200, 200);
    QAction* texContainerAction = new QAction(QIcon(":/icons/grid.png"),tr("Texture Container"), this);
    texContainerAction->setShortcut(tr("Ctrl+T"));
    VoreenToolWindow* tc = addToolWindow(texContainerAction, texContainerWidget, "TextureContainer");
    tc->resize(500, 500);
    
    // orientation
    OrientationPlugin* orientationPlugin
        = new OrientationPlugin(this, renderWidget_, renderWidget_->getTrackballNavigation()->getTrackball());
    orientationPlugin->createWidgets();
    orientationPlugin->createConnections();
    renderWidget_->getTrackballNavigation()->addReceiver(orientationPlugin);
    addToolDockWindow(new QAction(QIcon(":/icons/trackball-reset-inverted.png"), tr("Camera Orientation"), this),
                      orientationPlugin, "Orientation", Qt::LeftDockWidgetArea, false);
    
#ifdef VRN_WITH_DEVIL
    // snapshot
    SnapshotPlugin* snapshotPlugin = new SnapshotPlugin(this, dynamic_cast<VoreenPainter*>(renderWidget_->getPainter()));
    snapshotPlugin->createWidgets();
    snapshotPlugin->createConnections();
    addToolDockWindow(new QAction(QIcon(":/vrn_app/icons/snapshot.png"), tr("Snapshot"), this), snapshotPlugin, "Snapshot",
                      Qt::RightDockWidgetArea, false);
#endif

    // animation
    AnimationPlugin* animationPlugin = new AnimationPlugin(this, vis_->getCamera(), renderWidget_);
    animationPlugin->createWidgets();
    animationPlugin->createConnections();
    addToolDockWindow(new QAction(QIcon(":/vrn_app/icons/camera.png"), tr("Animation"), this), animationPlugin, "Animation",
                      Qt::RightDockWidgetArea, false);
    
    processorToolsToolBar_->addSeparator();

    // segmentation
    SegmentationPlugin* segwidget = new SegmentationPlugin(this, vis_->getEvaluator());
    addToolWindow(new QAction(QIcon(":/icons/segmentation-ncat.png"), tr("Segmentation"), this),
                  segwidget, "Segmentation", false);
}

////////// settings ////////////////////////////////////////////////////////////////////

void VoreenMainWindow::loadSettings() {
    // set defaults
    networkPath_ = VoreenApplication::app()->getNetworkPath().c_str();
    workspacePath_ = VoreenApplication::app()->getWorkspacePath().c_str();
    datasetPath_ = VoreenApplication::app()->getVolumePath().c_str();
	QSize windowSize = QSize(0, 0);
	QPoint windowPosition = QPoint(0, 0);
	bool windowMaximized = true;

    // restore settings
    if (!resetSettings_) {
        settings_.beginGroup("MainWindow");
        windowSize = settings_.value("size", windowSize).toSize();
		windowPosition = settings_.value("pos", windowPosition).toPoint();
		windowMaximized = settings_.value("maximized", windowMaximized).toBool();
        lastWorkspace_ = settings_.value("workspace", "").toString();
        loadLastWorkspace_ = settings_.value("loadLastWorkspace", false).toBool();
        visualizationModeState_ = settings_.value("visualizationModeState").toByteArray();
        networkModeState_ = settings_.value("networkModeState").toByteArray();
        renderWindowStateVisualizationMode_ = settings_.value("renderWindowStateVisualizationMode").toByteArray();
        renderWindowStateNetworkMode_ = settings_.value("renderWindowStateNetworkMode").toByteArray();
        networkEditorWindowState_ = settings_.value("networkEditorWindowState").toByteArray();
        settings_.endGroup();

        settings_.beginGroup("Paths");
        networkPath_ = settings_.value("network", networkPath_).toString();
        workspacePath_ = settings_.value("workspace", workspacePath_).toString();
        datasetPath_ = settings_.value("dataset", datasetPath_).toString();
        settings_.endGroup();
    }
    if (windowSize.isNull()) {
        resize(1024, 786);
    } else {
        resize(windowSize);
    }
	move(windowPosition);
	if (windowMaximized)
		setWindowState(windowState() | Qt::WindowMaximized);
}

void VoreenMainWindow::loadWindowSettings() {
    // Restore visiblity, position and size of tool windows from settings
    if (!resetSettings_) {
        settings_.beginGroup("Windows");
        for (int i=0; i < toolWindows_.size(); ++i) {
            if (!toolWindows_[i]->objectName().isEmpty()) {
                settings_.beginGroup(toolWindows_[i]->objectName());
                if (settings_.contains("size"))
                    toolWindows_[i]->resize(settings_.value("size").toSize());

                // Ignore position (0, 0) for invisible windows as otherwise all previously
                // invisible windows would be placed at (0, 0) after restarting the application.
                if (settings_.contains("pos") &&
                    (settings_.value("pos").toPoint() != QPoint(0, 0) || settings_.value("visible").toBool()))
                {
                    toolWindows_[i]->move(settings_.value("pos").toPoint());
                }

                if (settings_.contains("visible"))
                    toolWindows_[i]->setVisible(settings_.value("visible").toBool());
                settings_.endGroup();
            }
        }
        settings_.endGroup();
    }


    settings_.beginGroup("MainWindow");
    bool visualizationMode = settings_.value("visualizationMode").toBool();
    settings_.endGroup();
    
    // Set the initial GUI mode after everything is ready.
    setGuiMode(visualizationMode ? MODE_VISUALIZATION : MODE_NETWORK);
}    

void VoreenMainWindow::saveSettings() {
    // store settings
    settings_.setValue("ResetSettings", resetSettings_);

    // write version number of the config file format (might be useful someday)
    settings_.setValue("ConfigVersion", 1);

    if (guiMode_ == MODE_VISUALIZATION) {
        visualizationModeState_ = saveState(WINDOW_STATE_VERSION);
        renderWindowStateVisualizationMode_ = renderWindow_->saveGeometry();
    } else if (guiMode_ == MODE_NETWORK) {
        networkModeState_ = saveState(WINDOW_STATE_VERSION);
        renderWindowStateNetworkMode_ = renderWindow_->saveGeometry();
        networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
    }
    
    settings_.beginGroup("MainWindow");
    settings_.setValue("size", size());
	settings_.setValue("pos", pos());
	settings_.setValue("maximized", (windowState() & Qt::WindowMaximized) != 0);
    settings_.setValue("workspace", lastWorkspace_);
    settings_.setValue("loadLastworkspace", loadLastWorkspace_);
    settings_.setValue("visualizationModeState", visualizationModeState_);
    settings_.setValue("networkModeState", networkModeState_);
    settings_.setValue("renderWindowStateVisualizationMode", renderWindowStateVisualizationMode_);
    settings_.setValue("renderWindowStateNetworkMode", renderWindowStateNetworkMode_);
    settings_.setValue("networkEditorWindowState", networkEditorWindowState_);
    settings_.setValue("visualizationMode", (guiMode_ == MODE_VISUALIZATION));
    settings_.endGroup();

    settings_.beginGroup("Paths");
    settings_.setValue("network", networkPath_);
    settings_.setValue("workspace", workspacePath_);
    settings_.setValue("dataset", volumeSetWidget_->getCurrentDirectory().c_str());
    settings_.endGroup();

    settings_.beginGroup("Windows");

    for (int i=0; i < toolWindows_.size(); ++i) {
        if (!toolWindows_[i]->objectName().isEmpty()) {
            settings_.beginGroup(toolWindows_[i]->objectName());
            settings_.setValue("visible", toolWindows_[i]->isVisible());
            settings_.setValue("pos", toolWindows_[i]->pos());
            settings_.setValue("size", toolWindows_[i]->size());
            settings_.endGroup();
        }
    }
    settings_.endGroup();
    
}

////////// loading / saving ////////////////////////////////////////////////////////////

void VoreenMainWindow::openNetwork() {
    if (!askSave())
        return;

    QFileDialog fileDialog(this, tr("Open Network"),
                           QDir(networkPath_).absolutePath(),
                           "Voreen network files (*.vnw)");
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getNetworkPath().c_str());
    fileDialog.setSidebarUrls(urls);
    
    if (fileDialog.exec()) {
        openNetwork(fileDialog.selectedFiles().at(0));
        networkPath_ = fileDialog.directory().path();
    }
}

void VoreenMainWindow::openNetwork(const QString& filename) {
    try {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        vis_->openNetwork(filename.toStdString(), this);       
        currentNetwork_ = filename;
        addToRecentFiles(currentNetwork_);
        updateWindowTitle();
        QApplication::restoreOverrideCursor();
        evaluateNetwork();
    } catch (SerializerException& e) {
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
    }
}

bool VoreenMainWindow::saveNetworkAs() {
    QFileDialog fileDialog(this, tr("Save Network"), QDir(networkPath_).absolutePath(),
                           tr("Voreen network files (*.vnw)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    fileDialog.setDefaultSuffix("vnw");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getNetworkPath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        currentNetwork_ = fileDialog.selectedFiles().at(0);

        try {
            vis_->saveNetwork(currentNetwork_.toStdString(), setReuseTargetsAction_->isChecked());
        } catch (SerializerException& e) {
            QApplication::restoreOverrideCursor();
            QErrorMessage* errorMessageDialog = new QErrorMessage(this);
            errorMessageDialog->showMessage(e.what());
            return false;
        }
        
        setWindowModified(false);

        networkPath_ = fileDialog.directory().path();
        setWindowModified(false);
        updateWindowTitle();
        addToRecentFiles(currentNetwork_);
        return true;
    }
    return false;
}

bool VoreenMainWindow::askSave() {
    if (isWindowModified()) {
        switch (QMessageBox::question(this, tr("VoreenVE"), tr("Save the the current workspace?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
        {
        case QMessageBox::Yes:
            saveWorkspace();
            return true;
        case QMessageBox::No:
            return true;
        default:
            return false;
        }
    }
    return true;
}

void VoreenMainWindow::newWorkspace() {
    if (!askSave())
        return;

    vis_->newWorkspace();
    
    currentWorkspace_ = "";
    currentNetwork_ = "";
    lastWorkspace_ = currentWorkspace_;
    updateWindowTitle();

    clearScene();

    setWindowModified(false);    
    renderWidget_->update();
}

void VoreenMainWindow::openWorkspace(const QString& filename) {
    try {
        vis_->openWorkspace(filename.toStdString(), this);
    }
    catch (SerializerException e) {
        currentWorkspace_ = "";
        lastWorkspace_ = currentWorkspace_;
        updateWindowTitle();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(tr("Could not open workspace:\n") + e.what());
        return;
    }

    evaluateNetwork();
    currentWorkspace_ = filename;
    lastWorkspace_ = currentWorkspace_;
    currentNetwork_ = "";
    updateWindowTitle();
    addToRecentFiles(currentWorkspace_);
}

void VoreenMainWindow::openWorkspace() {
    if (!askSave())
        return;

    QFileDialog fileDialog(this, tr("Open Workspace"),
                           QDir(workspacePath_).absolutePath(),
                           "Voreen workspaces (*.vws)");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        openWorkspace(fileDialog.selectedFiles().at(0));
        workspacePath_ = fileDialog.directory().path();
    }
}

void VoreenMainWindow::saveWorkspace(const QString& filename) {
    QString f = filename;
    if (f.isEmpty() && !vis_->readOnlyWorkspace())
        f = currentWorkspace_;

    if (f.isEmpty()) {
        saveWorkspaceAs();
        return;
    }

    try {
        vis_->saveWorkspace(f.toStdString(), setReuseTargetsAction_->isChecked(), this);
    } catch (SerializerException& e) {
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        return;
    }
    
    currentWorkspace_ = f;
    lastWorkspace_ = currentWorkspace_;
    updateWindowTitle();
    addToRecentFiles(currentWorkspace_);
}
    
void VoreenMainWindow::saveWorkspaceAs() {
    QFileDialog fileDialog(this, tr("Save Workspace"), QDir(workspacePath_).absolutePath(),
                           tr("Voreen workspaces (*.vws)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    fileDialog.setDefaultSuffix("vws");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);
    
    if (fileDialog.exec()) {
        saveWorkspace(fileDialog.selectedFiles().at(0));
        workspacePath_ = fileDialog.directory().path();
    }
}

void VoreenMainWindow::openDataset() {
    std::vector<std::string> files = volumeSetWidget_->openFileDialog();
    if (!files.empty())
        volumeSetWidget_->addVolumeSets(files);
}

void VoreenMainWindow::loadDataset(const std::string& filename, bool showProgress) {
    if (!showProgress)
        volumeSetWidget_->setUseProgress(false);
    volumeSetWidget_->loadVolumeSet(filename);
    if (!showProgress)
        volumeSetWidget_->setUseProgress(true);
}

void VoreenMainWindow::openRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString file(action->data().toString());
        if (file.endsWith(".vws", Qt::CaseInsensitive))
            openWorkspace(file);
        else
            openNetwork(file);
    }
}

void VoreenMainWindow::addToRecentFiles(const QString& filename) {
    QStringList files = settings_.value("recentFileList").toStringList();
    files.removeAll("");        // delete empty entries
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_RECENT_FILES)
        files.removeLast();

    settings_.setValue("recentFileList", files);
    updateRecentFiles();
}

void VoreenMainWindow::updateRecentFiles() {
    QStringList files = settings_.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs_[i]->setText(text);
        recentFileActs_[i]->setData(files[i]);
        recentFileActs_[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
        recentFileActs_[j]->setVisible(false);
}

////////// network /////////////////////////////////////////////////////////////////////

void VoreenMainWindow::clearScene() {
    vis_->clearScene();
    propertyListWidget_->clear();
}

void VoreenMainWindow::setNetwork(RptNetwork* network) {
    clearScene();
    vis_->setNetwork(network);
    
    // placed here, because loading a network emits changed signals
    setWindowModified(false);

    showNetworkErrors();
}

void VoreenMainWindow::showNetworkErrors() {
    // alert about errors in the Network
    std::vector<std::string> errors = vis_->getNetworkErrors();
    if (!errors.empty()) {
        QString msg;
        for (size_t i=0; i < errors.size(); i++)
            msg += "<li>" + QString(errors[i].c_str()) + "</li>\n";

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(tr("There were %1 errors loading the network:\n<ul>").arg(errors.size())
                                        + msg + "\n</ul>");
    }
}

void VoreenMainWindow::evaluateNetwork() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (!vis_->evaluateNetwork()) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Error"),
                              tr("Initialization of one or more processors failed.\n"
                                 "Please check the console or log file for error messages."),
                              QMessageBox::Ok);
    }

    // ask each tool whether it should be visible for these processors
    std::pair<WidgetPlugin*, QAction*> p;
    foreach (p, tools_) {
        bool usable = p.first->usable(vis_->getEvaluator()->getProcessors());
        p.second->setVisible(usable);
        // hide tool's toolwindow if not usable
        VoreenToolWindow* toolWindow = dynamic_cast<VoreenToolWindow*>(p.first->parent());
        if (toolWindow && !usable)
            toolWindow->setVisible(false);
    }

    QApplication::restoreOverrideCursor();
}

////////// actions /////////////////////////////////////////////////////////////////////

//
// General
//

void VoreenMainWindow::closeEvent(QCloseEvent *event) {
    saveSettings();

    //TODO: use isWindowModified()
    if (!currentWorkspace_.isEmpty() && !vis_->readOnlyWorkspace()) {
        switch (QMessageBox::question(this, tr("VoreenVE"), tr("Save the the current workspace?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
        {
        case QMessageBox::Yes:
            saveWorkspace();
            event->accept();
            break;
        case QMessageBox::No:
            event->accept();
            break;
        default:
            event->ignore();
            break;
        }
    }
}

void VoreenMainWindow::dragEnterEvent(QDragEnterEvent* event) {
    QList<QUrl> urls = event->mimeData()->urls();
    
    if (!urls.empty() && urls.first().toLocalFile().endsWith(".vnw"))
        event->acceptProposedAction();
}

void VoreenMainWindow::dropEvent(QDropEvent* event) {
    openNetwork(event->mimeData()->urls().first().toLocalFile());
}

//
// Action menu
//

void VoreenMainWindow::runScript() {
#ifdef VRN_WITH_PYTHON
    QString filename = QFileDialog::getOpenFileName(this, tr("Run script"),
                                                    VoreenApplication::app()->getScriptPath().c_str(),
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

void VoreenMainWindow::rebuildShaders() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (vis_->rebuildShaders()) {
        LINFOC("VoreenMainWindow", "Shaders reloaded");
        #ifdef WIN32
        Beep(100, 100);
        #endif
    } else {
        LWARNINGC("VoreenMainWindow", "Shader reloading failed");
        QMessageBox::critical(this, tr("Shader reloading"),
                              tr("Shader reloading failed.\n"
                                 "See the Debug Console for details."));
        #ifdef WIN32
        Beep(10000, 100);
        #endif
    }        
    QApplication::restoreOverrideCursor();
}

//
// Options menu
//

void VoreenMainWindow::navigationChanged() {
    if (trackballNaviAction_->isChecked())
        renderWidget_->setCurrentNavigation(RptPainterWidget::TRACKBALL_NAVIGATION);
    else
        renderWidget_->setCurrentNavigation(RptPainterWidget::FLYTHROUGH_NAVIGATION);
}

void VoreenMainWindow::setLoadLastWorkspace() {
    loadLastWorkspace_ = loadLastWorkspaceAct_->isChecked();
}

void VoreenMainWindow::setReuseTargets() {
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,
                                     setReuseTargetsAction_->isChecked()), "evaluator");
}

//
// Help menu
//

void VoreenMainWindow::helpAbout() {
    AboutBox about("VoreenVE", tr("VoreenVE Visualization Environment"), "1.0", this);
    about.exec();
}

void VoreenMainWindow::helpFirstSteps() {
    HelpBrowser* help = new HelpBrowser("file:///" + QDir::currentPath() + "/doc/gsg.html");
    help->resize(900, 600);
    help->show();
}

////////// further functions ///////////////////////////////////////////////////////////

void VoreenMainWindow::showProperties() {
    propertyListTool_->setVisible(true);
}

void VoreenMainWindow::processorSelected(Processor* processor) {
    propertyListWidget_->processorSelected(processor);
}

void VoreenMainWindow::modified() {
    setWindowModified(true);
}

void VoreenMainWindow::changeEvent(QEvent* event) {
    // Filter out window title changes which were done outside setWindowTitle (non-virtual) of
    // this class. This is used to prevent MDI windows from adding their title to the main
    // window title when maximized.
    if (event->type() == QEvent::WindowTitleChange) {
        if (windowTitle() != originalWindowTitle_)
            setWindowTitle(originalWindowTitle_);
    }
}
void VoreenMainWindow::setWindowTitle(const QString& title) {
    originalWindowTitle_ = title;
    QMainWindow::setWindowTitle(title);
}

void VoreenMainWindow::updateWindowTitle() {
    QString title = tr("VoreenVE [*]");

    if (!currentNetwork_.isEmpty() || !currentWorkspace_.isEmpty() ) {
        QFileInfo f(currentWorkspace_.isEmpty() ? currentNetwork_ : currentWorkspace_); // get filename without path
        title += " - " + f.fileName();
    }

    setWindowTitle(title);
}

void VoreenMainWindow::guiModeChanged() {
    if (modeVisualizationAction_->isChecked())
        setGuiMode(MODE_VISUALIZATION);
    else if (modeNetworkAction_->isChecked())
        setGuiMode(MODE_NETWORK);
}

void VoreenMainWindow::setGuiMode(GuiMode guiMode) {
    if (guiMode_ == guiMode)
        return;

    renderWidget_->setUpdatesEnabled(false);
    renderWidget_->setVisible(false); // hide the OpenGL widget to prevent flicker

    if (guiMode == MODE_VISUALIZATION) {
        if (guiMode_ == MODE_NETWORK) {
            networkModeState_ = saveState(WINDOW_STATE_VERSION);
            renderWindowStateNetworkMode_ = renderWindow_->saveGeometry();
            networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
        }

        // hide all first to prevent some flicker
        networkEditorWindow_->hide();
        networkEditorWidget_->setVisible(false);
        renderWindow_->hide();

        if (!restoreState(visualizationModeState_, WINDOW_STATE_VERSION)) {
            processorListTool_->hide();
            propertyListTool_->show();
        }

        if (renderWindow_->restoreGeometry(renderWindowStateVisualizationMode_))
            renderWindow_->show();
        else
            renderWindow_->showMaximized();
        modeVisualizationAction_->setChecked(true);

        propertyListWidget_->setState(RptPropertyListWidget::LIST, Property::USER);
        processorListAction_->setEnabled(false);        
    }
    else if (guiMode == MODE_NETWORK) {
        if (guiMode_ == MODE_VISUALIZATION) {
            visualizationModeState_ = saveState(WINDOW_STATE_VERSION);
            renderWindowStateVisualizationMode_ = renderWindow_->saveGeometry();
        }

        networkEditorWindow_->hide();
        
        if (!restoreState(networkModeState_, WINDOW_STATE_VERSION)) {
            processorListTool_->show();
            propertyListTool_->show();
        }

        if (networkEditorWindow_->restoreGeometry(networkEditorWindowState_))
            networkEditorWindow_->show();
        else
            networkEditorWindow_->showMaximized();

        networkEditorWidget_->setVisible(true); // only show now, so it immediately gets the correct size
        
        if (renderWindow_->restoreGeometry(renderWindowStateNetworkMode_))
            renderWindow_->show();        
        else
            renderWindow_->showNormal();
        
        modeNetworkAction_->setChecked(true);

        propertyListWidget_->setState(RptPropertyListWidget::SINGLE, Property::DEVELOPER);
        processorListAction_->setEnabled(true);
    }
    renderWidget_->setVisible(true);
    renderWidget_->setUpdatesEnabled(true);

    guiMode_ = guiMode;
}

} // namespace
