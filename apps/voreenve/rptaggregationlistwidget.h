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

#ifndef VRN_RPTAGGREGATIONLISTWIDGET_H
#define VRN_RPTAGGREGATIONLISTWIDGET_H

#include <QtGui>


namespace voreen {

class RptAggregationListWidget : public QTreeWidget {
    Q_OBJECT

public:
    RptAggregationListWidget(QWidget* parent=0);
    void buildItems();

private:
    QList<QTreeWidgetItem *> items_;

protected slots:
    void removeActionSlot();

protected:
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

};

//---------------------------------------------------------------------------

class RptAggregationListItem : public QTreeWidgetItem {

public:
    RptAggregationListItem(std::string filename);

    std::string getFileName() {
        return filename_;
    }

private:
    std::string filename_;

};

} //namespace voreen

#endif // VRN_RPTAGGREGATIONLISTWIDGET_H
