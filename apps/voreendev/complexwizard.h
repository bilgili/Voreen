/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef COMPLEXWIZARD_H
#define COMPLEXWIZARD_H

#include <QDialog>
#include <QList>

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;
class WizardPage;

class ComplexWizard : public QDialog
{
    Q_OBJECT

public:
    ComplexWizard(QWidget *parent = 0);

    QList<WizardPage *> historyPages() const { return history; }

protected:
    void setFirstPage(WizardPage *page);

private slots:
    void backButtonClicked();
    void nextButtonClicked();
    void completeStateChanged();
    void finished();

protected:
    void switchPage(WizardPage *oldPage);

    QList<WizardPage *> history;
    QPushButton *cancelButton;
    QPushButton *backButton;
    QPushButton *nextButton;
    QPushButton *finishButton;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

class WizardPage : public QWidget
{
    Q_OBJECT

public:
    WizardPage(QWidget *parent = 0);

    virtual void resetPage();
    virtual WizardPage *nextPage();
    virtual bool isLastPage();
    virtual bool isComplete();

signals:
    void completeStateChanged();
};

#endif 
