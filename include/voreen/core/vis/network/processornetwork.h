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

#ifndef VRN_PROCESSORNETWORK_H
#define VRN_PROCESSORNETWORK_H

#include "tinyxml/tinyxml.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/observer.h"
#include "voreen/core/vis/exception.h"

#include <sstream>

namespace voreen {

class PropertyLink;
class LinkEvaluatorBase;
class Port;

/**
 * Interface for network observers. Objects of this type
 * can be registered at a ProcessorNetwork.
 */
class ProcessorNetworkObserver : public Observer {

public:

    /**
     * This method is called on all modifications of the observed network
     * that do not match one of the more specialized modifications below.
     *
     * @note This function is also called by the standard implementations
     *  of the specialized notifier functions below. Therefore, if you do not need
     *  to differentiate between the single modification actions, it is sufficient
     *  to implement just this function.
     */
    virtual void networkChanged() = 0;

    /**
     * This method is called by the observed network after a processor has been added.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     * @param processor the processor that has been added
     */
    virtual void processorAdded(const Processor* processor);

    /**
     * This method is called by the observed network immediately \e before
     * a processor is removed from the network and deleted.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     * @param processor the processor that is to be removed
     */
    virtual void processorRemoved(const Processor* processor);

    /**
     * This method is called by the observed network after 
     * a processor has been renamed.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     * @param processor the processor that has been renamed
     * @param prevName the previous name of the processor
     */
    virtual void processorRenamed(const Processor* processor, const std::string& prevName);

    /**
     * This method is called by the observed network when
     * a port connection has been added or removed.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     */
    virtual void connectionsChanged();

    /**
     * This method is called by the observed network after
     * a PropertyLink has been added.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     *
     * @param link the PropertyLink that has been added
     */
    virtual void propertyLinkAdded(const PropertyLink* link);

    /**
     * This method is called by the observed network immediately \e before
     * a PropertyLink is removed from the network and deleted.
     *
     * @note The standard implementation of this function calls \a networkChanged.
     *  Override it for a custom reaction to this particular event.
     *
     * @param link the link that is to be removed
     */
    virtual void propertyLinkRemoved(const PropertyLink* link);

};

//------------------------------------------------------------------------------

/**
 * Holds all data gathered from a ".vnw" XML network file.
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
     * @note The passed name must be unique among the processors in the network.
     *       The same applies to the processor's class name, if no name is specified.
     *
     * @throw VoreenException if the desired processor name already exists in the network, 
     *        or if the processor has already been added to the network.
     */
    void addProcessor(Processor* processor, const std::string& name = "")
        throw (VoreenException);

    /**
     * Removes a processor from the network and deletes it.
     */
    void removeProcessor(Processor* processor);

    /**
     * Returns the network's processors.
     */
    const std::vector<Processor*>& getProcessors() const;

    /** 
     * Assigns the passed name to the passed processor.
     *
     * @note Processor names must be unique among the processors in a network.
     *
     * @param processor the processor to rename. Must be part of the network.
     * @param name the name to assign. Must not be empty.
     *
     * @throw VoreenException if the passed processor is not contained by the
     *        network, or if the passed name is empty or already assigned 
     *        to a processor in the network.
     *
     * @see getProcessorByName
     *
     */
    void setProcessorName(Processor* processor, const std::string& name) const
        throw (VoreenException);

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
     * Returns the number of processor of the network.
     */
    int getNumProcessors() const;

    /**
     * Creates a connection between the passed ports.
     *
     * @param inport the inport to be connected
     * @param outport the outport to be connected
     *
     * @return true, if the connection was created
     */
    bool connectPorts(Port* inport, Port* outport);

    /**
     * Deletes the connection between the passed ports.
     *
     * @param inport the inport to be disconnected
     * @param outport the outport to be disconnected
     */
    void disconnectPorts(Port* inport, Port* outport);

    /**
     * Returns whether a link is established from the passed source
     * to the passed destination property.
     */
    bool arePropertiesLinked(const Property* src, const Property* dest) const;

    /**
     * Creates a property link, if possible and no link already exists from the source to the destination,
     * and adds it to the network.
     *
     * @return the created property link or null, if the link could not be created.
     */
    PropertyLink* addPropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator = 0);

    /**
     * Removes a property link from the network.
     */
    void removePropertyLink(const PropertyLink* propertyLink);

    /**
     * Returns the network's property links.
     */
    const std::vector<PropertyLink*>& getPropertyLinks() const;

    /**
     * Set the version of the network file.
     */
    void setVersion(int version);

    /**
     * Returns the version of the network file.
     */
    int getVersion() const;

    /**
     * @see Serializable::serialize
     */
    void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    void deserialize(XmlDeserializer& s);

    /**
     * Returns all processors in the network matching
     * the given template type.
     */
    template<class PROC>
    std::vector<PROC*> getProcessorsByType() const {
        std::vector<PROC*> result;
        for (size_t i = 0; i < processors_.size(); ++i) {
            PROC* proc = dynamic_cast<PROC*>(processors_[i]);
            if (proc)
                result.push_back(proc);
        }
        return result;
    }

    void setReuseTargets(bool reuse);
    bool getReuseTargets() const;

    void setErrors(const ErrorListType& errorList);
    ErrorListType getErrors() const;
    
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
         * to get a good looking serialization result.
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
    typedef std::vector<PortConnection> PortConnectionCollection;

    /// Calls networkChanged() on the registered observers.
    void notifyNetworkChanged() const;

    /// Calls processorAdded() on the registered observers.
    void notifyProcessorAdded(const Processor* processor) const;

    /// Calls processorRemoved() on the registered observers.
    void notifyProcessorRemoved(const Processor* processor) const;

    /// Calls processorRenamed() on the registered observers.
    void notifyProcessorRenamed(const Processor* processor, const std::string& prevName) const;

    /// Calls connectionsChanged() on the registered observers.
    void notifyConnectionsChanged() const;

    /// Calls propertyLinkAdded() on the registered observers.
    void notifyPropertyLinkAdded(const PropertyLink* link) const;

    /// Calls propertyLinkRemoved() on the registered observers.
    void notifyPropertyLinkRemoved(const PropertyLink* link) const;

    std::vector<Processor*> processors_;
    std::vector<PropertyLink*> propertyLinks_;

    bool reuseTCTargets_;
    int version_;
    ErrorListType errorList_;

    static const std::string loggerCat_; ///< category used in logging

};

} //namespace voreen

#endif
