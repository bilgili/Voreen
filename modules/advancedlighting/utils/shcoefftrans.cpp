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

#include "shcoefftrans.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

CoeffVolume::CoeffVolume(tgt::vec3 dimensions, std::string id) {
    volumeGL_ = 0;
    volume_ = new VolumeRAM_4xUInt8(dimensions, false);
    //volume_ = new VolumeRAM_4xUInt16(dimensions, tgt::vec3(1.0), 16, false);
    //volume_ = new VolumeRAM_4xFloat(dimensions, tgt::vec3(1.0), sizeof(float)*8);
    id_ = id;
}

// for debug purposes only
CoeffVolume::CoeffVolume(CoeffVolume* cv) {
    volumeGL_ = 0;
    //cv->getRepresentation<VolumeGL>()->updateTextureFromGPU();
    //volume_ = cv->getRepresentation<VolumeRAM>()->clone();

    tgt::col4* buf = (tgt::col4*)(cv->getVolumeGL()->getTexture()->downloadTextureToBuffer());
    //tgt::Vector4<uint16_t>* buf = (tgt::Vector4<uint16_t>*)(cv->getRepresentation<VolumeGL>()->getTexture()->downloadTextureToBuffer());
    //tgt::vec4* buf = (tgt::vec4*)(cv->getRepresentation<VolumeGL>()->getTexture()->downloadTextureToBuffer());

    volume_ = new VolumeRAM_4xUInt8(buf, cv->getVolume()->getDimensions());
    //volume_ = new VolumeRAM_4xUInt16(buf, cv->getRepresentation<VolumeRAM>()->getDimensions(), tgt::vec3(1.0), 16);
    //volume_ = new VolumeRAM_4xFloat(buf, cv->getRepresentation<VolumeRAM>()->getDimensions(), tgt::vec3(1.0), sizeof(float)*8);
}

CoeffVolume::~CoeffVolume() {
    delete volume_;
    delete volumeGL_;
}

VolumeGL* CoeffVolume::getVolumeGL() {
    if (!volumeGL_) {
        volumeGL_ = new VolumeGL(volume_);
        // necessary only if this is not done with empty textures
        const_cast<VolumeTexture*>(volumeGL_->getTexture())->uploadTexture();
    }
    return volumeGL_;
}

VolumeRAM_4xUInt8* CoeffVolume::getVolume() {
//VolumeRAM_4xUInt16* CoeffVolume::getRepresentation<VolumeRAM>() {
//VolumeRAM_4xFloat* CoeffVolume::getRepresentation<VolumeRAM>() {
    return volume_;
}

// --------------------------------------------------------------------

const std::string SHCoeffTrans::loggerCat_("voreen.SHCoeffTrans");

SHCoeffTrans::SHCoeffTrans(int sampleNumSqrt, int numBands, TransFuncProperty* tfprop,
                           const VolumeBase* vol, BleedingMode bm, float sizefac, bool considerNormals, bool eri, float dataScale)
    : SHCoeffCalc(sampleNumSqrt, numBands)
    , tfProp_(tfprop)
    , volume_(vol)
    , bm_(bm)
    , sizefac_(sizefac)
    , considerNormals_(considerNormals)
    , eri_(eri)
    , dataScale_(dataScale)
    , lightFunc_(0) {

    if(bm_ != SH_UNSHADOWED) {

        if(bm == SH_BLEED_SUB)
            considerNormals_ = true;

        fbo_ = new tgt::FramebufferObject();
        fbo2_ = new tgt::FramebufferObject();
        fbo3_ = new tgt::FramebufferObject();

        transCoeffsI0_ = 0; transCoeffsI1_ = 0; transCoeffsI2_ = 0; transCoeffsI3_ = 0;
        transCoeffsR0_ = 0; transCoeffsR1_ = 0; transCoeffsR2_ = 0; transCoeffsR3_ = 0;
        transCoeffsG0_ = 0; transCoeffsG1_ = 0; transCoeffsG2_ = 0; transCoeffsG3_ = 0;
        transCoeffsB0_ = 0; transCoeffsB1_ = 0; transCoeffsB2_ = 0; transCoeffsB3_ = 0;
        sssCoeffs0_ = 0; sssCoeffs1_ = 0; sssCoeffs2_ = 0; sssCoeffs3_ = 0;
        sssCoeffs4_ = 0; sssCoeffs5_ = 0; sssCoeffs6_ = 0;

        tfVolume_ = 0;
        normVolume_ = 0;

        ShdrMgr.addPath(VoreenApplication::app()->getModulePath("sphericalharmonics"));

        try {
            coeffPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "shcoeffs.frag", generateHeader(), false);
        } catch(tgt::Exception) {
                coeffPrg_ = 0;
        }

        if(bm_ & SH_SUBSURFACE) {
            try {
                coeffPrg2_ = ShdrMgr.loadSeparate("passthrough.vert", "shcoeffssubsurface.frag", generateHeader(), false);
            } catch(tgt::Exception) {
                coeffPrg2_ = 0;
            }
        }
        else if(bm_ == SH_REFLECT) {
            try {
                coeffPrg2_ = ShdrMgr.loadSeparate("passthrough.vert", "shcoeffsreflect.frag", generateHeader(), false);
            } catch(tgt::Exception) {
                coeffPrg2_ = 0;
            }
        }
        else
            coeffPrg2_ = 0;

        try {
            tfPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "tfid.frag", generateHeader(), false);
        } catch(tgt::Exception) {
            tfPrg_ = 0;
        }

        if(considerNormals_) {
            try {
                normPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "norm.frag", generateHeader(), false);
            } catch(tgt::Exception) {
                normPrg_ = 0;
            }
        }
        else
            normPrg_ = 0;
    }
}

