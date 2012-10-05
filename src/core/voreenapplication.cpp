/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "modules/core/coremodule.h"
#include "voreen/core/version.h"
#include "voreen/core/utils/commandlineparser.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/processorwidget.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertywidget.h"
#include "voreen/core/properties/propertywidgetfactory.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/processors/cache.h"

#include "tgt/init.h"
#include "tgt/filesystem.h"
#include "tgt/timer.h"
#include "tgt/gpucapabilities.h"

#include "gen_moduleregistration.h"

#include <string>
#include <iostream>

#ifdef WIN32
    #include <shlobj.h>
#else
    #include <stdlib.h>
#endif

#ifdef __APPLE__
    #include "CoreFoundation/CFBundle.h"
#endif

using std::string;

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

    if (p.empty())
        p = findWithSubDir(path, "resource/workspaces", 7);

    return p;
}

string findDataPath(const string& basePath) {
    return basePath + "/data";
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

/// istream operator for converting string to LogLevel (used by CommandLineParser)
std::istream& operator>>(std::istream& in, tgt::LogLevel& level) {
    std::string token;
    in >> token;
    std::string tokenLower = voreen::toLower(token);
    if (tokenLower == "debug")
        level = tgt::Debug;
    else if (tokenLower == "info")
        level = tgt::Info;
    else if (tokenLower == "warning")
        level = tgt::Warning;
    else if (tokenLower == "error")
        level = tgt::Error;
    else if (tokenLower == "fatal")
        level = tgt::Fatal;
    else
#ifndef VRN_OLD_BOOST
        throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, token);
#else
    ;
#endif
    return in;
}

std::string loglevelToString(tgt::LogLevel level) {
    if (level == tgt::Debug)
        return "debug";
    else if (level == tgt::Info)
        return "info";
    else if (level == tgt::Warning)
        return "warning";
    else if (level == tgt::Error)
        return "error";
    else if (level == tgt::Fatal)
        return "fatal";
    else
        return "unknown";
}

} // namespace anonymous

namespace voreen {

VoreenApplication* VoreenApplication::app_ = 0;
const std::string VoreenApplication::loggerCat_ = "voreen.VoreenApplication";

VoreenApplication::VoreenApplication(const std::string& binaryName, const std::string& displayName, const std::string& description,
                                     int argc, char** argv, ApplicationFeatures appType)
    : appFeatures_(appType)
    , binaryName_(binaryName)
    , displayName_(displayName)
    , enableLogging_(true)
    , logLevel_(tgt::Info)
    , enableFileLogging_(true)
    , logFile_(binaryName_ + "-log.html")
    , overrideGLSLVersion_("")
    , loadModules_(true)
#ifdef VRN_DEPLOYMENT
    , deploymentMode_(true)
#else
    , deploymentMode_(false)
#endif
    , cmdParser_(new CommandLineParser(binaryName, description, po::command_line_style::default_style))
    , networkEvaluator_(0)
    , schedulingTimer_(0)
    , eventHandler_()
    , useCaching_(new BoolProperty("useCaching", "Use Caching", true))
    , cacheLimit_(new IntProperty("cacheLimit", "Max Cache Size (MB)", 1024, 1, 999999))
    , deleteCache_(new ButtonProperty("deleteCache", "Delete Cache"))
    , availableGraphicsMemory_(new FloatProperty("availableGraphicsMemory", "Available Graphics Memory (MB)", -1.f, -1.f, 10000.f))
    , refreshAvailableGraphicsMemory_(new ButtonProperty("refreshAvailableGraphicsMemory", "Refresh"))
    , testDataPath_(new FileDialogProperty("testDataPath", "Test Data Directory", "Select Test Data Directory...",
        "", "", FileDialogProperty::DIRECTORY))
    , showSplashScreen_(new BoolProperty("showSplashScreen", "Show Splash Screen", true))
    , initialized_(false)
    , initializedGL_(false)
{
    app_ = this;
    cmdParser_->setCommandLine(argc, argv);

    // command line options
    cmdParser_->addFlagOption("help,h", CommandLineParser::AdditionalOption, "Print help message");

    cmdParser_->addOption("logging", enableLogging_, CommandLineParser::AdditionalOption,
        "If set to false, logging is disabled entirely (not recommended)",
        enableLogging_, enableLogging_ ? "true" : "false");

    cmdParser_->addOption("logLevel", logLevel_, CommandLineParser::AdditionalOption,
        "Sets the verbosity of the logger \n(debug|info|warning|error|fatal)",
        logLevel_, loglevelToString(logLevel_));

    cmdParser_->addOption("fileLogging", enableFileLogging_, CommandLineParser::AdditionalOption,
        "Enables HTML file logging \n(ignored, if logging is disabled)",
        enableFileLogging_, enableFileLogging_ ? "true" : "false");

    cmdParser_->addOption("logFile", logFile_, CommandLineParser::AdditionalOption,
        "Specifies the HTML log file",
        logFile_);

    cmdParser_->addOption<bool>("useCaching", CommandLineParser::AdditionalOption,
        "Enables or disables data caching. Overrides the setting stored in the application settings.");

    cmdParser_->addOption("glslVersion", overrideGLSLVersion_, CommandLineParser::AdditionalOption,
        "Overrides the detected GLSL version (1.10|1.20|1.30|1.40|1.50|3.30|4.00|..)",
        overrideGLSLVersion_);

    // caching properties
    deleteCache_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::deleteCache));
    addProperty(useCaching_);
    addProperty(cacheLimit_);
    addProperty(deleteCache_);
    useCaching_->setGroupID("caching");
    cacheLimit_->setGroupID("caching");
    deleteCache_->setGroupID("caching");
    setPropertyGroupGuiName("caching", "Data Caching");

    // gpu memory properties
    //availableGraphicsMemory_->setWidgetsEnabled(false);
    availableGraphicsMemory_->setViews(Property::SPINBOX);
    refreshAvailableGraphicsMemory_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::queryAvailableGraphicsMemory));
    addProperty(availableGraphicsMemory_);
    addProperty(refreshAvailableGraphicsMemory_);
    availableGraphicsMemory_->setGroupID("graphicsMemory");
    refreshAvailableGraphicsMemory_->setGroupID("graphicsMemory");
    setPropertyGroupGuiName("graphicsMemory", "Graphics Memory");

    // regression test properties
    addProperty(testDataPath_);
    testDataPath_->setGroupID("regressionTesting");
    setPropertyGroupGuiName("regressionTesting", "Regression Testing");

    // user interface properties
    addProperty(showSplashScreen_);
    showSplashScreen_->setGroupID("user-interface");
    setPropertyGroupGuiName("user-interface", "User Interface");
}

