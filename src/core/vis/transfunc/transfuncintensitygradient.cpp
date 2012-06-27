/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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


#include "fboClass/framebufferObject.h"

#include "tinyxml/tinyxml.h"

#include "tgt/tgt_gl.h"
#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"
#include "tgt/texturemanager.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

using namespace tgt;


namespace voreen {

const std::string TransFuncIntensityGradient::loggerCat_("voreen.vis.TransFunc.IntensityGradient");

TransFuncIntensityGradient::TransFuncIntensityGradient(int width, int height) {

    tgtAssert(width > 0 && height > 0, "Invalid dimensions");
    width = std::min(width, GpuCaps.getMaxTextureSize());
    height = std::min(height, GpuCaps.getMaxTextureSize());

    tex_ = new tgt::Texture();
    tex_->generateId();

    tex_->setType(GL_TEXTURE_2D);
    tex_->setDimensions(tgt::ivec3(width,height,1));
    tex_->setBpp(4);
    tex_->setFormat(GL_RGBA);
    tex_->setInternalFormat(GL_RGBA8);
    tex_->setDataType(GL_FLOAT);
    tex_->setPixelData(0);

    tex_->bind();
    tex_->setFilter(tgt::Texture::LINEAR);
    tex_->setWrapping(tgt::Texture::CLAMP);
    tex_->uploadTexture();

}

TransFuncIntensityGradient::~TransFuncIntensityGradient() {
}

std::string TransFuncIntensityGradient::getShaderDefines() {
    return "#define TF_INTENSITY_GRADIENT\n";
}

void TransFuncIntensityGradient::setPixelData(GLfloat* data) {
    delete[] tex_->getPixelData();
    tex_->setPixelData(reinterpret_cast<GLubyte*>(data));
    tex_->setBpp(sizeof(GLfloat)*4);
    updateTexture();
}

const GLfloat* TransFuncIntensityGradient::getPixelData() const {
    return reinterpret_cast<const GLfloat*>(tex_->getPixelData());
}

GLfloat* TransFuncIntensityGradient::getPixelData() {
    return reinterpret_cast<GLfloat*>(tex_->getPixelData());
}

int TransFuncIntensityGradient::getWidth() const {
    return tex_->getWidth();
}

int TransFuncIntensityGradient::getHeight() const {
    return tex_->getHeight();
}
    
void TransFuncIntensityGradient::clear() {
    GLfloat* newData = new GLfloat[getWidth()*getHeight()*4];
    for (int i=0; i<getWidth()*getHeight()*4; ++i)
        newData[i] = 0.f;

    setPixelData(newData);
}

#ifdef VRN_WITH_DEVIL
void TransFuncIntensityGradient::savePNG(const std::string& fname) const {
    ilEnable(IL_FILE_OVERWRITE);
    //download texture and save as png:
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);

    GLubyte* im = new GLubyte[tex_->getWidth()*tex_->getHeight()*4];

    tex_->bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, im);

    ilTexImage(tex_->getWidth(), tex_->getHeight(), 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, im);
    ilSave(IL_PNG, (ILstring)fname.c_str());

    ilDeleteImages(1, &img);
    delete[] im;
}
#else
void TransFuncIntensityGradient::savePNG(const std::string& /*fname*/) const {
    LERROR("Error saving TF as PNG: No devil support.");
}
#endif // VRN_WITH_DEVIL
        
void TransFuncIntensityGradient::updateTexture() {
    tex_->uploadTexture();
}

//-----------------------------------------------------------------------------

const std::string TransFuncIntensityGradientPrimitiveContainer::loggerCat_("voreen.vis.TransFunc.IntensityGradientPrimitiveContainer");

TransFuncIntensityGradientPrimitiveContainer::TransFuncIntensityGradientPrimitiveContainer(int size) : 
    TransFuncIntensityGradient(size, size),
    fbo_(0) 
{
    if ( !GpuCaps.areFramebufferObjectsSupported() )
        fbo_ = 0;
    else {
        fbo_ = new FramebufferObject();
        fbo_->Bind(); // Bind framebuffer object.

        // Attach texture to framebuffer color buffer
        fbo_->AttachTexture(GL_TEXTURE_2D, getTexture()->getId(), GL_COLOR_ATTACHMENT0_EXT);

        // Validate the FBO after attaching textures and render buffers
        if (fbo_->IsValid()) {
            LDEBUG("fbo valid!");
        }
        else {
            LERROR("fbo invalid!" <<  getTexture()->getId());
        }
        // Disable FBO rendering for now...
        FramebufferObject::Disable();
    }
}

