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

#ifdef VRN_WITH_PYTHON

// Do this at very first
#include "tgt/scriptmanager.h"

#include <QMessageBox>

static PyObject* voreenqt_messageBox(PyObject* /*self*/, PyObject* args) {
    const char* str;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    QMessageBox::information(0, "Script message", str);

    Py_RETURN_NONE;
}

static PyObject* voreenqt_questionBox(PyObject* /*self*/, PyObject* args) {
    const char* str;

    if ( !PyArg_ParseTuple(args, "s", &str) )
        return NULL;

    if (QMessageBox::question(0, "Script question", str,
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

//------------------------------------------------------------------------------
// method table
static PyMethodDef voreenqt_methods[] = {
    {
        "messageBox",
        voreenqt_messageBox,
        METH_VARARGS,
        "show a message box"
    },
    {
        "questionBox",
        voreenqt_questionBox,
        METH_VARARGS,
        "show a question box"
    },
    { NULL, NULL, 0, NULL} // sentinal
};

//------------------------------------------------------------------------------

void initVoreenqtPythonModule() {
    Py_InitModule("voreenqt", voreenqt_methods);
}

#endif // VRN_WITH_PYTHON