VoreenApplication::~VoreenApplication() {
    if (initializedGL_) {
        if (tgt::LogManager::isInited())
            LWARNING("~VoreenApplication(): OpenGL deinitialization has not been performed. Call deinitializeGL() before destruction.");
        return;
    }

    if (initialized_) {
        if (tgt::LogManager::isInited())
            LWARNING("~VoreenApplication(): application has not been deinitialized. Call deinitialize() before destruction.");
        return;
    }

    delete cmdParser_;
    cmdParser_ = 0;

    delete useCaching_;
    useCaching_ = 0;
    delete cacheLimit_;
    cacheLimit_ = 0;
    delete deleteCache_;
    deleteCache_ = 0;

    delete availableGraphicsMemory_;
    availableGraphicsMemory_ = 0;
    delete refreshAvailableGraphicsMemory_;
    refreshAvailableGraphicsMemory_ = 0;

    delete testDataPath_;
    testDataPath_ = 0;

    delete showSplashScreen_;
    showSplashScreen_ = 0;
}

VoreenApplication* VoreenApplication::app() {
    return app_;
}

const std::string& VoreenApplication::getBinaryName() const {
    return binaryName_;
}

const std::string& VoreenApplication::getDisplayName() const {
    return displayName_;
}

std::string VoreenApplication::getName() const {
    return getDisplayName();
}

VoreenApplication::ApplicationFeatures VoreenApplication::getApplicationType() const {
    return appFeatures_;
}

void VoreenApplication::setLoggingEnabled(bool enabled) {
    if (isInitialized()) {
        LERROR("Trying to change logging after application initialization");
    }
    else
        enableLogging_ = enabled;
}

bool VoreenApplication::isLoggingEnabled() const {
    return enableLogging_;
}

void VoreenApplication::setLogLevel(tgt::LogLevel logLevel) {
    if (isInitialized()) {
        LERROR("Trying to change log level after application initialization");
    }
    else
        logLevel_ = logLevel;
}

