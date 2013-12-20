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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/version.h"
#include "voreen/core/utils/commandlineparser.h"
#include "voreen/core/utils/stringutils.h"
#include "voreen/core/utils/memoryinfo.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/network/processornetwork.h"
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
#include "voreen/core/properties/link/linkevaluatorhelper.h"
#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"
#include "voreen/core/processors/cache.h"

// core module is always available
#include "modules/core/coremodule.h"
#include "modules/core/processors/input/octreecreator.h"

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

} // namespace anonymous

namespace tgt {

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

} // namespace tgt

namespace voreen {

VoreenApplication* VoreenApplication::app_ = 0;
const std::string VoreenApplication::loggerCat_ = "voreen.VoreenApplication";

VoreenApplication::VoreenApplication(const std::string& binaryName, const std::string& guiName, const std::string& description,
                                     int argc, char** argv, ApplicationFeatures appType)
    : PropertyOwner(binaryName, guiName)
    , appFeatures_(appType)
    , binaryName_(binaryName)
    , enableLogging_(new BoolProperty("enableLogging", "Enable Logging", true))
    , logLevel_(0)
    , enableHTMLLogging_(new BoolProperty("htmlLogging", "Enable HTML File Logging", true))
    , htmlLogFile_(new FileDialogProperty("htmlLogFile", "HTML Log File", "Select HTML Log File", "", ".html", FileDialogProperty::SAVE_FILE))
    , overrideGLSLVersion_("")
    , loadModules_(true)
#ifdef VRN_DEPLOYMENT
    , deploymentMode_(true)
#else
    , deploymentMode_(false)
#endif
    , cmdParser_(new CommandLineParser(binaryName, description, po::command_line_style::default_style))
    , networkEvaluators_()
    , schedulingTimer_(0)
    , eventHandler_()
    , useCaching_(new BoolProperty("useCaching", "Use Caching", true))
    , volumeCacheLimit_(new IntProperty("cacheLimit", "Volume Cache Size (GB)", 10, 0, 1000 /*1 TB*/))
    , octreeCacheLimit_(new IntProperty("octreeCacheLimit", "Octree Cache Size (GB)", 100, 0, 1000 /*1 TB*/))
    , cachePath_(new FileDialogProperty("cachePath", "Cache Directory", "Select Cache Directory...", "", "", FileDialogProperty::DIRECTORY))
    , resetCachePath_(new ButtonProperty("resetCachePath", "Reset Cache Path"))
    , deleteCache_(new ButtonProperty("deleteCache", "Delete Cache"))
    , cpuRamLimit_(new IntProperty("cpuRamLimit", "CPU RAM Limit (MB)", 4000, 0, 64000))
    , availableGraphicsMemory_(new IntProperty("availableGraphicsMemory", "Available Graphics Memory (MB)", -1, -1, 10000))
    , refreshAvailableGraphicsMemory_(new ButtonProperty("refreshAvailableGraphicsMemory", "Refresh"))
    , testDataPath_(new FileDialogProperty("testDataPath", "Test Data Directory", "Select Test Data Directory...",
        "", "", FileDialogProperty::DIRECTORY))
    , showSplashScreen_(new BoolProperty("showSplashScreen", "Show Splash Screen", true))
    , initialized_(false)
    , initializedGL_(false)
    , networkEvaluationRequired_(false)
{
    id_ = guiName;
    guiName_ = guiName;
    app_ = this;
    cmdParser_->setCommandLine(argc, argv);

    // command line options
    cmdParser_->addFlagOption("help,h", CommandLineParser::AdditionalOption, "Print help message");

    cmdParser_->addOption<bool>("logging", CommandLineParser::AdditionalOption,
        "If set to false, logging is disabled entirely (not recommended)"
        /*, enableLogging_->get(), enableLogging_->get() ? "true" : "false" */);

    cmdParser_->addOption<tgt::LogLevel>("logLevel", CommandLineParser::AdditionalOption,
        "Sets the verbosity of the logger \n(debug|info|warning|error|fatal)"
        /*, tgt::Info, loglevelToString(tgt::Info)*/);

    cmdParser_->addOption<bool>("fileLogging", CommandLineParser::AdditionalOption,
        "Enables HTML file logging \n(ignored, if logging is disabled)"
        /*, enableHTMLLogging_->get(), enableHTMLLogging_->get() ? "true" : "false" */);

    cmdParser_->addOption<std::string>("logFile", CommandLineParser::AdditionalOption,
        "Specifies the HTML log file"
        /*, htmlLogFile_->get() */);

    cmdParser_->addOption<bool>("useCaching", CommandLineParser::AdditionalOption,
        "Enables or disables data caching. Overrides the setting stored in the application settings.");

    cmdParser_->addOption("glslVersion", overrideGLSLVersion_, CommandLineParser::AdditionalOption,
        "Overrides the detected GLSL version (1.10|1.20|1.30|1.40|1.50|3.30|4.00|..)",
        overrideGLSLVersion_);

    // caching properties
    resetCachePath_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::resetCachePath));
    deleteCache_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::deleteCache));
    addProperty(useCaching_);
    addProperty(volumeCacheLimit_);
    addProperty(octreeCacheLimit_);
    addProperty(cachePath_);
    addProperty(resetCachePath_);
    addProperty(deleteCache_);
    useCaching_->setGroupID("caching");
    volumeCacheLimit_->setGroupID("caching");
    octreeCacheLimit_->setGroupID("caching");
    cachePath_->setGroupID("caching");
    resetCachePath_->setGroupID("caching");
    deleteCache_->setGroupID("caching");
    setPropertyGroupGuiName("caching", "Data Caching");

    // CPU RAM properties
    addProperty(cpuRamLimit_);
    cpuRamLimit_->setGroupID("cpu-ram");
    setPropertyGroupGuiName("cpu-ram", "CPU Memory");

    // gpu memory properties
    //availableGraphicsMemory_->setWidgetsEnabled(false);
    availableGraphicsMemory_->setViews(Property::SPINBOX);
    refreshAvailableGraphicsMemory_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::queryAvailableGraphicsMemory));
    addProperty(availableGraphicsMemory_);
    addProperty(refreshAvailableGraphicsMemory_);
    availableGraphicsMemory_->setGroupID("graphicsMemory");
    refreshAvailableGraphicsMemory_->setGroupID("graphicsMemory");
    setPropertyGroupGuiName("graphicsMemory", "Graphics Memory");

    // logging properties
    addProperty(enableLogging_);
    enableLogging_->setVisible(false);
    logLevel_ = new OptionProperty<tgt::LogLevel>("logLevel", "Log Level");
    logLevel_->addOption("debug",   "Debug",    tgt::Debug);
    logLevel_->addOption("info",    "Info",     tgt::Info);
    logLevel_->addOption("warning", "Warning",  tgt::Warning);
    logLevel_->addOption("error",   "Error",    tgt::Error);
    logLevel_->addOption("fatal",   "Fatal",    tgt::Fatal);
    logLevel_->selectByKey("info");
    logLevel_->setDefaultValue("info");
    addProperty(logLevel_);
    addProperty(enableHTMLLogging_);
    htmlLogFile_->set(binaryName_ + "-log.html");
    htmlLogFile_->setDefaultValue(binaryName_ + "-log.html");
    addProperty(htmlLogFile_);

    logLevel_->onChange(CallMemberAction<VoreenApplication>(this, &VoreenApplication::logLevelChanged));

    enableLogging_->setGroupID("logging");
    logLevel_->setGroupID("logging");
    enableHTMLLogging_->setGroupID("logging");
    htmlLogFile_->setGroupID("logging");
    setPropertyGroupGuiName("logging", "Logging");

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
        std::cerr << "~VoreenApplication(): OpenGL deinitialization has not been performed. Call deinitializeGL() before destruction.\n";
        return;
    }

    if (initialized_) {
        std::cerr << "~VoreenApplication(): application has not been deinitialized. Call deinitialize() before destruction.\n";
        return;
    }

    delete cmdParser_;
    cmdParser_ = 0;

    delete useCaching_;
    useCaching_ = 0;
    delete volumeCacheLimit_;
    volumeCacheLimit_ = 0;
    delete octreeCacheLimit_;
    octreeCacheLimit_ = 0;
    delete cachePath_;
    cachePath_ = 0;
    delete resetCachePath_;
    resetCachePath_ = 0;
    delete deleteCache_;
    deleteCache_ = 0;

    delete cpuRamLimit_;
    cpuRamLimit_ = 0;

    delete availableGraphicsMemory_;
    availableGraphicsMemory_ = 0;
    delete refreshAvailableGraphicsMemory_;
    refreshAvailableGraphicsMemory_ = 0;

    delete enableLogging_;
    enableLogging_ = 0;
    delete logLevel_;
    logLevel_ = 0;
    delete enableHTMLLogging_;
    enableHTMLLogging_ = 0;
    delete htmlLogFile_;
    htmlLogFile_ = 0;

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

