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

#include "voreen/qt/helpbrowser.h"

#include <QAction>
#include <QTextBrowser>
#include <QToolBar>

namespace voreen {

HelpBrowser::HelpBrowser(const QUrl& startPage, const QString& title, QWidget* parent)
    : QMainWindow(parent)
{
    browser_ = new QTextBrowser(this);
    browser_->setSource(startPage);
    browser_->createStandardContextMenu();
    setCentralWidget(browser_);

    setWindowIcon(QIcon(":/icons/bulb.png"));

    setWindowTitle(title.isEmpty() ? tr("Voreen Help Browser") : title);

    createAndConnectToolbar();
}

void HelpBrowser::createAndConnectToolbar() {
    // create toolbar
    QToolBar* toolBar = new QToolBar(tr("Navigation"), this);

    // backward
    QAction* backwardAction = new QAction(QIcon(":/icons/go-previous.png"), tr("&Backward"), this);
    backwardAction->setStatusTip(tr("Show previous page"));
    backwardAction->setToolTip(tr("Show previous page"));
    backwardAction->setShortcut(tr("Ctrl+B"));
    toolBar->addAction(backwardAction);

    // home
    QAction* homeAction = new QAction(QIcon(":/icons/go-home.png"), tr("&Home"), this);
    homeAction->setStatusTip(tr("Show index"));
    homeAction->setToolTip(tr("Show first page"));
    homeAction->setShortcut(tr("Ctrl+H"));
    toolBar->addAction(homeAction);

    // forward
    QAction* forwardAction = new QAction(QIcon(":/icons/go-next.png"), tr("&Forward"), this);
    forwardAction->setStatusTip(tr("Show next page"));
    forwardAction->setToolTip(tr("Show next page"));
    forwardAction->setShortcut(tr("Ctrl+F"));
    toolBar->addAction(forwardAction);

    // separator
    toolBar->addSeparator();

    // quit browser
    QAction* quitBrowserAction = new QAction(QIcon(":/icons/list-remove.png"), tr("&Quit"), this);
    quitBrowserAction->setStatusTip(tr("Exit Help"));
    quitBrowserAction->setToolTip(tr("Exit Help"));
    quitBrowserAction->setShortcut(tr("Ctrl+W"));
    toolBar->addAction(quitBrowserAction);

    //setMenuBar(menuBar_);

    addToolBar(toolBar);

    // CONNECT ACTIONS
    // file menu
    connect(backwardAction, SIGNAL(triggered()), browser_, SLOT(backward()));
    connect(homeAction, SIGNAL(triggered()), browser_, SLOT(home()));
    connect(forwardAction, SIGNAL(triggered()), browser_, SLOT(forward()));
    connect(quitBrowserAction, SIGNAL(triggered()), this, SLOT(close()));
}

} // namespace voreen
