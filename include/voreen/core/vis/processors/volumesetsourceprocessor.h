/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_VOLUMESETSOURCEPROCESSOR_H
#define VRN_VOLUMESETSOURCEPROCESSOR_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/volume/volumesetcontainer.h"
#include "voreen/core/volume/observer.h"

namespace voreen {

class VolumeSetSourceProcessor : public Processor, public Observer {
public: 
    VolumeSetSourceProcessor();
    ~VolumeSetSourceProcessor();
    	
    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;

    /**
     * Overwritten for managing connections to VolumeSelectionProcessor: Those processors need
     * to be informed about the processors they are connected to in RPTGui.
     */
    virtual bool connect(Port* outport, Port* inport);
    virtual Processor* create();

    virtual void process(LocalPortMapping* /*portMapping*/) {}
    virtual void processMessage(Message* msg,const Identifier &dest);

    VolumeSet* getVolumeSet();
    VolumeSet** getVolumeSetAddress();

    void setVolumeSet(VolumeSet* const volumeset);

    const VolumeSetContainer* getVolumeSetContainer() const;
    void setVolumeSetContainer(VolumeSetContainer* const volumesetContainer);

    static const Identifier msgUpdateCurrentVolumeset_;

    /**
     * Implementation of the method inherited from <code>class Observer</code>.
     * Due to "abuse" of the observer role of the processors, this method is
     * actually not needed and never called.
     * Instead of only notifying the processors, the observed objects shall
     * try to cast their observers to a VolumeSetSourceProcessor and call
     * the desired method like it is done by the VolumeSelectionPlugin from
     * kahuna: on changing the selected VolumeSet, the plugin calls
     * <code>setVolumeSet()</code> on the casted observer.
     */
    virtual void notify(const Observable* const /*source = 0*/) {}
    
protected:
    VolumeSetContainer* volumesetContainer_;

    VolumeSet* volumeset_;
    Identifier outportName_;

    std::vector<std::string> availableVolumesets_;
    EnumProp* volumesetsProp_;

private:
    void updateAvailableVolumeSets();
    void updateCurrentVolumeSet(const std::string& volumesetName);
};

} // namespace

#endif
