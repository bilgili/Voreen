################################################################################
# Project file for the Voreen-Core library
################################################################################
PROJECT(voreen_core)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
INCLUDE(../../cmake/commonconf.cmake)

MESSAGE(STATUS "Configuring voreen_core Library")

# please insert new files in alphabetically order!
SET(VRN_CORE_SOURCES 
    version.cpp
    voreenapplication.cpp
    voreenmodule.cpp
    
    animation/animatedprocessor.cpp
    animation/animation.cpp
    animation/interpolationfunction.cpp
    animation/interpolationfunctionbase.cpp
    animation/interpolationfunctionfactory.cpp
    animation/propertykeyvalue.cpp
    animation/serializationfactories.cpp
    animation/templatepropertytimeline.cpp
    animation/templatepropertytimelinestate.cpp
    animation/interpolation/basicfloatinterpolation.cpp
    animation/interpolation/basicintinterpolation.cpp
    animation/interpolation/boolinterpolationfunctions.cpp
    animation/interpolation/camerainterpolationfunctions.cpp
    animation/interpolation/floatinterpolationfunctions.cpp
    animation/interpolation/intinterpolationfunctions.cpp
    animation/interpolation/ivec2interpolationfunctions.cpp
    animation/interpolation/ivec3interpolationfunctions.cpp
    animation/interpolation/ivec4interpolationfunctions.cpp
    animation/interpolation/mat2interpolationfunctions.cpp
    animation/interpolation/mat3interpolationfunctions.cpp
    animation/interpolation/mat4interpolationfunctions.cpp
    animation/interpolation/quatinterpolationfunctions.cpp
    animation/interpolation/shadersourceinterpolationfunctions.cpp
    animation/interpolation/stringinterpolationfunctions.cpp
    animation/interpolation/transfuncinterpolationfunctions.cpp
    animation/interpolation/vec2interpolationfunctions.cpp
    animation/interpolation/vec3interpolationfunctions.cpp
    animation/interpolation/vec4interpolationfunctions.cpp
    
    datastructures/datetime.cpp
    datastructures/imagesequence.cpp
    datastructures/rendertarget.cpp
    datastructures/geometry/facegeometry.cpp
    datastructures/geometry/geometry.cpp
    datastructures/geometry/geometryfactory.cpp
    datastructures/geometry/meshgeometry.cpp
    datastructures/geometry/meshlistgeometry.cpp
    datastructures/geometry/vertexgeometry.cpp
    datastructures/meta/aggregationmetadata.cpp
    datastructures/meta/coremetadatafactory.cpp
    datastructures/meta/metadatacontainer.cpp
    datastructures/meta/positionmetadata.cpp
    datastructures/meta/realworldmappingmetadata.cpp
    datastructures/meta/windowstatemetadata.cpp
    datastructures/meta/zoommetadata.cpp
    datastructures/roi/roiaggregation.cpp
    datastructures/roi/roiunion.cpp
    datastructures/roi/roisubstract.cpp
    datastructures/roi/roibase.cpp
    datastructures/roi/roinormalizedgeometry.cpp
    datastructures/roi/roicube.cpp
    datastructures/roi/coreroifactory.cpp
    datastructures/roi/roifactory.cpp
    datastructures/roi/roigraph.cpp
    datastructures/roi/roiraster.cpp
    datastructures/roi/roisphere.cpp
    datastructures/roi/roicylinder.cpp
    datastructures/roi/roisingle.cpp
    datastructures/roi/roicollection.cpp
    datastructures/transfunc/preintegrationtable.cpp
    datastructures/transfunc/transfunc.cpp
    datastructures/transfunc/transfunc1dkeys.cpp
    datastructures/transfunc/transfunc2dprimitives.cpp
    datastructures/transfunc/transfuncfactory.cpp
    datastructures/transfunc/transfuncmappingkey.cpp
    datastructures/transfunc/transfuncprimitive.cpp    
    datastructures/volume/histogram.cpp
    datastructures/volume/modality.cpp
    datastructures/volume/volumeram.cpp
    datastructures/volume/volumecollection.cpp
    datastructures/volume/volumecontainer.cpp
    datastructures/volume/volumederiveddata.cpp
    datastructures/volume/volumederiveddatafactory.cpp 
    datastructures/volume/volumeelement.cpp 
    datastructures/volume/volumefactory.cpp 
    datastructures/volume/volumegl.cpp
    datastructures/volume/volumedisk.cpp
    datastructures/volume/volume.cpp
    datastructures/volume/volumedecorator.cpp
    datastructures/volume/volumehash.cpp
    datastructures/volume/volumeminmax.cpp
    datastructures/volume/volumepreview.cpp
    datastructures/volume/volumerepresentation.cpp
    datastructures/volume/volumetexture.cpp
    datastructures/volume/volumeslicehelper.cpp
    datastructures/volume/operators/volumeoperatorregiongrow.cpp
    
    interaction/booltoggleinteractionhandler.cpp
    interaction/buttonpressinteractionhandler.cpp
    interaction/camerainteractionhandler.cpp
    interaction/firstpersonnavigation.cpp
    interaction/flythroughnavigation.cpp
    interaction/idmanager.cpp
    interaction/interactionhandler.cpp
    interaction/interactionhandlerfactory.cpp
    interaction/pickingmanager.cpp
    interaction/slicecamerainteractionhandler.cpp
    interaction/trackballnavigation.cpp
    interaction/voreentrackball.cpp
    
    io/progressbar.cpp
    io/textfilereader.cpp
    io/volumereader.cpp
    io/volumeserializer.cpp
    io/volumeserializerpopulator.cpp
    io/volumewriter.cpp
    io/serialization/xmldeserializer.cpp
    io/serialization/xmlserializationconstants.cpp
    io/serialization/xmlserializer.cpp
    io/serialization/xmlserializerbase.cpp
    
    network/networkconverter.cpp
    network/networkevaluator.cpp
    network/networkgraph.cpp 
    network/networkserializer.cpp 
    network/portconnection.cpp 
    network/processornetwork.cpp 
    network/processornetworkobserver.cpp
    network/workspace.cpp
    
    ports/coprocessorport.cpp 
    ports/geometryport.cpp
    ports/port.cpp
    ports/renderport.cpp
    ports/roiport.cpp
    ports/textport.cpp
    ports/volumeport.cpp
    ports/conditions/portcondition.cpp
    ports/conditions/portconditionvolumetype.cpp 
    
    processors/cache.cpp
    processors/geometryrendererbase.cpp
    processors/imageprocessor.cpp
    processors/imageprocessorbypassable.cpp
    processors/processor.cpp
    processors/processorfactory.cpp
    processors/processorwidget.cpp
    processors/profiling.cpp
    processors/renderprocessor.cpp
    processors/volumeprocessor.cpp
    processors/volumeraycaster.cpp
    processors/volumerenderer.cpp
    processors/volumeslicer.cpp
    
    properties/boolproperty.cpp
    properties/buttonproperty.cpp
    properties/cameraproperty.cpp
    properties/condition.cpp
    properties/eventproperty.cpp
    properties/filedialogproperty.cpp
    properties/floatproperty.cpp
    properties/fontproperty.cpp
    properties/intproperty.cpp
    properties/matrixproperty.cpp 
    properties/numericproperty.cpp
    properties/optionproperty.cpp
    properties/property.cpp
    properties/propertyfactory.cpp
    properties/propertyowner.cpp
    properties/propertyvector.cpp
    properties/shaderproperty.cpp
    properties/stringproperty.cpp
    properties/stringexpressionproperty.cpp
    properties/transfuncproperty.cpp
    properties/vectorproperty.cpp
    properties/voxeltypeproperty.cpp
    properties/volumeurllistproperty.cpp
    properties/volumeurlproperty.cpp
    properties/link/corelinkevaluatorfactory.cpp
    properties/link/dependencylinkevaluator.cpp
    # properties/link/dependencylinkevaluators.cpp
    properties/link/linkevaluatorbase.cpp
    properties/link/linkevaluatorboolinvert.cpp
    properties/link/linkevaluatorfactory.cpp
    properties/link/linkevaluatorid.cpp
    properties/link/propertylink.cpp
    
    utils/classificationmodes.cpp
    utils/commandlineparser.cpp
    utils/glsl.cpp
    utils/hashing.cpp
    utils/observer.cpp
    utils/stringutils.cpp
    utils/statistics.cpp
    utils/voreenpainter.cpp
    utils/GLSLparser/grammarsymbol.cpp
    utils/GLSLparser/lexer.cpp
    utils/GLSLparser/parser.cpp
    utils/GLSLparser/parseraction.cpp
    utils/GLSLparser/parsetreenode.cpp
    utils/GLSLparser/production.cpp
    utils/GLSLparser/terminalsmap.cpp
    utils/GLSLparser/token.cpp
    utils/GLSLparser/annotations/annotationlexer.cpp
    utils/GLSLparser/annotations/annotationparser.cpp
    utils/GLSLparser/annotations/annotationterminals.cpp
    utils/GLSLparser/annotations/annotationvisitor.cpp
    utils/GLSLparser/generator/annotationgrammar.cpp
    utils/GLSLparser/generator/glslgrammar.cpp
    utils/GLSLparser/generator/grammar.cpp
    utils/GLSLparser/generator/lritem.cpp
    utils/GLSLparser/generator/parsertable.cpp
    utils/GLSLparser/generator/ppgrammar.cpp
    utils/GLSLparser/glsl/glslsymbol.cpp
    utils/GLSLparser/glsl/glslterminals.cpp
    utils/GLSLparser/glsl/glslvisitor.cpp
    utils/GLSLparser/preprocessor/pplexer.cpp
    utils/GLSLparser/preprocessor/ppmacro.cpp
    utils/GLSLparser/preprocessor/ppparser.cpp
    utils/GLSLparser/preprocessor/ppstatement.cpp
    utils/GLSLparser/preprocessor/ppterminals.cpp
    utils/GLSLparser/preprocessor/ppvisitor.cpp
    utils/voreenblas/voreenblascpu.cpp
)

