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

#include "voreen/core/ports/plotport.h"
#include "voreen/core/plotting/plotbase.h"

namespace voreen {


PlotPort::PlotPort(PortDirection direction, const std::string& name,
        bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
 : GenericPort<PlotBase>(direction, name, allowMultipleConnections, invalidationLevel)
{
    if (isOutport())
        setData(0);
}

PlotPort::~PlotPort() {
    if ((portData_) && (isOutport()) /*&& (portData_->getColumnCount() > 0)*/) {
        delete portData_;
        portData_ = 0;
    }
}

bool PlotPort::isReady() const {
    if (isOutport())
        return isConnected();
    else
        return (hasData() && getData());
}

void PlotPort::setData(PlotBase* handle, bool deletePrevious) {
    tgtAssert(isOutport(), "called setData on inport!");
    PlotBase* tempVol = portData_;
    if (handle != 0 || portData_ != 0) {
        portData_ = handle;
        invalidate();
    }
    else {
        portData_ = 0;
    }
    if (deletePrevious && tempVol && (tempVol != handle)) {
        delete tempVol;
    }
}

void PlotPort::deletePlotData() {
    tgtAssert(isOutport(), "deletePlotData called on inport!");
    PlotBase* tempVol = portData_;
    if (tempVol) {
        setData(0);
        delete tempVol;
    }
}

} // namespace
