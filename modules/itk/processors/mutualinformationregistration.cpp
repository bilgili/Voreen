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

#include "mutualinformationregistration.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include "voreen/core/datastructures/volume/operators/volumeoperatorconvert.h"
#include "voreen/core/datastructures/volume/volumedecorator.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "modules/itk/utils/itkwrapper.h"

#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkCenteredTransformInitializer.h"

#include <iostream>

namespace voreen {

using tgt::mat4;
using tgt::vec3;

template <typename TRegistration>
class RegistrationInterfaceCommand : public itk::Command
{
public:
    typedef  RegistrationInterfaceCommand   Self;
    typedef  itk::Command                   Superclass;
    typedef  itk::SmartPointer<Self>        Pointer;
    itkNewMacro( Self );
protected:
    RegistrationInterfaceCommand() {};

public:
    typedef   TRegistration                              RegistrationType;
    typedef   RegistrationType *                         RegistrationPointer;
    typedef   itk::VersorRigid3DTransformOptimizer       OptimizerType;
    typedef   OptimizerType *                            OptimizerPointer;

    void Execute(itk::Object * object, const itk::EventObject & event)
    {
        if( !(itk::IterationEvent().CheckEvent( &event )) )
        {
            return;
        }
        RegistrationPointer registration = dynamic_cast<RegistrationPointer>( object );

        OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( registration->GetOptimizer() );

        std::cout << "-------------------------------------" << std::endl;
        std::cout << "MultiResolution Level : " << registration->GetCurrentLevel()  << std::endl;
        std::cout << std::endl;

        if ( registration->GetCurrentLevel() == 0 )
        {
            //optimizer->SetMaximumStepLength( 16.00 );
            optimizer->SetMaximumStepLength( 0.20 );
            //optimizer->SetMaximumStepLength( 64.00 );
            //optimizer->SetMinimumStepLength( 0.04 );
            optimizer->SetMinimumStepLength( 0.0001 );
        }
        else
        {
            optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() / 1.1 );
            optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() / 3.0 );
        }
    }

    void Execute(const itk::Object * , const itk::EventObject & ) { return; }
};


//  The following section of code implements an observer
//  that will monitor the evolution of the registration process.
class CommandIterationUpdate : public itk::Command
{
    public:
        typedef  CommandIterationUpdate   Self;
        typedef  itk::Command             Superclass;
        typedef  itk::SmartPointer<Self>  Pointer;
        itkNewMacro( Self );
    protected:
        CommandIterationUpdate() {};
    public:
        //typedef   itk::RegularStepGradientDescentOptimizer OptimizerType;
        typedef   itk::VersorRigid3DTransformOptimizer OptimizerType;
        typedef   const OptimizerType *                    OptimizerPointer;

        void Execute(itk::Object *caller, const itk::EventObject & event)
        {
            Execute( (const itk::Object *)caller, event);
        }

        void Execute(const itk::Object * object, const itk::EventObject & event)
        {
            OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( object );
            if( !(itk::IterationEvent().CheckEvent( &event )) )
            {
                return;
            }
            std::cout << optimizer->GetCurrentIteration() << "   ";
            std::cout << optimizer->GetValue() << "   ";
            std::cout << optimizer->GetCurrentPosition() << std::endl;
        }
};

const std::string MutualInformationRegistration::loggerCat_("voreen.MutualInformationRegistration");