tgt::LogLevel VoreenApplication::getLogLevel() const {
    return logLevel_;
}

void VoreenApplication::setFileLoggingEnabled(bool enabled) {
    if (isInitialized()) {
        LERROR("Trying to change file logging after application initialization");
    }
    else
        enableFileLogging_ = enabled;
}

bool VoreenApplication::isFileLoggingEnabled() const {
    return enableFileLogging_;
}

void VoreenApplication::setLogFile(const std::string& logFile) {
    if (isInitialized()) {
        LERROR("Trying to change log file after application initialization");
    }
    else
        logFile_ = logFile;
}

const std::string& VoreenApplication::getLogFile() const {
    return logFile_;
}

void VoreenApplication::setOverrideGLSLVersion(const std::string& version) {
    if (isInitialized()) {
        LERROR("Trying to override GLSL version after application initialization");
    }
    else
        overrideGLSLVersion_ = version;
}

const std::string& VoreenApplication::getOverrideGLSLVersion() const {
    return overrideGLSLVersion_;
}

void VoreenApplication::setModuleLoadingEnabled(bool enabled) {
    if (isInitialized()) {
        LERROR("Trying to change module loading after application initialization");
    }
    else
        loadModules_ = enabled;
}

bool VoreenApplication::isModuleLoadingEnabled() const {
    return loadModules_;
}

void VoreenApplication::setDeploymentMode(bool dm) {
    if (isInitialized()) {
        LERROR("Trying to change deployment mode after application initialization");
    }
    else
        deploymentMode_ = dm;
}

bool VoreenApplication::getDeploymentMode() const {
    return deploymentMode_;
}

CommandLineParser* VoreenApplication::getCommandLineParser() const {
    return cmdParser_;
}

void VoreenApplication::loadModules() throw (VoreenException) {
    if (isModuleLoadingEnabled()) {
        LDEBUG("Loading modules from module registration header");
        registerAllModules(this); //< included from gen_moduleregistration.h
    }
    else {
        LDEBUG("Module auto loading disabled");
        registerModule(new CoreModule(getBasePath("modules/core")));  //< core module is always included
    }
}

void VoreenApplication::initialize() throw (VoreenException) {

    if (initialized_)
        throw VoreenException("Application already initialized");

    //
    // Path detection
    //

    // detect documents path first, needed for log file
    std::string documentsPath;
#ifdef WIN32
    TCHAR szPath[MAX_PATH];
    // get "my documents" directory
    if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath) == S_OK)
        documentsPath = szPath;
#else
    if (getenv("HOME") != 0) {
        documentsPath = getenv("HOME");
    }
    else {
        std::cerr << "Failed to detect home.";
    }
