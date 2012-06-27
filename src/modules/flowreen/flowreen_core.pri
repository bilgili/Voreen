#
# This file is included by voreenlib_core.pro with "VRN_MODULES += flowreen".
#

HEADERS += $${VRN_MODULE_INC_DIR}/flowreen/flowreenmodule.h \
           $${VRN_MODULE_INC_DIR}/flowreen/colorcodingability.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flow2d.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flow3d.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flowmagnitudes3d.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flowmath.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flowreader.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/flowreenprocessor.h \
           $${VRN_MODULE_INC_DIR}/flowreen/flowslicerenderer.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/flowslicerenderer2d.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/flowslicerenderer3d.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/floworthogonalslicerenderer.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/flowstreamlinestexture3d.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/pathlinerenderer3d.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/simpletexture.h \
           $${VRN_MODULE_INC_DIR}/flowreen/streamlinerenderer3d.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/streamlinetexture.h \
           $${VRN_MODULE_INC_DIR}/flowreen/volumeflow3d.h \
           $${VRN_MODULE_INC_DIR}/flowreen/volumeoperatorflowmagnitude.h \
		   $${VRN_MODULE_INC_DIR}/flowreen/volumeoperatorintensitymask.h \

SOURCES += $${VRN_MODULE_SRC_DIR}/flowreen/flowreenmodule.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/colorcodingability.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flow2d.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flow3d.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flowmagnitudes3d.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flowmath.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flowreader.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/flowreenprocessor.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/flowslicerenderer.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/flowslicerenderer2d.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/flowslicerenderer3d.cpp \
   		   $${VRN_MODULE_SRC_DIR}/flowreen/floworthogonalslicerenderer.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/flowstreamlinestexture3d.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/pathlinerenderer3d.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/simpletexture.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/streamlinerenderer3d.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/streamlinetexture.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/volumeflow3d.cpp \
		   $${VRN_MODULE_SRC_DIR}/flowreen/volumeoperatorflowmagnitude.cpp \
           $${VRN_MODULE_SRC_DIR}/flowreen/volumeoperatorintensitymask.cpp \

### Local Variables:
### mode:conf-unix
### End: