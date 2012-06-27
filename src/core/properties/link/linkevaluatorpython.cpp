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
// Python header must be included before other system headers
#include <Python.h>
#endif

#include "voreen/core/properties/link/linkevaluatorpython.h"
#include "voreen/core/properties/link/scriptmanagerlinking.h"
#include "voreen/core/properties/link/boxobject.h"

#include "voreen/core/properties/property.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "tgt/logmanager.h"
#include <string>
#include <vector>

#ifdef VRN_WITH_PYTHON

#if PY_VERSION_HEX < 0x02050000
// support Python < 2.5
#define PyInt_AsSsize_t(o) PyInt_AsLong(o)
#endif

// Ignore "dereferencing type-punned pointer will break strict-aliasing rules" warnings on gcc
// caused by Py_RETURN_TRUE and such. The problem lies in Python so we don't want the warning
// here.
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace {

PyObject* module;
PyObject* function;
PyObject* globals;

PyObject* pyError_type;
PyObject* pyError_value;
PyObject* pyError_traceback;
PyObject* pyError_string;

#define VecToPyVec(VARIABLE, CONVERTER, GETVEC, SIZE, VALUEPOSTFIX) \
    VARIABLE = PyList_New(SIZE); \
    for(unsigned int i=0; i<SIZE; i++){ \
        PyList_SetItem(VARIABLE, i, CONVERTER(GETVEC[i]VALUEPOSTFIX)); \
    }

#define PyVecToVec(PYVEC, CONVERTER, VEC) \
    { \
        PyObject* pyList = PYVEC; \
        for(int i=0; i < PyList_Size(pyList); i++) { \
            VEC[i] = (CONVERTER(PyList_GetItem(pyList, i))); \
        } \
    }
} // namespace

#endif

using tgt::Camera;

namespace voreen {

LinkEvaluatorPython::LinkEvaluatorPython(const std::string& functionName) {
    // Py_Initialize() may be called several times without harming performance.
    // The documentation states:
    // "This is a no-op when called for a second time
    // (without calling Py_Finalize() first)."
    init(functionName);
}

LinkEvaluatorPython::LinkEvaluatorPython() {
}

LinkEvaluatorPython::~LinkEvaluatorPython() {
    // Py_Finalize may lead to problems with other parts of voreen using Python.
    // Please double check before uncommenting!
#ifdef VRN_WITH_PYTHON
    // Py_Finalize();
#endif
}

void LinkEvaluatorPython::setScript(const std::string& script) {
    LinkEvaluatorFactory::getInstance()->getScriptManager()->setPythonScript(functionName_, script);
}

std::string LinkEvaluatorPython::getScript() const {
    return LinkEvaluatorFactory::getInstance()->getScriptManager()->getPythonScript(functionName_);
}

std::string LinkEvaluatorPython::getFunctionName() const {
    return functionName_;
}

void LinkEvaluatorPython::init(const std::string& functionName) {
    functionName_ = functionName;

#ifdef VRN_WITH_PYTHON
    Py_Initialize();
    module = PyImport_AddModule("__main__");
    checkPythonErrorState();
    globals = PyModule_GetDict(module);
    checkPythonErrorState();
    prepareContext();
    checkPythonErrorState();
#endif
}


#ifdef VRN_WITH_PYTHON

// anonymous namespace, so we don't need PyObject in the header
namespace {

/*
 * Convert the Python Camera object to a C++ Camera object
 */
Camera* pyCameraToCamera(PyObject* pyCamera) {
    PyObject* pyPosition = PyList_GetItem(pyCamera, 0);
    tgt::vec3 position(
        (float)PyFloat_AsDouble(PyList_GetItem(pyPosition, 0)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyPosition, 1)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyPosition, 2)));

