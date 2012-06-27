/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_TRANSFUNCGRADIENT_H
#define VRN_TRANSFUNCGRADIENT_H

#include <QWidget>

#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/qt/widgets/transfunc/transfuncintensitypainter.h"
#include "tgt/qt/qtcanvas.h"

namespace voreen {

class TransFuncGradient : public QWidget {
    Q_OBJECT
public:
    TransFuncGradient(QWidget* parent = 0, bool clipThresholds = false);
    ~TransFuncGradient();
    void setTransFunc(TransFuncIntensity* tf);
    void setThresholds(float lowerThreshold, float upperThreshold);
	void setTextureCoords(float start, float end);
    void paintEvent(QPaintEvent *);
	tgt::GLCanvas* getCanvas() { return transCanvas_; }

public slots:
    void toggleClipThresholds(bool enabled);
    
protected:
//     void paintEvent(QPaintEvent* event);
    tgt::QtCanvas* transCanvas_;
    TransFuncIntensityPainter* painter_;

    TransFuncIntensity* tf_;
    float lowerThreshold_, upperThreshold_;
    bool clipThresholds_;
};

} // namespace voreen

#endif //VRN_TRANSFUNCGRADIENT_H