SHCoeffTrans::~SHCoeffTrans() {
    if(bm_ != SH_UNSHADOWED) {
        delete transCoeffsI0_; delete transCoeffsI1_; delete transCoeffsI2_; delete transCoeffsI3_;
        delete transCoeffsR0_; delete transCoeffsR1_; delete transCoeffsR2_; delete transCoeffsR3_;
        delete transCoeffsG0_; delete transCoeffsG1_; delete transCoeffsG2_; delete transCoeffsG3_;
        delete transCoeffsB0_; delete transCoeffsB1_; delete transCoeffsB2_; delete transCoeffsB3_;

        delete sssCoeffs0_; delete sssCoeffs1_; delete sssCoeffs2_; delete sssCoeffs3_;
        delete sssCoeffs4_; delete sssCoeffs5_; delete sssCoeffs6_;

        delete tfVolume_;
        delete normVolume_;

        if(tfPrg_)
            ShdrMgr.dispose(tfPrg_);
        if(normPrg_)
            ShdrMgr.dispose(normPrg_);
        if(coeffPrg_)
            ShdrMgr.dispose(coeffPrg_);
        if(coeffPrg2_)
            ShdrMgr.dispose(coeffPrg2_);

        delete fbo_;
        delete fbo2_;
        delete fbo3_;

    }
}

std::string SHCoeffTrans::generateHeader() {
    std::string headerSource = std::string();
    headerSource += tfProp_->get()->getShaderDefines();
    headerSource += getShaderDefines();
    return headerSource;
}

std::string SHCoeffTrans::getShaderDefines() const {
    std::string headerSource = std::string();
    if(bm_ == SH_UNSHADOWED)
        headerSource += "#define SH_UNSHADOWED\n";
    else if(bm_ == SH_SHADOWED)
        headerSource += "#define SH_SHADOWED\n";
    else if(bm_ == SH_BLEEDING)
        headerSource += "#define SH_BLEEDING\n";
    else if(bm_ == SH_REFLECT)
        headerSource += "#define SH_REFLECT\n";
    else if(bm_ == SH_SUBSURFACE)
        headerSource += "#define SH_SUBSURFACE\n";
    else if(bm_ == SH_BLEED_SUB)
        headerSource += "#define SH_BLEED_SUB\n";

    if(considerNormals_)
        headerSource += "#define CONSIDER_NORMALS\n";

    if(eri_)
        headerSource += "#define SH_ERI\n";

    if(lightFunc_)
        headerSource += lightFunc_->getShaderDefines();

    return headerSource;
}

