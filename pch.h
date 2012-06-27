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

#ifdef VRN_MODULE_PYTHON
/* include this at very first or will break with some versions of Python */ 
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
#include "tgt/assert.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include "voreen/core/processors/processor.h"

#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/eventproperty.h"

#include "voreen/core/ports/renderport.h"
#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/genericport.h"
#include "voreen/core/ports/textport.h"

#endif