MutualInformationRegistration::MutualInformationRegistration()
    : VolumeProcessor(),
    fixedVolumeInport_(Port::INPORT, "fixedVolumeInport"),
    movingVolumeInport_(Port::INPORT, "movingVolumeInport"),
    outport_(Port::OUTPORT, "output", 0),
    fixedVolumeFloat_(0),
    movingVolumeFloat_(0),
    transformationMatrix_("voreenTransformMatrix", "Voreen transformation matrix", tgt::mat4::identity, tgt::mat4(-200.0), tgt::mat4(200.0), VALID),
    numLevels_("numLevels", "Number of levels", 5, 1, 20, VALID),
    numIterations_("numIterations", "Number of iterations", 200, 1, 5000, VALID),
    numHistogramBins_("numHistogramBins", "Histogram bins", 64, 1, 512, VALID),
    numSamples_("numSamples", "Number of samples", 0.05f, 0.0f, 1.0f, VALID),
    relaxationFactor_("relaxationFactor", "Relaxation factor", 0.9f, 0.0f, 2.0f, VALID),
    explicitPDF_("explicitPDF", "Use explicit PDF", false, VALID),
    updateButton_("updateButton", "Update"),
    resetButton_("resetButton", "Reset"),
    initializeButton_("initializeButton", "Initialize")
{
    addPort(fixedVolumeInport_);
    addPort(movingVolumeInport_);
    addPort(outport_);

    addProperty(transformationMatrix_);
    addProperty(numLevels_);
    addProperty(numIterations_);
    addProperty(numHistogramBins_);
    addProperty(numSamples_);
    addProperty(relaxationFactor_);
    addProperty(explicitPDF_);

    addProperty(updateButton_);
    updateButton_.onChange(CallMemberAction<MutualInformationRegistration>(this, &MutualInformationRegistration::updateRegistration));
    addProperty(resetButton_);
    resetButton_.onChange(CallMemberAction<MutualInformationRegistration>(this, &MutualInformationRegistration::resetRegistration));
    addProperty(initializeButton_);
    initializeButton_.onChange(CallMemberAction<MutualInformationRegistration>(this, &MutualInformationRegistration::initializeRegistration));

    transform_ = TransformType::New();
}


void MutualInformationRegistration::deinitialize() throw (tgt::Exception) {
    VolumeProcessor::deinitialize();

    delete movingVolumeFloat_;
    movingVolumeFloat_ = 0;

    delete fixedVolumeFloat_;
    fixedVolumeFloat_ = 0;
}

Processor* MutualInformationRegistration::create() const {
    return new MutualInformationRegistration();
}

std::string MutualInformationRegistration::getProcessorInfo() const {
    return std::string("ITK test processor (median filter)");
}

bool MutualInformationRegistration::isReady() const {
    return (movingVolumeInport_.isReady() && fixedVolumeInport_.isReady());
}

void MutualInformationRegistration::process() {
    VolumeBase* outputVolume = 0;

    if(fixedVolumeInport_.hasChanged()) {
        delete fixedVolumeFloat_;
        fixedVolumeFloat_ = 0;
    }

    if(movingVolumeInport_.hasChanged()) {
        delete movingVolumeFloat_;
        movingVolumeFloat_ = 0;
    }

    mat4 m = constructTrafoMatrix();
    calculateVoreenTrafo(m);

    outputVolume = new VolumeDecoratorReplace(movingVolumeInport_.getData(), "transformation", new Mat4MetaData(transformationMatrix_.get()));
    outport_.setData(outputVolume);
}


