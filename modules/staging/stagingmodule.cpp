/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "stagingmodule.h"

#include "processors/alignedsliceproxygeometry.h"
#include "processors/arbitraryvolumeclipping.h"
#include "processors/interactiveregistrationwidget.h"
#include "processors/multislicerenderer.h"
#include "processors/multisliceviewer.h"
#include "processors/multivolumegeometryraycaster.h"
#include "processors/registrationinitializer.h"
#include "processors/samplingpositiontransformation.h"
#include "processors/screenspaceambientocclusion.h"
#include "processors/sliceproxygeometry.h"
#include "processors/tabbedview.h"
#include "processors/transfuncoverlay.h"

// octree datastructures
#include "octree/datastructures/volumeoctree.h"
#include "octree/datastructures/octreebrickpoolmanager.h"
#include "octree/datastructures/octreebrickpoolmanagerdisk.h"

// octree processors
#include "octree/processors/octreecreator.h"
#include "octree/processors/octreeproxygeometry.h"
#include "octree/processors/singleoctreeraycastercpu.h"
#ifdef VRN_MODULE_OPENCL
    #include "octree/processors/singleoctreeraycastercl.h"
#endif

namespace voreen {

StagingModule::StagingModule(const std::string& modulePath)
    : VoreenModule(modulePath)
{
    setID("Staging");
    setGuiName("Staging");

    addShaderPath(getModulePath("glsl"));
    addShaderPath(getModulePath("octree/processors/glsl"));

    registerSerializableType(new AlignedSliceProxyGeometry());
    registerSerializableType(new ArbitraryVolumeClipping());
    registerSerializableType(new SamplingPositionTransformation());
    registerSerializableType(new InteractiveRegistrationWidget());
    registerSerializableType(new MultiSliceRenderer());
    registerSerializableType(new MultiSliceViewer());
    registerSerializableType(new SliceProxyGeometry());
    registerSerializableType(new ScreenSpaceAmbientOcclusion());
    registerSerializableType(new TabbedView());
    registerSerializableType(new TransFuncOverlay());
    registerSerializableType(new RegistrationInitializer());
#ifdef GL_ATOMIC_COUNTER_BUFFER //disable compilation for old gl headers
    registerSerializableType(new MultiVolumeGeometryRaycaster());
#endif

    // octree datastructures
    registerSerializableType(new VolumeOctree());
    registerSerializableType(new OctreeBrickPoolManagerRAM());
    registerSerializableType(new OctreeBrickPoolManagerDiskLimitedRam(64, 512, false, ""));

    // octree processors
    registerSerializableType(new OctreeProxyGeometry());
    registerSerializableType(new OctreeCreator());
    registerSerializableType(new SingleOctreeRaycasterCPU());
#ifdef VRN_MODULE_OPENCL
    registerSerializableType(new SingleOctreeRaycasterCL());
#endif

}

} // namespace
