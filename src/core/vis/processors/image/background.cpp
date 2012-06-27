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

#include "voreen/core/vis/processors/image/background.h"

#include "tgt/texturemanager.h"

namespace voreen {

const Identifier Background::setBackgroundFirstColor_("set.backgroundFirstColor");
const Identifier Background::setBackgroundSecondColor_("set.backgroundSecondColor");
const Identifier Background::setBackgroundAngle_("set.backgroundAngle");

const Identifier Background::shadeTexUnit0_ = "shadeTex0";
const Identifier Background::depthTexUnit0_ = "depthTex0";
const Identifier Background::shadeTexUnit1_ = "shadeTex1";
const Identifier Background::depthTexUnit1_ = "depthTex1";

Background::Background()
    : ImageProcessor("pp_combine")
    , firstcolor_(setBackgroundFirstColor_, "first color",
                  tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , secondcolor_(setBackgroundSecondColor_, "second color",
                   tgt::vec4(0.2f, 0.2f, 0.2f, 1.0f))
    , angle_(setBackgroundAngle_, "angle", 0, 0, 359, false)
    , tex_(0)
    , textureloaded_(false)
    , filename_("set.backgroundfilenameAsString", "Texture", "Select texture",
                "", "*.jpg;*.bmp;*.png", true, true)
    , tile_("set.backgroundtile", "Repeat Background", 1.0f, 0.f, 100.f, false),
    mode_(MONOCHROME)
{
    setName("Background");

    std::vector<std::string> backmode;
    backmode.push_back("Monochrome");
    backmode.push_back("Gradient");
    backmode.push_back("Radial");
    backmode.push_back("Cloud");
    backmode.push_back("Texture");
    modeProp_ = new EnumProp("set.backgroundModeAsString", "Layout", backmode);
    modeProp_->onChange(CallMemberAction<Background>(this, &Background::setBackgroundModeEvt));
    //condProp_ = new ConditionProp("backgroundModeCond", modeProp_);
    //addProperty(condProp_);
    addProperty(modeProp_);

    std::vector<Identifier> units;
    units.push_back(shadeTexUnit0_);
    units.push_back(depthTexUnit0_);
    units.push_back(shadeTexUnit1_);
    units.push_back(depthTexUnit1_);
    tm_.registerUnits(units);

    /*
    std::vector<int> conds;
    conds.push_back(1);
    conds.push_back(2);
    conds.push_back(3);
    tile_.setConditioned("backgroundModeCond", conds);
    conds.clear();
    conds.push_back(0);
    conds.push_back(1);
    conds.push_back(2);
    */

    // Note: Conditioning does currently not work properly,
    // and will become obsolete with the new property mechanism
    //firstcolor_.setConditioned("backgroundModeCond", conds);
    //secondcolor_.setConditioned("backgroundModeCond", conds);
    //angle_.setConditioned("backgroundModeCond", conds);

    addProperty(&firstcolor_);
    addProperty(&secondcolor_);
    addProperty(&angle_);
    filename_.onChange(CallMemberAction<Background>(this, &Background::loadTexture));
    addProperty(&filename_);
    addProperty(&tile_);

    createInport("image.input");
    createOutport("image.output");
    createPrivatePort("image.tmp");
}

Background::~Background() {
    if (tex_) {
        if (textureloaded_)
            TexMgr.dispose(tex_);
        else
            delete tex_;

        LGL_ERROR;
    }

    delete modeProp_;
    //delete condProp_;
}

const Identifier Background::getClassName() const {
    return "ImageProcessor.Background";
}

const std::string Background::getProcessorInfo() const {
    return "Creates special backgrounds like monochrome background by using the current background color, "
        "color gradient, radial gradient, procedural clouds or an image.";
}

Processor* Background::create() const {
    return new Background();
}

int Background::initializeGL() {
    loadTexture();
    return ImageProcessor::initializeGL();
}

std::string Background::generateHeader() {
    std::string header = ImageProcessor::generateHeader();
    header += "#define COMBINE_ALPHA_COMPOSITING\n";
    header += "#line 1\n";

    return header;
}

void Background::process(LocalPortMapping* portMapping) {
    int source = portMapping->getTarget("image.input");
    int dest = portMapping->getTarget("image.output");
    int tmpBckgrnd = portMapping->getTarget("image.tmp");

    if (source == -1)
        tc_->setActiveTarget(dest, "Background::process()"); // render directly into destination
    else
        tc_->setActiveTarget(tmpBckgrnd, "Background::process() tmp for background"); // render first in tmp target

    compileShader();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearDepth(1.0);

    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // first the background
    renderBackground();

    glEnable(GL_DEPTH_TEST);

    // leave if there's nothing to render above
    if (source != -1) {
        tc_->setActiveTarget(dest, "Background::process()"); // now blend src and tmp into dest
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use pp_combine.frag to blend the orginal picture over the background
        // and to keep the depth-values

        glActiveTexture(tm_.getGLTexUnit(shadeTexUnit0_));
        glBindTexture(tc_->getGLTexTarget(tmpBckgrnd), tc_->getGLTexID(tmpBckgrnd));
        glActiveTexture(tm_.getGLTexUnit(depthTexUnit0_));
        glBindTexture(tc_->getGLDepthTexTarget(tmpBckgrnd), tc_->getGLDepthTexID(tmpBckgrnd));
        LGL_ERROR;
        glActiveTexture(tm_.getGLTexUnit(shadeTexUnit1_));
        glBindTexture(tc_->getGLTexTarget(source), tc_->getGLTexID(source));
        glActiveTexture(tm_.getGLTexUnit(depthTexUnit1_));
        glBindTexture(tc_->getGLDepthTexTarget(source), tc_->getGLDepthTexID(source));
        LGL_ERROR;

        // initialize shader
        program_->activate();
        setGlobalShaderParameters(program_);
        program_->setUniform("shadeTex0_", static_cast<GLint>(tm_.getTexUnit(shadeTexUnit0_)));
        program_->setUniform("depthTex0_", static_cast<GLint>(tm_.getTexUnit(depthTexUnit0_)));
        program_->setUniform("shadeTex1_", static_cast<GLint>(tm_.getTexUnit(shadeTexUnit1_)));
        program_->setUniform("depthTex1_", static_cast<GLint>(tm_.getTexUnit(depthTexUnit1_)));

        glDepthFunc(GL_ALWAYS);
        renderQuad();
        glDepthFunc(GL_LESS);

        program_->deactivate();
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
    }
    LGL_ERROR;
}

void Background::renderBackground() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    switch(mode_) {
        case MONOCHROME:
            glPushAttrib(GL_COLOR_BUFFER_BIT);
            glClearColor(firstcolor_.get().r, firstcolor_.get().g,
                         firstcolor_.get().b, firstcolor_.get().a);
            glClear(GL_COLOR_BUFFER_BIT);
            glPopAttrib();
            break;

        case RADIAL:
            glBegin(GL_QUADS);
            glColor4fv(&secondcolor_.get().elem[0]);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f( 1.0f, -1.0f);
            glVertex2f( 1.0f,  1.0f);
            glVertex2f(-1.0f,  1.0f);
            glEnd();

            glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
            tex_->bind();
            LGL_ERROR;
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_TEXTURE_2D);

            glPushMatrix();
            glScalef(1.44f, 1.44f, 0.0f);

            glBegin(GL_QUADS);
            glColor4fv(&firstcolor_.get().elem[0]);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f( -1.0f, -1.0f);
            glTexCoord2f(tile_.get(), 0.0f);
            glVertex2f( 1.0f, -1.0f);
            glTexCoord2f(tile_.get(), tile_.get());
            glVertex2f( 1.0f, 1.0f);
            glTexCoord2f(0.0f, tile_.get());
            glVertex2f( -1.0f, 1.0f);
            glEnd();

            glPopMatrix();

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            break;

        default:
            glPushMatrix();
            glLoadIdentity();
            glRotatef(static_cast<float>(angle_.get()), 0.0f, 0.0f, 1.0f);

            // when you rotate the texture, you need to scale it.
            // otherwise the edges doesn't cover the complete screen
            // magic number: 0.8284271247461900976033774484194f = sqrt(8)-2;
            glScalef(1.0f + (45 - abs(angle_.get() % 90 - 45)) / 45.0f*0.8284271247461900976033774484194f,
                     1.0f + (45 - abs(angle_.get() % 90 - 45)) / 45.0f*0.8284271247461900976033774484194f, 1.0f);

            glBegin(GL_QUADS);
            glColor4fv(&firstcolor_.get().elem[0]);
            glVertex2f(-1.0, -1.0);
            glVertex2f( 1.0, -1.0);
            glColor4fv(&secondcolor_.get().elem[0]);
            glVertex2f( 1.0, 1.0);
            glVertex2f(-1.0, 1.0);
            glEnd();
            glPopMatrix();
            break;
    }

