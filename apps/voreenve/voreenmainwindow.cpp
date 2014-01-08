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

#include "voreenmainwindow.h"

#include "tgt/gpucapabilities.h"
#include "tgt/filesystem.h"
#include "tgt/logmanager.h"
#include "tgt/qt/qtcanvas.h"

#include "voreen/core/processors/processor.h"
#include "voreen/core/network/processornetwork.h"

#include "voreen/core/network/workspace.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/properties/filedialogproperty.h"

// core module is always available
#include "modules/core/processors/output/canvasrenderer.h"
#include "modules/core/qt/processor/canvasrendererwidget.h"

#include "voreen/qt/aboutbox.h"
#include "voreen/qt/helpbrowser.h"

#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/widgets/inputmappingdialog.h"
#include "voreen/qt/widgets/rendertargetviewer.h"
#include "voreen/qt/widgets/volumeviewer.h"
#include "voreen/qt/widgets/voreensettingsdialog.h"
#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/animation/animationeditor.h"
#include "voreen/qt/widgets/processorlistwidget.h"
#include "voreen/qt/widgets/propertylistwidget.h"
#include "voreen/qt/networkeditor/networkeditor.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/qt/voreenapplicationqt.h"
#include "voreenveapplication.h"
#include "voreenveplugin.h"
#include "voreenmoduleve.h"
#include "voreen/core/version.h"
#include "voreen/core/utils/stringutils.h"

#include <QDesktopServices>

namespace voreen {

namespace {

const int MAX_RECENT_FILES = 8;

// Version number of restoring state of the main window.
// Increase when incompatible changes happen.
const int WINDOW_STATE_VERSION = 15;  // V4.0

} // namespace

////////// VoreenSplashScreen //////////////////////////////////////////////////////////

VoreenSplashScreen::VoreenSplashScreen()
    : QSplashScreen()
{
    pixmap_ = new QPixmap(":/voreenve/image/splash.png");
    setPixmap(*pixmap_);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    progress_ = 0.f;
}

VoreenSplashScreen::~VoreenSplashScreen() {
    delete pixmap_;
}

void VoreenSplashScreen::drawContents(QPainter* painter) {
    // version
    QPen pen( Qt::white );
    QRect r = rect();
    QFont font = painter->font();
    font.setPointSize(11);
    painter->setFont(font);
    r.setRect(20, 94, r.width() - 10, r.height() - 10);
    std::string version = "Version " + VoreenVersion::getVersion();
    painter->setPen(pen);
    painter->drawText(r, Qt::AlignLeft, version.c_str());

    // progressbar
    r = rect();
    r.setRect(219, 253, 255, 19);
    pen.setColor(Qt::darkGray);
    painter->setPen(pen);
    painter->drawRect(r);
    QLinearGradient gradient(220, 253, 180+254*progress_, 273);
    gradient.setColorAt(0, QColor(255, 192, 0, 75));
    gradient.setColorAt(1, QColor(255, 192, 0, 175));
    painter->setBrush(gradient);
    pen.setColor(QColor(0,0,0,0));
    painter->setPen(pen);
    r.setRect(220, 254, 254*progress_, 18);
    painter->drawRect(r);
    //r.setWidth(200);
    r.setLeft(224);
    font.setPointSize(9);
    painter->setFont(font);
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, message_);

    // url
    font.setPointSize(9);
    painter->setFont(font);
    pen.setColor(Qt::lightGray);
    painter->setPen(pen);
    r = rect();
    r.setRect(3, 0, 300, r.height() - 2);
    painter->drawText(r, Qt::AlignLeft | Qt::AlignBottom, QString("voreen.uni-muenster.de"));
}

void VoreenSplashScreen::showMessage(const QString& message, qreal progress) {
    message_ = message;
    progress_ = progress;
    QSplashScreen::showMessage(message);
}

