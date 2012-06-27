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

#ifndef VRN_FLOWMAGNITUDES3D_H
#define VRN_FLOWMAGNITUDES3D_H

#include "voreen/core/processors/processor.h"
#include "voreen/modules/flowreen/flowreenprocessor.h"

#include <string>

namespace voreen {

class VolumeHandle;

/**
 * A volume pre-processor which takes a volume containing flow data for input
 * and calculates an output volume containing only the flow magnitudes.
 * The resulting output then can be treated as any other volume and can be
 * rendered using Raycaster or SliceRenderer.
 */
class FlowMagnitudes3D : public Processor, private FlowreenProcessor {
public:
    FlowMagnitudes3D();
    virtual ~FlowMagnitudes3D();

    virtual Processor* create() const { return new FlowMagnitudes3D(); }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "FlowMagnitudes3D"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual void process();

protected:
    void initialize() throw (VoreenException);

private:
    void calculateMagnitudes();

private:
    VolumeHandle* processedVolumeHandle_;   /** The volume containing the magnitudes. */

    VolumePort volInport_;
    VolumePort volOutport_;
};

}   //namespace

#endif