    // clouds and textures over the gradient-screen
    if ( ( mode_ == CLOUD || mode_ == TEXTURE ) && tex_ ) {
        glActiveTexture(TexUnitMapper::getGLTexUnitFromInt(0));
        tex_->bind();
        tex_->enable();
        LGL_ERROR;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(tile_.get(), 0.0f);
        glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(tile_.get(), tile_.get());
        glVertex2f( 1.0f, 1.0f);
        glTexCoord2f(0.0f, tile_.get());
        glVertex2f(-1.0f, 1.0f);
        glEnd();

        glDisable(GL_BLEND);

        tex_->disable();
    }
}

void Background::setBackgroundModeEvt() {
    if (modeProp_ == 0)
        return;

    const int mode = modeProp_->get();
    switch(mode) {
        case 0:
            mode_ = MONOCHROME;
            break;
        case 1:
            mode_ = GRADIENT;
            break;
        case 2:
            mode_ = RADIAL;
            break;
        case 3:
            mode_ = CLOUD;
            firstcolor_.set(tgt::vec4(0.0f, 0.0f, 0.8f, 1.0f));
            secondcolor_.set(tgt::vec4(0.7f, 0.7f, 1.0f, 1.0f));
            angle_.set(200);
            break;
        case 4:
            mode_ = TEXTURE;
            break;
    }

    loadTexture();
    invalidate();
}

