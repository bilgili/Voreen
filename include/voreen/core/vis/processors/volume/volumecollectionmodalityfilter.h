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

#ifndef VRN_SIMPLE_VOLUMECOLLECTIONMODALITYFILTER
#define VRN_SIMPLE_VOLUMECOLLECTIONMODALITYFILTER

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/vis/properties/optionproperty.h"
#include "voreen/core/volume/volumecollection.h"

namespace voreen {

class VolumeCollectionModalityFilter : public Processor {

public:
    VolumeCollectionModalityFilter();
    virtual ~VolumeCollectionModalityFilter();

    virtual Processor* create() const { return new VolumeCollectionModalityFilter(); }
    virtual std::string getCategory() const { return "Utility"; }
    virtual std::string getClassName() const { return "VolumeCollectionModalityFilter"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual const std::string getProcessorInfo() const;
    virtual bool isUtility() const { return true; }

    virtual void initialize() throw (VoreenException);
    virtual void invalidate(InvalidationLevel inv = INVALID_RESULT);
    virtual void process();

protected:
    OptionProperty<Modality*> modalityProp_;
    Modality currentModality_;
    VolumeCollection filteredCollection_;
    VolumeCollectionPort inport_;
    VolumeCollectionPort outport_;

    static const std::string loggerCat_;

private:
    void adjustFilteredCollection();

};

}   // namespace voreen

#endif