void MutualInformationRegistration::updateRegistration() {
    if(!isReady())
        return;

    convertVolumes();

    //typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
    typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
    typedef OptimizerType::ScalesType       OptimizerScalesType;
    typedef itk::LinearInterpolateImageFunction< InternalImageType, double             > InterpolatorType;
    typedef itk::MattesMutualInformationImageToImageMetric< InternalImageType, InternalImageType >   MetricType;
    typedef itk::MultiResolutionImageRegistrationMethod< InternalImageType, InternalImageType >   RegistrationType;

    typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   FixedImagePyramidType;
    typedef itk::MultiResolutionPyramidImageFilter< InternalImageType, InternalImageType >   MovingImagePyramidType;

    OptimizerType::Pointer      optimizer     = OptimizerType::New();
    InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
    RegistrationType::Pointer   registration  = RegistrationType::New();
    MetricType::Pointer         metric        = MetricType::New();

    FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
    MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();

    registration->SetOptimizer(optimizer);
    registration->SetTransform(transform_);
    registration->SetInterpolator(interpolator);
    registration->SetMetric(metric);
    registration->SetFixedImagePyramid(fixedImagePyramid);
    registration->SetMovingImagePyramid(movingImagePyramid);

    OptimizerScalesType optimizerScales( transform_->GetNumberOfParameters() );

    float rotScale = 1.0 / 1000.0f;
    optimizerScales[0] = 1.0f;
    optimizerScales[1] = 1.0f;
    optimizerScales[2] = 1.0f;
    optimizerScales[3] = rotScale;
    optimizerScales[4] = rotScale;
    optimizerScales[5] = rotScale;
    optimizer->SetScales( optimizerScales );
    optimizer->SetMaximumStepLength(0.2);
    optimizer->SetMinimumStepLength(0.0001);

    InternalImageType::Pointer fixed = voreenToITK<float>(fixedVolumeFloat_);
    InternalImageType::Pointer moving = voreenToITK<float>(movingVolumeFloat_);
    registration->SetFixedImage(fixed);
    registration->SetMovingImage(moving);
    registration->SetFixedImageRegion( fixed->GetBufferedRegion() );

    registration->SetInitialTransformParameters( transform_->GetParameters() );

    metric->SetNumberOfHistogramBins(numHistogramBins_.get());

    size_t numVoxels = hmul(fixedVolumeFloat_->getDimensions());
    metric->SetNumberOfSpatialSamples(numVoxels * numSamples_.get());

    metric->ReinitializeSeed( 76926294 );

    //// Define whether to calculate the metric derivative by explicitly
    //// computing the derivatives of the joint PDF with respect to the Transform
    //// parameters, or doing it by progressively accumulating contributions from
    //// each bin in the joint PDF.
    metric->SetUseExplicitPDFDerivatives(explicitPDF_.get());

    optimizer->SetNumberOfIterations(numIterations_.get());

    optimizer->SetRelaxationFactor(relaxationFactor_.get());

    // Create the Command observer and register it with the optimizer.
    CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
    optimizer->AddObserver( itk::IterationEvent(), observer );

    typedef RegistrationInterfaceCommand<RegistrationType> CommandType;
    CommandType::Pointer command = CommandType::New();
    registration->AddObserver( itk::IterationEvent(), command );

    registration->SetNumberOfLevels(numLevels_.get());

    try
    {
        registration->StartRegistration();
        std::cout << "Optimizer stop condition: " << registration->GetOptimizer()->GetStopConditionDescription() << std::endl;
    }
    catch( itk::ExceptionObject & err )
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        //return EXIT_FAILURE;
    }

    ParametersType finalParameters = registration->GetLastTransformParameters();
    transform_->SetParameters(finalParameters);

    unsigned int numberOfIterations = optimizer->GetCurrentIteration();

    double bestValue = optimizer->GetValue();

    // Print out results
    std::cout << "Result = " << std::endl;
    std::cout << " Versor " << finalParameters[0] << " " << finalParameters[1] << " " << finalParameters[2] << std::endl;
    std::cout << " Translation " << finalParameters[1] << " " << finalParameters[4] << " " << finalParameters[5] << std::endl;
    std::cout << " Iterations    = " << numberOfIterations << std::endl;
    std::cout << " Metric value  = " << bestValue          << std::endl;

    invalidate(INVALID_RESULT);
}

mat4 MutualInformationRegistration::constructTrafoMatrix() {
    //Construct matrix:
    mat4 itkMat = mat4::identity;
    itk::Point<double, 3> p;

    p[0] = 0.0;
    p[1] = 0.0;
    p[2] = 0.0;
    itk::Point<double, 3> p2 = transform_->TransformPoint(p);

    itkMat.elem[3] = p2[0];
    itkMat.elem[7] = p2[1];
    itkMat.elem[11] = p2[2];

    itk::Vector<double, 3> v;
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = 0.0;
    itk::Vector<double, 3> v2 = transform_->TransformVector(v);

    itkMat.elem[0] = v2[0];
    itkMat.elem[4] = v2[1];
    itkMat.elem[8] = v2[2];

    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = 0.0;
    v2 = transform_->TransformVector(v);

    itkMat.elem[1] = v2[0];
    itkMat.elem[5] = v2[1];
    itkMat.elem[9] = v2[2];

    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 1.0;
    v2 = transform_->TransformVector(v);

    itkMat.elem[2] = v2[0];
    itkMat.elem[6] = v2[1];
    itkMat.elem[10] = v2[2];

    return itkMat;
}