TransFuncIntensityGradientPrimitiveContainer::~TransFuncIntensityGradientPrimitiveContainer() {
    if (fbo_)
        delete fbo_;
}

std::string TransFuncIntensityGradientPrimitiveContainer::getShaderDefines() {
    return TransFuncIntensityGradient::getShaderDefines();
}

void TransFuncIntensityGradientPrimitiveContainer::paint() {
    if (primitives_.empty())
        return;
    for (size_t i=0; i<primitives_.size(); ++i)
        primitives_[i]->paint();
}

void TransFuncIntensityGradientPrimitiveContainer::paintSelection() {
    if (primitives_.empty())
        return;
    for (size_t i=0; i<primitives_.size(); ++i)
        primitives_[i]->paintSelection(i);
}

void TransFuncIntensityGradientPrimitiveContainer::paintInEditor() {
    if (primitives_.empty())
        return;
    for (size_t i=0; i<primitives_.size(); ++i)
        primitives_[i]->paintInEditor();
}

TransFuncPrimitive* TransFuncIntensityGradientPrimitiveContainer::getControlPointUnderMouse(tgt::vec2 m) {
    if (primitives_.empty())
        return 0;

	int min = 0;
    float mindist = 2.0f;
    float d;
    for (size_t i=0; i<primitives_.size(); ++i) {
         d = primitives_[i]->getClosestControlPointDist(m);
         if (d < mindist) {
            mindist = d;
            min = i;
         }
    }
    return primitives_[min];
}

void TransFuncIntensityGradientPrimitiveContainer::save(const std::string& fname) {
    TiXmlDocument doc;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement* root = new TiXmlElement( "transferfunc2d" );
    doc.LinkEndChild( root );

    for (size_t i=0; i<primitives_.size(); ++i)
         primitives_[i]->save(root);
    
    doc.SaveFile(fname);
}

bool TransFuncIntensityGradientPrimitiveContainer::load(const std::string& fname) {
    TiXmlDocument doc(fname);
    if (!doc.LoadFile())
        return false;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlElement* hRoot;

    hRoot=hDoc.FirstChildElement().Element();
    // should always have a valid root but handle gracefully if it does
    if (!hRoot)
        return false;

    pElem=hRoot->FirstChildElement("quad");
    for (; pElem; pElem=pElem->NextSiblingElement("quad")) {
        TransFuncQuad* q = new TransFuncQuad(tgt::vec2(0.5f, 0.5f), 0.25f, tgt::col4(255, 0, 0, 128));
        q->load(pElem);
        addPrimitive(q);
    }

    pElem=hRoot->FirstChildElement("banana");
    for (; pElem; pElem=pElem->NextSiblingElement("banana")) {
        TransFuncBanana* b = new TransFuncBanana(tgt::vec2(0.f, 0.f ), tgt::vec2(0.5f, 0.6f),
                                                 tgt::vec2(0.34f, 0.4f), tgt::vec2(0.45f, 0.f),
                                                 tgt::col4(0, 255, 0, 128));
        b->load(pElem);
        addPrimitive(b);
    }
    return true;
}

void TransFuncIntensityGradientPrimitiveContainer::removePrimitive(TransFuncPrimitive* p) {
    for (std::vector<TransFuncPrimitive*>::iterator it = primitives_.begin();
         it != primitives_.end();
         ++it)
    {
        if (*it == p) {
            delete *it;
            primitives_.erase(it);
            return;
        }
    }
}

void TransFuncIntensityGradientPrimitiveContainer::clear() {
    std::vector<TransFuncPrimitive*>::iterator it;
    while (!primitives_.empty()) {
        it = primitives_.begin();
        delete *it;
        primitives_.erase(it);
    }
}

void TransFuncIntensityGradientPrimitiveContainer::updateTexture() {
    fbo_->Bind();                                    // Render to the FBO
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);          // Draw into the first texture

    // store the window viewport dimensions so we can reset them,
    // and set the viewport to the dimensions of our texture
//     int vp[4];
//     glGetIntegerv(GL_VIEWPORT, vp);

    // GPGPU CONCEPT 3a: One-to-one Pixel to Texel Mapping: A Data-
    //                   Dimensioned Viewport.
    // We need a one-to-one mapping of pixels to texels in order to
    // ensure every element of our texture is processed. By setting our
    // viewport to the dimensions of our destination texture and drawing
    // a screen-sized quad (see below), we ensure that every pixel of our
    // texel is generated and processed in the fragment program.
    glViewport(0, 0, getTexture()->getWidth(), getTexture()->getHeight());

    // Render a teapot and 3 tori
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    //paint
    paint();

    FramebufferObject::Disable();
}

