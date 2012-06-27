/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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

#ifdef TGT_USE_PYTHON
// Init python at very first
#include "tgt/scriptmanager.h"
#endif // TGT_USE_PYTHON

#include "tgt/init.h"

#include "tgt/tgt_gl.h"

#include "tgt/assert.h"
#include "tgt/singleton.h"
#include "tgt/gpucapabilities.h"
#ifdef _MSC_VER
    #include "tgt/gpucapabilitieswindows.h"
#endif
#include "tgt/modelmanager.h"
#include "tgt/tesselator.h"
#include "tgt/texturemanager.h"
#include "tgt/shadermanager.h"
#include "tgt/memorymanager.h"
#include "tgt/event/eventhandler.h"

#ifdef TGT_HAS_FCOLLADA
#include "tgt/modelreaderfcollada.h"
#endif

#ifdef TGT_HAS_DEVIL
#include "tgt/texturereaderdevil.h"
#endif
#include "tgt/texturereadertga.h"

namespace tgt {

void init(InitFeature::Features featureset) {
    if (featureset & InitFeature::SHADER_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::TEXTURE_MANAGER) {
        featureset = (InitFeature::Features) (featureset | InitFeature::GPU_PROPERTIES | InitFeature::FILE_SYSTEM);
    }

    if (featureset & InitFeature::LOG_MANAGER) {
        Singleton<LogManager>::init(new LogManager());
        ConsoleLog* log = new ConsoleLog();
        log->addCat("", true, Info);
        LogMgr.addLog(log);
        // LogMgr disposes all its logs
    }

    if (featureset & InitFeature::FILE_SYSTEM)
        Singleton<FileSystem>::init(new FileSystem());

#ifdef TGT_USE_PYTHON
    if (featureset & InitFeature::SCRIPT_MANAGER)
        Singleton<ScriptManager>::init(new ScriptManager());
#endif
}

void deinit() {
#ifdef TGT_USE_PYTHON
    if (Singleton<ScriptManager> ::isInited())
        Singleton<ScriptManager> ::deinit();
#endif

    if (Singleton<FileSystem>    ::isInited())
        Singleton<FileSystem>    ::deinit();

    if (Singleton<LogManager>    ::isInited())
        Singleton<LogManager>    ::deinit();
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
        std::cerr << "glewInit failed, error: " << glewGetErrorString(err) << std::endl;
        exit(EXIT_FAILURE);
    }
    LINFOC("tgt.init", "GLEW version:       " << glewGetString(GLEW_VERSION));

    if (featureset & InitFeature::GPU_PROPERTIES )
        Singleton<GpuCapabilities> ::init(new GpuCapabilities());
#ifdef _MSC_VER
        Singleton<GpuCapabilitiesWindows> ::init(new GpuCapabilitiesWindows());
#endif
    if (featureset & InitFeature::TESSELATOR)
        Singleton<Tesselator>    ::init(new Tesselator());
    if (featureset & InitFeature::TEXTURE_MANAGER) {
        Singleton<TextureManager>::init(new TextureManager());
        #ifdef TGT_HAS_DEVIL
            TexMgr.registerReader(new TextureReaderDevil());
        //devil has tga support so we do not need the built-in reader:
        #else
            TexMgr.registerReader(new TextureReaderTga());
        #endif
    }
    if (featureset & InitFeature::MODEL_MANAGER) {
        Singleton<ModelManager>  ::init(new ModelManager());
        #ifdef TGT_HAS_FCOLLADA
            ModelMgr.registerReader(new ModelReaderFCollada());
        #endif
    }
    
    // starting shadermanager with or without caching
    if (featureset & (InitFeature::SHADER_MANAGER | InitFeature::NO_SHADER_CACHING))
        Singleton<ShaderManager> ::init(new ShaderManager(false));
    else if (featureset & InitFeature::SHADER_MANAGER & !InitFeature::NO_SHADER_CACHING)
        Singleton<ShaderManager> ::init(new ShaderManager());
}

void deinitGL() {
    if (Singleton<GpuCapabilities> ::isInited())
        Singleton<GpuCapabilities> ::deinit();
#ifdef _MSC_VER
    if (Singleton<GpuCapabilitiesWindows> ::isInited())
        Singleton<GpuCapabilitiesWindows> ::deinit();
#endif
    if (Singleton<ShaderManager> ::isInited())
        Singleton<ShaderManager> ::deinit();
    if (Singleton<ModelManager>  ::isInited())
        Singleton<ModelManager>  ::deinit();
    if (Singleton<TextureManager>::isInited())
        Singleton<TextureManager>::deinit();
    if (Singleton<Tesselator>    ::isInited())
        Singleton<Tesselator>    ::deinit();
}

} // namespace
