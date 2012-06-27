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

#ifndef VRN_VOLUMEPICKING_H
#define VRN_VOLUMEPICKING_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/datastructures/geometry/pointlistgeometry.h"

#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class VolumePicking : public RenderProcessor {
public:
    VolumePicking();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "VolumePicking";    }
    virtual std::string getCategory() const  { return "Utility";          }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual bool isUtility() const           { return true; }
    virtual std::string getProcessorInfo() const;

    virtual void invalidate(int inv = INVALID_RESULT);

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& d);

protected:
    void process();
    void initialize() throw (VoreenException);
    void deinitialize() throw (VoreenException);

private:
    tgt::vec3 getPickedPosition(tgt::ivec2 mousePosition);
    PointListGeometryVec3* getTransformedPositions() const;

    void mouseEvent(tgt::MouseEvent* e);
    void clearLast();
    void clearAll();
    void updateProperties();

    RenderPort inportFirstHit_;
    VolumePort inportVolume_;
    GeometryPort outportPicked_;

    StringOptionProperty outCoordinateSystem_;
    ButtonProperty clearLast_;
    ButtonProperty clearAll_;
    IntProperty numPicked_;
    FloatVec3Property lastPicked_;

    EventProperty<VolumePicking> pickEvent_;

    std::vector<tgt::vec3> pickedPositions_;
    PointListGeometryVec3* transformedPositions_;
    tgt::ivec2 lastMousePosition_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_VOLUMEPICKING_H