#endif

    // detect base path based on program location
    // (program path is available before command line parser execution
    string prog = cmdParser_->getProgramPath();

    basePath_ = ".";
    // cut path from program location
    string::size_type p = prog.find_last_of("/\\");
    if (p != string::npos) {
        basePath_ = prog.substr(0, p);
        prog = prog.substr(p + 1);
    }

    // try to find base path starting at program path
    basePath_ = findBasePath(basePath_);
    if (basePath_ == "") {
        std::cout << "WARNING: Base path not found. Using current directory.\n";
        basePath_ = ".";
    }
    basePath_ = tgt::FileSystem::absolutePath(basePath_);

    if (getDeploymentMode())
        userDataPath_ = tgt::FileSystem::cleanupPath(documentsPath + "Voreen");
    else
        userDataPath_ = tgt::FileSystem::cleanupPath(getBasePath("data"));


    //
    // Execute command line parser
    //
    if (appFeatures_ & APP_COMMAND_LINE) {
        tgtAssert(cmdParser_, "no command line parser");

        if (appFeatures_ & APP_CONFIG_FILE)
            cmdParser_->setConfigFile(getUserDataPath(tgt::FileSystem::baseName(binaryName_) + ".cfg"));

        try {
            cmdParser_->execute();
        }
        catch (VoreenException& e) {
            // a missing required argument causes an exception even if the --help flag has been passed,
            // so check for it here
            bool helpFlag = false;
            cmdParser_->getOptionValue("help", helpFlag);
            if (helpFlag) {
                std::cout << cmdParser_->getUsageString() << std::endl;
                exit(EXIT_SUCCESS);
            }
            else {
                std::cerr << "\nError: " << e.what() << "\n ";
                //std::cerr << cmdParser_->getUsageString(CommandLineParser::AllTypes, false) << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        // if --help has been passed, print usage and exit program
        bool helpFlag = false;
        cmdParser_->getOptionValue("help", helpFlag);
        if (helpFlag) {
            std::cout << cmdParser_->getUsageString() << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
    else {
        std::cout << "Command line disabled." << std::endl;
    }

    //
    // tgt initialization
    //
    tgt::InitFeature::Features featureset;
    if (enableLogging_)
        featureset = tgt::InitFeature::ALL;
    else
        featureset = tgt::InitFeature::Features(tgt::InitFeature::ALL &~ tgt::InitFeature::LOG_TO_CONSOLE);
    tgt::init(featureset, logLevel_);

    // HTML file logging
    std::string absLogPath;
    if (enableLogging_ && enableFileLogging_) {
        if (logFile_.empty()) {
            // should be never be empty (neither default value nor cmd line param)
            LERROR("HTML log file path is empty.");
        }
        else {
            // add HTML file logger
            tgt::Log* log = 0;
            if (tgt::FileSystem::isAbsolutePath(logFile_)) {
                log = new tgt::HtmlLog(logFile_);
                absLogPath = logFile_;
            }
            else {
                LogMgr.reinit(getUserDataPath()); //< write log file to user data dir by default
                log = new tgt::HtmlLog(logFile_);
                absLogPath = tgt::FileSystem::absolutePath(LogMgr.getLogDir() + "/" + logFile_);
            }
            tgtAssert(log, "no log");

            log->addCat("", true, logLevel_);
            LogMgr.addLog(log);
         }
    }

    // log basic information
    LINFO("Program: " << getBinaryName());
    VoreenVersion::logAll("voreen.VoreenApplication");
    LINFO("Base path: " << basePath_);
    LINFO("User data path: " << getUserDataPath() << (getDeploymentMode() ? " (Deployment mode)" : " (Developer mode)"));
    if (!absLogPath.empty())
        LINFO("HTML log file:  " << absLogPath);

    // mac app resources path
#ifdef __APPLE__
    appBundleResourcesPath_ = findAppBundleResourcesPath();
    if (appBundleResourcesPath_.empty())
        LERROR("Application bundle's resources path could not be detected!");
    else
        LINFO("Application bundle's resources path: " << appBundleResourcesPath_);
#endif

    // load modules
    try {
        LDEBUG("Loading modules");
        loadModules();
    }
    catch (VoreenException& e) {
        LERROR("Failed to load modules: " << e.what());
        throw;
    }
    if (modules_.empty())
        LWARNING("No modules loaded");
    else {
        std::vector<std::string> moduleNames;
        for (size_t i=0; i<modules_.size(); i++)
            moduleNames.push_back(modules_[i]->getName());
        LINFO("Modules: " << strJoin(moduleNames, ", "));
    }

    // load settings
    initApplicationSettings();
    loadApplicationSettings();

    // override caching setting, if specified on command line
    if (cmdParser_->isOptionSet("useCaching")) {
        bool caching = true;
        cmdParser_->getOptionValue<bool>("useCaching", caching);
        LINFO((caching ? "Enabled" : "Disabled") << " caching via command line");
        setUseCaching(caching);
    }

    // Check for temporary directory
    std::string tempDir = getTemporaryPath();
    if (!FileSys.dirExists(tempDir)) {
        LWARNING("Temporary directory (" << tempDir << ") does not exist, trying to create it...");
        if (!FileSys.createDirectoryRecursive(tempDir)) {
            throw VoreenException("Failed to create temporary directory");
        }
        else
            LINFO("Created temporary directory: " << tempDir);
    }

    // try to write to temporary directory (we can assume it exists)
    if (!FileSys.createDirectory(getTemporaryPath("writetest"))) {
        throw VoreenException("Failed to write to temporary directory");
    }
    else {
        FileSys.deleteDirectory(getTemporaryPath("writetest"));
    }

    // initialize modules
    LINFO("Initializing modules");
    for (size_t i=0; i<modules_.size(); i++) {
        try {
            LDEBUG("Initializing module '" << modules_.at(i)->getName() << "'");
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

    // init timer
    schedulingTimer_ = createTimer(&eventHandler_);
    eventHandler_.addListenerToFront(this);

    initialized_ = true;
}

void VoreenApplication::deinitialize() throw (VoreenException) {
    cleanCache(cacheLimit_->get());

    if (!initialized_)
        throw VoreenException("Application not initialized");

    if (initializedGL_)
        throw VoreenException("OpenGL deinitialization not performed. Call deinitializeGL() before deinitialization!");

    delete schedulingTimer_;
    schedulingTimer_ = 0;

    saveApplicationSettings();

    // deinitialize modules
    LINFO("Deinitializing modules");
    for (int i=(int)modules_.size()-1; i>=0; i--) {
        try {
            LDEBUG("Deinitializing module '" << modules_.at(i)->getName() << "'");
            modules_.at(i)->deinitialize();
            modules_.at(i)->initialized_ = false;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during deinitialization of module '" << modules_.at(i)->getName() << "': " << e.what());
        }
        catch (const std::exception& e) {
            LERROR("std::exception during deinitialization of module '" << modules_.at(i)->getName() << "': " << e.what());
        }
        catch (...) {
            LERROR("Unknown exception during deinitialization of module '" << modules_.at(i)->getName() << "'");
        }
    }

    // clear modules
    LDEBUG("Deleting modules");
    for (int i=(int)modules_.size()-1; i>=0; i--) {
        LDEBUG("Deleting module '" << modules_.at(i)->getName() << "'");
        delete modules_.at(i);
    }
    modules_.clear();

    LDEBUG("tgt::deinit()");
    tgt::deinit();

    initialized_ = false;
}

void VoreenApplication::initializeGL() throw (VoreenException) {
    if (!initialized_)
        throw VoreenException("Application not initialized. Call initialize() before OpenGL initialization!");

    if (initializedGL_)
        throw VoreenException("OpenGL initialization already done");

    LINFO("Initializing OpenGL");

    LDEBUG("tgt::initGL");
    tgt::initGL();
    glewInit();

    if (!overrideGLSLVersion_.empty()) {
        LWARNING("Overriding detected GLSL version " << GpuCaps.getShaderVersion()
            << " with version: " << overrideGLSLVersion_);
        GpuCaps.overrideGLSLVersion(overrideGLSLVersion_);
    }

    GpuCaps.logCapabilities(false, true);

    // OpenGL initialize modules
    LINFO("OpenGL initializing modules");
    for (size_t i=0; i<modules_.size(); i++) {
        if (!modules_.at(i)->isInitialized()) {
            LERROR("Module '" << modules_.at(i)->getName() << "' has not been initialized before OpenGL initialization");
            modules_.at(i)->initializedGL_ = false;
            continue;
        }
        try {
            LDEBUG("OpenGL initialization of module '" << modules_.at(i)->getName() << "'");
            modules_.at(i)->initializeGL();
            modules_.at(i)->initializedGL_ = true;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during OpenGL initialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            modules_.at(i)->initializedGL_ = false;
        }
        catch (const std::exception& e) {
            LERROR("std::exception during OpenGL initialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            modules_.at(i)->initializedGL_ = false;
        }
        catch (...) {
            LERROR("Unknown exception during OpenGL initialization of module '" << modules_.at(i)->getName() << "'");
            modules_.at(i)->initializedGL_ = false;
        }
    }

    queryAvailableGraphicsMemory();

    initializedGL_ = true;
}

void VoreenApplication::deinitializeGL() throw (VoreenException) {

    if (!initializedGL_)
        throw VoreenException("OpenGL not initialized");

    LINFO("OpenGL deinitializing modules");
    for (int i=(int)modules_.size()-1; i>=0; i--) {

        if (modules_.at(i)->isInitializedGL()) {
            try {
                LDEBUG("OpenGL deinitialization of module '" << modules_.at(i)->getName() << "'");
                modules_.at(i)->deinitializeGL();
                modules_.at(i)->initializedGL_ = false;
            }
            catch (const VoreenException& e) {
                LERROR("VoreenException during OpenGL deinitialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            }
            catch (const std::exception& e) {
                LERROR("std::exception during OpenGL deinitialization of module '" << modules_.at(i)->getName() << "': " << e.what());
            }
            catch (...) {
                LERROR("unknown exception during OpenGL deinitialization of module '" << modules_.at(i)->getName() << "'");
            }
        }
        else {
            LWARNING("Skipping OpenGL deinitialization of module '" << modules_.at(i)->getName() << "': not OpenGL initialized");
        }
    }

    LDEBUG("tgt::deinitGL");
    tgt::deinitGL();

    initializedGL_ = false;
}

bool VoreenApplication::isInitialized() const {
    return initialized_;
}

bool VoreenApplication::isInitializedGL() const {
    return initializedGL_;
}

bool VoreenApplication::useCaching() const {
    return useCaching_->get();
}

void VoreenApplication::setUseCaching(bool useCaching) {
    useCaching_->set(useCaching);
}

void VoreenApplication::registerModule(VoreenModule* module) {
    tgtAssert(module, "null pointer passed");

    // check if module's name and dirName have been set
    if (module->getName().empty() || module->getName() == "undefined") {
        tgtAssert(false, "module has no name (set in constructor!)");
        LERROR("Module has no name (set in constructor!). Skipping.");
        return;
    }

    // check if module directory exists
    /*std::string moduleDir = module->getModulePath();
    if (!tgt::FileSystem::dirExists(moduleDir))
        LWARNING("Module '" << module->getName() << "': module directory '" << moduleDir << "' does not exist"); */

    // register module
    if (std::find(modules_.begin(), modules_.end(), module) == modules_.end())
        modules_.push_back(module);
    else
        LWARNING("Module '" << module->getName() << "' has already been registered. Skipping.");
}

const std::vector<VoreenModule*>& VoreenApplication::getModules() const {
    return modules_;
}

VoreenModule* VoreenApplication::getModule(const std::string& moduleName) const {
    // search by module name first
    for (size_t i = 0 ; i < modules_.size() ; ++i) {
        VoreenModule* module = modules_.at(i);
        if (module->getName() == moduleName)
            return module;
    }

    // then search by module directory name
    for (size_t i = 0 ; i < modules_.size() ; ++i) {
        VoreenModule* module = modules_.at(i);
        if (module->getDirName() == moduleName)
            return module;
    }

    return 0;
}

void VoreenApplication::registerSerializerFactory(SerializableFactory* factory) {
    tgtAssert(factory, "null pointer passed");
    if (std::find(serializerFactories_.begin(), serializerFactories_.end(), factory) == serializerFactories_.end())
        serializerFactories_.push_back(factory);
    else
        LWARNING("SerializerFactory already registered. Skipping.");
}

const std::vector<SerializableFactory*>& VoreenApplication::getSerializerFactories() const {
    return serializerFactories_;
}

ProcessorWidget* VoreenApplication::createProcessorWidget(Processor* processor) const {
    tgtAssert(processor, "null pointer passed");
    if ((appFeatures_ & APP_PROCESSOR_WIDGETS) == 0)
        return 0;

    const std::vector<VoreenModule*>& modules = getModules();
    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<ProcessorWidgetFactory*>& factories = modules_.at(m)->getProcessorWidgetFactories();
        for (size_t f=0; f<factories.size(); f++) {
            ProcessorWidget* processorWidget = factories.at(f)->createWidget(processor);
            if (processorWidget)
                return processorWidget;
        }
    }
    return 0;
}

PropertyWidget* VoreenApplication::createPropertyWidget(Property* property) const {
    tgtAssert(property, "null pointer passed");
    if ((appFeatures_ & APP_PROPERTY_WIDGETS) == 0)
        return 0;

    const std::vector<VoreenModule*>& modules = getModules();
    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<PropertyWidgetFactory*>& factories = modules_.at(m)->getPropertyWidgetFactories();
        for (size_t f=0; f<factories.size(); f++) {
            PropertyWidget* propertyWidget = factories.at(f)->createWidget(property);
            if (propertyWidget)
                return propertyWidget;
        }
    }
    return 0;
}

LinkEvaluatorBase* VoreenApplication::createLinkEvaluator(const std::string& typeString) const{
    for (size_t m=0; m<getModules().size(); m++) {
        const std::vector<LinkEvaluatorFactory*>& factories = getModules().at(m)->getLinkEvaluatorFactories();
        for (size_t i=0; i<factories.size(); i++) {
            LinkEvaluatorBase* evaluator = factories.at(i)->createEvaluator(typeString);
            if (evaluator)
                return evaluator;
        }
    }
    return 0;
}

bool VoreenApplication::lazyInstantiation(Property* property) const {
    tgtAssert(property, "null pointer passed");

    const std::vector<VoreenModule*>& modules = getModules();
    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<PropertyWidgetFactory*>& factories = modules_.at(m)->getPropertyWidgetFactories();
        for (size_t f=0; f<factories.size(); f++) {
            if(!factories.at(f)->lazyInstantiation(property))
                return false;
        }
    }
    return true;
}

tgt::Timer* VoreenApplication::createTimer(tgt::EventHandler* /*handler*/) const {
    return 0;
}

ProgressBar* VoreenApplication::createProgressDialog() const {
    return 0;
}

std::string VoreenApplication::getBasePath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(basePath_ + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getCachePath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(getUserDataPath("cache") + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getProgramPath() const {
    tgtAssert(cmdParser_, "no command line parser");
    return tgt::FileSystem::dirName(cmdParser_->getProgramPath());
}

void VoreenApplication::cleanCache(int maxSize) const {
    CacheCleaner cleaner;
    cleaner.initialize(getCachePath());
    cleaner.deleteUnused();
    cleaner.limitCache(maxSize);
}

void VoreenApplication::deleteCache() {
    std::string cacheDir = getCachePath();

    std::vector<std::string> files = FileSys.listFiles(cacheDir);
    for(size_t i=0; i<files.size(); i++)
        FileSys.deleteFile(cacheDir+"/"+files[i]);

    std::vector<std::string> dirs = FileSys.listSubDirectories(cacheDir);
    for(size_t i=0; i<dirs.size(); i++)
        FileSys.deleteDirectoryRecursive(cacheDir+"/"+dirs[i]);
}

std::string VoreenApplication::getResourcePath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(getBasePath() + "/resource" + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getUserDataPath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(userDataPath_ + (filename.empty() ? "" : "/" + filename));

}

std::string VoreenApplication::getFontPath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(getResourcePath("fonts") + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getModulePath(const std::string& moduleName) const {
    tgtAssert(moduleName != "", "empty module name passed");
    if (getModule(moduleName))
        return getModule(moduleName)->getModulePath();
    else
        return "";
}

std::string VoreenApplication::getTemporaryPath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(getUserDataPath("tmp") + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getTestDataPath() const {
    tgtAssert(testDataPath_, "testDataPath_ property is null");
    return testDataPath_->get();
}

void VoreenApplication::setTestDataPath(const std::string& path) {
    tgtAssert(testDataPath_, "testDataPath_ property is null");
    testDataPath_->set(path);
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

void VoreenApplication::queryAvailableGraphicsMemory() {
    tgtAssert(availableGraphicsMemory_, "availableGraphicsMemory property not instantiated");
    if (tgt::Singleton<tgt::GpuCapabilities>::isInited()) {
        float mem = (GpuCaps.retrieveAvailableTextureMemory() != -1 ? GpuCaps.retrieveAvailableTextureMemory() / 1024.f : -1.f);
        availableGraphicsMemory_->setMinValue(mem);
        availableGraphicsMemory_->setMaxValue(mem);
        availableGraphicsMemory_->setDefaultValue(mem);
        availableGraphicsMemory_->set(mem);
    }
    else {
        LWARNING("queryAvailableGraphicsMemory(): GpuCapabilities not instantiated");
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

void VoreenApplication::initApplicationSettings() {
}

void VoreenApplication::loadApplicationSettings() {
    std::string filename = getUserDataPath("voreensettings.xml");

    if (!deserializeSettings(this, filename))
        LWARNING("Failed to deserialize application settings");

    const std::vector<VoreenModule*>& modules = getModules();
    for(size_t i=0; i<modules.size(); i++) {
        if(!modules[i]->getProperties().empty()) {
            deserializeSettings(modules[i], getUserDataPath(toLower(modules[i]->getName()) + "_settings.xml"));
        }
    }
}

void VoreenApplication::saveApplicationSettings() {
    std::string filename = getUserDataPath("voreensettings.xml");

    if(!serializeSettings(this, filename))
        LWARNING("Failed to save application settings");

    const std::vector<VoreenModule*>& modules = getModules();
    for(size_t i=0; i<modules.size(); i++) {
        if(!modules[i]->getProperties().empty()) {
            serializeSettings(modules[i], getUserDataPath(toLower(modules[i]->getName()) + "_settings.xml"));
        }
    }
}

void VoreenApplication::serialize(XmlSerializer& s) const {
    PropertyOwner::serialize(s);
}

void VoreenApplication::deserialize(XmlDeserializer& s) {
    PropertyOwner::deserialize(s);
}

} // namespace
