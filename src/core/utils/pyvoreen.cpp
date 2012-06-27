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
#include "voreen/core/utils/pyvoreen.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/network/networkevaluator.h"
#include "voreen/core/processors/canvasrenderer.h"
#include "voreen/core/processors/processorwidget.h"
#ifdef VRN_MODULE_BASE
#include "voreen/modules/base/processors/entryexitpoints/entryexitpoints.h"
#include "voreen/modules/base/processors/entryexitpoints/meshentryexitpoints.h"
#include "voreen/modules/base/processors/utility/clockprocessor.h"
#endif
#include "voreen/core/interaction/voreentrackball.h"

static PyObject* voreen_setViewport(PyObject* /*self*/, PyObject* args) {
    int i1, i2;

    using namespace voreen;

    if (!PyArg_ParseTuple(args, "ii", &i1, &i2))
        return NULL;

    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();

    const std::vector<Processor*> processors = p.getEvaluator()->getProcessorNetwork()->getProcessors();

    for (std::vector<Processor*>::const_iterator iter = processors.begin(); iter != processors.end(); iter++) {
        CanvasRenderer* canvasProc = dynamic_cast<CanvasRenderer*>(*iter);
        if (canvasProc && canvasProc->getProcessorWidget())
            canvasProc->getProcessorWidget()->setSize(i1, i2);
    }

    Py_RETURN_NONE;

}

