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

#include "voreen/qt/voreenmainframe.h"

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/datvolumewriter.h"

#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/vis/exception.h"
#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/trackballnavigation.h"

#include "voreen/qt/widgets/widgetgenerator.h"
#include "voreen/qt/widgets/widgetplugin.h"
#include "voreen/qt/widgets/informationplugin.h"
#include "voreen/qt/widgets/plugindialog.h"
#include "voreen/core/cmdparser/commandlineparser.h"

#include "tgt/init.h"

#include <QToolBar>

using namespace voreen;

VoreenMainframe::VoreenMainframe(QWidget *parent, std::string name)
    : QMainWindow(parent)
    , MessageReceiver(name)
    , loggerCat_(name)
    , cmdLineParser_(0)
    , painter_(0)
    , infoPlugin_(0)
{
#ifndef __APPLE__
    move(0, 0);
#endif

    setWindowTitle(tr("Voreen"));
    setWindowIcon(QIcon("icons/icon-64.png"));

    // initialize canvases
    canvas3D_ = new tgt::QtCanvas("", tgt::ivec2(1, 1), tgt::GLCanvas::RGBADD, 0, true, 0);
    canvas3D_->setUpdatesEnabled(false); // disable repaints until GUI is fully initialized
//     canvas3D_->getEventHandler()->addListenerToFront(canvas3D_);

    setCentralWidget(canvas3D_);

    volumeSerializer_ = volumeSerializerPopulator_.getVolumeSerializer();

    fileDialogDir_ = QDir("../../data");
    filters_ << tr("Volume data (*.DAT *.I4D *.PVM *.RDM *.RDI *.HDR *.SW *.SEG *.TUV "
                    "*.ZIP *.TIFF *.TIF *.MAT *.HV *.NRRD *.NHDR)");
}

VoreenMainframe::~VoreenMainframe() {
    deinit();
    MsgDistr.remove(painter_);
    delete painter_;
}

void VoreenMainframe::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
    MessageReceiver::processMessage(msg, dest);
    MsgDistr.processMessage(msg, dest);

    if (msg->id_ == "main.dataset.load")
        fileOpen(QStringList(QString(msg->getValue<std::string>().c_str())), false);
}

void VoreenMainframe::init(QStringList args) {

    cmdLineParser_ = new CommandlineParser();
	std::vector<std::string> arguments;

	for(int i=0; i<args.size(); ++i)
		arguments.push_back(args[i].toStdString());
	cmdLineParser_->setCommandLine(arguments);

    MsgDistr.setAllowRepaints(false); // disallow all repaints until fully initialized -
                                      // prevents some bad GFX driver crashes on Linux

    tgt::Trackball* track = new tgt::Trackball(canvas3D_, false);
    painter_ = new VoreenPainter(canvas3D_, track, "mainview");

    MsgDistr.insert(painter_);

    camera_ = new tgt::Camera(tgt::vec3(0.0f, 0.0f, 3.75f));
    canvas3D_->setCamera(camera_);

    TrackballNavigation* trackNavi = new TrackballNavigation(track, true);
    canvas3D_->getEventHandler()->addListenerToBack(trackNavi);
    MsgDistr.insert(trackNavi);

    initializeRenderer();
    initializeWidgets();

    // Load datasets specified as program arguments
	// TODO?
    //if (!cmdLineParser_->getFileNames().isEmpty())
        //fileOpen(cmdLineParser_->getFileNames(), false);

    canvas3D_->startTimer(10);
    canvas3D_->setUpdatesEnabled(true); // everything is initialized, now allow repaints

    postMessage(new ColorMsg("set.backgroundColor", tgt::vec4(1.f, 1.f, 1.f, 1.f)));

	//TODO?
    //if (cmdLineParser_->getMaximized())
        //setWindowState(windowState() | Qt::WindowMaximized);

    MsgDistr.setAllowRepaints(true); // all initialized, allow repaints via messages

    canvas3D_->setPainter(painter_);
    canvas3D_->repaint();
}

