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

#include "voreen/core/datastructures/transfunc/transfuncintensitygradient.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"
#include "voreen/core/datastructures/transfunc/transfuncfactory.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

#include <fstream>

using tgt::Texture;

namespace voreen {

const std::string TransFuncIntensityGradient::loggerCat_("voreen.TransFuncIntensityGradient");

TransFuncIntensityGradient::TransFuncIntensityGradient(int width, int height)
: TransFunc(width, height, 1, GL_RGBA, GL_FLOAT, Texture::LINEAR)
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

    // check type information
    if (dimensions_ != tf.dimensions_ || format_ != tf.format_ || dataType_ != tf.dataType_)
        return false;

    // textures available?
    if (!tex_ || !tf.tex_ || !tex_->getPixelData() || !tf.tex_->getPixelData())
        return false;

    // compare texture data
    return (memcmp(tex_->getPixelData(), tf.tex_->getPixelData(), tgt::hmul(tex_->getDimensions())*tex_->getBpp()) == 0);
}

bool TransFuncIntensityGradient::operator!=(const TransFuncIntensityGradient& tf) {
    return !(*this == tf);
}

void TransFuncIntensityGradient::setScaleFactor(float /*factor*/) {
    //for (size_t i = 0; i < primitives_.size(); ++i)
    //    primitives_[i]->setScaleFactor(factor);
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

    // open file stream
    std::ofstream stream(filename.c_str(), std::ios_base::out);
    if (stream.fail()) {
        LWARNING("Unable to open file " << filename << " for writing.");
        return false;
    }

    // serialize to stream
    bool success = true;
    try {
        XmlSerializer s(filename);
        s.registerFactory(TransFuncFactory::getInstance());
        s.serialize("TransFuncIntensityGradient", this);

        s.write(stream);
        if (stream.bad()) {
            LWARNING("Unable to write to file: " << filename);
            success = false;
        }
        stream.close();
    }
    catch (SerializationException &e) {
        LWARNING("SerializationException: " << e.what());
        stream.close();
        success = false;
    }

    // log result
    if (success)
        LINFO("Saved transfer function to file: " << filename);
    else
        LWARNING("Saving transfer function failed.");

    return success;

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

    // open file stream
    std::ifstream stream(filename.c_str(), std::ios_base::in);
    if (stream.fail()) {
        LWARNING("Unable to open file " << filename << " for reading.");
        return false;
    }

    // deserialize from stream
    bool success = true;
    try {
        XmlDeserializer d(filename);
        d.registerFactory(TransFuncFactory::getInstance());
        d.read(stream);
        d.deserialize("TransFuncIntensityGradient", *this);
        stream.close();
    }
    catch (SerializationException &e) {
        LWARNING("SerializationException: " << e.what());
        stream.close();
        success = false;
    }

    // log result
    if (success)
        LINFO("Loaded transfer function from file: " << filename);
    else
        LWARNING("Loading transfer function failed.");

    return success;
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

void TransFuncIntensityGradient::clear() {
    std::vector<TransFuncPrimitive*>::iterator it;
    while (!primitives_.empty()) {
        it = primitives_.begin();
        delete *it;
        primitives_.erase(it);
    }

    textureInvalid_ = true;
}

void TransFuncIntensityGradient::updateTexture() {

    if (!tex_ || (tex_->getDimensions() != dimensions_))
        createTex();
    tgtAssert(tex_, "No texture");

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
    GLfloat* pixels = new GLfloat[tgt::hmul(dimensions_)*4];
    glReadPixels(0, 0, dimensions_.x, dimensions_.y, GL_RGBA, GL_FLOAT, pixels);
    setPixelData(reinterpret_cast<GLubyte*>(pixels));

    tex_->uploadTexture();
    textureInvalid_ = false;
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

TransFuncPrimitive* TransFuncIntensityGradient::getPrimitive(int i) const {
    if ((i < 0) || (i >= static_cast<int>(primitives_.size())))
        return 0;
    else
        return primitives_[i];
}

TransFuncPrimitive* TransFuncIntensityGradient::getPrimitiveForClickedControlPoint(tgt::vec2 pos) const {
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

void TransFuncIntensityGradient::serialize(XmlSerializer& s) const {
    TransFunc::serialize(s);
    s.serialize("Primitives", primitives_, "Primitive");
}

void TransFuncIntensityGradient::deserialize(XmlDeserializer& s) {
    TransFunc::deserialize(s);
    s.deserialize("Primitives", primitives_, "Primitive");
}


TransFunc* TransFuncIntensityGradient::clone() const {
    TransFuncIntensityGradient* func = new TransFuncIntensityGradient();

    func->dimensions_.x = dimensions_.x;
    func->dimensions_.y = dimensions_.y;
    func->dimensions_.z = dimensions_.z;
    func->format_ = format_;
    func->dataType_ = dataType_;
    func->filter_ = filter_;

    func->primitives_.clear();
    std::vector<TransFuncPrimitive*>::const_iterator it;
    for(it = primitives_.begin(); it!=primitives_.end(); it++) {
        func->primitives_.push_back((*it)->clone());
    }

    func->textureInvalid_ = true;

    return func;
}

} // namespace voreen