void MutualInformationRegistration::resetRegistration() {
    transform_->SetIdentity();

    invalidate(INVALID_RESULT);
}

void MutualInformationRegistration::initializeRegistration() {
    if(!isReady())
        return;

    convertVolumes();
    resetRegistration();

    InternalImageType::Pointer fixed = voreenToITK<float>(fixedVolumeFloat_);
    InternalImageType::Pointer moving = voreenToITK<float>(movingVolumeFloat_);

    typedef itk::CenteredTransformInitializer< TransformType, InternalImageType, InternalImageType >  TransformInitializerType;

    TransformInitializerType::Pointer initializer = TransformInitializerType::New();

    initializer->SetTransform(transform_);
    initializer->SetFixedImage(fixed);
    initializer->SetMovingImage(moving);

    initializer->MomentsOn();
    initializer->InitializeTransform();

    typedef TransformType::VersorType  VersorType;
    typedef VersorType::VectorType     VectorType;
    VersorType     rotation;
    VectorType     axis;
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    const double angle = 0;
    rotation.Set(  axis, angle  );
    transform_->SetRotation( rotation );

    invalidate(INVALID_RESULT);
}

void MutualInformationRegistration::convertVolumes() {
    const VolumeBase* fixedVolume = fixedVolumeInport_.getData();
    const VolumeBase* movingVolume = movingVolumeInport_.getData();

    if(!fixedVolumeFloat_) {
        //fixedVolumeFloat_ = new VolumeRAM_Float(fixedVolume->getDimensions(), fixedVolume->getSpacing(), fixedVolume->getTransformation());
        VolumeOperatorConvert voConvert;
        fixedVolumeFloat_ = voConvert.apply<float>(fixedVolume);
    }

    if(!movingVolumeFloat_) {
        //movingVolumeFloat_ = new VolumeRAM_Float(movingVolume->getDimensions(), movingVolume->getSpacing(), movingVolume->getTransformation());
        VolumeOperatorConvert voConvert;
        movingVolumeFloat_ = voConvert.apply<float>(movingVolume);
    }
}

void MutualInformationRegistration::calculateVoreenTrafo(const mat4& itkMatrix) {
    const VolumeBase* fixed = fixedVolumeInport_.getData();

    mat4 invertedITKTransform;
    bool success = itkMatrix.invert(invertedITKTransform);
    if(!success) {
        LERROR("Failed to invert ITK transformation matrix!");
        return;
    }
    mat4 f_physicalToWorld = fixed->getPhysicalToWorldMatrix();

    mat4 voreenMatrix = f_physicalToWorld * invertedITKTransform;
    transformationMatrix_.set(voreenMatrix);
    LINFO("Setting matrix");
}


void MutualInformationRegistration::serialize(XmlSerializer& s) const {
    VolumeProcessor::serialize(s);

    ParametersType parameters = transform_->GetParameters();
    ParametersType fixedParameters = transform_->GetFixedParameters();

    std::vector<double> params;
    std::vector<double> fixedParams;

    for(size_t i=0; i<parameters.GetSize(); i++)
        params.push_back(parameters[i]);

    for(size_t i=0; i<fixedParameters.GetSize(); i++)
        fixedParams.push_back(fixedParameters[i]);

    s.serialize("parameters", params);
    s.serialize("fixedParameters", fixedParams);
}

void MutualInformationRegistration::deserialize(XmlDeserializer& s) {
    VolumeProcessor::deserialize(s);

    ParametersType parameters = transform_->GetParameters();
    ParametersType fixedParameters = transform_->GetFixedParameters();

    std::vector<double> params;
    std::vector<double> fixedParams;

    s.deserialize("parameters", params);
    s.deserialize("fixedParameters", fixedParams);

    for(size_t i=0; i<parameters.GetSize(); i++)
        parameters[i] = params[i];

    for(size_t i=0; i<fixedParameters.GetSize(); i++)
        fixedParameters[i] = fixedParams[i];

    transform_->SetParameters(parameters);
    transform_->SetFixedParameters(fixedParameters);
}

}   // namespace
