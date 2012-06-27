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

#ifndef VRN_PLOTPREDICATEDIALOG_H
#define VRN_PLOTPREDICATEDIALOG_H

#include "voreen/core/plotting/plotbase.h"
#include "voreen/core/plotting/plotcell.h"

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QObject>
#include <QLabel>


namespace voreen {

class PlotPredicateDialog : public QDialog {
    Q_OBJECT

public:
    PlotPredicateDialog(const std::vector<std::pair<std::string,int> >& comboBoxText,
                 const PlotBase::ColumnType valueType, const int selected,
                 const std::vector<PlotCellValue>& values, const std::vector<int>& comboboxValues,
                 QWidget* parent=0);
    ~PlotPredicateDialog();

    /// Gives back the Stringvalues in the edit Field in the Windows
    std::vector<std::string> getStringValues() const;
    /// Gives back the selected Number in the QComboBox
    int getSelected() const;
    /// Gives back the Value of the selected Line in the QComboBox
    int getSelectedValue() const;

private:

    void initialize(const std::vector<std::pair<std::string,int> >& comboBoxText,
                 const PlotBase::ColumnType valueType, const int selected,
                 const std::vector<PlotCellValue>& values, const std::vector<int>& comboboxValues);

    QComboBox* choice_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QGridLayout* mainLayout;

    std::vector<std::pair<std::string,int> > comboBoxText_;
    PlotBase::ColumnType valueType_;

    std::vector<QLineEdit*> editVector_;
    std::vector<PlotCellValue> values_;
    std::vector<QLabel*> labelVector_;


private slots:
    void clickedOk();
    void clickedCancel();
    void choiceChange(int index);

};

}
#endif // VRN_PLOTPREDICATEDIALOG_H
