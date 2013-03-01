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

//#define NUMCOEFFSSQRT 6
#define NUMCOEFFSSQRT 4
#define NUMCOEFFS NUMCOEFFSSQRT*NUMCOEFFSSQRT

#include "shclass.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/utils/voreenpainter.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/cameraproperty.h"

namespace voreen {

SHClass::SHClass(VolumeRaycaster* rc, TransFuncProperty* tf)
    : rc_(rc)
    , tf_(tf)
    , currentVolume_(0) {

    tf_->onChange(CallMemberAction<SHClass>(this, &SHClass::onTransFuncChange));

    bleedModeProp_ = new StringOptionProperty("sh-transfer", "Transfer Mode", Processor::INVALID_PROGRAM);
    bleedModeProp_->addOption("unshadowed", "Unshadowed transfer");
    bleedModeProp_->addOption("shadowed", "Shadowed transfer");
    bleedModeProp_->addOption("shadowed-direct", "Direct bleeding");
    bleedModeProp_->addOption("shadowed-interreflect", "Interreflection");
    bleedModeProp_->addOption("shadowed-subsurface", "Subsurface Scattering");
    bleedModeProp_->addOption("shad-bleed-sub", "Subsurf + Bleed");
    bleedModeProp_->select("shadowed");

    bleedModeProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::onTransCoeffChange));
    coeffSizeFactor_ = new FloatProperty("set.coeffSizeFac", "Coeff. Vol. Size", 1.f, 0.1f, 1.f);
    numSampleProp_ = new IntProperty("set.numSamples", "Number of Samples", 400, 20, 1000);
    recalcProp_ = new ButtonProperty("recalc.shCoeffs", "Recalculate");
    recalcProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::onTransCoeffChange));

    considerNormalsProp_ = new BoolProperty("set.consNorms", "Consider Normals for Visibility", false, Processor::INVALID_PARAMETERS);
    considerNormalsProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::onTransCoeffChange));
    eriProp_ = new BoolProperty("set.eri", "Use exp. ray increase", true, Processor::INVALID_PARAMETERS);
    eriProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::onTransCoeffChange));
    scaleProp_ = new IntProperty("set.scale", "Dataset sidelength in mm (for SSS)", 200, 20, 1000);

    lpFilename_ = new FileDialogProperty("set.lpFN", "Select Light Function", "Select a Lightfunction file", "../../data/lightprobes", "Lightfunctions(*.lsc *.lpc)");
    lpFilename_->onChange(CallMemberAction<SHClass>(this, &SHClass::onLightFuncChange));
    //lightRotProp_ = new BoolProperty("set.lightnavi", "Rotate Light-Function", false, Processor::VALID);
    //lightRotProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::onNavigationChange));

    interactionProp_ = new BoolProperty("set.shinteraction", "No Recalc for TF change", false, Processor::VALID);
    interactionProp_->onChange(CallMemberAction<SHClass>(this, &SHClass::setSHInteraction));

    // this is not very nice. We still need this to use the trackball math, although we don't want to control a camera at all. FL
    cameraDummy_ = new CameraProperty("lightPropDummy", "Light Property", tgt::Camera(tgt::vec3(0.f, 0.f, 3.5f), tgt::vec3(0.f, 0.f, 0.f), tgt::vec3(0.f, 1.f, 0.f)));
    lightHandler_  = new LightInteractionHandler(this, cameraDummy_);

    transCalc_ = 0;

    interactiveMode_ = false;

    props_ = std::vector<Property*>();
    props_.push_back(bleedModeProp_);
    props_.push_back(coeffSizeFactor_);
    props_.push_back(numSampleProp_);
    props_.push_back(recalcProp_);
    props_.push_back(considerNormalsProp_);
    props_.push_back(eriProp_);
    props_.push_back(scaleProp_);
    props_.push_back(lpFilename_);
    //props_.push_back(lightRotProp_);
    props_.push_back(interactionProp_);
}

SHClass::~SHClass() {
    delete bleedModeProp_;
    delete coeffSizeFactor_;
    delete numSampleProp_;
    delete lpFilename_;
    delete considerNormalsProp_;
    delete scaleProp_;
    //delete lightRotProp_;
    delete eriProp_;
    delete recalcProp_;

    delete transCalc_;
    delete lightHandler_;
    delete interactionProp_;
    delete cameraDummy_;
}

void SHClass::initialize() throw (tgt::Exception) {
    ShdrMgr.addPath(VoreenApplication::app()->getModulePath("advancedlighting") + "/glsl");
}