SET(VRN_CORE_HEADERS 
    ../../include/voreen/core/version.h
    ../../include/voreen/core/voreenapplication.h
    ../../include/voreen/core/voreencoreapi.h
    ../../include/voreen/core/voreenmodule.h

    ../../include/voreen/core/animation/animatedprocessor.h
    ../../include/voreen/core/animation/animation.h
    ../../include/voreen/core/animation/animationobserver.h
    ../../include/voreen/core/animation/interpolationfunction.h
    ../../include/voreen/core/animation/interpolationfunctionbase.h
    ../../include/voreen/core/animation/interpolationfunctionfactory.h
    ../../include/voreen/core/animation/propertykeyvalue.h 
    ../../include/voreen/core/animation/propertytimeline.h
    ../../include/voreen/core/animation/serializationfactories.h
    ../../include/voreen/core/animation/templatepropertytimeline.h
    ../../include/voreen/core/animation/templatepropertytimelinestate.h
    ../../include/voreen/core/animation/timelineobserver.h
    ../../include/voreen/core/animation/undoableanimation.h
    ../../include/voreen/core/animation/interpolation/basicfloatinterpolation.h
    ../../include/voreen/core/animation/interpolation/basicintinterpolation.h
    ../../include/voreen/core/animation/interpolation/boolinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/camerainterpolationfunctions.h 
    ../../include/voreen/core/animation/interpolation/floatinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/intinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/ivec2interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/ivec3interpolationfunctions.h 
    ../../include/voreen/core/animation/interpolation/ivec4interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/mat2interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/mat3interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/mat4interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/quatinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/shadersourceinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/stringinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/transfuncinterpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/vec2interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/vec3interpolationfunctions.h
    ../../include/voreen/core/animation/interpolation/vec4interpolationfunctions.h

    ../../include/voreen/core/datastructures/datetime.h
    ../../include/voreen/core/datastructures/imagesequence.h
    ../../include/voreen/core/datastructures/rendertarget.h
    ../../include/voreen/core/datastructures/tensor.h
    ../../include/voreen/core/datastructures/geometry/facegeometry.h
    ../../include/voreen/core/datastructures/geometry/geometry.h
    ../../include/voreen/core/datastructures/geometry/geometryfactory.h
    ../../include/voreen/core/datastructures/geometry/meshgeometry.h
    ../../include/voreen/core/datastructures/geometry/meshlistgeometry.h
    ../../include/voreen/core/datastructures/geometry/pointlistgeometry.h
    ../../include/voreen/core/datastructures/geometry/pointsegmentlistgeometry.h
    ../../include/voreen/core/datastructures/geometry/vertexgeometry.h
    ../../include/voreen/core/datastructures/meta/aggregationmetadata.h 
    ../../include/voreen/core/datastructures/meta/coremetadatafactory.h
    ../../include/voreen/core/datastructures/meta/filelistmetadata.h
    ../../include/voreen/core/datastructures/meta/metadatabase.h
    ../../include/voreen/core/datastructures/meta/metadatacontainer.h
    ../../include/voreen/core/datastructures/meta/primitivemetadata.h 
    ../../include/voreen/core/datastructures/meta/positionmetadata.h
    ../../include/voreen/core/datastructures/meta/realworldmappingmetadata.h
    ../../include/voreen/core/datastructures/meta/selectionmetadata.h
    ../../include/voreen/core/datastructures/meta/windowstatemetadata.h
    ../../include/voreen/core/datastructures/meta/zoommetadata.h
    ../../include/voreen/core/datastructures/roi/roiaggregation.h
    ../../include/voreen/core/datastructures/roi/roiunion.h
    ../../include/voreen/core/datastructures/roi/roisubstract.h
    ../../include/voreen/core/datastructures/roi/roibase.h
    ../../include/voreen/core/datastructures/roi/roinormalizedgeometry.h
    ../../include/voreen/core/datastructures/roi/roicube.h
    ../../include/voreen/core/datastructures/roi/coreroifactory.h
    ../../include/voreen/core/datastructures/roi/roifactory.h
    ../../include/voreen/core/datastructures/roi/roigraph.h
    ../../include/voreen/core/datastructures/roi/roiraster.h
    ../../include/voreen/core/datastructures/roi/roisingle.h
    ../../include/voreen/core/datastructures/roi/roisphere.h
    ../../include/voreen/core/datastructures/roi/roicylinder.h
    ../../include/voreen/core/datastructures/roi/roicollection.h
    ../../include/voreen/core/datastructures/transfunc/preintegrationtable.h
    ../../include/voreen/core/datastructures/transfunc/transfunc.h
    ../../include/voreen/core/datastructures/transfunc/transfunc1dkeys.h
    ../../include/voreen/core/datastructures/transfunc/transfunc2dprimitives.h    
	../../include/voreen/core/datastructures/transfunc/transfuncfactory.h
    ../../include/voreen/core/datastructures/transfunc/transfuncmappingkey.h
    ../../include/voreen/core/datastructures/transfunc/transfuncprimitive.h   
    ../../include/voreen/core/datastructures/volume/histogram.h
    ../../include/voreen/core/datastructures/volume/modality.h
    ../../include/voreen/core/datastructures/volume/volumeram.h
    ../../include/voreen/core/datastructures/volume/volumeatomic.h
    ../../include/voreen/core/datastructures/volume/volumecollection.h
    ../../include/voreen/core/datastructures/volume/volumecontainer.h
    ../../include/voreen/core/datastructures/volume/volumeelement.h
    ../../include/voreen/core/datastructures/volume/volumederiveddata.h
    ../../include/voreen/core/datastructures/volume/volumederiveddatafactory.h
    ../../include/voreen/core/datastructures/volume/volumefactory.h
    ../../include/voreen/core/datastructures/volume/volumefusion.h
    ../../include/voreen/core/datastructures/volume/volumegl.h
    ../../include/voreen/core/datastructures/volume/volumedisk.h
    ../../include/voreen/core/datastructures/volume/volume.h
    ../../include/voreen/core/datastructures/volume/volumedecorator.h
    ../../include/voreen/core/datastructures/volume/volumehash.h
    ../../include/voreen/core/datastructures/volume/volumeminmax.h
    ../../include/voreen/core/datastructures/volume/volumeoperator.h
    ../../include/voreen/core/datastructures/volume/volumepreview.h
    ../../include/voreen/core/datastructures/volume/volumerepresentation.h
    ../../include/voreen/core/datastructures/volume/volumetexture.h
    ../../include/voreen/core/datastructures/volume/volumeslicehelper.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorcalcerror.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorcurvature.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorconvert.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorgradient.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorhalfsample.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorinvert.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorisuniform.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatormagnitude.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatormedian.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorminmax.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatormirror.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatormorphology.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatornumsignificant.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorregiongrow.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorresample.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorresize.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorsecondderivatives.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorsubset.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatorswapendianness.h
    ../../include/voreen/core/datastructures/volume/operators/volumeoperatortranspose.h 

    ../../include/voreen/core/interaction/booltoggleinteractionhandler.h
    ../../include/voreen/core/interaction/buttonpressinteractionhandler.h 
    ../../include/voreen/core/interaction/camerainteractionhandler.h
    ../../include/voreen/core/interaction/firstpersonnavigation.h
    ../../include/voreen/core/interaction/flythroughnavigation.h
    ../../include/voreen/core/interaction/idmanager.h
    ../../include/voreen/core/interaction/interactionhandler.h
    ../../include/voreen/core/interaction/interactionhandlerfactory.h
    ../../include/voreen/core/interaction/mwheelnumpropinteractionhandler.h
    ../../include/voreen/core/interaction/pickingmanager.h
    ../../include/voreen/core/interaction/slicecamerainteractionhandler.h
    ../../include/voreen/core/interaction/trackballnavigation.h
    ../../include/voreen/core/interaction/voreentrackball.h

    ../../include/voreen/core/io/progressbar.h
    ../../include/voreen/core/io/textfilereader.h
    ../../include/voreen/core/io/volumereader.h
    ../../include/voreen/core/io/volumeserializer.h
    ../../include/voreen/core/io/volumeserializerpopulator.h
    ../../include/voreen/core/io/volumewriter.h
    ../../include/voreen/core/io/serialization/abstractserializable.h
    ../../include/voreen/core/io/serialization/resourcefactory.h
    ../../include/voreen/core/io/serialization/serializable.h
    ../../include/voreen/core/io/serialization/serializablefactory.h
    ../../include/voreen/core/io/serialization/serialization.h
    ../../include/voreen/core/io/serialization/serializationexceptions.h 
    ../../include/voreen/core/io/serialization/xmldeserializer.h
    ../../include/voreen/core/io/serialization/xmlprocessor.h
    ../../include/voreen/core/io/serialization/xmlserializationconstants.h
    ../../include/voreen/core/io/serialization/xmlserializer.h
    ../../include/voreen/core/io/serialization/xmlserializerbase.h

    ../../include/voreen/core/network/networkconverter.h 
    ../../include/voreen/core/network/networkevaluator.h
    ../../include/voreen/core/network/networkgraph.h
    ../../include/voreen/core/network/networkserializer.h
    ../../include/voreen/core/network/portconnection.h
    ../../include/voreen/core/network/processornetwork.h
    ../../include/voreen/core/network/processornetworkobserver.h
    ../../include/voreen/core/network/workspace.h

    ../../include/voreen/core/ports/allports.h
    ../../include/voreen/core/ports/coprocessorport.h
    ../../include/voreen/core/ports/genericcoprocessorport.h
    ../../include/voreen/core/ports/genericport.h
    ../../include/voreen/core/ports/geometryport.h
    ../../include/voreen/core/ports/port.h
    ../../include/voreen/core/ports/renderport.h
    ../../include/voreen/core/ports/roiport.h
    ../../include/voreen/core/ports/textport.h
    ../../include/voreen/core/ports/volumeport.h

    ../../include/voreen/core/ports/conditions/portcondition.h
    ../../include/voreen/core/ports/conditions/portconditionvolumetype.h

    ../../include/voreen/core/processors/cache.h
    ../../include/voreen/core/processors/geometryrendererbase.h
    ../../include/voreen/core/processors/imageprocessor.h
    ../../include/voreen/core/processors/imageprocessorbypassable.h
    ../../include/voreen/core/processors/processor.h 
    ../../include/voreen/core/processors/processorfactory.h
    ../../include/voreen/core/processors/processorwidget.h
    ../../include/voreen/core/processors/processorwidgetfactory.h
    ../../include/voreen/core/processors/profiling.h
    ../../include/voreen/core/processors/renderprocessor.h
    ../../include/voreen/core/processors/volumeprocessor.h
    ../../include/voreen/core/processors/volumeraycaster.h
    ../../include/voreen/core/processors/volumerenderer.h
    ../../include/voreen/core/processors/volumeslicer.h

    ../../include/voreen/core/properties/action.h
    ../../include/voreen/core/properties/allactions.h
    ../../include/voreen/core/properties/allproperties.h
    ../../include/voreen/core/properties/boolproperty.h
    ../../include/voreen/core/properties/buttonproperty.h
    ../../include/voreen/core/properties/callmemberaction.h
    ../../include/voreen/core/properties/cameraproperty.h
    ../../include/voreen/core/properties/condition.h
    ../../include/voreen/core/properties/eventproperty.h
    ../../include/voreen/core/properties/filedialogproperty.h
    ../../include/voreen/core/properties/floatproperty.h
    ../../include/voreen/core/properties/fontproperty.h
    ../../include/voreen/core/properties/intproperty.h
    ../../include/voreen/core/properties/matrixproperty.h
    ../../include/voreen/core/properties/numericproperty.h
    ../../include/voreen/core/properties/optionproperty.h
    ../../include/voreen/core/properties/property.h
    ../../include/voreen/core/properties/propertyfactory.h
    ../../include/voreen/core/properties/propertyowner.h
    ../../include/voreen/core/properties/propertyvector.h
    ../../include/voreen/core/properties/propertywidget.h
    ../../include/voreen/core/properties/propertywidgetfactory.h
    ../../include/voreen/core/properties/shaderproperty.h
    ../../include/voreen/core/properties/stringexpressionproperty.h
    ../../include/voreen/core/properties/stringproperty.h 
    ../../include/voreen/core/properties/targetaction.h
    ../../include/voreen/core/properties/templateproperty.h
    ../../include/voreen/core/properties/templatepropertycondition.h
    ../../include/voreen/core/properties/transfuncproperty.h
    ../../include/voreen/core/properties/vectorproperty.h
    ../../include/voreen/core/properties/volumeurlproperty.h
    ../../include/voreen/core/properties/volumeurllistproperty.h
    ../../include/voreen/core/properties/voxeltypeproperty.h
    ../../include/voreen/core/properties/link/corelinkevaluatorfactory.h
    ../../include/voreen/core/properties/link/dependencylinkevaluator.h
    ../../include/voreen/core/properties/link/dependencylinkevaluators.h
    ../../include/voreen/core/properties/link/linkevaluatorbase.h
    ../../include/voreen/core/properties/link/linkevaluatorboolinvert.h
    ../../include/voreen/core/properties/link/linkevaluatorfactory.h
    ../../include/voreen/core/properties/link/linkevaluatorid.h
    ../../include/voreen/core/properties/link/linkevaluatoridgeneric.h
    ../../include/voreen/core/properties/link/propertylink.h

    ../../include/voreen/core/utils/classificationmodes.h
    ../../include/voreen/core/utils/commandlineparser.h
    ../../include/voreen/core/utils/exception.h
    ../../include/voreen/core/utils/glsl.h
    ../../include/voreen/core/utils/hashing.h
    ../../include/voreen/core/utils/observer.h
    ../../include/voreen/core/utils/stringutils.h
    ../../include/voreen/core/utils/statistics.h
    ../../include/voreen/core/utils/voreenpainter.h
    ../../include/voreen/core/utils/GLSLparser/glslannotation.h
    ../../include/voreen/core/utils/GLSLparser/grammarsymbol.h
    ../../include/voreen/core/utils/GLSLparser/lexer.h
    ../../include/voreen/core/utils/GLSLparser/parser.h
    ../../include/voreen/core/utils/GLSLparser/parseraction.h
    ../../include/voreen/core/utils/GLSLparser/parsetreenode.h
    ../../include/voreen/core/utils/GLSLparser/production.h
    ../../include/voreen/core/utils/GLSLparser/symbol.h
    ../../include/voreen/core/utils/GLSLparser/symboltable.h
    ../../include/voreen/core/utils/GLSLparser/terminalsmap.h
    ../../include/voreen/core/utils/GLSLparser/token.h
    ../../include/voreen/core/utils/GLSLparser/annotations/annotationlexer.h
    ../../include/voreen/core/utils/GLSLparser/annotations/annotationnodes.h
    ../../include/voreen/core/utils/GLSLparser/annotations/annotationparser.h
    ../../include/voreen/core/utils/GLSLparser/annotations/annotationterminals.h
    ../../include/voreen/core/utils/GLSLparser/annotations/annotationvisitor.h
    ../../include/voreen/core/utils/GLSLparser/generator/annotationgrammar.h
    ../../include/voreen/core/utils/GLSLparser/generator/glslgrammar.h
    ../../include/voreen/core/utils/GLSLparser/generator/grammar.h
    ../../include/voreen/core/utils/GLSLparser/generator/lritem.h
    ../../include/voreen/core/utils/GLSLparser/generator/parsertable.h
    ../../include/voreen/core/utils/GLSLparser/generator/ppgrammar.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslcondition.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glsldeclaration.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslexpression.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslexternaldeclaration.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslfunctioncall.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslfunctiondefinition.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslfunctionprototype.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslparameter.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslparsetreenode.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslqualifier.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslstatement.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslstructspecifier.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslsymbol.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslterminals.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glsltranslation.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glsltoken.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glsltypespecifier.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslvalue.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslvariable.h
    ../../include/voreen/core/utils/GLSLparser/glsl/glslvisitor.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppexpression.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/pplexer.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppmacro.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppparser.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppparsetreenode.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppstatement.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppsymbol.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppterminals.h
    ../../include/voreen/core/utils/GLSLparser/preprocessor/ppvisitor.h
    ../../include/voreen/core/utils/voreenblas/voreenblas.h
    ../../include/voreen/core/utils/voreenblas/voreenblascpu.h
    ../../include/voreen/core/utils/voreenblas/ellpackmatrix.h    
)

