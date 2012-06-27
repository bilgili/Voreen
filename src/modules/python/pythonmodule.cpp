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

#include "voreen/modules/python/pythonmodule.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/utils/stringconversion.h"

namespace voreen {

const std::string PythonModule::loggerCat_("voreen.Python.Module");
PythonModule* PythonModule::instance_ = 0;
std::vector<PythonOutputListener*> PythonModule::outputListeners_;

// -----------------------------------------------------------------
// Python functions

static PyObject* voreen_print(PyObject* /*self*/, PyObject* args) {
    char* msg;
    int len;
    int isStderr;
    if (!PyArg_ParseTuple(args, "s#i", &msg, &len, &isStderr)) {
        LWARNINGC("voreen.Python.voreen_print", "failed to parse log message");
    }
    else {
        if (len > 1 || ((len == 1) && (msg[0] != '\0') && (msg[0] != '\r') && (msg[0] != '\n'))) {
            std::string message(msg);
            const std::vector<PythonOutputListener*> listeners = PythonModule::getOutputListeners();
            if (!listeners.empty()) {
                // pass output to listeners
                for (size_t i=0; i<listeners.size(); i++) {
                    if (isStderr)
                        listeners[i]->pyStderr(message);
                    else
                        listeners[i]->pyStdout(message);
                }
            }
            else {
                // log output if no listener registered
                if (isStderr)
                    LWARNINGC("voreen.Python.voreen_print", message);
                else
                    LINFOC("voreen.Python.voreen_print", message);
            }
        }
    }

    Py_RETURN_NONE;
}

// module 'voreen_internal'
static PyMethodDef internal_methods[] = {
    {
        "vrnPrint",
        voreen_print,
        METH_VARARGS,
        "Internal helper function used for accessing script output"
    },
    { NULL, NULL, 0, NULL} // sentinal
};

// -----------------------------------------------------------------
PythonModule::PythonModule()
    : VoreenModule()
    , tgt::ResourceManager<PythonScript>(false)
    , pyVoreen_(0)
{
    setName("Python");
    instance_ = this;
}

PythonModule::~PythonModule() {
    instance_ = 0;
}

PythonModule* PythonModule::getInstance() {
    return instance_;
}

void PythonModule::initialize() throw (VoreenException) {
    VoreenModule::initialize();

    //
    // Initialize Python interpreter
    //
    LINFO("Python version: " << Py_GetVersion());

    // Pass program name to the Python interpreter
    char str_pyvoreen[] = "PyVoreen";
    Py_SetProgramName(str_pyvoreen);

    // Initialize the Python interpreter. Required.
    Py_InitializeEx(false);
    if (!Py_IsInitialized())
        throw VoreenException("Failed to initialize Python interpreter");

    // required in order to use threads.
    PyEval_InitThreads();

    // init ResourceManager search path
    addPath("");
    addPath(VoreenApplication::app()->getScriptPath());
    addPath(VoreenApplication::app()->getModulePath("python/scripts"));

    // init Python's internal module search path
    addModulePath(VoreenApplication::app()->getScriptPath());
    addModulePath(VoreenApplication::app()->getModulePath("python/scripts"));

    //
    // Redirect script output from std::cout to voreen_print function (see above)
    //

    // import helper module
    if (!Py_InitModule("voreen_internal", internal_methods)) {
        LWARNING("Failed to init helper module 'voreen_internal'");
    }

    // load output redirector script and run it once
    std::string filename = "outputcatcher.py";
    LDEBUG("Loading Python init script '" << filename << "'");
    PythonScript* initScript = load(filename);
    if (initScript) {
        if (!initScript->run())
            LWARNING("Failed to run init script '" << filename << "': " << initScript->getLog());
        dispose(initScript);
    }
    else {
        LWARNING("Failed to load init script '" << filename << "'");
    }

    //
    // Create actual Voreen Python bindings
    //
    pyVoreen_ = new PyVoreen();
}

void PythonModule::deinitialize() throw (VoreenException) {

    if (!isInitialized())
        throw VoreenException("not initialized");

    delete pyVoreen_;
    pyVoreen_ = 0;

    // clean up python interpreter
    Py_Finalize();

    VoreenModule::deinitialize();
}

PythonScript* PythonModule::load(const std::string& filename, bool compileDirectly) {

    // do not check isInitialized(), since we call this function from initialize()
    if (!Py_IsInitialized()) {
        LWARNING("load(): not initialized");
        return 0;
    }

    if (isLoaded(filename)) {
        increaseUsage(filename);
        return get(filename);
    }

    PythonScript* script = new PythonScript();
    if (script->load(completePath(filename), compileDirectly)) {
        reg(script, filename);
        return script;
    }
    delete script;

    return 0;
}

void PythonModule::setArgv(int argc, char* argv[]) {
    PySys_SetArgv(argc, argv);
}

void PythonModule::setProgramName(const std::string& prgName) {
    char* str = strtochr(prgName);
    Py_SetProgramName(str);
    delete[] str;
}

void PythonModule::addModulePath(const std::string& path) {

    // do not check isInitialized(), since we call this function from initialize()
    if (!Py_IsInitialized()) {
        LWARNING("addModulePath(): not initialized");
        return;
    }

    // convert windows back slashes to slashes
    std::string pathConv = strReplaceAll(path, "\\", "/");

    LDEBUG("Adding '" << pathConv << "' to Python module search path");
    std::string runString = "import sys\n";
    runString.append(std::string("sys.path.append('") + pathConv + std::string("')"));
    int ret = PyRun_SimpleString(runString.c_str());
    if (ret != 0)
        LWARNING("Failed to add '" << pathConv << "' to Python module search path");
}

bool PythonModule::checkForPythonError(std::string& errorMsg) {

    if (!PyErr_Occurred())
        return true;

    // retrieve error indicator into variables
    PyObject *errtype, *errvalue, *traceback;
    PyErr_Fetch(&errtype, &errvalue, &traceback);
    PyErr_NormalizeException(&errtype, &errvalue, &traceback);

    // convert indicators to Python strings, if possible
    PyObject* errtypeStr = PyObject_Str(errtype);
    PyObject* errvalueStr = PyObject_Str(errvalue);
    if (errtypeStr && PyString_AsString(errtypeStr))
        errorMsg = std::string(PyString_AsString(errtypeStr));
    if (errvalueStr && PyString_AsString(errvalueStr))
        errorMsg += std::string(", ") + std::string(PyString_AsString(errvalueStr));

    // free allocated ressources
    Py_XDECREF(errtypeStr);
    Py_XDECREF(errvalueStr);

    Py_XDECREF(errtype);
    Py_XDECREF(errvalue);
    Py_XDECREF(traceback);

    return false;
}

void PythonModule::addOutputListener(PythonOutputListener* listener) {
    tgtAssert(listener, "null pointer passed");
    if (std::find(outputListeners_.begin(), outputListeners_.end(), listener) == outputListeners_.end())
        outputListeners_.push_back(listener);
    else
        LWARNING("Output listener already registered");
}

void PythonModule::removeOutputListener(PythonOutputListener* listener) {
    tgtAssert(listener, "null pointer passed");
    std::vector<PythonOutputListener*>::iterator it = std::find(outputListeners_.begin(), outputListeners_.end(), listener);
    if (it != outputListeners_.end())
        outputListeners_.erase(it);
    else
        LWARNING("Output listener not registered");
}

const std::vector<PythonOutputListener*>& PythonModule::getOutputListeners() {
    return outputListeners_;
}

} // namespace
