/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/properties/link/corelinkevaluatorfactory.h"

//#include "voreen/core/properties/link/dependencylinkevaluators.h"
#include "voreen/core/properties/link/linkevaluatorboolinvert.h"
#include "voreen/core/properties/link/linkevaluatorid.h"
#include "voreen/core/ports/renderport.h"

namespace voreen {

CoreLinkEvaluatorFactory::CoreLinkEvaluatorFactory() {

    // -----------------
    //  primitive types
    // -----------------

    // id
    registerType(new LinkEvaluatorBoolId());
    registerType(new LinkEvaluatorIntId());
    registerType(new LinkEvaluatorFloatId());
    registerType(new LinkEvaluatorDoubleId());

    // id conversion
    registerType(new LinkEvaluatorDoubleFloatId());
    registerType(new LinkEvaluatorDoubleIntId());
    registerType(new LinkEvaluatorDoubleBoolId());
    registerType(new LinkEvaluatorFloatIntId());
    registerType(new LinkEvaluatorFloatBoolId());
    registerType(new LinkEvaluatorIntBoolId());

    // id normalized
    registerType(new LinkEvaluatorIntIdNormalized());
    registerType(new LinkEvaluatorFloatIdNormalized());
    registerType(new LinkEvaluatorDoubleIdNormalized());

    // id normalized conversion
    registerType(new LinkEvaluatorDoubleFloatIdNormalized());
    registerType(new LinkEvaluatorDoubleIntIdNormalized());
    registerType(new LinkEvaluatorFloatIntIdNormalized());

    registerType(new LinkEvaluatorBoolInvert());


    // -----------------
    //  vector types
    // -----------------

    // id
    registerType(new LinkEvaluatorRenderSize());      //< specialized ivec2 id link for rendering sizes 

    registerType(new LinkEvaluatorIVec2Id());
    registerType(new LinkEvaluatorIVec3Id());
    registerType(new LinkEvaluatorIVec4Id());

    registerType(new LinkEvaluatorVec2Id());
    registerType(new LinkEvaluatorVec3Id());
    registerType(new LinkEvaluatorVec4Id());

    registerType(new LinkEvaluatorDVec2Id());
    registerType(new LinkEvaluatorDVec3Id());
    registerType(new LinkEvaluatorDVec4Id());

    // id conversion
    registerType(new LinkEvaluatorDVec2IVec2Id());
    registerType(new LinkEvaluatorDVec3IVec3Id());
    registerType(new LinkEvaluatorDVec4IVec4Id());

    registerType(new LinkEvaluatorDVec2Vec2Id());
    registerType(new LinkEvaluatorDVec3Vec3Id());
    registerType(new LinkEvaluatorDVec4Vec4Id());

    registerType(new LinkEvaluatorVec2IVec2Id());
    registerType(new LinkEvaluatorVec3IVec3Id());
    registerType(new LinkEvaluatorVec4IVec4Id());

    // id normalized
    registerType(new LinkEvaluatorIVec2IdNormalized());
    registerType(new LinkEvaluatorIVec3IdNormalized());
    registerType(new LinkEvaluatorIVec4IdNormalized());

    registerType(new LinkEvaluatorVec2IdNormalized());
    registerType(new LinkEvaluatorVec3IdNormalized());
    registerType(new LinkEvaluatorVec4IdNormalized());

    registerType(new LinkEvaluatorDVec2IdNormalized());
    registerType(new LinkEvaluatorDVec3IdNormalized());
    registerType(new LinkEvaluatorDVec4IdNormalized());

    // id normalized conversion
    registerType(new LinkEvaluatorDVec2IVec2IdNormalized());
    registerType(new LinkEvaluatorDVec3IVec3IdNormalized());
    registerType(new LinkEvaluatorDVec4IVec4IdNormalized());

    registerType(new LinkEvaluatorDVec2Vec2IdNormalized());
    registerType(new LinkEvaluatorDVec3Vec3IdNormalized());
    registerType(new LinkEvaluatorDVec4Vec4IdNormalized());

    registerType(new LinkEvaluatorVec2IVec2IdNormalized());
    registerType(new LinkEvaluatorVec3IVec3IdNormalized());
    registerType(new LinkEvaluatorVec4IVec4IdNormalized());


    // --------------
    //  matrix types
    // --------------

    // id
    registerType(new LinkEvaluatorMat2Id());
    registerType(new LinkEvaluatorMat3Id());
    registerType(new LinkEvaluatorMat4Id());

    // id normalized
    registerType(new LinkEvaluatorMat2IdNormalized());
    registerType(new LinkEvaluatorMat3IdNormalized());
    registerType(new LinkEvaluatorMat4IdNormalized());


    // ------------------------
    //  non-numeric properties
    // ------------------------

    registerType(new LinkEvaluatorStringId());

    registerType(new LinkEvaluatorShaderId());

//    registerType(new LinkEvaluatorVolumeHandleId());

    registerType(new LinkEvaluatorCameraId());
    registerType(new LinkEvaluatorCameraOrientationId());
    registerType(new LinkEvaluatorCameraPosId());
    registerType(new LinkEvaluatorCameraLookId());
    registerType(new LinkEvaluatorCameraFocusId());

    registerType(new LinkEvaluatorTransFuncId());

    registerType(new LinkEvaluatorButtonId());

    //Dependency links:
    //registerType(new DependencyLinkEvaluator);

}

LinkEvaluatorBase* CoreLinkEvaluatorFactory::createEvaluator(const std::string& typeString) {
    LinkEvaluatorBase* evaluator = LinkEvaluatorFactory::createEvaluator(typeString);
    if (evaluator)
        return evaluator;

    // TODO: remove
    if (typeString == "LinkEvaluatorId") {
        LWARNINGC("voreen.CoreLinkEvaluatorFactory", "Legacy LinkEvaluatorId not supported anymore");
        return new LinkEvaluatorId();
    }
    else if (typeString == "LinkEvaluatorIdNormalized") {
        LWARNINGC("voreen.CoreLinkEvaluatorFactory", "Legacy LinkEvaluatorIdNormalized not supported anymore");
        return new LinkEvaluatorIdNormalized();
    }

    return 0;
}

} // namespace