    PyObject* pyFocus = PyList_GetItem(pyCamera, 1);
    tgt::vec3 focus (
        (float)PyFloat_AsDouble(PyList_GetItem(pyFocus, 0)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyFocus, 1)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyFocus, 2)));

    PyObject* pyUp = PyList_GetItem(pyCamera, 2);
    tgt::vec3 up (
        (float)PyFloat_AsDouble(PyList_GetItem(pyUp, 0)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyUp, 1)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyUp, 2)));

    float fovy = (float)PyFloat_AsDouble(PyList_GetItem(pyCamera, 3));

    float ratio = (float)PyFloat_AsDouble(PyList_GetItem(pyCamera, 4));

    float distn = (float)PyFloat_AsDouble(PyList_GetItem(pyCamera, 5));

    float distf = (float)PyFloat_AsDouble(PyList_GetItem(pyCamera, 6));

    return new Camera(position, focus, up, fovy, ratio, distn, distf);
}

/*
 * Converts a C++ Camera object to a Python Camera object
 */
PyObject* cameraToPyCamera(const Camera* camera) {
    PyObject* pyCamera = PyList_New(7);

    PyObject* pyPosition = PyList_New(3);
    PyList_SetItem(pyPosition, 0, PyFloat_FromDouble(camera->getPosition()[0]));
    PyList_SetItem(pyPosition, 1, PyFloat_FromDouble(camera->getPosition()[1]));
    PyList_SetItem(pyPosition, 2, PyFloat_FromDouble(camera->getPosition()[2]));
    PyList_SetItem(pyCamera, 0, pyPosition);

    PyObject* pyFocus = PyList_New(3);
    PyList_SetItem(pyFocus, 0, PyFloat_FromDouble(camera->getFocus()[0]));
    PyList_SetItem(pyFocus, 1, PyFloat_FromDouble(camera->getFocus()[1]));
    PyList_SetItem(pyFocus, 2, PyFloat_FromDouble(camera->getFocus()[2]));
    PyList_SetItem(pyCamera, 1, pyFocus);

    PyObject* pyUp = PyList_New(3);
    PyList_SetItem(pyUp, 0, PyFloat_FromDouble(camera->getUpVector()[0]));
    PyList_SetItem(pyUp, 1, PyFloat_FromDouble(camera->getUpVector()[1]));
    PyList_SetItem(pyUp, 2, PyFloat_FromDouble(camera->getUpVector()[2]));
    PyList_SetItem(pyCamera, 2, pyUp);

    PyList_SetItem(pyCamera, 3, PyFloat_FromDouble(camera->getFovy()));

    PyList_SetItem(pyCamera, 4, PyFloat_FromDouble(camera->getRatio()));

    PyList_SetItem(pyCamera, 5, PyFloat_FromDouble(camera->getNearDist()));

    PyList_SetItem(pyCamera, 6, PyFloat_FromDouble(camera->getFarDist()));

    return pyCamera;
}

/*
 * Convert a Python Transfunc object to a C++ Transfunc object
 */
TransFuncIntensity* pyTransFuncToTransFunc(PyObject* pyTransFunc) {
    PyObject* pyThresholds = PyList_GetItem(pyTransFunc, 0);
    tgt::vec2 thresholds(
        (float)PyFloat_AsDouble(PyList_GetItem(pyThresholds, 0)),
        (float)PyFloat_AsDouble(PyList_GetItem(pyThresholds, 1)));

    PyObject* pyTextureDimension = PyList_GetItem(pyTransFunc, 1);
    int textureWidth = PyInt_AsLong(PyList_GetItem(pyTextureDimension, 0));
    int textureHeight = PyInt_AsLong(PyList_GetItem(pyTextureDimension, 1));

    PyObject* pyKeys = PyList_GetItem(pyTransFunc, 2);
    std::vector<TransFuncMappingKey*> keys = std::vector<TransFuncMappingKey*>();
    for(int i=0; i < PyList_Size(pyKeys); i++) {
        PyObject* pyKey = PyList_GetItem(pyKeys, i);
        float intensity = (float)PyFloat_AsDouble(PyList_GetItem(pyKey, 0));
        bool isSplit = (PyList_GetItem(pyKey, 1) == Py_True);

        PyObject* pyColorL = PyList_GetItem(pyKey, 2);
        tgt::col4 colorL = tgt::col4(
            PyInt_AsSsize_t(PyList_GetItem(pyColorL, 0)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorL, 1)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorL, 2)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorL, 3)));

        float alphaL = (float)PyFloat_AsDouble(PyList_GetItem(pyKey, 3));

        PyObject* pyColorR = PyList_GetItem(pyKey, 4);
        tgt::col4 colorR = tgt::col4(
            PyInt_AsSsize_t(PyList_GetItem(pyColorR, 0)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorR, 1)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorR, 2)),
            PyInt_AsSsize_t(PyList_GetItem(pyColorR, 3)));

        float alphaR = (float)PyFloat_AsDouble(PyList_GetItem(pyKey, 5));

        TransFuncMappingKey* key = new TransFuncMappingKey(intensity, colorL);
        key->setSplit(isSplit, true);
        if(isSplit) {
            key->setColorL(colorL);
            key->setAlphaL(alphaL);
            key->setColorR(colorR);
            key->setAlphaR(alphaR);
        }

        keys.push_back(key);
    }

    TransFuncIntensity* tf = new TransFuncIntensity(keys.size());
    tf->resize(textureWidth, textureHeight);
    tf->setThresholds(thresholds);
    tf->setKeys(keys);
    tf->updateTexture();
    return tf;
}

