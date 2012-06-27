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

// do this at very first
#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

#include "tgt/init.h"
#include "tgt/logmanager.h"
#include "tgt/memorymanager.h"
#include "tgt/singleton.h"

// will remove this when unnecessary (jms)
#ifdef VRN_WITH_MATLAB
#include "voreen/core/io/matvolumereader.h"
#endif

#include "voreen/core/vis/messagedistributor.h"

#ifndef VRN_SNAPSHOT
#include "voreen/core/vis/pyvoreen.h"

#ifdef VRN_WITH_PYTHON
#include "voreen/core/vis/pyvoreen.h"
#include "voreen/qt/pyvoreenqt.h"
#endif
#endif

#if defined(VRN_WITH_BUGTRAP) && defined(WIN32)
#include "BugTrap.h"
#include <TCHAR.H>
#endif


#include <QSplashScreen>
#include <QTranslator>

#include "mainframe.h"

using namespace voreen;

void initGL() {
    tgt::initGL();

	// set shader source path
	ShdrMgr.addPath("../../src/core/vis/glsl");
	ShdrMgr.setPath("../../src/core/vis/glsl");

    // initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

int start(int argc, char** argv) {
    tgt::Singleton<voreen::MessageDistributor>::init(new MessageDistributor());

#ifdef VRN_WITH_PYTHON
    ScriptMgr.setPath("");
    initVoreenPythonModule();
    initVoreenqtPythonModule();
#endif // VRN_WITH_PYTHON

#define VRN_SHOWCONSOLE
#if defined(VRN_SHOWCONSOLE) && defined(WIN32)
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

#if defined(VRN_WITH_BUGTRAP) && defined(WIN32)
    // install bugtrap
    BT_InstallSehFilter();
    BT_SetAppName(_T("Voreen development application"));
    BT_SetSupportEMail(_T("jennis.meyer-spradow@math.uni-muenster.de"));
    BT_SetFlags(BTF_DETAILEDMODE | BTF_EDITMAIL | BTF_ATTACHREPORT| BTF_SCREENCAPTURE);
    //BT_SetSupportServer(_T("localhost"), 9999);
    BT_SetSupportURL(_T("http://www.voreen.org"));
#endif

    // initialize virtual file system for shaders
    // (only in release builds)
#ifdef NDEBUG
#pragma message("WARNING: Using 'shaders.tar'. Is it up to date???")
    FileSys.addPackage("shaders.tar", "../../");
#endif

    QApplication a(argc, argv);

#ifdef VRN_USE_TRANSLATIONS
    // load and install qt translations
    QTranslator qtTranslator;
#ifdef NDEBUG
    qtTranslator.load("lang/qt_" + QLocale::system().name());
#else
    qtTranslator.load("../translations/qt_" + QLocale::system().name());
#endif
    a.installTranslator(&qtTranslator);

    // load and install voreen translations
    QTranslator voreenTranslator;
#ifdef NDEBUG
    voreenTranslator.load("lang/voreen_" + QLocale::system().name());
#else
    voreenTranslator.load("../translations/voreen_" + QLocale::system().name());
#endif
    a.installTranslator(&voreenTranslator);
#endif

#ifdef VRN_SPLASHSCREEN
    QPixmap pixmap(":/vrn_app/image/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
#endif

    QStringList args(a.arguments());
    if (!QGLFormat::hasOpenGL())
        qFatal("This system has no OpenGL support");

    VoreenMainFrame mainFrame(&args);

    // init tgt logging system:
    CmdLineParser& clp = mainFrame.getCmdLineParser();

    // add a console logger
    tgt::Log* clog = new tgt::ConsoleLog();
    clog->addCat(clp.getCategory().toStdString(), true, clp.getDebugLevel());
    LogMgr.addLog(clog);

#ifdef VRN_ADD_FILE_LOGGER
    // add a file logger
    tgt::Log* log = new tgt::HtmlLog("voreen-log.html");
    log->addCat(clp.getCategory().toStdString(), true, clp.getDebugLevel());
    LogMgr.addLog(log);
#endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#if defined(VRN_SPLASHSCREEN) && defined(WIN32)
    mainFrame.showMinimized();
#else
    mainFrame.show();
#endif

    initGL();
    mainFrame.init();
    QApplication::restoreOverrideCursor();

#ifdef VRN_SPLASHSCREEN
    a.processEvents();
    splash.close();
	mainFrame.showMaximized();
#endif

    // HACK remove later
    if (argc >= 2 && strcmp(argv[1], "--overview") == 0) {
        //mainFrame.fileOpen("../../data/dat-cleaned-up/mensch-anonym-ct.dat", false, Modality::MODALITY_CT);
        //mainFrame.fileOpen("../../data/dat-cleaned-up/mensch-anonym-pet.dat", true, Modality::MODALITY_PET);
        mainFrame.fileOpen("../../data/dat-cleaned-up/mensch-anonym-ct.dat", Modality::MODALITY_CT);
        mainFrame.fileOpen("../../data/dat-cleaned-up/mensch-anonym-pet.dat", Modality::MODALITY_PET);
    }
	// used for final video
	//mainFrame.fileOpen("V:/voreen-dao/tested/hand_48/hand.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/hand_48/hand_spread_dao.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	// used for final video
	//mainFrame.fileOpen("V:/voreen-dao/tested/hand_24/hand.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/hand_24/hand_spread_dao.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	//mainFrame.fileOpen("V:/voreen-dao/tested/feet_16_r/feet_256x128x256_32bit.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/feet_16_r/feet_256x128x256_daof.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	//mainFrame.fileOpen("V:/voreen-dao/tested/feet_128_64_128_12/feet_32bit.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/feet_128_64_128_12/feet_dao.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	// used for final video
	//mainFrame.fileOpen("V:/voreen-dao/tested/vmhead_half_small_16/vmhead-half-small_32bit.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/vmhead_half_small_16/vmhead-half-small_dao.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	//mainFrame.fileOpen("V:/voreen-dao/vmhead-r20-nc2048-b256-p16/vmhead-8bit-small_32bit.dat");
	//mainFrame.fileOpen("V:/voreen-dao/vmhead-r20-nc2048-b256-p16/vmhead-8bit-small_dao16-0.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	//mainFrame.fileOpen("V:/voreen-dao/cornell256_32bitf.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/cornell128-r32-nc1024-b256-p16/cornell128_scaled256.dat");

	//mainFrame.fileOpen("D:/voreen/apps/voltool/debug/cornell128_32bit-gw13.dat");
	//mainFrame.fileOpen("V:/voreen-dao/tested/cornell128-r32-nc1024-b256-p16/cornell128_dao16-0.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

	// used for final video
	//mainFrame.fileOpen("D:/voreen/data/cornell128_sobel_f3.dat");
	//mainFrame.fileOpen("H:/private/voreen-dao/cornell128-r32-nc1024-b256-p16/cornell128_32bit.dat");
	//mainFrame.fileOpen("H:/private/voreen-dao/cornell128-r32-nc1024-b256-p16/cornell128_dao16-0.dat", true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);

//#define VOREEN_DAO
#ifdef VOREEN_DAO
	// automatically load volume and dao data set. following assumptions are made:
	// - dirName should be given without appended /
	// - dirName should fullfill naming conventions described in /share/voreen/voreen-dao/tested/namingconventions.txt

	std::string dirName;
	//dirName = "V:/voreen-dao/tested/cornell64-r4-nc512-b256-p16";
	//dirName = "V:/voreen-dao/tested/cornell64-r4-nc1024-b256-p16";
	//dirName = "V:/voreen-dao/tested/cornell64-r4-nc2048-b256-p16";
	//dirName = "V:/voreen-dao/tested/cornell128-r32-nc1024-b256-p16_nolightsource";
	//dirName = "V:/voreen-dao/tested/cornell128-r32-nc1024-b256-p16";
	dirName = "V:/voreen-dao/tested/cornell128-rx-nc512-b256-p16_bothhemispheres";
	//dirName = "V:/voreen-dao/tested/cornell256-r32-nc2048-b256-p16";
	//dirName = "V:/voreen-dao/cornell256-r48-nc1024-b256-p16";

	std::ostringstream volumeFileName;
	volumeFileName << dirName;
	volumeFileName << dirName.substr(dirName.find_last_of("/"), dirName.find("-", dirName.find_last_of("/"))-dirName.find_last_of("/"));
	volumeFileName << "_32bit-tf.dat";
	mainFrame.fileOpen(QString(volumeFileName.str().c_str()));

	std::ostringstream daoFileName;
	daoFileName << dirName;
	daoFileName << dirName.substr(dirName.find_last_of("/"), dirName.find("-", dirName.find_last_of("/"))-dirName.find_last_of("/"));
	daoFileName << "_dao16-0.dat";
	mainFrame.fileOpen(QString(daoFileName.str().c_str()), true, Modality::MODALITY_DYNAMICAMBIENTOCCLUSION);
#endif

    /*
	// I need this until the new volume classes and the new loading mechanism work. (jms)

    VolumeContainer* volCon = new VolumeContainer();

	MatVolumeReader reader;

    VolumeContainer volConX, volConY, volConZ, volConI;
    //reader.readIntoContainer("../../../data/dawood/Datensatz1_1/Vx.mat", &volConX);
    //reader.readIntoContainer("../../../data/dawood/Datensatz1_1/Vy.mat", &volConY);
    //reader.readIntoContainer("../../../data/dawood/Datensatz1_1/Vz.mat", &volConZ);
    reader.readIntoContainer("../../../data/dawood/Datensatz1_1/AltekampOriginalVol.mat", &volConI);
	reader.readIntoContainer("../../../data/dawood/Datensatz1_1/AltekampXTo2Vol.mat", &volConI);
    //VolumeDatasetDirections* vol = new VolumeDatasetDirections(volConX.get(0), volConY.get(0), volConZ.get(0), volConI.get(0));
    //volCon->add(vol, "Altekamp");
	VolumeDataset16Bit* dataset16Bit;
	for (int i=0; i<8; ++i) {
		VolumeDataset* orgVol = volConI.get(i);
		dataset16Bit = new VolumeDataset16Bit(orgVol->getDimensions(), orgVol->getSpacing(), reinterpret_cast<uint16_t*>(orgVol->convertScalars(17)));
		volCon->add(dataset16Bit, "volume");
		reader.create3DTexture(dataset16Bit, tgt::LINEAR, false);
	}
	for (int i=8; i<16; ++i) {
		VolumeDataset* orgVol = volConI.get(i);
		dataset16Bit = new VolumeDataset16Bit(orgVol->getDimensions(), orgVol->getSpacing(), reinterpret_cast<uint16_t*>(orgVol->convertScalars(16)));
		volCon->add(dataset16Bit, "volume");
		reader.create3DTexture(dataset16Bit, tgt::LINEAR, false);
	}

    mainFrame.volumeContainer_ = volCon;
    mainFrame.postMessage(new VolumeContainerPtrMsg(Identifier::setVolumeContainer, mainFrame.volumeContainer_));
    mainFrame.postMessage(new IntMsg(Identifier::setCurrentDataset, 0));
    */

    int result = a.exec();
    mainFrame.deinit();
    tgt::Singleton<voreen::MessageDistributor>::deinit();

    return result;
}

int main(int argc, char** argv) {
#ifdef TGT_USE_MEMORY_MANAGER
	// the memory manager can be used to fin memory leaks
    tgt::MemoryManager::init();
#endif

    int result;
    tgt::init();
    // init resources for voreen_qt
    Q_INIT_RESOURCE(vrn_qt);
    // init common application resources
    Q_INIT_RESOURCE(vrn_app);
    result = start(argc, argv);
    tgt::deinitGL();
    tgt::deinit();

#ifdef TGT_USE_MEMORY_MANAGER
    tgt::MemoryManager::deinit();
#endif

    return result;
}
