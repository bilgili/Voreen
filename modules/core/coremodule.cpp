/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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
#include "processors/input/volumelistsource.h"
#include "processors/input/volumesource.h"
#include "processors/input/volumeselector.h"

// output processors
#include "processors/output/canvasrenderer.h"
#include "processors/output/geometrysave.h"
#include "processors/output/imagesequencesave.h"
#include "processors/output/textsave.h"
#include "processors/output/volumesave.h"
#include "processors/output/volumelistsave.h"

// ports
#include "voreen/core/ports/genericport.h"
#include "voreen/core/ports/geometryport.h"
#include "voreen/core/ports/port.h"
#include "voreen/core/ports/loopport.h"
#include "voreen/core/ports/renderport.h"
#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/textport.h"

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

// property link evaluators
#include "voreen/core/properties/link/linkevaluatorboolinvert.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/ports/renderport.h"

// geometry
#include "voreen/core/datastructures/geometry/vertexgeometry.h"
#include "voreen/core/datastructures/geometry/facegeometry.h"
#include "voreen/core/datastructures/geometry/meshgeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointlistgeometry.h"
#include "voreen/core/datastructures/geometry/pointsegmentlistgeometry.h"
#include "voreen/core/datastructures/geometry/geometrysequence.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"

// meta data
#include "voreen/core/datastructures/meta/aggregationmetadata.h"
#include "voreen/core/datastructures/meta/positionmetadata.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "voreen/core/datastructures/meta/selectionmetadata.h"
#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"
#include "voreen/core/datastructures/meta/windowstatemetadata.h"
#include "voreen/core/datastructures/meta/zoommetadata.h"

// transfer functions
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"

// volume derived data
#include "voreen/core/datastructures/volume/volumehash.h"
#include "voreen/core/datastructures/volume/volumeminmax.h"
#include "voreen/core/datastructures/volume/volumepreview.h"
#include "voreen/core/datastructures/volume/histogram.h"

// ROI
#include "voreen/core/datastructures/roi/roicube.h"
#include "voreen/core/datastructures/roi/roicylinder.h"
#include "voreen/core/datastructures/roi/roisphere.h"
#include "voreen/core/datastructures/roi/roiraster.h"
#include "voreen/core/datastructures/roi/roiunion.h"
#include "voreen/core/datastructures/roi/roisubtract.h"
#include "voreen/core/datastructures/roi/roigraph.h"

// volume i/o
#include "io/datvolumereader.h"
#include "io/datvolumewriter.h"
#include "io/rawvolumereader.h"
#include "io/vvdvolumereader.h"
#include "io/vvdvolumewriter.h"

// volume operator
#include "voreen/core/datastructures/volume/operators/volumeoperatorcalcerror.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorcurvature.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorequalize.h"
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
#include "voreen/core/datastructures/volume/operators/volumeoperatorresizepoweroftwo.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsecondderivatives.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorsubset.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorswapendianness.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatortranspose.h"

#include "voreen/core/animation/animation.h"
#include "voreen/core/voreenapplication.h"

