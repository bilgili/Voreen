/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_DEPENDENCYLINKEVALUATOR_H
#define VRN_DEPENDENCYLINKEVALUATOR_H

#include "voreen/core/properties/link/linkevaluatorbase.h"

namespace voreen {

/** 
 *  @deprecated
 */
class VRN_CORE_API DependencyLinkEvaluator : public LinkEvaluatorBase {
public:
    DependencyLinkEvaluator();

    virtual void eval(Property* src, Property* dest) throw (VoreenException);
    std::string getGuiName() const;

    virtual void serialize(XmlSerializer&) const;
    virtual void deserialize(XmlDeserializer&);

    void clearDependencyMap();

    void setHistoryLength(int newLength);
    int getHistoryLength() const;

    virtual void propertiesChanged(Property* src, Property* dst);

    virtual std::string getClassName() const;
    virtual LinkEvaluatorBase* create() const;

    virtual bool arePropertiesLinkable(const Property* /*p1*/, const Property* /*p2*/) const;

protected:
    std::string serializeProperty(Property* p) const;
    void deserializeProperty(Property* p, std::string s) const;
    void setKeyValue(std::string key, std::string value);
    void removeFrontItemFromMap();

    std::vector<std::string> keys_;
    std::vector<std::string> values_;

    std::string sourceOld_;

    int historyLength_;
};

} // namespace

#endif // VRN_DEPENDENCYLINKEVALUATOR_H
