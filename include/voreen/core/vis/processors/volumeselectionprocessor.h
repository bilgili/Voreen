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

#ifndef VRN_VOLUMESELECTIONPROCESSOR_H
#define VRN_VOLUMESELECTIONPROCESSOR_H

#include "processor.h"

#ifndef VRN_OBSERVER_H
#include "voreen/core/volume/observer.h"
#endif

namespace voreen {

class VolumeSet;
class VolumeSeries;
class VolumeHandle;

class VolumeSelectionProcessor : public Processor, public Observer
{

public: 
    VolumeSelectionProcessor();
    ~VolumeSelectionProcessor();
    	
    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create();

    virtual bool isMultipassCompatible() { return true; }
    virtual void process(LocalPortMapping* portMapping);
    virtual void processMessage(Message* msg, const Identifier& dest);

    VolumeHandle** getVolumeHandleAddress();

    void setVolumeSet(VolumeSet* const volumeset);

    static const Identifier msgSetCurrentModality_;
    static const Identifier msgSetCurrentTimestep_;

    /**
     * Implementation of the method inherited from <code>class Observer</code>.
     * Due to "abuse" of the observer role of the processors, this method is
     * actually not needed and never called.
     * Instead of only notifying the processors, the observed objects shall
     * try to cast their observers to a VolumeSelectionProcessor and call
     * the desired method like it is done by the VolumeSelectionPlugin from
     * kahuna: on changing the selected VolumeSet, the plugin calls
     * <code>setVolumeSeries()</code> and <code>setVolumeHandle()</code>
     * on the casted observer.
     */
    virtual void notify(const Observable* const /*source = 0*/) {}

protected:
    VolumeSet* curVolumeSet_;
    VolumeSeries* volumeSeries_;
    VolumeHandle* volumeHandle_;

    EnumProp* modalitiesProp_;
    IntProp* timestepProp_;
    std::vector<std::string> availableModalities_;

    Identifier volumeSetInportName_;
    Identifier volumeOutportName_;

private:
    void updateAvailableModalities();
    void updateAvailableTimesteps();
    void setCurrentSeries(const std::string& seriesName);
    void setCurrentTimestep(const int timestepIndex);
};

} // namespace

#endif // VRN_VOLUMESELECTIONPROCESSOR_H
