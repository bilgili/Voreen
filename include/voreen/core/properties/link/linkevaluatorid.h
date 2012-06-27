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

#ifndef VRN_LINKEVALUATORID_H
#define VRN_LINKEVALUATORID_H

#include "voreen/core/properties/link/linkevaluatoridgeneric.h"

#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/volumehandleproperty.h"

namespace voreen {

/**
 * Propagates the unmodified source value to the destination property.
 * @deprecated
 */
class LinkEvaluatorId : public LinkEvaluatorBase {
public:
    virtual void eval(Property* src, Property* dest) throw (VoreenException);

    virtual std::string name() const;
    virtual std::string getClassName() const { return "LinkEvaluatorId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorId(); }

    bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};


//-------------------------------------------------------------------------------------------------------
//Bool, Int, Float:

class LinkEvaluatorBoolId : public LinkEvaluatorIdGeneric<bool> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorBoolId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorBoolId(); }
};

class LinkEvaluatorIntId : public LinkEvaluatorIdGeneric<int> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorIntId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorIntId(); }
};

class LinkEvaluatorFloatId : public LinkEvaluatorIdGeneric<float> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorFloatId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorFloatId(); }
};

class LinkEvaluatorFloatIntId : public LinkEvaluatorIdGenericConversion<float, int> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorFloatIntId "; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorFloatIntId (); }
};

class LinkEvaluatorFloatBoolId : public LinkEvaluatorIdGenericConversion<float, int> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorFloatBoolId "; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorFloatBoolId (); }
};

class LinkEvaluatorIntBoolId : public LinkEvaluatorIdGenericConversion<float, int> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorIntBoolId "; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorIntBoolId (); }
};

//-------------------------------------------------------------------------------------------------------
//String (+FileDialog)

class LinkEvaluatorStringId : public LinkEvaluatorIdGeneric<std::string> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorStringId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorStringId(); }
};

//-------------------------------------------------------------------------------------------------------
//Matrix properties:

class LinkEvaluatorMat2Id : public LinkEvaluatorIdGeneric<tgt::mat2> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorMat2Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorMat2Id(); }
};

class LinkEvaluatorMat3Id : public LinkEvaluatorIdGeneric<tgt::mat3> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorMat3Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorMat3Id(); }
};

class LinkEvaluatorMat4Id : public LinkEvaluatorIdGeneric<tgt::mat4> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorMat4Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorMat4Id(); }
};

//-------------------------------------------------------------------------------------------------------
// Vector properties:

class LinkEvaluatorVec2Id : public LinkEvaluatorIdGeneric<tgt::vec2> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorVec2Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorVec2Id(); }
};

class LinkEvaluatorVec3Id : public LinkEvaluatorIdGeneric<tgt::vec3> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorVec3Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorVec3Id(); }
};

class LinkEvaluatorVec4Id : public LinkEvaluatorIdGeneric<tgt::vec4> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorVec4Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorVec4Id(); }
};

class LinkEvaluatorIVec2Id : public LinkEvaluatorIdGeneric<tgt::ivec2> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorIVec2Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorIVec2Id(); }
};

class LinkEvaluatorIVec3Id : public LinkEvaluatorIdGeneric<tgt::ivec3> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorIVec3Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorIVec3Id(); }
};

class LinkEvaluatorIVec4Id : public LinkEvaluatorIdGeneric<tgt::ivec4> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorIVec4Id"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorIVec4Id(); }
};

//-------------------------------------------------------------------------------------------------------

class LinkEvaluatorShaderId : public LinkEvaluatorIdGeneric<ShaderSource> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorShaderId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorShaderId(); }
};

//-------------------------------------------------------------------------------------------------------

class LinkEvaluatorVolumeHandleId : public LinkEvaluatorIdGeneric<VolumeHandle*> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorVolumeHandleId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorVolumeHandleId(); }
};

//-------------------------------------------------------------------------------------------------------
//Plotting:

class LinkEvaluatorColorMapId : public LinkEvaluatorIdGeneric<ColorMap> {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorColorMapId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorColorMapId(); }
};

class LinkEvaluatorPlotEntitiesId : public LinkEvaluatorIdGeneric<std::vector<PlotEntitySettings> > {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorPlotEntitiesId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorPlotEntitiesId(); }
};

//-------------------------------------------------------------------------------------------------------

class LinkEvaluatorCameraId : public LinkEvaluatorBase {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorCameraId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorCameraId(); }
    virtual std::string name() const { return "id"; }

    ///Special implementation to only link position focus and up vector
    virtual void eval(Property* src, Property* dst) throw (VoreenException);

    bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};

//-------------------------------------------------------------------------------------------------------

class LinkEvaluatorTransFuncId : public LinkEvaluatorBase {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorTransFuncId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorTransFuncId(); }
    virtual std::string name() const { return "id"; }

    ///Special implementation to only link position focus and up vector
    virtual void eval(Property* src, Property* dst) throw (VoreenException);

    bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};

//-------------------------------------------------------------------------------------------------------

class LinkEvaluatorButtonId : public LinkEvaluatorBase {
public:
    virtual std::string getClassName() const { return "LinkEvaluatorButtonId"; }
    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorButtonId(); }
    virtual std::string name() const { return "id"; }

    ///Special implementation to only link position focus and up vector
    virtual void eval(Property* src, Property* dst) throw (VoreenException);

    bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};

} // namespace

#endif // VRN_LINKEVALUATORID_H
