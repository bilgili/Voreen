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

// Do this at very first
#include "tgt/scriptmanager.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/transfunc/transfunc.h"

// static voreen::TransFunc* tf1 = 0;
// static voreen::TransFunc* tf2 = 0;
// static voreen::TransFunc* tf3 = 0;

// static voreen::TransFunc* itf1 = 0;
// static voreen::TransFunc* itf2 = 0;
// static voreen::TransFunc* itf3 = 0;

/*
    wrappers for python
*/

static PyObject* voreen_postMsg(PyObject* /*self*/, PyObject* args) {
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ss", &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::Message(voreen::Identifier(msg)), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postFloatMsg(PyObject* /*self*/, PyObject* args) {
    float f;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "fss", &f, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::FloatMsg(voreen::Identifier(msg), f), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postIntMsg(PyObject* /*self*/, PyObject* args) {
    int i;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "iss", &i, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::IntMsg(voreen::Identifier(msg), i), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postBoolMsg(PyObject* /*self*/, PyObject* args) {
    int i;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "iss", &i, &msg, &dst))
        return NULL;

    bool b = i ? true : false;

    MsgDistr.postMessage( new voreen::BoolMsg(voreen::Identifier(msg), b), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postSizeTMsg(PyObject* /*self*/, PyObject* args) {
    ulong i;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "kss", &i, &msg, &dst)) // k = unsigned long
        return NULL;

    MsgDistr.postMessage( new voreen::SizeTMsg(voreen::Identifier(msg), size_t(i)), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postStringMsg(PyObject* /*self*/, PyObject* args) {
    const char* str;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "sss", &str, &msg, &dst ))
        return NULL;

    MsgDistr.postMessage( new voreen::StringMsg(voreen::Identifier(msg), std::string(str)) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postVec2Msg(PyObject* /*self*/, PyObject* args) {
    float f1, f2;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ffss", &f1, &f2, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::Vec2Msg(voreen::Identifier(msg), tgt::vec2(f1, f2)), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postVec3Msg(PyObject* /*self*/, PyObject* args) {
    float f1, f2, f3;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "fffss", &f1, &f2, &f3, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::Vec3Msg(voreen::Identifier(msg), tgt::vec3(f1, f2, f3)) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postVec4Msg(PyObject* /*self*/, PyObject* args) {
    float f1, f2, f3, f4;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ffffss", &f1, &f2, &f3, &f4, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::Vec4Msg(voreen::Identifier(msg), tgt::vec4(f1, f2, f3, f4)), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postIVec2Msg(PyObject* /*self*/, PyObject* args) {
    int i1, i2;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "iiss", &i1, &i2, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::IVec2Msg(voreen::Identifier(msg), tgt::ivec2(i1, i2)));

    Py_RETURN_NONE;
}

static PyObject* voreen_postQuatMsg(PyObject* /*self*/, PyObject* args) {
    float f1, f2, f3, f4;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ffffss", &f1, &f2, &f3, &f4, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::QuatMsg(voreen::Identifier(msg), tgt::quat(f1, f2, f3, f4)), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postCreateQuatMsg(PyObject* /*self*/, PyObject* args) {
    float f1, f2, f3, f4;
    const char* msg;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ffffss", &f1, &f2, &f3, &f4, &msg, &dst))
        return NULL;

    MsgDistr.postMessage( new voreen::QuatMsg(voreen::Identifier(msg), tgt::quat::createQuat(f1, tgt::vec3(f2, f3, f4))), voreen::Identifier(dst) );

    Py_RETURN_NONE;
}

static PyObject* voreen_postKeyframeMsg(PyObject* /*self*/, PyObject* args) {
    float f1, f2, f3, f4, f5;
    const char* msg;
    const char* dst;

    if ( !PyArg_ParseTuple(args, "fffffss", &f1, &f2, &f3, &f4, &f5, &msg, &dst) )
        return NULL;

    std::vector<float> keyframe;
    keyframe.push_back(f1);
    keyframe.push_back(f2);
    keyframe.push_back(f3);
    keyframe.push_back(f4);
    keyframe.push_back(f5);

    MsgDistr.postMessage( new voreen::TemplateMessage<std::vector<float> >(voreen::Identifier(msg), keyframe), voreen::Identifier(dst));

    Py_RETURN_NONE;
}

// For blending of two transfer functions

static PyObject* voreen_loadTransferFunc1(PyObject* /*self*/, PyObject* args) {
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

//    if (!tf1)
  //      tf1 = new voreen::TransFunc();

    // FIXME
//     tf1->loadImage(std::string(name));

    Py_RETURN_NONE;
}

static PyObject* voreen_loadTransferFunc2(PyObject* /*self*/, PyObject* args) {
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

//    if (!tf2)
  //      tf2 = new voreen::TransFunc();

    // FIXME
//     tf2->loadImage(std::string(name));

    Py_RETURN_NONE;
}

static PyObject* voreen_loadIntTransferFunc1(PyObject* /*self*/, PyObject* args) {
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

  //  if (!itf1)
//        itf1 = new voreen::TransFunc();

    // FIXME
//     itf1->loadImage(std::string(name));

    Py_RETURN_NONE;
}

static PyObject* voreen_loadIntTransferFunc2(PyObject* /*self*/, PyObject* args) {
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

//    if (!itf2)
  //      itf2 = new voreen::TransFunc();

    // FIXME
//     itf2->loadImage(std::string(name));

    Py_RETURN_NONE;
}

//FIXME:  currently out of order
static PyObject* voreen_setBlendedTransferFunc(PyObject* /*self*/, PyObject* /*args*/) {
/*
    double d;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ds", &d, &dst))
        return NULL;

    if (d > 1.0)
        d = 1.0;

    if (d < 0.0)
        d = 0.0;

    if (!tf3)
        tf3 = new voreen::TransferFunc1D(4096);

    tf3->invalidateGLTex();
    tf3->getTexture()->destroy();

    tf3->getTexture()->alloc();

    for (size_t i = 0; i < size_t(tf3->getTexture()->getDimensions().x * 4); ++i) {
        double result = (1.0-d) * double(tf1->getTexture()->getPixelData()[i]) + d * double(tf2->getTexture()->getPixelData()[i]) + 0.5; // + 0.5 to round
        uint8_t value = (result > 255.0) ? 255 : uint8_t(result);
        tf3->getTexture()->getPixelData()[i] = value;
    }

    tf3->checkTexture();

    MsgDistr.postMessage( new voreen::TransferFunctionPtrMsg(voreen::Identifier::setTransferFunction, tf3), voreen::Identifier(dst) );
*/
    Py_RETURN_NONE;
}

//FIXME:  currently out of order
static PyObject* voreen_setAlphaBlendedTransferFunc(PyObject* /*self*/, PyObject* /*args*/) {
/*
    double d;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ds", &d, &dst))
        return NULL;

    if (d > 1.0)
        d = 1.0;

    if (d < 0.0)
        d = 0.0;

    if (!tf3)
        tf3 = new voreen::TransferFunc1D(4096);

    tf3->invalidateGLTex();
    tf3->getTexture()->destroy();

    tf3->getTexture()->alloc();

    // FIXME use memcpy here!!!
    for (size_t i = 0; i < size_t(tf3->getTexture()->getDimensions().x * 4); ++i)
        tf3->getTexture()->getPixelData()[i] = tf2->getTexture()->getPixelData()[i];

    for (size_t i = 0; i < size_t(tf3->getTexture()->getDimensions().x); ++i) {
        double result = (1.0-d) * static_cast<double>(tf1->getTexture()->getPixelData()[i*4+3]) + d * static_cast<double>(tf2->getTexture()->getPixelData()[i*4+3]) + 0.5;
        uint8_t value = (result > 255.0) ? 255 : static_cast<uint8_t>(result);
        tf3->getTexture()->getPixelData()[i*4+3] = value;
    }

    tf3->checkTexture();

    MsgDistr.postMessage( new voreen::TransferFunctionPtrMsg(voreen::Identifier::setTransferFunction, tf3), voreen::Identifier(dst) );
*/
    Py_RETURN_NONE;
}

//FIXME: currently out of order
static PyObject* voreen_setBlendedIntTransferFunc(PyObject* /*self*/, PyObject* /*args*/) {
/*
    double d;
    const char* dst;

    if (!PyArg_ParseTuple(args, "ds", &d, &dst))
        return NULL;

    if (d > 1.0)
        d = 1.0;

    if (d < 0.0)
        d = 0.0;

    if (!itf3)
        itf3 = new voreen::TransferFunc1D(4096);

    itf3->invalidateGLTex();
    itf3->getTexture()->destroy();

    itf3->getTexture()->alloc();

    for (size_t i = 0; i < size_t(itf3->getTexture()->getDimensions().x * 4); ++i) {
        double result = (1.0-d) * static_cast<double>(itf1->getTexture()->getPixelData()[i]) + d * static_cast<double>(itf2->getTexture()->getPixelData()[i]) + 0.5; // + 0.5 to round
        uint8_t value = (result > 255.0) ? 255 : static_cast<uint8_t>(result);
        itf3->getTexture()->getPixelData()[i] = value;
    }

    itf3->checkTexture();

    MsgDistr.postMessage( new voreen::TransferFunctionPtrMsg(voreen::Identifier::setTransferFunction2, itf3), voreen::Identifier(dst) );
*/
    Py_RETURN_NONE;
}

//------------------------------------------------------------------------------

// method table
static PyMethodDef voreen_methods[] = {
    {
        "postMsg",
        voreen_postMsg,
        METH_VARARGS,
        "post a Message to the MsgDistr"
    },
    {
        "postFloatMsg",
        voreen_postFloatMsg,
        METH_VARARGS,
        "post a FloatMsg to the MsgDistr"
    },
    {
        "postIntMsg",
        voreen_postIntMsg,
        METH_VARARGS,
        "post a IntMsg to the MsgDistr"
    },
    {
        "postSizeTMsg",
        voreen_postSizeTMsg,
        METH_VARARGS,
        "post a SizeTMsg to the MsgDistr"
    },
    {
        "postBoolMsg",
        voreen_postBoolMsg,
        METH_VARARGS,
        "post a BoolMsg to the MsgDistr"
    },
    {
        "postStringMsg",
        voreen_postStringMsg,
        METH_VARARGS,
        "post a StringMsg to the MsgDistr"
    },
    {
        "postVec2Msg",
        voreen_postVec2Msg,
        METH_VARARGS,
        "post a Vec2Msg to the MsgDistr"
    },
    {
        "postVec3Msg",
        voreen_postVec3Msg,
        METH_VARARGS,
        "post a Vec3Msg to the MsgDistr"
    },
    {
        "postVec4Msg",
        voreen_postVec4Msg,
        METH_VARARGS,
        "post a Vec4Msg to the MsgDistr"
    },
    {
        "postIVec2Msg",
        voreen_postIVec2Msg,
        METH_VARARGS,
        "post a IVec2Msg to the MsgDistr"
    },
    {
        "postQuatMsg",
        voreen_postQuatMsg,
        METH_VARARGS,
        "post a QuatMsg to the MsgDistr"
    },
    {
        "postCreateQuatMsg",
        voreen_postCreateQuatMsg,
        METH_VARARGS,
        "Post a QuatMsg to the MsgDistr. "
        "The 4 given floats create the quaternion via "
        "tgt::quat::createQuat(f1, tgt::vec3(f2, f3, f4)). "
        "Thus the first given float is the angle in radian. "
        "The other three floats define the rotation axis"
    },
    {
        "postKeyframeMsg",
        voreen_postKeyframeMsg,
        METH_VARARGS,
        "Post a camera keyframe"
    },
    {
        "loadTransferFunc1",
        voreen_loadTransferFunc1,
        METH_VARARGS,
        "loads the first transfer function which should be used for blending"
    },
    {
        "loadTransferFunc2",
        voreen_loadTransferFunc2,
        METH_VARARGS,
        "loads the second transfer function which should be used for blending"
    },
    {
        "setBlendedTransferFunc",
        voreen_setBlendedTransferFunc,
        METH_VARARGS,
        "Blends transfer function 1 & 2 with a parameter t:[0, 1] and sends it all destination views."
        "t is automatically clamped to [0, 1]."
    },
    {
        "setAlphaBlendedTransferFunc",
        voreen_setAlphaBlendedTransferFunc,
        METH_VARARGS,
        "Blends alpha of transfer function 1 & 2 with a parameter t:[0, 1] and sends it all destination views."
        "t is automatically clamped to [0, 1]."
    },
    {
        "loadIntTransferFunc1",
        voreen_loadIntTransferFunc1,
        METH_VARARGS,
        "loads the first transfer function which should be used for blending"
    },
    {
        "loadIntTransferFunc2",
        voreen_loadIntTransferFunc2,
        METH_VARARGS,
        "loads the second transfer function which should be used for blending"
    },
    {
        "setBlendedIntTransferFunc",
        voreen_setBlendedIntTransferFunc,
        METH_VARARGS,
        "Blends transfer function 1 & 2 with a parameter t:[0, 1] and sends it all destination views."
        "t is automatically clamped to [0, 1]."
    },
    { NULL, NULL, 0, NULL} // sentinal
};

//------------------------------------------------------------------------------

namespace voreen {

void initVoreenPythonModule() {
    Py_InitModule("voreen", voreen_methods);
}

} // namespace voreen

#endif // VRN_WITH_PYTHON
