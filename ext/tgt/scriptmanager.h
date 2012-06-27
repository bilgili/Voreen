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

#ifndef TGT_SCRIPTMANAGER_H
#define TGT_SCRIPTMANAGER_H

/*
    Have a look at the script.cpp sample and at
    http://docs.python.org/api/api.html
    for more information about binding C/C++ stuff to python
*/

#ifdef TGT_USE_PYTHON

// include this at very first
#include <Python.h>

#include <string>

#include "tgt/config.h"
#include "tgt/manager.h"
#include "tgt/singleton.h"

namespace tgt {

class Script {
private:
    char*           source_;
    PyObject*       byteCode_;
    bool            compiled_;
    std::string     log_;

    static const std::string loggerCat_;

public:

    Script();
    ~Script();

    bool load(const std::string& filename, bool compileDirectly = true);

    /// if you compile the script execution will be faster and the internal string will be deleted
    bool compile();
    bool run();
    std::string log() const;
};

//------------------------------------------------------------------------------

class ScriptManager : public ResourceManager<Script> {
protected:
    static const std::string loggerCat_;

public:
    ScriptManager(bool initSignalHandlers = true);
    ~ScriptManager();

    Script* load(const std::string& filename, bool compileDirectly = true);

    /**
     * you can use this function to change sys.argv in the python enviroment
     * @param argc number of command line arguments
     * @param argv array of arguments
    */
    void static setArgv(int argc, char* argv[]);

    void static setProgramName(const std::string& prgName);
};

} // namespace tgt

#define ScriptMgr tgt::Singleton<tgt::ScriptManager>::getRef()

#endif // TGT_USE_PYTHON

#endif // TGT_SCRIPTMANAGER_H
