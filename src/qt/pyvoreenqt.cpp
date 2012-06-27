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

#ifdef VRN_WITH_PYTHON

// Ignore "dereferencing type-punned pointer will break strict-aliasing rules" warnings on gcc
// caused by Py_RETURN_TRUE and such. The problem lies in Python so we don't want the warning
// here.
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

// Do this at very first
#include "tgt/scriptmanager.h"

#include "voreen/qt/pyvoreenqt.h"
#include "voreen/core/utils/pyvoreen.h"

#include "voreen/core/network/processornetwork.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/processors/canvasrenderer.h"
#include "voreen/qt/widgets/processor/canvasrendererwidget.h"

#include <cstdlib>
#include <QApplication>
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

static PyObject* voreenqt_quit(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    qApp->closeAllWindows();
    exit(0);
    Py_RETURN_NONE;
}

static PyObject* voreenqt_processEvents(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    qApp->processEvents();
    Py_RETURN_NONE;
}

static PyObject* voreenqt_repaintCanvas(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    using namespace voreen;
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    const std::vector<Processor*> processors = p.getEvaluator()->getProcessorNetwork()->getProcessors();
    for (std::vector<Processor*>::const_iterator iter = processors.begin(); iter != processors.end(); iter++) {
        CanvasRenderer* canvasProc = dynamic_cast<CanvasRenderer*>(*iter);
        if (canvasProc && canvasProc->getCanvas()) {
            canvasProc->getCanvas()->repaint();
        }
    }

    Py_RETURN_NONE;
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
    {
        "quit",
        voreenqt_quit,
        METH_VARARGS,
        "quit the application"
    },
    {
        "processEvents",
        voreenqt_processEvents,
        METH_VARARGS,
        "processes all events waiting in the Qt event queue"
    },
    {
        "repaintCanvas",
        voreenqt_repaintCanvas,
        METH_VARARGS,
        "forces a repaint of the first canvas"
    },
    { NULL, NULL, 0, NULL} // sentinal
};

//------------------------------------------------------------------------------

namespace voreen {

VoreenPythonQt::VoreenPythonQt() {
    Py_InitModule("voreenqt", voreenqt_methods);
}

} // namespace

#endif // VRN_WITH_PYTHON
