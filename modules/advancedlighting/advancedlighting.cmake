
################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS AdvancedLightingModule)

SET(MOD_CORE_SOURCES
    # processors
    ${MOD_DIR}/processors/halfangleslicer.cpp
    ${MOD_DIR}/processors/occlusionslicer.cpp
    ${MOD_DIR}/processors/shadowraycaster.cpp
    ${MOD_DIR}/processors/shraycaster.cpp
    
    # utils
    ${MOD_DIR}/utils/shclass.cpp
    ${MOD_DIR}/utils/shcoeffcalc.cpp
    ${MOD_DIR}/utils/shcoefftrans.cpp
    ${MOD_DIR}/utils/shlightfunc.cpp
    ${MOD_DIR}/utils/shrot.cpp
)

SET(MOD_CORE_HEADERS
    # processors
    ${MOD_DIR}/processors/halfangleslicer.h
    ${MOD_DIR}/processors/occlusionslicer.h
    ${MOD_DIR}/processors/shadowraycaster.h
    ${MOD_DIR}/processors/shraycaster.h
    
    # utils
    ${MOD_DIR}/utils/shclass.h
    ${MOD_DIR}/utils/shcoeffcalc.h
    ${MOD_DIR}/utils/shcoefftrans.h
    ${MOD_DIR}/utils/shlightfunc.h
    ${MOD_DIR}/utils/shrot.h
)
