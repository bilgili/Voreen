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

#ifndef VRN_QPROCESSORWIDGET_H
#define VRN_QPROCESSORWIDGET_H

#include "tgt/vector.h"
#include "voreen/core/vis/processors/processorwidget.h"
#include <QDialog>
#include <QWidget>
#include <QPoint>

namespace voreen {

class QProcessorWidget : public QWidget, public ProcessorWidget {
    Q_OBJECT
public:

    QProcessorWidget(Processor* processor, QWidget* parent = 0);

    virtual void setVisible(bool);
    virtual bool isVisible() const;

    virtual void setSize(int,int);
    virtual tgt::ivec2 getSize() const;

    virtual void setPosition(int,int);
    virtual tgt::ivec2 getPosition() const;

    virtual void processorNameChanged();

protected:
    /// Passes the show event to the super classes.
    void showEvent(QShowEvent*);
    /// Passes the hide event to the super classes.
    void hideEvent(QHideEvent*);
    /// Passes the move event to the super classes.
    void moveEvent(QMoveEvent*);
    /// Passes the resize event to the super classes.
    void resizeEvent(QResizeEvent*);

};


} //namespace voreen

#endif