void SHCoeffTrans::init() {

    if(bm_ == SH_UNSHADOWED)
        return;

    LGL_ERROR;
    delete tfVolume_;
    delete normVolume_;
    delete transCoeffsI0_; delete transCoeffsI1_; delete transCoeffsI2_; delete transCoeffsI3_;
    delete transCoeffsR0_; delete transCoeffsR1_; delete transCoeffsR2_; delete transCoeffsR3_;
    delete transCoeffsG0_; delete transCoeffsG1_; delete transCoeffsG2_; delete transCoeffsG3_;
    delete transCoeffsB0_; delete transCoeffsB1_; delete transCoeffsB2_; delete transCoeffsB3_;

    // the coefficient-volumes don't necessarily have to have the size of the original volume.  We can save a lot of
    // computation time with an acceptible loss of quality, especially since we deal with soft-lighting effects here.
    // The computation time sinks linearly with the coefficient-volume-size; I think a size of 1/4th of the orignial still
    // delivers good-looking results.
    float fac = std::pow(sizefac_, 0.3333f);
    tgt::ivec3 dims = tgt::ivec3(tgt::vec3(volume_->getDimensions()) * fac);

    if(!(bm_ & SH_BLEEDING)) {
        transCoeffsI0_ = new CoeffVolume(dims, "shcoeffsI0_");
        transCoeffsI1_ = new CoeffVolume(dims, "shcoeffsI1_");
        transCoeffsI2_ = new CoeffVolume(dims, "shcoeffsI2_");
        transCoeffsI3_ = new CoeffVolume(dims, "shcoeffsI3_");
    }

    if(bm_ == SH_REFLECT) {
        transCoeffsR0_ = new CoeffVolume(dims, "shcoeffsR0_");
        transCoeffsG0_ = new CoeffVolume(dims, "shcoeffsG0_");
        transCoeffsB0_ = new CoeffVolume(dims, "shcoeffsB0_");
    }

    if(bm_ & SH_SUBSURFACE) {
        //tgt::ivec3 sssDims = tgt::ivec3(tgt::vec3(volume_->getRepresentation<VolumeRAM>()->getDimensions()) * float(std::pow(0.15f, 0.3333f)));
        //tgt::ivec3 sssDims = tgt::ivec3(tgt::vec3(volume_->getRepresentation<VolumeRAM>()->getDimensions()) * float(std::pow(0.30, 0.3333f)));
        tgt::ivec3 sssDims = dims;

        sssCoeffs0_ = new CoeffVolume(sssDims, "sssCoeffs0_");
        sssCoeffs1_ = new CoeffVolume(sssDims, "sssCoeffs1_");
        sssCoeffs2_ = new CoeffVolume(sssDims, "sssCoeffs2_");
        sssCoeffs3_ = new CoeffVolume(sssDims, "sssCoeffs3_");
        sssCoeffs4_ = new CoeffVolume(sssDims, "sssCoeffs4_");
        sssCoeffs5_ = new CoeffVolume(sssDims, "sssCoeffs5_");
        sssCoeffs6_ = new CoeffVolume(sssDims, "sssCoeffs6_");
    }

    if(bm_ & SH_BLEEDING) {
        transCoeffsR0_ = new CoeffVolume(dims, "shcoeffsR0_");
        transCoeffsR1_ = new CoeffVolume(dims, "shcoeffsR1_");
        transCoeffsR2_ = new CoeffVolume(dims, "shcoeffsR2_");
        transCoeffsR3_ = new CoeffVolume(dims, "shcoeffsR3_");
        transCoeffsG0_ = new CoeffVolume(dims, "shcoeffsG0_");
        transCoeffsG1_ = new CoeffVolume(dims, "shcoeffsG1_");
        transCoeffsG2_ = new CoeffVolume(dims, "shcoeffsG2_");
        transCoeffsG3_ = new CoeffVolume(dims, "shcoeffsG3_");
        transCoeffsB0_ = new CoeffVolume(dims, "shcoeffsB0_");
        transCoeffsB1_ = new CoeffVolume(dims, "shcoeffsB1_");
        transCoeffsB2_ = new CoeffVolume(dims, "shcoeffsB2_");
        transCoeffsB3_ = new CoeffVolume(dims, "shcoeffsB3_");
    }

    dims = volume_->getRepresentation<VolumeRAM>()->getDimensions();
    tfVolume_ = new CoeffVolume(dims, "tfVolume_");

    if(considerNormals_)
        normVolume_ = new CoeffVolume(dims, "normVolume_");

    TextureUnit transVolUnit;
    transVolUnit.activate();
    const tgt::Texture* tfTex = tfVolume_->getVolumeGL()->getTexture();
    tfTex->enable();
    tfTex->bind();

    int level = 0;
    bool done = false;
    tgt::ivec3 tmp = dims;

    while(!done) {
        //allocate gpu-memory for each mipmap level that we will render into
        glTexImage3D(GL_TEXTURE_3D, level, tfTex->getInternalFormat(), tmp.x, tmp.y, tmp.z, 0, tfTex->getFormat(), tfTex->getDataType(), NULL);
        if(tmp.x == 1 && tmp.y == 1 && tmp.z == 1)
            done = true;
        level++;
        if(tmp.x != 1) tmp.x /= 2;
        if(tmp.y != 1) tmp.y /= 2;
        if(tmp.z != 1) tmp.z /= 2;
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, tgt::ivec4(0).elem);

    tfTex->disable();

    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

void SHCoeffTrans::generateCoeffs() {
    if(bm_ != SH_UNSHADOWED)
        calcCoeffs();
}

void SHCoeffTrans::bindCoeffVolume(tgt::Shader* shader, CoeffVolume* cv, TextureUnit* texUnit) {

    VolumeGL* volumeGL = cv->getVolumeGL();
    const VolumeTexture* volumeTex = volumeGL->getTexture();

    // bind volume texture and pass sampler to the shader
    GLint loc = shader->getUniformLocation(cv->getId());

    if (loc != -1 && texUnit) {

        texUnit->activate();

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, tgt::vec4(0.0f).elem);

        volumeTex->enable();
        volumeTex->bind();

        shader->setUniform(loc, texUnit->getUnitNumber());
        volumeTex->disable();
        TextureUnit::setZeroUnit();
    }
}