//-----------------------------------------------------------------------------

void TransFuncPrimitive::paintControlPoint(const tgt::vec2& v) {
    glTranslatef(v.x,v.y,0.1f);
    float radius = cpSize_;
    tgt::vec2 t;
    glDisable(GL_BLEND);
    
    glBegin(GL_POLYGON);
    glColor4ub(150,150,150,255);
    for (int i=0; i<20; ++i) {
        t.x = cosf((i/20.0f)*2.0f*PIf)*radius;
        t.y = sinf((i/20.0f)*2.0f*PIf)*radius;
        vertex(t);
    }
    glEnd();
    
    glTranslatef(0.0f,0.0f, 0.1f);

    glBegin(GL_LINE_LOOP);
    if (selected_)
        glColor4ub(255,255,255,255);
    else
        glColor4ub(128,128,128,255);
    for (int i=0; i<20; ++i) {
        t.x = cosf((i/20.0f)*2.0f*PIf)*radius;
        t.y = sinf((i/20.0f)*2.0f*PIf)*radius;
        vertex(t);
    }
    glEnd();
    
    glEnable(GL_BLEND);
    glTranslatef(-v.x,-v.y,-0.2f);
}

//-----------------------------------------------------------------------------

TransFuncQuad::TransFuncQuad(tgt::vec2 center, float size, tgt::col4 col) : TransFuncPrimitive(col) {
    size *= 0.5f;
    coords_[0] = center + vec2(-size, -size);
    coords_[1] = center + vec2(size, -size);
    coords_[2] = center + vec2(size, size);
    coords_[3] = center + vec2(-size, size);
    grabbed_ = -1;

    fuzziness_ = 1.0f;
}


