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

#include "tgt/gpucapabilities.h"
#include "tgt/filesystem.h"
#include "tgt/ziparchive.h"
#include "tgt/qt/qtcanvas.h"

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/network/processornetwork.h"
#include "voreen/core/vis/processors/processorfactory.h"

#include "voreen/core/vis/workspace/workspace.h"
#include "voreen/core/vis/network/networkevaluator.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"

#include "voreen/core/volume/volumecontainer.h"

#include "voreen/qt/aboutbox.h"
#include "voreen/qt/helpbrowser.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/widgets/segmentationplugin.h"
#include "voreen/qt/widgets/shortcutpreferenceswidget.h"
#include "voreen/qt/widgets/rendertargetdebugwidget.h"
#include "voreen/qt/widgets/volumecontainerwidget.h"
#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/animationplugin.h"
#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"
#include "voreen/qt/widgets/processor/canvasrendererwidget.h"

#include "voreen/qt/widgets/network/processorlistwidget.h"
#include "voreen/qt/widgets/network/propertylistwidget.h"
#include "voreen/qt/widgets/network/editor/networkeditor.h"

#include "voreen/core/application.h"
#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/core/version.h"

#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#include "voreen/core/vis/pyvoreen.h"
#endif // VRN_WITH_PYTHON

#include <QVariant>

