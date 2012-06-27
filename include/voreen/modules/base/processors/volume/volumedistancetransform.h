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

#ifndef VRN_VOLUMEDISTANCETRANSFORM_H
#define VRN_VOLUMEDISTANCETRANSFORM_H

#include <string>
#include "voreen/modules/base/processors/volume/volumeprocessor.h"
#include "voreen/core/properties/boolproperty.h"


namespace voreen {

class VolumeHandle;

class VolumeDistanceTransform : public VolumeProcessor {
public:
    VolumeDistanceTransform();
    virtual ~VolumeDistanceTransform();

    virtual std::string getCategory() const;
    virtual std::string getClassName() const;
    virtual Processor::CodeState getCodeState() const;
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const;

protected:
    virtual void process();
    virtual void deinitialize() throw (VoreenException);

private:
    void forceUpdate();
    void distanceTransform();

private:
    BoolProperty enableProcessingProp_;
    bool forceUpdate_;
    bool volumeOwner_;

    VolumePort inport_;
    VolumePort outport_;
};

}   //namespace

#endif // VRN_VOLUMEDISTANCETRANSFORM_H