SET(VRN_CORE_SOURCES_EXT 
    ../../ext/tinyxml/tinyxml.cpp
    ../../ext/tinyxml/tinyxml.cpp
    ../../ext/tinyxml/tinystr.cpp
    ../../ext/tinyxml/tinyxmlerror.cpp
    ../../ext/tinyxml/tinyxmlparser.cpp
    ../../ext/md5/md5.cpp
)

SET(VRN_CORE_HEADERS_EXT
    ../../ext/tinyxml/tinyxml.h
    ../../ext/tinyxml/tinystr.h
    ../../ext/md5/md5.h
)

# add module sources and headers
LIST(APPEND VRN_CORE_SOURCES ${VRN_MODULE_CORE_SOURCES} ${VRN_CORE_SOURCES_EXT})
LIST(APPEND VRN_CORE_HEADERS ${VRN_MODULE_CORE_HEADERS} ${VRN_CORE_HEADERS_EXT})


################################################################################
# generate module registration header
################################################################################
SET(REGISTRATION_FILE "${CMAKE_BINARY_DIR}/gen_moduleregistration.h" )
MESSAGE(STATUS "* Generating module registration header ${REGISTRATION_FILE}")

SET(REGISTRATION_SOURCE "// WARNING: This file is generated and will be overwritten on next cmake run!\n" )
LIST(APPEND REGISTRATION_SOURCE "#include \"voreen/core/voreenapplication.h\"\n\n" )
LIST(APPEND REGISTRATION_SOURCE "// include core module class headers\n" )
FOREACH(inc ${VRN_MODULE_CORE_MODULECLASSES_INCLUDES})
    LIST(APPEND REGISTRATION_SOURCE "#include \"${inc}\"\n" )