void Background::loadTexture() {
    // is a texture already loaded? -> then delete
    if (tex_) {
        if (textureloaded_) {
            TexMgr.dispose(tex_);
            textureloaded_ = false;
        }
        else
            delete tex_;

        LGL_ERROR;
        tex_ = 0;
    }

    // create Texture
    switch (mode_) {
    case CLOUD:
        createCloudTexture();
        break;

    case RADIAL:
        createRadialTexture();
        break;

    case TEXTURE:
        if (!filename_.get().empty()) {
            tex_ = TexMgr.load(filename_.get(), tgt::Texture::LINEAR, false, false, true, true,
                               (tc_->getTextureContainerTextureType() == TextureContainer::VRN_TEXTURE_RECTANGLE));
        }
        if (tex_)
            textureloaded_ = true;
        break;

    default:
        break;
    }
}

GLubyte* Background::blur(GLubyte* image, int size) {
    GLubyte* n = new GLubyte[size * size];

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int pos = x + y * size;
            n[pos] = image[pos] / 5;

            if (x == 0)
                n[pos] += image[pos + (size - 1)] / 5;
            else
                n[pos] += image[pos - 1] / 5;

            if (y == 0)
                n[pos] += image[pos + (size - 1) * size] / 5;
            else
                n[pos] += image[pos - size] / 5;

            if (x == size-1)
                n[pos] += image[y * size] / 5;
            else
                n[pos] += image[pos + 1] / 5;

            if (y == size-1)
                n[pos] += image[x] / 5;
            else
                n[pos] +=image[pos + size] / 5;
        }
    }
    delete[] image;
    return n;
}

GLubyte* Background::resize(GLubyte* image, int size) {
    GLubyte* n = new GLubyte[4 * size * size];

    for (int y = 0; y < size*2; y++)
        for (int x = 0; x < size*2; x++)
            n[x + y*size*2] = image[(x / 2) + (y / 2) * size];

    delete[] image;

    return blur(blur(n, size*2), size*2);
}

