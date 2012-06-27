/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/utils/cmdparser/commandlineparser.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/processors/processorwidgetfactory.h"

#include "tgt/init.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"
#include "tgt/timer.h"
#include "tgt/gpucapabilities.h"

#ifndef VRN_NO_REGISTRATION_HEADER_GENERATION
    #include "voreen/modules/gen_moduleregistration.h"
#else
    #include "voreen/modules/moduleregistration.h"
#endif

#ifdef WIN32
#ifdef _MSC_VER
    #include "tgt/gpucapabilitieswindows.h"
#endif
    #include <shlobj.h>
#else
    #include <stdlib.h>
#endif

#ifdef __APPLE__
    #include "CoreFoundation/CFBundle.h"
#endif

using std::string;

namespace voreen {

namespace {

string findWithSubDir(const string& path, const string& subdir, int iterations = 0) {
    string p = path;

    // try in start directory
    if (tgt::FileSystem::dirExists(p + "/" + subdir))
        return p;

    // now try parent dirs
    for (int i = 0; i < iterations; i++) {
        p += "/..";
        if (tgt::FileSystem::dirExists(p + "/" + subdir))
            return p;
    }

    return "";
}

// Convert define into a string, compare
// http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define VRN_XSTRINGIFY(s) VRN_STRINGIFY(s)
#define VRN_STRINGIFY(s) #s

string findBasePath(const string& path) {
    string p;

#ifdef VRN_INSTALL_PREFIX
    // first try relative from application file
    p = findWithSubDir(path, "share/voreen/networks", 1);
#else
    if (p.empty())
        p = findWithSubDir(path, "data/workspaces", 7);
#endif

#ifdef VRN_INSTALL_PREFIX
    // try the install prefix as last resort
    if (p.empty())
        p = VRN_XSTRINGIFY(VRN_INSTALL_PREFIX);
#endif

    return p;
}

string findDataPath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/voreen";
#else
    return basePath + "/data";
#endif
}

string findVolumePath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/voreen/volumes";
#else
    return basePath + "/data/volumes";
#endif
}

string findShaderPath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/voreen/shaders";
#elif VRN_DEPLOYMENT
    return basePath + "/glsl";
#else
    return basePath + "/src/core/glsl";
#endif
}

string findDocumentationPath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/doc/voreen";
#else
    return basePath + "/doc";
#endif
}

#ifdef __APPLE__
string findAppBundleResourcesPath() {

    CFBundleRef bundle;

    bundle = CFBundleGetMainBundle();
    if(!bundle)
        return "";

    CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(bundle);
    char* path = new char [200];
    if(!CFURLGetFileSystemRepresentation(resourceURL, true, (UInt8*)path, 200))
        return "";

    string pathStr;
    if (path)
        pathStr = string(path);

    delete[] path;
     return pathStr;

}
#endif

} // namespace

VoreenApplication* VoreenApplication::app_ = 0;
const std::string VoreenApplication::loggerCat_ = "voreen.VoreenApplication";

VoreenApplication::VoreenApplication(const std::string& name, const std::string& displayName,
                                     int argc, char** argv, ApplicationType appType)
    : appType_(appType),
      name_(name),
      displayName_(displayName),
      cmdParser_(displayName),
      processorWidgetFactory_(0),
      logLevel_(tgt::Info),
      initialized_(false),
      initializedGL_(false),
      networkEvaluator_(0),
      schedulingTimer_(0),
      eventHandler_()
{
    app_ = this;
    cmdParser_.setCommandLine(argc, argv);
}

VoreenApplication::~VoreenApplication() {
    if (initializedGL_) {
        if (tgt::Singleton<tgt::LogManager>::isInited())
            LWARNING("~VoreenApplication(): OpenGL has not been deinitialized. Call deinitGL() before destruction.");
        return;
    }

    if (initialized_) {
        if (tgt::Singleton<tgt::LogManager>::isInited())
            LWARNING("~VoreenApplication(): application has not been deinitialized. Call deinit() before destruction.");
        return;
    }
}