/*
 * Converts a C++ Transfunc object to a Python Transfunc object
 */
PyObject* transFuncToPyTransFunc(const TransFuncIntensity* transFunc) {
    PyObject* tfi = PyList_New(3);

    PyObject* pyThresholds = PyList_New(2);
    tgt::vec2 thresholds = transFunc->getThresholds();
    PyList_SetItem(pyThresholds, 0, PyFloat_FromDouble(thresholds.x));
    PyList_SetItem(pyThresholds, 1, PyFloat_FromDouble(thresholds.y));
    PyList_SetItem(tfi, 0, pyThresholds);

    PyObject* pyDimension = PyList_New(2);
    const tgt::Texture* texture = const_cast<TransFuncIntensity*>(transFunc)->getTexture();
    PyList_SetItem(pyDimension, 0, PyInt_FromLong(texture->getWidth()));
    PyList_SetItem(pyDimension, 1, PyInt_FromLong(texture->getHeight()));
    PyList_SetItem(tfi, 1, pyDimension);

    PyObject* keys = PyList_New(transFunc->getNumKeys());
    for(int i = 0; i<transFunc->getNumKeys(); i++) {
        TransFuncMappingKey* key = transFunc->getKey(i);
        PyObject* pyKey = PyList_New(6);

        PyList_SetItem(pyKey, 0, PyFloat_FromDouble(key->getIntensity()));
        PyList_SetItem(pyKey, 1, PyBool_FromLong((int)key->isSplit()));

        PyObject* colorL = PyList_New(4);
        PyList_SetItem(colorL, 0, PyInt_FromLong(key->getColorL()[0]));
        PyList_SetItem(colorL, 1, PyInt_FromLong(key->getColorL()[1]));
        PyList_SetItem(colorL, 2, PyInt_FromLong(key->getColorL()[2]));
        PyList_SetItem(colorL, 3, PyInt_FromLong(key->getColorL()[3]));
        PyList_SetItem(pyKey, 2, colorL);

        PyList_SetItem(pyKey, 3, PyFloat_FromDouble(key->getAlphaL()));

        PyObject* colorR = PyList_New(4);
        PyList_SetItem(colorR, 0, PyInt_FromLong(key->getColorR()[0]));
        PyList_SetItem(colorR, 1, PyInt_FromLong(key->getColorR()[1]));
        PyList_SetItem(colorR, 2, PyInt_FromLong(key->getColorR()[2]));
        PyList_SetItem(colorR, 3, PyInt_FromLong(key->getColorR()[3]));
        PyList_SetItem(pyKey, 4, colorR);

        PyList_SetItem(pyKey, 5, PyFloat_FromDouble(key->getAlphaR()));

        PyList_SetItem(keys, i, pyKey);
    }
    PyList_SetItem(tfi, 2, keys);
    return tfi;
}

