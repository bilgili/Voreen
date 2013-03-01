/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/init.h"

#include "tgt/tgt_gl.h"

#include "tgt/assert.h"
#include "tgt/singleton.h"
#include "tgt/gpucapabilities.h"
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
    if (ShaderManager::isInited())
        ShaderManager::deinit();
    if (TextureManager::isInited())
        TextureManager::deinit();
    if (Tesselator::isInited())
        Tesselator::deinit();
}

} // namespace