VoreenApplication::ApplicationFeatures VoreenApplication::getApplicationType() const {
    return appFeatures_;
}

void VoreenApplication::setLoggingEnabled(bool enabled) {
    tgtAssert(enableLogging_, "property not created");
    enableLogging_->set(enabled);
}

bool VoreenApplication::isLoggingEnabled() const {
    return enableLogging_->get();
}

void VoreenApplication::setLogLevel(tgt::LogLevel logLevel) {
    tgtAssert(logLevel_, "property not created");
    logLevel_->selectByValue(logLevel);
}

tgt::LogLevel VoreenApplication::getLogLevel() const {
    tgtAssert(logLevel_, "property not created");
    return logLevel_->getValue();
}

void VoreenApplication::setFileLoggingEnabled(bool enabled) {
    tgtAssert(enableHTMLLogging_, "property not created");
    enableHTMLLogging_->set(enabled);
}

bool VoreenApplication::isFileLoggingEnabled() const {
    tgtAssert(enableHTMLLogging_, "property not created");
    return enableHTMLLogging_->get();
}

void VoreenApplication::setLogFile(const std::string& logFile) {
    tgtAssert(htmlLogFile_, "property not created");
    htmlLogFile_->set(logFile);
}

const std::string& VoreenApplication::getLogFile() const {
    tgtAssert(htmlLogFile_, "property not created");
    return htmlLogFile_->get();
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
    string programPath = cmdParser_->getProgramPath();

#if defined(VRN_BASE_PATH) // use base path passed by CMAKE, if present
    basePath_ = VRN_BASE_PATH;
    if (!tgt::FileSystem::dirExists(basePath_)) {
        std::cerr << "WARNING: Passed base path does not exist: " << basePath_ << ". Using current directory instead.\n";
        basePath_ = ".";
    }
    else {
        basePath_ = tgt::FileSystem::cleanupPath(basePath_);
    }
#else // else try to find base path starting at program path
    basePath_ = ".";
    // cut path from program location
    string::size_type p = programPath.find_last_of("/\\");
    if (p != string::npos) {
        basePath_ = programPath.substr(0, p);
        //programPath = programPath.substr(p + 1);
    }

    // try to find base path starting at program path
    basePath_ = findBasePath(basePath_);
    if (basePath_ == "") {
        std::cerr << "WARNING: Base path not found. Using current directory instead.\n";
        basePath_ = ".";
    }
    basePath_ = tgt::FileSystem::absolutePath(basePath_);
#endif

    // use user directory for custom data, if in deployment mode
//#ifdef WIN32
    userDataPath_ = tgt::FileSystem::cleanupPath(getBasePath("data"));
//#else
    if (getDeploymentMode() && !documentsPath.empty()) {
        char lastChar = *documentsPath.rbegin();
        if (lastChar != '/' && lastChar != '\\')
            documentsPath += "/";
        userDataPath_ = tgt::FileSystem::cleanupPath(documentsPath + "Voreen");
    }
    else // use VRN_HOME/data as data directory
        userDataPath_ = tgt::FileSystem::cleanupPath(getBasePath("data"));
//#endif

    //
    // Execute command line parser
    //
    if (appFeatures_ & APP_COMMAND_LINE) {
        tgtAssert(cmdParser_, "no command line parser");

        // add remote control command line options (TODO: move to module)
#ifdef VRN_MODULE_REMOTECONTROL
        cmdParser_->addOption<bool>("enable-remote-control", CommandLineParser::AdditionalOption,
            "Enable/disable remote control TCP interface (overrides application setting)");
        cmdParser_->addOption<int>("remote-control-port", CommandLineParser::AdditionalOption,
            "TCP port of remote control interface (overrides application setting)");
#endif

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
    tgt::init(tgt::InitFeature::ALL, tgt::Info);

    std::string absLogPath;
    initLogging(absLogPath);

    // log basic information
    LINFO("Program: " << getBinaryName());
    VoreenVersion::logAll("voreen.VoreenApplication");
    LINFO("Base path:      " << basePath_);
    LINFO("Program path:   " << tgt::FileSystem::dirName(programPath));
    LINFO("User data path: " << getUserDataPath() << (getDeploymentMode() ? " (Deployment mode)" : " (Developer mode)"));

    LINFO(MemoryInfo::getTotalMemoryAsString());
    LINFO(MemoryInfo::getAvailableMemoryAsString());

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
            moduleNames.push_back(modules_[i]->getID());
        LINFO("Modules: " << strJoin(moduleNames, ", "));
    }

    // load settings
    initApplicationSettings();
    loadApplicationSettings();

    // apply command-line logging parameters, if specified
    if (cmdParser_->isOptionSet("logging")) {
        bool logging;
        cmdParser_->getOptionValue("logging", logging);
        setLoggingEnabled(logging);
    }
    else {
        setLoggingEnabled(true);
    }
    if (cmdParser_->isOptionSet("logLevel")) {
        tgt::LogLevel logLevel;
        cmdParser_->getOptionValue("logLevel", logLevel);
        setLogLevel(logLevel);
    }
    if (cmdParser_->isOptionSet("fileLogging")) {
        bool fileLogging;
        cmdParser_->getOptionValue("fileLogging", fileLogging);
        setFileLoggingEnabled(fileLogging);
    }
    if (cmdParser_->isOptionSet("logFile")) {
        std::string logFile;
        cmdParser_->getOptionValue("logFile", logFile);
        if (!logFile.empty())
            setLogFile(logFile);
    }

    // reinit logging, if default settings have been overwritten
    if (enableLogging_->get() != enableLogging_->getDefault()         || logLevel_->get() != logLevel_->getDefault()        ||
        enableHTMLLogging_->get() != enableHTMLLogging_->getDefault() || htmlLogFile_->get() != htmlLogFile_->getDefault()  )
    {
        initLogging(absLogPath);
    }
    if (!absLogPath.empty())
        LINFO("HTML log file:  " << absLogPath);

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
            LDEBUG("Initializing module '" << modules_.at(i)->getID() << "'");
            modules_.at(i)->initialize();
            modules_.at(i)->initialized_ = true;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during initialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            modules_.at(i)->initialized_ = false;
        }
        catch (const std::exception& e) {
            LERROR("std::exception during initialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            modules_.at(i)->initialized_ = false;
        }
        catch (...) {
            LERROR("Unknown exception during initialization of module '" << modules_.at(i)->getID() << "'");
            modules_.at(i)->initialized_ = false;
        }
    }

    // init timer
    schedulingTimer_ = createTimer(&eventHandler_);
    eventHandler_.addListenerToFront(this);

    initialized_ = true;
}

