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

#ifndef VRN_NETWORKEDITOR_COMMON_H
#define VRN_NETWORKEDITOR_COMMON_H

#include <vector>

#include <QtGlobal>
#include <QList>

namespace voreen {

const qreal ZValueArrowGraphicsItem               = 1.f;
const qreal ZValueRootGraphicsItemMouseOver       = 24.f;
const qreal ZValuePropertyListGraphicsItem        = 23.f;
const qreal ZValueLinkArrowGraphicsItem           = 22.f;
const qreal ZValueOpenPropertyListButton          = 21.f;
const qreal ZValueRootGraphicsItemNormal          = 2.f;
const qreal ZValueLinkArrowGraphicsItemStub       = 30.f;

// This enum serves as a central place for registering UserTypes used for qgraphicsitem_cast's
// (see QGraphicsItem in Qt doc)
enum UserTypes {
    UserTypesProcessorGraphicsItem = 1,
    UserTypesPortGraphicsItem = 2,
    UserTypesTextGraphicsItem = 3,
    UserTypesArrowGraphicsItem = 4,
    UserTypesPropertyGraphicsItem = 5,
    UserTypesLinkArrowGraphicsItem = 6,
    UserTypesLinkArrowGraphicsItemStub = 7,
    UserTypesLinkDialogPropertyGraphicsItem = 8,
    UserTypesLinkDialogArrowGraphicsItem = 9,
    UserTypesLinkDialogProcessorGraphicsItem = 10,
    UserTypesPortArrowGraphicsItem = 11,
    UserTypesAggregationGraphicsItem = 12,
    UserTypesProgressBarGraphicsItem = 13,
    UserTypesWidgetIndicatorButton = 14,
    UserTypesArrowHeadSelectionGraphicsItem = 15,
    UserTypesOpenPropertyListButton = 16
};

template<class T>
QList<T> stdVectorToQList(const std::vector<T>& vec) {
    QList<T> result;

    foreach (T i, vec)
        result.append(i);

    return result;
}

template<class T>
std::vector<T> qListToStdVector(const QList<T>& vec) {
    std::vector<T> result;

    foreach (T i, vec)
        result.push_back(i);

    return result;
}

template<class T, class U>
QList<U> convertQList(const QList<T>& list, bool ignoreFailedConversion = true) {
    QList<U> result;

    foreach (T item, list) {
        U castItem = dynamic_cast<U>(item);

        if (!ignoreFailedConversion) {
            tgtAssert(castItem, "dynamic_cast failed");
        }

        if (castItem)
            result.append(castItem);
    }

    return result;
}

} // namespace

#endif // VRN_NETWORKEDITOR_COMMON_H
