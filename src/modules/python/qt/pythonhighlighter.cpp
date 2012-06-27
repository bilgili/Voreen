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

#include "voreen/modules/python/qt/pythonhighlighter.h"

namespace voreen {

PythonHighlighter::PythonHighlighter(QTextDocument* doc)
    : SyntaxHighlighter(doc)
{
    setupKeywords();
    setupNumberRules();
    setupComments();
}

void PythonHighlighter::setupComments() {
    highlightRule_t rule;

    m_singleLineCommentFormat.setForeground(Qt::darkGreen);
    m_singleLineCommentFormat.setFontItalic(true);

    rule.pattern = QRegExp("#[^\n]*");
    rule.format = m_singleLineCommentFormat;
    m_rules.append(rule);
}

void PythonHighlighter::setupNumberRules() {
    highlightRule_t rule;

    m_integerFormat.setForeground(Qt::magenta);
    m_floatFormat.setForeground(Qt::darkBlue);

    rule.pattern = QRegExp("\\b[0-9]+\\b");
    rule.format = m_integerFormat;
    m_rules.append(rule);

    // must be appended after integer rule to "overwrite" it.
    rule.pattern = QRegExp("\\b[0-9]\\.[0-9]+\\b");
    rule.format = m_floatFormat;
    m_rules.append(rule);
}
void PythonHighlighter::setupKeywords() {
    highlightRule_t rule;
    QStringList keywordPatterns;

    m_keywordFormat.setForeground(Qt::blue);

    keywordPatterns << "\\band\\b"
                    << "\\bas\\b"
                    << "\\bassert\\b"
                    << "\\bbreak\\b"
                    << "\\bclass\\b"
                    << "\\bcontinue\\b"
                    << "\\bdef\\b"
                    << "\\bdel\\b"
                    << "\\belif\\b"
                    << "\\belse\\b"
                    << "\\bexcept\\b"
                    << "\\bexec\\b"
                    << "\\bfinally\\b"
                    << "\\bfor\\b"
                    << "\\bfrom\\b"
                    << "\\bglobal\\b"
                    << "\\bif\\b"
                    << "\\bimport\\b"
                    << "\\bin\\b"
                    << "\\bis\\b"
                    << "\\blambda\\b"
                    << "\\bNone\\b"
                    << "\\bnot\\b"
                    << "\\bor\\b"
                    << "\\bpass\\b"
                    << "\\bprint\\b"
                    << "\\braise\\b"
                    << "\\breturn\\b"
                    << "\\btriple\\b"
                    << "\\btry\\b"
                    << "\\bwhile\\b"
                    << "\\bwith\\b"
                    << "\\byield\\b"
                    ;

    foreach(QString pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = m_keywordFormat;
        m_rules.append(rule);
    }
}

} // namespace