/*
 * Converts a C++ Shader object to a Python Shader object
 */
PyObject* shaderToPyShader(ShaderSource shaderSource) {
    PyObject* pyShader = PyList_New(9);
    PyList_SetItem(pyShader, 0, PyString_FromString(shaderSource.geometryFilename_.c_str()));
    PyList_SetItem(pyShader, 1, PyString_FromString(shaderSource.vertexFilename_.c_str()));
    PyList_SetItem(pyShader, 2, PyString_FromString(shaderSource.fragmentFilename_.c_str()));
    PyList_SetItem(pyShader, 3, PyString_FromString(shaderSource.geometrySource_.c_str()));
    PyList_SetItem(pyShader, 4, PyString_FromString(shaderSource.vertexSource_.c_str()));
    PyList_SetItem(pyShader, 5, PyString_FromString(shaderSource.fragmentSource_.c_str()));
    PyList_SetItem(pyShader, 6, (shaderSource.geometryModified_ ? Py_True : Py_False));
    PyList_SetItem(pyShader, 7, (shaderSource.vertexModified_ ? Py_True : Py_False));
    PyList_SetItem(pyShader, 8, (shaderSource.fragmentModified_ ? Py_True : Py_False));
    return pyShader;
}

/*
 * Convert a Python Shader object to a C++ Shader object
 */
ShaderSource pyShaderToShader(PyObject* pyShader) {
    ShaderSource shaderSource = ShaderSource(
        PyString_AsString(PyList_GetItem(pyShader, 0)),
        PyString_AsString(PyList_GetItem(pyShader, 1)),
        PyString_AsString(PyList_GetItem(pyShader, 2)));
    shaderSource.geometrySource_ = PyString_AsString(PyList_GetItem(pyShader, 3));
    shaderSource.vertexSource_ = PyString_AsString(PyList_GetItem(pyShader, 4));
    shaderSource.fragmentSource_ = PyString_AsString(PyList_GetItem(pyShader, 5));
    shaderSource.geometryModified_ = (PyList_GetItem(pyShader, 6) == Py_True);
    shaderSource.vertexModified_ = (PyList_GetItem(pyShader, 7) == Py_True);
    shaderSource.fragmentModified_ = (PyList_GetItem(pyShader, 8) == Py_True);
    return shaderSource;
}

} // anonymous namespace

#endif // VRN_WITH_PYTHON

void LinkEvaluatorPython::serialize(XmlSerializer& s) const {
    LinkEvaluatorBase::serialize(s);

    s.serialize("functionName", functionName_);
}

void LinkEvaluatorPython::deserialize(XmlDeserializer& s) {
    LinkEvaluatorBase::deserialize(s);

    s.deserialize("functionName", functionName_);
}

#ifdef VRN_WITH_PYTHON
void LinkEvaluatorPython::prepareContext() {
    try {
        PyRun_String(getScript().c_str(), Py_file_input, globals, globals);
    } catch (...) {
        throw VoreenException("Python: Could not compile module.");
    }
    checkPythonErrorState();
    if (!module) {
        throw VoreenException("Python: Module not found.");
    }
#if PY_VERSION_HEX >= 0x02050000
    function = PyObject_GetAttrString(module, functionName_.c_str());
#else
    function = PyObject_GetAttrString(module, const_cast<char*>(functionName_.c_str()));
#endif
    if (!function) {
        throw VoreenException("Python: Function not found.");
    }
}

