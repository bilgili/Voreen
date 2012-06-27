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

#ifndef VRN_SNAPSHOTPLUGIN_H
#define VRN_SNAPSHOTPLUGIN_H

#include "voreen/qt/widgets/widgetplugin.h"

#include <vector>

class QToolButton;
class QSpinBox;
class QComboBox;

namespace voreen {

class VoreenPainter;

class SnapshotPlugin : public WidgetPlugin {
    Q_OBJECT
public:
    SnapshotPlugin(QWidget* parent, VoreenPainter* painter);
    ~SnapshotPlugin();

    void createWidgets();
    void createConnections();

public slots:
    void makeSnapshot();
    void widthSpinChanged(int value);
    void heightSpinChanged(int value);
    void sizeComboChanged(int index);

private:
    VoreenPainter* painter_;
    QComboBox* sizeCombo_;
    QToolButton* buMakeSnapshot_;
    QSpinBox* spWidth_;
    QSpinBox* spHeight_;
    QString path_;

    std::vector<QString> resolutions_;
};

} // namespace voreen

#endif // VRN_SNAPSHOTPLUGIN_H
