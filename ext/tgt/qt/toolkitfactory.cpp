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

#include "tgt/toolkitfactory.h"

#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtapplication.h"

namespace tgt {

GUIApplication* ToolkitFactory::createApplication(int& argc, char** argv) {
    return new QtApplication(argc, argv);
}

GLCanvas* ToolkitFactory::createCanvas( const std::string& title /*= ""*/,
                        const ivec2& size /*= tgt_DEFAULT_WINDOW_SIZE*/,
                        const GLCanvas::Buffers buffers /*= GLCanvas::RGBADD*/ )
{
    return new QtCanvas(title, size, buffers);
}


Timer* ToolkitFactory::createTimer(EventHandler* eh) {
    return new QtTimer(eh);
}

} // namespace tgt
