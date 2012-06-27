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

#include "voreen/qt/widgets/codeedit.h"

#include <QPainter>
#include <QTextBlock>

CodeEdit::CodeEdit(QWidget* parent) : QPlainTextEdit(parent) {
    statusArea = new StatusArea(this);
    QFont font;
    font.setFamily("Courier New");
    font.setStyleHint(QFont::TypeWriter);
    font.insertSubstitution("Courier New", "monospace");
    font.setFixedPitch(true);
    font.setPointSize(9);
    setFont(font);
    QFontMetrics metrics(font);
    setTabStopWidth(metrics.width(" ")*4);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateStatusAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateStatusArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateStatusAreaWidth(0);
    highlightCurrentLine();
}

int CodeEdit::statusAreaWidth() {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void CodeEdit::updateStatusAreaWidth(int /* newBlockCount */) {
    setViewportMargins(statusAreaWidth(), 0, 0, 0);
}

void CodeEdit::updateStatusArea(const QRect& rect, int dy) {
    if (dy)
        statusArea->scroll(0, dy);
    else
        statusArea->update(0, rect.y(), statusArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateStatusAreaWidth(0);
}

void CodeEdit::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    statusArea->setGeometry(QRect(cr.left(), cr.top(), statusAreaWidth(), cr.height()));
}

void CodeEdit::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (isEnabled() && !isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(170);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEdit::statusAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(statusArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, statusArea->width(), fontMetrics().height(),
                    Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEdit::moveCursorToPosition(int line, int col) {
    QTextCursor cursor = textCursor();
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
    if (col > 0)
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);
    setTextCursor(cursor);
}

void CodeEdit::updateHighlight() {
    highlightCurrentLine();
}