static PyObject* voreen_snapshot(PyObject* /*self*/, PyObject* args) {
    const char* filename = 0;

    if (!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

    using namespace voreen;

    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();

    const std::vector<Processor*> processors = p.getEvaluator()->getProcessorNetwork()->getProcessors();

    for (std::vector<Processor*>::const_iterator iter = processors.begin(); iter != processors.end(); iter++) {
        CanvasRenderer* canvasProc = dynamic_cast<CanvasRenderer*>(*iter);
        if (canvasProc) {
            bool success = canvasProc->renderToImage(filename, canvasProc->getCanvas()->getSize());
            if (!success)
                LERRORC("voreen.python", "renderToImage() failed: " << canvasProc->getRenderToImageError());
            break;
        }
    }

    Py_RETURN_NONE;
}

static PyObject* voreen_canvas_count(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    using namespace voreen;

    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();

    int count = 0;
    const std::vector<Processor*> processors = p.getEvaluator()->getProcessorNetwork()->getProcessors();

    for (std::vector<Processor*>::const_iterator iter = processors.begin(); iter != processors.end(); iter++) {
        if (dynamic_cast<CanvasRenderer*>(*iter))
            count++;
    }

    PyObject* py_count = Py_BuildValue("i", count);
    Py_INCREF(py_count);
    return py_count;
}

static PyObject* voreen_canvas_snapshot(PyObject* /*self*/, PyObject* args) {
    int index;
    const char* filename = 0;
    if (!PyArg_ParseTuple(args, "is", &index, &filename))
        return NULL;

    using namespace voreen;

    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();

    int count = 0;
    const std::vector<Processor*> processors = p.getEvaluator()->getProcessorNetwork()->getProcessors();

    for (std::vector<Processor*>::const_iterator iter = processors.begin(); iter != processors.end(); iter++) {
        CanvasRenderer* canvasProc = dynamic_cast<CanvasRenderer*>(*iter);
        if (canvasProc) {
            if (count == index) {
                bool success = canvasProc->renderToImage(filename, canvasProc->getCanvas()->getSize());
                if (!success)
                    LERRORC("voreen.python", "renderToImage() failed: " << canvasProc->getRenderToImageError());
                break;
            }
            count++;
        }
    }

    Py_RETURN_NONE;
}

static PyObject* voreen_resetCamera(PyObject* /*self*/, PyObject* /*args*/) {
    using namespace voreen;

#ifdef VRN_MODULE_BASE
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    std::vector<EntryExitPoints*> eep = p.getEvaluator()->getProcessorNetwork()->getProcessorsByType<EntryExitPoints>();
    for (size_t i=0; i < eep.size(); i++) {
        const std::vector<Property*>& properties = eep[i]->getProperties();
        for (size_t j = 0; j < properties.size(); ++j) {
            CameraProperty* cp = dynamic_cast<CameraProperty*>(properties[j]);
            if (cp) {
                tgt::Camera* cam = cp->get();
                cam->setPosition(tgt::vec3(0.0f, 0.0f, 3.75f));
                cam->setFocus(tgt::vec3(0.0f, 0.0f, 0.0f));
                cam->setUpVector(tgt::vec3(0.0f, 1.0f, 0.0f));
                cp->notifyChange();
            }
        }
    }
#endif
    Py_RETURN_NONE;
}

static PyObject* voreen_rotateCamera(PyObject* /*self*/, PyObject* args) {
    using namespace voreen;

    float f1, f2, f3, f4;
    if (!PyArg_ParseTuple(args, "ffff", &f1, &f2, &f3, &f4))
        return NULL;

#ifdef VRN_MODULE_BASE
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    std::vector<EntryExitPoints*> eep = p.getEvaluator()->getProcessorNetwork()->getProcessorsByType<EntryExitPoints>();
    if (eep.size() > 0) {
        const std::vector<Property*>& properties = eep[0]->getProperties();
        for (size_t j = 0; j < properties.size(); ++j) {
            CameraProperty* cp = dynamic_cast<CameraProperty*>(properties[j]);
            if (cp) {
                tgt::Camera* cam = cp->get();
                VoreenTrackball track(cam);
                track.rotate(tgt::quat::createQuat(f1, tgt::vec3(f2, f3, f4)));
                cp->notifyChange();
                break;
            }
        }
    }
    else {
        //fix/hack to support MeshEntryExitPoints:
        std::vector<MeshEntryExitPoints*> eep = p.getEvaluator()->getProcessorNetwork()->getProcessorsByType<MeshEntryExitPoints>();
        if (eep.size() > 0) {
            const std::vector<Property*>& properties = eep[0]->getProperties();
            for (size_t j = 0; j < properties.size(); ++j) {
                CameraProperty* cp = dynamic_cast<CameraProperty*>(properties[j]);
                if (cp) {
                    tgt::Camera* cam = cp->get();
                    VoreenTrackball track(cam);
                    track.rotate(tgt::quat::createQuat(f1, tgt::vec3(f2, f3, f4)));
                    cp->notifyChange();
                    break;
                }
            }
        }
    }
#endif
    Py_RETURN_NONE;
}

static PyObject* voreen_invalidateProcessors(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    using namespace voreen;
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    p.getEvaluator()->invalidateProcessors();

    Py_RETURN_NONE;
}

static PyObject* voreen_tickClockProcessor(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

#ifdef VRN_MODULE_BASE
    using namespace voreen;
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    std::vector<ClockProcessor*> clocks = p.getEvaluator()->getProcessorNetwork()->getProcessorsByType<ClockProcessor>();
    if (clocks.size() > 0)
        clocks[0]->timerEvent(0);
#endif

    Py_RETURN_NONE;
}

static PyObject* voreen_resetClockProcessor(PyObject* /*self*/, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

#ifdef VRN_MODULE_BASE
    using namespace voreen;
    VoreenPython& p = tgt::Singleton<VoreenPython>::getRef();
    std::vector<ClockProcessor*> clocks = p.getEvaluator()->getProcessorNetwork()->getProcessorsByType<ClockProcessor>();
    if (clocks.size() > 0)
        clocks[0]->resetCounter();
#endif

    Py_RETURN_NONE;
}

//------------------------------------------------------------------------------

// method table
static PyMethodDef voreen_methods[] = {
    {
        "setViewport",
        voreen_setViewport,
        METH_VARARGS,
        "sets the viewport on all processors"
    },
    {
        "snapshot",
        voreen_snapshot,
        METH_VARARGS,
        "saves a snapshot of the first canvas in the given file"
    },
    {
        "snapshotCanvas",
        voreen_canvas_snapshot,
        METH_VARARGS,
        "saves a snapshot of the given canvas"
    },
    {
        "canvasCount",
        voreen_canvas_count,
        METH_VARARGS,
        "returns the number of canvases"
    },
    {
        "resetCamera",
        voreen_resetCamera,
        METH_VARARGS,
        "resets the camera"
    },
    {
        "rotateCamera",
        voreen_rotateCamera,
        METH_VARARGS,
        "rotates the camera"
    },
    {
        "invalidateProcessors",
        voreen_invalidateProcessors,
        METH_VARARGS,
        "Invalidates all processors in the current network."
    },
    {
        "tickClockProcessor",
        voreen_tickClockProcessor,
        METH_VARARGS,
        "Sends a tick to a clock processor."
    },
    {
        "resetClockProcessor",
        voreen_resetClockProcessor,
        METH_VARARGS,
        "Sets the clock back to zero."
    },
    { NULL, NULL, 0, NULL} // sentinal
};

//------------------------------------------------------------------------------

namespace voreen {

VoreenPython::VoreenPython()
    : eval_(0)
{
    Py_InitModule("voreen", voreen_methods);
}

void VoreenPython::setEvaluator(NetworkEvaluator* eval) {
    eval_ = eval;
}

} // namespace voreen

#endif // VRN_WITH_PYTHON
