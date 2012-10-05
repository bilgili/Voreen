
################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS StagingModule)

SET(MOD_CORE_SOURCES
    ${MOD_DIR}/processors/alignedsliceproxygeometry.cpp
    ${MOD_DIR}/processors/fhptransformation.cpp
    ${MOD_DIR}/processors/multislicerenderer.cpp
    ${MOD_DIR}/processors/multisliceviewer.cpp
    ${MOD_DIR}/processors/optimizedproxygeometry.cpp
    ${MOD_DIR}/processors/minmaxtexture.cpp
    ${MOD_DIR}/processors/interactiveregistrationwidget.cpp
    ${MOD_DIR}/processors/sliceproxygeometry.cpp
    ${MOD_DIR}/processors/tabbedview.cpp
    ${MOD_DIR}/processors/transfuncoverlay.cpp
    ${MOD_DIR}/processors/registrationinitializer.cpp
)

SET(MOD_CORE_HEADERS
    ${MOD_DIR}/processors/alignedsliceproxygeometry.h
    ${MOD_DIR}/processors/fhptransformation.h
    ${MOD_DIR}/processors/minmaxoctree.h
    ${MOD_DIR}/processors/minmaxtexture.h
    ${MOD_DIR}/processors/sliceproxygeometry.h
    ${MOD_DIR}/processors/interactiveregistrationwidget.h
    ${MOD_DIR}/processors/optimizedproxygeometry.h
    ${MOD_DIR}/processors/multislicerenderer.h
    ${MOD_DIR}/processors/multisliceviewer.h
    ${MOD_DIR}/processors/tabbedview.h
    ${MOD_DIR}/processors/transfuncoverlay.h
    ${MOD_DIR}/processors/registrationinitializer.h
)

# deployment
SET(MOD_INSTALL_DIRECTORIES
    ${MOD_DIR}/glsl
)
   
