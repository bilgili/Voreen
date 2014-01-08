
################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS StagingModule)

SET(MOD_CORE_SOURCES
    ${MOD_DIR}/processors/alignedsliceproxygeometry.cpp
    ${MOD_DIR}/processors/arbitraryvolumeclipping.cpp
    ${MOD_DIR}/processors/interactiveregistrationwidget.cpp
    ${MOD_DIR}/processors/multislicerenderer.cpp
    ${MOD_DIR}/processors/multisliceviewer.cpp
    ${MOD_DIR}/processors/multivolumegeometryraycaster.cpp
    ${MOD_DIR}/processors/pong.cpp
    ${MOD_DIR}/processors/registrationinitializer.cpp
    ${MOD_DIR}/processors/samplingpositiontransformation.cpp
    ${MOD_DIR}/processors/screenspaceambientocclusion.cpp
    ${MOD_DIR}/processors/singleoctreeraycastercpu.cpp
    ${MOD_DIR}/processors/sliceproxygeometry.cpp
    ${MOD_DIR}/processors/toucheventsimulator.cpp
    ${MOD_DIR}/processors/tabbedview.cpp
    ${MOD_DIR}/processors/transfuncoverlay.cpp
    ${MOD_DIR}/processors/volumechannelmerger.cpp
    ${MOD_DIR}/processors/volumechannelseparator.cpp
)

SET(MOD_CORE_HEADERS
    ${MOD_DIR}/processors/alignedsliceproxygeometry.h
    ${MOD_DIR}/processors/arbitraryvolumeclipping.h
    ${MOD_DIR}/processors/interactiveregistrationwidget.h
    ${MOD_DIR}/processors/multislicerenderer.h
    ${MOD_DIR}/processors/multisliceviewer.h
    ${MOD_DIR}/processors/multivolumegeometryraycaster.h   
    ${MOD_DIR}/processors/pong.h
    ${MOD_DIR}/processors/registrationinitializer.h
    ${MOD_DIR}/processors/samplingpositiontransformation.h
    ${MOD_DIR}/processors/screenspaceambientocclusion.h
    ${MOD_DIR}/processors/sliceproxygeometry.h
    ${MOD_DIR}/processors/singleoctreeraycastercpu.h
    ${MOD_DIR}/processors/tabbedview.h
    ${MOD_DIR}/processors/toucheventsimulator.h
    ${MOD_DIR}/processors/transfuncoverlay.h
    ${MOD_DIR}/processors/volumechannelmerger.h
    ${MOD_DIR}/processors/volumechannelseparator.h
)

# deployment
SET(MOD_INSTALL_DIRECTORIES
    ${MOD_DIR}/glsl
    ${MOD_DIR}/textures
)
   
