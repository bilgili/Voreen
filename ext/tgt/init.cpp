/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "tgt/init.h"

#include "tgt/tgt_gl.h"

#include "tgt/assert.h"
#include "tgt/singleton.h"
#include "tgt/gpucapabilities.h"
#ifdef _MSC_VER
    #include "tgt/gpucapabilitieswindows.h"
#endif
#include "tgt/tesselator.h"
#include "tgt/texturemanager.h"
#include "tgt/shadermanager.h"
#include "tgt/event/eventhandler.h"

#ifdef TGT_HAS_DEVIL
#include "tgt/texturereaderdevil.h"
#endif
#include "tgt/texturereadertga.h"

namespace tgt {

void init(InitFeature::Features featureset, LogLevel logLevel) {
    if (featureset & InitFeature::SHADER_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::TEXTURE_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::LOG_MANAGER) {
        LogManager::init();
        if (featureset & InitFeature::LOG_TO_CONSOLE) {
            ConsoleLog* log = new ConsoleLog();
            log->addCat("", true, logLevel);
            LogMgr.addLog(log);
        }
        // LogMgr disposes all its logs
    }

    if (featureset & InitFeature::FILE_SYSTEM)
        FileSystem::init();
}

void deinit() {
    if (FileSystem::isInited())
        FileSystem::deinit();

    if (LogManager::isInited())
        LogManager::deinit();
}

void initGL(InitFeature::Features featureset) {
    if (featureset & InitFeature::SHADER_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }
    if (featureset & InitFeature::TEXTURE_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }


    GLenum err = glewInit();
    if (err != GLEW_OK) {
        // Problem: glewInit failed, something is seriously wrong.
        tgtAssert(false, "glewInit failed");
        std::cerr << "glewInit failed, error: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
    LINFOC("tgt.init", "GLEW version:       " << glewGetString(GLEW_VERSION));

    if (featureset & InitFeature::GPU_PROPERTIES )
        GpuCapabilities::init();
#ifdef _MSC_VER
        GpuCapabilitiesWindows::init();
#endif
    if (featureset & InitFeature::TESSELATOR)
        Tesselator::init();
    if (featureset & InitFeature::TEXTURE_MANAGER) {
        TextureManager::init();
        #ifdef TGT_HAS_DEVIL
            TexMgr.registerReader(new TextureReaderDevil());
        //devil has tga support so we do not need the built-in reader:
        #else
            TexMgr.registerReader(new TextureReaderTga());
        #endif
    }

    // starting shadermanager
    ShaderManager::init();
}

void deinitGL() {
    if (GpuCapabilities::isInited())
        GpuCapabilities::deinit();
#ifdef _MSC_VER
    if (GpuCapabilitiesWindows::isInited())
        GpuCapabilitiesWindows::deinit();
#endif
    if (ShaderManager::isInited())
        ShaderManager::deinit();
    if (TextureManager::isInited())
        TextureManager::deinit();
    if (Tesselator::isInited())
        Tesselator::deinit();
}

} // namespace
