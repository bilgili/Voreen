/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/scriptmanager.h"
#include "tgt/filesystem.h"

#ifdef TGT_USE_PYTHON

namespace tgt {

const std::string Script::loggerCat_ = "tgt.Script";

bool Script::load(const std::string& filename, bool compileDirectly) {
    File* file = FileSys.open(filename);

    // Check if file is open
    if (!file || !file->isOpen())
        return false;

    size_t len = file->size();

    // Check if file is empty
    if (len == 0)
        return false;

    if (source_ != 0)
        delete[] source_;

    // allocate memory
    source_ = new char[len+1];

    if (source_ == 0)
        return false;   //allocation failed

    file->read(source_, len);
    source_[len] = 0; // terminate with 0

    file->close();
    delete file;

    if (compileDirectly)
        compile();

    return true;
}

bool Script::compile() {
    byteCode_ = Py_CompileString(source_, "PyTgt", Py_file_input);

    if (byteCode_ != 0) {
        compiled_ = true;
        delete[] source_;
        source_ = 0;
        return true;
    }
    else if (PyErr_ExceptionMatches (PyExc_SyntaxError)) {
        char ps1[] = ">>> ";
        char ps2[] = "... ";
        char *prompt = ps1;
        char *msg= NULL;
          PyObject *exc, *val, *trb, *obj;
          PyErr_Fetch (&exc, &val, &trb);

          if (PyArg_ParseTuple (val, "sO", &msg, &obj) &&
              !strcmp (msg, "unexpected EOF while parsing")) /* E_EOF */
          {
            Py_XDECREF (exc);
            Py_XDECREF (val);
            Py_XDECREF (trb);
            prompt = ps2;
          }
          else                                   /* some other syntax error */
          {
            PyErr_Restore (exc, val, trb);
            PyErr_Print ();
            prompt = ps1;
          }
          LERROR("Error while compiling (see stdout)");
        return false;
    }
    else {
        PyErr_Print ();
        LERROR("Error while compiling (see stdout)");
        return false;
    }

}

bool Script::run() {
    char str_pytgt[] = "PyTgt";
    if (compiled_)
        PyImport_ExecCodeModule(str_pytgt, byteCode_);
    else
        PyRun_SimpleString(source_);

    if (PyErr_Occurred()) {
        PyErr_Print ();
        return false;
    } else {
        return true;
    }
}

//------------------------------------------------------------------------------

const std::string ScriptManager::loggerCat_ = "tgt.Script.Manager";

ScriptManager::ScriptManager(bool initSignalHandlers)
    : ResourceManager<Script>()
{
    // Pass PyTgt to the Python interpreter
    char str_pytgt[] = "PyTgt";
    Py_SetProgramName(str_pytgt);

    // Initialize the Python interpreter.  Required.
    Py_InitializeEx(initSignalHandlers);
    // required in order to use threads
    PyEval_InitThreads();
}

ScriptManager::~ScriptManager() {
    // clean up python interpreter
    Py_Finalize();
}

Script* ScriptManager::load(const std::string& filename, bool compileDirectly) {
    if (isLoaded(filename)) {
        increaseUsage(filename);
        return get(filename);
    }

    Script* script = new Script();
	if (script->load(completePath(filename), compileDirectly)) {
        reg(script, filename);
        return script;
    }
    delete script;

    return 0;
}

} // namespace tgt

#endif // TGT_USE_PYTHON
