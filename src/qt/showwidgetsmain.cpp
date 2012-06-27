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

#include <qapplication.h>
#include <qlabel.h>

#include "voreen/qt/sliderspinboxwidget.h"
#include "voreen/qt/thresholdwidget.h"
#include "voreen/qt/transferfuncwidget.h"
// #include "voreen/qt/qtglcanvas.h"

#include "ui_transferfuncwidgetexample.h"

namespace voreen {

int main( int argc, char ** argv )
{
    // create console window
#ifdef WIN32
#ifndef _DEBUG
    // Ausgaben auf cout und cerr in Logdateien umleiten
    freopen("voreenapp-log.txt", "w", stdout);
    freopen("voreenapp-errors.txt", "w", stderr);
#else
    //Hack für Anzeige der Konsole unter Windows und die Umleitung von
    //cout und cerr dorthin
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif // _DEBUG
#endif // WIN32

    QApplication a( argc, argv );


    ThresholdWidget threshold;
    threshold.show();

    /*
    SliderSpinBoxWidget sliderSpinBox;
    sliderSpinBox.setEnabled(false);
    sliderSpinBox.show();
    */

    /*
    QtCanvas canvas(0);
    canvas.show();
    */

    /*
    TransferFunc1D func;
    Ui::TransferFuncExample ui;
    QWidget transferFuncWidget;
    ui.setupUi(&transferFuncWidget);
    func.loadBMP("../../transferFunc/transferfunc0.bmp");
    ui.transferFunc->addTransferFunc(&func);
    func.loadBMP("../../transferFunc/transferfunc1.bmp");
    ui.transferFunc->addTransferFunc(&func);
    func.loadBMP("../../transferFunc/transferfunc2.bmp");
    ui.transferFunc->addTransferFunc(&func);
    func.loadBMP("../../transferFunc/transferfunc3.bmp");
    ui.transferFunc->addTransferFunc(&func);
    func.loadBMP("../../transferFunc/transferfunc4.bmp");
    ui.transferFunc->addTransferFunc(&func);
    ui.transferFunc->setCurrentIndex(0);
    transferFuncWidget.show();
    */

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
} // namespace voreen

