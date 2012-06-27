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

#include "voreen/core/properties/link/linkevaluatoridnormalized.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/vectorproperty.h"

#include "voreen/core/properties/link/boxobjecthelper.h"

namespace voreen {

void LinkEvaluatorIdNormalized::eval(Property* src, Property* dst) throw (VoreenException) {
    BoxObject targetNew;
    BoxObject sourceNew = BoxObjectHelper::createBoxObjectFromProperty(src);
    BoxObject targetOld = BoxObjectHelper::createBoxObjectFromProperty(dst);
    switch (sourceNew.getType()) {
    case BoxObject::BOOL:
        {
            switch (targetOld.getType())
            {
                case BoxObject::BOOL:
                    {
                        targetNew = BoxObject(sourceNew.getBool());
                    }
                    break;
                case BoxObject::INTEGER:
                    {
                        IntProperty* d = ((IntProperty*)dst);
                        targetNew = BoxObject((sourceNew.getBool() ? d->getMaxValue() : d->getMinValue()));
                    }
                    break;
                case BoxObject::FLOAT:
                    {
                        FloatProperty* d = ((FloatProperty*)dst);
                        targetNew = BoxObject((sourceNew.getBool() ? d->getMaxValue() : d->getMinValue()));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type.");
            }
        }
        break;

    case BoxObject::INTEGER:
        {
            IntProperty* s = ((IntProperty*)src);
            int diff_s = s->getMaxValue() - s->getMinValue();
            switch (targetOld.getType())
            {
                case BoxObject::BOOL:
                    {
                        targetNew = BoxObject((s->getMinValue() + diff_s / 2 >= s->get()));
                    }
                    break;
                case BoxObject::INTEGER:
                    {
                        IntProperty* d = ((IntProperty*)dst);
                        int diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject((int)(d->getMinValue() + (s->get() - s->getMinValue()) * (float)diff_d / (float)diff_s));
                    }
                    break;
                case BoxObject::FLOAT:
                    {
                        FloatProperty* d = ((FloatProperty*)dst);
                        float diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject((float)(d->getMinValue() + (s->get() - s->getMinValue()) * (float)diff_d / (float)diff_s));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::FLOAT:
        {
            FloatProperty* s = ((FloatProperty*)src);
            float diff_s = s->getMaxValue() - s->getMinValue();
            switch (targetOld.getType())
            {
                case BoxObject::BOOL:
                    {
                        targetNew = BoxObject((s->getMinValue() + diff_s / 2 >= s->get()));
                    }
                    break;
                case BoxObject::INTEGER:
                    {
                        IntProperty* d = ((IntProperty*)dst);
                        int diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject((int)(d->getMinValue() + (s->get() - s->getMinValue()) * (float)diff_d / diff_s));
                    }
                    break;
                case BoxObject::FLOAT:
                    {
                        FloatProperty* d = ((FloatProperty*)dst);
                        float diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject((float)(d->getMinValue() + (s->get() - s->getMinValue()) * diff_d / diff_s));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::IVEC2:
        {
            IntVec2Property* s = ((IntVec2Property*)src);
            tgt::ivec2 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::IVEC3:
        {
            IntVec3Property* s = ((IntVec3Property*)src);
            tgt::ivec3 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::IVEC3:
                    {
                        IntVec3Property* d = ((IntVec3Property*)dst);
                        tgt::ivec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec3(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / (float)diff_s.z)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y
                        ));
                    }
                    break;
                case BoxObject::VEC3:
                    {
                        FloatVec3Property* d = ((FloatVec3Property*)dst);
                        tgt::vec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec3(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / (float)diff_s.z
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::IVEC4:
        {
            IntVec4Property* s = ((IntVec4Property*)src);
            tgt::ivec4 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::IVEC3:
                    {
                        IntVec3Property* d = ((IntVec3Property*)dst);
                        tgt::ivec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec3(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / (float)diff_s.z)
                        ));
                    }
                    break;
                case BoxObject::IVEC4:
                    {
                        IntVec4Property* d = ((IntVec4Property*)dst);
                        tgt::ivec4 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec4(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / (float)diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / (float)diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / (float)diff_s.z),
                            (int)(d->getMinValue().w + (s->get().w - s->getMinValue().w) * (float)diff_d.w / (float)diff_s.w)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y
                        ));
                    }
                    break;
                case BoxObject::VEC3:
                    {
                        FloatVec3Property* d = ((FloatVec3Property*)dst);
                        tgt::vec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec3(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / (float)diff_s.z
                        ));
                    }
                    break;
                case BoxObject::VEC4:
                    {
                        FloatVec4Property* d = ((FloatVec4Property*)dst);
                        tgt::vec4 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec4(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / (float)diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / (float)diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / (float)diff_s.z,
                            d->getMinValue().w + (s->get().w - s->getMinValue().w) * diff_d.w / (float)diff_s.w
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::VEC2:
        {
            FloatVec2Property* s = ((FloatVec2Property*)src);
            tgt::vec2 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::VEC3:
        {
            FloatVec3Property* s = ((FloatVec3Property*)src);
            tgt::vec3 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::IVEC3:
                    {
                        IntVec3Property* d = ((IntVec3Property*)dst);
                        tgt::ivec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec3(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / diff_s.z)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y
                        ));
                    }
                    break;
                case BoxObject::VEC3:
                    {
                        FloatVec3Property* d = ((FloatVec3Property*)dst);
                        tgt::vec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec3(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / diff_s.z
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    case BoxObject::VEC4:
        {
            FloatVec4Property* s = ((FloatVec4Property*)src);
            tgt::vec4 diff_s = s->getMaxValue() - s->getMinValue();
            switch(targetOld.getType())
            {
                case BoxObject::IVEC2:
                    {
                        IntVec2Property* d = ((IntVec2Property*)dst);
                        tgt::ivec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec2(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y)
                        ));
                    }
                    break;
                case BoxObject::IVEC3:
                    {
                        IntVec3Property* d = ((IntVec3Property*)dst);
                        tgt::ivec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec3(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / diff_s.z)
                        ));
                    }
                    break;
                case BoxObject::IVEC4:
                    {
                        IntVec4Property* d = ((IntVec4Property*)dst);
                        tgt::ivec4 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::ivec4(
                            (int)(d->getMinValue().x + (s->get().x - s->getMinValue().x) * (float)diff_d.x / diff_s.x),
                            (int)(d->getMinValue().y + (s->get().y - s->getMinValue().y) * (float)diff_d.y / diff_s.y),
                            (int)(d->getMinValue().z + (s->get().z - s->getMinValue().z) * (float)diff_d.z / diff_s.z),
                            (int)(d->getMinValue().w + (s->get().w - s->getMinValue().w) * (float)diff_d.w / diff_s.w)
                        ));
                    }
                    break;
                case BoxObject::VEC2:
                    {
                        FloatVec2Property* d = ((FloatVec2Property*)dst);
                        tgt::vec2 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec2(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y
                        ));
                    }
                    break;
                case BoxObject::VEC3:
                    {
                        FloatVec3Property* d = ((FloatVec3Property*)dst);
                        tgt::vec3 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec3(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / diff_s.z
                        ));
                    }
                    break;
                case BoxObject::VEC4:
                    {
                        FloatVec4Property* d = ((FloatVec4Property*)dst);
                        tgt::vec4 diff_d = d->getMaxValue() - d->getMinValue();
                        targetNew = BoxObject(tgt::vec4(
                            d->getMinValue().x + (s->get().x - s->getMinValue().x) * diff_d.x / diff_s.x,
                            d->getMinValue().y + (s->get().y - s->getMinValue().y) * diff_d.y / diff_s.y,
                            d->getMinValue().z + (s->get().z - s->getMinValue().z) * diff_d.z / diff_s.z,
                            d->getMinValue().w + (s->get().w - s->getMinValue().w) * diff_d.w / diff_s.w
                        ));
                    }
                    break;
                default:
                    throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
            }
        }
        break;

    default:
        throw VoreenException("LinkEvaluatorIdNormalized: Unsupported property type");
    }
    BoxObjectHelper::setPropertyFromBoxObject(dst, targetNew);
}

std::string LinkEvaluatorIdNormalized::name() const {
    return "idNormalized";
}

bool LinkEvaluatorIdNormalized::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    if( (dynamic_cast<const BoolProperty*>(p1)
    ||   dynamic_cast<const FloatProperty*>(p1)
    ||   dynamic_cast<const IntProperty*>(p1) ) &&
        (dynamic_cast<const BoolProperty*>(p2)
    ||   dynamic_cast<const FloatProperty*>(p2)
    ||   dynamic_cast<const IntProperty*>(p2) ) )
        return true;

    if( (dynamic_cast<const FloatVec2Property*>(p1)
    ||   dynamic_cast<const IntVec2Property*>(p1) ) &&
        (dynamic_cast<const FloatVec2Property*>(p2)
    ||   dynamic_cast<const IntVec2Property*>(p2) ) )
        return true;

    if( (dynamic_cast<const FloatVec3Property*>(p1)
    ||   dynamic_cast<const IntVec3Property*>(p1) ) &&
        (dynamic_cast<const FloatVec2Property*>(p2)
    ||   dynamic_cast<const IntVec2Property*>(p2)
    ||   dynamic_cast<const IntVec3Property*>(p2)
    ||   dynamic_cast<const FloatVec3Property*>(p2) ) )
        return true;

    if( (dynamic_cast<const FloatVec4Property*>(p1)
    ||   dynamic_cast<const IntVec4Property*>(p1) ) &&
        (dynamic_cast<const FloatVec2Property*>(p2)
    ||   dynamic_cast<const IntVec2Property*>(p2)
    ||   dynamic_cast<const IntVec3Property*>(p2)
    ||   dynamic_cast<const FloatVec3Property*>(p2)
    ||   dynamic_cast<const IntVec4Property*>(p2)
    ||   dynamic_cast<const FloatVec4Property*>(p2) ) )
        return true;

    return false;
}

} // namespace
