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
#include "voreen/core/xml/serializable.h"
#include "voreen/core/volume/volumesetcontainer.h"

namespace voreen {

/**
 * Interface for network observers. Objects of this type
 * can be registered at a ProcessorNetwork.
 */
class ProcessorNetworkObserver {

public:

    virtual ~ProcessorNetworkObserver() {}
    /**
     * This method is called on all modifications of the observed network 
     * that do not match one of the more specialized modifications below.
     */
    virtual void networkChanged() = 0;

    /** 
     * This method is called by the observed network when a processor has been added.
     *
     * @param processor the processor that has been added
     */
    virtual void processorAdded(Processor* processor) = 0;

    /** 
     * This method is called by the observed network when a processor has been removed.
     *
     * @param processor the processor that has been removed
     */
    virtual void processorRemoved(Processor* processor) = 0;

};

//------------------------------------------------------------------------------

/**
 * Holds all data gathered from a ".vnw" XML network file.
 *
 */
class ProcessorNetwork : public Serializable {
public:
    ProcessorNetwork();
    ProcessorNetwork(const ProcessorNetwork& othernet);
    ~ProcessorNetwork();

    void operator=(const ProcessorNetwork& othernet);

    /**
     * Returns the name of the xml element used when serializing the object
     */
    virtual std::string getXmlElementName() const { return "ProcessorNetwork"; }

    /**
     * Serializes the object to XML.
     */
    virtual TiXmlElement* serializeToXml() const;

    /**
     * Updates the object from XML.
     */
    virtual void updateFromXml(TiXmlElement* elem);
    virtual void updateMetaFromXml(TiXmlElement* elem);

    /**
     * Adds processor to the network.
     */
    virtual void addProcessor(Processor* processor);

    /**
     * Removes a processor from the network.
     */
    virtual void removeProcessor(Processor* processor);

    /**
     * Returns the network's processors.
     */
    virtual std::vector<Processor*>& getProcessors();

    /**
     * Sets the networks processors.
     */
    virtual void setProcessors(std::vector<Processor*>& processors);
    
    /**
     * Returns the number of processor of the network.
     */
    int getNumProcessors() const;

    /** 
     * Set the version of the network file.
     */
    virtual void setVersion(int version);

    /** 
     * 
     * Returns the version of the network file.
     */
    virtual int getVersion() const;

    void addObserver(ProcessorNetworkObserver* observer);

    void removeObserver(ProcessorNetworkObserver* observer);

    void setReuseTargets(bool reuse);

    bool getReuseTargets() const;

    void addToMeta(TiXmlElement* elem) { meta_.addData(elem); }
    void removeFromMeta(std::string elemName) { meta_.removeData(elemName); }
    void clearMeta() { meta_.clearData(); }
    TiXmlElement* getFromMeta(std::string elemName) const { return meta_.getData(elemName); }
    bool hasInMeta(std::string elemName) const { return meta_.hasData(elemName); }

  

protected:

    /**
     * Calls the networkChanged() method on all registered observers.
     */
    virtual void notifyObservers();

    static const std::string loggerCat_; ///< category used in logging

private:

    void initializeFrom(const ProcessorNetwork& othernet);

    std::vector<ProcessorNetworkObserver*> observers_;

    std::vector<Processor*> processors_;
    bool reuseTCTargets_;
    int version_;

    MetaSerializer meta_; // Can store metadata
};

} //namespace voreen

#endif