void SHCoeffTrans::setRCShaderUniforms(tgt::Shader* rcProg) {
    // deprecated, but might be useful for debugging
    TextureUnit transVolUnit(true);
    bindCoeffVolume(rcProg, tfVolume_, &transVolUnit);
    if(normVolume_) {
        TextureUnit normVolUnit(true);
        bindCoeffVolume(rcProg, normVolume_, &normVolUnit);
    }

    TextureUnit coeffUnit0(true), coeffUnit1(true), coeffUnit2(true), coeffUnit3(true),
                coeffUnit4(true), coeffUnit5(true), coeffUnit6(true), coeffUnit7(true),
                coeffUnit8(true), coeffUnit9(true), coeffUnit10(true), coeffUnit11(true);

    if(bm_ != SH_UNSHADOWED) {
        // add sh-specific volumes
        if(!(bm_ & SH_BLEEDING)) {
            bindCoeffVolume(rcProg, transCoeffsI0_, &coeffUnit0);
            bindCoeffVolume(rcProg, transCoeffsI1_, &coeffUnit1);
            bindCoeffVolume(rcProg, transCoeffsI2_, &coeffUnit2);
            bindCoeffVolume(rcProg, transCoeffsI3_, &coeffUnit3);
        }

        if(bm_ == SH_REFLECT) {
            bindCoeffVolume(rcProg, transCoeffsR0_, &coeffUnit0);
            bindCoeffVolume(rcProg, transCoeffsG0_, &coeffUnit1);
            bindCoeffVolume(rcProg, transCoeffsB0_, &coeffUnit2);
        }

        if(bm_ & SH_SUBSURFACE) {
            bindCoeffVolume(rcProg, sssCoeffs0_, &coeffUnit0);
            bindCoeffVolume(rcProg, sssCoeffs1_, &coeffUnit1);
            bindCoeffVolume(rcProg, sssCoeffs2_, &coeffUnit2);
            bindCoeffVolume(rcProg, sssCoeffs3_, &coeffUnit3);
            bindCoeffVolume(rcProg, sssCoeffs4_, &coeffUnit4);
            bindCoeffVolume(rcProg, sssCoeffs5_, &coeffUnit5);
            bindCoeffVolume(rcProg, sssCoeffs6_, &coeffUnit6);
        }

        if(bm_ == SH_BLEEDING) {
            bindCoeffVolume(rcProg, transCoeffsR0_, &coeffUnit0);
            bindCoeffVolume(rcProg, transCoeffsR1_, &coeffUnit1);
            bindCoeffVolume(rcProg, transCoeffsR2_, &coeffUnit2);
            bindCoeffVolume(rcProg, transCoeffsR3_, &coeffUnit3);
            bindCoeffVolume(rcProg, transCoeffsG0_, &coeffUnit4);
            bindCoeffVolume(rcProg, transCoeffsG1_, &coeffUnit5);
            bindCoeffVolume(rcProg, transCoeffsG2_, &coeffUnit6);
            bindCoeffVolume(rcProg, transCoeffsG3_, &coeffUnit7);
            bindCoeffVolume(rcProg, transCoeffsB0_, &coeffUnit8);
            bindCoeffVolume(rcProg, transCoeffsB1_, &coeffUnit9);
            bindCoeffVolume(rcProg, transCoeffsB2_, &coeffUnit10);
            bindCoeffVolume(rcProg, transCoeffsB3_, &coeffUnit11);
        }
    }

    if(lightFunc_)
        lightFunc_->setRCShaderUniforms(rcProg);
}

