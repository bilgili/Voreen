/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_SCREENSHOTPLUGIN_H
#define VRN_SCREENSHOTPLUGIN_H

#include <QWidget>

#include "voreen/qt/voreenqtapi.h"
#include <vector>

class QToolButton;
class QSpinBox;
class QComboBox;

namespace voreen {

class CanvasRenderer;

class VRN_QT_API ScreenshotPlugin : public QWidget {
Q_OBJECT
public:
    ScreenshotPlugin(QWidget* parent, CanvasRenderer* canvasRenderer);
    ~ScreenshotPlugin();

public slots:
    void takeScreenshot();
    void widthSpinChanged(int value);
    void heightSpinChanged(int value);
    virtual void sizeComboChanged(int index);
    void updateFromProcessor();

protected:
    virtual void saveScreenshot(const QString& filename, int width, int height);

    CanvasRenderer* canvasRenderer_;
    QComboBox* sizeCombo_;
    QToolButton* buMakeScreenshot_;
    QSpinBox* spWidth_;
    QSpinBox* spHeight_;
    QString path_;

    std::vector<QString> resolutions_;
};

} // namespace voreen

#endif // VRN_SCREENSHOTPLUGIN_H
