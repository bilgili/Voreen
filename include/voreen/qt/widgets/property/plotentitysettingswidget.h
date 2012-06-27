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

#ifndef VRN_PLOTENTITYSETTINGSWIDGET_H
#define VRN_PLOTENTITYSETTINGSWIDGET_H

#include "voreen/qt/widgets/property/colorpropertywidget.h"
#include "voreen/core/properties/plotentitiesproperty.h"
#include "voreen/qt/widgets/property/plotentitiespropertywidget.h"

class QCheckBox;
class QComboBox;
class QTabWidget;
class ClickableColorLabel;
class QDoubleSpinBox;
class QPushButton;

namespace voreen {

class PlotEntitySettingsWidget : public QWidget {
    Q_OBJECT
public:

    PlotEntitySettingsWidget(PlotEntitiesProperty* prop);
    ~PlotEntitySettingsWidget();

    void updateWidgets();

    void setEntity(PlotEntitySettings entitySettings, int index);

    tgt::Color toTgtColor(QColor color);
    QColor toQColor(tgt::Color color);

public slots:
    virtual void createWidgets();

private slots:
    void firstColorDialog();
    void secondColorDialog();
    void updateProperty();
    void cbMainColumnChanged(int index);
    void cbCandleTopColumnChanged(int index);
    void cbCandleBottomColumnChanged(int index);
    void cbStickTopColumnChanged(int index);
    void cbStickBottomColumnChanged(int index);
    void cbOptionalColumnChanged(int index);
    void cbSecondOptionalColumnChanged(int index);
    void cbStyleChanged(int index);
    void chbSplineChanged();
    void chbErrorbarChanged();
    void chbWireOnlyChanged();
    void chbHeightMapChanged();
    void cbColorMapChanged(int index);
    void tabChanged(int index);
    void sbMinGlyphSizeChanged(double value);
    void sbMaxGlyphSizeChanged(double value);
    void btOpenFileDialogClicked();

private:

    /* fills the ComboBox cb with number columns of the associated plotdata
    if empty entry is true, an empty entry will be inserted first (with value -1) */
    void fillComboBox(QComboBox* cb, bool emptyEntry = false);

    /// Recreates the widgets
    void reCreateWidgets();

    /// sets the text of btOpenFileDialog
    void updateBtOpenFileDialogText(const std::string& text);

    //entity settings widgets
    QComboBox* cbMainColumn_;
    QComboBox* cbCandleTopColumn_;
    QComboBox* cbCandleBottomColumn_;
    QComboBox* cbStickTopColumn_;
    QComboBox* cbStickBottomColumn_;
    //this box has the possibility to select no column
    QComboBox* cbOptionalColumn_;
    QComboBox* cbSecondOptionalColumn_;

    QComboBox* cbStyle_;
    QCheckBox* chbSpline_;
    QCheckBox* chbErrorbar_;

    QCheckBox* chbWireOnly_;
    QCheckBox* chbHeightMap_;
    QComboBox* cbColorMap_;

    QTabWidget* tabWidget_;

    ClickableColorLabel* cclrFirst_;
    QColor firstColor_;
    ClickableColorLabel* cclrSecond_;
    QColor secondColor_;
    ClickableColorLabel* cclrThird_;
    ClickableColorLabel* cclrFourth_;

    QDoubleSpinBox* sbMinGlyphSize_;
    QDoubleSpinBox* sbMaxGlyphSize_;

    QPushButton* btOpenFileDialog_;

    PlotEntitiesProperty* property_;

    //the entity the widget edits
    PlotEntitySettings entitySettings_;

    //the corresponding index of the vector<PlotEntitySettings> in property_
    int index_;

};


} // namespace

#endif // VRN_PLOTENTITYSETTINGSWIDGET_H