void VoreenMainframe::createActions() {
    openAct_ = new QAction(QIcon("icons/open.png"), tr("&Open Dataset..."), this);
    openAct_->setShortcut(tr("Ctrl+O"));
    openAct_->setStatusTip(tr("Open an existing file"));
    openAct_->setToolTip(tr("Open an existing file"));
    connect(openAct_, SIGNAL(triggered()), this, SLOT(fileOpen()));

    exportAct_ = new QAction(QIcon("icons/save.png"), tr("&Export Dataset (as .dat + .raw)"), this);
    exportAct_->setShortcut(tr("Ctrl+E"));
    exportAct_->setStatusTip(tr("Export the current volume to a dat/raw file"));
    exportAct_->setToolTip(tr("Export the current volume"));
//     connect(exportAct_, SIGNAL(triggered()), this, SLOT(fileExport()));

    exitAct_ = new QAction(QIcon("icons/exit.png"), tr("E&xit"), this);
    exitAct_->setShortcut(tr("Ctrl+Q"));
    exitAct_->setStatusTip(tr("Exit the application"));
    exitAct_->setToolTip(tr("Exit the application"));
    connect(exitAct_, SIGNAL(triggered()), this, SLOT(fileExit()));

    aboutAct_ = new QAction(QIcon("icons/about.png"), tr("&About"), this);
    aboutAct_->setStatusTip(tr("Show the application's About box"));
    aboutAct_->setToolTip(tr("Show the application's About box"));
    connect(aboutAct_, SIGNAL(triggered()), this, SLOT(helpAbout()));

//     infoAction_ = new QAction(QIcon("icons/info.png"), tr("&Dataset Info"), this);
    infoAction_->setIcon(QIcon("icons/info.png"));
    infoAction_->setStatusTip(tr("Show dataset information"));
    infoAction_->setToolTip(tr("Dataset Information"));
}

void VoreenMainframe::createMenus() {
    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(openAct_);
    fileMenu_->addAction(exportAct_);

    fileMenu_->addSeparator();

    fileMenu_->addSeparator();
    fileMenu_->addAction(exitAct_);

    toolsMenu_ = menuBar()->addMenu(tr("&Utilities"));
    toolsMenu_->addAction(infoAction_);

    helpMenu_ = menuBar()->addMenu(tr("&Help"));
    helpMenu_->addAction(aboutAct_);
}

// show file open dialog
bool VoreenMainframe::getFileDialog(QStringList& filenames, QDir& dir) {
    QFileDialog *fd = new QFileDialog(this, tr("Choose a volume dataset to open"),
                                        dir.absolutePath());

    fd->setFilters(filters_);

    if (fd->exec()) {
        if (fd->selectedFiles().size() > 1) {
            QMessageBox::information(this, "Voreen",
                                        tr("Multiple selection is not allowed for these filetypes."));
            return false;
        }
        else {
            filenames = fd->selectedFiles();
            QDir dir = fd->directory();
            fileDialogDir_.setPath(dir.absolutePath());
            return true;
        }
    }
    else
        return false;
}

void VoreenMainframe::fileOpen() {
    QStringList filenames;
    if (!getFileDialog(filenames, fileDialogDir_))
        return;
    fileOpen(filenames);
}

void VoreenMainframe::fileOpen(const QStringList& fileNames, bool /*add*/) {
    if (fileNames.empty())
        return;

    // don't repaint while loading
    setUpdatesEnabled(false);

    // fixes reopen problem if currentDataset has value out of range
    //if (volumeContainer_->size())
    //    postMessage(new IntMsg(Processor::setCurrentDataset_, 0));

    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // put into a std::vector of strings
    std::vector<std::string> vFileNames(fileNames.size());
    for (int i = 0; i < fileNames.size(); ++i)
          vFileNames[i] = fileNames.at(i).toStdString();

    postMessage( new Message(ProxyGeometry::resetClipPlanes_) );

    // reset camera
    postMessage( new Message(TrackballNavigation::resetTrackball_) );

    // allow repaints
    setUpdatesEnabled(true);

    // repaint
    postMessage( new Message(VoreenPainter::repaint_) , VoreenPainter::visibleViews_);

    QApplication::restoreOverrideCursor();
}

void VoreenMainframe::fileExport() {
    // FIXME: VolumeContainer is obsolete. New design required for this (Dirk)
    /*
    if (!volumeContainer_->getVolume(0)) {
        QMessageBox::information(this, "Voreen", tr("No Datasets available.\n"));
        return;
    }

    QString fn = QFileDialog::getSaveFileName(this, tr("Export Dataset"), ".", tr("Dat/Raw Dataset (*.DAT)"));
    std::string fileName = fn.toStdString();

    if (fileName.length() == 0)
        return;

    DatVolumeWriter datwriter;
    datwriter.write(fileName, volumeContainer_->getVolume(0));
    */
}

