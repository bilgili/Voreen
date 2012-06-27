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

#ifndef DYNAMICSPLUGIN_H
#define DYNAMICSPLUGIN_H

#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QThread>
#include <QLabel>

#include "widgetplugin.h"
#include "voreen/qt/widgets/sliderspinboxwidget.h"

namespace voreen {

class DynamicsPlugin : public WidgetPlugin {
    Q_OBJECT
public:
    DynamicsPlugin(QWidget* parent = 0, MessageReceiver* msgReceiver = 0);
    ~DynamicsPlugin();

    void setNumDatasets(unsigned int num);
protected:
    virtual void createWidgets();
    virtual void createConnections();

public:
    bool isLooping() { return loopCheckBox_->isChecked(); }

public slots:
    void setDataset(int dataset);
    void play();
    void stop();
    void rewind();
    void fastForward();

protected slots:
    void run();

protected:
    SliderSpinBoxWidget* dataset_;
    unsigned int numDatasets_;


private:
    QPushButton *playButton_, *stopButton_, *rewindButton_, *ffButton_;
    QCheckBox *loopCheckBox_;
    QTimer* timer_;

};

} // namespace voreen

#endif // DYNAMICSPLUGIN_H
