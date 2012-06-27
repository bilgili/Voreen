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

#ifndef VRN_DEPENDENCYLINKEVALUATORS_H
#define VRN_DEPENDENCYLINKEVALUATORS_H

#include "voreen/core/properties/link/dependencylinkevaluatorbase.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/volumehandleproperty.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

template<class T>
class DependencyLinkEvaluatorGeneric : public DependencyLinkEvaluatorBase {
public:
    virtual bool arePropertiesLinkable(const Property* p1, const Property* /*p2*/) const {
        return dynamic_cast<const T*>(p1);
    }
};

//----------------------------------------------

class DependencyLinkEvaluatorBool : public DependencyLinkEvaluatorGeneric<BoolProperty> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorBool"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorBool(); }
};

//----------------------------------------------

class DependencyLinkEvaluatorFileDialog : public DependencyLinkEvaluatorGeneric<FileDialogProperty> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorFileDialog"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorFileDialog(); }
};

////----------------------------------------------

class DependencyLinkEvaluatorInt : public DependencyLinkEvaluatorGeneric<IntProperty> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorInt"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorInt(); }
};

////----------------------------------------------

class DependencyLinkEvaluatorString : public DependencyLinkEvaluatorGeneric<StringProperty> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorString"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorString(); }
};

////----------------------------------------------

class DependencyLinkEvaluatorIntVec2 : public DependencyLinkEvaluatorGeneric<IntVec2Property> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorIntVec2"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorIntVec2(); }
};

////----------------------------------------------

class DependencyLinkEvaluatorIntVec3 : public DependencyLinkEvaluatorGeneric<IntVec3Property> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorIntVec3"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorIntVec3(); }
};

////----------------------------------------------

class DependencyLinkEvaluatorIntVec4 : public DependencyLinkEvaluatorGeneric<IntVec4Property> {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorIntVec4"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorIntVec4(); }
};

//----------------------------------------------

///Special DependancyLinkEvaluator for VolumeHandle that deletes entries in the history if the corresponding VolumeHandle is deleted.
class DependencyLinkEvaluatorVolumeHandle : public DependencyLinkEvaluatorGeneric<VolumeHandleProperty>, public VolumeHandleObserver {
public:
    virtual std::string getClassName() const { return "DependancyLinkEvaluatorVolumeHandle"; }
    virtual LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorVolumeHandle(); }

    virtual void eval(Property* src, Property* dest) throw (VoreenException);
    virtual void propertiesChanged(Property* src, Property* dst);

    void volumeChange(const VolumeHandle*);
    void volumeHandleDelete(const VolumeHandle* source);
};

} // namespace

#endif // VRN_DEPENDENCYLINKEVALUATORS_H