void VoreenMainframe::showDatasetInfo(Volume* dataset, QString fileName) {
    if (!infoPlugin_)
        return;

    std::ostringstream info;
    info << tr("Filename: ").toStdString() << fileName.toStdString() << "\n";

    info << tr("Dimensions: ").toStdString() << dataset->getDimensions() << std::endl;
    info << tr("Spacing: ").toStdString() << dataset->getSpacing() << std::endl;

    info << tr("Bits stored/allocated: ").toStdString()  << dataset->getBitsStored() << "/"
         << dataset->getBitsAllocated() << "\n";

    infoPlugin_->setPlainText(QString(info.str().c_str()));
}

void VoreenMainframe::fileExit() {
    close();
}

void VoreenMainframe::closeEvent(QCloseEvent* event) {
    event->accept();
}

void VoreenMainframe::helpAbout() {
    QDialog* window = new QDialog(this);
//     Ui::VoreenAboutBox ui;
//     ui.setupUi(window);
#ifndef WIN32
    // On Unix the windows manager should take care of this
    int posX = pos().x() + (width() - window->width()) / 2;
    int posY = pos().y() + (height() - window->height()) / 2;
    window->move(posX, posY);
#endif
    window->setWindowIcon(QIcon("icons/icon-64.png"));
    setDisabled(true);
    window->setDisabled(false);
    window->exec();
    setDisabled(false);
}

void VoreenMainframe::initializeWidgets() {
    // initialize information plugin widget
    infoPlugin_ = new InformationPlugin(this);
    infoPlugin_->setMaximumHeight(130);
    PluginDialog* infoDialog = new PluginDialog(infoPlugin_, this, true);
    infoAction_ = infoDialog->createAction();
    addDockWidget(Qt::RightDockWidgetArea, infoDialog);

    createActions();
    createMenus();

    QToolBar* fileToolBar = new QToolBar(tr("File Toolbar"), this);
    QToolBar* toolsToolBar = new QToolBar(tr("Utilities Toolbar"), this);
    QToolBar* optionsToolBar = new QToolBar(tr("Options Toolbar"), this);

    fileToolBar->addAction(openAct_);
    fileToolBar->addAction(exportAct_);
    toolsToolBar->addAction(infoAction_);

    addToolBar(fileToolBar);
    addToolBar(toolsToolBar);
    addToolBar(optionsToolBar);
    statusBar();
}

void VoreenMainframe::initializeRenderer() {
    /*RPTMERGE
    RendererFactory factory(camera_);

    RendererFactory::RayCastingType rendererType;

    rendererType = cmdLineParser_->getRendererType();

    if (rendererType == RendererFactory::VRN_SIMPLERAYCASTING && !factory.tryToInitRenderer(RendererFactory::VRN_SIMPLERAYCASTING)){
        LWARNING("Simple-Raycaster not supported!");
        rendererType = RendererFactory::VRN_SLICERENDERER3D;
        LINFO("Trying Slicerenderer as renderer type.")
    }
    if (rendererType == RendererFactory::VRN_SLICERENDERER3D && !factory.tryToInitRenderer(RendererFactory::VRN_SLICERENDERER3D)){
        LWARNING("Slicerenderer not supported!");
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        LINFO("Graphics Board Vendor: " << vendor);
        LINFO("Used Renderer:         " << renderer);
        LINFO("OpenGL-Version:        " << version);
        LERROR("No renderer supported! Exiting ...");
        QMessageBox::critical(this, "Failure", "No renderer supported!\n\nExiting ...");
        exit(0);
    }

    Renderer* renderer;
    renderer = factory.generate(rendererType);

    tc_ = renderer->getTextureContainer();
    if ( !tc_ ) {
        // print information that would normally be
        // printed by texture container's initGL()
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        LINFO("Graphics Board Vendor: " << vendor);
        LINFO("Used Renderer:         " << renderer);
        LINFO("OpenGL-Version:        " << version);
    }

    if (!renderer) {
        LERROR("Renderer could not be instantiated! Exiting ...");
        QMessageBox::critical(this, "Failure", "Renderer could not be instantiated!\n\nExiting ...");
        exit(0);
    }

    painter_->setRenderer(renderer);

    //in case the slicerenderer is used, we have to manually specify a standard transferfunction
    if (rendererType == RendererFactory::VRN_SLICERENDERER3D)
        postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, new TransFuncIntensity()));
    */
}
