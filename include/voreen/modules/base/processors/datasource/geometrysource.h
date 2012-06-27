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

#ifndef VRN_GEOMETRYSOURCE_H
#define VRN_GEOMETRYSOURCE_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

namespace voreen {

/**
 * Reads a point list or point segment list from a text file
 * and provides it as geometry through its outport.
 */
class GeometrySource : public Processor {
public:
    GeometrySource();
    virtual ~GeometrySource();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "GeometrySource";  }
    virtual std::string getCategory() const  { return "Data Source";     }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    void readPointList();

    FileDialogProperty geometryFile_;   ///< filename of the file containing the point information
    StringOptionProperty geometryType_; ///< Determines whether the geometry is read as point or segment list.
    IntProperty skipItemCount_;         ///< data item to skip in the file after reading each point

    GeometryPort outportPointList_;

    static const std::string loggerCat_;
};

}   //namespace

#endif // VRN_GEOMETRYSOURCE_H