namespace voreen {

namespace {

const int MAX_RECENT_FILES = 5;

// Version number of restoring state of the main window.
// Increase when incompatible changes happen.
const int WINDOW_STATE_VERSION = 11;


} // namespace

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

VoreenMainWindow::VoreenMainWindow(const std::string& workspace, const std::string& dataset)
    : QMainWindow()
    , guiMode_(MODE_NONE)
    , resetSettings_(false)
    , canvasPos_(0, 0)
    , canvasSize_(0, 0)
{
    setDockOptions(QMainWindow::AnimatedDocks); // disallow tabbed docks

    // initialialize the console early so it gets all the interesting messages
    consolePlugin_ = new ConsolePlugin(this);

    // if we have a stylesheet we want the fancy menu bar, please
    if (!qApp->styleSheet().isEmpty())
        setMenuBar(new FancyMenuBar());

    loadSettings();
    if (!workspace.empty())
        currentWorkspace_ = workspace.c_str();
    if (!dataset.empty())
        defaultDataset_ = dataset.c_str();

    setMinimumSize(300, 200);
    setWindowIcon(QIcon(":/voreenve/icons/voreen-logo_64x64.png"));
    setAcceptDrops(true);

    // show tooltips (for icons in toolbar) even if the window is inactive, as often the case
    // when a canvas window is active
    setAttribute(Qt::WA_AlwaysShowToolTips);
    
    ignoreWindowTitleModified_ = true;
    updateWindowTitle();
}

VoreenMainWindow::~VoreenMainWindow() {
    ProcessorFactory::getInstance()->destroy();

    delete propertyListWidget_;             // needs to be deleted before properties and thus processors
    delete vis_;
}

void VoreenMainWindow::init() {
    // initGL requires a valid OpenGL context
    sharedContext_ = new tgt::QtCanvas("Init Canvas", tgt::ivec2(32, 32), tgt::GLCanvas::RGBADD, this, true);
    sharedContext_->init(); //neccessary?

    VoreenApplication::app()->initGL();

    sharedContext_->hide();


    // some hardware/driver checks
    if (GpuCaps.getVendor() != GpuCaps.GPU_VENDOR_NVIDIA && GpuCaps.getVendor() != GpuCaps.GPU_VENDOR_ATI) {
        qApp->processEvents();
        QMessageBox::warning(this, tr("Unsupported Video Card Vendor"),
                             tr("Voreen has only been tested with video cards from NVIDIA and ATI. "
                                "The card in this system (reported vendor: '%1') is not supported and the application "
                                "might not work properly.").arg(GpuCaps.getVendorAsString().c_str()));
        qApp->processEvents();
    }

    if (!GpuCaps.isOpenGlVersionSupported(tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_2_0)) {
        qApp->processEvents();
        std::ostringstream glVersion;
        glVersion << GpuCaps.getGlVersion();
        QMessageBox::critical(this, tr("Incompatible OpenGL Version"),
                              tr("Voreen requires OpenGL version 2.0 or higher, which does not seem be "
                                 "supported on this system (reported version: %1). Therefore, the application "
                                 "will most likely not work properly.").arg(glVersion.str().c_str()));
        qApp->processEvents();
    }
    else if (!GpuCaps.areFramebufferObjectsSupported()) {
        qApp->processEvents();
        QMessageBox::critical(this, tr("Framebuffer Objects Missing"),
                              tr("Voreen uses OpenGL framebuffer objects, which do not seem be supported "
                                 "on this system. Therefore, the application will most likely not work properly."));
        qApp->processEvents();
    }
    else if (!GpuCaps.isShaderModelSupported(tgt::GpuCapabilities::SHADER_MODEL_3)) {
        qApp->processEvents();
        QMessageBox::critical(this, tr("Incompatible Shader Model"),
                              tr("Voreen requires Shader Model 3 or higher, which does not seem be "
                                 "supported on this system. Therefore, the application will most likely not "
                                 "work properly."));
        qApp->processEvents();
    }
    else if (GpuCaps.getShaderVersion() < tgt::GpuCapabilities::GlVersion::SHADER_VERSION_110) {
        qApp->processEvents();
        std::ostringstream glslVersion;
        glslVersion << GpuCaps.getShaderVersion();
        QMessageBox::critical(this, tr("Incompatible Shader Language Version"),
                              tr("Voreen requires OpenGL shader language (GLSL) version 1.10, which does not "
                                 "seem to be supported on this system (reported version: %1)."
                                 "Therefore, the application will most likely not work properly.")
                              .arg(QString::fromStdString(glslVersion.str())));
        qApp->processEvents();
    }



    // create visualization object
    vis_ = new VoreenVisualization(sharedContext_);

    // mdi area
    mdiArea_ = new QMdiArea(this);
    mdiArea_->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
    setCentralWidget(mdiArea_);

    // create tool windows now, after everything is initialized, but before window settings are restored
    createMenus();
    createToolBars();
    createToolWindows();

    // signals indicating a change in network
    connect(vis_, SIGNAL(networkModified(ProcessorNetwork*)), this, SLOT(adjustSnapshotMenu()));
    connect(vis_, SIGNAL(newNetwork(ProcessorNetwork*)), this, SLOT(adjustSnapshotMenu()));
    connect(networkEditorWidget_, SIGNAL(pasteSignal()), this, SLOT(adjustSnapshotMenu()));
    connect(vis_, SIGNAL(modified()), this, SLOT(networkModified()));    

    qApp->processEvents();

    // restore session
    if (resetSettings_) {
        QMessageBox::information(this, tr("VoreenVE"), tr("Configuration reset."));
        qApp->processEvents();
        resetSettings_ = false;
    }
    loadWindowSettings();
    setGuiMode(guiMode_);

    //
    // now the GUI is complete
    //
    if (!currentWorkspace_.isEmpty()) {
        // load an initial workspace
        openWorkspace(currentWorkspace_);
    }
    else if (!lastWorkspace_.isEmpty() && loadLastWorkspace_) {
        // load last workspace
        openWorkspace(lastWorkspace_);
    }
    else {
        // load an initial workspace
        openWorkspace(VoreenApplication::app()->getWorkspacePath("standard.vws").c_str());
    }

    // adjust canvas widgets (created during workspace load) to visualization mode
    if (guiMode_ == MODE_VISUALIZATION)
        adjustCanvasWidgets(MODE_VISUALIZATION);

    // load an initial dataset
    if (!defaultDataset_.isEmpty())
        loadDataset(defaultDataset_.toStdString());
}

////////// GUI setup ///////////////////////////////////////////////////////////////////

void VoreenMainWindow::createMenus() {
    menu_ = menuBar();

    //
    // File menu
    //
    fileMenu_ = menu_->addMenu(tr("&File"));

    // Workspace
    workspaceNewAction_ = new QAction(QIcon(":/voreenve/icons/clear.png"), tr("New Workspace"),  this);
    workspaceNewAction_->setShortcut(QKeySequence::New);
    connect(workspaceNewAction_, SIGNAL(triggered()), this, SLOT(newWorkspace()));
    fileMenu_->addAction(workspaceNewAction_);

    workspaceOpenAction_ = new QAction(QIcon(":/voreenve/icons/open.png"), tr("Open Workspace..."),  this);
    workspaceOpenAction_->setShortcut(QKeySequence::Open);
    connect(workspaceOpenAction_, SIGNAL(triggered()), this, SLOT(openWorkspace()));
    fileMenu_->addAction(workspaceOpenAction_);

    workspaceSaveAction_ = new QAction(QIcon(":/voreenve/icons/save.png"), tr("Save Workspace"),  this);
    workspaceSaveAction_->setShortcut(QKeySequence::Save);
    connect(workspaceSaveAction_, SIGNAL(triggered()), this, SLOT(saveWorkspace()));
    fileMenu_->addAction(workspaceSaveAction_);


    workspaceSaveAsAction_ = new QAction(QIcon(":/voreenve/icons/saveas.png"), tr("Save Workspace As..."),  this);
    connect(workspaceSaveAsAction_, SIGNAL(triggered()), this, SLOT(saveWorkspaceAs()));
    fileMenu_->addAction(workspaceSaveAsAction_);

    fileMenu_->addSeparator();

    // Volumes
    openDatasetAction_ = new QAction(QIcon(":/voreenve/icons/open-volume.png"), tr("Load Volume..."), this);
    openDatasetAction_->setStatusTip(tr("Load a volume data set"));
    openDatasetAction_->setShortcut(tr("Ctrl+L"));
    connect(openDatasetAction_, SIGNAL(triggered()), this, SLOT(openDataset()));
    fileMenu_->addAction(openDatasetAction_);

    openDicomFilesAct_ = new QAction(QIcon(":/voreenve/icons/open-dicom.png"), tr("Load DICOM Slices..."), this);
    openDicomFilesAct_->setStatusTip(tr("Load DICOM slices"));
    openDicomFilesAct_->setToolTip(tr("Load DICOM files containing individual slices"));
    connect(openDicomFilesAct_, SIGNAL(triggered()), this, SLOT(buttonAddDICOMClicked()));
    fileMenu_->addAction(openDicomFilesAct_);

    fileMenu_->addSeparator();

#ifdef VRN_WITH_ZLIB
    // ZIP-Workspaces
    workspaceExtractAction_ = new QAction(QIcon(":/voreenve/icons/extract-workspace.png"),
        tr("Extract Workspace Archive..."), this);
    connect(workspaceExtractAction_, SIGNAL(triggered()), this, SLOT(extractZippedWorkspace()));
    fileMenu_->addAction(workspaceExtractAction_);
#endif // VRN_WITH_ZLIB

    // Network
    importNetworkAction_ = new QAction(QIcon(":/voreenve/icons/import.png"), tr("Import Network..."), this);
    connect(importNetworkAction_, SIGNAL(triggered()), this, SLOT(openNetwork()));
    fileMenu_->addAction(importNetworkAction_);

    exportNetworkAction_ = new QAction(QIcon(":/voreenve/icons/export.png"), tr("Export Network..."), this);
    connect(exportNetworkAction_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
    fileMenu_->addAction(exportNetworkAction_);

    fileMenu_->addSeparator();

    quitAction_ = new QAction(QIcon(":/voreenve/icons/exit.png"), tr("&Quit"), this);
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
    // View menu
    //
    viewMenu_ = menu_->addMenu(tr("&View"));

    modeNetworkAction_ = new QAction(QIcon(":/voreenve/icons/development-mode.png"),
                                     tr("Development Mode"), this);
    modeNetworkAction_->setCheckable(true);
    modeNetworkAction_->setShortcut(tr("F2"));

    modeVisualizationAction_ = new QAction(QIcon(":/voreenve/icons/visualization-mode.png"),
                                           tr("Visualization Mode"), this);
    modeVisualizationAction_->setCheckable(true);
    modeVisualizationAction_->setShortcut(tr("F3"));

    QActionGroup* guiModeGroup = new QActionGroup(this);
    guiModeGroup->addAction(modeVisualizationAction_);
    guiModeGroup->addAction(modeNetworkAction_);
    modeVisualizationAction_->setChecked(true);
    connect(guiModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(guiModeChanged()));

    viewMenu_->addAction(modeNetworkAction_);
    viewMenu_->addAction(modeVisualizationAction_);
    viewMenu_->addSeparator();

    //
    // Tools menu
    //
    toolsMenu_ = menu_->addMenu(tr("&Tools"));

    snapshotAction_ = new QAction(QIcon(":/voreenve/icons/make-snapshot.png"), "Snapshot", this);
    snapshotAction_->setData(0);
    QMenu* snapshotMenu = new QMenu(this);
    snapshotAction_->setMenu(snapshotMenu);
    snapshotAction_->setEnabled(false);
    connect(snapshotAction_, SIGNAL(triggered(bool)), this, SLOT(snapshotActionTriggered(bool)));
    toolsMenu_->addAction(snapshotAction_);


    //
    // Action menu
    //
    actionMenu_ = menu_->addMenu(tr("&Action"));
    rebuildShadersAction_ = new QAction(QIcon(":/voreenve/icons/rebuild.png"), tr("Rebuild Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Rebuilds all currently used shaders"));
    rebuildShadersAction_->setToolTip(tr("Rebuild all shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));
    actionMenu_->addAction(rebuildShadersAction_);

#ifdef VRN_WITH_PYTHON
    scriptAction_ = new QAction(QIcon(":/voreenve/icons/python.png"), tr("Run Python Script..."), this);
    scriptAction_->setShortcut(tr("F7"));
    scriptAction_->setStatusTip(tr("Select and run a python script"));
    scriptAction_->setToolTip(tr("Run Python script"));
    connect(scriptAction_, SIGNAL(triggered()), this, SLOT(runScript()));
    actionMenu_->addAction(scriptAction_);
#endif

    //
    // Options menu
    //
    optionsMenu_ = menu_->addMenu(tr("&Options"));

    loadLastWorkspaceAct_ = new QAction(tr("&Load Last Workspace on Startup"), this);
    loadLastWorkspaceAct_->setCheckable(true);
    loadLastWorkspaceAct_->setChecked(loadLastWorkspace_);
    connect(loadLastWorkspaceAct_, SIGNAL(triggered()), this, SLOT(setLoadLastWorkspace()));
    optionsMenu_->addAction(loadLastWorkspaceAct_);

    setReuseTargetsAction_ = new QAction("Reuse TC targets (needs rebuild)", this);
    setReuseTargetsAction_->setCheckable(true);
    setReuseTargetsAction_->setChecked(false);

    //
    // Help menu
    //
    helpMenu_ = menu_->addMenu(tr("&Help"));

    helpFirstStepsAct_ = new QAction(QIcon(":/voreenve/icons/help.png"), tr("&Getting Started Guide..."), this);
    helpFirstStepsAct_->setShortcut(tr("F1"));
    connect(helpFirstStepsAct_, SIGNAL(triggered()), this, SLOT(helpFirstSteps()));
    helpMenu_->addAction(helpFirstStepsAct_);

    helpMenu_->addSeparator();

    aboutAction_ = new QAction(QIcon(":/voreenve/icons/about.png"), tr("&About VoreenVE..."), this);
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(helpAbout()));
    helpMenu_->addAction(aboutAction_);
}

void VoreenMainWindow::createToolBars() {

    // file toolbar
    fileToolBar_ = addToolBar(tr("File"));
    fileToolBar_->setObjectName("file-toolbar");
    fileToolBar_->addAction(workspaceNewAction_);
    fileToolBar_->addAction(workspaceOpenAction_);
    fileToolBar_->addAction(workspaceSaveAction_);
    fileToolBar_->addAction(openDatasetAction_);

    // view toolbar
    viewToolBar_ = addToolBar(tr("View"));
    viewToolBar_->setObjectName("view-toolbar");
    QLabel* label = new QLabel(tr("   View  "));
    label->setObjectName("toolBarLabel");
    viewToolBar_->addWidget(label);

    viewToolBar_->addAction(modeNetworkAction_);
    viewToolBar_->addAction(modeVisualizationAction_);
    viewToolBar_->addSeparator();

    // tools toolbar
    toolsToolBar_ = addToolBar(tr("Tools"));
    toolsToolBar_->setObjectName("tools-toolbar");
    label = new QLabel(tr("   Tools "));
    label->setObjectName("toolBarLabel");
    toolsToolBar_->addWidget(label);
    toolsToolBar_->addAction(snapshotAction_);

    // action toolbar
    actionToolBar_ = addToolBar(tr("Action"));
    actionToolBar_->setObjectName("action-toolbar");
    label = new QLabel(tr("   Action "));
    label->setObjectName("toolBarLabel");
    actionToolBar_->addWidget(label);
    actionToolBar_->addAction(rebuildShadersAction_);
    #ifdef VRN_WITH_PYTHON
        actionToolBar_->addAction(scriptAction_);
    #endif

#ifdef __APPLE__ // we are on a mac system
    // HACK (Workaround) for Qt Mac Bug, makes MainWindow reappear
    // see for details:
    // http://bugreports.qt.nokia.com/browse/QTBUG-5069?page=com.atlassian.jira.plugin.system.issuetabpanels%3Aall-tabpanel
    show();
#endif

}

VoreenToolWindow* VoreenMainWindow::addToolWindow(QAction* action, QWidget* widget, const QString& name, bool basic) {
    action->setCheckable(true);

    VoreenToolWindow* window = new VoreenToolWindow(action, this, widget, name, false);
    addDockWidget(Qt::LeftDockWidgetArea, window);

    if (WidgetPlugin* plugin = dynamic_cast<WidgetPlugin*>(widget)) {
        if (!plugin->usable(std::vector<Processor*>())) // not usable without processors?
            action->setVisible(false); // will be made visible in evaluateNetwork()
        tools_ << std::make_pair(plugin, action);
    }

    if (basic) {
        if (viewMenu_)
            viewMenu_->addAction(action);
        if (viewToolBar_)
            viewToolBar_->addAction(action);
    }
    else {
        if (toolsMenu_)
            toolsMenu_->addAction(action);
        if (toolsToolBar_)
            toolsToolBar_->addAction(action);
    }

    window->setVisible(false);
    toolWindows_ << window;

    return window;
}

VoreenToolWindow* VoreenMainWindow::addToolDockWindow(QAction* action, QWidget* widget, const QString& name,
                                                      Qt::DockWidgetArea dockarea, Qt::DockWidgetAreas allowedAreas, 
                                                      bool basic)
{
    action->setCheckable(true);

    VoreenToolWindow* window = new VoreenToolWindow(action, this, widget, name, true);

    if (WidgetPlugin* plugin = dynamic_cast<WidgetPlugin*>(widget)) {
        if (!plugin->usable(std::vector<Processor*>())) // not usable without processors?
            action->setVisible(false); // will be made visible in evaluateNetwork()
        tools_ << std::make_pair(plugin, action);
    }

    if (basic) {
        if (viewMenu_)
            viewMenu_->addAction(action);
        if (viewToolBar_)
            viewToolBar_->addAction(action);
    }
    else {
        if (toolsMenu_)
            toolsMenu_->addAction(action);
        if (toolsToolBar_)
            toolsToolBar_->addAction(action);
    }

    window->setAllowedAreas(allowedAreas);
    addDockWidget(dockarea, window);
    window->setVisible(false);

    return window;
}

void VoreenMainWindow::createToolWindows() {
    // network editor
    networkEditorWidget_ = new NetworkEditor(this, vis_->getWorkspace(), vis_->getEvaluator());
    networkEditorWidget_->setWindowTitle(tr("Processor Network"));
    networkEditorWindow_ = new VoreenMdiSubWindow(networkEditorWidget_, this);
    networkEditorWindow_->setWindowState(networkEditorWindow_->windowState() | Qt::WindowFullScreen);
    mdiArea_->addSubWindow(networkEditorWindow_);
    vis_->setNetworkEditorWidget(networkEditorWidget_);

    // processor list
    processorListWidget_ = new ProcessorListWidget();
    processorListWidget_->setMinimumSize(200, 200);
    processorListAction_ = new QAction(QIcon(":/voreenve/icons/show-processors.png"), tr("Processors"), this);
    processorListTool_ = addToolDockWindow(processorListAction_, processorListWidget_, "ProcessorList", 
        Qt::LeftDockWidgetArea, Qt::LeftDockWidgetArea);
    vis_->setProcessorListWidget(processorListWidget_);

    // property list
    propertyListWidget_ = new PropertyListWidget(this, 0);
    propertyListTool_ = addToolDockWindow(new QAction(QIcon(":/voreenve/icons/show-properties.png"), tr("Properties"), this),
                                          propertyListWidget_, "Properties", Qt::RightDockWidgetArea, Qt::RightDockWidgetArea);
    vis_->setPropertyListWidget(propertyListWidget_);

    // VolumeContainerWidget
    volumeContainerWidget_ = new VolumeContainerWidget(vis_->getVolumeContainer(), this);
    volumeContainerWidget_->setMinimumSize(250, 250);
    volumeContainerTool_ = addToolDockWindow(new QAction(QIcon(":/voreenve/icons/show-volumecontainer.png"), tr("Volumes"), this),
                      volumeContainerWidget_, "VolumeContainer", Qt::RightDockWidgetArea);
    vis_->setVolumeContainerWidget(volumeContainerWidget_);

    // shortcut preferences
    shortcutPrefWidget_ =  new ShortcutPreferencesWidget(vis_->getEvaluator());
    //connect(this, SIGNAL(newNetwork(processorNetwork_)), shortcutPrefWidget_, SLOT(rebuildWidgets()));
    addToolWindow(new QAction(QIcon(":/voreenve/icons/show-keymapping.png"), tr("Input Mapping"), this),
                  shortcutPrefWidget_, tr("Show shortcut preferences"), true);

    // console
    QAction* consoleAction = new QAction(QIcon(":/voreenve/icons/show-console.png"), tr("Debug Console"), this);
    consoleAction->setShortcut(tr("Ctrl+D"));
    consoleTool_ = addToolDockWindow(consoleAction, consolePlugin_, "Console", Qt::BottomDockWidgetArea, Qt::BottomDockWidgetArea);
    consoleTool_->setAllowedAreas(Qt::BottomDockWidgetArea);
    consoleTool_->setFloating(false);
    consoleTool_->resize(700, 300);
    consoleTool_->setMinimumHeight(200);

    // render target debug window
    renderTargetDebugWidget_ = new RenderTargetDebugWidget(sharedContext_);
    renderTargetDebugWidget_->setEvaluator(vis_->getEvaluator());
    renderTargetDebugWidget_->setMinimumSize(200, 200);
    QAction* texContainerAction = new QAction(QIcon(":/voreenve/icons/show-rendertargets.png"),tr("Render Target Viewer"), this);
    texContainerAction->setShortcut(tr("Ctrl+T"));
    VoreenToolWindow* tc = addToolWindow(texContainerAction, renderTargetDebugWidget_, "Rendertargets");
    tc->resize(500, 500);                                    
                                                             
    // animation
    QAction* animationAction = new QAction(QIcon(":/voreenve/icons/make-animation.png"), tr("Animation"), this);
    AnimationPlugin* animationPlugin = new AnimationPlugin(this);
    animationAction->setShortcut(tr("Ctrl+A"));
    animationTool_ = addToolDockWindow(animationAction, animationPlugin, "Animation", Qt::RightDockWidgetArea, Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea, false);
    animationTool_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    animationTool_->setFloating(false);
    animationTool_->resize(700, 300);
    animationTool_->setMinimumHeight(200);
    vis_->setAnimationPlugin(animationPlugin);

    // segmentation
    //SegmentationPlugin* segwidget = new SegmentationPlugin(this, vis_->getEvaluator());
    //addToolWindow(new QAction(QIcon(":/voreenve/icons/segmentation.png"), tr("Segmentation"), this),
                  //segwidget, "Segmentation", false);



    // connections between tool widgets
    vis_->createConnections();

}

void VoreenMainWindow::buttonAddDICOMClicked() {

    if (!volumeContainerWidget_) {
        LWARNINGC("voreenve.VoreenMainWindow", "No volume container widget");
        return;
    }

    volumeContainerWidget_->loadDicomFiles();
}

////////// settings ////////////////////////////////////////////////////////////////////

void VoreenMainWindow::loadSettings() {
    // set defaults
    networkPath_ = VoreenApplication::app()->getNetworkPath().c_str();
    workspacePath_ = VoreenApplication::app()->getWorkspacePath().c_str();
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
        settings_.endGroup();
    }

    if (windowSize.isNull()) {
        windowMaximized = true;
    } else {
        resize(windowSize);
    }

    // ensure that the main window is restored on a visible screen
    // particular, when switching between different multi desktops modes
    QRect screenGeometry = QApplication::desktop()->screen()->geometry();

    // modify screen geometry to account maximized windows having negative position
    screenGeometry.setRect(screenGeometry.x() - 10, screenGeometry.y() - 10,
                           screenGeometry.width() + 20, screenGeometry.height() + 20);

    if (screenGeometry.contains(windowPosition) &&
        screenGeometry.contains(QPoint(windowPosition.x()+windowSize.width(),
                                       windowPosition.y()+windowSize.height())))
    {
        move(windowPosition);
    }

    if (windowMaximized)
        setWindowState(windowState() | Qt::WindowMaximized);
}

void VoreenMainWindow::loadWindowSettings() {
    // Restore visibility, position and size of tool windows from settings
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

    guiMode_ = visualizationMode ? MODE_VISUALIZATION : MODE_NETWORK;
}

void VoreenMainWindow::saveSettings() {
    // store settings
    settings_.setValue("ResetSettings", resetSettings_);

    // write version number of the config file format (might be useful someday)
    settings_.setValue("ConfigVersion", 1);

    if (guiMode_ == MODE_VISUALIZATION) {
        visualizationModeState_ = saveState(WINDOW_STATE_VERSION);
        //renderWindowStateVisualizationMode_ = renderWindow_->saveGeometry();
    } else if (guiMode_ == MODE_NETWORK) {
        networkModeState_ = saveState(WINDOW_STATE_VERSION);
        //renderWindowStateNetworkMode_ = renderWindow_->saveGeometry();
        networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
    }

    settings_.beginGroup("MainWindow");
    settings_.setValue("size", size());
    settings_.setValue("pos", pos());
    settings_.setValue("maximized", (windowState() & Qt::WindowMaximized) != 0);
    settings_.setValue("workspace", lastWorkspace_);
    settings_.setValue("loadLastWorkspace", loadLastWorkspace_);
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

    QFileDialog fileDialog(this, tr("Import Network..."),
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
        ignoreWindowTitleModified_ = true;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        vis_->openNetwork(filename.toStdString());
        showNetworkErrors();
        currentNetwork_ = filename;
        addToRecentFiles(currentNetwork_);
        QApplication::restoreOverrideCursor();
        adaptWidgetsToNetwork();
    }
    catch (SerializationException& e) {
        QApplication::restoreOverrideCursor();

        // Trying to serialize Data of old format?
        if ((typeid(e) == typeid(XmlSerializationFormatException))
            && std::string(e.what()).find("ProcessorNetwork") != std::string::npos)
        {
            QMessageBox::critical(this, tr("VoreenVE"),
                                  tr("The network file you are trying to load uses an old file format incompatible "
                                     "with this version of Voreen.\n"
                                     "Please use an older version of Voreen to save it as a workspace and then "
                                     "use the 'workspaceconverter' application to convert it to the new file format."));
        }
        else {
            QErrorMessage* errorMessageDialog = new QErrorMessage(this);
            errorMessageDialog->showMessage(e.what());
        }
    }
    ignoreWindowTitleModified_ = false;
    updateWindowTitle();
}

bool VoreenMainWindow::saveNetworkAs() {
    QFileDialog fileDialog(this, tr("Export Network..."), QDir(networkPath_).absolutePath(),
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
        } 
        catch (SerializationException& e) {
            QApplication::restoreOverrideCursor();
            QErrorMessage* errorMessageDialog = new QErrorMessage(this);
            errorMessageDialog->showMessage(e.what());
            return false;
        } 

        networkPath_ = fileDialog.directory().path();
        vis_->setModified(false);
        updateWindowTitle();
        addToRecentFiles(currentNetwork_);
        return true;
    }
    return false;
}

bool VoreenMainWindow::askSave() {
    if (vis_->isModified()) {
        switch (QMessageBox::question(this, tr("Modified Workspace"), tr("Save the current workspace?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes))
        {
        case QMessageBox::Yes:
            return saveWorkspace();
        case QMessageBox::No:
            return true;
        default:
            return false;
        }
    }
    return true;
}

void VoreenMainWindow::exportWorkspace() {
   QFileDialog fileDialog(this, tr("Export Workspace..."), QDir(workspacePath_).absolutePath(),
                           tr("Voreen workspace archives (*.zip)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    fileDialog.setDefaultSuffix("zip");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec() == false)
        return;

    try {
        vis_->exportWorkspaceAsZip(fileDialog.selectedFiles().first().toStdString());
    }
    catch (SerializationException& e) {
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        return;
    }
}

void VoreenMainWindow::extractZippedWorkspace() {
    if (!askSave())
        return;

    QFileDialog fileDialog(this, tr("Extract Workspace Archive..."),
                           QDir(workspacePath_).absolutePath(), "Voreen workspace archives (*.zip)");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Extract To..."),
        workspacePath_, (QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
        if (dir.isEmpty() == false)
            vis_->importZippedWorkspace(fileDialog.selectedFiles().first().toStdString(),
                                       dir.toStdString(), false);
    }
}

void VoreenMainWindow::newWorkspace() {
    if (!askSave())
        return;

    vis_->newWorkspace();

    currentWorkspace_ = "";
    currentNetwork_ = "";
    lastWorkspace_ = currentWorkspace_;

    propertyListWidget_->clear();
    renderTargetDebugWidget_->update();

    vis_->setModified(false);
    updateWindowTitle();
}

void VoreenMainWindow::openWorkspace(const QString& filename) {
    // disable render target widget during load
    renderTargetDebugWidget_->setEvaluator(0);
    ignoreWindowTitleModified_ = true;
    updateWindowTitle();
    try {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        qApp->processEvents();
        vis_->newWorkspace();
        qApp->processEvents();
        vis_->openWorkspace(filename);
    }
    catch (SerializationException& e) {
        currentWorkspace_ = "";
        lastWorkspace_ = currentWorkspace_;
        ignoreWindowTitleModified_ = false;
        updateWindowTitle();
        QApplication::restoreOverrideCursor();

        // Trying to serialize Data of old format?
        if ((typeid(e) == typeid(XmlSerializationFormatException))
            && std::string(e.what()).find("Workspace") != std::string::npos)
        {
            QMessageBox::critical(this, tr("Incompatible File Format"),
                                  tr("The workspace you are trying to load uses an old file format incompatible "
                                     "with this version of Voreen.\n"
                                     "Please use the 'workspaceconverter' application to convert it to "
                                     "the new file format."));
            return;
        }

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(tr("Could not open workspace:\n") + e.what());
        return;
    }

    showWorkspaceErrors();
    showNetworkErrors();

    // adjust canvas widgets (created during workspace load) to visualization mode
    if (guiMode_ == MODE_VISUALIZATION)
        adjustCanvasWidgets(MODE_VISUALIZATION);

    adaptWidgetsToNetwork();
    currentWorkspace_ = filename;
    lastWorkspace_ = currentWorkspace_;
    currentNetwork_ = "";
    addToRecentFiles(currentWorkspace_);
    vis_->setModified(false);
    ignoreWindowTitleModified_ = false;
    updateWindowTitle();
    renderTargetDebugWidget_->setEvaluator(vis_->getEvaluator());
    QApplication::restoreOverrideCursor();
}

void VoreenMainWindow::openWorkspace() {
    if (!askSave())
        return;

#ifdef VRN_WITH_ZLIB
    QFileDialog fileDialog(this, tr("Open Workspace..."),
                           QDir(workspacePath_).absolutePath(),
                           "Voreen workspaces (*.vws *.zip)");
#else
    QFileDialog fileDialog(this, tr("Open Workspace..."),
                           QDir(workspacePath_).absolutePath(),
                           "Voreen workspaces (*.vws)");
#endif

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        openWorkspace(fileDialog.selectedFiles().at(0));
        workspacePath_ = fileDialog.directory().path();
    }
}

bool VoreenMainWindow::saveWorkspace(const QString& filename) {
    QString f = filename;
    if (f.isEmpty() && !vis_->readOnlyWorkspace())
        f = currentWorkspace_;

    if (f.isEmpty())
        return saveWorkspaceAs();

    try {
        vis_->saveWorkspace(f.toStdString(), setReuseTargetsAction_->isChecked());
    }
    catch (SerializationException& e) {
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        return false;
    }

    currentWorkspace_ = f;
    lastWorkspace_ = currentWorkspace_;
    vis_->setModified(false);
    updateWindowTitle();
    addToRecentFiles(currentWorkspace_);
    return true;
}

bool VoreenMainWindow::saveWorkspaceAs() {    
    QFileDialog fileDialog(this, tr("Save Workspace As..."), QDir(workspacePath_).absolutePath());
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    
    QStringList filters;
    // do not allow standard vws-format for zipped workspaces (data sets would be missing)
    if (!QString::fromStdString(vis_->getWorkspace()->getFilename()).endsWith(".zip", Qt::CaseInsensitive))
        filters << "Voreen workspaces (*.vws)";
#ifdef VRN_WITH_ZLIB
    filters << "Voreen workspace archives (*.zip)";
#endif
    fileDialog.setNameFilters(filters);

    if (QString::fromStdString(vis_->getWorkspace()->getFilename()).endsWith(".zip", Qt::CaseInsensitive)) 
        fileDialog.setDefaultSuffix("zip");
    else
        fileDialog.setDefaultSuffix("vws");

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getWorkspacePath().c_str());
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        bool result = saveWorkspace(fileDialog.selectedFiles().at(0));
        workspacePath_ = fileDialog.directory().path();
        return result;
    }
    else {
        return false;
    }
}

void VoreenMainWindow::openDataset() {
    if (!volumeContainerWidget_) {
        LWARNINGC("voreenve.VoreenMainWindow", "No volume container widget");
        return;
    }

    volumeContainerWidget_->loadVolume();
}

void VoreenMainWindow::loadDataset(const std::string& filename) {
    if (!vis_) {
        LWARNINGC("voreenve.VoreenMainWindow", "No VoreenVisualization object");
        return;
    }

    if (!vis_->getVolumeContainer()) {
        LWARNINGC("voreenve.VoreenMainWindow", "No volume container");
        return;
    }

    try {
        vis_->getVolumeContainer()->loadVolume(filename);
    }
    catch (tgt::FileException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("voreenve.VoreenMainWindow", e.what());
    }
    catch (std::bad_alloc) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage("std::Error BAD ALLOCATION, File: " + QString::fromStdString(filename));
        LWARNINGC("voreenve.VoreenMainWindow", "std::Error BAD ALLOCATION");
    }
}

void VoreenMainWindow::openRecentFile() {
    if (!askSave())
        return;
    
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString file(action->data().toString());
        openWorkspace(file);
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

void VoreenMainWindow::showNetworkErrors() {

    // alert about errors in the Network
    std::vector<std::string> errors = vis_->getNetworkErrors();
    if (!errors.empty()) {
        QString msg;
        for (size_t i=0; i < errors.size(); i++)
            msg += "<li>" + QString(errors[i].c_str()) + "</li>\n";

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->resize(400,200);
        errorMessageDialog->showMessage(tr("There were %1 errors loading the network:\n<ul>").arg(errors.size())
                                        + msg + "\n</ul>");

        qApp->processEvents();
    }
}

void VoreenMainWindow::showWorkspaceErrors() {

    // alert about errors in the Network
    std::vector<std::string> errors = vis_->getWorkspaceErrors();
    if (!errors.empty()) {
        QString msg;
        for (size_t i=0; i < errors.size(); i++)
            msg += "<li>" + QString(errors[i].c_str()) + "</li>\n";

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->resize(400,200);
        errorMessageDialog->showMessage(tr("There were %1 errors loading the workspace:\n<ul>").arg(errors.size())
            + msg + "\n</ul>");

        qApp->processEvents();
    }
}

void VoreenMainWindow::adaptWidgetsToNetwork() {

    shortcutPrefWidget_->rebuildWidgets();

    // ask each tool whether it should be visible for these processors
    std::pair<WidgetPlugin*, QAction*> p;
    foreach (p, tools_) {
        bool usable;
        if (!vis_->getEvaluator()->getProcessorNetwork())
            usable = false;
        else
            usable = p.first->usable(vis_->getEvaluator()->getProcessorNetwork()->getProcessors());
        p.second->setVisible(usable);
        // hide tool's toolwindow if not usable
        VoreenToolWindow* toolWindow = dynamic_cast<VoreenToolWindow*>(p.first->parent());
        if (toolWindow && !usable)
            toolWindow->setVisible(false);
    }
}

////////// actions /////////////////////////////////////////////////////////////////////

//
// General
//

void VoreenMainWindow::closeEvent(QCloseEvent *event) {
    if (vis_->isModified()) {
        if (askSave()) {
            event->accept();
            emit closeMainWindow();
        } else {
            event->ignore();
            return;
        }
    }

    saveSettings();
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
    QString filename = QFileDialog::getOpenFileName(this, tr("Run Python Script..."),
                                                    VoreenApplication::app()->getScriptPath().c_str(),
                                                    "Python scripts (*.py)");
    if (!filename.isEmpty())
        runScript(filename);
#else
    QMessageBox::warning(this, "Voreen", tr("Voreen has been compiled without Python support"));
#endif // VRN_WITH_PYTHON
}
            
void VoreenMainWindow::runScript(const QString& filename) {
#ifdef VRN_WITH_PYTHON

    tgt::Singleton<VoreenPython>::getRef().setEvaluator(vis_->getEvaluator());

    tgt::Script* script = ScriptMgr.load(filename.toStdString(), false);
    if (script->compile()) {
        if (!script->run())
            QMessageBox::warning(this, "Voreen", tr("Python runtime error (see stdout)"));

    } else {
        QMessageBox::warning(this, "Voreen", tr("Python compile error (see stdout)"));
    }
    ScriptMgr.dispose(script);
    std::cout << "runscript finished" << std::endl;
#else
    QMessageBox::warning(this, "Voreen", tr("Voreen has been compiled without Python support, "
                                            "can not run '%1'").arg(filename));
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
    }
    else {
        LWARNINGC("VoreenMainWindow", "Shader reloading failed");
        #ifdef WIN32
        Beep(10000, 100);
        #endif
        QApplication::restoreOverrideCursor();
        consoleTool_->show();
        qApp->processEvents();
        QMessageBox::critical(this, tr("Shader Reloading"),
                              tr("Shader reloading failed.\n"
                                 "See the Debug Console for details."));
    }
    QApplication::restoreOverrideCursor();
}

//
// Options menu
//

void VoreenMainWindow::setLoadLastWorkspace() {
    loadLastWorkspace_ = loadLastWorkspaceAct_->isChecked();
}

void VoreenMainWindow::setReuseTargets() {
}

//
// Help menu
//

void VoreenMainWindow::helpFirstSteps() {
    QString path(VoreenApplication::app()->getDocumentationPath("gettingstarted/gsg.html").c_str());
    HelpBrowser* help = new HelpBrowser(QUrl::fromLocalFile(path), tr("VoreenVE Help"));
    help->resize(1050, 700);
    help->show();
    connect(this, SIGNAL(closeMainWindow()), help, SLOT(close()));
}

void VoreenMainWindow::helpAbout() {
    AboutBox about("VoreenVE", tr("VoreenVE Visualization Environment"), this);
    about.exec();
}

////////// further functions ///////////////////////////////////////////////////////////

void VoreenMainWindow::networkModified() {
    updateWindowTitle();
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
    QString title = tr("VoreenVE");

    if (!ignoreWindowTitleModified_ && vis_ && vis_->isModified())
        title += " *";

    if (!currentNetwork_.isEmpty() || !currentWorkspace_.isEmpty() ) {
        QFileInfo f(currentWorkspace_.isEmpty() ? currentNetwork_ : currentWorkspace_); // get filename without path
        title += " - " + f.fileName();
    }

    setWindowTitle(title);
}

void VoreenMainWindow::guiModeChanged() {
    if (modeVisualizationAction_->isChecked() && guiMode_ != MODE_VISUALIZATION)
        setGuiMode(MODE_VISUALIZATION);
    else if (modeNetworkAction_->isChecked() && guiMode_ != MODE_NETWORK)
        setGuiMode(MODE_NETWORK);
}

void VoreenMainWindow::setGuiMode(GuiMode guiMode) {
    if (guiMode == MODE_VISUALIZATION) {
        if (guiMode_ == MODE_NETWORK) {
            networkModeState_ = saveState(WINDOW_STATE_VERSION);
            networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
        }

        // hide all first to prevent some flicker
        modeVisualizationAction_->setChecked(true);
        processorListAction_->setEnabled(false);
        networkEditorWindow_->hide();
        networkEditorWidget_->setVisible(false);
        processorListTool_->hide();
        qApp->processEvents();

        if (!restoreState(visualizationModeState_, WINDOW_STATE_VERSION)) {
            propertyListTool_->show();
            volumeContainerTool_->show();
        }

        // resize canvas after gui has been settled
        qApp->processEvents();
        adjustCanvasWidgets(MODE_VISUALIZATION);

    }
    else if (guiMode == MODE_NETWORK) {
        if (guiMode_ == MODE_VISUALIZATION) {
            visualizationModeState_ = saveState(WINDOW_STATE_VERSION);
        }

        // first update canvas widget
        adjustCanvasWidgets(MODE_NETWORK);
        qApp->processEvents();

        if (!restoreState(networkModeState_, WINDOW_STATE_VERSION)) {
            processorListTool_->show();
            propertyListTool_->show();
            volumeContainerTool_->show();
        }

        if (networkEditorWindow_->restoreGeometry(networkEditorWindowState_))
            networkEditorWindow_->show();
        else
            networkEditorWindow_->showMaximized();

        networkEditorWidget_->setVisible(true); // only show now, so it immediately gets the correct size
        modeNetworkAction_->setChecked(true);
        processorListAction_->setEnabled(true);
    }

    // adjust property list widget at last, since this is quite expensive and may flicker
    qApp->processEvents();
    if (guiMode == MODE_VISUALIZATION) {
        propertyListWidget_->setState(PropertyListWidget::LIST, Property::USER);
    }
    else {
        propertyListWidget_->setState(PropertyListWidget::SELECTED, Property::DEVELOPER);
    }

    guiMode_ = guiMode;
}

void VoreenMainWindow::adjustCanvasWidgets(GuiMode guiMode) {
    // adjust canvas widgets
    if (vis_->getWorkspace() && vis_->getWorkspace()->getProcessorNetwork()) {

        const ProcessorNetwork* network = vis_->getWorkspace()->getProcessorNetwork();
        const std::vector<CanvasRenderer*>& processors = network->getProcessorsByType<CanvasRenderer>();
        for (size_t i=0; i<processors.size(); ++i) {

            ProcessorWidget* pw = processors[i]->getProcessorWidget();
            if (pw) {
                QProcessorWidget* qpw = dynamic_cast<QProcessorWidget*>(pw);
                if (qpw) {
                    if (guiMode == MODE_VISUALIZATION) {
                        canvasPos_ = qpw->getPosition();
                        canvasSize_ = qpw->getSize();
                        QMdiSubWindow* subw = mdiArea_->addSubWindow(qpw);
                        subw->showMaximized();
                    }
                    else {
                        QObject* par = qpw->parent();
                        qpw->setParent(this);
                        if (dynamic_cast<QMdiSubWindow*>(par)) {
                            mdiArea_->removeSubWindow(qpw);
                            delete par;
                        }
                        qpw->setParent(this);
                        qpw->setWindowFlags(Qt::Tool);
                        static_cast<QWidget*>(qpw)->setVisible(true);
                        if (canvasSize_.x > 0 && canvasSize_.y > 0) {
                            qpw->setSize(canvasSize_.x,  canvasSize_.y);
                            qpw->setPosition(canvasPos_.x, canvasPos_.y);
                        }
                        qpw->show();
                    }
                    return;
                }
            }
        }
    }
}

void VoreenMainWindow::adjustSnapshotMenu() {
    if (!snapshotAction_)
        return;

#ifdef VRN_WITH_DEVIL
    snapshotAction_->menu()->clear();
    if (!vis_->getWorkspace()->getProcessorNetwork()) {
        snapshotAction_->setEnabled(false);
        return;
    }

    std::vector<CanvasRenderer*> canvasRenderers
        = vis_->getWorkspace()->getProcessorNetwork()->getProcessorsByType<CanvasRenderer>();
    snapshotAction_->setEnabled(!canvasRenderers.empty());
    
    for (size_t i=0; i<canvasRenderers.size(); ++i) {
        QAction* menuAction = new QAction(QString::fromStdString(canvasRenderers[i]->getName()), this);
        // store index of canvasrenderer in action for identification
        // TODO: replace by name, when processors' names are unique
        menuAction->setData((int)i);
        snapshotAction_->menu()->addAction(menuAction);
        connect(menuAction, SIGNAL(triggered(bool)), this, SLOT(snapshotActionTriggered(bool)));
    }
#endif
}

void VoreenMainWindow::snapshotActionTriggered(bool /*triggered*/) {
    if (dynamic_cast<QAction*>(QObject::sender())) {

        int rendererIndex = static_cast<QAction*>(QObject::sender())->data().toInt();
        std::vector<CanvasRenderer*> canvasRenderers
            = vis_->getWorkspace()->getProcessorNetwork()->getProcessorsByType<CanvasRenderer>();

        if (rendererIndex >= 0 && (size_t)rendererIndex < canvasRenderers.size()) {
            CanvasRendererWidget* canvasWidget
                = dynamic_cast<CanvasRendererWidget*>(canvasRenderers[rendererIndex]->getProcessorWidget());
            if (!canvasWidget) {
                LERRORC("voreenve.mainwindow", "No canvas renderer widget");
                return;
            }
            canvasWidget->showSnapshotTool();
        }
    }
}

} // namespace
