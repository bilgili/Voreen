/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/network/networkconverter.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/core/utils/stringconversion.h"

#include "tinyxml/tinyxml.h"
#include "tgt/logmanager.h"

namespace voreen {

const std::string NetworkConverter::loggerCat_ = "voreen.NetworkConverter";

void NetworkConverter::changePropertyName(TiXmlElement* elem, const std::string& ofProcessor, const std::string& from, const std::string& to) {
    TiXmlElement* processorsNode = elem->FirstChildElement("Processors");
    for (TiXmlElement* procNode = processorsNode->FirstChildElement("Processor")
        ; procNode != 0
        ; procNode = procNode->NextSiblingElement("Processor"))
    {
        const std::string* type = procNode->Attribute(std::string("type"));
        if (ofProcessor == "" || (type && *type == ofProcessor)) {
            TiXmlElement* propertiesNode = procNode->FirstChildElement("Properties");
            for (TiXmlElement* propNode = propertiesNode->FirstChildElement("Property")
                ; propNode != 0
                ; propNode = propNode->NextSiblingElement("Property"))
            {
                const std::string* name = propNode->Attribute(std::string("name"));
                if (name && *name == from)
                    propNode->SetAttribute("name", to);
            }
        }
    }
}

void NetworkConverter::changePropertyValue(TiXmlElement* elem, const std::string& ofProcessor, const std::string& from, const std::string& to) {
    TiXmlElement* processorsNode = elem->FirstChildElement("Processors");
    for (TiXmlElement* procNode = processorsNode->FirstChildElement("Processor")
        ; procNode != 0
        ; procNode = procNode->NextSiblingElement("Processor"))
    {
        const std::string* type = procNode->Attribute(std::string("type"));
        if (ofProcessor == "" || (type && *type == ofProcessor)) {
            TiXmlElement* propertiesNode = procNode->FirstChildElement("Properties");
            for (TiXmlElement* propNode = propertiesNode->FirstChildElement("Property")
                ; propNode != 0
                ; propNode = propNode->NextSiblingElement("Property"))
            {
                const std::string* value = propNode->Attribute(std::string("value"));
                if (value && *value == from)
                    propNode->SetAttribute("value", to);
            }
        }
    }
}

void NetworkConverter::changeProcessorType(TiXmlElement* elem, const std::string& from, const std::string& to) {
    TiXmlElement* processorsNode = elem->FirstChildElement("Processors");
    for (TiXmlElement* node = processorsNode->FirstChildElement("Processor"); node != 0; node = node->NextSiblingElement("Processor")) {
        const std::string* type = node->Attribute(std::string("type"));
        if (type && *type == from) {
            node->SetAttribute("type", to);
            const std::string* name = node->Attribute(std::string("name"));
            if (name && *name == from)
                node->SetAttribute("name", to);
        }
    }
}

//-------------------------------------------------------------------------------------------------

void NetworkConverter4to5::convert(TiXmlElement* elem) {
    //changePropertyName(elem, "", "OldPropertyDescriptor", "NewPropertyDescriptor");
    //changeProcessorType(elem, "OldProcessorClassname", "NewProcessorClassname");
    // AxialSliceProxyGeometry processor
    changePropertyName(elem, "", "set.Axis", "axis");
    changePropertyName(elem, "", "set.Begin", "begin");
    changePropertyName(elem, "", "set.Thickness", "thickness");
    // ClippingPlaneWidget processor
    changePropertyName(elem, "", "set.ClipSliceColor", "clipSliceColor");
    changePropertyName(elem, "", "set.polyOffsetFact", "polyOffsetFact");
    changePropertyName(elem, "", "set.polyOffsetUnit", "polyOffsetUnit");
    changePropertyName(elem, "", "switch.ClipXLock", "switchClipXLock");
    changePropertyName(elem, "", "switch.ClipYLock", "switchClipYLock");
    changePropertyName(elem, "", "switch.ClipZLock", "switchClipZLock");
    // ColorDepth processor
    changePropertyName(elem, "", "set.colordepth.factor", "factor");
    // Compositer processor
    changePropertyName(elem, "", "Compositer.set.blendMode", "blendMode");
    changePropertyValue(elem, "", "Alpha Compositing", "Alpha compositing");
    changePropertyValue(elem, "", "Maximum Alpha", "Maximum alpha");
    changePropertyName(elem, "", "Compositer.set.blendFactor", "blendFactor");
    // CubeProxyGeometry processor
    changePropertyName(elem, "", "bricking.region.priority", "regionPriority");
    changePropertyName(elem, "", "switch.virtualClipplane", "enableVirtualClipplane");
    changePropertyName(elem, "", "set.virtualClipplane", "virtualClipplane");
    // Gaussian processor
    changeProcessorType(elem, "Blur", "Gaussian");
    changePropertyName(elem, "", "set.blurDelta", "sigma");
    changePropertyName(elem, "", "set.blurRed", "blurRed");
    changePropertyName(elem, "", "set.blurGreen", "blurGreen");
    changePropertyName(elem, "", "set.blurBlue", "blurBlue");
    changePropertyName(elem, "", "set.blurAlpha", "blurAlpha");
    // GeomBoundingBox/SlicePositionRenderer processor
    changePropertyName(elem, "", "set.BoundingboxColor", "boundingboxColor");
    changePropertyName(elem, "", "set.BoundingBoxWidth", "boundingBoxWidth");
    changePropertyName(elem, "", "set.BoundingBoxStippleFactor", "boundingBoxStippleFactor");
    changePropertyName(elem, "", "set.BoundingBoxStipplePattern", "boundingBoxStipplePattern");
    // GeomLightWidget processor
    changePropertyName(elem, "", "set.showLightWidget", "showLightWidget");
    // GeomPlane processor
    changePropertyName(elem, "", "set.PlaneXSize", "planeXSize");
    changePropertyName(elem, "", "set.PlaneYSize", "planeYSize");
    // Grayscale processor
    changeProcessorType(elem, "Greyscale", "Grayscale");
    // EdgeDetect processor
    changePropertyName(elem, "", "set.edgeThreshold", "edgeThreshold");
    changePropertyName(elem, "", "set.showImage", "showImage");
    changePropertyName(elem, "", "set.edgeColor", "edgeColor");
    // IDRaycaster processor
    changePropertyName(elem, "", "set.penetrationDepth", "penetrationDepth");
    // ImageOverlay processor
    changePropertyName(elem, "", "set.imageoverlayLeft", "left");
    changePropertyName(elem, "", "set.imageoverlayWidth", "width");
    changePropertyName(elem, "", "set.imageoverlayTop", "top");
    changePropertyName(elem, "", "set.imageoverlayHeight", "height");
    changePropertyName(elem, "", "set.imageoverlayOpacity", "opacity");
    // ImageProcessorDepth processor
    changePropertyName(elem, "", "set.minDepthPP", "minDepth");
    changePropertyName(elem, "", "set.maxDepthPP", "maxDepth");
    // Labeling processor
    changePropertyName(elem, "", "set.labelingWidget", "labelingWidget");
    changePropertyName(elem, "", "set.Layout", "layout");
    changePropertyName(elem, "", "set.LabelColor", "labelColor");
    changePropertyName(elem, "", "set.HaloColor", "haloColor");
    changePropertyName(elem, "", "set.Font", "font");
    changePropertyName(elem, "", "set.FontSize", "fontSize");
    changePropertyName(elem, "", "switch.drawHalo", "drawHalo");
    changePropertyName(elem, "", "set.SegmentDescriptionFile", "segmentDescriptionFile");
    changePropertyName(elem, "", "set.lockInternalFontSettings", "lockInternalFontSettings");
    changePropertyName(elem, "", "set.labelColorIntern", "labelColorIntern");
    changePropertyName(elem, "", "set.haloColorIntern", "haloColorIntern");
    changePropertyName(elem, "", "set.fontSizeIntern", "fontSizeIntern");
    changePropertyName(elem, "", "set.shape3D", "shape3D");
    changePropertyName(elem, "", "set.minSegmentSize", "minSegmentSize");
    changePropertyName(elem, "", "set.maxIterations", "maxIterations");
    changePropertyName(elem, "", "set.filterDelta", "filterDelta");
    changePropertyName(elem, "", "set.distanceMapStep", "distanceMapStep");
    changePropertyName(elem, "", "set.glyphAdvance", "glyphAdvance");
    changePropertyName(elem, "", "set.polynomialDegree", "polynomialDegree");
    changePropertyName(elem, "", "set.bezierHorzDegree", "bezierHorzDegree");
    changePropertyName(elem, "", "set.bezierVertDegree", "bezierVertDegree");
    changePropertyName(elem, "", "set.filterKernel", "filterKernel");
    changePropertyName(elem, "", "set.thresholdPPthreshold", "thresholdPPthreshold");
    changePropertyName(elem, "", "set.labelLayoutAsString", "labelLayoutAsString");
    changePropertyName(elem, "", "set.showLabels", "showLabels");
    // MultipleAxialSliceProxyGeometry processor
    changePropertyName(elem, "", "set.nSlicesPerRow", "nSlicesPerRow");
    changePropertyName(elem, "", "set.nSlicesPerCol", "nSlicesPerCol");
    // RenderProcessor processor
    changePropertyName(elem, "", "set.backgroundColor", "backgroundColor");
    // SliceProxyGeometry processor
    changePropertyName(elem, "", "set.slicePos", "slicePos");
    // Threshold processor
    changePropertyName(elem, "", "set.thresholdPPthreshold", "threshold");
    // Background processor
    changePropertyName(elem, "", "backgroundFirstColor", "color1");
    changePropertyName(elem, "", "backgroundSecondColor", "color2");
    changePropertyName(elem, "", "backgroundAngle", "angle");
    changePropertyName(elem, "", "backgroundfilenameAsString", "texture");
    changePropertyName(elem, "", "backgroundtile", "repeat");
}

void NetworkConverter5to6::convert(TiXmlElement* elem) {
    changeProcessorType(elem, "Compositer", "Compositor");
    changeProcessorType(elem, "LoopCompositer", "LoopCompositor");
    changeProcessorType(elem, "VVCompositer", "VVCompositor");
    changeProcessorType(elem, "SingleSliceRenderer", "SliceViewer");
    changeProcessorType(elem, "SliceSequenceRenderer", "SliceViewer");
    changeProcessorType(elem, "GeomLightWidget", "LightWidgetRenderer");
}

} // namespace