namespace voreen {

const std::string CoreModule::loggerCat_("voreen.CoreModule");

CoreModule::CoreModule(const std::string& modulePath)
    : VoreenModule(modulePath)
{
    setID("Core");
    setGuiName("Core");

    tgtAssert(VoreenApplication::app(), "VoreenApplicaton not instantiated");

    addShaderPath(getModulePath("glsl"));
    addShaderPath(getModulePath("glsl/utils"));

    // processors
    registerSerializableType(new VolumeSource());
    registerSerializableType(new VolumeListSource());
    registerSerializableType(new VolumeSelector());
    registerSerializableType(new ImageSource());
    registerSerializableType(new ImageSequenceSource());
    registerSerializableType(new ImageSelector());
    registerSerializableType(new GeometrySource());
    registerSerializableType(new TextSource());

    registerSerializableType(new CanvasRenderer());
    registerSerializableType(new VolumeSave());
    registerSerializableType(new VolumeListSave());
    registerSerializableType(new ImageSequenceSave());
    registerSerializableType(new GeometrySave());
    registerSerializableType(new TextSave());

    // ports
    registerSerializableType(new VolumePort(Port::OUTPORT, "dummy"));
    registerSerializableType(new RenderPort(Port::OUTPORT, "dummy"));
    registerSerializableType(new GeometryPort(Port::OUTPORT, "dummy"));
    registerSerializableType(new TextPort(Port::OUTPORT, "dummy"));
    registerSerializableType(new VolumeListPort(Port::OUTPORT, "dummy"));
    registerSerializableType(new ImageSequencePort(Port::OUTPORT, "dummy"));
    registerSerializableType(new LoopPort(Port::OUTPORT, "dummy"));

    // properties
    registerSerializableType(new FloatProperty());
    registerSerializableType(new IntProperty());
    registerSerializableType(new IntVec2Property());
    registerSerializableType(new IntVec3Property());
    registerSerializableType(new IntVec4Property());
    registerSerializableType(new FloatVec2Property());
    registerSerializableType(new FloatVec3Property());
    registerSerializableType(new FloatVec4Property());
    registerSerializableType(new FloatMat2Property());
    registerSerializableType(new FloatMat3Property());
    registerSerializableType(new FloatMat4Property());

    registerSerializableType(new BoolProperty());
    registerSerializableType(new ButtonProperty());
    registerSerializableType(new CameraProperty());
    registerSerializableType(new FileDialogProperty());
    registerSerializableType(new FontProperty());
    registerSerializableType(dynamic_cast<Property*>(new PropertyVector()));
    registerSerializableType(new ShaderProperty());
    registerSerializableType(new StringExpressionProperty());
    registerSerializableType(new StringProperty());
    registerSerializableType(new TransFuncProperty());
    registerSerializableType(new VolumeURLListProperty());
    registerSerializableType(new VolumeURLProperty());
    registerSerializableType(dynamic_cast<Property*>(new VoxelTypeProperty()));

    registerSerializableType(new IntOptionProperty());
    registerSerializableType(new FloatOptionProperty());
    registerSerializableType(new GLEnumOptionProperty());
    registerSerializableType(new StringOptionProperty());

    // -----------------
    //  Link Evaluators
    // -----------------

    // id
    registerSerializableType(new LinkEvaluatorBoolId());
    registerSerializableType(new LinkEvaluatorIntId());
    registerSerializableType(new LinkEvaluatorFloatId());
    registerSerializableType(new LinkEvaluatorDoubleId());

    registerSerializableType(new LinkEvaluatorRenderSize());      //< specialized ivec2 id link for rendering sizes
    registerSerializableType(new LinkEvaluatorIVec2Id());
    registerSerializableType(new LinkEvaluatorIVec3Id());
    registerSerializableType(new LinkEvaluatorIVec4Id());

    registerSerializableType(new LinkEvaluatorLightSourceId());
    registerSerializableType(new LinkEvaluatorVec2Id());
    registerSerializableType(new LinkEvaluatorVec3Id());
    registerSerializableType(new LinkEvaluatorVec4Id());

    registerSerializableType(new LinkEvaluatorDVec2Id());
    registerSerializableType(new LinkEvaluatorDVec3Id());
    registerSerializableType(new LinkEvaluatorDVec4Id());

    registerSerializableType(new LinkEvaluatorMat2Id());
    registerSerializableType(new LinkEvaluatorMat3Id());
    registerSerializableType(new LinkEvaluatorMat4Id());

    // id conversion
    registerSerializableType(new LinkEvaluatorDoubleFloatId());
    registerSerializableType(new LinkEvaluatorDoubleIntId());
    registerSerializableType(new LinkEvaluatorDoubleBoolId());
    registerSerializableType(new LinkEvaluatorFloatIntId());
    registerSerializableType(new LinkEvaluatorFloatBoolId());
    registerSerializableType(new LinkEvaluatorIntBoolId());

    registerSerializableType(new LinkEvaluatorDVec2IVec2Id());
    registerSerializableType(new LinkEvaluatorDVec3IVec3Id());
    registerSerializableType(new LinkEvaluatorDVec4IVec4Id());

    registerSerializableType(new LinkEvaluatorDVec2Vec2Id());
    registerSerializableType(new LinkEvaluatorDVec3Vec3Id());
    registerSerializableType(new LinkEvaluatorDVec4Vec4Id());

    registerSerializableType(new LinkEvaluatorVec2IVec2Id());
    registerSerializableType(new LinkEvaluatorVec3IVec3Id());
    registerSerializableType(new LinkEvaluatorVec4IVec4Id());

    // id normalized
    registerSerializableType(new LinkEvaluatorIntIdNormalized());
    registerSerializableType(new LinkEvaluatorFloatIdNormalized());
    registerSerializableType(new LinkEvaluatorDoubleIdNormalized());

    registerSerializableType(new LinkEvaluatorIVec2IdNormalized());
    registerSerializableType(new LinkEvaluatorIVec3IdNormalized());
    registerSerializableType(new LinkEvaluatorIVec4IdNormalized());

    registerSerializableType(new LinkEvaluatorVec2IdNormalized());
    registerSerializableType(new LinkEvaluatorVec3IdNormalized());
    registerSerializableType(new LinkEvaluatorVec4IdNormalized());

    registerSerializableType(new LinkEvaluatorDVec2IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec3IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec4IdNormalized());

    registerSerializableType(new LinkEvaluatorMat2IdNormalized());
    registerSerializableType(new LinkEvaluatorMat3IdNormalized());
    registerSerializableType(new LinkEvaluatorMat4IdNormalized());

    // id normalized conversion
    registerSerializableType(new LinkEvaluatorDoubleFloatIdNormalized());
    registerSerializableType(new LinkEvaluatorDoubleIntIdNormalized());
    registerSerializableType(new LinkEvaluatorFloatIntIdNormalized());
    registerSerializableType(new LinkEvaluatorBoolInvert());

    registerSerializableType(new LinkEvaluatorDVec2IVec2IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec3IVec3IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec4IVec4IdNormalized());

    registerSerializableType(new LinkEvaluatorDVec2Vec2IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec3Vec3IdNormalized());
    registerSerializableType(new LinkEvaluatorDVec4Vec4IdNormalized());

    registerSerializableType(new LinkEvaluatorVec2IVec2IdNormalized());
    registerSerializableType(new LinkEvaluatorVec3IVec3IdNormalized());
    registerSerializableType(new LinkEvaluatorVec4IVec4IdNormalized());

    // non-numeric links
    registerSerializableType(new LinkEvaluatorStringId());
    registerSerializableType(new LinkEvaluatorShaderId());

    registerSerializableType(new LinkEvaluatorCameraId());
    registerSerializableType(new LinkEvaluatorCameraOrientationId());
    registerSerializableType(new LinkEvaluatorCameraPosId());
    registerSerializableType(new LinkEvaluatorCameraLookId());
    registerSerializableType(new LinkEvaluatorCameraFocusId());
    registerSerializableType(new LinkEvaluatorCameraFrustumId());

    registerSerializableType(new LinkEvaluatorTransFuncId());
    registerSerializableType(new LinkEvaluatorButtonId());

    //----------------------------------------------------------------------
    // link evaluators end

    // geometry
    registerSerializableType(new VertexGeometry());
    registerSerializableType(new FaceGeometry());
    registerSerializableType(new MeshGeometry());
    registerSerializableType(new MeshListGeometry());
    registerSerializableType(new GeometrySequence());
    registerSerializableType(new TriangleMeshGeometrySimple());
    registerSerializableType(new TriangleMeshGeometryVec3());
    registerSerializableType(new TriangleMeshGeometryVec4Vec3());
    registerSerializableType(new PointListGeometryVec3());
    registerSerializableType(new PointSegmentListGeometryVec3());

    // volume derived data
    registerSerializableType(new VolumeMinMax());
    registerSerializableType(new VolumeHash());
    registerSerializableType(new VolumePreview());
    registerSerializableType(new VolumeHistogramIntensity());
    registerSerializableType(new VolumeHistogramIntensityGradient());

    // meta data
    registerSerializableType(new BoolMetaData());
    registerSerializableType(new StringMetaData());
    registerSerializableType(new IntMetaData());
    registerSerializableType(new SizeTMetaData());
    registerSerializableType(new FloatMetaData());
    registerSerializableType(new DoubleMetaData());

    registerSerializableType(new Vec2MetaData());
    registerSerializableType(new IVec2MetaData());
    registerSerializableType(new Vec3MetaData());
    registerSerializableType(new IVec3MetaData());
    registerSerializableType(new Vec4MetaData());
    registerSerializableType(new IVec4MetaData());
    registerSerializableType(new Mat2MetaData());
    registerSerializableType(new Mat3MetaData());
    registerSerializableType(new Mat4MetaData());
    registerSerializableType(new DateTimeMetaData());
    registerSerializableType(new AggregationMetaDataContainer());
    registerSerializableType(new PositionMetaData());
    registerSerializableType(new RealWorldMappingMetaData());
    registerSerializableType("SelectionMetaData::Processor", new SelectionMetaData<Processor*>());
    registerSerializableType(new WindowStateMetaData());
    registerSerializableType(new ZoomMetaData());

    // transfer functions
    registerSerializableType("TransFuncIntensity", new TransFunc1DKeys());
    registerSerializableType("TransFuncIntensityGradient", new TransFunc2DPrimitives());
    registerSerializableType(new TransFuncMappingKey(0.f, tgt::vec4(0.f)));
    registerSerializableType(new TransFuncQuad());
    registerSerializableType(new TransFuncBanana());

    // ROI
    registerSerializableType(new ROICube());
    registerSerializableType(new ROICylinder());
    registerSerializableType(new ROISphere());
    registerSerializableType(new ROIRaster());
    registerSerializableType(new ROIUnion());
    registerSerializableType(new ROISubtract());
    registerSerializableType(new ROIGraph());

    // io
    registerVolumeReader(new DatVolumeReader());
    registerVolumeReader(new RawVolumeReader());
    registerVolumeReader(new VvdVolumeReader());
    registerVolumeWriter(new DatVolumeWriter());
    registerVolumeWriter(new VvdVolumeWriter());

    // animation (TODO: convert resources into VoreenSerializableObjects)
    if (VoreenApplication::app()) {
        std::vector<SerializableFactory*> animationFactories = Animation::getSerializerFactories();
        for (size_t i=0; i<animationFactories.size(); i++)
            VoreenApplication::app()->registerSerializerFactory(animationFactories.at(i));
    }


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

    INST_SCALAR_TYPES(VolumeOperatorResizePowerOfTwo, VolumeOperatorResizePowerOfTwoGeneric)
    INST_VECTOR_TYPES(VolumeOperatorResizePowerOfTwo, VolumeOperatorResizePowerOfTwoGeneric)

    INST_SCALAR_TYPES(VolumeOperatorSubset, VolumeOperatorSubsetGeneric)
    INST_VECTOR_TYPES(VolumeOperatorSubset, VolumeOperatorSubsetGeneric)

    INST_SCALAR_TYPES(VolumeOperatorIsUniform, VolumeOperatorIsUniformGeneric)
    INST_VECTOR_TYPES(VolumeOperatorIsUniform, VolumeOperatorIsUniformGeneric)

    INST_SCALAR_TYPES(VolumeOperatorNumSignificant, VolumeOperatorNumSignificantGeneric)
    INST_VECTOR_TYPES(VolumeOperatorNumSignificant, VolumeOperatorNumSignificantGeneric)

    INST_SCALAR_TYPES(VolumeOperatorCalcError, VolumeOperatorCalcErrorGeneric)
    INST_VECTOR_TYPES(VolumeOperatorCalcError, VolumeOperatorCalcErrorGeneric)

    INST_SCALAR_TYPES(VolumeOperatorEqualize, VolumeOperatorEqualizeGeneric)
    //INST_VECTOR_TYPES(VolumeOperatorEqualize, VolumeOperatorEqualizeGeneric)
}

CoreModule::~CoreModule() {
    Animation::deleteSerializerFactories();
}

} // namespace
