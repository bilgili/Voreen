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

#ifndef VRN_VISIBLEHUMANDATASETCREATOR_H
#define VRN_VISIBLEHUMANDATASETCREATOR_H

#include "voreen/core/vis/processors/volume/datasetcreator.h"
#include "voreen/core/io/visiblehumanreader.h"
#include "voreen/core/vis/properties/allproperties.h"
#include "voreen/core/vis/processors/ports/allports.h"

namespace voreen {

    class VisibleHumanDatasetCreator : public DatasetCreator {
    public:

    VisibleHumanDatasetCreator();

    virtual std::string getCategory() const { return "Data Source"; }
    virtual std::string getClassName() const { return "VisibleHumanDatasetCreator"; }
    virtual CodeState getCodeState() const { return CODE_STATE_BROKEN; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new VisibleHumanDatasetCreator();}

        void createDataset();

        void process();

        void setSlicePath(std::string path) {
            slicePath_ = path;
        }

        void setDatasetName(std::string name) {
            datasetName_ = name;
        }

        void readHeaderInfos();

    protected:
        bool datasetReady_;

        float lastResolution_;

        VisibleHumanReader* vishumReader_;

        std::vector<std::string> sliceNames_;
        std::vector<std::string> headerNames_;

        std::string headerPath_;
        std::string datFileName_;

        std::string headerFileName_;

        float sliceThicknessX_;
        float sliceThicknessY_;
        float sliceThicknessZ_;
        std::string format_;
        std::string objectModel_;
        int bitsStored_;
        int headerSize_;
        bool readInfosFromHeader_;
        int imageDimensionX_;
        int imageDimensionY_;
        int imageMatrixSizeX_;
        int imageMatrixSizeY_;
        int cropLeft_;
        int cropRight_;
        int cropTop_;
        int cropBottom_;

        IntProperty imageMatrixSizeXProp_;
        IntProperty imageMatrixSizeYProp_;
        FloatProperty thicknessXProp_;
        FloatProperty thicknessYProp_;
        FloatProperty thicknessZProp_;
//        EnumProperty* formatProp_;
//        EnumProperty* objectModelProp_;
        IntProperty bitsStoredProp_;
        IntProperty headerSizeProp_;
        BoolProperty readInfosFromHeaderProp_;
        FileDialogProperty datasetNameProp_;
/*       StringVectorProperty* sliceNamesProp_;
        StringVectorProperty* headerNamesProp_; */
        IntProperty cropBottomProp_;
        IntProperty cropTopProp_;
        IntProperty cropLeftProp_;
        IntProperty cropRightProp_;


        VolumePort outport_;

    };
}


#endif
