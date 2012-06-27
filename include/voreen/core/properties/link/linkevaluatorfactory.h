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

#ifndef VRN_LINKEVALUATORFACTORY_H
#define VRN_LINKEVALUATORFACTORY_H

#include "voreen/core/io/serialization/serialization.h"

#include "voreen/core/properties/link/linkevaluatorbase.h"

#include <map>
#include <string>

namespace voreen {

class LinkEvaluatorFactory : public SerializableFactory {
public:
    ///Gets the singleton instance of the factory
    static LinkEvaluatorFactory* getInstance();

    /*
     * Returns a LinkEvaluator instance identified by the
     * provided function name or throws an Exception
     *
     * @deprecated Use createType instead!
     */
    //LinkEvaluatorBase* createLinkEvaluator(std::string functionName);

    ///Returns the complete list of functions registered with the factory.
    std::vector<std::string> listFunctionNames();

    ///@see SerializableFactory::getTypeString
    virtual const std::string getTypeString(const std::type_info& type) const;

    ///@see SerializableFactory::createType
    virtual Serializable* createType(const std::string& typeString);

    virtual LinkEvaluatorBase* create(const std::string& typeString);

    /**
     * Checks if the properties p1 and p2 are linkable.
     * A property is linkable if at least one registered LinkEvaluator can link these properties.
     *
     * \return true, if p1 and p2 are compatible, false otherwise
     */
    bool arePropertiesLinkable(const Property* p1, const Property* p2, bool bidirectional = false) const;

    /**
     * @brief Get all linkevaluators that can link p1 to p2.
     *
     * @return Vector of compatible linkevaluators, in the form of <Classname, name> pairs.
     */
    std::vector<std::pair<std::string, std::string> > getCompatibleLinkEvaluators(const Property* p1, const Property* p2) const;

    void registerClass(LinkEvaluatorBase* const newClass);

    /// Returns true, if a linkevaluator instance with the passed class name has been registered
    bool isLinkEvaluatorKnown(const std::string& className) const;
private:
    LinkEvaluatorFactory();
    static LinkEvaluatorFactory* instance_;

    std::map<std::string, LinkEvaluatorBase*> classList_;
    std::vector<std::string> knownClasses_;
};

} // namespace

#endif // VRN_LINKEVALUATORFACTORY_H
