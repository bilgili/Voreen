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

#ifndef VRN_RANDOMWALKER_H
#define VRN_RANDOMWALKER_H

#include "voreen/core/processors/volumeprocessor.h"
#include "voreen/core/ports/geometryport.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/optionproperty.h"


#include "voreen/core/datastructures/geometry/pointsegmentlistgeometry.h"

#include "voreen/core/processors/cache.h"

#include "voreen/core/utils/voreenblas/voreenblascpu.h"
#ifdef VRN_MODULE_OPENMP
#include "modules/openmp/include/voreenblasmp.h"
#endif
#ifdef VRN_MODULE_OPENCL
#include "modules/opencl/utils/voreenblascl.h"
#endif

#include <string>

namespace voreen {

class Volume;
class RandomWalkerSolver;
class RandomWalkerSeeds;
class RandomWalkerWeights;

/**
 * Performs a semi-automatic volume segmentation using the 3D random walker algorithm.
 * User manual: http://voreen.uni-muenster.de/?q=random-walker
 *
 * @see RandomWalkerSolver
 */
class RandomWalker : public VolumeProcessor {
public:
    RandomWalker();
    virtual ~RandomWalker();
    virtual Processor* create() const;

    virtual std::string getCategory() const             { return "Volume Processing"; }
    virtual std::string getClassName() const            { return "RandomWalker";      }
    virtual Processor::CodeState getCodeState() const   { return CODE_STATE_TESTING;  }

    virtual void invalidate(int inv = INVALID_RESULT);
    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Performs a semi-automatic volume segmentation using the 3D random walker algorithm. "
                       "<p>See: <a href=\"http://voreen.uni-muenster.de/?q=random-walker\" >voreen.uni-muenster.de/?q=random-walker</a></p>");
    }

    virtual void process();
    virtual void beforeProcess();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

private:
    RandomWalkerSolver* computeRandomWalkerSolution();

    void getSeedListsFromPorts(PointSegmentListGeometry<tgt::vec3>& foregroundSeeds,
        PointSegmentListGeometry<tgt::vec3>& backgroundSeeds) const;

    RandomWalkerWeights* getEdgeWeightsFromProperties() const;

    const VoreenBlas* getVoreenBlasFromProperties() const;

    void putOutSegmentation(const RandomWalkerSolver* solver);
    void putOutProbabilities(const RandomWalkerSolver* solver);
    void putOutEdgeWeights(const RandomWalkerSolver* solver);

    void computeButtonClicked();
    void segmentationPropsChanged();
    void lodMinLevelChanged();
    void lodMaxLevelChanged();
    void updateGuiState();
    void clearCache();

    VolumePort inportVolume_;
    GeometryPort inportForegroundSeeds_;
    GeometryPort inportBackgroundSeeds_;
    VolumePort inportForegroundSeedsVolume_;
    VolumePort inportBackgroundSeedsVolume_;
    VolumePort outportSegmentation_;
    VolumePort outportProbabilities_;
    VolumePort outportEdgeWeights_;

    ButtonProperty computeButton_;

    IntProperty beta_;
    IntProperty minEdgeWeight_;
    StringOptionProperty preconditioner_;
    IntProperty errorThreshold_;
    IntProperty maxIterations_;
    StringOptionProperty conjGradImplementation_;

    BoolProperty enableLevelOfDetail_;
    IntProperty lodMinLevel_;
    IntProperty lodMaxLevel_;
    FloatProperty lodForegroundSeedThresh_;
    FloatProperty lodBackgroundSeedThresh_;
    IntOptionProperty lodSeedErosionKernelSize_;
    IntVec3Property lodMinResolution_;
    IntVec3Property lodMaxResolution_;

    BoolProperty enableClipping_;
    IntProperty clipLeftX_;
    IntProperty clipRightX_;
    IntProperty clipFrontY_;
    IntProperty clipBackY_;
    IntProperty clipBottomZ_;
    IntProperty clipTopZ_;

    BoolProperty enableTransFunc_;
    TransFuncProperty edgeWeightTransFunc_;
    FloatProperty edgeWeightBalance_;

    FloatProperty foregroundThreshold_;
    BoolProperty resampleOutputVolumes_;

    BoolProperty useCaching_;
    ButtonProperty clearCache_;

    BoolProperty runOnInit_;

    VoreenBlasCPU voreenBlasCPU_;
#ifdef VRN_MODULE_OPENMP
    VoreenBlasMP voreenBlasMP_;
#endif
#ifdef VRN_MODULE_OPENCL
    VoreenBlasCL voreenBlasCL_;
#endif

    Cache cache_;
    std::vector<const Volume*> lodVolumes_;
    bool recomputeRandomWalker_;

    const VolumeRAM* currentInputVolume_;

    static const std::string loggerCat_; ///< category used in logging
};

} //namespace

#endif
