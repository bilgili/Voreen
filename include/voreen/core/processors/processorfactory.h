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

#ifndef VRN_PROCESSORFACTORY_H
#define VRN_PROCESSORFACTORY_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/processors/processor.h"

#include <vector>
#include <map>
#include <string>

namespace voreen {

class Processor;

class ProcessorFactory : public SerializableFactory {
public:
    typedef std::pair<std::string, std::string> StringPair;
    typedef std::vector<StringPair> KnownClassesVector;

    ~ProcessorFactory();

    Processor* create(const std::string& name);
    const ProcessorFactory::KnownClassesVector& getKnownClasses() const { return knownClasses_; }

    static ProcessorFactory* getInstance();

    /// Returns true, if a processor instance with the passed class name has been registered
    bool isProcessorKnown(const std::string& className) const;

    /// Returns processor information
    std::string getProcessorInfo(const std::string& name) const;

    /// Returns processor category
    std::string getProcessorCategory(const std::string& name) const;

    /// Returns processor modulename
    std::string getProcessorModuleName(const std::string& name) const;

    /// Returns processor codestate
    Processor::CodeState getProcessorCodeState(const std::string& name) const;

    /**
     * Destroys the instance of this Singleton.
     */
    static void destroy();

    /**
     * intializes the ClassList by registering processors
     * adding new processors will happen here.
     */
    void initializeClassList();

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
    void registerClass(Processor* const newClass, bool isCore = false);

    static ProcessorFactory* instance_;

    std::map<std::string, Processor*> classList_;
    ProcessorFactory::KnownClassesVector knownClasses_;

    /// category used for logging
    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_PROCESSORFACTORY_H
