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

#ifndef VRN_NWESTYLE_CLASSIC_PRINT_H
#define VRN_NWESTYLE_CLASSIC_PRINT_H

#include "nwestyle_classic.h"

namespace voreen {

/**
 * Like the classic look of Voreen with white processor items.
 */
class NWEStyle_Classic_Print : public NWEStyle_Classic{
public:
    NWEStyle_Classic_Print();
    ~NWEStyle_Classic_Print();

    /**
     * Help function to use getColorAndDepthSetting without template
     */
    virtual NWEItemSettings getNEWItemSettings(const NWEBaseGraphicsItem* item, const QStyleOptionGraphicsItem* option) {
        return NWEStyle_Base::getItemSettings<NWEStyle_Classic_Print>(item,option);
    }

    /*********************************************************************
     *                       General Color Defines
     ********************************************************************/
    //aggregation
    static const QColor NWEStyle_AggregationColor1;
    //processor
    static const QColor NWEStyle_ProcessorColor1;
    /*********************************************************************
     *                       Core Elements
     ********************************************************************/
    //aggregation
    virtual void AggregationGI_initializePaintSettings(AggregationGraphicsItem* item);
    virtual void AggregationGI_paint(AggregationGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting);

    //processor
    virtual void ProcessorGI_initializePaintSettings(ProcessorGraphicsItem* item);
    virtual void ProcessorGI_paint(ProcessorGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget, NWEItemSettings& setting);
};

} //namespace voreen

#endif // VRN_NWESTYLE_CLASSIC_PRINT_H

