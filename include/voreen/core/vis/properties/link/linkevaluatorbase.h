/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_LINKEVALUATOR_H
#define VRN_LINKEVALUATOR_H

/*
 * ATTENTION: Do not include voreen/core/vis/properties/property.h due to circular
 *            header-file inclusion. Keep the Property forward declaration instead.
 */
#include "voreen/core/vis/properties/link/boxobject.h"
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Property;

class LinkEvaluatorBase : public AbstractSerializable {
public:
    virtual bool isActive() { return true; }

    virtual BoxObject eval(const BoxObject& sourceOld, const BoxObject& sourceNew, const BoxObject& targetOld, Property* src, Property* dest) = 0;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer&) const {}

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer&) {}
};

} // namespace

#endif // VRN_LINKEVALUATOR_H
