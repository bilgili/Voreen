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

#include <math.h>
#include <limits>
#include <algorithm>
#include <sstream>

#include "tgt/tgt_gl.h"
#include "tgt/gpucapabilities.h"

#include "tgt/plane.h"
#include "tgt/glmath.h"

#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/render/slicerenderer.h"
#include "voreen/core/vis/processors/portmapping.h"
#include "voreen/core/volume/modality.h"

using tgt::vec2;
using tgt::vec3;
using tgt::vec4;
using tgt::ivec2;
using tgt::ivec3;
using tgt::bvec3;
using tgt::mat3;
using tgt::mat4;
using tgt::quat;
using tgt::plane;

namespace voreen {

/*
    init statics
*/

const Identifier SliceRendererBase::transFuncTexUnit_("transFuncTexUnit");
const Identifier SliceRendererBase::volTexUnit_("volTexUnit");

static const char* setSamplingRateId_   = "sr.set.samplingRate";
static const char* useLowerThresholdId_ = "sr.use.lowerThreshold";
static const char* useUpperThresholdId_ = "sr.use.upperThreshold";
static const char* usePhongLightingId_  = "sr.use.phongLighting";
static const char* useCalcGradientsId_  = "sr.use.calcGradients";
static const char* usePreIntegrationId_ = "sr.use.preIntegration";
static const char* showLookupTableId_   = "sr.show.lookupTable";

SliceRendererBase::SliceRendererBase()
    : VolumeRenderer()
    , transferFunc_(0)
    , lookupTable_(0)
    , transferFuncShader_(0)
    , needRecompileShader_(false)
    , useLowerThreshold_(useLowerThresholdId_, "Use lower threshold", &needRecompileShader_, true)
    , useUpperThreshold_(useUpperThresholdId_, "Use Upper threshold", &needRecompileShader_, true)
    , usePhongLighting_ (usePhongLightingId_,  "Use Phong lighting", &needRecompileShader_, false)
    , useCalcGradients_ (useCalcGradientsId_,  "Calculate gradients on the fly", &needRecompileShader_, false)
    , usePreIntegration_(usePreIntegrationId_, "Use Pre-Integration", &needRecompileShader_, false)
    , threshold_( vec2(0.f, 1.f) )
{
    std::vector<Identifier> units;
    units.push_back(transFuncTexUnit_);
    units.push_back(volTexUnit_);
    tm_.registerUnits(units);

    addProperty(&useLowerThreshold_);
    addProperty(&useUpperThreshold_);
    addProperty(&usePhongLighting_);
    addProperty(&useCalcGradients_);
    addProperty(&usePreIntegration_);

	TransFuncIntensity* tf = new TransFuncIntensity();
	tf->createStdFunc();
    transferFunc_ = tf;

    createInport("volumehandle.volumehandle");
    createOutport("image.outport");
}



SliceRendererBase::~SliceRendererBase() {
    if (lookupTable_)
        delete lookupTable_;

    if (transferFuncShader_)
        ShdrMgr.dispose(transferFuncShader_);

    MsgDistr.remove(this);
}

void SliceRendererBase::init() {
    if ( GpuCaps.areShadersSupported() )
        setupShader();
    else if (GpuCaps.areSharedPalettedTexturesSupported())
        glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
    // else -> no support for transfer functions
}

void SliceRendererBase::deinit() {
    if ( GpuCaps.areShadersSupported() )
        tgt::Shader::deactivate();
    else if ( GpuCaps.areSharedPalettedTexturesSupported() )
        glDisable(GL_SHARED_TEXTURE_PALETTE_EXT);
    // else -> no support for transfer functions
}

/*
    internal helpers
*/

bool SliceRendererBase::ready() const {
    if ( currentVolumeHandle_ == 0 )
        return false;

    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();
    // we need at least one texture
    if ( (volumeGL == 0) || (volumeGL->getNumTextures() == 0) )
        return false;

    tgtAssert( volumeGL->getTexture() != NULL, "No volume texture" );

    bool ready;

    // transferfunction and transferfuncshader are needed, if shaders are supported
    if ( GpuCaps.areShadersSupported() )
        ready = ( transferFunc_ != 0 && transferFuncShader_ != 0 );
    // transferfunction is needed and texture's format has to be color-index, if we use shared palette textures
    else if ( !GpuCaps.areShadersSupported() && GpuCaps.areSharedPalettedTexturesSupported() )
        ready = ( transferFunc_ != 0 && volumeGL->getTexture()->getFormat() == GL_COLOR_INDEX );
    // without shaders / palette textures, the texture's format has to be RGBA or luminance-alpha
    else if ( !GpuCaps.areShadersSupported() && !GpuCaps.areSharedPalettedTexturesSupported() )
        ready = ( volumeGL->getTexture()->getFormat() == GL_LUMINANCE_ALPHA ||
                  volumeGL->getTexture()->getFormat() == GL_RGBA );
    else
        ready = false;

    if ( !ready )
        LWARNING("SliceRenderer3D not ready to render!");

    return ready;
}

void SliceRendererBase::saveModelViewAndTextureMatrices() {
    // save matrices
    //every texture unit has its own matrix stack, so save both
    glMatrixMode(GL_TEXTURE);
    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    glPushMatrix();
    glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void SliceRendererBase::restoreModelViewAndTextureMatrices() {
    // restore matrices
    glMatrixMode(GL_TEXTURE);
    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    glPopMatrix();
    glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void SliceRendererBase::setupTextures() {
    /*
        use either the look-up table or the transfer function for the transFuncTexUnit_
    */
    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    if ( usePreIntegration_.get() )
        lookupTable_->bind();
    else
        transferFunc_->bind();

    glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
}

void SliceRendererBase::setupShader() {
    // if we don't have hardware support -> return
    if ( !GpuCaps.areShadersSupported() )
        return;

    if (!transferFuncShader_)
        transferFuncShader_ = ShdrMgr.load("sr_transfunc", buildHeader(), false);

    // activate if everything went fine and set the needed uniforms
    if (transferFuncShader_) {
        transferFuncShader_->activate();

        transferFuncShader_->setUniform("volumeDataset_", tm_.getTexUnit(volTexUnit_));

        if ( useLowerThreshold_.get() || useUpperThreshold_.get() )
            transferFuncShader_->setUniform("threshold_", threshold_);
        if ( usePreIntegration_.get() ) {
            transferFuncShader_->setUniform("lookupTable_", tm_.getTexUnit(transFuncTexUnit_));
        }
        else
            transferFuncShader_->setUniform("transferFunc_", tm_.getTexUnit(transFuncTexUnit_));
    }

    LGL_ERROR;
}

std::string SliceRendererBase::buildHeader() {
	std::string header = VolumeRenderer::generateHeader();

    if (transferFunc_)
        header += transferFunc_->getShaderDefines();

    std::ostringstream oss;

    if ( useLowerThreshold_.get() )
        oss << "#define USE_LOWER_THRESHOLD" << std::endl;
    if ( useUpperThreshold_.get() )
        oss << "#define USE_UPPER_THRESHOLD" << std::endl;
    if ( usePhongLighting_.get() )
        oss << "#define USE_PHONG_LIGHTING" << std::endl;
    if ( usePreIntegration_.get() )
        oss << "#define USE_PRE_INTEGRATION" << std::endl;

    oss << "#define VOL_TEX " << tm_.getTexUnit(volTexUnit_) << std::endl;

    header += oss.str();

    return header;
}

void SliceRendererBase::rebuildShader() {
    // do nothing if there is no shader at the moment
    if (!transferFuncShader_)
        return;

    transferFuncShader_->setHeaders( buildHeader(), false);
    transferFuncShader_->rebuild();
}

void SliceRendererBase::deactivateShader() {
    if (transferFuncShader_ && transferFuncShader_->isActivated())
        transferFuncShader_->deactivate();
}

void SliceRendererBase::usePreIntegration(TransFunc* source) {
	if ( usePreIntegration_.get() ) {
		if (lookupTable_)
			delete lookupTable_;
		lookupTable_ = TransFunc2D::createPreIntegration(source);
	}
}

void SliceRendererBase::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
    VolumeRenderer::processMessage(msg, dest);

    /*
        transfer fucntion
    */
    if (msg->id_ == usePreIntegrationId_) {
        usePreIntegration_.set( msg->getValue<bool>() );
		usePreIntegration(transferFunc_);
    }
    else if (msg->id_ == setTransFunc_) {
        TransFunc* tf = msg->getValue<TransFunc*>();
		usePreIntegration(tf);

        if (tf != transferFunc_) {
            // shader has to be recompiled, if the transferfunc header has changed
            std::string definesOld = transferFunc_ ? transferFunc_->getShaderDefines() : "";
            std::string definesNew = tf ? tf->getShaderDefines() : "";
            if ( definesOld != definesNew )
                needRecompileShader_ = true;
            transferFunc_ = tf;
        }

        if( (currentVolumeHandle_ != 0) && (currentVolumeHandle_->getVolumeGL() != 0) )
            // VolumeGL handles all hardware support cases
            currentVolumeHandle_->getVolumeGL()->applyTransFunc(transferFunc_); 
    }

    /*
        lighting
    */
    else if (msg->id_ == usePhongLightingId_)
        usePhongLighting_.set( msg->getValue<bool>() );
    else if (msg->id_ == useCalcGradientsId_)
        useCalcGradients_.set( msg->getValue<bool>() );

    /*
        threshold
    */
    else if (msg->id_ == useLowerThresholdId_)
        useLowerThreshold_.set( msg->getValue<bool>() );
    else if (msg->id_ == useUpperThresholdId_)
        useUpperThreshold_.set( msg->getValue<bool>() );
    else if (msg->id_ == setLowerThreshold_) {
        threshold_.x = msg->getValue<float>();
        if ( useLowerThreshold_.get() && transferFuncShader_ && transferFuncShader_->isActivated() )
            transferFuncShader_->setUniform("threshold_", threshold_);
        LGL_ERROR;
    }
    else if (msg->id_ == setUpperThreshold_) {
        threshold_.y = msg->getValue<float>();
        if ( useUpperThreshold_.get() && transferFuncShader_ && transferFuncShader_->isActivated())
            transferFuncShader_->setUniform("threshold_", threshold_);
        LGL_ERROR;
    }

    /*
        recompile shader if neccessary
    */
    if (needRecompileShader_) {
        rebuildShader();
        needRecompileShader_ = false;
    }
}

void SliceRendererBase::setPropertyDestination(Identifier tag) {
    setTag(tag);
    MsgDistr.insert(this);
}

//---------------------------------------------------------------------------

/*
    helper
*/

enum SamplingRate
{
    UGLY,
    BAD,
    NORMAL,
    GOOD,
    HIGH,
    VERY_HIGH,
    EXTREME_HIGH,
    NUM_SAMPLING_RATE_VALUES
};

float samplingRate2Float(int sr) {
    switch (sr) {
        case UGLY:
            return 4.f;
        case BAD:
            return 2.f;
        case NORMAL:
            return 1.f;
        case GOOD:
            return 0.5f;
        case HIGH:
            return 0.25f;
        case VERY_HIGH:
            return 0.125f;
        case EXTREME_HIGH:
            return 0.0625f;
        default:
            tgtAssert(false, "illegal switch/case value");
    }

    return 1.f; // avoid warning
}

SliceRenderer3D::SliceRenderer3D()
  : SliceRendererBase(),
    showLookupTable_  (showLookupTableId_, "Show lookup table", false),
    samplingRate_(1.f)
{
    std::vector<std::string> strings;
    strings.push_back("ugly");
    strings.push_back("bad");
    strings.push_back("normal");
    strings.push_back("good");
    strings.push_back("high");
    strings.push_back("very high");
    strings.push_back("extreme high");

    samplingRateProp_ = new EnumProp(setSamplingRateId_, "Sampling rate", strings, NORMAL, false);
    addProperty(&showLookupTable_);
    addProperty(samplingRateProp_);
}


SliceRenderer3D::~SliceRenderer3D() {
    MsgDistr.remove(this);
    delete samplingRateProp_;
}

const std::string SliceRenderer3D::getProcessorInfo() const {
	return "This class renders volume datasets 3d with 3d slicing.";
}

void SliceRenderer3D::setupShader() {
    // if we don't have hardware support -> return
    if ( !GpuCaps.areShadersSupported() )
        return;

    SliceRendererBase::setupShader();

    // add the samplingRate_ uniform to the normal stuff
    if (transferFuncShader_)
        transferFuncShader_->setUniform("samplingRate_", samplingRate_);
}

std::string SliceRenderer3D::buildHeader() {
    std::string header = SliceRendererBase::buildHeader();
    header += "#define USE_SAMPLING_RATE\n";

    return header;
}

void SliceRenderer3D::setPropertyDestination(Identifier tag) {
    SliceRendererBase::setPropertyDestination(tag);

    const Properties& props = getProperties();

    for (size_t i = 0; i < props.size(); ++i)
        props[i]->setMsgDestination(tag);
}

// FIXME: obsoleted by new volume concept
/*
void SliceRenderer3D::setVolumeContainer() {
    Volume* volume = getCurrentDataset()->getVolume();

    //Calculate sliceDensity_
    sliceDensity_ = (volume->getCubeSize().z) / (volume->getDimensions().z + 0.01f); // TODO what offset should be used here?
    sliceDensity_ *= samplingRate_;
    //TODO: consider spacing of volume data set when calculating density
}
*/

void SliceRenderer3D::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
    SliceRendererBase::processMessage(msg, dest);

    // set showLookupTable_ to visible state or invisible state
    showLookupTable_.setVisible( usePreIntegration_.get() );
    if ( usePreIntegration_.get() == false )
        showLookupTable_.set(false);

    if (msg->id_ == showLookupTableId_)
        showLookupTable_.set( msg->getValue<bool>() );
    else if (msg->id_ == setSamplingRateId_) {
        samplingRateProp_->set( msg->getValue<int>() );
        samplingRate_ = samplingRate2Float( samplingRateProp_->get() );
        if ((currentVolumeHandle_ != 0) && (currentVolumeHandle_->getVolume() != 0) ) {
			Volume* volume = currentVolumeHandle_->getVolume();
            // TODO what offset should be used here?
			sliceDensity_ = (volume->getCubeSize().z) / (volume->getDimensions().z + 0.01f);
			sliceDensity_ *= samplingRate_;

			if ( transferFuncShader_ && usePreIntegration_.get() )
				transferFuncShader_->setUniform("sliceDistance_", sliceDensity_);
		}
    }
}

void SliceRenderer3D::calculateTransformation() {
    matrix_ = camera_->getViewMatrix();
    matrix_.t23 = -matrix_.t23;
    rot_ = matrix_.getRotationalPart();
    // get translation
    trans_ = vec3(matrix_.elemRowCol[0][3], matrix_.elemRowCol[1][3], matrix_.elemRowCol[2][3]);
}

inline bool findMinMax(const vec3& v1, const vec3& v2) {
    return v1.z < v2.z;
};

void SliceRenderer3D::paint() {
    glClearColor(1,1,1,0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH);
/*    glColor3f(0,1,0);
    glBegin(GL_QUADS);
        glVertex3f(-0.8,-0.8, 0);
        glVertex3f(-0.8,0.8, 0);
        glVertex3f(0.8,0.8, 0);
        glVertex3f(0.8,-0.8, 0);
    glEnd();*/
//    return;

    
    if ( !ready() )
        return;

    setupShader();

    if (usePhongLighting_.get())
        setLightingParameters();

    // save matrices
    saveModelViewAndTextureMatrices();
/*
    // setup blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0.0);
*/
    setupTextures();
    // volTexUnit_ is now active

    /*
        Stores the depth of the first slice which is rendered.
        All other slices must be aligned to that one.
        INFINITY means that the first slice has not been rendered yet.
    */
    float firstDepth = std::numeric_limits<float>::infinity();


    // sort textures and store pointers to them
    // FIXME: not testing for NULL-pointers is very unsafe...
    //
    VolumeGL::SortedTextures textures = currentVolumeHandle_->getVolumeGL()->getSortedTextures(matrix_);

    for (size_t i = 0; i < textures.size(); ++i) {
        const VolumeTexture* tex = textures[i];
        tex->bind();

        vec3 tcv[8]; // stores transformed cube vertices
        const vec3* cv = tex->getCubeVertices();
        vec3 center = tex->getCenter();

        // transform cube vertices
        for (size_t j = 0; j < 8; ++j)
            tcv[j] = rot_ * (cv[j] - center); // center so planes' normals always point outwards

        // stores transformed cube planes
        plane tcp[8];
        plane::createCubePlanes(tcv, tcp);

        // get useful texture related data
        vec3 cubeSize = tex->getCubeSize();
        vec3 llf = tex->getLLF();
        vec3 urb = tex->getURB();

        if (transferFuncShader_)
            transferFuncShader_->activate();

        glMatrixMode(GL_TEXTURE);
            // fourth:  apply texture matrix to map [0, 1] to the useful portion
            tgt::loadMatrix( tex->getMatrix() );
            // third:   map to [0, 1]
            tgt::translate(vec3(0.5f));
            // second:  map to [-0.5, 0.5]
            tgt::scale(1.f/cubeSize);
            // first:   invert (via transposing) the rotation, thus we are back in the orginal coordinate system
            tgt::multTransposeMatrix(rot_);

        glMatrixMode(GL_MODELVIEW);
            // third:   apply normal camera transformation (i.e. reapply rotation and apply translation)
            tgt::loadMatrix(matrix_);
            // second:  reapply center translation
            tgt::translate(center);
            // first:   invert (via transposing) the rotation, thus we are back in the orginal coordinate system
            tgt::multTransposeMatrix(rot_);

        vec3 vMin = *std::min_element(tcv, tcv + 8, findMinMax);
        vec3 vMax = *std::max_element(tcv, tcv + 8, findMinMax);

        // used to loop the slices from the farest to the nearest
        float depth;
        // knows the calculated depth which must be aligned to firstDepth
        float calcDepth = vMin.z + sliceDensity_;

        if (firstDepth == std::numeric_limits<float>::infinity()) {
            // we are going to render the first slice here
            depth = calcDepth;
            firstDepth = depth;
        }
        else {
            // align to firstDepth
            depth = firstDepth;

            // TODO use fmod here instead of loops once this is working
            if (depth < calcDepth) {
                while (depth < calcDepth)
                    depth += sliceDensity_;
            }
            else { // depth >= calcDepth
                while (depth > calcDepth)
                    depth -= sliceDensity_;
            }
        }

        // render all slices
        while (depth < vMax.z) {
            vec3 polygon[6] = {
                vec3(-100.f, -100.f, depth),
                vec3(-100.f,  100.f, depth),
                vec3( 100.f,  100.f, depth),
                vec3( 100.f, -100.f, depth),
                vec3(),
                vec3()
            };
            size_t numIn = 4;

            /*
                clip against all 6 planes
            */

            // actually there can only be 6 vertices, but for security let's allocate 12
            vec3 clippedPolygon[12];
            size_t numOut;
            std::vector<int> vi;

            vec3* in = &polygon[0];
            vec3* out = &clippedPolygon[0];
            for (size_t j = 0; j < 6; ++j) {
                tcp[j].clipPolygon(in, numIn, out, numOut);
                // swap
                vec3* tmp = in;
                in = out;
                out = tmp;
                numIn = numOut;
            }

            // render a slice
#if 0
            glBegin(GL_POLYGON);
#else
            tgt::Shader::deactivate();
            glBegin(GL_LINE_LOOP);
            glColor4ub(0xff, 0x00, 0x00, 0xff);
#endif
            for (size_t j = 0; j < numIn; ++j) {
                vec3 tc = polygon[j];
                tgt::texCoord(tc);
                tgt::vertex(tc);
            }
            glEnd();

            //glMatrixMode(GL_PROJECTION);
            //glLoadIdentity();
//            glMatrixMode(GL_MODELVIEW);
//            glLoadIdentity();


            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH);
            glColor3f(0,1,0);
            glBegin(GL_QUADS);
                glVertex3f(-0.8f, -0.8f, 0.f);
                glVertex3f(-0.8f, 0.8f, 0.f);
                glVertex3f(0.8f, 0.8f, 0.f);
                glVertex3f(0.8f, -0.8f, 0.f);
            glEnd();

            depth += sliceDensity_;
        } // while
    } // for

