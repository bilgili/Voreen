
################################################################################
# External dependency: FFmpeg library
################################################################################

IF(WIN32)
    SET(MOD_INCLUDE_DIRECTORIES "${MOD_DIR}/ext/ffmpeg/include")

    IF(VRN_WIN32)
        SET(MOD_LIBRARIES 
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avcodec.lib
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avdevice.lib
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avformat.lib
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avutil.lib
            ${MOD_DIR}/ext/ffmpeg/lib/win32/swscale.lib
        )
        
        SET(MOD_DEBUG_DLLS
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avcodec-52.dll
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avdevice-52.dll
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avformat-52.dll
            ${MOD_DIR}/ext/ffmpeg/lib/win32/avutil-50.dll
            ${MOD_DIR}/ext/ffmpeg/lib/win32/swscale-0.dll
        )

        SET(MOD_RELEASE_DLLS ${MOD_DEBUG_DLLS})
    ELSEIF(VRN_WIN64)
        MESSAGE(FATAL_ERROR "FFmpeg module not supported on Win64")
    ENDIF()
    
    # deployment
    SET(MOD_INSTALL_FILES
        ${MOD_DIR}/ext/ffmpeg/lgpl-2.1.txt
    )

ELSEIF(UNIX)
    SET(MOD_LIBRARIES 
        -lbz2 
        -lavformat 
        -lavcodec 
        -lavutil
        -lswscale
    )
ENDIF()


################################################################################
# Core module resources 
################################################################################
SET(MOD_CORE_MODULECLASS FFmpegModule)

SET(MOD_CORE_SOURCES 
    ${MOD_DIR}/videoencoder/videoencoder.cpp
)

SET(MOD_CORE_HEADERS 
    ${MOD_DIR}/videoencoder/videoencoder.h
)
   