void LinkEvaluatorPython::checkPythonErrorState() {
    if(PyErr_Occurred()){
        std::string pyException = "Python Error:\n";
        PyErr_Fetch(&pyError_type, &pyError_value, &pyError_traceback);
        // PyErr_Restore(pyError_type, pyError_value, pyError_traceback);
        // PyErr_Print();
        PyErr_Restore(pyError_type, pyError_value, pyError_traceback);

        pyError_string = NULL;
        if(pyError_type != NULL && (pyError_string = PyObject_Str(pyError_type)) != NULL && (PyString_Check(pyError_string))) {
            pyException.append(PyString_AsString(pyError_string));
        }
        else {
            pyException.append("<Exception unknown>");
        }
        pyException.append("\n");
        // Py_XDECREF(pyError_string);

        pyError_string = NULL;
        if(pyError_value != NULL && (pyError_string = PyObject_Str(pyError_value)) != NULL && (PyString_Check(pyError_string))) {
            pyException.append(PyString_AsString(pyError_string));
        }
        else {
            pyException.append("<No data available>");
        }
        pyException.append("\n");
        // Py_XDECREF(pyError_string);

        pyError_string = NULL;
        if(pyError_traceback != NULL && (pyError_string = PyObject_Str(pyError_traceback)) != NULL && (PyString_Check(pyError_string))) {
            pyException.append(PyString_AsString(pyError_string));
        }
        else {
            pyException.append("<No stacktrace available>");
        }
        pyException.append("\n");
        // Py_XDECREF(pyError_string);

        /*
        Py_XDECREF(pyError_type);
        Py_XDECREF(pyError_value);
        Py_XDECREF(pyError_traceback);
        */

        PyErr_Clear();

        LERRORC("voreen.LinkEvaluatorPython", pyException);

        throw VoreenException(pyException);
    }
}
#endif