void SHCoeffTrans::calcCoeffs() {
    LGL_ERROR;
    LINFO("Starting calculation of SH transfer coefficients...");

    //int maxTargets;
    //glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxTargets);

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if(considerNormals_)
        generateNormals();

    generateTfVolume();

    coeffPrg_->activate();
    //coeffPrg_->setIgnoreUniformLocationError(true);
    //setGlobalShaderParameters(coeffPrg_);

    TextureUnit transVolUnit;
    bindCoeffVolume(coeffPrg_, tfVolume_, &transVolUnit);

    TextureUnit normVolUnit;
    if(considerNormals_)
        bindCoeffVolume(coeffPrg_, normVolume_, &normVolUnit);

    TextureUnit sampleUnit, coeffUnit;
    sampleUnit.activate();
    sampleTex_->enable();
    sampleTex_->bind();
    coeffUnit.activate();
    coeffTex_->enable();
    coeffTex_->bind();

    tgt::ivec3 targetDims;
    if(transCoeffsI0_)
        targetDims = transCoeffsI0_->getVolume()->getDimensions();
    else
        targetDims = transCoeffsR0_->getVolume()->getDimensions();

    coeffPrg_->setUniform("lightSamples_", sampleUnit.getUnitNumber());
    coeffPrg_->setUniform("lightCoeffs_", coeffUnit.getUnitNumber());

    coeffPrg_->setUniform("volSize_", targetDims);
    coeffPrg_->setUniform("origVolSize_", tgt::ivec3(tfVolume_->getVolume()->getDimensions()));
    coeffPrg_->setUniform("sampleNumSqrt_", sampleNumSqrt_);

    // for future use
    //coeffPrg_->setUniform("numCoeffs_", (int)numCoeffs_);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, targetDims.x, targetDims.y);

    fbo_->activate();
    LGL_ERROR;

    unsigned int numBufs = 4;

    if(bm_ & SH_BLEEDING)
        numBufs += 4;
    std::vector<GLenum> buffers(numBufs);

    buffers[0] = GL_COLOR_ATTACHMENT0_EXT;
    buffers[1] = GL_COLOR_ATTACHMENT1_EXT;
    buffers[2] = GL_COLOR_ATTACHMENT2_EXT;
    buffers[3] = GL_COLOR_ATTACHMENT3_EXT;
    if(bm_ & SH_BLEEDING) {
        buffers[4] = GL_COLOR_ATTACHMENT4_EXT;
        buffers[5] = GL_COLOR_ATTACHMENT5_EXT;
        buffers[6] = GL_COLOR_ATTACHMENT6_EXT;
        buffers[7] = GL_COLOR_ATTACHMENT7_EXT;
    }

    if(!(bm_ & SH_BLEEDING)) {
        glDrawBuffers(numBufs, &(*buffers.begin()));

        for(int curSlice = 0; curSlice < targetDims.z; curSlice++) {
            fbo_->attachTexture(transCoeffsI0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsI1_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsI2_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT2_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsI3_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT3_EXT, 0, curSlice);

            coeffPrg_->setUniform("curSlice_", curSlice);
            renderQuad();
        }
    } else {
        glDrawBuffers(4, &(*buffers.begin()));
        coeffPrg_->setUniform("pass_", 0);

        for(int curSlice = 0; curSlice < targetDims.z; curSlice ++) {
            fbo_->attachTexture(transCoeffsR0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsR1_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsR2_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT2_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsR3_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT3_EXT, 0, curSlice);

            coeffPrg_->setUniform("curSlice_", curSlice);
            renderQuad();
        }

        LGL_ERROR;

        // necessary??
        //fbo_->UnattachAll();
        glDrawBuffers(numBufs, &(*buffers.begin()));
        coeffPrg_->setUniform("pass_", 1);

        for(int curSlice = 0; curSlice < targetDims.z; curSlice ++) {
            fbo_->attachTexture(transCoeffsG0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsG1_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsG2_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT2_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsG3_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT3_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsB0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT4_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsB1_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT5_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsB2_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT6_EXT, 0, curSlice);
            fbo_->attachTexture(transCoeffsB3_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT7_EXT, 0, curSlice);

            coeffPrg_->setUniform("curSlice_", curSlice);
            renderQuad();
        }
    }

    LGL_ERROR;
    fbo_->deactivate();
    glEnable(GL_DEPTH_TEST);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    coeffPrg_->deactivate();

    TextureUnit::setZeroUnit();

    if(bm_ == SH_REFLECT || bm_ & SH_SUBSURFACE)
        executeSecondPass();

    glPopAttrib();
    LINFO("Finished calculation of SH transfer coefficients.");
}

