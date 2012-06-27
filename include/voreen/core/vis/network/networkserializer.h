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

namespace voreen {

class ProcessorNetwork;

/**
 * Serializes the data structure of processors to an XML file. It can also
 * read an XML file and build the data structure again. (working)
 *
 * @see XmlProcessor
 */
class NetworkSerializer : public XmlProcessor {
public:
    /**
    * Writes the ProcessorNetwork as XML to the given file.
    */
    void writeNetworkToFile(const ProcessorNetwork* network, const std::string& filename)
        throw (SerializationException);

    /**
     * Reads a ".vnw" file and returns a ProcessorNetwork holding all Processors and metadata
     * Ensure to free the obtained pointer using C++ delete afterwards!
     */
    ProcessorNetwork* readNetworkFromFile(std::string filename)
        throw (SerializationException);

    /**
     * Returns the version of the ".vnw" file
     */
    int readVersionFromFile(std::string filename)
        throw (SerializationException);

    /**
     * @see XmlProcessor::process
     */
    virtual void process(TiXmlDocument& document);

protected:
    /**
     * Finds the Version of the saved network.
     */
    int findVersion(TiXmlNode* node);

    /**
     * Search processor network node in the subtree of the given node.
     *
     * @param node node to start search
     *
     * @return processor network node, or @c 0 if not found
     */
    TiXmlElement* findProcessorNetworkNode(TiXmlElement* node);

private:
    /**
     * Helper class ensuring freeing used resources in case an exception is thrown.
     */
    class SerializationResource {
    public:
        /**
         * Creates a @c SerializationResource.
         *
         * @param network the processor network to serialize or deserialize
         * @param serializer the serializer
         * @param filename the filename
         * @param openMode the open mode
         */
        SerializationResource(
            ProcessorNetwork* network,
            XmlSerializerBase* serializer,
            const std::string& filename,
            const std::ios_base::openmode openMode);

        /**
         * Frees used resources.
         */
        ~SerializationResource();

        /**
         * Returns the file stream of this resource.
         *
         * @return the file stream
         */
        std::fstream& getStream();

    private:
        /**
         * ProcessorNetwork to serialize or deserialize.
         */
        ProcessorNetwork* network_;

        /**
         * Serializer or deserializer.
         */
        XmlSerializerBase* serializer_;

        /**
         * File stream for serialization process.
         */
        std::fstream stream_;
    };
};

} //namespace voreen

#endif
