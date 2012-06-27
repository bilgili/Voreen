/**
 * @file
 * 
 * Precompiled Header File for voreenlib.
 *
 * The precompiled header must contain code which is *stable* and *static*
 * throughout the project.
 *
 */

/* Add C includes here */

#ifdef VRN_WITH_PYTHON
/* include this at very first */
#include <Python.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>

#ifndef __APPLE__
    #include <GL/gl.h>
    #include <GL/glu.h>
#else
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#endif


#if defined __cplusplus
/* Add C++ includes here */

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "tgt/matrix.h"
#include "tgt/quaternion.h"
#include "tgt/shadermanager.h"
#include "tgt/texturemanager.h"
#include "tgt/types.h"
#include "tgt/vector.h"

#endif
