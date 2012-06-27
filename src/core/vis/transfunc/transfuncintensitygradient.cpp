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

#include "voreen/core/vis/transfunc/transfuncintensitygradient.h"
#include "voreen/core/vis/transfunc/transfuncprimitive.h"

#include "tinyxml/tinyxml.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

namespace voreen {

const std::string TransFuncIntensityGradient::loggerCat_("voreen.vis.TransFunc.IntensityGradient");

TransFuncIntensityGradient::TransFuncIntensityGradient(int width, int height)
    : TransFunc(width, height)
{
    loadFileFormats_.push_back("tfig");

    saveFileFormats_.push_back("tfig");
#ifdef VRN_WITH_DEVIL
    saveFileFormats_.push_back("png");
#endif

}

TransFuncIntensityGradient::~TransFuncIntensityGradient() {
    std::vector<TransFuncPrimitive*>::iterator it;
    while (!primitives_.empty()) {
        it = primitives_.begin();
        delete *it;
        primitives_.erase(it);
    }
}

bool TransFuncIntensityGradient::operator==(const TransFuncIntensityGradient& tf) {
    if ((getHeight() != tf.getHeight()) || (getWidth() != tf.getWidth()))
        return false;
    // compare every texel
    const GLfloat* pixel_l = getPixelData();
    const GLfloat* pixel_r = tf.getPixelData();
    for (int i = 0; i < getHeight() * getWidth() * 4; ++i) {
        if (pixel_l[i] != pixel_r[i])
            return false;
    }

    return true;
}

bool TransFuncIntensityGradient::operator!=(const TransFuncIntensityGradient& tf) {
    return !(*this == tf);
}

void TransFuncIntensityGradient::setScaleFactor(float factor) {
    for (size_t i = 0; i < primitives_.size(); ++i)
        primitives_[i]->setScaleFactor(factor);
}

void TransFuncIntensityGradient::createTex() {
    delete tex_;

    // create texture but do not alloc data
    tex_ = new tgt::Texture(0, tgt::ivec3(dimension_, 1), GL_RGBA, GL_FLOAT, tgt::Texture::LINEAR, false);
    tex_->setWrapping(tgt::Texture::CLAMP);
    tex_->uploadTexture();

    textureUpdateNeeded_ = true;
}

bool TransFuncIntensityGradient::save(const std::string& filename) {
    //look for fileExtension
    std::string fileExtension;
    size_t dotPosition = filename.rfind(".");
    if (dotPosition == std::string::npos)
        return false;
    else
        fileExtension = filename.substr(dotPosition+1);

    if (fileExtension == "tfig")
        return saveTfig(filename);
    else
        return saveImage(filename);
}

bool TransFuncIntensityGradient::saveTfig(const std::string& filename) {
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
    doc.LinkEndChild(decl);

    TiXmlElement* root = new TiXmlElement("transferfunc2d");
    doc.LinkEndChild(root);

    savePrimitivesToXml(root);

    return doc.SaveFile(filename);
}

void TransFuncIntensityGradient::savePrimitivesToXml(TiXmlElement* root) {
    for (size_t i = 0; i < primitives_.size(); ++i)
         primitives_[i]->saveToXml(root);
}

bool TransFuncIntensityGradient::saveImage(const std::string& filename) {
    //extract file extension
    std::string fileExtension;
    size_t dotPosition = filename.rfind(".");
    fileExtension = filename.substr(dotPosition+1);

#ifdef VRN_WITH_DEVIL
    //IL does _NOT_ overwrite files by default
    ilEnable(IL_FILE_OVERWRITE);
    //download texture and save as png:
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);

    GLubyte* im = new GLubyte[tex_->getWidth()*tex_->getHeight()*4];

    tex_->bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, im);

    ilTexImage(tex_->getWidth(), tex_->getHeight(), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, im);

    if (fileExtension == "png")
        ilSave(IL_PNG, (ILstring)filename.c_str());
    else {
        ilDeleteImages(1, &img);
        delete[] im;
        return false;
    }

    ilDeleteImages(1, &img);
    delete[] im;
    return true;
#else
    LERROR("Saving of " << filename  << " failed: No DevIL support.");
    return false;
#endif // VRN_WITH_DEVIL
}


bool TransFuncIntensityGradient::load(const std::string& filename) {
    // Extract the file extension
    std::string fileExtension;
    size_t dotPosition = filename.rfind(".");
    if (dotPosition != std::string::npos)
        // => the last (seperating) dot was found
        fileExtension = filename.substr(dotPosition+1);
    else
        return false;

    if (fileExtension == "tfig")
        return loadTfig(filename);
    else
        return false;
}

bool TransFuncIntensityGradient::loadTfig(const std::string& filename) {
    TiXmlDocument doc(filename);
    if (!doc.LoadFile())
        return false;

    TiXmlHandle hDoc(&doc);

    TiXmlElement* hRoot = hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it does
    if (!hRoot)
        return false;

    // clear transfer function
    clear();

    loadPrimitivesFromXml(hRoot);

    return true;
}

