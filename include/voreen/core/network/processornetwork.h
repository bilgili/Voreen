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

#ifndef VRN_PROCESSORNETWORK_H
#define VRN_PROCESSORNETWORK_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/utils/exception.h"
#include "voreen/core/network/processornetworkobserver.h"
#include "voreen/core/processors/processor.h"

#include <sstream>

namespace voreen {

class NetworkSerializer;
class Property;
class PropertyLink;
class LinkEvaluatorBase;
class Port;

/**
 * Manages a network of processors.
 */
class ProcessorNetwork : public Serializable, public Observable<ProcessorNetworkObserver> {
public:
    ProcessorNetwork();
    ~ProcessorNetwork();

    /**
     * Adds a processor to the network and assigns the given name to it.
     * The network takes ownership of the added processor and deletes it on destruction.
     *
     * @param processor the processor to be added to the network.
     *        Must not be already contained by the network.
     * @param name the name to assign to the processor. If no name is specified,
     *        the processor's class name is used.
     *
     * @note If the passed name is already assigned to a processor in the network,
     *       a unique name based on the passed one is generated. The same applies to
     *       the processor's class name, if no name is specified.
     */
    void addProcessor(Processor* processor, const std::string& name = "");

    /**
     * Removes a processor from the network and deletes it. Also
     * removes the processor's port connections and property links.
     *
     * @param processor the processor to be removed from the network.
     *        Must be contained by the network.
     */
    void removeProcessor(Processor* processor);

    /**
     * Removes all processors, port connections and property links
     * from the network and deletes them.
     */
    void clear();

    /**
     * Returns the number of processor of the network.
     */
    int numProcessors() const;

    /**
     * Returns true, if the network does not contain any processors.
     */
    bool empty() const;

    /**
     * Returns the network's processors.
     */
    const std::vector<Processor*>& getProcessors() const;

    /**
     * Returns all processors in the network matching
     * the given template type.
     */
    template<class T>
    std::vector<T*> getProcessorsByType() const;

    /**
     * Returns the processor with the passed name or null,
     * of the network does not contain a processor with that name.
     *
     * @note The name is unique among all Processor instances
     *       in the network.
     *
     * @see setProcessorName
     */
    Processor* getProcessorByName(const std::string& name) const;

    /**
     * Returns whether the passed processor is an element of the network.
     */
    bool contains(const Processor* processor) const;

    /**
     * Assigns the passed name to the passed processor.
     *
     *
     * @param processor the processor to rename. Must be part of the network.
     * @param name the name to assign. Must not be empty and must not be
     *      assigned to any prcessor in the network.
     *
     * @throw VoreenException if the passed processor is not contained by the
     *        network, or if the passed name is empty or already assigned
     *        to a processor in the network.
     *
     * @see getProcessorByName
     * @see generateUniqueProcessorName
     */
    void setProcessorName(Processor* processor, const std::string& name) const
        throw (VoreenException);

    /**
     * Creates a connection between the passed ports.
     *
     * @param inport the inport to be connected.
     *      Must be owned by a processor of the network.
     * @param outport the outport to be connected.
     *      Must be owned by a processor of the network.
     *
     * @return true, if the connection was created,
     *      false if the passed ports are not compatible
     */
    bool connectPorts(Port* inport, Port* outport);

    /**
     * Deletes the connection between the passed ports.
     *
     * @param inport the inport to be disconnected.
     *      Must be owned by a processor of the network.
     * @param outport the outport to be disconnected
     *      Must be owned by a processor of the network.
     */
    void disconnectPorts(Port* inport, Port* outport);

    /**
     * Removes all ingoing and outgoing port connections
     * from the passed processor.
     *
     * @param processor the processor to disconnect.
     *      Must be part of the network.
     */
    void disconnectPorts(Processor* processor);

    /**
     * Returns the network's property links.
     */
    const std::vector<PropertyLink*>& getPropertyLinks() const;

    /**
     * Creates a property link, if possible and no link already exists from the source to the destination,
     * and adds it to the network.
     *
     * @note EventProperties cannot be linked.
     *
     * @param src the source property of the link. Must be owned by processor of this network.
     * @param dest the destination property of the link. Must be owned by processor of this network
     *      and must not equal src.
     * @param linkEvaluator the link evaluator defining the type of the link.
     *      If not passed, an identity link is created.
     *
     * @return the created property link or null, if the link could not be created.
     */
    PropertyLink* createPropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator = 0);

    /**
     * Removes a property link from the network and deletes it.
     */
    void removePropertyLink(PropertyLink* propertyLink);

    /**
     * Removes all links from the passed property and deletes them.
     *
     * @param property the property to unlink. Must be owned by a processor
     *      that is part of the network.
     */
    void removePropertyLinks(Property* property);

    /**
     * Removes all ingoing and outgoing property links
     * from the properties of the passed processor and deletes them.
     *
     * @param processor processor to unlink. Must be an element of the network.
     */
    void removePropertyLinks(Processor* processor);

    /**
     * Removes all property links from the network and deletes them.
     */
    void clearPropertyLinks();

