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

#ifndef VRN_MODULEREGISTRATION_H
#define VRN_MODULEREGISTRATION_H

#include "voreen/core/voreenapplication.h"

// standard modules
#ifdef VRN_MODULE_BASE
#include "voreen/modules/base/basemodule.h"
#endif

#ifdef VRN_MODULE_CONNECTEDCOMPONENTS
#include "voreen/modules/connectedcomponents/connectedcomponentsmodule.h"
#endif

#ifdef VRN_MODULE_DICOM
#include "voreen/modules/dicom/dicommodule.h"
#endif

#ifdef VRN_MODULE_FLOWREEN
#include "voreen/modules/flowreen/flowreenmodule.h"
#endif

#ifdef VRN_MODULE_HPMC
#include "voreen/modules/hpmc/hpmcmodule.h"
#endif

#ifdef VRN_MODULE_OPENCL
#include "voreen/modules/opencl/openclmodule.h"
#endif

#ifdef VRN_MODULE_PVM
#include "voreen/modules/pvm/pvmmodule.h"
#endif

#ifdef VRN_MODULE_PYTHON
#include "voreen/modules/python/pythonmodule.h"
#endif

#ifdef VRN_MODULE_SEGY
#include "voreen/modules/segy/segymodule.h"
#endif

#ifdef VRN_MODULE_TIFF
#include "voreen/modules/tiff/tiffmodule.h"
#endif




namespace voreen {

void addAllModules(VoreenApplication* vapp) {
    #ifdef VRN_MODULE_BASE
        vapp->addModule(new BaseModule());
    #endif

    #ifdef VRN_MODULE_CONNECTEDCOMPONENTS
        vapp->addModule(new ConnectedComponentsModule());
    #endif

    #ifdef VRN_MODULE_DICOM
        vapp->addModule(new DicomModule());
    #endif

    #ifdef VRN_MODULE_FLOWREEN
        vapp->addModule(new FlowreenModule());
    #endif

    #ifdef VRN_MODULE_HPMC
        vapp->addModule(new HPMCModule());
    #endif

    #ifdef VRN_MODULE_OPENCL
        vapp->addModule(new OpenCLModule());
    #endif

    #ifdef VRN_MODULE_PVM
        vapp->addModule(new PVMModule());
    #endif

    #ifdef VRN_MODULE_PYTHON
        vapp->addModule(new PythonModule());
    #endif

    #ifdef VRN_MODULE_SEGY
        vapp->addModule(new SEGYModule());
    #endif

    #ifdef VRN_MODULE_TIFF
        vapp->addModule(new TiffModule());
    #endif


}

} // namespace

#endif // VRN_MODULEREGISTRATION_H
