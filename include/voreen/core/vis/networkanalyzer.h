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

#ifndef VRN_NETWORKANALYZER_H
#define VRN_NETWORKANALYZER_H

#include "voreen/core/vis/property.h"
#include "voreen/core/vis/processors/image/postprocessor.h"
#include "voreen/core/vis/processors/image/copytoscreenrenderer.h"
#include "voreen/core/vis/processors/image/geometryprocessor.h"
#include "voreen/core/vis/voreenpainter.h"

namespace voreen {

/**
 * Class containing methods for rebuilding the pipeline.
 */
class NetworkAnalyzer {
public:

    /**
     * Returns all ConditionProp's in the pipeline
     */
    static std::pair<std::vector<ConditionProp*>, std::vector<GroupProp*> > findCondProps(std::vector<Processor*> rend);

    /**
     * Maps all grouped or conditioned props in the pipeline to the correct ConditionProp's or GroupProp's
     */
    static void markCondProps(std::vector<Processor*> rend, std::vector<ConditionProp*> conds, std::vector<GroupProp*> groups);

    /**
     * Finds all ConditionProp's and GroupProp's in overlays
     * Maps conditioned or grouped props to the correct ConditionProp's and GroupProp's
     */
	static void connectOverlayProps(std::vector<VoreenPainterOverlay*> overlays);
protected:
    /**
     * Cycles through group_props and finds the correct GroupProp for prop and links them
     */
	static void associateGroupProps(std::vector<GroupProp*> group_props, Property* prop);

    /**
     * Cycles through cond_props and finds the correct ConditionProp for prop and links them
     */
	static void associateCondProps(std::vector<ConditionProp*> cond_props, Property* prop);

private:

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_NETWORKANALYZER_H
