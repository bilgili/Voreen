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

#ifndef VRN_THRESHOLDWIDGET_H
#define VRN_THRESHOLDWIDGET_H

#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "voreen/qt/widgets/sliderspinboxwidget.h"

namespace voreen {

    class ThresholdWidget : public QWidget {
        Q_OBJECT
            Q_PROPERTY( int lowerValue READ getLowerValue WRITE setLowerValue )
            Q_PROPERTY( int upperValue READ getUpperValue WRITE setUpperValue )
            Q_PROPERTY( int minValue READ getMinValue WRITE setMinValue )
            Q_PROPERTY( int maxValue READ getMaxValue WRITE setMaxValue )
            public:
        ThresholdWidget(QWidget* parent = 0);
        virtual int getLowerValue()const;
        virtual int getUpperValue()const;
        virtual int getMinValue()const;
        virtual int getMaxValue()const;
    signals:
        void upperValueChanged(int);
        void lowerValueChanged(int);
        void valuesChanged(int,int);
        void upperStateToggled(bool);
        void lowerStateToggled(bool);
        void sliderPressedChanged(bool);
        
    public slots:
        virtual void setValues(int lowerValue, int upperValue);
        virtual void setLowerValue( int value );
        virtual void setUpperValue( int value );
        virtual void setMaxValue( int value );
        virtual void setMinValue( int value );
        virtual void setUpperStateChecked(bool state);
        virtual void setLowerStateChecked(bool state);
        virtual void resetThresholds();
        void slidersPressedChange(bool pressed);
        void setHounsfieldRange();
        protected slots:
    protected:

    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    QLabel *upperValueLBL;
    QHBoxLayout *hboxLayout;
    SliderSpinBoxWidget* upperValueSLB;
    QVBoxLayout *vboxLayout2;
    QLabel *lowerValueLBL;
    QHBoxLayout *hboxLayout1;
    SliderSpinBoxWidget* lowerValueSLB;
    QLabel *hounsfieldLBL;
    QComboBox *hounsfield;

        int lowerValue_;
        int upperValue_;
        int minValue_;
        int maxValue_;
        bool lowerState_;
        bool upperState_;
    };

} // namespace voreen;

#endif //VRN_THRESHOLDWIDGET_H
