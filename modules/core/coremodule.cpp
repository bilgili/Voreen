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

#include "coremodule.h"

// input processors
#include "processors/input/geometrysource.h"
#include "processors/input/imagesequencesource.h"
#include "processors/input/imagesource.h"
#include "processors/input/imageselector.h"
#include "processors/input/textsource.h"
#include "processors/input/volumecollectionsource.h"
#include "processors/input/volumesource.h"
#include "processors/input/volumeselector.h"

// output processors
#include "processors/output/canvasrenderer.h"
#include "processors/output/geometrysave.h"
#include "processors/output/imagesequencesave.h"
#include "processors/output/textsave.h"
#include "processors/output/volumesave.h"
#include "processors/output/volumecollectionsave.h"

// properties
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/fontproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/propertyvector.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/stringexpressionproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/properties/volumeurlproperty.h"
#include "voreen/core/properties/voxeltypeproperty.h"

// volume i/o
#include "io/datvolumereader.h"
#include "io/datvolumewriter.h"
#include "io/rawvolumereader.h"
#include "io/vvdvolumereader.h"
#include "io/vvdvolumewriter.h"

// serialization factories
#include "voreen/core/datastructures/meta/coremetadatafactory.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/datastructures/geometry/geometryfactory.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/datastructures/volume/volumederiveddatafactory.h"
#include "voreen/core/animation/animation.h"

#include "voreen/core/properties/link/corelinkevaluatorfactory.h"

// volume operator
#include "voreen/core/datastructures/volume/operators/volumeoperatorcalcerror.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorcurvature.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorgradient.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorhalfsample.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorinvert.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorisuniform.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormagnitude.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormedian.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormirror.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormorphology.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatornumsignificant.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorregiongrow.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresample.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresize.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsecondderivatives.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsubset.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorswapendianness.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatortranspose.h"


#include "voreen/core/datastructures/roi/coreroifactory.h"

#include "voreen/core/voreenapplication.h"