void SHCoeffTrans::executeSecondPass() {

    LGL_ERROR;
    LINFO("Starting calculation of SH transfer coefficients, 2nd pass...");

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if(bm_ == SH_SUBSURFACE) {
        std::vector<CoeffVolume*> sssVols;
        sssVols.push_back(transCoeffsI0_);
        sssVols.push_back(transCoeffsI1_);
        sssVols.push_back(transCoeffsI2_);
        sssVols.push_back(transCoeffsI3_);
        generateMipMapLevels(sssVols);
        LGL_ERROR;
    }
    if(bm_ == SH_BLEED_SUB) {
        std::vector<CoeffVolume*> sssVols;
        sssVols.push_back(transCoeffsR0_); sssVols.push_back(transCoeffsR1_); sssVols.push_back(transCoeffsR2_); /*sssVols.push_back(transCoeffsR3_);*/
        sssVols.push_back(transCoeffsG0_); sssVols.push_back(transCoeffsG1_); sssVols.push_back(transCoeffsG2_); /*sssVols.push_back(transCoeffsG3_);*/
        sssVols.push_back(transCoeffsB0_); sssVols.push_back(transCoeffsB1_); sssVols.push_back(transCoeffsB2_); /*sssVols.push_back(transCoeffsB3_);*/
        generateMipMapLevels(sssVols);
        LGL_ERROR;
    }

    coeffPrg2_->activate();
    //coeffPrg2_->setIgnoreUniformLocationError(true);

    //bindCoeffVolume(coeffPrg2_, tfVolume_);
    tgt::ivec3 targetDims;
    TextureUnit sampleUnit, coeffUnit;
    TextureUnit coeffUnit0, coeffUnit1, coeffUnit2, coeffUnit3,
                coeffUnit4, coeffUnit5, coeffUnit6, coeffUnit7, coeffUnit8;
    TextureUnit normVolUnit, transVolUnit;

    if(bm_ == SH_REFLECT) {
        targetDims = transCoeffsI0_->getVolume()->getDimensions();
        sampleUnit.activate();
        sampleTex_->enable();
        sampleTex_->bind();
        coeffUnit.activate();
        coeffTex_->enable();
        coeffTex_->bind();

        bindCoeffVolume(coeffPrg2_, tfVolume_, &transVolUnit);
        bindCoeffVolume(coeffPrg2_, transCoeffsI0_, &coeffUnit0);
        if(considerNormals_)
            bindCoeffVolume(coeffPrg2_, normVolume_, &normVolUnit);

        coeffPrg2_->setUniform("lightSamples_", sampleUnit.getUnitNumber());
        coeffPrg2_->setUniform("lightCoeffs_", coeffUnit.getUnitNumber());
        coeffPrg2_->setUniform("origVolSize_", tgt::ivec3(tfVolume_->getVolume()->getDimensions()));
        coeffPrg2_->setUniform("sampleNumSqrt_", sampleNumSqrt_);
    } else if (bm_ == SH_SUBSURFACE) {
        targetDims = sssCoeffs0_->getVolume()->getDimensions();
        bindCoeffVolume(coeffPrg2_, transCoeffsI0_, &coeffUnit0);
        bindCoeffVolume(coeffPrg2_, transCoeffsI1_, &coeffUnit0);
        bindCoeffVolume(coeffPrg2_, transCoeffsI2_, &coeffUnit0);
        coeffPrg2_->setUniform("scale_", dataScale_);
        coeffPrg2_->setUniform("origVolSize_", tgt::ivec3(transCoeffsI0_->getVolume()->getDimensions()));
    } else if (bm_ == SH_BLEED_SUB) {
        targetDims = sssCoeffs0_->getVolume()->getDimensions();
        bindCoeffVolume(coeffPrg2_, transCoeffsR0_, &coeffUnit0); bindCoeffVolume(coeffPrg2_, transCoeffsR1_, &coeffUnit1); bindCoeffVolume(coeffPrg2_, transCoeffsR2_, &coeffUnit2);
        bindCoeffVolume(coeffPrg2_, transCoeffsG0_, &coeffUnit3); bindCoeffVolume(coeffPrg2_, transCoeffsG1_, &coeffUnit4); bindCoeffVolume(coeffPrg2_, transCoeffsG2_, &coeffUnit5);
        bindCoeffVolume(coeffPrg2_, transCoeffsB0_, &coeffUnit6); bindCoeffVolume(coeffPrg2_, transCoeffsB1_, &coeffUnit7); bindCoeffVolume(coeffPrg2_, transCoeffsB2_, &coeffUnit8);

        coeffPrg2_->setUniform("scale_", dataScale_);
        coeffPrg2_->setUniform("origVolSize_", tgt::ivec3(transCoeffsR0_->getVolume()->getDimensions()));
    }

    coeffPrg2_->setUniform("volSize_", targetDims);

    //for future use
    //coeffPrg2_->setUniform("numCoeffs_", (int)numCoeffs_);

    //LGL_ERROR;
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, targetDims.x, targetDims.y);

    fbo3_->activate();
    LGL_ERROR;

    unsigned int numBufs;
    if(bm_ == SH_REFLECT)
        numBufs = 3;
    else
        numBufs = 7;

    std::vector<GLenum> buffers(numBufs);

    buffers[0] = GL_COLOR_ATTACHMENT0_EXT;
    buffers[1] = GL_COLOR_ATTACHMENT1_EXT;
    buffers[2] = GL_COLOR_ATTACHMENT2_EXT;
    if(bm_ & SH_SUBSURFACE) {
        buffers[3] = GL_COLOR_ATTACHMENT3_EXT;
        buffers[4] = GL_COLOR_ATTACHMENT4_EXT;
        buffers[5] = GL_COLOR_ATTACHMENT5_EXT;
        buffers[6] = GL_COLOR_ATTACHMENT6_EXT;
    }

    glDrawBuffers(numBufs, &(*buffers.begin()));

    for(int curSlice = 0; curSlice < targetDims.z; curSlice++) {

        if(bm_ == SH_REFLECT) {
            fbo3_->attachTexture(transCoeffsR0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
            fbo3_->attachTexture(transCoeffsG0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);
            fbo3_->attachTexture(transCoeffsB0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT2_EXT, 0, curSlice);
        }
        if(bm_ & SH_SUBSURFACE) {
            fbo3_->attachTexture(sssCoeffs0_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs1_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs2_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT2_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs3_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT3_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs4_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT4_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs5_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT5_EXT, 0, curSlice);
            fbo3_->attachTexture(sssCoeffs6_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT6_EXT, 0, curSlice);
        }

        coeffPrg2_->setUniform("curSlice_", curSlice);
        renderQuad();
    }

    LGL_ERROR;
    fbo3_->deactivate();
    //glEnable(GL_DEPTH_TEST);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    coeffPrg2_->deactivate();
    TextureUnit::setZeroUnit();

    glPopAttrib();

    //if(bm_ == SH_BLEED_SUB) {
        //delete transCoeffsR0_; transCoeffsR0_ = 0;
        //delete transCoeffsR1_; transCoeffsR1_ = 0;
        //delete transCoeffsR2_; transCoeffsR2_ = 0;
        //delete transCoeffsG0_; transCoeffsG0_ = 0;
        //delete transCoeffsG1_; transCoeffsG1_ = 0;
        //delete transCoeffsG2_; transCoeffsG2_ = 0;
        //delete transCoeffsB0_; transCoeffsB0_ = 0;
        //delete transCoeffsB1_; transCoeffsB1_ = 0;
        //delete transCoeffsB2_; transCoeffsB2_ = 0;
    //}
}

