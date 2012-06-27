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

#ifndef VRN_BENCHMARK_H
#define VRN_BENCHMARK_H

#include "voreen/core/vis/processors/networkevaluator.h"

#include <string>
#include <vector>

namespace voreen {

/**
 * Wrapper around Processor::process() calls which times its execution time. It calls glFlush()
 * before and after process() to ensure all OpenGL operations are timed correctly.
 */
class ProcessBenchmark : public NetworkEvaluator::ProcessWrapper {
public:
    /**
     * Constructor.
     * @param Name of a processor which should be timed.
     */
    ProcessBenchmark(const std::string& processorName = "");

    /**
     * At a processor for which timing information should be acquired.
     * @name Name of the processor
     */
    void addProcessorName(const std::string& name);

    void processMessage(Message* msg, const Identifier& dest);

protected:
    std::vector<std::string> processorNames_;
    uint64_t startTicks_;

    uint64_t totalTime_;
    int frameCount_;

    void beforeProcess(Processor* p);
    void afterProcess(Processor* p);

    uint64_t getTicks();

    static const std::string loggerCat_;
};

} // namespace

#endif //VRN_BENCHMARK_H