    // deactivate Shader
    deactivateShader();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);

    if ( showLookupTable_.get() ) {
        glActiveTexture(GL_TEXTURE0);

        glEnable(GL_TEXTURE_2D);
        lookupTable_->bind();
        glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
        glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.5f, 0.5f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex2f  (1.0f, 0.5f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex2f  (1.0f, 1.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex2f  (0.5f, 1.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    // restore matrices
    restoreModelViewAndTextureMatrices();
}


void SliceRenderer3D::process(LocalPortMapping* portMapping) {

    VolumeHandle* volumeHandle = portMapping->getVolumeHandle("volumehandle.volumehandle");
    if (volumeHandle != currentVolumeHandle_)
        setVolumeHandle(volumeHandle);
    
    if ( currentVolumeHandle_ == 0 )
        return;

     if (tc_) {
        std::ostringstream oss;
        oss << "SliceRenderer3D::render(dataset=" << currentVolumeHandle_ << ") dest";
		int dest = portMapping->getTarget("image.outport");
        tc_->setActiveTarget(dest, oss.str());
    }
    glViewport(0,0,static_cast<int>(size_.x),static_cast<int>(size_.y));
    LGL_ERROR;
    // set background to background color, even if no dataset is rendered
    tgt::Color clearColor = backgroundColor_.get();
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    calculateTransformation();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    paint();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glDisable(GL_DEPTH_TEST);

}

//------------------------------------------------------------------------------

OverviewRenderer::OverviewRenderer(bool drawGrid,
                                   tgt::Color innerGridColor,
                                   tgt::Color outerGridColor)
    : SliceRendererBase()
    , trans_(0.f)
    , scaleVector_(vec3(1.f))
    , scaleFactor_(1.f)
    , numX_(-1)
    , numY_(-1)
    , drawGrid_(drawGrid)
    , innerGridColor_(innerGridColor)
    , outerGridColor_(outerGridColor)
{
    dragging_ = false;
}

//RPTMERGE todo portmapping
void OverviewRenderer::process(LocalPortMapping* /*localPortMapping*/) {
    // FIXME: obsolete due to removal of VolumeContainer. Migrate to new
    // volume concept.
    //
    /*
    if ( !getCurrentDataset() || (getCurrentDataset()->getNumTextures() == 0) )
        return;
    */
    if ( !ready() )
        return;

    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();

    glDisable(GL_DEPTH_TEST);

    setupTextures();
    // volTexUnit_ is now active

    saveModelViewAndTextureMatrices();

    setupShader();

    int dest;
    if (tc_) {
        std::ostringstream oss;
        oss << "OverviewRenderer::render(dataset=" << currentVolumeHandle_ << ") dest";
		dest = 0; //FIXME: tc_->getTargetID(getTargetConfig(ttImage_), oss.str());
        tc_->setActiveTarget(dest);
    }

    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);


    // translate and scale our virtual slice plate [-1,-1][1,1]
    // according to current scale vector / translation vector
    calcScaleVector();
    vec3 scaleVec = scaleVector_*scaleFactor_;
    glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScalef(scaleVec.x, scaleVec.y, scaleVec.z);
        glTranslatef(trans_.x, trans_.y, 0.f);


    //
    // render slice plate
    //

    // dimensions of the main volume
    vec3 volumeCubeSize = volumeGL->getVolume()->getCubeSize();

    // iterate over all split volumes
    for (size_t i = 0; i < volumeGL->getNumTextures(); ++i) {

        // get current split volume
        const VolumeTexture* tex = volumeGL->getTexture(i);

        // bind split volume
        tex->bind();

        // load texture scaling matrix of split volume
        glMatrixMode(GL_TEXTURE);
        tgt::loadMatrix(tex->getMatrix());

        // lower-left-front and upper-right-back of split volume
        vec3 llf = tex->getLLF();
        vec3 urb = tex->getURB();
        // transform llf/urb to [0;1]-space
        llf = (llf + volumeCubeSize/2.f) / volumeCubeSize;
        urb = (urb + volumeCubeSize/2.f) / volumeCubeSize;
        // invert y-axis (runs from top to bottom in bottom axial view)
        llf.y = 1.f - llf.y;
        urb.y = 1.f - urb.y;
        // invert z-axis (first slice has z-coordinate 1.f instead of 0.f)
        llf.z = 1.f - llf.z;
        urb.z = 1.f - urb.z;

        // calculate numbers of start/end slice this split-volume contains
        int numSlices = volumeGL->getVolume()->getDimensions().z;
        int startSlice = int(llf.z * numSlices);
        int endSlice = int(urb.z * (numSlices-1));

        // x-y-offset of a split-slice within the main volume
        vec3 sliceTranslation = (llf + urb) - 1.f;
        sliceTranslation.z = 0.f;
        // x-y-scaling of a split-slice within the main volume
        vec3 sliceScale = urb - llf;
        sliceScale.z = 1.f;

        glMatrixMode(GL_MODELVIEW);

        // render all slices this split-volume contains
        for (int sliceNr=startSlice; sliceNr <= endSlice; sliceNr++) {

            glPushMatrix();

            // slice's z texture coordinate in split volume
            float depth = (sliceNr-startSlice) / float(endSlice-startSlice);

            // calculate coordinates of current (complete) slice on
            // virtual slice plate with dimensions [-1,-1][1,1]
            vec2 slice_ll, slice_ur;
            getSliceCoords(sliceNr, slice_ll, slice_ur);

            // translate/scale slice to correct position on slice plate
            tgt::translate( (slice_ur+slice_ll) / 2.f );
            tgt::scale( (slice_ur-slice_ll) / 2.f );

            // translate/scale split-slice to correct position in volume
            tgt::translate(sliceTranslation);
            tgt::scale(sliceScale);

            // finally render split-slice
            glBegin(GL_QUADS);

                glTexCoord3f(0.0f, 0.0f, depth);
                glVertex3f(-1.f, -1.f, 0.0f);

                glTexCoord3f(1.f, 0.f, depth);
                glVertex3f(1.f, -1.f, 0.0f);

                glTexCoord3f(1.f, 1.f, depth);
                glVertex3f(1.f, 1.f, 0.0f);

                glTexCoord3f(0.f, 1.f, depth);
                glVertex3f(-1.f, 1.f, 0.0f);

            glEnd();

            glPopMatrix();

        } // iteration over split-volume slices
    } // iteration over split-volumes

    //
    // end of slice plate rendering
    //

    deactivateShader();

    // render grid
    if (drawGrid_) {

        float sliceWidth  = 2.f / float(numX_);
        float sliceHeight = 2.f / float(numY_);

        glPushAttrib(GL_CURRENT_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw lines between slices
        glColor4fv(innerGridColor_.elem);
        glBegin(GL_LINES);
            for (int x = 1; x < numX_ ; ++x) {
                glVertex3f(-1.f + sliceWidth * x, -1.f, 0.0f);
                glVertex3f(-1.f + sliceWidth * x, 1.f, 0.0f);
            }
            for (int y = 1; y < numY_ ; ++y) {
                glVertex3f(-1.f, -1.f + sliceHeight * y, 0.0f);
                glVertex3f(1.f, -1.f + sliceHeight * y, 0.0f);
            }
            glEnd();

        // draw border around slice plate
        glColor4fv(outerGridColor_.elem);
        glBegin(GL_LINE_LOOP);
            glVertex3f(-1.f,-1.f,0.f);
            glVertex3f(1.f,-1.f,0.f);
            glVertex3f(1.f,1.f,0.f);
            glVertex3f(-1.f,1.f,0.f);
        glEnd();
        glBegin(GL_POINTS);
            glVertex3f(-1.f,-1.f,0.f);
            glVertex3f(1.f,-1.f,0.f);
            glVertex3f(1.f,1.f,0.f);
            glVertex3f(-1.f,1.f,0.f);
        glEnd();

        glDisable(GL_BLEND);
        glPopAttrib();

    }

    // restore OpenGL matrices
    restoreModelViewAndTextureMatrices();

    glEnable(GL_DEPTH_TEST);

	/*
    if (tc_)
        tc_->releaseTarget(dest, getTargetConfig(ttImage_));
	*/
}

void OverviewRenderer::processMessage(Message* msg, const Identifier& dest) {

    SliceRendererBase::processMessage(msg, dest);

    if (msg->id_ == setVolumeContainer_)
        showOverview();
    else if (msg->id_ == setCurrentDataset_)
        showOverview();

}

void OverviewRenderer::setTranslation(const tgt::vec2& translation) {

    // minimum/maximum translation should center the slice plate's corners on canvas
    float minTranslationX = -1.f;
    float maxTranslationX = 1.f;
    float minTranslationY = -1.f;
    float maxTranslationY = 1.f;

    trans_.x = std::min(std::max(translation.x, minTranslationX), maxTranslationX);
    trans_.y = std::min(std::max(translation.y, minTranslationY), maxTranslationY);

    if (dragging_) {
        realTrans_ = trans_;
        if (snapToGrid_) {
            int slice_x, slice_y;
            int dist = distanceToNearestCenter(slice_x, slice_y);
            int canvasDim = std::max(int(getSize().x), int(getSize().y));
            if (dist <= canvasDim*draggingSnapFactor_) {
                snapToGrid();
            }
        }
    }

}

void OverviewRenderer::addTranslation(const tgt::vec2& translation, bool canvasTranslation) {

    vec2 scaledTranslation = translation;
    // synchronize mouse motion over canvas with slice plate translation
    // (=> mouse cursor does not move relatively to slice plate)
    if (canvasTranslation) {
        scaledTranslation.x *= 2.f/(getSize().x*scaleVector_.x*scaleFactor_);
        scaledTranslation.y *= 2.f/(getSize().y*scaleVector_.y*scaleFactor_);
    }

    if (dragging_)
        setTranslation(realTrans_+scaledTranslation);
    else
        setTranslation(trans_+scaledTranslation);


}

void OverviewRenderer::setScale(float scale) {
    // maximum zoom-out should show the complete slice plate
    float minValue = 1.f;
    // maximum zoom-in should shows 1/3 of a slice
    float maxValue = std::min(numX_/scaleVector_.x, numY_/scaleVector_.y)*3.f;
    scaleFactor_ = std::min(std::max(scale, minValue), maxValue);
}

void OverviewRenderer::scale(float scale) {
    setScale(scaleFactor_*scale);
}

void OverviewRenderer::snapToGrid() {

    // column / row indices of the slice currently nearest to the canvas' center
    int slice_x = int( (1.f-trans_.x) / (2.f/numX_) );
    int slice_y = int( (1.f-trans_.y) / (2.f/numY_) );

    trans_.x = 1.f -( slice_x + 1/2.f)/(static_cast<float>(numX_)/2.f);
    trans_.y = 1.f -( slice_y + 1/2.f)/(static_cast<float>(numY_)/2.f);

}

void OverviewRenderer::showOverview() {

    scaleFactor_ = 1.f;
    trans_ = vec2(0.f);

}

void OverviewRenderer::showSlice(int sliceID) {

    if (sliceID > numX_*numY_ || sliceID < 0)
        return;

    // calc column/row indices on slice plate of the slice to be shown
    int slice_x = sliceID % numX_;
    int slice_y = sliceID / numY_;

    // calc world space translation to the slice, which should be shown
    trans_.x = 1.f - (slice_x + 1/2.f)/(numX_/2.f);
    trans_.y = (slice_y + 1/2.f)/(numY_/2.f) - 1.f;

    // set scale factor to maximum
    scaleFactor_ = std::min(numX_/scaleVector_.x, numY_/scaleVector_.y)*0.99f;
}

void OverviewRenderer::showSlice(const tgt::ivec2 &canvasCoordinates) {

    // shift slice plate so that the passed canvas coordinates are centered
    ivec2 canvasDim = getSize();
    vec2 shift = vec2(static_cast<float>(canvasDim.x/2 - canvasCoordinates.x),
        static_cast<float>(canvasCoordinates.y - canvasDim.y/2));
    addTranslation(shift, true);

    // set scale factor to maximum
    scaleFactor_ = std::min(numX_/scaleVector_.x, numY_/scaleVector_.y)*0.99f;

    // finally center slice
    snapToGrid();

}

void OverviewRenderer::startDragging(bool snapToGrid, float snapFactor) {

    dragging_ = true;
    snapToGrid_ = snapToGrid;
    // do not snap to grid if user has zoomed into a slice
    if (scaleFactor_ > std::min(numX_/scaleVector_.x, numY_/scaleVector_.y)*1.1)
        snapToGrid_ = false;
    draggingSnapFactor_ = snapFactor;
    realTrans_ = trans_;
}

void OverviewRenderer::stopDragging() {

    dragging_ = false;
    realTrans_ = tgt::vec2(0.f);

}

void OverviewRenderer::calcScaleVector() {

    if ( (currentVolumeHandle_ == 0) || (currentVolumeHandle_->getVolume() == 0) )
        return;

    ivec3 dim = currentVolumeHandle_->getVolume()->getDimensions();
    ivec2 canvasDim = getSize();

    sliceDiagonal_ = int(sqrt(double(dim.x*dim.x + dim.y*dim.y)));

    // calc number of slice per column / row
    int k = dim.z;
    numY_ = int( sqrt( (double(k)*dim.x*canvasDim.y)/(dim.y*canvasDim.x)) );
    numX_ = int( ceil(float(k)/numY_) );

    // calc scale vector for slice plate in order to scale the quad (-1,-1),(1,1) to correct
    // aspect ratio of the slices
    ivec2 slicePlateDim = ivec2(numX_*dim.x, numY_*dim.y);
    scaleVector_ = vec3(1.f,
        (static_cast<float>(slicePlateDim.y)*canvasDim.x) / (static_cast<float>(slicePlateDim.x)*canvasDim.y),
        1.f);
    scaleVector_ *= 0.99f/std::max(scaleVector_.x, scaleVector_.y);
}

int OverviewRenderer::distanceToNearestCenter(int &slice_x, int &slice_y) {

    // slice currently nearest to the center
    slice_x = int( (1.f-trans_.x) / (2.f/numX_) );
    slice_y = int( (1.f-trans_.y) / (2.f/numY_) );

    // distance between canvas center and slice center in world coordinates
    float distWorld_x = (1.f - trans_.x) - ( ( slice_x + 1/2.f ) / (static_cast<float>(numX_)/2.f) );
    float distWorld_y = (1.f - trans_.y) - ( ( slice_y + 1/2.f ) / (static_cast<float>(numY_)/2.f) );

    // distance between canvas center and slice center in pixel coordinates
    vec2 canvasDim = getSize();
    float dist_x = distWorld_x*scaleVector_.x/2.f * canvasDim.x;
    float dist_y = distWorld_y*scaleVector_.y/2.f * canvasDim.y;
    int dist = int( sqrt( double(dist_x*dist_x + dist_y*dist_y) ) * scaleFactor_ );

    return dist;
}

void OverviewRenderer::getSliceCoords(int sliceID, vec2 &ll, vec2 &ur) {

    if( currentVolumeHandle_ == 0 )
        return;

    Volume* volume = currentVolumeHandle_->getVolume();
    if( volume == 0 )
        return;

    tgtAssert(sliceID >= 0 &&
        sliceID < volume->getDimensions().z, "slice index out of range" );

    int row = sliceID / numX_;
    int column = sliceID % numX_;

    ll.x = (column * 1.f/numX_)*2.f - 1.f;
    ll.y = ((numY_ - row - 1) * 1.f/numY_)*2.f - 1.f;

    ur = ll + vec2(2.f/numX_, 2.f/numY_);
}

//------------------------------------------------------------------------------

SingleSliceRenderer::SingleSliceRenderer(Alignment alignment)
    : SliceRendererBase()
    , alignment_(alignment)
{
    // calculate direction of width, height, depth respectively
    xyz_ = (tgt::Vector3<size_t>(0, 1, 2) + size_t(alignment_)) % size_t(3);

	std::vector<std::string> alignments;
	alignments.push_back("AXIAL");
	alignments.push_back("SAGITTAL");
	alignments.push_back("CORONAL");


	alignmentProp_ = new EnumProp("alignment.changed","Alignment",alignments,0,false);
	addProperty(alignmentProp_);
}

const std::string SingleSliceRenderer::getProcessorInfo() const {
	return "This class renders a single slice parallel to the front, side or top of a data set.";
}

size_t SingleSliceRenderer::getNumSlices() const {
    return numSlices_;
}

void SingleSliceRenderer::setSliceIndex(size_t sliceIndex) {
    sliceIndex_ = sliceIndex;
}

// FIXME: obsoleted by removal of VolumeContainer.
/*
void SingleSliceRenderer::setVolumeContainer() {
    numSlices_ = getCurrentDataset()->getVolume()->getDimensions()[xyz_.z];
    sliceIndex_ = static_cast<int>(numSlices_/2.0);
}
*/

void SingleSliceRenderer::processMessage(Message* msg, const Identifier& ident) {
	SliceRendererBase::processMessage(msg,ident);

	if (msg->id_ == "alignment.changed") {
		int i= msg->getValue<int>();
		if (i==0) {
			alignment_ = SingleSliceRenderer::AXIAL;
		}
		else if (i==1) {
			alignment_ = SingleSliceRenderer::SAGITTAL;
		}
		else if (i==2) {
			alignment_ = SingleSliceRenderer::CORONAL;
		}
		xyz_ = (tgt::Vector3<size_t>(0, 1, 2) + size_t(alignment_)) % size_t(3);
	}
}

void SingleSliceRenderer::process(LocalPortMapping* portMapping) {
    if ( !ready() )
        return;
    // FIXME: obsolete due to removal of VolumeContainer. Migrate to new
    // volume concept. (dirk)
    //
    /*
	int datasetNumber = portMapping->getVolumeNumber("volume.dataset");
	if (datasetNumber != currentDataset_)
		postMessage(new IntMsg(setCurrentDataset_,datasetNumber));
    */
    if (tc_) {
        std::ostringstream oss;
        oss << "SingleSliceRenderer::render(dataset=" << currentVolumeHandle_ << ") dest";
        int dest = portMapping->getTarget("image.outport");
        tc_->setActiveTarget(dest);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // init stuff
    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();
    if( volumeGL == 0 )
        return;

    setupTextures();
    // volTexUnit_ is now active

    setupShader();

    // save matrices
    saveModelViewAndTextureMatrices();

    for (size_t i = 0; i < volumeGL->getNumTextures(); ++i) {
        const VolumeTexture* tex = volumeGL->getTexture(i);

        // get useful volume texture data
        vec3 llb = tex->getLLF();
        vec3 urf = tex->getURB();
        llb.z = tex->getURB().z; // convert from llf to llb
        urf.z = tex->getLLF().z; // convert from urb to urf
        float cubeSize = tex->getCubeSize()[xyz_.z];

        // calculate depth in llb/urf space
        float depth = ( float(sliceIndex_) / float(numSlices_ - 1) - 0.5f )
            * volumeGL->getVolume()->getCubeSize()[xyz_.z];

        // check whether the given slice is not within tex
        if ( depth < llb[xyz_.z] || depth > urf[xyz_.z])
            continue;

        // everything fine so map depth to [0, 1]
        depth -= tex->getCenter()[xyz_.z]; // center around origin
        depth /= cubeSize;                 // map to [-0.5, -0.5]
        depth += 0.5f;                     // map to [0, 1]

        float canvasRatio = static_cast<float>(size_.x)/static_cast<float>(size_.y);

        glMatrixMode(GL_TEXTURE);
            tgt::loadMatrix( tex->getMatrix() );

        glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            // second: scale image to keep aspect ratio
            if (canvasRatio > 1.f)
                glScalef(1.f/canvasRatio, 1.f, 1.f); // scale in x direction with the inverse value
            else
                glScalef(1.f, canvasRatio, 1.f);     // scale in y direction

            // first: scale and rotate to get proper viewing
            static float  angleLookUp[3] = {0.f, 180.f, 90.f};  // AXIAL, SAGITTAL, CORONAL
            static float yScaleLookUp[3] = {-1.f, -1.f, -1.f};  // AXIAL, SAGITTAL, CORONAL
            glRotatef(angleLookUp[alignment_], 0.f, 0.f, 1.f);
            glScalef(1.f, yScaleLookUp[alignment_], 1.f);

        /*
            setup tex coords
        */
        // calculate remaining indices

        vec3 tc[4];
        tc[0][xyz_.x] = 0.f; tc[0][xyz_.y] = 1.f; tc[0][xyz_.z] = depth;
        tc[1][xyz_.x] = 0.f; tc[1][xyz_.y] = 0.f; tc[1][xyz_.z] = depth;
        tc[2][xyz_.x] = 1.f; tc[2][xyz_.y] = 0.f; tc[2][xyz_.z] = depth;
        tc[3][xyz_.x] = 1.f; tc[3][xyz_.y] = 1.f; tc[3][xyz_.z] = depth;

        /*
            setup up vertices
        */
        vec2 vc[4];
        vc[0].x = llb[xyz_.x]; vc[0].y = urf[xyz_.y];
        vc[1].x = llb[xyz_.x]; vc[1].y = llb[xyz_.y];
        vc[2].x = urf[xyz_.x]; vc[2].y = llb[xyz_.y];
        vc[3].x = urf[xyz_.x]; vc[3].y = urf[xyz_.y];

        /*
            perform the actual rendering
        */
        tex->bind();
        glBegin(GL_QUADS);
        for (size_t j = 0; j < 4; ++j) {
            tgt::texCoord( tc[j] );
            tgt::vertex( vc[j] );
        }
        glEnd();
    } // for

    // restore matrices
    restoreModelViewAndTextureMatrices();

    deactivateShader();

}

//------------------------------------------------------------------------------

const Identifier MultimodalSingleSliceRenderer::transFuncTexUnit1_("transFuncTexUnit1");
const Identifier MultimodalSingleSliceRenderer::volTexUnit1_("volTexUnit1");

MultimodalSingleSliceRenderer::MultimodalSingleSliceRenderer(Alignment alignment)
    : SingleSliceRenderer(alignment)
{
    std::vector<Identifier> units;
    units.push_back(transFuncTexUnit_);
    units.push_back(transFuncTexUnit1_);
    units.push_back(volTexUnit_);
    units.push_back(volTexUnit1_);
	tm_.registerUnits(units);

	transferFunc1_ = new TransFuncIntensity();
}

void MultimodalSingleSliceRenderer::process(LocalPortMapping* localPortMapping) {
    if ( !ready() )
        return;

    // FIXME: implement port mapping for new volume concept here (dirk)

    if (tc_) {
        std::ostringstream oss;
        oss << "MultimodalSingleSliceRenderer::render(dataset=" << currentVolumeHandle_ << ") dest";
        int dest = localPortMapping->getTarget("image.outport");
        tc_->setActiveTarget(dest);
    }

    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    // init stuff
    VolumeGL* volumeCT = currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_CT);
    VolumeGL* volumePET = currentVolumeHandle_->getRelatedVolumeGL(Modality::MODALITY_PET);
    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit_));
    transferFunc_->bind();
    glActiveTexture(tm_.getGLTexUnit(transFuncTexUnit1_));
    transferFunc1_->bind();

    // if we don't have hardware support -> return
    if (!GpuCaps.areShadersSupported()) return;

    //if (!transferFuncShader_)
        transferFuncShader_ = ShdrMgr.load("sr_multimodal", buildHeader(), false);

    // activate if everything went fine and set the needed uniforms
    if (transferFuncShader_) {
        transferFuncShader_->activate();
        transferFuncShader_->setUniform("volumeCT_", tm_.getTexUnit(volTexUnit_));
        transferFuncShader_->setUniform("volumePET_", tm_.getTexUnit(volTexUnit1_));
        transferFuncShader_->setUniform("transferFuncCT_", tm_.getTexUnit(transFuncTexUnit_));
        transferFuncShader_->setUniform("transferFuncPET_", tm_.getTexUnit(transFuncTexUnit1_));
    }


    // save matrices
    saveModelViewAndTextureMatrices();

    if (volumePET) {
        for (size_t i = 0; i < volumeCT->getNumTextures(); ++i) {
            const VolumeTexture* tex = volumeCT->getTexture(i);
            const VolumeTexture* tex1 = volumePET->getTexture(i);

            // get useful volume texture data
            vec3 llb = tex->getLLF();
            vec3 urf = tex->getURB();
            llb.z = tex->getURB().z; // convert from llf to llb
            urf.z = tex->getLLF().z; // convert from urb to urf
            float cubeSize = tex->getCubeSize()[xyz_.z];

            // calculate depth in llb/urf space
            float depth = ( static_cast<float>(sliceIndex_) / static_cast<float>(numSlices_ - 1) - 0.5f )
                * volumeCT->getVolume()->getCubeSize()[xyz_.z];

            // check whether the given slice is not within tex
            if ( depth < llb[xyz_.z] || depth > urf[xyz_.z])
                continue;

            // everything fine so map depth to [0, 1]
            depth -= tex->getCenter()[xyz_.z]; // center around origin
            depth /= cubeSize;                 // map to [-0.5, -0.5]
            depth += 0.5f;                     // map to [0, 1]

            float canvasRatio = static_cast<float>(size_.x)/static_cast<float>(size_.y);

    /*
                glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
        glMatrixMode(GL_TEXTURE);
                tgt::loadMatrix( tex->getMatrix() );
                glActiveTexture(tm_.getGLTexUnit(volTexUnit1_));
        glMatrixMode(GL_TEXTURE);
                tgt::loadMatrix( tex1->getMatrix() );
    */
            glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                // second: scale image to keep aspect ratio
                if (canvasRatio > 1.f)
                    glScalef(1.f/canvasRatio, 1.f, 1.f); // scale in x direction with the inverse value
                else
                    glScalef(1.f, canvasRatio, 1.f);     // scale in y direction

                // first: scale and rotate to get proper viewing
                static float angleLookUp[3] = {0.f, 180.f, 90.f};  // AXIAL, SAGITTAL, CORONAL
                static float yScaleLookUp[3] = {-1.f, -1.f, -1.f};  // AXIAL, SAGITTAL, CORONAL
                glRotatef(angleLookUp[alignment_], 0.f, 0.f, 1.f);
                glScalef(1.f, yScaleLookUp[alignment_], 1.f);


                //setup tex coords

            // calculate remaining indices

            vec3 tc[4];
            tc[0][xyz_.x] = 0.f; tc[0][xyz_.y] = 1.f; tc[0][xyz_.z] = depth;
            tc[1][xyz_.x] = 0.f; tc[1][xyz_.y] = 0.f; tc[1][xyz_.z] = depth;
            tc[2][xyz_.x] = 1.f; tc[2][xyz_.y] = 0.f; tc[2][xyz_.z] = depth;
            tc[3][xyz_.x] = 1.f; tc[3][xyz_.y] = 1.f; tc[3][xyz_.z] = depth;

            /*
                setup up vertices
            */
            vec2 vc[4];
            vc[0].x = llb[xyz_.x]; vc[0].y = urf[xyz_.y];
            vc[1].x = llb[xyz_.x]; vc[1].y = llb[xyz_.y];
            vc[2].x = urf[xyz_.x]; vc[2].y = llb[xyz_.y];
            vc[3].x = urf[xyz_.x]; vc[3].y = urf[xyz_.y];

            /*
                perform the actual rendering
            */
            glActiveTexture(tm_.getGLTexUnit(volTexUnit_));
            tex->bind();

            glActiveTexture(tm_.getGLTexUnit(volTexUnit1_));
            tex1->bind();

            //glActiveTexture(tm_.getGLTexUnit(volTexUnit_));

            glBegin(GL_QUADS);
            for (size_t j = 0; j < 4; ++j) {
                tgt::texCoord( tc[j] );
                tgt::vertex( vc[j] );
            }
            glEnd();
        } // for
    }

    // restore matrices
    restoreModelViewAndTextureMatrices();

    deactivateShader();

}

//------------------------------------------------------------------------------
//RPTMERGE needs an extra setter for cutData to get a parameter-less constructor, +portmapping
CustomSliceRenderer::CustomSliceRenderer(const CutData* cutData)
    : SliceRendererBase()
    , cutData_(cutData)
{}

void CustomSliceRenderer::process(LocalPortMapping* localPortMapping) {
    if ( !ready() || camera_ == 0 )
        return;

    // FIXME: implement portmapping for new volume concept.

    Volume* volume = currentVolumeHandle_->getVolume();
    if( volume == 0 )
        return;

    mat4 m = camera_->getViewMatrix();
    vec3 transl = vec3(m.elemRowCol[0][3], m.elemRowCol[1][3], m.elemRowCol[2][3]); // get translation
    quat q = camera_->getQuat();
    mat4 mRot = generateMatrixFromQuat(q);

    ivec3 texSize = volume->getDimensions();

    // FIXME This must only be done once
    vec3 spacing = vec3(volume->getDimensions()) * volume->getSpacing();
    float maxdim = static_cast<float>( max(volume->getDimensions()) );
    vec3 ratio = maxdim / spacing;
    vec3 size = (spacing * 2.0f) / (static_cast<vec3>(texSize) * ratio);

    if (tc_) {
        std::ostringstream oss;
        oss << "CustomSliceRenderer::render(dataset=" << currentVolumeHandle_ << ") dest";
        int dest = localPortMapping->getTarget("image.outport");
        tc_->setActiveTarget(dest);
    }
    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    VolumeGL* volumeGL = currentVolumeHandle_->getVolumeGL();
	const VolumeTexture* tex = volumeGL->getTexture();

    //
    //FIXME: needed for stack view only? should be disabled in the general case.
    //
    // save current projection matrix
    m = tgt::getTransposeProjectionMatrix();
    glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLineWidth(6.f);
    glColor3ubv(cutData_->color_.elem);
    glBegin(GL_LINE_LOOP);
        glVertex3f(-1.f,  1.f, 0.f);
        glVertex3f( 1.f,  1.f, 0.f);
        glVertex3f( 1.f, -1.f, 0.f);
        glVertex3f(-1.f, -1.f, 0.f);
    glEnd();
    glColor3ub(255, 255, 255);
    glLineWidth(1.f);


    // restore projection matrix
    glMatrixMode(GL_PROJECTION);
    tgt::loadTransposeMatrix(m);

    setupTextures();
    // volTexUnit_ is now active

	tex->bind();

    setupShader();

    // let cutData_->cutPlane_.n point in a consistent direction
    plane p = cutData_->cutPlane_;
    p.n = tgt::abs(p.n);

    // angle between z-axis and normal 
    float angle_y_axis_rad = acosf(tgt::dot(p.n, vec3(0.f, 0.f, 1.f)));
    // convert to degree
    float angle_y_axis_deg = tgt::rad2deg(angle_y_axis_rad);

    // rotate normal around y-axis
    mat4 rotation_y = mat4::createRotationY(angle_y_axis_rad);
    vec3 rotated_normal = tgt::abs(rotation_y * p.n);

    // angle between z-axis and normal
    float angle_x_axis_rad = acosf(tgt::dot(rotated_normal, vec3(0.f, 0.f, 1.f)));
    // convert to degree
    float angle_x_axis_deg = tgt::rad2deg(angle_x_axis_rad);

    glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glTranslatef(0.5f, 0.5f, 0.5f);
        glScalef(1.f/size.x, 1.f/size.y, 1.f/size.z);
        tgt::multTransposeMatrix(mRot); // invert via transposing
    glMatrixMode(GL_MODELVIEW);
        glTranslatef(transl.x, transl.y, transl.z);
        glRotatef( angle_x_axis_deg, 1, 0, 0);
        glRotatef( angle_y_axis_deg, 0, 1, 0);

    glBegin(GL_POLYGON);
    for (size_t i = 0; i < cutData_->numVertices_; ++i) {
        vec3 tc = cutData_->cutPolygon_[i];
        tgt::texCoord(tc);
        tgt::vertex(tc);
    }
    glEnd();

    deactivateShader();

}

//------------------------------------------------------------------------------

//RPTMERGE needs an extra setter for cutData to get a parameter-less constructor
EmphasizedSliceRenderer3D::EmphasizedSliceRenderer3D(const CutData* cutData)
    : SliceRenderer3D()
    , cutData_(cutData)
{
}

void EmphasizedSliceRenderer3D::processMessage(Message* msg, const Identifier& dest) {
    SliceRenderer3D::processMessage(msg, dest);

    if (msg->id_ == "set.quat")
        quat_ = msg->getValue<quat>();
}


void EmphasizedSliceRenderer3D::calculateTransformation() {
    trans_ = vec3(0.f, 0.f, -3.f);
    // rotate 90° degrees further
	rot_ = generateMatrixFromQuat( quat_ * quat::createQuat(tgt::PIf * 0.5f, vec3(0.f, 1.f, 0.f)) );
}

void EmphasizedSliceRenderer3D::paint() {
    if( (currentVolumeHandle_ == 0) || (currentVolumeHandle_->getVolumeGL() == 0) )
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setupTextures();
    // volTexUnit_ is now active

    const VolumeTexture* volTex = currentVolumeHandle_->getVolumeGL()->getTexture();
    if( volTex == 0 )
        return;

    volTex->bind();

    vec3 cubeSize = volTex->getCubeSize();

    vec3 tcv[8]; // transformed cube vertices

    // transform cube vertices
    for (size_t i = 0; i < 8; ++i)
        tcv[i] = rot_ * volTex->getCubeVertices()[i];

    plane tcp[8]; // transformed cube planes
    plane::createCubePlanes(tcv, tcp);

    glMatrixMode(GL_TEXTURE);
        // see below for details
        glLoadIdentity();
        glTranslatef(0.5f, 0.5f, 0.5f);
        glScalef(1.f/cubeSize.x, 1.f/cubeSize.y, 1.f/cubeSize.z);
        tgt::multTransposeMatrix(rot_); // invert via transposing
    glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(trans_.x, trans_.y, trans_.z);

    vec3 vMin = *std::min_element(tcv, tcv + 8, findMinMax);
    vec3 vMax = *std::max_element(tcv, tcv + 8, findMinMax);

    // TODO numSlices_ to sliceDensity
    float depthIncr = (vMax.z - vMin.z) / (300 + 1);
    float depth = vMin.z + depthIncr;

    float visible = cutData_->cutPlane_.d * cutData_->cutPlane_.n.x;

    int dest;
    if (tc_) {
        std::ostringstream oss;
        oss << "EmphasizedSliceRenderer3D::render(dataset=" << ") dest";
        dest = 0; // FIXME tc_->getTargetID(getTargetConfig(ttImage_), oss.str());
        tc_->setActiveTarget(dest);
    }

    glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

    // render all slices
    bool first = false;
    glColor4ub(64, 64, 64, 64);
// TODO numSlices_ to sliceDensity
    for (size_t i = 0; i < 300; ++i, depth += depthIncr) {
        if (depth > visible)
            continue;

        if (depth + depthIncr > visible)
            glColor4ub(255, 128, 128, 255);

        first = true;

        // build a polygon huge enough
        vec3 polygon[6] = {
            vec3(-100.f, -100.f, depth),
            vec3(-100.f,  100.f, depth),
            vec3( 100.f,  100.f, depth),
            vec3( 100.f, -100.f, depth),
            vec3(),
            vec3()
        };
        size_t numIn = 4;

        // clip against all 6 planes
        vec3 clippedPolygon[12];
        size_t numOut;

        vec3* in = &polygon[0];
        vec3* out = &clippedPolygon[0];

        for (size_t j = 0; j < 6; ++j) {
            tcp[j].clipPolygon(in, numIn, out, numOut);
            // swap
            vec3* tmp = in;
            in = out;
            out = tmp;
            numIn = numOut;
        }

        // render a slice
        glBegin(GL_POLYGON);
        for (size_t j = 0; j < numIn; ++j) {
            /*
                calculate by transforming the polygon-vertex back
                to the orignal coordinate system:
                    vec3 tc = (inv * polygon[j]) / volSize_ + 0.5f;
                This is done by the texture matrix.
            */
            vec3 tc = polygon[j];
            glTexCoord3fv(tc.elem);
            glVertex3fv(tc.elem);
        }
        glEnd();
    }

    // Now draw first hit line
    deactivateShader();

    glColor4ub(255, 0, 0, 128);
    tgt::col4* buffer = new tgt::col4[size_t(size_.x)];
    if (tc_) {//tc_ version is not tested!!!
        //read whole image
        float* values = tc_->getTargetAsFloats(dest);
        //copy only needed values
        //actual line
        int line = int(size_.y * (mouseY_ + 1.f) * 0.5f + 0.5f);
        for (int i = 0; i < int(size_.x); i += 4) {//skip 4 because rgba was read
            buffer[i].r = (uint8_t)values[line*int(size_.x)+i];
            buffer[i].g = (uint8_t)values[line*int(size_.x)+i+1];
            buffer[i].b = (uint8_t)values[line*int(size_.x)+i+2];
            buffer[i].a = (uint8_t)values[line*int(size_.x)+i+3];
        }
        delete[] values;
    }
    else {
        glReadPixels(
            0,
            int(size_.y * (mouseY_ + 1.f) * 0.5f + 0.5f),
            int(size_.x),
            1, GL_RGBA, GL_UNSIGNED_BYTE, buffer
        );
    }

    glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glPointSize(5.f);
    glColor4ub(255, 0, 0, 128);

    int hit;
    for (hit = 0; hit < size_.x; ++hit) {
        if (buffer[hit].r != 0 || buffer[hit].g != 0 || buffer[hit].b != 0)
            break;
    }
    // hit now holds the index of the first hit

    glBegin(GL_POINTS);
        glVertex3f(
            -2.f * (size_.x/size_.y) + (float(hit)/size_.x) * 4.f * (size_.x/size_.y),
            mouseY_ * 2.f,
            -2.f);
    glEnd();

    // clean up
    delete[] buffer;

    glPointSize(1.f);
    glColor4ub(255, 255, 255, 255);

}

} // namespace voreen
