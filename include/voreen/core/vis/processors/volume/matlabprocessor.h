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

#ifndef VRN_MATLABPROCESSOR_H
#define VRN_MATLABPROCESSOR_H

#include <string>
#include "voreen/core/vis/processors/volume/volumeprocessor.h"
#include "voreen/core/vis/properties/boolproperty.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumeatomic.h"

#include "mat.h" //matlab
#include "engine.h" //matlab

namespace voreen {

#if !defined(MX_API_VER) | (MX_API_VER < 0x07030000)
    typedef int mwSize;
#endif

class VolumeHandle;

class MatlabProcessor : public VolumeProcessor {
public:
    MatlabProcessor();
    virtual ~MatlabProcessor();

    virtual const std::string getClassName() const;
    virtual std::string getModuleName() const { return "matlab"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new MatlabProcessor(); }

    virtual void process();
    virtual void initialize() throw (VoreenException);

private:
    void forceUpdate();
    void processMatlab();

    template<class T>
    static Volume* readMatrix(mxArray* pa, tgt::ivec3 dim, int w) {
        VolumeAtomic<T>* dataset;
        try {
            dataset = new VolumeAtomic<T>(dim);
        }
        catch (std::bad_alloc) {
            throw;
        }
        T* scalars = dataset->voxel();
        T* data = (T *)mxGetData(pa);
        for (int z=0; z<dim.z; ++z) {
            for (int y=0; y<dim.y; ++y) {
                for (int x=0; x<dim.x; ++x) {
                    int pos = z*dim.x*dim.y + y*dim.x + x;
                    int matPos = w*dim.x*dim.y*dim.y + z*dim.x*dim.y + y*dim.x + x;
                    scalars[pos] = data[matPos];
                }
            }
        }
        return dataset;
    }

    template<class T>
    static void writeMatrix(mxArray* pa, VolumeAtomic<T>* dataset, int w) {
        tgt::ivec3 dim = dataset->getDimensions();
        T* scalars = dataset->voxel();
        T* data = (T *)mxGetData(pa);
        for (int z=0; z<dim.z; ++z) {
            for (int y=0; y<dim.y; ++y) {
                for (int x=0; x<dim.x; ++x) {
                    int pos = z*dim.x*dim.y + y*dim.x + x;
                    int matPos = w*dim.x*dim.y*dim.y + z*dim.x*dim.y + y*dim.x + x;
                    data[matPos] = scalars[pos];
                }
            }
        }
    }

    void writeMatrix(Volume* dataset, std::string name);
    Volume* readMatrix(std::string name);
private:
    VolumeHandle* inputVolumeHandle_;       /** VolumeHandle from the inport */
    VolumeHandle* processedVolumeHandle_;   /** VolumeHandle for the locally Volume */
    BoolProp enableProcessingProp_;
    bool forceUpdate_;

    static const std::string loggerCat_;

    Engine *matlabEngine_;
    VolumePort inport_;
    VolumePort outport_;
};

}   //namespace

#endif //VRN_MATLABPROCESSOR_H
