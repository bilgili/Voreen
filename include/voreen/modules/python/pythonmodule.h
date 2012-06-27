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

#ifndef VRN_PYTHONMODULE_H
#define VRN_PYTHONMODULE_H

// include this at very first
#include <Python.h>
#include "voreen/modules/python/pythonscript.h"
#include "voreen/modules/python/pyvoreen.h"

#include "voreen/core/voreenmodule.h"
#include "tgt/manager.h"

#include <string>

namespace voreen {

/**
 * Implement this interface in order to receive the output of
 * Python scripts.
 *
 * @see PythonModule::addOutputListener
 */
class PythonOutputListener {
public:
    virtual ~PythonOutputListener() {}

    /// Receives sys.stdout
    virtual void pyStdout(const std::string& out) = 0;

    /// Receives sys.stderr
    virtual void pyStderr(const std::string& err) = 0;
};


class PythonModule : public VoreenModule, public tgt::ResourceManager<PythonScript> {

public:
    PythonModule();

    ~PythonModule();

    virtual std::string getDescription() const {
        return "Provides a Python interface to Voreen resources, such as the processor network.";
    }

    /**
     * Loads a Python script from the specified file, considering the current search path.
     *
     * @see ResourceManager
     *
     * @return the created script object, or null if loading failed
     */
    PythonScript* load(const std::string& filename, bool compile = true);

    /**
     * Modifies sys.argv in the Python environment.
     *
     * @param argc number of command line arguments
     * @param argv array of arguments
     */
    void setArgv(int argc, char* argv[]);

    /**
     * Passes the specified program name to the Python environment.
     */
    void setProgramName(const std::string& prgName);

    /**
     * Appends the passed path to the Python interpreter's module search path.
     */
    void addModulePath(const std::string& path);

    /**
     * Registers a listener to receive the output of Python scripts.
     */
    void addOutputListener(PythonOutputListener* listener);

    /**
     * Removes an output listener.
     */
    void removeOutputListener(PythonOutputListener* listener);

    /**
     * Checks the state of the Python error indicator.
     * If an error has occurred, false is returned and the
     * type and value variables are written to the passed string reference.
     */
    bool checkForPythonError(std::string& errorMsg);

    /**
     * Returns the global instance of this class.
     *
     * @note Does not create the instance. If the module class has not been
     *       instantiated yet, the null pointer is returned.
     */
    static PythonModule* getInstance();

    /// To accessed from the vrn_print() function.
    static const std::vector<PythonOutputListener*>& getOutputListeners();

protected:
    /**
     * Initializes the Python interpreter and the Voreen bindings.
     */
    virtual void initialize() throw (VoreenException);

    /**
     * Disposes the Python interpreter and the Voreen bindings.
     */
    virtual void deinitialize() throw (VoreenException);

private:
    /// The actual Voreen Python bindings
    PyVoreen* pyVoreen_;

    /// Output of Python scripts is redirected to these.
    static std::vector<PythonOutputListener*> outputListeners_;

    static PythonModule* instance_;

    static const std::string loggerCat_;
};

} // namespace

#endif
