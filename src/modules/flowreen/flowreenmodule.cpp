/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/modules/flowreen/flowreenmodule.h"

// processors
#include "voreen/modules/flowreen/flowmagnitudes3d.h"
#include "voreen/modules/flowreen/flowslicerenderer2d.h"
#include "voreen/modules/flowreen/flowslicerenderer3d.h"
#include "voreen/modules/flowreen/flowstreamlinestexture3d.h"
#include "voreen/modules/flowreen/pathlinerenderer3d.h"
#include "voreen/modules/flowreen/streamlinerenderer3d.h"
#include "voreen/modules/flowreen/floworthogonalslicerenderer.h"

// I/O
#include "voreen/modules/flowreen/flowreader.h"

namespace voreen {

FlowreenModule::FlowreenModule()
    : VoreenModule()
{
    setName("Flowreen");

    // processors
    addProcessor(new FlowMagnitudes3D());
    addProcessor(new FlowSliceRenderer2D());
    addProcessor(new FlowSliceRenderer3D());
    addProcessor(new FlowStreamlinesTexture3D());
    addProcessor(new PathlineRenderer3D());
    addProcessor(new StreamlineRenderer3D());
    addProcessor(new FlowOrthogonalSliceRenderer());

    // I/O
    addVolumeReader(new FlowReader());
}

} // namespace