namespace voreen {

const std::string CoreModule::loggerCat_("voreen.CoreModule");

CoreModule::CoreModule(const std::string& modulePath)
    : VoreenModule(modulePath)
{
    setName("Core");

    tgtAssert(VoreenApplication::app(), "VoreenApplicaton not instantiated");

    addShaderPath(getModulePath("glsl"));
    addShaderPath(getModulePath("glsl/utils"));

    // core processors
    registerProcessor(new VolumeSource());
    registerProcessor(new VolumeCollectionSource());
    registerProcessor(new VolumeSelector());
    registerProcessor(new ImageSource());
    registerProcessor(new ImageSequenceSource());
    registerProcessor(new ImageSelector());
    registerProcessor(new GeometrySource());
    registerProcessor(new TextSource());

    registerProcessor(new CanvasRenderer());
    registerProcessor(new VolumeSave());
    registerProcessor(new VolumeCollectionSave());
    registerProcessor(new ImageSequenceSave());
    registerProcessor(new GeometrySave());
    registerProcessor(new TextSave());

    // core properties
    registerProperty(new FloatProperty());
    registerProperty(new IntProperty());
    registerProperty(new IntVec2Property());
    registerProperty(new IntVec3Property());
    registerProperty(new IntVec4Property());
    registerProperty(new FloatVec2Property());
    registerProperty(new FloatVec3Property());
    registerProperty(new FloatVec4Property());
    registerProperty(new FloatMat2Property());
    registerProperty(new FloatMat3Property());
    registerProperty(new FloatMat4Property());

    registerProperty(new BoolProperty());
    registerProperty(new ButtonProperty());
    registerProperty(new CameraProperty());
    registerProperty(new FileDialogProperty());
    registerProperty(new FontProperty());
    registerProperty(new PropertyVector());
    registerProperty(new ShaderProperty());
    registerProperty(new StringExpressionProperty());
    registerProperty(new StringProperty());
    registerProperty(new TransFuncProperty());
    registerProperty(new VolumeURLListProperty());
    registerProperty(new VolumeURLProperty());
    registerProperty(new VoxelTypeProperty());

    registerProperty(new IntOptionProperty());
    registerProperty(new FloatOptionProperty());
    registerProperty(new GLEnumOptionProperty());
    registerProperty(new StringOptionProperty());

    // core io
    registerVolumeReader(new DatVolumeReader());
    registerVolumeReader(new RawVolumeReader());
    registerVolumeReader(new VvdVolumeReader());
    registerVolumeWriter(new DatVolumeWriter());
    registerVolumeWriter(new VvdVolumeWriter());

    // core factories
    registerSerializerFactory(new CoreMetaDataFactory());
    registerSerializerFactory(new CoreROIFactory());
    registerSerializerFactory(ProcessorFactory::getInstance());
    registerSerializerFactory(new GeometryFactory());
    registerSerializerFactory(new TransFuncFactory());
    registerSerializerFactory(new VolumeDerivedDataFactory());
    std::vector<SerializableFactory*> animationFactories = Animation::getSerializerFactories();
    for (size_t i=0; i<animationFactories.size(); i++)
        registerSerializerFactory(animationFactories.at(i));

    registerLinkEvaluatorFactory(new CoreLinkEvaluatorFactory());

    // Instance operator for all scalar types:
    INST_SCALAR_TYPES(VolumeOperatorInvert, VolumeOperatorInvertGeneric)
    //INST_VECTOR_TYPES(NewVolumeOperatorInvert, VolumeOperatorInvertGeneric)

    INST_SCALAR_TYPES(VolumeOperatorMirrorX, VolumeOperatorMirrorXGeneric)
    INST_VECTOR_TYPES(VolumeOperatorMirrorX, VolumeOperatorMirrorXGeneric)

    INST_SCALAR_TYPES(VolumeOperatorMirrorY, VolumeOperatorMirrorYGeneric)
    INST_VECTOR_TYPES(VolumeOperatorMirrorY, VolumeOperatorMirrorYGeneric)

    INST_SCALAR_TYPES(VolumeOperatorMirrorZ, VolumeOperatorMirrorZGeneric)
    INST_VECTOR_TYPES(VolumeOperatorMirrorZ, VolumeOperatorMirrorZGeneric)

    INST_SCALAR_TYPES(VolumeOperatorTranspose, VolumeOperatorTransposeGeneric)
    INST_VECTOR_TYPES(VolumeOperatorTranspose, VolumeOperatorTransposeGeneric)
    INST_TENSOR_TYPES(VolumeOperatorTranspose, VolumeOperatorTransposeGeneric)

    INST_SCALAR_TYPES(VolumeOperatorSwapEndianness, VolumeOperatorSwapEndiannessGeneric)
    INST_VECTOR_TYPES(VolumeOperatorSwapEndianness, VolumeOperatorSwapEndiannessGeneric)
    INST_TENSOR_TYPES(VolumeOperatorSwapEndianness, VolumeOperatorSwapEndiannessGeneric)

    INST_SCALAR_TYPES(VolumeOperatorMedian, VolumeOperatorMedianGeneric)

    INST_SCALAR_TYPES(VolumeOperatorDilation, VolumeOperatorDilationGeneric)
    INST_SCALAR_TYPES(VolumeOperatorErosion, VolumeOperatorErosionGeneric)

    INST_SCALAR_TYPES(VolumeOperatorResample, VolumeOperatorResampleGeneric)
    INST_VECTOR_TYPES(VolumeOperatorResample, VolumeOperatorResampleGeneric)

    INST_SCALAR_TYPES(VolumeOperatorHalfsample, VolumeOperatorHalfsampleGeneric)
    INST_VECTOR_TYPES(VolumeOperatorHalfsample, VolumeOperatorHalfsampleGeneric)

    INST_SCALAR_TYPES(VolumeOperatorRegionGrow, VolumeOperatorRegionGrowGeneric)

    INST_SCALAR_TYPES(VolumeOperatorResize, VolumeOperatorResizeGeneric)
    INST_VECTOR_TYPES(VolumeOperatorResize, VolumeOperatorResizeGeneric)
    INST_TENSOR_TYPES(VolumeOperatorResize, VolumeOperatorResizeGeneric)

    INST_SCALAR_TYPES(VolumeOperatorSubset, VolumeOperatorSubsetGeneric)
    INST_VECTOR_TYPES(VolumeOperatorSubset, VolumeOperatorSubsetGeneric)

    INST_SCALAR_TYPES(VolumeOperatorIsUniform, VolumeOperatorIsUniformGeneric)
    INST_VECTOR_TYPES(VolumeOperatorIsUniform, VolumeOperatorIsUniformGeneric)

    INST_SCALAR_TYPES(VolumeOperatorNumSignificant, VolumeOperatorNumSignificantGeneric)
    INST_VECTOR_TYPES(VolumeOperatorNumSignificant, VolumeOperatorNumSignificantGeneric)

    INST_SCALAR_TYPES(VolumeOperatorCalcError, VolumeOperatorCalcErrorGeneric)
    INST_VECTOR_TYPES(VolumeOperatorCalcError, VolumeOperatorCalcErrorGeneric)
}

} // namespace