void VoreenApplication::prepareCommandParser() {
    cmdParser_.addCommand(new Command_LogLevel(&logLevel_));
    cmdParser_.addCommand(new Command_LogFile(&logFile_));

    cmdParser_.addCommand(new SingleCommand<std::string>(&overrideGLSLVersion_,
        "--glslVersion", "",
        "Overrides the detected GLSL version", "<1.10|1.20|1.30|1.40|1.50|3.30|4.00>"));
}

void VoreenApplication::init() {

    if (initialized_) {
        if (tgt::Singleton<tgt::LogManager>::isInited())
            LWARNING("init() Application already initialized. Skip.");
        return;
    }

    //
    // tgt initialization
    //
    tgt::InitFeature::Features featureset
        = tgt::InitFeature::Features(tgt::InitFeature::ALL &~ tgt::InitFeature::LOG_TO_CONSOLE);
    tgt::init(featureset);

    if (appType_ & APP_CONSOLE_LOGGING) {
        tgt::Log* clog = new tgt::ConsoleLog();
        clog->addCat("", true, logLevel_);
        LogMgr.addLog(clog);
    }

    // init timer
    schedulingTimer_ = createTimer(&eventHandler_);
    eventHandler_.addListenerToFront(this);

    //
    // Command line parser
    //
    prepareCommandParser();
    cmdParser_.execute();

    // detect documents path first, needed for log file
#ifdef WIN32
    TCHAR szPath[MAX_PATH];
    // get "my documents" directory
    if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath) == S_OK)
        documentsPath_ = szPath;
#else
    if (getenv("HOME") != 0)
        documentsPath_ = getenv("HOME");
#endif

    // HTML logging
    if (appType_ & APP_HTML_LOGGING) {

#ifdef VRN_DEPLOYMENT
        LogMgr.reinit(documentsPath_);
#endif

        if (logFile_.empty())
            logFile_ = name_ + "-log.html";

        // add a file logger
        tgt::Log* log = new tgt::HtmlLog(logFile_);
        log->addCat("", true, logLevel_);
        LogMgr.addLog(log);
    }

#ifdef VRN_DEPLOYMENT
    LINFO("Deployment build.");
#endif

    //
    // Path detection
    //

    // detect base path based on program location
    string prog = cmdParser_.getProgramPath();

    basePath_ = ".";
    // cut path from program location
    string::size_type p = prog.find_last_of("/\\");
    if (p != string::npos) {
        basePath_ = prog.substr(0, p);
        prog = prog.substr(p + 1);
    }

    // try to find base path starting at program path
    basePath_ = tgt::FileSystem::absolutePath(findBasePath(basePath_));

    LINFO("Voreen base path: " << basePath_);

    // mac app resources path
#ifdef __APPLE__
    appBundleResourcesPath_ = findAppBundleResourcesPath();
    if (appBundleResourcesPath_.empty())
        LERROR("Application bundle's resources path could not be detected!");
    else
        LINFO("Application bundle's resources path: " << appBundleResourcesPath_);
#endif

    // shader path
    if (appType_ & APP_SHADER) {
#if defined(__APPLE__) && defined(VRN_DEPLOYMENT)
        shaderPath_ = appBundleResourcesPath_ + "/glsl";
#else
        shaderPath_ = findShaderPath(basePath_);
#endif
    }

    // data path
    if (appType_ & APP_DATA) {
        dataPath_ = findDataPath(basePath_);
        cachePath_ = dataPath_ + "/cache";
        temporaryPath_ = dataPath_ + "/tmp";
        volumePath_ = findVolumePath(basePath_);
#if defined(__APPLE__) && defined(VRN_DEPLOYMENT)
        fontPath_ = appBundleResourcesPath_ + "/fonts";
        texturePath_ = appBundleResourcesPath_ + "/textures";
        documentationPath_ = appBundleResourcesPath_ + "/doc";
#else
        fontPath_ = dataPath_ + "/fonts";
        texturePath_ = dataPath_ + "/textures";
        documentationPath_ = findDocumentationPath(basePath_);
#endif
    }

    //
    // Modules
    //
    if (appType_ & APP_AUTOLOAD_MODULES) {
        LDEBUG("Loading modules from module registration header");
        addAllModules(this);
        if (modules_.empty()) {
            LWARNING("No modules loaded");
        }
        else {
            std::string modString = modules_[0]->getName();
            for (size_t i=1; i<modules_.size(); i++)
                modString += ", " +  modules_[i]->getName();
            LINFO("Modules: " << modString);
        }
    }
    else {
        LDEBUG("Module auto loading disabled");
    }

    initialized_ = true;
}

