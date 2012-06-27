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

#ifndef VRN_NETWORKSERIALIZER_H
#define VRN_NETWORKSERIALIZER_H

#include "tinyxml/tinyxml.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/xml/serializable.h"
#include "voreen/core/volume/volumesetcontainer.h"

namespace voreen {

class ProcessorNetwork;

/**
 * Serializes the data structure of processors to an XML file. It can also
 * read an XML file and build the data structure again. (working)
 */
class NetworkSerializer {
public:
    /**
     * Standard Constructor
     */
    NetworkSerializer();

    ~NetworkSerializer();

    /**
    * serializes a ProcessorNetwork to xml
    */
    void serializeToXml(const ProcessorNetwork* network, const std::string& filename)
        throw (SerializerException);

    /**
     * Reads a ".vnw" file and returns a ProcessorNetwork holding all Processors and metadata
     * Ensure to free the obtained pointer using C++ delete afterwards!
     */
    ProcessorNetwork* readNetworkFromFile(std::string filename) throw (SerializerException);

    /**
     * Returns the version of the ".vnw" file
     */
    int readVersionFromFile(std::string filename);

    /**
     * Connects all the given processors which inherit from <code>tgt::EventListener</code>
     * to the given <code>tgt::EventHandler</code>. The latter is usually one
     * of the applications <code>tgt::OGLCanvas</code>es
     *
     * @param   eventHandler    pointer to the <code>tgt::EventHandler</code>
     *                          the processors shall be connected to
     * @param   processors  vector containing processors being potential listeners
     * @param   addToFront  if set to "true", the listeners are added at the front of
     *                      the handler, otherwise they are added to the back
     * @return  number of processors being connected as <code>tgt::EventListener</code>
     */
    static size_t connectEventListenersToHandler(tgt::EventHandler* const eventHandler,
        const std::vector<Processor*>& processors, const bool addToFront = false);

    /**
     * Removes all the given processors which inherit from <code>tgt::EventListener</code>
     * from the given <code>tgt::EvenHandler</code>.The latter is usually one
     * of the applications <code>tgt::OGLCanvas</code>es.
     *
     * @param   eventHandler    pointer to the <code>tgt::EventHandler</code>
     *                          the processors shall be removed from
     * @param   processors  vector containing processors being potential listeners
     * @return  number of processors which were removed
     */
    static size_t removeEventListenersFromHandler(tgt::EventHandler* const eventHandler,
        const std::vector<Processor*>& processors);

protected:
    /**
     * Finds the Version of the saved network.
     */
    int findVersion(TiXmlNode* node);
};

} //namespace voreen

#endif
