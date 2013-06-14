
################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS StagingModule)

#SET(MOD_DEFINITIONS "-DVRN_VOLUMEOCTREE_DEBUG")

SET(MOD_CORE_SOURCES
    ${MOD_DIR}/processors/alignedsliceproxygeometry.cpp
    ${MOD_DIR}/processors/arbitraryvolumeclipping.cpp
    ${MOD_DIR}/processors/interactiveregistrationwidget.cpp
    ${MOD_DIR}/processors/multislicerenderer.cpp
    ${MOD_DIR}/processors/multisliceviewer.cpp
    ${MOD_DIR}/processors/multivolumegeometryraycaster.cpp
    ${MOD_DIR}/processors/registrationinitializer.cpp
    ${MOD_DIR}/processors/samplingpositiontransformation.cpp
    ${MOD_DIR}/processors/screenspaceambientocclusion.cpp
    ${MOD_DIR}/processors/sliceproxygeometry.cpp
    ${MOD_DIR}/processors/tabbedview.cpp
    ${MOD_DIR}/processors/transfuncoverlay.cpp

    # octree
    ${MOD_DIR}/octree/datastructures/volumeoctreebase.cpp
    ${MOD_DIR}/octree/datastructures/volumeoctree.cpp
    ${MOD_DIR}/octree/datastructures/octreebrickpoolmanager.cpp
    ${MOD_DIR}/octree/datastructures/octreebrickpoolmanagerdisk.cpp
    ${MOD_DIR}/octree/datastructures/volumeoctreeport.cpp
    
    ${MOD_DIR}/octree/processors/octreecreator.cpp
    ${MOD_DIR}/octree/processors/octreeproxygeometry.cpp
    ${MOD_DIR}/octree/processors/singleoctreeraycastercpu.cpp
)

IF(VRN_MODULE_OPENCL)
    LIST(APPEND MOD_CORE_SOURCES ${MOD_DIR}/octree/processors/singleoctreeraycastercl.cpp)
ENDIF()


SET(MOD_CORE_HEADERS
    ${MOD_DIR}/processors/alignedsliceproxygeometry.h
    ${MOD_DIR}/processors/arbitraryvolumeclipping.h
    ${MOD_DIR}/processors/interactiveregistrationwidget.h
    ${MOD_DIR}/processors/multislicerenderer.h
    ${MOD_DIR}/processors/multisliceviewer.h
    ${MOD_DIR}/processors/multivolumegeometryraycaster.h
    ${MOD_DIR}/processors/registrationinitializer.h
    ${MOD_DIR}/processors/samplingpositiontransformation.h
    ${MOD_DIR}/processors/screenspaceambientocclusion.h
    ${MOD_DIR}/processors/sliceproxygeometry.h
    ${MOD_DIR}/processors/tabbedview.h
    ${MOD_DIR}/processors/transfuncoverlay.h

    # octree
    ${MOD_DIR}/octree/datastructures/volumeoctreebase.h
    ${MOD_DIR}/octree/datastructures/volumeoctree.h
    ${MOD_DIR}/octree/datastructures/octreebrickpoolmanager.h
    ${MOD_DIR}/octree/datastructures/octreebrickpoolmanagerdisk.h
    ${MOD_DIR}/octree/datastructures/volumeoctreeport.h
    ${MOD_DIR}/octree/datastructures/octreeutils.h
    
    ${MOD_DIR}/octree/processors/octreecreator.h
    ${MOD_DIR}/octree/processors/octreeproxygeometry.h
    ${MOD_DIR}/octree/processors/singleoctreeraycastercpu.h
)

IF(VRN_MODULE_OPENCL)
    LIST(APPEND MOD_CORE_HEADERS ${MOD_DIR}/octree/processors/singleoctreeraycastercl.h)
ENDIF()

IF(VRN_BUILD_TESTAPPS)
SET(MOD_CORE_APPLICATIONS
    ${MOD_DIR}/octree/test/octreetest.cpp
)
ENDIF()

# deployment
SET(MOD_INSTALL_DIRECTORIES
    ${MOD_DIR}/glsl
    ${MOD_DIR}/octree/processors/cl
    ${MOD_DIR}/octree/processors/glsl
)
   