void SHCoeffTrans::generateNormals() {

    normPrg_->activate();
    //normPrg_->setIgnoreUniformLocationError(true);

    tgt::ivec3 dims = normVolume_->getVolume()->getDimensions();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, dims.x, dims.y);

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    LGL_ERROR;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    std::vector<GLenum> buffers = std::vector<GLenum>();
    buffers.push_back(GL_COLOR_ATTACHMENT0_EXT);

    fbo2_->activate();

    glDrawBuffers(static_cast<GLsizei>(buffers.size()), &(*buffers.begin()));

    // bind volume texture and pass sampler to the shader
    TextureUnit volUnit;
    volUnit.activate();
    volume_->getRepresentation<VolumeGL>()->getTexture()->bind();
    //set clamp method for volume texture
    glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, tgt::ivec4(0).elem);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    normPrg_->setUniform("volume_", volUnit.getUnitNumber());
    normPrg_->setUniform("volSize_", dims);

    for(int curSlice = 0; curSlice < dims.z; curSlice++) {
        fbo2_->attachTexture(normVolume_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, 0, curSlice);
        normPrg_->setUniform("curSlice_", curSlice);

        renderQuad();
    }

    LGL_ERROR;
    fbo2_->deactivate();

    glEnable(GL_DEPTH_TEST);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    normPrg_->deactivate();
    TextureUnit::setZeroUnit();
    LINFO("Finished calculation of SH transfer coefficients, 2nd pass.");
}

