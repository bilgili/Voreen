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

#ifndef VRN_PROPERTYCONTAINER_H
#define VRN_PROPERTYCONTAINER_H

#include "voreen/core/processors/processor.h"

class PropertyVector;

namespace voreen {

class PropertyContainer : public Processor {
public:
    PropertyContainer();
    virtual ~PropertyContainer();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "PropertyContainer"; }
    virtual std::string getCategory() const     { return "Utility"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }
    virtual bool isUtility() const              { return true; }

    virtual std::string getProcessorInfo() const;
    void addNewProperty(Property*);

protected:
    virtual void process();

    PropertyVector* propertyVector_;
    std::vector<Property*> properties_;

};

} // namespace

#endif // VRN_PROPERTYCONTAINER_H