GLubyte* Background::tile(GLubyte* image, int size) {
    GLubyte* n = new GLubyte[4 * size * size];

    for (int y = 0; y < size*2; y++)
        for (int x = 0; x < size*2; x++)
            n[x + y*size*2] = image[(x % size) + (y % size) * size];

    delete[] image;

    return n;
}

void Background::createCloudTexture() {
    int imgx = 256;
    int imgy = 256;
    int size = 32;

    // creates 4 blured and resized noise-datas
    GLubyte* tex1 = new GLubyte[size * size];

    for (int k = 0; k < size*size; k++)
        tex1[k] = rand() % 256;

    tex1 = tile(tile(tile(blur(tex1, size), size), size * 2), size * 4);
    GLubyte* tex2 = new GLubyte[size * size];

    for (int k = 0; k < size*size; k++)
        tex2[k] = rand() % 256;

    tex2 = tile(tile(resize(blur(tex2, size), size), size * 2), size * 4);
    GLubyte* tex3 = new GLubyte[size * size];

    for (int k = 0; k < size*size; k++) {
        tex3[k] = rand() % 256;
    }

    tex3 = tile(resize(resize(blur(tex3, size), size), size * 2), size * 4);
    GLubyte* tex4 = new GLubyte[size * size];

    for (int k = 0; k < size*size; k++)
        tex4[k] = rand() % 256;

    tex4 = resize(resize(resize(blur(tex4, size), size), size * 2), size * 4);

    // creates the final cloud data with the 4 textures from above
    GLubyte* tex5 = new GLubyte[imgx * imgy];

    for (int y = 0; y < imgy; y++) {
        for (int x = 0; x < imgx; x++) {
            int pos = x + imgx * y;
            // mix textures and don't use all values, so that there are free areas in the sky
            float c = (tex4[pos] + tex3[pos] / 2 + tex2[pos] / 4 + tex1[pos] / 8) / 2 - 110.0f;
            if (c < 0)
                c = 0;
            // exp-function to calc the final value for a nicer look
            tex5[pos] = (GLubyte)(255 - (pow(0.95f, c) * 255));
        }
    }

    tex5 = blur(tex5, imgx);

    tex_ = new tgt::Texture(tgt::ivec3(imgx, imgy, 1));

    for (int y = 0; y < imgy; y++) {
        for (int x = 0; x < imgx; x++) {
            tex_->getPixelData()[x*4 + imgx*4*y + 0] = 255;
            tex_->getPixelData()[x*4 + imgx*4*y + 1] = 255;
            tex_->getPixelData()[x*4 + imgx*4*y + 2] = 255;
            tex_->getPixelData()[x*4 + imgx*4*y + 3] = tex5[x + imgx * y];
        }
    }

    delete[] tex1;
    delete[] tex2;
    delete[] tex3;
    delete[] tex4;
    delete[] tex5;

    tex_->bind();
    tex_->uploadTexture();
    LGL_ERROR;
}

void Background::createRadialTexture() {
    int imgx = 256;
    int imgy = 256;
    tex_ = new tgt::Texture(tgt::ivec3(imgx, imgy, 1));

    //create pixeldata:
    GLubyte a = 255;
    double r;

    for (int y = 0; y < imgy; y++) {
        for (int x = 0; x < imgx; x++) {
            // calculate radius (Pythagoras)
            r = static_cast<double>((x - imgx / 2) * (x - imgx / 2) + (y - imgy / 2) * (y - imgy / 2));
            r = sqrt(r);
            // norm to half size of smaller value
            r = 2 * r / ((imgx < imgy) ? imgx : imgy);
            // crop margin
            r = (r > 1) ? 1 : r;
            tex_->getPixelData()[x*4 + imgx*4*y + 0] = 255;
            tex_->getPixelData()[x*4 + imgx*4*y + 1] = 255;
            tex_->getPixelData()[x*4 + imgx*4*y + 2] = 255;
            // invert and multiply with intensity
            tex_->getPixelData()[x*4 + imgx*4*y + 3] = static_cast<GLubyte>(a * (1 - r));
        }
    }

    tex_->bind();
    tex_->uploadTexture();
    LGL_ERROR;
}

} //namespace voreen
