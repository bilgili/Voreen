
SET(MOD_CORE_MODULECLASS FlowreenModule)

SET(MOD_CORE_SOURCES
    ${MOD_DIR}/datastructures/flow2d.cpp
    ${MOD_DIR}/datastructures/flow3d.cpp
    ${MOD_DIR}/datastructures/simpletexture.cpp
    ${MOD_DIR}/datastructures/streamlinetexture.cpp
    ${MOD_DIR}/datastructures/volumeflow3d.cpp
    ${MOD_DIR}/datastructures/volumeoperatorflowmagnitude.cpp
    ${MOD_DIR}/io/flowreader.cpp
    ${MOD_DIR}/processors/flowmagnitudes3d.cpp
    ${MOD_DIR}/processors/floworthogonalslicerenderer.cpp
    ${MOD_DIR}/processors/flowreenadapter.cpp
    ${MOD_DIR}/processors/flowreenprocessor.cpp
    ${MOD_DIR}/processors/flowslicerenderer.cpp
    ${MOD_DIR}/processors/flowslicerenderer2d.cpp
    ${MOD_DIR}/processors/flowslicerenderer3d.cpp
    ${MOD_DIR}/processors/flowstreamlinestexture3d.cpp
    ${MOD_DIR}/processors/pathlinerenderer3d.cpp
    ${MOD_DIR}/processors/streamlinerenderer3d.cpp
    ${MOD_DIR}/utils/colorcodingability.cpp
    ${MOD_DIR}/utils/flowmath.cpp
)

SET(MOD_CORE_HEADERS
    ${MOD_DIR}/datastructures/flow2d.h
    ${MOD_DIR}/datastructures/flow3d.h
    ${MOD_DIR}/datastructures/simpletexture.h
    ${MOD_DIR}/datastructures/streamlinetexture.h
    ${MOD_DIR}/datastructures/volumeflow3d.h
    ${MOD_DIR}/datastructures/volumeoperatorflowmagnitude.h
    ${MOD_DIR}/datastructures/volumeoperatorintensitymask.h
    ${MOD_DIR}/flowreenmodule.h
    ${MOD_DIR}/io/flowreader.h
    ${MOD_DIR}/processors/flowmagnitudes3d.h
    ${MOD_DIR}/processors/floworthogonalslicerenderer.h
    ${MOD_DIR}/processors/flowreenadapter.h
    ${MOD_DIR}/processors/flowreenprocessor.h
    ${MOD_DIR}/processors/flowslicerenderer.h
    ${MOD_DIR}/processors/flowslicerenderer2d.h
    ${MOD_DIR}/processors/flowslicerenderer3d.h
    ${MOD_DIR}/processors/flowstreamlinestexture3d.h
    ${MOD_DIR}/processors/pathlinerenderer3d.h
    ${MOD_DIR}/processors/streamlinerenderer3d.h
    ${MOD_DIR}/utils/colorcodingability.h
    ${MOD_DIR}/utils/flowmath.h
)
 
SET(MOD_INSTALL_DIRECTORIES
    ${MOD_DIR}/glsl
    ${MOD_DIR}/data
    ${MOD_DIR}/transferfuncs
)