void VoreenSplashScreen::setProgress(qreal progress) {
    progress_ = progress;
    QSplashScreen::showMessage(message_);
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

const std::string VoreenMainWindow::loggerCat_("voreenve.VoreenMainWindow");

VoreenMainWindow::VoreenMainWindow(const std::string& workspace, bool noInitialWorkspace, bool resetSettings)
    : QMainWindow()
    , networkEditorWindow_(0)
    , networkEditorWidget_(0)
    , processorListWidget_(0)
    , propertyListWidget_(0)
    , volumeViewer_(0)
    , consolePlugin_(0)
    , inputMappingDialog_(0)
    , animationEditor_(0)
    , renderTargetViewer_(0)
    , guiMode_(MODE_NONE)
    , noInitialWorkspace_(noInitialWorkspace)
{

    setDockOptions(QMainWindow::AnimatedDocks); // disallow tabbed docks

    // initialialize the console early so it gets all the interesting messages
    consolePlugin_ = new ConsolePlugin(this, VoreenApplication::app()->getLogLevel());

    // if we have a stylesheet we want the fancy menu bar, please
    if (!qApp->styleSheet().isEmpty())
        setMenuBar(new FancyMenuBar());

    // clear session settings (window states, paths, ...), if specified by cmd line parameter
    if (resetSettings) {
        settings_.clear();
        LWARNING("Restored session settings");
    }
    else {
        loadSettings();
    }

    if (!workspace.empty())
        currentWorkspace_ = workspace.c_str();

    setMinimumSize(600, 400);
    setWindowIcon(QIcon(":/qt/icons/voreen-logo_64x64.png"));
    setAcceptDrops(true);

    // show tooltips (for icons in toolbar) even if the window is inactive, as often the case
    // when a canvas window is active
    setAttribute(Qt::WA_AlwaysShowToolTips);

    ignoreWindowTitleModified_ = true;
    updateWindowTitle();
}

VoreenMainWindow::~VoreenMainWindow() {
    try {
        VoreenApplication::app()->deinitialize();
    }
    catch (VoreenException& e) {
        if (tgt::LogManager::isInited())
            LERROR("Failed to deinitialize VoreenApplication: " << e.what());
        std::cerr << "Failed to deinitialize VoreenApplication: " << e.what() << std::endl;
    }
}

void VoreenMainWindow::initialize(VoreenSplashScreen* splash) {
    if (splash)
        splash->showMessage("Initializing OpenGL...", 0.50f);

    if (tgt::Singleton<tgt::LogManager>::isInited())
        LINFO("Log file: " << tgt::FileSystem::cleanupPath(LogMgr.getLogDir() + "/" + VoreenApplication::app()->getLogFile()));

    // initGL requires a valid OpenGL context
    sharedContext_ = new tgt::QtCanvas("Init Canvas", tgt::ivec2(32, 32), tgt::GLCanvas::RGBADD, this, true);
    sharedContext_->init(); //neccessary?

    // initialize OpenGL
    try {
        VoreenApplication::app()->initializeGL();
    }
    catch(VoreenException& e) {
        if (tgt::LogManager::isInited())
            LFATALC("voreenve.MainWindow", "OpenGL initialization failed: " << e.what());
        else
            std::cerr << "OpenGL initialization failed: " << e.what();

        if (splash)
            splash->close();
        qApp->processEvents();
        QMessageBox::critical(this, tr("Initialization Error"), tr("OpenGL initialization failed. Quit."));

        exit(EXIT_FAILURE);
    }

    sharedContext_->hide();

    // some hardware/driver checks
    if (!GpuCaps.isOpenGlVersionSupported(tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_2_0)) {
        if (splash)
            splash->close();
        qApp->processEvents();
        std::ostringstream glVersion;
        glVersion << GpuCaps.getGlVersion();
        QMessageBox::critical(this, tr("Incompatible OpenGL Version"),
                              tr("Voreen requires OpenGL version 2.0 or higher, which does not seem to be "
                                 "supported on this system (reported version: %1). Therefore, the application "
                                 "will most likely not work properly.").arg(glVersion.str().c_str()));
        qApp->processEvents();
    }
    // Deactivated on intel until we have a reliable detection. (stefan)
    else if ( !GpuCaps.isShaderModelSupported(tgt::GpuCapabilities::SHADER_MODEL_3) && (GpuCaps.getVendor() != tgt::GpuCapabilities::GPU_VENDOR_INTEL) ) {
        if (splash)
            splash->close();
        qApp->processEvents();
        QMessageBox::critical(this, tr("Incompatible Shader Model"),
                              tr("Voreen requires Shader Model 3 or higher, which does not seem to be "
                                 "supported on this system. Therefore, the application will most likely not "
                                 "work properly."));
        qApp->processEvents();
    }
    else if (!GpuCaps.areFramebufferObjectsSupported()) {
        if (splash)
            splash->close();
        qApp->processEvents();
        QMessageBox::critical(this, tr("Framebuffer Objects Missing"),
                              tr("Voreen uses OpenGL framebuffer objects, which do not seem to be supported "
                                 "on this system. Therefore, the application will most likely not work properly."));
        qApp->processEvents();
    }

    if (splash)
        splash->showMessage("Creating visualization...", 0.60f);

    // create visualization object
    vis_ = new VoreenVisualization(sharedContext_);

    if (splash)
        splash->showMessage("Creating user interface...", 0.80f);

    // mdi area
    mdiArea_ = new QMdiArea(this);
    mdiArea_->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, true);
    setCentralWidget(mdiArea_);

    // put network editor in mdi area
    networkEditorWidget_ = new NetworkEditor(this, vis_->getWorkspace()->getProcessorNetwork(), vis_->getEvaluator());
    networkEditorWidget_->setWindowTitle(tr("Processor Network"));
    networkEditorWindow_ = new VoreenMdiSubWindow(networkEditorWidget_, this, Qt::FramelessWindowHint);
    networkEditorWindow_->setWindowState(networkEditorWindow_->windowState() | Qt::WindowFullScreen);
    networkEditorWindow_->hide(); // hide initially to prevent flicker
    mdiArea_->addSubWindow(networkEditorWindow_);
    vis_->setNetworkEditorWidget(networkEditorWidget_);
    qApp->processEvents();

    // create tool windows now, after everything is initialized, but before window settings are restored
    createMenus();
    createToolBars();

    // create built-in tools and add plugins
    createToolWindows();
    addVEPlugins();
    qApp->processEvents();

    // signals indicating a change in network
    connect(vis_, SIGNAL(networkModified(ProcessorNetwork*)), this, SLOT(adjustScreenshotMenu()));
    connect(vis_, SIGNAL(modified()), this, SLOT(updateWindowTitle()));
    connect(vis_, SIGNAL(newNetwork(ProcessorNetwork*)), this, SLOT(adjustScreenshotMenu()));

    loadWindowSettings();

    // hide splash
    if (splash) {
        splash->showMessage("Initialization complete.", 1.0f);
        qApp->processEvents();
        splash->close();
    }

    //
    // now the GUI is complete => load initial workspace
    //
    if (!noInitialWorkspace_) {
        if (!currentWorkspace_.isEmpty()) {
            // load workspace passed as program parameter
            openWorkspace(currentWorkspace_);
        }
        else if (!lastWorkspace_.isEmpty() && loadLastWorkspace_ && startupWorkspace_) {
            // load last workspace, but only if loading has been successful last time
            openWorkspace(lastWorkspace_);
        }
        else {
            // load an initial workspace
            openWorkspace(VoreenApplication::app()->getResourcePath("workspaces/standard.vws").c_str());
        }
    }
    else {
        newWorkspace();
    }

    startupComplete("workspace");

    setGuiMode(guiMode_);
}

void VoreenMainWindow::deinitialize() {

    // save widget settings first
    saveSettings();

    // render target viewer is an OpenGL widget,
    // so destruct before OpenGL deinitialization
    delete renderTargetViewer_;
    renderTargetViewer_ = 0;

    // deinitialize plugins (deleting not necessary, since done by the Qt parent)
    for (size_t i=0; i<plugins_.size(); i++) {
        VoreenVEPlugin* plugin = plugins_.at(i);
        if (plugin->isInitialized()) {
            try {
                LINFO("Deinitializing VoreenVE plugin '" << plugin->getName() << "'");
                plugin->deinitialize();
                plugin->initialized_ = false;
            }
            catch (tgt::Exception& e) {
                LERROR("Failed to deinitialize VoreenVE plugin '" << plugin->getName() << "': " << e.what());
            }
        }
    }

    // free workspace, unregister network from widgets
    delete vis_;
    vis_ = 0;

    // finalize OpenGL
    try {
        VoreenApplication::app()->deinitializeGL();
    }
    catch (VoreenException& e) {
        if (tgt::LogManager::isInited())
            LERROR("VoreenApplication::deinitializeGL failed: " << e.what());
        std::cerr << "VoreenApplication::deinitializeGL failed: " << e.what() << std::endl;
    }

    delete sharedContext_;
    sharedContext_ = 0;
}

////////// GUI setup ///////////////////////////////////////////////////////////////////

void VoreenMainWindow::createMenus() {
    menu_ = menuBar();

    //
    // File menu
    //
    fileMenu_ = menu_->addMenu(tr("&File"));

    // Workspace
    workspaceNewAction_ = new QAction(QIcon(":/qt/icons/clear.png"), tr("&New Workspace"),  this);
    workspaceNewAction_->setShortcut(QKeySequence::New);
    connect(workspaceNewAction_, SIGNAL(triggered()), this, SLOT(newWorkspace()));
    fileMenu_->addAction(workspaceNewAction_);

    workspaceOpenAction_ = new QAction(QIcon(":/qt/icons/open.png"), tr("&Open Workspace..."),  this);
    workspaceOpenAction_->setShortcut(QKeySequence::Open);
    connect(workspaceOpenAction_, SIGNAL(triggered()), this, SLOT(openWorkspace()));
    fileMenu_->addAction(workspaceOpenAction_);

    workspaceSaveAction_ = new QAction(QIcon(":/qt/icons/save.png"), tr("&Save Workspace"),  this);
    workspaceSaveAction_->setShortcut(QKeySequence::Save);
    connect(workspaceSaveAction_, SIGNAL(triggered()), this, SLOT(saveWorkspace()));
    fileMenu_->addAction(workspaceSaveAction_);

    workspaceSaveAsAction_ = new QAction(QIcon(":/qt/icons/saveas.png"), tr("Save Workspace &As..."),  this);
    connect(workspaceSaveAsAction_, SIGNAL(triggered()), this, SLOT(saveWorkspaceAs()));
    fileMenu_->addAction(workspaceSaveAsAction_);

    workspaceSaveCopyAsAction_ = new QAction(tr("Save Workspace &Copy As..."), this);
    connect(workspaceSaveCopyAsAction_, SIGNAL(triggered()), this, SLOT(saveWorkspaceCopyAs()));
    fileMenu_->addAction(workspaceSaveCopyAsAction_);

    fileMenu_->addSeparator();

    // Network
    importNetworkAction_ = new QAction(QIcon(":/qt/icons/import.png"), tr("&Import Network..."), this);
    connect(importNetworkAction_, SIGNAL(triggered()), this, SLOT(openNetwork()));
    fileMenu_->addAction(importNetworkAction_);

    exportNetworkAction_ = new QAction(QIcon(":/qt/icons/export.png"), tr("E&xport Network..."), this);
    connect(exportNetworkAction_, SIGNAL(triggered()), this, SLOT(saveNetworkAs()));
    fileMenu_->addAction(exportNetworkAction_);

    fileMenu_->addSeparator();

    quitAction_ = new QAction(QIcon(":/qt/icons/exit.png"), tr("&Quit"), this);
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

    modeDevelopmentAction_ = new QAction(QIcon(":/voreenve/icons/development-mode.png"),
                                     tr("&Development Mode"), this);
    modeDevelopmentAction_->setCheckable(true);
    modeDevelopmentAction_->setShortcut(tr("F3"));

    modeApplicationAction_ = new QAction(QIcon(":/voreenve/icons/visualization-mode.png"),
                                           tr("&Application Mode"), this);
    modeApplicationAction_->setCheckable(true);
    modeApplicationAction_->setShortcut(tr("F4"));

    QActionGroup* guiModeGroup = new QActionGroup(this);
    guiModeGroup->addAction(modeApplicationAction_);
    guiModeGroup->addAction(modeDevelopmentAction_);
    modeApplicationAction_->setChecked(true);
    connect(guiModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(guiModeChanged()));

    viewMenu_->addAction(modeDevelopmentAction_);
    viewMenu_->addAction(modeApplicationAction_);
    viewMenu_->addSeparator();


    //
    // Tools menu
    //
    toolsMenu_ = menu_->addMenu(tr("&Tools"));

    rebuildShadersAction_ = new QAction(QIcon(":/qt/icons/rebuild.png"), tr("&Rebuild Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Rebuilds all currently used shaders"));
    rebuildShadersAction_->setToolTip(tr("Rebuild all shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));
    toolsMenu_->addAction(rebuildShadersAction_);

    screenshotAction_ = new QAction(QIcon(":/qt/icons/screenshot.png"), "&Screenshot", this);
    screenshotAction_->setData(0);
    QMenu* screenshotMenu = new QMenu(this);
    screenshotAction_->setMenu(screenshotMenu);
    screenshotAction_->setEnabled(false);
    connect(screenshotAction_, SIGNAL(triggered(bool)), this, SLOT(screenshotActionTriggered(bool)));
    toolsMenu_->addAction(screenshotAction_);


    //
    // Options menu
    //
    optionsMenu_ = menu_->addMenu(tr("&Options"));

    loadLastWorkspaceAct_ = new QAction(tr("&Load Last Workspace on Startup"), this);
    loadLastWorkspaceAct_->setCheckable(true);
    loadLastWorkspaceAct_->setChecked(loadLastWorkspace_);
    connect(loadLastWorkspaceAct_, SIGNAL(triggered()), this, SLOT(setLoadLastWorkspace()));
    optionsMenu_->addAction(loadLastWorkspaceAct_);

    // settings editor
    settingsEditor_ = new VoreenSettingsDialog(this);
    QAction* settingsAction = new QAction(QIcon(":/qt/icons/saveas.png"), tr("&Settings"), this);
    settingsAction->setCheckable(true);
    VoreenToolWindow* settingsWindow = new VoreenToolWindow(settingsAction, this, settingsEditor_, "Settings", true);
    if (optionsMenu_)
        optionsMenu_->addAction(settingsAction);
    addDockWidget(Qt::BottomDockWidgetArea, settingsWindow);
    settingsWindow->setAllowedAreas(Qt::NoDockWidgetArea);
    settingsWindow->setVisible(false);
    settingsWindow->setFloating(true);
    settingsWindow->resize(350, 600);
    settingsWindow->setMinimumSize(275, 350);
    connect(settingsEditor_, SIGNAL(closeSettings()), settingsWindow, SLOT(hide()));

    //
    // Help menu
    //
    helpMenu_ = menu_->addMenu(tr("&Help"));

    helpFirstStepsAct_ = new QAction(QIcon(":/qt/icons/help.png"), tr("&Getting Started..."), this);
    helpFirstStepsAct_->setShortcut(tr("F1"));
    connect(helpFirstStepsAct_, SIGNAL(triggered()), this, SLOT(helpFirstSteps()));
    helpMenu_->addAction(helpFirstStepsAct_);

    helpNetworkEditorAct_ = new QAction(QIcon(":/qt/icons/help.png"), tr("&Network Editor..."), this);
    connect(helpNetworkEditorAct_, SIGNAL(triggered()), this, SLOT(helpNetworkEditor()));
    helpMenu_->addAction(helpNetworkEditorAct_);

    helpAnimationAct_ = new QAction(QIcon(":/qt/icons/video_export.png"), tr("&Animation Manual..."), this);
    connect(helpAnimationAct_, SIGNAL(triggered()), this, SLOT(helpAnimation()));
    helpMenu_->addAction(helpAnimationAct_);

    helpTutorialSlidesAct_ = new QAction(QIcon(":/qt/icons/pdf.png"), tr("&Tutorial Slides..."), this);
    connect(helpTutorialSlidesAct_, SIGNAL(triggered()), this, SLOT(helpTutorialSlides()));
    helpMenu_->addAction(helpTutorialSlidesAct_);

    helpMenu_->addSeparator();

    // Add some web links to the menu. Use the redirects (in the "go" directory) to be
    // independent of website reorganization.
    QAction* websiteAct = new QAction(tr("Voreen Website..."), this);
    websiteAct->setData(tr("http://voreen.uni-muenster.de"));
    connect(websiteAct, SIGNAL(triggered()), this, SLOT(helpWebsite()));
    helpMenu_->addAction(websiteAct);

    helpMenu_->addSeparator();

    aboutAction_ = new QAction(QIcon(":/qt/icons/about.png"), tr("&About VoreenVE..."), this);
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(helpAbout()));
    helpMenu_->addAction(aboutAction_);
}

void VoreenMainWindow::createToolBars() {
#ifdef __APPLE__
    const QSize iconSize = QSize(23,23);
#endif

    // file toolbar
    fileToolBar_ = addToolBar(tr("File"));
#ifdef __APPLE__
    fileToolBar_->setIconSize(iconSize);
#endif
    fileToolBar_->setObjectName("file-toolbar");
    fileToolBar_->addAction(workspaceNewAction_);
    fileToolBar_->addAction(workspaceOpenAction_);
    fileToolBar_->addAction(workspaceSaveAction_);

    // view toolbar
    viewToolBar_ = addToolBar(tr("View"));
#ifdef __APPLE__
    viewToolBar_->setIconSize(iconSize);
#endif
    viewToolBar_->setObjectName("view-toolbar");
    QLabel* label = new QLabel(tr("   View  "));
    label->setObjectName("toolBarLabel");
    viewToolBar_->addWidget(label);

    viewToolBar_->addAction(modeDevelopmentAction_);
    viewToolBar_->addAction(modeApplicationAction_);
    viewToolBar_->addSeparator();

    // tools toolbar
    toolsToolBar_ = addToolBar(tr("Tools"));
#ifdef __APPLE__
    toolsToolBar_->setIconSize(iconSize);
#endif
    toolsToolBar_->setObjectName("tools-toolbar");
    label = new QLabel(tr("   Tools "));
    label->setObjectName("toolBarLabel");
    toolsToolBar_->addWidget(label);
    toolsToolBar_->addAction(rebuildShadersAction_);
    toolsToolBar_->addAction(screenshotAction_);

#ifdef __APPLE__ // we are on a mac system
    // HACK (Workaround) for Qt Mac Bug, makes MainWindow reappear
    // see for details:
    // http://bugreports.qt.nokia.com/browse/QTBUG-5069?page=com.atlassian.jira.plugin.system.issuetabpanels%3Aall-tabpanel
    show();
#endif

}

VoreenToolWindow* VoreenMainWindow::addToolWindow(QAction* action, QWidget* widget, const QString& name,
                                                      Qt::DockWidgetArea dockarea, Qt::DockWidgetAreas allowedAreas,
                                                      bool basic)
{
    action->setCheckable(true);
    action->setChecked(false);

    bool dockable = (allowedAreas != Qt::NoDockWidgetArea);

    VoreenToolWindow* window = new VoreenToolWindow(action, this, widget, name, dockable);

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
    if (dockable) {
        if (dockarea == Qt::NoDockWidgetArea) {
            addDockWidget(Qt::LeftDockWidgetArea, window);
            window->setFloating(true);
        }
        else {
            addDockWidget(dockarea, window);
        }
    }

    toolWindows_ << window;

    return window;
}

VoreenToolWindow* VoreenMainWindow::getToolWindow(QWidget* childWidget) const {
    if (!childWidget)
        return 0;
    foreach(VoreenToolWindow* toolWindow, toolWindows_) {
        if (toolWindow->child() == childWidget)
            return toolWindow;
    }
    return 0;
}

void VoreenMainWindow::createToolWindows() {
    // processor list
    processorListWidget_ = new ProcessorListWidget(this);
    processorListWidget_->setMinimumSize(200, 200);
    QAction* processorListAction = new QAction(QIcon(":/qt/icons/processors.png"), tr("&Processors"), this);
    addToolWindow(processorListAction, processorListWidget_, "ProcessorList",
        Qt::LeftDockWidgetArea, Qt::LeftDockWidgetArea);
    vis_->setProcessorListWidget(processorListWidget_);

    // property list
    propertyListWidget_ = new PropertyListWidget(this, 0);
    addToolWindow(new QAction(QIcon(":/qt/icons/properties.png"), tr("P&roperties"), this),
                                          propertyListWidget_, "Properties", Qt::RightDockWidgetArea, Qt::RightDockWidgetArea);
    vis_->setPropertyListWidget(propertyListWidget_);

    // VolumeViewre
    volumeViewer_ = new VolumeViewer(this);
    volumeViewer_->setNetworkEvaluator(vis_->getEvaluator());
    addToolWindow(new QAction(QIcon(":/qt/icons/volumes.png"), tr("V&olume Viewer"), this),
                      volumeViewer_, "Volume Viewer", Qt::RightDockWidgetArea);
    vis_->setVolumeViewer(volumeViewer_);

    // console (note: has been created in constructor!)
    QAction* consoleAction = new QAction(QIcon(":/qt/icons/console.png"), tr("&Debug Console"), this);
    consoleAction->setShortcut(tr("Ctrl+D"));
    VoreenToolWindow* consoleTool = addToolWindow(consoleAction, consolePlugin_, "Console", Qt::BottomDockWidgetArea, Qt::BottomDockWidgetArea);
    consoleTool->setMinimumHeight(100);
    //consoleTool->resize(700, 150);

    // input mapping
    inputMappingDialog_ =  new InputMappingDialog(this, vis_->getWorkspace()->getProcessorNetwork());
    QAction* inputMappingAction = new QAction(QIcon(":/qt/icons/keymapping.png"), tr("&Input Mapping"), this);
    inputMappingAction->setShortcut(tr("Ctrl+I"));
    VoreenToolWindow* inputWindow = addToolWindow(inputMappingAction, inputMappingDialog_, tr("Show shortcut preferences"),
        Qt::NoDockWidgetArea, Qt::NoDockWidgetArea, true);
    inputWindow->resize(520, 400);
    vis_->setInputMappingDialog(inputMappingDialog_);

    // render target debug window
    renderTargetViewer_ = new RenderTargetViewer(sharedContext_);
    renderTargetViewer_->setEvaluator(vis_->getEvaluator());
    renderTargetViewer_->setMinimumSize(200, 200);
    QAction* texContainerAction = new QAction(QIcon(":/qt/icons/rendertargets.png"),tr("Render &Target Viewer"), this);
    texContainerAction->setShortcut(tr("Ctrl+T"));
    VoreenToolWindow* tc = addToolWindow(texContainerAction, renderTargetViewer_, "RenderTargetViewer",
        Qt::NoDockWidgetArea, Qt::NoDockWidgetArea);
    tc->widget()->setContentsMargins(0,0,0,0);
    tc->widget()->layout()->setContentsMargins(0,0,0,0);
    tc->resize(500, 500);
    vis_->setRenderTargetViewer(renderTargetViewer_);

    // animation editor
    animationEditor_ = new AnimationEditor(vis_->getEvaluator(), vis_->getWorkspace(), this);
    VoreenToolWindow* animationWindow = addToolWindow(new QAction(QIcon(":/qt/icons/video.png"), tr("&Animation"), this), animationEditor_, "Animation",
                      Qt::NoDockWidgetArea, Qt::BottomDockWidgetArea, false);
    animationWindow->resize(925, 400);

    // create connections between tool widgets
    vis_->createConnections();
}

void VoreenMainWindow::addVEPlugins() {
    // retrieve VoreenVE plugins from application/modules
    VoreenVEApplication* veApp = VoreenVEApplication::veApp();
    if (!veApp) {
        LERROR("VoreenVEApplication not instantiated");
    }
    else {
        std::vector<std::string> pluginNames;
        for (size_t i=0; i<veApp->getVEModules().size(); i++) {
            VoreenModuleVE* veModule = veApp->getVEModules().at(i);
            const std::vector<VoreenVEPlugin*>& modPlugins = veModule->getVoreenVEPlugins();
            for (size_t j=0; j<modPlugins.size(); j++) {
                plugins_.push_back(modPlugins.at(j));
                pluginNames.push_back(modPlugins.at(j)->getName());
            }
        }
        LINFO("VoreenVE plugins: " << strJoin(pluginNames, ", "));
    }

    /// Initialize all VoreenVEPlugins and add them to the main window
    for (size_t i=0; i<plugins_.size(); i++) {
        VoreenVEPlugin* plugin = plugins_.at(i);

        QAction* action = new QAction(plugin->getIcon(), QString::fromStdString(plugin->getName()), this);
        VoreenToolWindow* pluginWindow = addToolWindow(action, plugin, QString::fromStdString(plugin->getName()),
            plugin->getInitialDockWidgetArea(), plugin->getAllowedDockWidgetAreas(), false);

        plugin->setParentWindow(pluginWindow);
        plugin->setMainWindow(this);
        plugin->setNetworkEvaluator(vis_->getEvaluator());
        plugin->createWidgets();

        try {
            LINFO("Initializing VoreenVE plugin '" << plugin->getName() << "'");
            plugin->initialize();
            plugin->initialized_ = true;
        }
        catch (tgt::Exception& e) {
            LERROR("Failed to initialize VoreenVE plugin '" << plugin->getName() << "': " << e.what());
            toolWindows_.removeOne(pluginWindow);
            delete pluginWindow;
        }
    }
}

////////// settings ////////////////////////////////////////////////////////////////////

void VoreenMainWindow::loadSettings() {
    // set defaults
    networkPath_ = VoreenApplication::app()->getUserDataPath("networks").c_str();
    workspacePath_ = VoreenApplication::app()->getUserDataPath("workspaces").c_str();
    QSize windowSize = QSize(0, 0);
    QPoint windowPosition = QPoint(0, 0);
    bool windowMaximized = true;

    // restore settings
    settings_.beginGroup("MainWindow");
    windowSize = settings_.value("size", windowSize).toSize();
    windowPosition = settings_.value("pos", windowPosition).toPoint();
    windowMaximized = settings_.value("maximized", windowMaximized).toBool();
    lastWorkspace_ = settings_.value("workspace", "").toString();
    loadLastWorkspace_ = settings_.value("loadLastWorkspace", true).toBool();
    applicationModeState_ = settings_.value("visualizationModeState").toByteArray();
    developmentModeState_ = settings_.value("networkModeState").toByteArray();
    networkEditorWindowState_ = settings_.value("networkEditorWindowState").toByteArray();
    settings_.endGroup();

    settings_.beginGroup("Paths");
    networkPath_ = settings_.value("network", networkPath_).toString();
    workspacePath_ = settings_.value("workspace", workspacePath_).toString();
    settings_.endGroup();

    settings_.beginGroup("Startup");
    // load last startup values
    startupWorkspace_ = settings_.value("workspace", true).toBool();
    // set default values for the current startup
    settings_.setValue("workspace", false);
    settings_.endGroup();

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
    for (int i=0; i < toolWindows_.size(); ++i) {
        if (!toolWindows_[i]->objectName().isEmpty()) {
            toolWindows_[i]->setVisible(false);
        }
    }

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

    settings_.beginGroup("MainWindow");
    bool applicationMode = settings_.value("visualizationMode").toBool();
    settings_.endGroup();

    guiMode_ = applicationMode ? MODE_APPLICATION : MODE_DEVELOPMENT;
}

void VoreenMainWindow::saveSettings() {
    // write version number of the config file format (might be useful someday)
    settings_.setValue("ConfigVersion", 1);

    if (guiMode_ == MODE_APPLICATION) {
        applicationModeState_ = saveState(WINDOW_STATE_VERSION);
    }
    else if (guiMode_ == MODE_DEVELOPMENT) {
        developmentModeState_ = saveState(WINDOW_STATE_VERSION);
        networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
    }

    settings_.beginGroup("MainWindow");
    settings_.setValue("size", size());
    settings_.setValue("pos", pos());
    settings_.setValue("maximized", (windowState() & Qt::WindowMaximized) != 0);
    settings_.setValue("workspace", lastWorkspace_);
    settings_.setValue("loadLastWorkspace", loadLastWorkspace_);
    settings_.setValue("visualizationModeState", applicationModeState_);
    settings_.setValue("networkModeState", developmentModeState_);
    settings_.setValue("networkEditorWindowState", networkEditorWindowState_);
    settings_.setValue("visualizationMode", (guiMode_ == MODE_APPLICATION));
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

void VoreenMainWindow::startupComplete(const std::string& phaseName) {
    settings_.beginGroup("Startup");
    settings_.setValue(phaseName.c_str(), true);
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
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
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
        vis_->importNetwork(filename);
        showNetworkErrors();
        QApplication::restoreOverrideCursor();
        adaptWidgetsToNetwork();
    }
    catch (SerializationException& e) {
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
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
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        try {
            vis_->exportNetwork(fileDialog.selectedFiles().at(0));
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
        return true;
    }
    return false;
}

bool VoreenMainWindow::askSave() {
    if (vis_ && vis_->isModified()) {
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

void VoreenMainWindow::newWorkspace() {
    if (!askSave())
        return;

    vis_->newWorkspace();

    currentWorkspace_ = "";
    currentWorkspaceWorkDir_ = "";
    lastWorkspace_ = currentWorkspace_;

    if (propertyListWidget_)
        propertyListWidget_->clear();
    if (renderTargetViewer_)
        renderTargetViewer_->update();

    vis_->setModified(false);
    updateWindowTitle();
    if (animationEditor_)
        animationEditor_->setWorkspace(vis_->getWorkspace());

    for (size_t i=0; i<plugins_.size(); i++)
        if (plugins_[i]->isInitialized())
            plugins_[i]->setWorkspace(vis_->getWorkspace());
}

void VoreenMainWindow::openWorkspace(const QString& filename) {

    // disable render target widget during load
    if (renderTargetViewer_)
        renderTargetViewer_->setEvaluator(0);

    vis_->newWorkspace();
    qApp->processEvents();

    // check if workspace to load is a test workspace (i.e. located inside a */test/* subdirectory)
    // if so, query user whether the test data path should be used as workDir
    QString workDir;
    std::vector<std::string> pathComponents = tgt::FileSystem::splitPath(filename.toStdString());
    if (std::find(pathComponents.begin(), pathComponents.end(), "test") != pathComponents.end()) {
        tgtAssert(VoreenApplication::app(), "Voreen app not instantiated");
        if (VoreenApplication::app()->getTestDataPath() != "") {
            QMessageBox::StandardButton button =
                QMessageBox::question(this, "Regression Test Workspace",
                    "You are apparently opening a regression test workspace. "
                    "Should the test data directory be used as working directory for this workspace?",
                    static_cast<QMessageBox::StandardButtons>(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
            if (button == QMessageBox::Yes)
                workDir = QString::fromStdString(VoreenApplication::app()->getTestDataPath());
        }
        else {
            QMessageBox::warning(this, "Regression Test Workspace",
                "You are apparently opening a regression test workspace, "
                "but the test data directory has not been specified in the application settings. "
                "Therefore, the file paths inside the workspace will most likely be invalid.");
        }
    }

    // open vws workspace
    try {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        qApp->processEvents();
        vis_->openWorkspace(filename, workDir);
    }
    catch (SerializationException& e) {
        vis_->newWorkspace();
        setUpdatesEnabled(true);
        currentWorkspace_ = "";
        currentWorkspaceWorkDir_ = "";
        lastWorkspace_ = currentWorkspace_;
        QApplication::restoreOverrideCursor();

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(tr("Could not open workspace:\n") + e.what());
        return;
    }

    ignoreWindowTitleModified_ = false;
    updateWindowTitle();

    // adjust canvas widgets (created during workspace load) to application mode
    if (guiMode_ == MODE_APPLICATION) {
        adjustCanvasWidgets(MODE_APPLICATION);
        setGuiMode(MODE_APPLICATION);
    }

    showWorkspaceErrors();
    showNetworkErrors();

    adaptWidgetsToNetwork();
    currentWorkspace_ = filename;
    currentWorkspaceWorkDir_ = workDir;
    lastWorkspace_ = currentWorkspace_;
    addToRecentFiles(currentWorkspace_);
    vis_->setModified(false);
    ignoreWindowTitleModified_ = false;
    updateWindowTitle();
    if (renderTargetViewer_)
        renderTargetViewer_->setEvaluator(vis_->getEvaluator());
    QApplication::restoreOverrideCursor();

    if (animationEditor_)
        animationEditor_->setWorkspace(vis_->getWorkspace());

    for (size_t i=0; i<plugins_.size(); i++)
        plugins_.at(i)->setWorkspace(vis_->getWorkspace());

}

void VoreenMainWindow::openWorkspace() {
    if (!askSave())
        return;

    QFileDialog fileDialog(this, tr("Open Workspace..."),
                           QDir(workspacePath_).absolutePath());

    QStringList filters;
    filters << "Voreen workspaces (*.vws)";
    fileDialog.setNameFilters(filters);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("workspaces").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        QString file = fileDialog.selectedFiles().at(0);
        openWorkspace(file);
        workspacePath_ = fileDialog.directory().path();
    }
}

bool VoreenMainWindow::saveWorkspace(const QString& filename) {
    QString f = filename;
    if (f.isEmpty() && !vis_->readOnlyWorkspace())
        f = currentWorkspace_;

    if (f.isEmpty())
        return saveWorkspaceAs();

    if (currentWorkspaceWorkDir_.isEmpty()) {
        // check if workspace is saved as a test workspace (i.e. located inside a */test/* subdirectory)
        // if so, query user whether the test data path should be used as workDir
        QString workDir;
        std::vector<std::string> pathComponents = tgt::FileSystem::splitPath(f.toStdString());
        if (std::find(pathComponents.begin(), pathComponents.end(), "test") != pathComponents.end()) {
            tgtAssert(VoreenApplication::app(), "Voreen app not instantiated");
            if (VoreenApplication::app()->getTestDataPath() != "") {
                QMessageBox::StandardButton button =
                    QMessageBox::question(this, "Regression Test Workspace",
                        "You are apparently saving a regression test workspace. "
                        "Should the test data directory be used as working directory for this workspace?",
                        static_cast<QMessageBox::StandardButtons>(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
                if (button == QMessageBox::Yes)
                    currentWorkspaceWorkDir_ = QString::fromStdString(VoreenApplication::app()->getTestDataPath());
            }
            else {
                QMessageBox::warning(this, "Regression Test Workspace",
                    "You are apparently saving a regression test workspace, "
                    "but the test data directory has not been specified in the application settings. "
                    "Therefore, the file paths inside the workspace will most likely be invalid.");
            }
        }
    }

    //safe development position of first canvas
    if (vis_->getWorkspace() && vis_->getWorkspace()->getProcessorNetwork()) {
       const ProcessorNetwork* network = vis_->getWorkspace()->getProcessorNetwork();
       const std::vector<CanvasRenderer*>& processors = network->getProcessorsByType<CanvasRenderer>();

       for (size_t i=0; i < processors.size(); ++i) {
           ProcessorWidget* pw = processors[i]->getProcessorWidget();
           if (pw) {
               QProcessorWidget* qpw = dynamic_cast<QProcessorWidget*>(pw);
               if (qpw) {
                   //save only if we are in development mode
                   if(guiMode_ == MODE_DEVELOPMENT){
                       IVec2MetaData* sizeMeta = new IVec2MetaData(qpw->getSize());
                       processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasSize",sizeMeta);
                       IVec2MetaData* positionMeta = new IVec2MetaData(qpw->getPosition());
                       processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasPosition",positionMeta);
                       BoolMetaData* fullscreenMeta = new BoolMetaData(qpw->isFullScreen());
                       processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasFS",fullscreenMeta);
                   }
               }
           }
           break;
       }
    }

    try {
        vis_->blockSignals(true);
        vis_->saveWorkspace(f, true, currentWorkspaceWorkDir_);
        vis_->blockSignals(false);
        currentWorkspace_ = f;
        lastWorkspace_ = currentWorkspace_;
        vis_->setModified(false);
        updateWindowTitle();
        addToRecentFiles(currentWorkspace_);
    }
    catch (SerializationException& e) {
        vis_->blockSignals(false);
        QApplication::restoreOverrideCursor();
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->setWindowTitle(tr("Workspace serialization failed"));
        errorMessageDialog->showMessage(e.what());
        return false;
    }

    return true;
}

bool VoreenMainWindow::saveWorkspaceAs() {
    QFileDialog fileDialog(this, tr("Save Workspace As..."), QDir(workspacePath_).absolutePath());
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);

    QStringList filters;
    filters << "Voreen workspaces (*.vws)";
    fileDialog.setNameFilters(filters);
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getResourcePath("workspaces").c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getUserDataPath().c_str());
    urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("modules").c_str());
    if (QDir(VoreenApplication::app()->getBasePath("custommodules").c_str()).exists())
        urls << QUrl::fromLocalFile(VoreenApplication::app()->getBasePath("custommodules").c_str());
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
    urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    fileDialog.setSidebarUrls(urls);

    if (fileDialog.exec()) {
        currentWorkspaceWorkDir_ = "";
        bool result;
        QString name = fileDialog.selectedFiles().at(0);
        if (!name.endsWith(".vws"))
            result = saveWorkspace(fileDialog.selectedFiles().at(0) + ".vws");
        else
            result = saveWorkspace(fileDialog.selectedFiles().at(0));
        workspacePath_ = fileDialog.directory().path();
        return result;
    }
    else {
        return false;
    }
}

bool VoreenMainWindow::saveWorkspaceCopyAs() {
    QString current = currentWorkspace_;
    bool success = saveWorkspaceAs();
    if (success) {
        currentWorkspace_ = current;
        vis_->setModified(true);
        updateWindowTitle();
    }
    return success;
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
        for (size_t i=0; i < errors.size(); i++) {
            msg += "<li>" + QString(errors[i].c_str()) + "</li>\n";
            LWARNING(errors[i]);
        }

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->resize(600, 300);
        errorMessageDialog->setWindowTitle(tr("Network Deserialization"));
        errorMessageDialog->showMessage(tr("There were <b>%1 errors</b> loading the network:\n<ul>").arg(errors.size())
                                        + msg + "\n</ul>");

        qApp->processEvents();
    }
}

void VoreenMainWindow::showWorkspaceErrors() {

    // alert about errors in the Network
    std::vector<std::string> errors = vis_->getWorkspaceErrors();
    if (!errors.empty()) {
        QString msg;
        for (size_t i=0; i < errors.size(); i++) {
            msg += "<li>" + QString(errors[i].c_str()) + "</li>\n";
            LWARNING(errors[i]);
        }

        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->resize(600, 300);
        errorMessageDialog->setWindowTitle(tr("Workspace Deserialization"));
        errorMessageDialog->showMessage(tr("There were <b>%1 errors</b> loading the workspace %2:\n<ul>").arg(
            errors.size()).arg(QString::fromStdString(vis_->getWorkspace() ? vis_->getWorkspace()->getFilename() : ""))
            + msg + "\n</ul>");

        qApp->processEvents();
    }
}

void VoreenMainWindow::adaptWidgetsToNetwork() {
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

    deinitialize();
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

void VoreenMainWindow::rebuildShaders() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (vis_->rebuildShaders()) {
        LINFO("Shaders reloaded");
        #ifdef WIN32
        Beep(100, 100);
        #endif
    }
    else {
        LWARNING("Shader reloading failed");
        #ifdef WIN32
        Beep(10000, 100);
        #endif
        QApplication::restoreOverrideCursor();
        //consoleTool_->show();
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
    QDesktopServices::openUrl(QUrl("http://voreen.uni-muenster.de/?q=user-interface"));
}

void VoreenMainWindow::helpNetworkEditor() {
    QDesktopServices::openUrl(QUrl("http://voreen.uni-muenster.de/?q=network-editor"));
}

void VoreenMainWindow::helpTutorialSlides() {
    QString path(VoreenApplication::app()->getBasePath("doc/vis2010-tutorial-slides.pdf").c_str());
    QDesktopServices::openUrl(QUrl(QString::fromStdString("file:///") + path, QUrl::TolerantMode));
}

void VoreenMainWindow::helpAnimation() {
    QString path(VoreenApplication::app()->getBasePath("doc/animation/animation.html").c_str());
    HelpBrowser* help = new HelpBrowser(QUrl::fromLocalFile(path), tr("VoreenVE Animation Manual"));
    help->resize(1050, 700);
    help->show();
    connect(this, SIGNAL(closeMainWindow()), help, SLOT(close()));
}

void VoreenMainWindow::helpWebsite() {
    // URL is stored in data portion of the sender
    if (QAction* act = dynamic_cast<QAction*>(sender()))
        QDesktopServices::openUrl(QUrl(act->data().toString()));
}

void VoreenMainWindow::helpAbout() {
    AboutBox about("VoreenVE", tr("VoreenVE Visualization Environment"), this);
    about.exec();
}

////////// further functions ///////////////////////////////////////////////////////////

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

    if (!currentWorkspace_.isEmpty() ) {
        QFileInfo f(currentWorkspace_); // get filename without path
        title += " - " + f.fileName();
    }

    setWindowTitle(title);
}

void VoreenMainWindow::guiModeChanged() {
    if (modeApplicationAction_->isChecked() && guiMode_ != MODE_APPLICATION)
        setGuiMode(MODE_APPLICATION);
    else if (modeDevelopmentAction_->isChecked() && guiMode_ != MODE_DEVELOPMENT)
        setGuiMode(MODE_DEVELOPMENT);
}

void VoreenMainWindow::setGuiMode(GuiMode guiMode) {
    if (guiMode == MODE_APPLICATION) {
        if (guiMode_ == MODE_DEVELOPMENT) {
            developmentModeState_ = saveState(WINDOW_STATE_VERSION);
            networkEditorWindowState_ = networkEditorWindow_->saveGeometry();
        }

        setUpdatesEnabled(false);

        // hide all first to prevent some flicker
        modeApplicationAction_->setChecked(true);
        networkEditorWindow_->hide();
        networkEditorWidget_->setVisible(false);
        if (getToolWindow(processorListWidget_))
            getToolWindow(processorListWidget_)->hide();
//        qApp->processEvents(); //TODO: seems unneccessary

        if (!restoreState(applicationModeState_, WINDOW_STATE_VERSION)) {
            if (getToolWindow(processorListWidget_) && getToolWindow(processorListWidget_)->isEnabled()) {
                if (getToolWindow(propertyListWidget_))
                    getToolWindow(propertyListWidget_)->show();
            }
            if (getToolWindow(volumeViewer_) && getToolWindow(volumeViewer_)->isEnabled())
                getToolWindow(volumeViewer_)->show();
            if (getToolWindow(consolePlugin_))
                getToolWindow(consolePlugin_)->hide();
        }
        setUpdatesEnabled(true);

        // resize canvas after gui has been settled
//        qApp->processEvents(); //TODO: seems unneccessary
        adjustCanvasWidgets(MODE_APPLICATION);
    }
    else if (guiMode == MODE_DEVELOPMENT) {
        if (guiMode_ == MODE_APPLICATION)
            applicationModeState_ = saveState(WINDOW_STATE_VERSION);

        // first update canvas widget
        adjustCanvasWidgets(MODE_DEVELOPMENT);

//        qApp->processEvents(); //TODO: seems unneccessary

        if (!restoreState(developmentModeState_, WINDOW_STATE_VERSION)) {
            if (getToolWindow(processorListWidget_))
                getToolWindow(processorListWidget_)->show();
            if (getToolWindow(propertyListWidget_))
                getToolWindow(propertyListWidget_)->show();
            if (getToolWindow(volumeViewer_))
                getToolWindow(volumeViewer_)->show();
            if (getToolWindow(consolePlugin_))
                getToolWindow(consolePlugin_)->show();
        }

        setUpdatesEnabled(false);

        if (networkEditorWindow_->restoreGeometry(networkEditorWindowState_))
            networkEditorWindow_->show();
        else
             networkEditorWindow_->showMaximized();

        networkEditorWidget_->setVisible(true); // only show now, so it immediately gets the correct size
        modeDevelopmentAction_->setChecked(true);

        setUpdatesEnabled(true);
    }

    setUpdatesEnabled(false);

    // adjust property list widget at last, since this is quite expensive and may flicker
    if (propertyListWidget_) {
        if (guiMode == MODE_APPLICATION)
            propertyListWidget_->setState(PropertyListWidget::LIST, Property::USER);
        else
            propertyListWidget_->setState(PropertyListWidget::SELECTED, Property::DEVELOPER);
    }

    setUpdatesEnabled(true);
    guiMode_ = guiMode;
}

void VoreenMainWindow::adjustCanvasWidgets(GuiMode guiMode) {
    // adjust canvas widgets
    if (vis_->getWorkspace() && vis_->getWorkspace()->getProcessorNetwork()) {
        const ProcessorNetwork* network = vis_->getWorkspace()->getProcessorNetwork();
        const std::vector<CanvasRenderer*>& processors = network->getProcessorsByType<CanvasRenderer>();

        for (size_t i=0; i < processors.size(); ++i) {
            ProcessorWidget* pw = processors[i]->getProcessorWidget();
            if (pw) {
                QProcessorWidget* qpw = dynamic_cast<QProcessorWidget*>(pw);
                if (qpw) {
                    if (guiMode == MODE_APPLICATION) {
                        //save only on switch from dev to app
                        if(guiMode_ == MODE_DEVELOPMENT){
                            IVec2MetaData* sizeMeta = new IVec2MetaData(qpw->getSize());
                            processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasSize",sizeMeta);
                            IVec2MetaData* positionMeta = new IVec2MetaData(qpw->getPosition());
                            processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasPosition",positionMeta);
                            BoolMetaData* fullscreenMeta = new BoolMetaData(qpw->isFullScreen());
                            processors[i]->getMetaDataContainer().addMetaData("preferedDevelopmentCanvasFS",fullscreenMeta);
                        }
                        QMdiSubWindow* subw = mdiArea_->addSubWindow(qpw);
                        subw->showMaximized();
                        //disable bool property
                        Property* prop = processors[i]->getProperty("showFullScreen");
                        if(prop) prop->setWidgetsEnabled(false);
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
                        //restore canvas. if started in app mode, old values are loaded
                        if(guiMode_ == MODE_APPLICATION){
                            IVec2MetaData* positionMeta = dynamic_cast<IVec2MetaData*>(processors[i]->getMetaDataContainer().getMetaData("preferedDevelopmentCanvasPosition"));
                            if (!positionMeta)
                                LDEBUGC("voreenmainwindow", "adjustCanvasWidgets(): No meta data object returned");
                            else
                                qpw->setPosition(positionMeta->getValue().x, positionMeta->getValue().y);
                            IVec2MetaData* sizeMeta = dynamic_cast<IVec2MetaData*>(processors[i]->getMetaDataContainer().getMetaData("preferedDevelopmentCanvasSize"));
                            if (!sizeMeta)
                                LDEBUGC("voreenmainwindow", "adjustCanvasWidgets(): No meta data object returned");
                            else
                                qpw->setSize(sizeMeta->getValue().x, sizeMeta->getValue().y);
                            BoolMetaData* fullscreenMeta = dynamic_cast<BoolMetaData*>(processors[i]->getMetaDataContainer().getMetaData("preferedDevelopmentCanvasFS"));
                            if (!sizeMeta)
                                LDEBUGC("voreenmainwindow", "adjustCanvasWidgets(): No meta data object returned");
                            else
                                if(fullscreenMeta->getValue())
                                    qpw->setWindowState(windowState() | Qt::WindowFullScreen);
                        }
                        qpw->show();
                        //enable bool property
                        Property* prop = processors[i]->getProperty("showFullScreen");
                        if(prop) prop->setWidgetsEnabled(true);
                    }
                    return;
                }
            }
        }
    }
}

void VoreenMainWindow::adjustScreenshotMenu() {
#ifdef VRN_MODULE_DEVIL
    if (!screenshotAction_)
        return;

    screenshotAction_->menu()->clear();

    if (!vis_->getWorkspace()->getProcessorNetwork()) {
        screenshotAction_->setEnabled(false);
        return;
    }

    screenshotAction_->setEnabled(true);
    std::vector<CanvasRenderer*> canvasRenderers
        = vis_->getWorkspace()->getProcessorNetwork()->getProcessorsByType<CanvasRenderer>();

    for (size_t i=0; i < canvasRenderers.size(); ++i) {
        QAction* menuAction = new QAction(QString::fromStdString(canvasRenderers[i]->getID()), this);
        // store index of canvasrenderer in action for identification
        // TODO: replace by name, when processors' names are unique
        menuAction->setData((int)i);
        screenshotAction_->menu()->addAction(menuAction);
        connect(menuAction, SIGNAL(triggered(bool)), this, SLOT(screenshotActionTriggered(bool)));
    }

    // add network screenshot functionality
    QAction* networkAction = new QAction(tr("Network Graph"), this);
    networkAction->setData(-1);
    screenshotAction_->menu()->addSeparator();
    screenshotAction_->menu()->addAction(networkAction);
    connect(networkAction, SIGNAL(triggered(bool)), this, SLOT(screenshotActionTriggered(bool)));
#endif
}

void VoreenMainWindow::screenshotActionTriggered(bool /*triggered*/) {
    if (dynamic_cast<QAction*>(QObject::sender())) {
        int rendererIndex = static_cast<QAction*>(QObject::sender())->data().toInt();
        std::vector<CanvasRenderer*> canvasRenderers
            = vis_->getWorkspace()->getProcessorNetwork()->getProcessorsByType<CanvasRenderer>();

        if (rendererIndex >= 0 && (size_t)rendererIndex < canvasRenderers.size()) {
            // screenshot of canvas
            CanvasRendererWidget* canvasWidget
                = dynamic_cast<CanvasRendererWidget*>(canvasRenderers[rendererIndex]->getProcessorWidget());
            if (!canvasWidget) {
                LERROR("No canvas renderer widget");
                return;
            }
            canvasWidget->showScreenshotTool();
        }
        else if (rendererIndex == -1) {
            // screenshot of network graph
            NetworkScreenshotPlugin* screenshotTool_ = new NetworkScreenshotPlugin(this, networkEditorWidget_);
            screenshotTool_->adjustSize();
            screenshotTool_->setFixedSize(screenshotTool_->sizeHint());
            screenshotTool_->show();
        }
    }
}


} // namespace
