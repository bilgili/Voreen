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

#ifndef VRN_LINKEVALUATORIDGENERIC_H
#define VRN_LINKEVALUATORIDGENERIC_H

#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/plotting/colormap.h"
#include "voreen/core/plotting/plotentitysettings.h"

namespace voreen {

template<class T>
class LinkEvaluatorIdGeneric : public LinkEvaluatorBase {
public:
    virtual void eval(Property* src, Property* dst) throw (VoreenException);

    virtual std::string name() const { return "id"; }

    virtual bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};

template<class T>
void LinkEvaluatorIdGeneric<T>::eval(Property* src, Property* dst) throw (VoreenException) {
    static_cast<TemplateProperty<T>*>(dst)->set(static_cast<TemplateProperty<T>*>(src)->get());
}

template<class T>
bool LinkEvaluatorIdGeneric<T>::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return (dynamic_cast<const TemplateProperty<T>*>(p1) && dynamic_cast<const TemplateProperty<T>*>(p2));
}

template<class T, class R>
class LinkEvaluatorIdGenericConversion : public LinkEvaluatorBase {
public:
    virtual void eval(Property* src, Property* dst) throw (VoreenException);

    virtual std::string name() const { return "id"; }

    virtual bool arePropertiesLinkable(const Property* p1, const Property* p2) const;
};

template<class T, class R>
void LinkEvaluatorIdGenericConversion<T, R>::eval(Property* src, Property* dst) throw (VoreenException) {
    //Find out direction:
    TemplateProperty<R>* srcR = dynamic_cast<TemplateProperty<R>*>(src);
    TemplateProperty<T>* dstT = dynamic_cast<TemplateProperty<T>*>(dst);

    TemplateProperty<T>* srcT = dynamic_cast<TemplateProperty<T>*>(src);
    TemplateProperty<R>* dstR = dynamic_cast<TemplateProperty<R>*>(dst);

    if(srcR && dstT) {
        dstT->set(static_cast<T>(srcR->get()));
    }
    else if(srcT && dstR) {
        dstR->set(static_cast<R>(srcT->get()));
    }
    else {
        tgtAssert(false, "Should not get here!");
    }
}

template<class T, class R>
bool LinkEvaluatorIdGenericConversion<T, R>::arePropertiesLinkable(const Property* p1, const Property* p2) const {
    tgtAssert(p1, "null pointer");
    tgtAssert(p2, "null pointer");

    return ( (dynamic_cast<const TemplateProperty<T>*>(p1) && dynamic_cast<const TemplateProperty<R>*>(p2))
          || (dynamic_cast<const TemplateProperty<R>*>(p1) && dynamic_cast<const TemplateProperty<T>*>(p2)) );
}

} // namespace

#endif
