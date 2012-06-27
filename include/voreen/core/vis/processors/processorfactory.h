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

#ifndef VRN_PROCESSORFACTORY_H
#define VRN_PROCESSORFACTORY_H

#include <vector>
#include <map>
#include <string>

namespace voreen {

class Processor;
class Identifier;

class ProcessorFactory {
public:

    Processor* create(Identifier name);

    const std::vector<Identifier>& getKnownClasses();

    static ProcessorFactory* getInstance();

    // Returns processor information
    std::string getProcessorInfo(Identifier);

    /**
     * Destroys the instance of this Singleton.
     */
    static void destroy();

    /**
     * intializes the ClassList by registering processors
     * adding new processors will happen here
     */
    void initializeClassList();

private:
    static ProcessorFactory* instance_;

    ProcessorFactory();
    ~ProcessorFactory();

    void registerClass(Processor* newClass);

    std::map<Identifier, Processor*> classList_;
    std::vector<Identifier> knownClasses_;
};

} // namespace voreen

#endif //VRN_PROCESSORFACTORY_H