void TransFuncQuad::paint() {
    vec2 center;
    center = coords_[0]+coords_[1]+coords_[2]+coords_[3];
    center /= 4.0f;

    glTranslatef(0.0f,0.0f,-0.5f);
    glBegin(GL_QUADS);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(coords_[0]);
        vertex(coords_[1]);
        glColor4ubv(color_.elem);
        vertex(fuzziness_*coords_[1] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[0] + (1.0f-fuzziness_)*center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(coords_[1]);
        vertex(coords_[2]);
        glColor4ubv(color_.elem);
        vertex(fuzziness_*coords_[2] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[1] + (1.0f-fuzziness_)*center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(coords_[2]);
        vertex(coords_[3]);
        glColor4ubv(color_.elem);
        vertex(fuzziness_*coords_[3] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[2] + (1.0f-fuzziness_)*center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(coords_[3]);
        vertex(coords_[0]);
        glColor4ubv(color_.elem);
        vertex(fuzziness_*coords_[0] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[3] + (1.0f-fuzziness_)*center);

        glColor4ubv(color_.elem);
        vertex(fuzziness_*coords_[0] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[1] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[2] + (1.0f-fuzziness_)*center);
        vertex(fuzziness_*coords_[3] + (1.0f-fuzziness_)*center);
    glEnd();
    glTranslatef(0.0f,0.0f,0.5f);

//     glTranslatef(0.0f,0.0f,-0.5f);
//     glBegin(GL_QUADS);
//         glColor4ubv(color_.elem);
//         vertex(coords_[0]);
//         vertex(coords_[1]);
//         vertex(coords_[2]);
//         vertex(coords_[3]);
//     glEnd();
//     glTranslatef(0.0f,0.0f,0.5f);
}

void TransFuncQuad::paintSelection(GLubyte id) {
    glBegin(GL_QUADS);
        glColor3ub(id,123,123);
        vertex(coords_[0]);
        vertex(coords_[1]);
        vertex(coords_[2]);
        vertex(coords_[3]);
    glEnd();
}

void TransFuncQuad::paintInEditor() {
    paint();

    glBegin(GL_LINE_LOOP);
        if (selected_)
            glColor4ub(255,255,255,255);
        else
            glColor4ub(128,128,128,255);
        vertex(coords_[0]);
        vertex(coords_[1]);
        vertex(coords_[2]);
        vertex(coords_[3]);
    glEnd();

    paintControlPoint(coords_[0]);
    paintControlPoint(coords_[1]);
    paintControlPoint(coords_[2]);
    paintControlPoint(coords_[3]);

}

float TransFuncQuad::getClosestControlPointDist(tgt::vec2 m) {
    float min = distance(m, coords_[0]);
    float d;
    for (int i = 1; i<4; ++i) {
        d = distance(m, coords_[i]);
        if (d < min)
            min = d;
    }
    return min;
}

bool TransFuncQuad::mousePress(tgt::vec2 m) {
    grabbed_ = -1;
    int n = 0;
    float min = distance(m, coords_[0]);
    float d;
    for (int i = 0; i<4; ++i) {
        d = distance(m, coords_[i]);
        if (d < min) {
            min = d;
            n = i;
        }
    }
    if (min < cpSize_) {
        grabbed_ = n;
        return true;
    }
    return false;
}

void TransFuncQuad::save(TiXmlElement* root) {
    TiXmlElement* e = new TiXmlElement( "quad" );
    for (int i = 0; i<4; ++i)
        TransFunc::saveXml(e, coords_[i]);
    
        TransFunc::saveXml(e, color_);
    e->SetDoubleAttribute("fuzzy", fuzziness_);
    root->LinkEndChild(e);
}

void TransFuncQuad::load(TiXmlElement* root) {
    double temp;
    if (root->Attribute("fuzzy", &temp))
        fuzziness_ = static_cast<float>(temp);
    else
        fuzziness_ = 1.0f;

    TiXmlElement* pElem;
    pElem = root->FirstChild("vec2")->ToElement();
    int i = 0;
    for (; pElem && i<4; pElem=pElem->NextSiblingElement("vec2")) {
        TransFunc::loadXml(pElem, coords_[i]);
        i++;
    }
    pElem = root->FirstChild("col4")->ToElement();
    if (pElem)
        TransFunc::loadXml(pElem, color_);
}

void TransFuncQuad::mouseDrag(tgt::vec2 offset) {
    if (grabbed_ > -1)
        coords_[grabbed_] += offset;
    else {
        for (int i = 0; i<4; ++i)
            coords_[i] += offset;
    }
}

//-----------------------------------------------------------------------------

TransFuncBanana::TransFuncBanana(tgt::vec2 a, tgt::vec2 b1, tgt::vec2 b2, tgt::vec2 c, tgt::col4 col) : TransFuncPrimitive(col) {
    coords_[0] = a;
    coords_[1] = b1;
    coords_[2] = b2;
    coords_[3] = c;
    steps_ = 20;
    grabbed_ = -1;;

    fuzziness_ = 1.0f;
}

void TransFuncBanana::paint() {
    glTranslatef(0.0f,0.0f,-0.5f);
    glColor4ubv(color_.elem);
    paintInner();
    glTranslatef(0.0f,0.0f,0.5f);
}

void TransFuncBanana::paintSelection(GLubyte id) {
    glColor3ub(id,123,123);
    float t;
    tgt::vec2 v1, v2, t1, t2, t3, t4, tc;
    t1 = (2.0f*coords_[1])-(0.5f*coords_[0])-(0.5f*coords_[3]);
    t2 = (2.0f*coords_[2])-(0.5f*coords_[0])-(0.5f*coords_[3]);

    //fill the space between the two bezier curves:
    glBegin(GL_TRIANGLE_STRIP);
    vertex(coords_[0]);
    for (int i=0; i<steps_; ++i) {
        t = i/static_cast<float>(steps_-1);
        v1 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t1)+((t*t)*coords_[3]);
        v2 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t2)+((t*t)*coords_[3]);
        vertex(v1);
        vertex(v2);
    }
    vertex(coords_[3]);
    glEnd();
}

void TransFuncBanana::paintInner() {
    float t;
    tgt::vec2 v1, v2, t1, t2, t3, t4, tc;
    t1 = (2.0f*coords_[1])-(0.5f*coords_[0])-(0.5f*coords_[3]);
    t2 = (2.0f*coords_[2])-(0.5f*coords_[0])-(0.5f*coords_[3]);

    tc = (t1+t2)/2.0f;
    t3 = fuzziness_*t1+(1.0f-fuzziness_)*tc;
    t4 = fuzziness_*t2+(1.0f-fuzziness_)*tc;
    //fill the space between the two bezier curves:
    glBegin(GL_TRIANGLE_STRIP);
    glColor4ubv(color_.elem);
    vertex(coords_[0]);
    for (int i=0; i<steps_; ++i) {
        t = i/static_cast<float>(steps_-1);
        v1 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t1)+((t*t)*coords_[3]);
        v2 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t3)+((t*t)*coords_[3]);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(v1);
        glColor4ubv(color_.elem);
        vertex(v2);
    }
    vertex(coords_[3]);

    glColor4ubv(color_.elem);
    vertex(coords_[0]);
    for (int i=0; i<steps_; ++i) {
        t = i/static_cast<float>(steps_-1);
        v1 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t3)+((t*t)*coords_[3]);
        v2 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t4)+((t*t)*coords_[3]);
        vertex(v1);
        vertex(v2);
    }
    vertex(coords_[3]);

    vertex(coords_[0]);
    for (int i=0; i<steps_; ++i) {
        t = i/static_cast<float>(steps_-1);
        v1 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t4)+((t*t)*coords_[3]);
        v2 = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t2)+((t*t)*coords_[3]);
        glColor4ubv(color_.elem);
        vertex(v1);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        vertex(v2);
    }
    vertex(coords_[3]);

    glEnd();
}