#ifndef VRN_WITH_PYTHON
BoxObject LinkEvaluatorPython::eval(const BoxObject& /*sourceOld*/, const BoxObject& /*sourceNew*/, const BoxObject& targetOld, Property* /*src*/, Property* /*dest*/) {
    return targetOld;
}
#else
BoxObject LinkEvaluatorPython::eval(const BoxObject& sourceOld, const BoxObject& sourceNew, const BoxObject& targetOld, Property* /*src*/, Property* /*dest*/) {
    prepareContext();
    checkPythonErrorState();

    PyObject* args;
    PyObject* ret;

    BoxObject result;
    args = PyTuple_New(3);
    PyObject* value0;
    PyObject* value1;
    PyObject* value2;

    BoxObject::BoxObjectType type = sourceOld.getType();
    switch(type) {
    case BoxObject::BOOL:
        value0 = PyBool_FromLong(sourceOld.getLong());
        checkPythonErrorState();
        value1 = PyBool_FromLong(sourceNew.getLong());
        checkPythonErrorState();
        break;
    case BoxObject::DOUBLE:
    case BoxObject::FLOAT:
        value0 = PyFloat_FromDouble(sourceOld.getDouble());
        checkPythonErrorState();
        value1 = PyFloat_FromDouble(sourceNew.getDouble());
        checkPythonErrorState();
        break;
    case BoxObject::INTEGER:
        value0 = PyInt_FromLong(sourceOld.getInt());
        checkPythonErrorState();
        value1 = PyInt_FromLong(sourceNew.getInt());
        checkPythonErrorState();
        break;
    case BoxObject::IVEC2:
        VecToPyVec(value0, PyInt_FromLong, sourceOld.getIVec2(), 2, );
        checkPythonErrorState();
        VecToPyVec(value1, PyInt_FromLong, sourceNew.getIVec2(), 2, );
        checkPythonErrorState();
        break;
    case BoxObject::IVEC3:
        VecToPyVec(value0, PyInt_FromLong, sourceOld.getIVec3(), 3, );
        checkPythonErrorState();
        VecToPyVec(value1, PyInt_FromLong, sourceNew.getIVec3(), 3, );
        checkPythonErrorState();
        break;
    case BoxObject::IVEC4:
        VecToPyVec(value0, PyInt_FromLong, sourceOld.getIVec4(), 4, );
        checkPythonErrorState();
        VecToPyVec(value1, PyInt_FromLong, sourceNew.getIVec4(), 4, );
        checkPythonErrorState();
        break;
    case BoxObject::LONG:
        value0 = PyLong_FromLong(sourceOld.getLong());
        checkPythonErrorState();
        value1 = PyLong_FromLong(sourceNew.getLong());
        checkPythonErrorState();
        break;
    case BoxObject::STRING:
        value0 = PyString_FromString(sourceOld.getString().c_str());
        checkPythonErrorState();
        value1 = PyString_FromString(sourceNew.getString().c_str());
        checkPythonErrorState();
        break;
    case BoxObject::STRINGVEC:
        VecToPyVec(value0, PyString_FromString, sourceOld.getStringVec(), sourceOld.getStringVec().size(), .c_str());
        checkPythonErrorState();
        VecToPyVec(value1, PyString_FromString, sourceNew.getStringVec(), sourceNew.getStringVec().size(), .c_str());
        checkPythonErrorState();
        break;
    case BoxObject::VEC2:
        VecToPyVec(value0, PyFloat_FromDouble, sourceOld.getVec2(), 2, );
        checkPythonErrorState();
        VecToPyVec(value1, PyFloat_FromDouble, sourceNew.getVec2(), 2, );
        checkPythonErrorState();
        break;
    case BoxObject::VEC3:
        VecToPyVec(value0, PyFloat_FromDouble, sourceOld.getVec3(), 3, );
        checkPythonErrorState();
        VecToPyVec(value1, PyFloat_FromDouble, sourceNew.getVec3(), 3, );
        checkPythonErrorState();
        break;
    case BoxObject::VEC4:
        VecToPyVec(value0, PyFloat_FromDouble, sourceOld.getVec4(), 4, );
        checkPythonErrorState();
        VecToPyVec(value1, PyFloat_FromDouble, sourceNew.getVec4(), 4, );
        checkPythonErrorState();
        break;
    case BoxObject::TRANSFUNC:
        {
            const TransFuncIntensity* sourceTfOld = dynamic_cast<const TransFuncIntensity*>(sourceOld.getTransFunc());
            const TransFuncIntensity* sourceTfNew = dynamic_cast<const TransFuncIntensity*>(sourceNew.getTransFunc());
            if (sourceTfOld && sourceTfNew) {
                value0 = transFuncToPyTransFunc(sourceTfOld);
                checkPythonErrorState();
                value1 = transFuncToPyTransFunc(sourceTfNew);
                checkPythonErrorState();
                break;
            }
        }
    case BoxObject::CAMERA:
        value0 = cameraToPyCamera(sourceOld.getCamera());
        checkPythonErrorState();
        value1 = cameraToPyCamera(sourceNew.getCamera());
        checkPythonErrorState();
        break;
    case BoxObject::SHADER:
        value0 = shaderToPyShader(sourceOld.getShader());
        checkPythonErrorState();
        value1 = shaderToPyShader(sourceNew.getShader());
        checkPythonErrorState();
        break;
    default:
        throw VoreenException("LinkEvaluatorPython: Unsupported property type");
    }
    PyTuple_SetItem(args, 0, value0);
    checkPythonErrorState();
    PyTuple_SetItem(args, 1, value1);
    checkPythonErrorState();

    type = targetOld.getType();
    switch(type) {
    case BoxObject::BOOL:
        value2 = PyBool_FromLong(targetOld.getLong());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(BoxObject(PyString_AsString(ret)).getBool());
        checkPythonErrorState();
        break;
    case BoxObject::DOUBLE:
        value2 = PyFloat_FromDouble(targetOld.getDouble());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(PyFloat_AsDouble(ret));
        checkPythonErrorState();
        break;
    case BoxObject::FLOAT:
        value2 = PyFloat_FromDouble(targetOld.getDouble());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject((float)PyFloat_AsDouble(ret));
        checkPythonErrorState();
        break;
    case BoxObject::INTEGER:
        value2 = PyInt_FromLong(targetOld.getInt());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject((int)PyInt_AsLong(ret));
        checkPythonErrorState();
        break;
    case BoxObject::IVEC2:
        {
            VecToPyVec(value2, PyInt_FromLong, targetOld.getIVec2(), 2, );
            checkPythonErrorState();
            PyTuple_SetItem(args, 2, value2);
            checkPythonErrorState();
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::ivec2 resultVec = tgt::ivec2();
            PyVecToVec(ret, (int)PyInt_AsLong, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::IVEC3:
        {
            VecToPyVec(value2, PyInt_FromLong, targetOld.getIVec3(), 3, );
            checkPythonErrorState();
            PyTuple_SetItem(args, 2, value2);
            checkPythonErrorState();
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::ivec3 resultVec = tgt::ivec3();
            PyVecToVec(ret, (int)PyInt_AsLong, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::IVEC4:
        {
            VecToPyVec(value2, PyInt_FromLong, targetOld.getIVec4(), 4, );
            checkPythonErrorState();
            PyTuple_SetItem(args, 2, value2);
            checkPythonErrorState();
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::ivec4 resultVec = tgt::ivec4();
            PyVecToVec(ret, (int)PyInt_AsLong, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::LONG:
        value2 = PyLong_FromLong(targetOld.getLong());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(PyLong_AsLong(ret));
        checkPythonErrorState();
        break;
    case BoxObject::STRING:
        value2 = PyString_FromString(targetOld.getString().c_str());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(PyString_AsString(ret));
        checkPythonErrorState();
        break;
    case BoxObject::STRINGVEC:
        {
            VecToPyVec(value2, PyString_FromString, targetOld.getStringVec(), targetOld.getStringVec().size(), .c_str());
            checkPythonErrorState();
            PyTuple_SetItem(args, 2, value2);
            checkPythonErrorState();
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            std::vector<std::string> resultVec = std::vector<std::string>();
            PyVecToVec(ret, PyString_AsString, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::VEC2:
        {
            VecToPyVec(value2, PyFloat_FromDouble, targetOld.getVec2(), 2, );
            PyTuple_SetItem(args, 2, value2);
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::vec2 resultVec = tgt::vec2();
            PyVecToVec(ret, (float)PyFloat_AsDouble, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::VEC3:
        {
            VecToPyVec(value2, PyFloat_FromDouble, targetOld.getVec3(), 3, );
            PyTuple_SetItem(args, 2, value2);
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::vec3 resultVec = tgt::vec3();
            PyVecToVec(ret, (float)PyFloat_AsDouble, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
            checkPythonErrorState();
        }
        break;
    case BoxObject::VEC4:
        {
            VecToPyVec(value2, PyFloat_FromDouble, targetOld.getVec4(), 4, );
            PyTuple_SetItem(args, 2, value2);
            ret = PyObject_CallObject(function, args);
            checkPythonErrorState();
            tgt::vec4 resultVec = tgt::vec4();
            PyVecToVec(ret, (float)PyFloat_AsDouble, resultVec);
            checkPythonErrorState();
            result = BoxObject(resultVec);
        }
        break;
    case BoxObject::TRANSFUNC:
        {
            const TransFuncIntensity* sourceTfOld = dynamic_cast<const TransFuncIntensity*>(sourceOld.getTransFunc());
            if (sourceTfOld) {
                value2 = transFuncToPyTransFunc(sourceTfOld);
                PyTuple_SetItem(args, 2, value2);
                ret = PyObject_CallObject(function, args);
                checkPythonErrorState();
                result = BoxObject(pyTransFuncToTransFunc(ret));
                checkPythonErrorState();
                break;
            }
        }
    case BoxObject::CAMERA:
        value2 = cameraToPyCamera(targetOld.getCamera());
        PyTuple_SetItem(args, 2, value2);
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(pyCameraToCamera(ret));
        checkPythonErrorState();
        break;
    case BoxObject::SHADER:
        value2 = shaderToPyShader(targetOld.getShader());
        checkPythonErrorState();
        PyTuple_SetItem(args, 2, value2);
        checkPythonErrorState();
        ret = PyObject_CallObject(function, args);
        checkPythonErrorState();
        result = BoxObject(pyShaderToShader(ret));
        checkPythonErrorState();
        break;
    default:
        checkPythonErrorState();
        throw VoreenException("LinkEvaluatorPython: Unsupported property type");
    }
    checkPythonErrorState();
    return result;
}
#endif // VRN_WITH_PYTHON

std::string LinkEvaluatorPython::name() const {
    return getFunctionName();
}

} // namespace
