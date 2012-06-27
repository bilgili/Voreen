/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_MUTUALINFORMATIONREGISTRATION_H
#define VRN_MUTUALINFORMATIONREGISTRATION_H

#include "itkprocessor.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/matrixproperty.h"

#include <string>

#include "itkImage.h"
//#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"

namespace voreen {

class Volume;

class MutualInformationRegistration : public VolumeProcessor {
    static const unsigned int Dimension = 3;

    typedef float InternalPixelType;
    typedef itk::Image<InternalPixelType, Dimension> InternalImageType;

    //typedef itk::AffineTransform<double, Dimension> TransformType;
    //typedef TransformType::ParametersType ParametersType;
    typedef itk::VersorRigid3DTransform<double> TransformType;
    typedef TransformType::ParametersType ParametersType;
public:
    MutualInformationRegistration();
    virtual Processor* create() const;

    virtual std::string getCategory() const   { return "Volume Processing"; }
    virtual std::string getClassName() const  { return "MutualInformationRegistration";  }
    virtual CodeState getCodeState() const    { return CODE_STATE_BROKEN; }
    virtual std::string getProcessorInfo() const;

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

    virtual bool isReady() const;
protected:
    virtual void setDescriptions() {
        setDescription("");
    }

    virtual void deinitialize() throw (tgt::Exception);

    virtual void process();

    void updateRegistration();
    void resetRegistration();
    void initializeRegistration();

    void convertVolumes();
    void calculateVoreenTrafo(const tgt::mat4& itkMatrix);
    tgt::mat4 constructTrafoMatrix();

private:
    VolumePort fixedVolumeInport_;
    VolumePort movingVolumeInport_;
    VolumePort outport_;

    Volume* fixedVolumeFloat_;
    Volume* movingVolumeFloat_;

    TransformType::Pointer transform_;

    FloatMat4Property transformationMatrix_;
    IntProperty numLevels_;
    IntProperty numIterations_;
    IntProperty numHistogramBins_;
    FloatProperty numSamples_;
    FloatProperty relaxationFactor_;
    BoolProperty explicitPDF_;
    ButtonProperty updateButton_;
    ButtonProperty resetButton_;
    ButtonProperty initializeButton_;

    static const std::string loggerCat_; ///< category used in logging
};

}   //namespace

#endif