ENDFOREACH()
LIST(APPEND REGISTRATION_SOURCE "\nnamespace voreen {\n" )
LIST(APPEND REGISTRATION_SOURCE "\n// instantiate core module classes\n" )
LIST(APPEND REGISTRATION_SOURCE "void registerAllModules(VoreenApplication* vapp) {\n" )
IF(VRN_MODULE_CORE_MODULECLASSES)
    LIST(APPEND REGISTRATION_SOURCE "    VoreenModule* module\;\n\n")
    LIST(LENGTH VRN_MODULE_CORE_MODULECLASSES num_modules)
    MATH(EXPR max_index "${num_modules} - 1")
    FOREACH(i RANGE ${max_index})
        LIST(GET VRN_MODULE_CORE_MODULECLASSES ${i} class)
        LIST(GET VRN_MODULE_CORE_MODULECLASSES_INCLUDES ${i} inc)
        FILE(RELATIVE_PATH inc_rel ${VRN_HOME} ${inc})
        GET_FILENAME_COMPONENT(mod_path_rel ${inc_rel} PATH)
        LIST(APPEND REGISTRATION_SOURCE "    module = new ${class}(\"${mod_path_rel}\")\;\n")
        LIST(APPEND REGISTRATION_SOURCE "    vapp->registerModule(module)\;\n\n")
    ENDFOREACH()