void VoreenApplication::deinit() {

    if (!initialized_) {
        if (tgt::Singleton<tgt::LogManager>::isInited())
            LWARNING("deinit() Application not initialized. Skip.");
        return;
    }

    delete schedulingTimer_;
    schedulingTimer_ = 0;

    // clear modules
    for (size_t i=0; i<modules_.size(); i++) {
        LDEBUG("Deleting module '" << modules_[i]->getName() << "'");
        delete modules_[i];
    }
    modules_.clear();

    LDEBUG("tgt::deinit()");
    tgt::deinit();

    initialized_ = false;
}

void VoreenApplication::initGL() throw (VoreenException) {

    if (!initialized_)
        throw VoreenException("VoreenApplication::initGL(): Application not initialized");

    if (initializedGL_)
        throw VoreenException("VoreenApplication::initGL(): OpenGL already initialized");

    if ((appType_ & APP_HTML_LOGGING) && !logFile_.empty()) {
        std::string logPath = !LogMgr.getLogDir().empty() ? LogMgr.getLogDir() + "/" : "";
        LINFO("Log file: " << logPath << logFile_);
    }

    LDEBUG("tgt::initGL");
    tgt::initGL();

    if (!overrideGLSLVersion_.empty()) {
        LWARNING("Overriding detected GLSL version " << GpuCaps.getShaderVersion()
            << " with version: " << overrideGLSLVersion_);
#ifdef _MSC_VER
        bool success = GpuCaps.overrideGLSLVersion(overrideGLSLVersion_);
        if (success)
            GpuCapsWin.overrideGLSLVersion(overrideGLSLVersion_);
#else
        GpuCaps.overrideGLSLVersion(overrideGLSLVersion_);
#endif
    }

#ifdef _MSC_VER
    GpuCapsWin.logCapabilities(false, true);
#else
    GpuCaps.logCapabilities(false, true);
#endif
    ShdrMgr.addPath(getShaderPath());
    ShdrMgr.addPath(getShaderPath("utils"));

    // initialize modules
    for (size_t i=0; i<modules_.size(); i++) {
        try {
            LINFO("Initializing module '" << modules_.at(i)->getName() << "'");
            modules_.at(i)->initialize();
            modules_.at(i)->initialized_ = true;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during initialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            modules_.at(i)->initialized_ = false;
        }
        catch (const std::exception& e) {
            LERROR("std::exception during initialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            modules_.at(i)->initialized_ = false;
        }
        catch (...) {
            LERROR("Unknown exception during initialization of module '" << modules_.at(i)->getName() << "'");
            modules_.at(i)->initialized_ = false;
        }
    }

    initializedGL_ = true;
}

void VoreenApplication::deinitGL() throw (VoreenException) {

    if (!initializedGL_) {
        if (tgt::Singleton<tgt::LogManager>::isInited())
            LWARNING("deinitGL() OpenGL not initialized. Skip.");
        return;
    }

    // deinitialize modules
    for (size_t i=0; i<modules_.size(); i++) {

        if (modules_[i]->isInitialized()) {
            try {
                LDEBUG("Deinitializing module '" << modules_[i]->getName() << "'");
                modules_[i]->deinitialize();
                modules_[i]->initialized_ = false;
            }
            catch (const VoreenException& e) {
                LERROR("VoreenException during deinitialization of module '" << modules_[i]->getName() << "': " << e.what());
            }
            catch (const std::exception& e) {
                LERROR("std::exception during deinitialization of module '" << modules_[i]->getName() << "': " << e.what());
            }
            catch (...) {
                LERROR("unknown exception during deinitialization of module '" << modules_[i]->getName() << "'");
            }
        }
        else {
            LWARNING("Skipping deinitialization of module '" << modules_[i]->getName() << "': not initialized");
        }
    }

    LDEBUG("tgt::deinitGL");
    tgt::deinitGL();

    initializedGL_ = false;
}

VoreenApplication* VoreenApplication::app() {
    return app_;
}

void VoreenApplication::addModule(VoreenModule* module) {
    tgtAssert(module, "null pointer passed");
    if (std::find(modules_.begin(), modules_.end(), module) == modules_.end())
        modules_.push_back(module);
    else
        LWARNING("Module '" << module->getName() << "' has already been registered. Skipping.");
}

const std::vector<VoreenModule*> VoreenApplication::getModules() const {
    return modules_;
}

tgt::Timer* VoreenApplication::createTimer(tgt::EventHandler* /*handler*/) const {
    return 0;
}

ProgressBar* VoreenApplication::createProgressDialog() const {
    return 0;
}

void VoreenApplication::setProcessorWidgetFactory(ProcessorWidgetFactory* factory) {
    processorWidgetFactory_ = factory;
}

const ProcessorWidgetFactory* VoreenApplication::getProcessorWidgetFactory() const {
    return processorWidgetFactory_;
}

std::string VoreenApplication::getBasePath() const {
    return basePath_;
}

std::string VoreenApplication::getShaderPath(const std::string& filename) const {
    return shaderPath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getCachePath(const std::string& filename) const {
    return cachePath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getDataPath(const std::string& filename) const {
    return dataPath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getVolumePath(const std::string& filename) const {
    return volumePath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getFontPath(const std::string& filename) const {
    return fontPath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getNetworkPath(const std::string& filename) const {
    return dataPath_ + "/networks" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getWorkspacePath(const std::string& filename) const {
    return dataPath_ + "/workspaces" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getScriptPath(const std::string& filename) const {
    return dataPath_ + "/scripts" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getSnapshotPath(const std::string& filename) const {
#ifdef VRN_DEPLOYMENT
    return getDocumentsPath(filename);
#else
    return dataPath_ + "/snapshots" + (filename.empty() ? "" : "/" + filename);
#endif
}

std::string VoreenApplication::getTransFuncPath(const std::string& filename) const {
    return dataPath_ + "/transferfuncs" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getModulePath(const std::string& filename) const {
#ifdef VRN_DEPLOYMENT
    #if defined(__APPLE__)
        return appBundleResourcesPath_ + "/modules" + (filename.empty() ? "" : "/" + filename);
    #else
        return basePath_ + "/modules" + (filename.empty() ? "" : "/" + filename);
    #endif
#else
    return basePath_ + "/src/modules" + (filename.empty() ? "" : "/" + filename);
#endif
}

std::string VoreenApplication::getTexturePath(const std::string& filename) const {
    return texturePath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getTemporaryPath(const std::string& filename) const {
    return temporaryPath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getDocumentationPath(const std::string& filename) const {
    return documentationPath_ + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getDocumentsPath(const std::string& filename) const {
    return documentsPath_ + (filename.empty() ? "" : "/" + filename);
}

#ifdef __APPLE__
std::string VoreenApplication::getAppBundleResourcesPath(const std::string& filename) const {
   return appBundleResourcesPath_ + (filename.empty() ? "" : "/" + filename);
}
#endif

void VoreenApplication::scheduleNetworkProcessing() {
    if (schedulingTimer_ && networkEvaluator_ && schedulingTimer_->isStopped()) {
        schedulingTimer_->start(0, 1);
    }
}

void VoreenApplication::timerEvent(tgt::TimeEvent* /*e*/) {
    if (networkEvaluator_ && !networkEvaluator_->isLocked())
        networkEvaluator_->process();
}

void VoreenApplication::setNetworkEvaluator(NetworkEvaluator* evaluator) {
    networkEvaluator_ = evaluator;
}

NetworkEvaluator* VoreenApplication::getNetworkEvaluator() const {
    return networkEvaluator_;
}

} // namespace
