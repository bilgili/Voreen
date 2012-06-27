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

#include "voreen/modules/connectedcomponents/connectedcomponents2d.h"
#include "tgt/gpucapabilities.h"
#include "tgt/textureunit.h"

#include "connexe/connexe.h"

using tgt::TextureUnit;

namespace voreen {

const std::string ConnectedComponents2D::loggerCat_("voreen.ConnectedComponents2D");

ConnectedComponents2D::ConnectedComponents2D()
    : ImageProcessor("copyimage"),
      inport_(Port::INPORT, "inport"),
      outport_(Port::OUTPORT, "outport"),
      channel_("channel", "Channel"),
      connectivity_("connectivity", "Connectivity"),
      minComponentSize_("minComponentSize", "Min Component Size", 1, 1, 100000),
      maxComponents_("maxComponents", "Max Components", (1<<16)-1, 1, (1<<16)-1),
      componentSorting_("sorting", "Component Sorting"),
      binarizeOutput_("binarizeOutput", "Binarize Output", false),
      stretchLabels_("stretchLabels", "Stretch Labels", true)
{
    addPort(inport_);
    addPort(outport_);

    channel_.addOption("red",       "Red",      GL_RED);
    channel_.addOption("green",     "Green",    GL_GREEN);
    channel_.addOption("blue",      "Blue",     GL_BLUE);
    channel_.addOption("alpha",     "Alpha",    GL_ALPHA);
    channel_.addOption("rgb",       "RGB",      GL_RGB);
    channel_.addOption("rgba",      "RGBA",     GL_RGBA);
    channel_.select("alpha");

    connectivity_.addOption("4-neighborhood", "4 Neighborhood", 4);
    connectivity_.addOption("8-neighborhood", "8 Neighborhood", 8);
    connectivity_.select("8-neighborhood");

    componentSorting_.addOption("none", "None");
    componentSorting_.addOption("decreasing", "Decreasing Size");
    componentSorting_.addOption("increasing", "Increasing Size");

    addProperty(channel_);
    addProperty(connectivity_);
    addProperty(minComponentSize_);
    addProperty(maxComponents_);
    addProperty(componentSorting_);
    addProperty(binarizeOutput_);
    addProperty(stretchLabels_);
}

Processor* ConnectedComponents2D::create() const {
    return new ConnectedComponents2D();
}

std::string ConnectedComponents2D::getProcessorInfo() const {
    return "Detects connected components in a 2D image and luminance-codes the assigned labels in a 16 bit output image."
        "<p><strong>Properties</strong>:"
        "<ul>"
        "  <li>Channel: channel(s) to analyze, pixels with a value greater zero in any of the selected channels are regarded as foreground.</li>"
        "  <li>Connectivity: pixel neighborhood to consider.</li>"
        "  <li>Min Component Size: components consisting of less pixels are discarded.</li>"
        "  <li>Max Components: maximal number of connected components that are put out.</li>"
        "  <li>Component Sorting: determines in which order the component labels are assigned.</li>"
        "  <li>Binarize Output: all connected components are assigned the same label (max intensity).</li>"
        "  <li>Stretch Labels: component labels are stretched over the whole intensity range of the output image in order to maximize visual contrast.</li>"
        "</ul></p>"
        "<p><strong>See</strong>: ConnectedComponents3D</p>";
}

std::string ConnectedComponents2D::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define LUMINANCE_TEXTURE\n";
    return header;
}

void ConnectedComponents2D::process() {

    // activate and clear output render target
    outport_.activateTarget();
    outport_.clearTarget();

    tgt::col4* colorBuffer = inport_.readColorBuffer();
    LGL_ERROR;
    int numPixels = tgt::hmul(inport_.getSize());
    uint8_t* img = new uint8_t[numPixels];
    switch (channel_.getValue()) {
        case GL_RED:
            for (int i=0; i<numPixels; i++)
                img[i] = colorBuffer[i].r;
            break;
        case GL_GREEN:
            for (int i=0; i<numPixels; i++)
                img[i] = colorBuffer[i].g;
            break;
        case GL_BLUE:
            for (int i=0; i<numPixels; i++)
                img[i] = colorBuffer[i].b;
            break;
        case GL_ALPHA:
            for (int i=0; i<numPixels; i++)
                img[i] = colorBuffer[i].a;
            break;
        case GL_RGB:
            // no correct gray value conversion necessary here, since we only need to assign a value greater zero
            // to foreground pixels
            for (int i=0; i<numPixels; i++)
                img[i] = (colorBuffer[i].r > 0 ? 1 : 0) + (colorBuffer[i].g > 0 ? 1 : 0) + (colorBuffer[i].b > 0 ? 1 : 0);
            break;
        case GL_RGBA:
            for (int i=0; i<numPixels; i++)
               img[i] = (colorBuffer[i].r > 0 ? 1 : 0) + (colorBuffer[i].g > 0 ? 1 : 0) + (colorBuffer[i].b > 0 ? 1 : 0) + (colorBuffer[i].a > 0 ? 1 : 0);
            break;

        default:
            LWARNING("Unknown channel option: " << channel_.get());
    }
    delete[] colorBuffer;

    // compute connected components
    uint16_t* labels = new uint16_t[tgt::hmul(inport_.getSize())];
    //Connexe_verbose();
    tgt::ivec3 size(inport_.getSize(), 1);
    int numLabels = CountConnectedComponentsWithAllParams(reinterpret_cast<void*>(img), UCHAR_TYPE,
        reinterpret_cast<void*>(labels), USHORT_TYPE, size.elem, 1.0,
        connectivity_.getValue(), minComponentSize_.get(), maxComponents_.get(), binarizeOutput_.get() ? 1 : 0);

    // free cpu buffer
    delete[] img;

    // sort components
    if (!componentSorting_.isSelected("none") && !binarizeOutput_.get()) {
        int decreasing = componentSorting_.isSelected("decreasing") ? 1 : 0;
        RelabelConnectedComponentsBySize(labels, USHORT_TYPE, size.elem, decreasing);
    }

    // stretch labels to use full range
    if (stretchLabels_.get() && !binarizeOutput_.get()) {
        float scale = static_cast<float>((1<<16) - 2) / numLabels;
        for (int i=0; i<tgt::hmul(inport_.getSize()); i++)
            labels[i] = tgt::ifloor(labels[i]*scale);
    }

    // render label texture
    TextureUnit shadeUnit, depthUnit;
    shadeUnit.activate();
    tgt::Texture* labelTex = new tgt::Texture(reinterpret_cast<GLubyte*>(labels), tgt::ivec3(inport_.getSize(), 1),
        GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_SHORT, tgt::Texture::NEAREST, !GpuCaps.isNpotSupported());
    labelTex->uploadTexture();

    program_->activate();
    inport_.bindDepthTexture(depthUnit.getEnum());
    inport_.setTextureParameters(program_, "texParams_");

    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", shadeUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());

    // render screen aligned quad:
    glDepthFunc(GL_ALWAYS);
    renderQuad();
    glDepthFunc(GL_LESS);

    // cleanup
    program_->deactivate();
    delete labelTex;
    outport_.deactivateTarget();
    glActiveTexture(GL_TEXTURE0);

    LGL_ERROR;
}

} // namespace