    /**
     * Returns true, is the passed property object is
     * the source or destination of any property link.
     */
    bool isPropertyLinked(const Property* property) const;

    /**
     * Creates links between the passed properties.
     *
     * @param properties the properties to be linked. Must be owned by processors
     *      that are part of the network.
     * @param linkEvaluator the link evaluator defining the type of the link.
     *      If not passed, an identity link is created.
     * @param replace if set to true, already existing links between properties to link
     *      are replaced. Otherwise, the existing links are kept.
     * @param transitive defines whether transitive linking is sufficient or
     *      if each pair of properties has to be linked directly. If set to true,
     *      a cyclic chain of links connecting all of the passed properties is created.
     *      Transitive linking is highly recommended, since the number of links to create is
     *      proportional to the number of properties, whereas pair-wise linking has quadratic complexity!
     *
     * @return the number of links that have been created
     */
    int linkProperties(const std::vector<Property*>& properties, LinkEvaluatorBase* linkEvaluator = 0,
        bool replace = true, bool transitive = true);

    /**
     * Creates links between a subset of processor properties that is specified
     * by the passed filter criteria.
     *
     * @tparam T Type of the properties to be linked, including subtypes.
     *      Use \c Property to embrace all types.
     * @param processors processors whose properties are to be considered.
     *      Must be part of the network. Pass an empty vector for embracing all processors in the network.
     * @param propertyIDs IDs of the properties to link.
     *      Pass an empty vector for embracing all properties.
     * @param linkEvaluator the link evaluator defining the type of the link.
     *      If not passed, an identity link is created.
     * @param replace if set to true, already existing links between properties to link
     *      are replaced. Otherwise, the existing links are kept.
     * @param transitive defines whether transitive linking is sufficient or
     *      if each pair of properties has to be linked directly. If set to true,
     *      a cyclic chain of links connecting all of the matching properties is created.
     *      Transitive linking is highly recommended, since the number of links to create is
     *      proportional to the number of properties, whereas pair-wise linking has quadratic complexity!
     *
     * @return the number of links that have been created
     */
    template<class T>
    int linkProperties(const std::vector<Processor*>& processors, const std::vector<std::string>& propertyIDs,
        LinkEvaluatorBase* linkEvaluator = 0, bool replace = true, bool transitive = true);

    /**
     * Returns all processor properties in the network
     * with the specified id.
     */
    std::vector<Property*> getPropertiesByID(const std::string& id) const;

    /**
     * Returns all processor properties in the network matching
     * the given template type.
     */
    template<class T>
    std::vector<T*> getPropertiesByType() const;

    /**
     * Set the version of the network file.
     */
    void setVersion(int version);

    /**
     * Returns the version of the network file.
     */
    int getVersion() const;

    /**
     * Generates a processor name that is unique among all processors in
     * the network by appending a number to the passed name.
     *
     * If the passed name is already unique, it is returned without
     * modifications.
     */
    std::string generateUniqueProcessorName(const std::string& name) const;

    /**
     * Convenience function: Inserts a processor into the network
     * at the connection between two other processors.
     *
     * The connection between those ports (A,B) will be terminated and will be
     * routed through processor C.<br>
     * A->B  will be A->C->B afterwards.<br>
     *
     * @param outport The outoprt of the predecessing processor
     * @param inport The inport of the successing processor
     * @param processor The new processor which is to be inserted between outport and inport
     */
    void addProcessorInConnection(Port* outport, Port* inport, Processor* processor);

    /**
     * Convenience function: Replaces a given processor in the network by another.
     *
     * If possible, connections and links from other processors to the old one are redirected
     * to the new processor. Incompatible connections are terminated.
     * If there is no compatible connection, this operation leads to a new isolated processor
     * in the network.
     *
     * @note The replaced processor is deleted after the operation.
     *
     * @param oldProc the old processor which will be replaced, must not be null,
     *  gets deleted.
     * @param newProc the new processor which should be entered, must not be null
     */
    void replaceProcessor(Processor* oldProc, Processor* newProc);

    /**
     * Returns the meta data container of this processor.
     * External objects, such as GUI widgets, can use it
     * to store and retrieve persistent meta data without
     * having to bother with the serialization themselves.
     *
     * @see MetaDataContainer
     */
    MetaDataContainer& getMetaDataContainer() const;

    /**
     * @see Serializable::serialize
     */
    void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    void deserialize(XmlDeserializer& s);

    /**
     * Serialization helper function: Returns serialization error messages.
     */
    std::vector<std::string> getErrors() const;

    /**
     * Serialization helper function: Sets serialization error messages.
     */
    void setErrors(const std::vector<std::string>& errorList);

private:
    /**
     * The @c PortConnection is responsible for serialization and deserialization
     * of connections between different processor ports.
     *
     * @see Serializable
     */
    class PortConnection : public Serializable {
    public:
        /**
         * Creates a @c PortConnection from given outport to given inport.
         *
         * @param outport the outport
         * @param inport the inport
         */
        PortConnection(Port* outport, Port* inport);

        /**
         * @see Serializable::serialize
         */
        virtual void serialize(XmlSerializer& s) const;

        /**
         * @see Serializable::deserialize
         */
        virtual void deserialize(XmlDeserializer& s);

