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
#include "processors/samplingpositiontransformation.h"
#include "processors/multislicerenderer.h"
#include "processors/multisliceviewer.h"
#include "processors/tabbedview.h"
#include "processors/transfuncoverlay.h"
#include "processors/sliceproxygeometry.h"
#include "processors/screenspaceambientocclusion.h"
#include "processors/interactiveregistrationwidget.h"
#include "processors/registrationinitializer.h"

namespace voreen {

StagingModule::StagingModule(const std::string& modulePath)
    : VoreenModule(modulePath)
{
    setID("Staging");
    setGuiName("Staging");

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

    addShaderPath(getModulePath("glsl"));
}

} // namespace