void VoreenApplication::deinitialize() throw (VoreenException) {
    cleanCache();

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
            LDEBUG("Deinitializing module '" << modules_.at(i)->getID() << "'");
            modules_.at(i)->deinitialize();
            modules_.at(i)->initialized_ = false;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during deinitialization of module '" << modules_.at(i)->getID() << "': " << e.what());
        }
        catch (const std::exception& e) {
            LERROR("std::exception during deinitialization of module '" << modules_.at(i)->getID() << "': " << e.what());
        }
        catch (...) {
            LERROR("Unknown exception during deinitialization of module '" << modules_.at(i)->getID() << "'");
        }
    }

    // clear modules
    LDEBUG("Deleting modules");
    for (int i=(int)modules_.size()-1; i>=0; i--) {
        LDEBUG("Deleting module '" << modules_.at(i)->getID() << "'");
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
            LERROR("Module '" << modules_.at(i)->getID() << "' has not been initialized before OpenGL initialization");
            modules_.at(i)->initializedGL_ = false;
            continue;
        }
        try {
            LDEBUG("OpenGL initialization of module '" << modules_.at(i)->getID() << "'");
            modules_.at(i)->initializeGL();
            modules_.at(i)->initializedGL_ = true;
        }
        catch (const VoreenException& e) {
            LERROR("VoreenException during OpenGL initialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            modules_.at(i)->initializedGL_ = false;
        }
        catch (const std::exception& e) {
            LERROR("std::exception during OpenGL initialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            modules_.at(i)->initializedGL_ = false;
        }
        catch (...) {
            LERROR("Unknown exception during OpenGL initialization of module '" << modules_.at(i)->getID() << "'");
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
                LDEBUG("OpenGL deinitialization of module '" << modules_.at(i)->getID() << "'");
                modules_.at(i)->deinitializeGL();
                modules_.at(i)->initializedGL_ = false;
            }
            catch (const VoreenException& e) {
                LERROR("VoreenException during OpenGL deinitialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            }
            catch (const std::exception& e) {
                LERROR("std::exception during OpenGL deinitialization of module '" << modules_.at(i)->getID() << "': " << e.what());
            }
            catch (...) {
                LERROR("unknown exception during OpenGL deinitialization of module '" << modules_.at(i)->getID() << "'");
            }
        }
        else {
            LWARNING("Skipping OpenGL deinitialization of module '" << modules_.at(i)->getID() << "': not OpenGL initialized");
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

size_t VoreenApplication::getCpuRamLimit() const {
    tgtAssert(cpuRamLimit_, "CPU RAM limit property not created");
    if (cpuRamLimit_->get() > 0)
        return static_cast<size_t>(static_cast<uint64_t>(cpuRamLimit_->get()) << 20); //< property specifies the limit in MB
    else
        return static_cast<size_t>(static_cast<uint64_t>(cpuRamLimit_->getMaxValue()) << 20); //< use max value
}

void VoreenApplication::setCpuRamLimit(size_t ramLimit) {
    tgtAssert(cpuRamLimit_, "CPU RAM limit property not created");
    cpuRamLimit_->set(static_cast<int>(ramLimit >> 20)); //< property specifies the limit in MB
}

void VoreenApplication::registerModule(VoreenModule* module) {
    tgtAssert(module, "null pointer passed");

    // check if module's name and dirName have been set
    if (module->getID().empty() || module->getID() == "undefined") {
        tgtAssert(false, "module has no name (set in constructor!)");
        LERROR("Module has no name (set in constructor!). Skipping.");
        return;
    }

    // check if module directory exists
    /*std::string moduleDir = module->getModulePath();
    if (!tgt::FileSystem::dirExists(moduleDir))
        LWARNING("Module '" << module->getID() << "': module directory '" << moduleDir << "' does not exist"); */

    // register module
    if (std::find(modules_.begin(), modules_.end(), module) == modules_.end())
        modules_.push_back(module);
    else
        LWARNING("Module '" << module->getID() << "' has already been registered. Skipping.");
}

const std::vector<VoreenModule*>& VoreenApplication::getModules() const {
    return modules_;
}

VoreenModule* VoreenApplication::getModule(const std::string& moduleName) const {
    // search by module name first
    for (size_t i = 0 ; i < modules_.size() ; ++i) {
        VoreenModule* module = modules_.at(i);
        if (module->getID() == moduleName)
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
        const std::vector<ProcessorWidgetFactory*>& factories = modules_.at(m)->getRegisteredProcessorWidgetFactories();
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
        const std::vector<PropertyWidgetFactory*>& factories = modules_.at(m)->getRegisteredPropertyWidgetFactories();
        for (size_t f=0; f<factories.size(); f++) {
            PropertyWidget* propertyWidget = factories.at(f)->createWidget(property);
            if (propertyWidget)
                return propertyWidget;
        }
    }
    return 0;
}

bool VoreenApplication::lazyInstantiation(Property* property) const {
    tgtAssert(property, "null pointer passed");

    const std::vector<VoreenModule*>& modules = getModules();
    for (size_t m=0; m<modules.size(); m++) {
        const std::vector<PropertyWidgetFactory*>& factories = modules_.at(m)->getRegisteredPropertyWidgetFactories();
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

void VoreenApplication::showMessageBox(const std::string& /*title*/, const std::string& message, bool /*error=false*/) const {
    LINFO("showMessageBox() not implemented: message=" << message);
}

std::string VoreenApplication::getBasePath(const std::string& filename) const {
    return tgt::FileSystem::cleanupPath(basePath_ + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getCachePath(const std::string& filename) const {
    tgtAssert(cachePath_, "cache path property not created");
    std::string cacheBasePath;

    // use property value, if set and directory does exist
    if (cachePath_->get() != "") {
        cacheBasePath = cachePath_->get();
        if (!tgt::FileSystem::dirExists(cacheBasePath)) {
            LWARNING("Cache path does not exist: " << cacheBasePath << ". Switching to user data path: " << getUserDataPath("cache"));
            cacheBasePath = "";
        }
    }
    // otherwise use user data path
    if (cacheBasePath == "")
        cacheBasePath = getUserDataPath("cache");

    return tgt::FileSystem::cleanupPath(cacheBasePath + (filename.empty() ? "" : "/" + filename));
}

std::string VoreenApplication::getProgramPath() const {
    tgtAssert(cmdParser_, "no command line parser");
    return tgt::FileSystem::dirName(cmdParser_->getProgramPath());
}

void VoreenApplication::cleanCache() const {
    // clean volume cache
    int volumeCacheLimitMB = volumeCacheLimit_->get() >> 10; //< property specifies GB
    CacheCleaner cleaner;
    cleaner.initialize(getCachePath());
    cleaner.deleteUnused();
    cleaner.limitCache(volumeCacheLimitMB);

    // clean octree cache
    uint64_t octreeCacheLimitBytes = (uint64_t)octreeCacheLimit_->get() << 30;//< property specifies GB
    OctreeCreator::limitCacheSize(getCachePath(), octreeCacheLimitBytes, false);
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

void VoreenApplication::resetCachePath() {
    cachePath_->set("");
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
    if (schedulingTimer_ && !networkEvaluators_.empty() /*&& schedulingTimer_->isStopped()*/) {
        // schedule network for immediate re-evaluation
        networkEvaluationRequired_ = true;
        if (!schedulingTimer_->isStopped())
            schedulingTimer_->stop();
        schedulingTimer_->start(0, 1);
    }
}

void VoreenApplication::timerEvent(tgt::TimeEvent* /*e*/) {
    if (!isInitialized())
        return;

    if (networkEvaluationRequired_) {
        networkEvaluationRequired_ = false;
        for (std::set<NetworkEvaluator*>::iterator it = networkEvaluators_.begin(); it != networkEvaluators_.end(); it++) {
            if(!(*it)->isLocked())
                (*it)->process();
        }
    }


    // check every 100 ms if the network has to be re-evaluated,
    // unless a re-evaluation has been explicitly scheduled via scheduleNetworkProcessing
    if (schedulingTimer_->isStopped() || schedulingTimer_->getTickTime() != 100) {
        schedulingTimer_->stop();
        schedulingTimer_->start(100, 0);
    }
}

void VoreenApplication::queryAvailableGraphicsMemory() {
    tgtAssert(availableGraphicsMemory_, "availableGraphicsMemory property not instantiated");
    if (tgt::Singleton<tgt::GpuCapabilities>::isInited()) {
        int mem = (GpuCaps.retrieveAvailableTextureMemory() != -1 ? (GpuCaps.retrieveAvailableTextureMemory() >> 10) : -1);
        availableGraphicsMemory_->setMinValue(mem);
        availableGraphicsMemory_->setMaxValue(mem);
        availableGraphicsMemory_->setDefaultValue(mem);
        availableGraphicsMemory_->set(mem);
    }
    else {
        LWARNING("queryAvailableGraphicsMemory(): GpuCapabilities not instantiated");
    }
}

void VoreenApplication::registerNetworkEvaluator(NetworkEvaluator* evaluator) {
    networkEvaluators_.insert(evaluator);
}

void VoreenApplication::deregisterNetworkEvaluator(NetworkEvaluator* evaluator) {
    networkEvaluators_.erase(evaluator);
}

NetworkEvaluator* VoreenApplication::getNetworkEvaluator() const {
    if(networkEvaluators_.empty())
        return 0;
    else
        return *(networkEvaluators_.begin());
}

NetworkEvaluator* VoreenApplication::getNetworkEvaluator(Processor* p) const {
    tgtAssert(p, "null pointer passed");
    for(std::set<NetworkEvaluator*>::iterator it = networkEvaluators_.begin(); it != networkEvaluators_.end(); it++) {
        std::vector<Processor*> pv = (*it)->getProcessorNetwork()->getProcessors();
        for(size_t i = 0; i < pv.size(); i++) {
            if(pv.at(i) == p)
                return *it;
        }
    }
    return 0;
}

NetworkEvaluator* VoreenApplication::getNetworkEvaluator(ProcessorNetwork* network) const {
    tgtAssert(network, "null pointer passed");
    for(std::set<NetworkEvaluator*>::iterator it = networkEvaluators_.begin(); it != networkEvaluators_.end(); it++) {
        if ((*it)->getProcessorNetwork() == network)
            return (*it);
    }
    return 0;
}

void VoreenApplication::initApplicationSettings() {
}

void VoreenApplication::loadApplicationSettings() {
    std::string filename = getUserDataPath(toLower(binaryName_) + "_settings.xml");
    if (!deserializeSettings(this, filename)) {
        // try old voreensettings.xml
        if (!deserializeSettings(this, getUserDataPath("voreensettings.xml")))
            LWARNING("Failed to deserialize application settings");
    }

    const std::vector<VoreenModule*>& modules = getModules();
    for(size_t i=0; i<modules.size(); i++) {
        if(!modules[i]->getProperties().empty()) {
            deserializeSettings(modules[i], getUserDataPath(toLower(modules[i]->getID()) + "_settings.xml"));
        }
    }
}

void VoreenApplication::saveApplicationSettings() {
    std::string filename = getUserDataPath(toLower(binaryName_) + "_settings.xml");

    if(!serializeSettings(this, filename))
        LWARNING("Failed to save application settings");

    const std::vector<VoreenModule*>& modules = getModules();
    for(size_t i=0; i<modules.size(); i++) {
        if(!modules[i]->getProperties().empty()) {
            serializeSettings(modules[i], getUserDataPath(toLower(modules[i]->getID()) + "_settings.xml"));
        }
    }
}

void VoreenApplication::serialize(XmlSerializer& s) const {
    PropertyOwner::serialize(s);
}

void VoreenApplication::deserialize(XmlDeserializer& s) {
    PropertyOwner::deserialize(s);
}

std::string VoreenApplication::getSerializableTypeString(const std::type_info& type) const {
    for (size_t i=0; i<modules_.size(); i++) {
        std::string typeString = modules_[i]->getSerializableTypeString(type);
        if (!typeString.empty())
            return typeString;
    }
    return "";
}

VoreenSerializableObject* VoreenApplication::createSerializableType(const std::string& typeString) const {
    const VoreenSerializableObject* type = getSerializableType(typeString);
    if (type)
        return type->create();
    else
        return 0;
}

const VoreenSerializableObject* VoreenApplication::getSerializableType(const std::string& typeString) const {
    for (size_t i=0; i<modules_.size(); i++) {
        const VoreenSerializableObject* instance = modules_[i]->getSerializableType(typeString);
        if (instance)
            return instance;
    }
    return 0;
}

void VoreenApplication::initLogging(std::string& htmlLogFile) {
    tgtAssert(enableLogging_, "property not created");
    tgtAssert(logLevel_, "property not created");
    tgtAssert(enableHTMLLogging_, "property not created");
    tgtAssert(htmlLogFile_, "property not created");

    if (!tgt::Singleton<tgt::LogManager>::isInited()) {
        std::cerr << "LogManager not initialized";
        return;
    }

    // extract current HTML file path
    std::string previousLogFile;
    std::vector<tgt::Log*> previousLogs = LogMgr.getLogs();
    for (size_t i=0; i<previousLogs.size(); i++)
        if (dynamic_cast<const tgt::HtmlLog*>(previousLogs.at(i)))
            previousLogFile = static_cast<const tgt::HtmlLog*>(previousLogs.at(i))->getAbsFilename();

    LogMgr.clear();

    if (!enableLogging_->get()) {
        std::cout << "Logging disabled!" << std::endl;
        return;
    }

    // Console log
    tgt::ConsoleLog* log = new tgt::ConsoleLog();
    log->addCat("", true, logLevel_->getValue());
    LogMgr.addLog(log);

    // HTML file logging
    if (!htmlLogFile_->get().empty()) {
        std::string absLogPath;

        // add HTML file logger
        tgt::Log* log = 0;
        htmlLogFile = htmlLogFile_->get();
        if (tgt::FileSystem::isAbsolutePath(htmlLogFile)) {
            absLogPath = htmlLogFile;
        }
        else {
            LogMgr.reinit(getUserDataPath()); //< write log file to user data dir by default
            absLogPath = tgt::FileSystem::absolutePath(LogMgr.getLogDir() + "/" + htmlLogFile);
        }
        log = new tgt::HtmlLog(htmlLogFile, false, true, true, true, tgt::FileSystem::cleanupPath(absLogPath) == tgt::FileSystem::cleanupPath(previousLogFile));
        tgtAssert(log, "no log");

        log->addCat("", true, logLevel_->getValue());
        LogMgr.addLog(log);

        htmlLogFile = absLogPath;

        //std::cout << "HTML Log File: " << htmlLogFile << "\n";
    }
    else {
        // should be never be empty (neither default value nor cmd line param)
        LERROR("HTML log file path is empty.");
    }

}

void VoreenApplication::logLevelChanged() {
    if (!initialized_)
        return;

    if (!tgt::Singleton<tgt::LogManager>::isInited()) {
        std::cerr << "LogManager not initialized";
        return;
    }

    LogMgr.setLogLevel(logLevel_->getValue());

}

} // namespace
