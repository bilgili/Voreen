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

#ifndef VRN_VOREENGLOBAL_H
#define VRN_VOREENGLOBAL_H

namespace voreen {

const qreal ZValueArrowGraphicsItem               = 1.f;
const qreal ZValueProcessorGraphicsItemMouseOver  = 24.f;
const qreal ZValuePropertyListGraphicsItem        = 23.f;
const qreal ZValueLinkArrowGraphicsItem           = 22.f;
const qreal ZValueOpenPropertyListButton          = 21.f;
const qreal ZValueProcessorGraphicsItemNormal     = 2.f;

// This enum serves as a central place for registering UserTypes used for qgraphicsitem_cast's
// (see QGraphicsItem in Qt doc)
enum UserTypes {
    UserTypesProcessorGraphicsItem = 1,
    UserTypesPortGraphicsItem = 2,
    UserTypesTextGraphicsItem = 3,
    UserTypesArrowGraphicsItem = 4,
    UserTypesPropertyGraphicsItem = 5,
    UserTypesLinkArrowGraphicsItem = 6
};

} // namespace

#endif // VRN_VOREENGLOBAL_H
