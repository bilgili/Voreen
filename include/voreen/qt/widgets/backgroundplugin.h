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

#ifndef BACKGROUNDPLUGIN_H
#define BACKGROUNDPLUGIN_H

#include "voreen/qt/widgets/widgetplugin.h"

#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

namespace voreen {

class BackgroundPlugin : public WidgetPlugin {
    Q_OBJECT
public:
    BackgroundPlugin(QWidget* parent = 0, MessageReceiver* msgReceiver = 0);
    ~BackgroundPlugin();

    /**
     * This enumeration can be used to enable / disable certain layouts.
     * Note: None of these layouts - except MONOCHROME - is available for Slicerendering.
     * \see Background
     */
    enum BackgroundLayouts {
        MONOCHROME  = 1,
        GRADIENT    = 2,
        RADIAL      = 4,
        CLOUD       = 8,
        TEXTURE     = 16,
        ALL_LAYOUTS = 31
    };

    void setIsSliceRenderer(bool isSlice);
    void enableBackgroundLayouts(int bitString);
    void disableBackgroundLayouts(int bitString);

protected:

    virtual void createConnections();
    virtual void initGL();

    /// Creates the widgets
    virtual void createWidgets();

    /// In this function the initial values for all parameters are set.
    void initParameters();
    /// Enables / disables widgets depending on the current background layout.
    void setWidgetState();

public slots:

    void setBackgroundLayout(int);
    void selectBackgroundColor();
    void selectBackgroundFirstColor();
    void selectBackgroundSecondColor();
    void setBackgroundAngle(int);
    void setBackgroundTile(int);

private:

    QGroupBox* groupBox_;
    QLabel* layoutLabel_;
    QLabel* gradientLayoutLabel_;
    QComboBox* backgroundLayoutCombo_;
    QPushButton* buttonBackgroundColor_;
    QPushButton* buttonBackgroundFirstColor_;
    QPushButton* buttonBackgroundSecondColor_;
    QSpinBox* backgroundAngleSpin_;
    QLabel* colorLabelBackground_;
    QLabel* colorLabelBackgroundFirst_;
    QLabel* colorLabelBackgroundSecond_;
    QLabel* tileLabel_;
    QSpinBox* spinBoxTile_;

    bool isSliceRenderer_;

    // rendering parameters
    BackgroundLayouts backgroundLayout_;
    tgt::Color backgroundColor_;
    tgt::Color backgroundFirstColor_;
    tgt::Color backgroundSecondColor_;
    int backgroundAngle_;
    int backgroundTile_;

    // this is a bitstring determining the layouts to offer the user
    int activeLayouts_;

};

} // namespace voreen

#endif // BACKGROUNDPLUGIN_H