void TransFuncBanana::paintInEditor() {
    float t;
    tgt::vec2 v, t1, t2;
    t1 = (2.0f*coords_[1])-(0.5f*coords_[0])-(0.5f*coords_[3]);
    t2 = (2.0f*coords_[2])-(0.5f*coords_[0])-(0.5f*coords_[3]);
    paint();

    //draw outer line of double bezier curve:
    glBegin(GL_LINE_LOOP);
        if (selected_)
            glColor4ub(255,255,255,255);
        else
            glColor4ub(128,128,128,255);
        for (int i=0; i<steps_; ++i) {
            t = i/static_cast<float>(steps_-1);
            v = (((1-t)*(1-t))*coords_[0])+((2*(1-t)*t)*t1)+((t*t)*coords_[3]);
            vertex(v);
        }

        for (int i=0; i<steps_; ++i) {
            t = 1.0f-(i/static_cast<float>(steps_-1));
            v = (((1.0f-t)*(1.0f-t))*coords_[0])+((2.0f*(1.0f-t)*t)*t2)+((t*t)*coords_[3]);
            vertex(v);
        }
     glEnd();

    paintControlPoint(coords_[0]);
    paintControlPoint(coords_[1]);
    paintControlPoint(coords_[2]);
    paintControlPoint(coords_[3]);
}

float TransFuncBanana::getClosestControlPointDist(tgt::vec2 m) {
    float min = distance(m, coords_[0]);
    float d;
    for (int i = 1; i<4; ++i) {
        d = distance(m, coords_[i]);
        if (d < min)
            min = d;
    }
    return min;
}

bool TransFuncBanana::mousePress(tgt::vec2 m) {
    grabbed_ = -1;
    int n = 0;
    float min = distance(m, coords_[0]);
    float d;
    for (int i = 0; i<4; ++i) {
        d = distance(m, coords_[i]);
        if (d < min) {
            min = d;
            n = i;
        }
    }
    if (min < cpSize_) {
        grabbed_ = n;
        return true;
    }
    return false;
}

void TransFuncBanana::save(TiXmlElement* root) {
    TiXmlElement* e = new TiXmlElement( "banana" );
    for (int i = 0; i<4; ++i) {
        TransFunc::saveXml(e, coords_[i]);
    }
    TransFunc::saveXml(e, color_);
    e->SetDoubleAttribute("fuzzy", fuzziness_);
    root->LinkEndChild(e);
}

void TransFuncBanana::load(TiXmlElement* root) {
    double temp;
    if (root->Attribute("fuzzy", &temp))
        fuzziness_ = static_cast<float>(temp);
    else
        fuzziness_ = 1.0f;

    TiXmlElement* pElem;
    pElem = root->FirstChild("vec2")->ToElement();
    int i = 0;
    for ( ; pElem && i<4; pElem=pElem->NextSiblingElement("vec2")) {
        TransFunc::loadXml(pElem, coords_[i]);
        ++i;
    }
    pElem = root->FirstChild("col4")->ToElement();
    if (pElem)
        TransFunc::loadXml(pElem, color_);
}

void TransFuncBanana::mouseDrag(tgt::vec2 offset) {
    if (grabbed_ > -1)
        coords_[grabbed_] += offset;
    else {
        for (int i = 0; i<4; ++i)
            coords_[i] += offset;
    }
}

}  // namespace