ENDIF()
LIST(APPEND REGISTRATION_SOURCE "}\n\n" )
LIST(APPEND REGISTRATION_SOURCE "} // namespace\n" )

FILE(WRITE ${REGISTRATION_FILE} ${REGISTRATION_SOURCE})
LIST(APPEND VRN_CORE_HEADERS ${REGISTRATION_FILE})

################################################################################
# Set git revision
################################################################################

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("revision_template.inc" "gen_revision.inc" @ONLY)

################################################################################
# define library target
################################################################################

# add msvc pch
IF (VRN_PRECOMPILED_HEADER AND MSVC)
    ADD_MSVC_PRECOMPILED_HEADER(pch.h pch.cpp VRN_CORE_SOURCES)
ENDIF()

# define target
ADD_LIBRARY(voreen_core 
	${VRN_CORE_SOURCES} ${VRN_CORE_HEADERS}
) 
ADD_DEFINITIONS(${VRN_DEFINITIONS} ${VRN_MODULE_DEFINITIONS})
IF(VRN_SHARED_LIBS AND MSVC)
    ADD_DEFINITIONS("-DVRN_CORE_BUILD_DLL")
ENDIF()
INCLUDE_DIRECTORIES(${VRN_INCLUDE_DIRECTORIES} ${VRN_MODULE_INCLUDE_DIRECTORIES})
TARGET_LINK_LIBRARIES(voreen_core tgt ${VRN_EXTERNAL_LIBRARIES})

# group source files by subdir
IF(VRN_GROUP_SOURCE_FILES)
    LIST(APPEND RemovePathPrefixes "include/voreen/core")
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VRN_CORE_SOURCES ${VRN_HOME} RemovePathPrefixes)
    DEFINE_SOURCE_GROUPS_FROM_SUBDIR(VRN_CORE_HEADERS ${VRN_HOME} RemovePathPrefixes)
ENDIF()

# add gcc pch
IF (VRN_PRECOMPILED_HEADER AND CMAKE_COMPILER_IS_GNUCXX)
    ADD_GCC_PRECOMPILED_HEADER(voreen_core pch.h)
ENDIF()

################################################################################
# deployment
################################################################################
IF(VRN_ADD_INSTALL_TARGET)
    INSTALL(TARGETS voreen_core 
        RUNTIME DESTINATION .
    )
ENDIF()
