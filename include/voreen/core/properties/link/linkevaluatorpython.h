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

#ifndef VRN_LINKEVALUATORPYTHON_H
#define VRN_LINKEVALUATORPYTHON_H

#include "voreen/core/properties/link/linkevaluatorbase.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include <string>
#include "tgt/camera.h"

namespace voreen {

/**
 * Modifies the source value by an arbitrary Python scripts and
 * assigns the result to the destination property.
 */
class LinkEvaluatorPython : public LinkEvaluatorBase {
public:
    LinkEvaluatorPython(const std::string& functionName);
    virtual ~LinkEvaluatorPython();

    /**
     * @see LinkEvaluatorBase::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see LinkEvaluatorBase::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Evaluate the Python script link and propagate the modified data.
     */
    virtual BoxObject eval(const BoxObject& sourceOld, const BoxObject& sourceNew, const BoxObject& targetOld, Property* src, Property* dest);

    /**
     * Set a new script to replace the currently used.
     */
    void setScript(const std::string& script);

    std::string getScript() const;

    std::string getFunctionName() const;

    virtual std::string name() const;

private:
    friend class XmlDeserializer;
    friend class LinkEvaluatorFactory;

    /**
     * Private default constructor for serialization purposes.
     */
    LinkEvaluatorPython();

    /**
     * This needs to be called before the instance of the linkevaluator can be used
     */
    void init(const std::string& functionName);
#ifdef VRN_WITH_PYTHON

    /**
     * Call this method to prepare the context for usage before evaluating
     */
    void prepareContext();

    /**
     * This method checks for python errors and throws a VoreenException if error is present
     */
    void checkPythonErrorState();
#endif
    std::string functionName_;
};

} // namespace

#endif // VRN_LINKEVALUATORPYTHON_H
