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

#ifndef VRN_APPLICATION_H
#define VRN_APPLICATION_H

#include "voreen/core/cmdparser/commandlineparser.h"

#include <string>
#include "tgt/logmanager.h"

namespace voreen {

/**
 * Represents basic properties of a Voreen application. There should only be one instance of
 * this class, which can be access via the static metho app().
 */
class VoreenApplication {
public:
    /// Features used in this application
    enum ApplicationType {
        APP_NONE    =  0,      ///< nothing
        APP_SHADER  =  1,      ///< detect shader path
        APP_DATA    =  2,      ///< detect data path
        APP_LOGGING =  4,      ///< activate tgt logging
        APP_PYTHON  =  8,      ///< activate python scripting
        APP_DEFAULT =  0xFFFF  ///< all features
    };

    /**
     * @param name Short name of the application in lowercase ("voreendev")
     * @param displayName Nice-looking name of the application ("VoreenDev")
     * @param argc Number of arguments as retrieved from main()
     * @param argv Argument vector as retrieved from main()
     * @param appType Features to activate
     */
    VoreenApplication(const std::string& name, const std::string& displayName,
                      int argc, char** argv, ApplicationType appType = APP_DEFAULT);

    virtual ~VoreenApplication() {}

    /**
     * Allows access to the global instance of this class.
     */
    static VoreenApplication* app();

    std::string getName() const { return name_; }
    std::string getDisplayName() const { return displayName_; }

    CommandlineParser* getCommandLineParser() { return &cmdParser_; }

    /**
     * Overwrite this method to add commands to the CommandlineParser.
     */
    virtual void prepareCommandParser();
    
    //
    // Initialization
    //
    
    /**
     * Do the actual initializations as controlled by appType_:
     * Initialize tgt, execute command parser, start logging, detect paths and initialize Python.
     */
    virtual void init();

    /**
     * Do OpenGL-specific initialization.
     */
    virtual void initGL();

    ///
    /// Paths
    ///
    
    /**
     * Returns the application's base path as detected by \sa init().
     */
    std::string getBasePath() const;
     
    /**
     * Constructs an absolute path consisting of the cache directory 
     * (typically voreen/data/cache) and the given filename.
     */
    std::string getCachePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the data directory (typically voreen/data) and
     * the given filename.
     */
    std::string getDataPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the volume data directory and the given
     * filename.
     */
    std::string getVolumePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the shader source directory and the given
     * filename.
     */
    std::string getShaderPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the font directory (typically
     * voreen/data/fonts) and the given filename.
     */
    std::string getFontPath(const std::string& filename = "") const;

	/**
	* Constructs an absolute path consisting of network file directory (typically
	* voreen/data/networks) and the given filename.
	*/
	std::string getNetworkPath(const std::string& filename = "") const;

	/**
	* Constructs an absolute path consisting of workspace file directory (typically
	* voreen/data/workspaces) and the given filename.
	*/
	std::string getWorkspacePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of script directory (typically
     * voreen/data/scripts) and the given filename.
     */
    std::string getScriptPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of transfer function directory (typically
     * voreen/data/transferfuncs) and the given filename.
     */
    std::string getTransFuncPath(const std::string& filename = "") const;

    /**
    * Constructs an absolute path consisting of the textures directory (typically
    * voreen/data/textures) and the given filename.
    */
    std::string getTexturePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the module directory (typically
     * voreen/src/modules) and the given filename.
     */
    std::string getModulePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the temporary directory (typically
     * voreen/data/tmp) and the given filename.
     */
    std::string getTemporaryPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the documentation directory (typically
     * voreen/doc) and the given filename.
     */
    std::string getDocumentationPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the documents directory (typically
     * "C:\Documents and Settings\user\Documents" on Windows and $HOME on unix) and the given
     * filename.
     */
    std::string getDocumentsPath(const std::string& filename = "") const;
    
#ifdef __APPLE__
    /**
     * Constructs an absolute path consisting of the Mac application bundle's resource
     * directory (path Contents/Resources within the bundle) and the given filename.
     */
    std::string getAppBundleResourcesPath(const std::string& filename = "") const;
#endif
    
protected:
    static VoreenApplication* app_;

    ApplicationType appType_;
    std::string name_;
    std::string displayName_;
    CommandlineParser cmdParser_;

    std::string basePath_;
    std::string shaderPath_;
    std::string cachePath_;
    std::string dataPath_;
    std::string texturePath_;
    std::string fontPath_;
    std::string volumePath_;
    std::string temporaryPath_;
    std::string documentationPath_;
    std::string documentsPath_;
#ifdef __APPLE__
	std::string appBundleResourcesPath_;
#endif

    tgt::LogLevel logLevel_;
    std::string logFile_;

    static const std::string loggerCat_;
};

} // namespace

#endif