void SHClass::initAndCalcCoeffs(const VolumeBase* handle) {

    if(!handle || handle == currentVolume_)
        return;

    currentVolume_ = handle;
    onTransCoeffChange(); //-> includes call to calcCoeffs()
}

void SHClass::onTransFuncChange() {

    if(!currentVolume_)
        return;

    if(!interactiveMode_) {
        // As soon as there is a change in the transfer-function, we have to recalculate the transfer-Coefficients for
        // our Volume (as visibility / selfshadowing has most likely changed).
        if(transCalc_)
            transCalc_->generateCoeffs();

        rc_->invalidate(Processor::INVALID_PARAMETERS);
    }
}

void SHClass::onLightFuncChange() {

    if(!currentVolume_)
        return;

    std::string fn = lpFilename_->get();
    if(fn.empty()) {
        fn = VoreenApplication::app()->getModulePath("advancedlighting") + "/lightprobes/standard.lsc";
    } else {
        std::ostringstream o;
        o << (VoreenApplication::app()->getModulePath("advancedlighting") + "/lightprobes") << "/" << tgt::FileSystem::fileName(fn);
        fn = o.str();
    }

    if(!tgt::FileSystem::fileExists(fn))
        return;

    SHLightFunc* lightFuncTrans = 0;

    if(tgt::FileSystem::fileExtension(fn, true) == "lpc") {
        if(transCalc_)
            lightFuncTrans = new SHLightProbe(4, std::string("Trans"), fn, bleedModeProp_->get() == "unshadowed" ? true : false);
        lpFilename_->set(fn);
    } else if(tgt::FileSystem::fileExtension(fn, true) == "lsc") {
        if(transCalc_)
            lightFuncTrans = new SHScalarFunc(4, std::string("Trans"), fn, bleedModeProp_->get() == "unshadowed" ? true : false);
        lpFilename_->set(fn);
    }

    if(transCalc_)
        transCalc_->setLightFunc(lightFuncTrans);

    rc_->invalidate(Processor::INVALID_PROGRAM);
}

void SHClass::onTransCoeffChange() {

    if(!currentVolume_)
        return;

    SHLightFunc* lf = 0;
    if(transCalc_)
        lf = transCalc_->getLightFunc();
    delete transCalc_;
    transCalc_ = 0;
    std::string bm = bleedModeProp_->get();

    SHCoeffTrans::BleedingMode bleedMode = SHCoeffTrans::SH_UNSHADOWED;
    if(bm == "shadowed")
        bleedMode = SHCoeffTrans::SH_SHADOWED;
    else if(bm == "shadowed-direct")
        bleedMode = SHCoeffTrans::SH_BLEEDING;
    else if(bm == "shadowed-interreflect")
        bleedMode = SHCoeffTrans::SH_REFLECT;
    else if(bm == "shadowed-subsurface")
        bleedMode = SHCoeffTrans::SH_SUBSURFACE;
    else if(bm == "shad-bleed-sub")
        bleedMode = SHCoeffTrans::SH_BLEED_SUB;

    int rootNum = int(sqrt(static_cast<float>(numSampleProp_->get())));
    float sizefac = coeffSizeFactor_->get();

    transCalc_ = new SHCoeffTrans(rootNum, 4, tf_, currentVolume_,
                                  bleedMode, sizefac, considerNormalsProp_->get(), eriProp_->get(), (float)scaleProp_->get());

    transCalc_->init();
    if(lf)
        transCalc_->setLightFunc(lf);
    else
        onLightFuncChange();

    bool oldinter = interactiveMode_;
    interactiveMode_ = false;
    //this results in the actual recalculation of the coefficients and forces a repaint
    onTransFuncChange();
    interactiveMode_ = oldinter;
}

//void SHClass::onNavigationChange() {
    //lightHandler_->setEnabled(lightRotProp_->get());
//}

void SHClass::updateLight(const tgt::mat4& diff) {
    if(transCalc_)
        transCalc_->getLightFunc()->updateRotation(diff);

    rc_->invalidate(Processor::INVALID_PARAMETERS);
}

void SHClass::setSHInteraction() {
    if (interactionProp_->get())
        interactiveMode_ = true;
    else {
        interactiveMode_ = false;
        onTransFuncChange();
    }
}

void SHClass::setRCShaderUniforms(tgt::Shader* rcPrg) {
    if(transCalc_)
        transCalc_->setRCShaderUniforms(rcPrg);
}

std::string SHClass::getShaderDefines() const {
    std::string headerSource = std::string("");

    if(transCalc_)
        headerSource += transCalc_->getShaderDefines();

    return headerSource;
}

} // namespace

