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

#ifdef VRN_WITH_PYTHON
// this needs to come first
#include "tgt/scriptmanager.h"
#include "voreen/core/vis/pyvoreen.h"
#endif

#include "voreen/core/application.h"

#include "voreen/core/cmdparser/commandlineparser.h"
#include "voreen/core/vis/messagedistributor.h"
#include "tgt/init.h"
#include "tgt/filesystem.h"

#include "tgt/gpucapabilities.h"
#ifdef WIN32
    #include "tgt/gpucapabilitieswindows.h"
#endif

using std::string;

namespace voreen {

namespace {

string findWithSubDir(const string& path, const string& subdir, int iterations = 0) {
    string p = path;

    // try in start directory
    if (tgt::File::dirExists(p + "/" + subdir))
        return p;

    // now try parent dirs
    for (int i = 0; i < iterations; i++) {
        p += "/..";
        if (tgt::File::dirExists(p + "/" + subdir))
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
        p = findWithSubDir(path, "data/networks", 7);
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
    return basePath + "/data";
#endif
}

string findShaderPath(const string& basePath) {
#ifdef VRN_INSTALL_PREFIX
    return basePath + "/share/voreen/shaders";
#else
    return basePath + "/src/core/vis/glsl";
#endif
}

} // namespace

VoreenApplication* VoreenApplication::app_ = 0;
const std::string VoreenApplication::loggerCat_ = "voreen.application";

VoreenApplication::VoreenApplication(const std::string& name, const std::string& displayName,
                                     int argc, char** argv, ApplicationType appType, const std::string& logDir)
    : appType_(appType),
      name_(name),
      displayName_(displayName),
      cmdParser_(displayName),
      logLevel_(tgt::Info),
      logDir_(logDir)
{
    app_ = this;
    cmdParser_.setCommandLine(argc, argv);
}

void VoreenApplication::prepareCommandParser() {
    cmdParser_.addCommand(new Command_LogLevel(&logLevel_));
    cmdParser_.addCommand(new Command_LogFile(&logFile_));
}

void VoreenApplication::init() {

    //
    // tgt initialization
    //
    tgt::init();
    tgt::Singleton<voreen::MessageDistributor>::init(new MessageDistributor());

    //
    // Command line parser
    //
    prepareCommandParser();
    cmdParser_.execute();

    //
    // Logging
    //
    if (appType_ & APP_LOGGING) {
        LogMgr.reinit(logDir_);
        tgt::Log* clog = new tgt::ConsoleLog();
        clog->addCat("", true, logLevel_);
        LogMgr.addLog(clog);

#ifdef VRN_ADD_FILE_LOGGER
        if (logFile_.empty())
            logFile_ = name_ + "-log.html";
#endif

        if (!logFile_.empty()) {
            // add a file logger
            tgt::Log* log = new tgt::HtmlLog(logFile_);
            log->addCat("", true, logLevel_);
            LogMgr.addLog(log);
        }
    }

    //
    // Path finding
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
    basePath_ = tgt::File::absolutePath(findBasePath(basePath_));

    LINFO("Voreen base path: " << basePath_);

    // shader path
    if (appType_ & APP_SHADER) {
#ifdef VRN_DISTRIBUTION_MODE
        // using tgt's virtual file system in distribution mode => no base path for shaders
        shaderPath_ = findShaderPath("");
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
    }

    //
    // Python
    //
#ifdef VRN_WITH_PYTHON
    if (appType_ & APP_PYTHON) {
        ScriptMgr.addPath("");
        initVoreenPythonModule();
    }
#endif
}

void VoreenApplication::initGL() {
    tgt::initGL();

#ifdef WIN32
    GpuCapsWin.logCapabilities(false, true);
#else
    GpuCaps.logCapabilities(false, true);
#endif
}

VoreenApplication* VoreenApplication::app() {
    return app_;
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
    return dataPath_ + "/fonts" + (filename.empty() ? "" : "/" + filename);
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

std::string VoreenApplication::getTransFuncPath(const std::string& filename) const {
    return dataPath_ + "/transferfuncs" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getModulePath(const std::string& filename) const {
    return basePath_ + "/src/modules" + (filename.empty() ? "" : "/" + filename);
}

std::string VoreenApplication::getTemporaryPath(const std::string& filename) const {
    return temporaryPath_ + (filename.empty() ? "" : "/" + filename);
}

} // namespace