        /**
         * Sets the outport of the connection.
         *
         * @param value the outport
         */
        void setOutport(Port* value);

        /**
         * Returns the outport of the connection.
         *
         * @return the outport
         */
        Port* getOutport() const;

        /**
         * Sets the inport of the connection.
         *
         * @param value the inport
         */
        void setInport(Port* value);

        /**
         * Returns the inport of the connection.
         *
         * @return the inport
         */
        Port* getInport() const;

    private:
        /**
         * The @c PortEntry class processes the port data
         * to get a well readable XML file.
         *
         * @see PortConnection
         * @see Serializable
         */
        class PortEntry : public Serializable {
        public:
            /**
             * Creates a @c PortEntry for given @c Port.
             *
             * @param port the port
             */
            PortEntry(Port* port);

            /**
            * @see Serializable::serialize
            */
            virtual void serialize(XmlSerializer& s) const;

            /**
            * @see Serializable::deserialize
            */
            virtual void deserialize(XmlDeserializer& s);

            /**
             * Returns the port which data will be processed.
             *
             * @return the port
             */
            Port* getPort() const;

        private:
            /**
             * Port which data will be processed.
             */
            Port* port_;
        };

        friend class XmlDeserializer;
        /**
         * Default constructor for serialization purposes.
         */
        PortConnection();

        /**
         * Outport entry of connection.
         */
        PortEntry outport_;

        /**
         * Inport entry of connection.
         */
        PortEntry inport_;
    };

    /// Calls networkChanged() on the registered observers.
    void notifyNetworkChanged() const;

    /// Calls processorAdded() on the registered observers.
    void notifyProcessorAdded(const Processor* processor) const;

    /// Calls processorRemoved() on the registered observers.
    void notifyProcessorRemoved(const Processor* processor) const;

    /// Calls processorRenamed() on the registered observers.
    void notifyProcessorRenamed(const Processor* processor, const std::string& prevName) const;

    /// Calls portConnectionAdded on the registered observers.
    void notifyPortConnectionAdded(const Port* source, const Port* destination) const;

    /// Calls portConnectionRemoved on the registered observers.
    void notifyPortConnectionRemoved(const Port* source, const Port* destination) const;

    /// Calls propertyLinkAdded() on the registered observers.
    void notifyPropertyLinkAdded(const PropertyLink* link) const;

    /// Calls propertyLinkRemoved() on the registered observers.
    void notifyPropertyLinkRemoved(const PropertyLink* link) const;

    std::vector<Processor*> processors_;
    std::vector<PropertyLink*> propertyLinks_;

    int version_;
    std::vector<std::string> errorList_;

    /**
     * Contains the associated meta data.
     *
     * We want to return a non-const reference to it from a const member function
     * and since the MetaDataContainer does not affect the processor itself,
     * mutable appears justifiable.
     */
    mutable MetaDataContainer metaDataContainer_;

    static const std::string loggerCat_; ///< category used in logging
};

//---------------------------------------------------------------------------
// template definitions

template<class T>
std::vector<T*> ProcessorNetwork::getProcessorsByType() const {
    std::vector<T*> result;
    for (size_t i = 0; i < processors_.size(); ++i) {
        T* proc = dynamic_cast<T*>(processors_[i]);
        if (proc)
            result.push_back(proc);
    }
    return result;
}

template<class T>
std::vector<T*> ProcessorNetwork::getPropertiesByType() const {
    std::vector<T*> result;
    for (size_t i = 0; i < processors_.size(); ++i) {
        std::vector<T*> processorProps = processors_[i]->getPropertiesByType<T>();
        result.insert(result.end(), processorProps.begin(), processorProps.end());
    }
    return result;
}

template<class T>
int ProcessorNetwork::linkProperties(const std::vector<Processor*>& processors,
    const std::vector<std::string>& propertyIDs,
    LinkEvaluatorBase* linkEvaluator,
    bool replace, bool transitive) {

    // determine processors to be linked
    std::vector<Processor*> linkProcessors = (processors.empty() ? getProcessors() : processors);

    // collect properties to be linked
    std::vector<Property*> linkProps;
    for (size_t i=0; i<linkProcessors.size(); ++i) {
        Processor* curProcessor = linkProcessors[i];
        if (!contains(curProcessor)) {
            LWARNING("Processor is not part of the network: " << curProcessor->getName());
            continue;
        }

        if (propertyIDs.empty()) {
            // no property ids specified => select all properties of matching type
            std::vector<T*> procProps = curProcessor->getPropertiesByType<T>();
            linkProps.insert(linkProps.end(), procProps.begin(), procProps.end());
        }
        else {
            // property ids specified => query each processor for property by id and check type
            for (size_t j=0; j<propertyIDs.size(); ++j) {
                if (dynamic_cast<T*>(curProcessor->getPropertyByID(propertyIDs[j])))
                    linkProps.push_back(curProcessor->getPropertyByID(propertyIDs[j]));
            }
        }
    }

    // passed collected properties to helper function
    return linkProperties(linkProps, linkEvaluator, replace, transitive);
}

} //namespace voreen

#endif