void SHCoeffTrans::generateTfVolume() {
    LINFO("Starting generation of TF volume...");
    LGL_ERROR;

    tfPrg_->activate();
    //tfPrg_->setIgnoreUniformLocationError(true);

    tgt::ivec3 dims = tfVolume_->getVolume()->getDimensions();
    std::vector<CoeffVolume*> logVols;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    TextureUnit transferUnit;
    tfProp_->get()->setUniform(tfPrg_, "transferFunc_", "transferFuncTex_", transferUnit.getUnitNumber());

    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.pushMatrix();
    MatStack.loadIdentity();

    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.pushMatrix();
    MatStack.loadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    LGL_ERROR;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    std::vector<GLenum> buffers = std::vector<GLenum>();
    buffers.push_back(GL_COLOR_ATTACHMENT0_EXT);
    if(eri_)
        buffers.push_back(GL_COLOR_ATTACHMENT1_EXT);

    fbo2_->activate();

    glDrawBuffers(static_cast<GLsizei>(buffers.size()), &(*buffers.begin()));
    TextureUnit logUnit, volUnit;

    for(int c = 0; dims.x != 1 && dims.y != 1 && dims.z != 1; c++) {
        if(c == 0) {
            // bind volume texture and pass sampler to the shader
            volUnit.activate();
            volume_->getRepresentation<VolumeGL>()->getTexture()->bind();
            //set clamp method for volume texture
            glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, tgt::ivec4(0).elem);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

            tfPrg_->setUniform("volume_", volUnit.getUnitNumber());
            LGL_ERROR;
        } else {
            // add previous tfVolume_-Mipmaplevel
            bindCoeffVolume(tfPrg_, logVols[logVols.size()-1], &logUnit);
            LGL_ERROR;
        }

        // for some reason, rebinding the transfer-texture is necessary in every iteration
        transferUnit.activate();
        if (tfProp_->get())
            tfProp_->get()->bind();

        if(eri_)
            logVols.push_back(new CoeffVolume(dims, "volume_"));

        glViewport(0, 0, dims.x, dims.y);
        tfPrg_->setUniform("volSize_", dims);
        tfPrg_->setUniform("pass_", c);

        tgt::Texture* eriTex = 0;
        if(eri_)
            eriTex = logVols[logVols.size() - 1]->getVolumeGL()->getTexture();

        // Perform rendering to specific mipmap levels.  The minification-filter is non-standard and implemented in the tfid shader;
        // it makes exponential growth of the length of raycasting steps for determining shadow-factor and bleeding-color in the
        // shcoeffs-shader possible, resulting in a considerable acceleration while still getting good-looking pictures.  Rendering
        // to two targets is necessary since we require the result from the previous iteration, yet we cannot bind a texture and render to
        // it at the same time as far as I know (that is, access one mipmap level and simultaneously render to another at the same time).
        // This is based on the paper "Fast GPU-based Visibility Computation for Natural Illumination of Volume Data" by Tobias Ritschel.
        for(int curSlice = 0; curSlice < dims.z; curSlice++) {
            fbo2_->attachTexture(tfVolume_->getVolumeGL()->getTexture(), GL_COLOR_ATTACHMENT0_EXT, c, curSlice);
            if(eri_)
                fbo2_->attachTexture(eriTex, GL_COLOR_ATTACHMENT1_EXT, 0, curSlice);

            tfPrg_->setUniform("curSlice_", curSlice);

            renderQuad();
        }

        if(!eri_)
            break;

        // We have to be sure that the final mipmap level has a resolution of 1x1x1, even when using non-cube datasets - otherwise, there
        // will be undefined results.  For example, one would expect the final mipmap-resolution of the engine dataset (256x256x128)
        // to be 2x2x1, but we have to go one step further and reduce it once more to 1x1x1.
        if(dims.x != 1) dims.x /= 2;
        if(dims.y != 1) dims.y /= 2;
        if(dims.z != 1) dims.z /= 2;
    }

    LGL_ERROR;
    fbo2_->deactivate();

    glEnable(GL_DEPTH_TEST);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    MatStack.matrixMode(tgt::MatrixStack::PROJECTION);
    MatStack.popMatrix();
    MatStack.matrixMode(tgt::MatrixStack::MODELVIEW);
    MatStack.popMatrix();

    for(size_t i = 0; i < logVols.size(); i++)
        delete logVols[i];

    tfPrg_->deactivate();
    TextureUnit::setZeroUnit();

    LINFO("Finished generation of TF volume.");
}

void SHCoeffTrans::renderQuad() {
    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(-1.0, -1.0);
        glTexCoord2f(1.f, 0.f);
        glVertex2f( 1.f, -1.f);
        glTexCoord2f(1.f, 1.f);
        glVertex2f( 1.0,  1.0);
        glTexCoord2f(0.f, 1.f);
        glVertex2f(-1.0,  1.0);
    glEnd();
}

void SHCoeffTrans::generateMipMapLevels(std::vector<CoeffVolume*> vols) {

    TextureUnit texUnit;
    for(size_t i = 0; i < vols.size(); i++) {
        const VolumeTexture* tex = vols[i]->getVolumeGL()->getTexture();
        texUnit.activate();

        tex->enable();
        tex->bind();

        glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmapEXT(GL_TEXTURE_3D);

        LGL_ERROR;
        tex->disable();
        TextureUnit::setZeroUnit();
    }
}

} // namespace