void TransFuncIntensityGradient::loadPrimitivesFromXml(TiXmlElement* elem) {
    TiXmlElement* primitive = elem->FirstChildElement("quad");
    for (; primitive; primitive = primitive->NextSiblingElement("quad")) {
        TransFuncQuad* quad = new TransFuncQuad();
        quad->updateFromXml(primitive);
        addPrimitive(quad);
    }

    primitive = elem->FirstChildElement("banana");
    for (; primitive; primitive = primitive->NextSiblingElement("banana")) {
        TransFuncBanana* b = new TransFuncBanana();
        b->updateFromXml(primitive);
        addPrimitive(b);
    }
}

void TransFuncIntensityGradient::addPrimitive(TransFuncPrimitive* p) {
    primitives_.push_back(p);
}

void TransFuncIntensityGradient::removePrimitive(TransFuncPrimitive* p) {
    std::vector<TransFuncPrimitive*>::iterator it;
    for (it = primitives_.begin(); it != primitives_.end(); ++it) {
        if (*it == p) {
            delete *it;
            primitives_.erase(it);
            break;
        }
    }
}

int TransFuncIntensityGradient::getWidth() const {
    return tex_->getWidth();
}

int TransFuncIntensityGradient::getHeight() const {
    return tex_->getHeight();
}

void TransFuncIntensityGradient::setPixelData(GLfloat* data) {
    if (tex_->getPixelData() != reinterpret_cast<GLubyte*>(data))
        delete[] tex_->getPixelData();
    tex_->setPixelData(reinterpret_cast<GLubyte*>(data));
    tex_->setBpp(sizeof(GLfloat)*4);
    tex_->uploadTexture();
    textureUpdateNeeded_ = false;
}

const GLfloat* TransFuncIntensityGradient::getPixelData() const {
    return reinterpret_cast<const GLfloat*>(tex_->getPixelData());
}

GLfloat* TransFuncIntensityGradient::getPixelData() {
    return reinterpret_cast<GLfloat*>(tex_->getPixelData());
}

void TransFuncIntensityGradient::clear() {
    std::vector<TransFuncPrimitive*>::iterator it;
    while (!primitives_.empty()) {
        it = primitives_.begin();
        delete *it;
        primitives_.erase(it);
    }

    textureUpdateNeeded_ = true;
}

void TransFuncIntensityGradient::updateTexture() {

    if (!tex_ || (tex_->getDimensions().xy() != dimension_))
        createTex();

    // save current viewport, e.g. size of mapping canvas and current drawbuffer
    glPushAttrib(GL_VIEWPORT_BIT);
    // set viewport to the dimensions of the texture
    glViewport(0, 0, tex_->getWidth(), tex_->getHeight());

    // clear previous content
    glClear(GL_COLOR_BUFFER_BIT);

    // set correct projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    // paint primitives
    paint();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // restore viewport
    glPopAttrib();

    // read tf data from framebuffer and copy it into tf texture
    GLfloat* pixels = new GLfloat[getWidth()*getHeight()*4];
    glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_FLOAT, pixels); 
    setPixelData(pixels); 

    textureUpdateNeeded_ = false;
}

void TransFuncIntensityGradient::paint() {
    for (size_t i = 0; i < primitives_.size(); ++i)
        primitives_[i]->paint();
}

void TransFuncIntensityGradient::paintForSelection() {
    for (size_t i = 0; i < primitives_.size(); ++i)
        primitives_[i]->paintForSelection(i);
}

void TransFuncIntensityGradient::paintInEditor() {
    for (size_t i = 0; i < primitives_.size(); ++i)
        primitives_[i]->paintInEditor();
}

TransFuncPrimitive* TransFuncIntensityGradient::getPrimitive(int i) {
    if ((i < 0) || (i >= static_cast<int>(primitives_.size())))
        return 0;
    else
        return primitives_[i];
}

TransFuncPrimitive* TransFuncIntensityGradient::getPrimitiveForClickedControlPoint(tgt::vec2 pos) {
    if (primitives_.empty())
        return 0;

    int min = 0;
    // A distance of 2 can never happen because the canvas is normalized to [0,1]x[0,1],
    // so this value is huge enough.
    float mindist = 2.f;
    float d;
    for (size_t i = 0; i < primitives_.size(); ++i) {
         d = primitives_[i]->getClosestControlPointDist(pos);
         if ((d < mindist) && (d < primitives_[i]->getControlPointSize())) {
            mindist = d;
            min = i;
         }
    }
    if (mindist == 2.f)
        return 0;
    else
        return primitives_[min];
}

const std::string TransFuncIntensityGradient::getShaderDefines() const {
    return "#define TF_SAMPLER_TYPE " + getSamplerType() + "\n";
}

const std::string TransFuncIntensityGradient::getSamplerType() const {
    return "sampler2D";
}

} // namespace voreen
