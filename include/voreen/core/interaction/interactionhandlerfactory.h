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

#ifndef VRN_INTERACTIONHANDLER_FACTORY_H
#define VRN_INTERACTIONHANDLER_FACTORY_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

/**
 * The @c InteractionHandlerFactory class is responsible for the creation
 * of @c InteractionHandler objects during serialization process. Make
 * sure to reflect each InteractionHandler class here.
 *
 * @see SerializableFactory
 */
class InteractionHandlerFactory : public SerializableFactory {
public:
    /**
     * Returns the singleton instance of the interaction handler factory.
     *
     * @returns the instance.
     */
    static InteractionHandlerFactory* getInstance();

    /**
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const;

    /**
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString);

private:
    /**
     * Default constructor.
     */
    InteractionHandlerFactory();

    /**
     * Singleton instance of the interaction handler factory.
     */
    static InteractionHandlerFactory* instance_;
};

} // namespace voreen

#endif // VRN_INTERACTIONHANDLER_FACTORY_H
