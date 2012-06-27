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

#ifndef VRN_PROCESSORFACTORY_H
#define VRN_PROCESSORFACTORY_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/processors/processor.h"

#include <vector>
#include <map>
#include <string>

namespace voreen {

class Processor;

class VRN_CORE_API ProcessorFactory : public SerializableFactory {
public:
    ~ProcessorFactory();
    static ProcessorFactory* getInstance();

    /**
     * Creates an instance of the passed Processor class.
     */
    Processor* create(const std::string& className);

    /**
     * Returns a vector containing instances of the registered Processors.
     */
    const std::vector<Processor*>& getRegisteredProcessors() const;

    /**
     * Returns the mapping from Processor class name to the corresponding Processor instance.
     */
    const std::map<std::string, Processor*>& getClassMap() const;

    /**
     * Returns true, if a processor instance with the passed class name has been registered.
     */
    bool isProcessorKnown(const std::string& className) const;

    /**
     * Returns an instance of the passed Processor class.
     * If the class name is not known, null is returned.
     */
    const Processor* getProcessor(const std::string& className) const;

    /**
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const;

    /**
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString);

private:
    ProcessorFactory();
    static ProcessorFactory* instance_;

    /**
     * Retrieves the registered processors from the VoreenApplication
     * and initializes the processor list and class map.
     * Internally called on first access of the factory instance.
     */
    void initialize() const;

    /**
     * Adds the passed processor to the processor vector and the classmap.
     */
    void registerClass(Processor* const newClass) const;

    /// Contains the registered processors.
    mutable std::vector<Processor*> processors_;

    /// Maps from Processor class name to the corresponding Processor instance.
    mutable std::map<std::string, Processor*> classMap_;

    mutable bool initialized_;

    /// category used for logging
    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_PROCESSORFACTORY_H
