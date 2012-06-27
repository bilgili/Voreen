//=============================================================================
/** @file        highlighter.cpp
 *
 * Implements a GLSL syntax highlighter.
 *
    @internal
    created:    2007-12-06
    last mod:    2008-01-03

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer

=============================================================================*/

#include <QtGui/QTextCharFormat>
#include <QtCore/QVector>
#include <QtCore/QRegExp>
#include <QSyntaxHighlighter>

//=============================================================================
//    GLSLHighlighter definition
//=============================================================================

/** A syntax highlighter for GLSL.
 * This is mainly the modified syntax highlighter example from the Qt toolkit.
 * Because GLSL has really many special worlds (keyworlds, built-in uniforms,
 * attributes, ... ) this syntax highlighter operates quite slow. A possible
 * improvement would be to reduce the number of supported keywords, but that
 * would confuse the user.
 */
class GLSLHighlighter : public QSyntaxHighlighter
{
public:
    /** Constructs a GLSL syntax highlighter object.
     * @param doc Document to highlight.
     */
    GLSLHighlighter( QTextDocument* doc );

protected:
    /** Does actual highlighting.
     * See Qt documentation for details.
     */
    void highlightBlock( const QString & text );

private:

    // construction hlpers
    void setupKeywords( void );
    void setupReservedKeywords( void );
    void setupComments( void );
    void setupNumberRules( void );
    void setupRegisters( void );
    void setupBuiltInFunctions( void );
    void setupPreprocessor( void );

    // rules
    typedef struct highlightRule_s {
        QRegExp pattern;
        QTextCharFormat format;
    } highlightRule_t;
    QVector< highlightRule_t > m_rules;

    // comment expressions
    QRegExp m_commentStartExpression;
    QRegExp m_commentEndExpression;

    // format infos
    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_reservedKeywordFormat;
    QTextCharFormat m_integerFormat;
    QTextCharFormat m_floatFormat;
    QTextCharFormat m_registerFormat; // IO registers and GL uniform state
    QTextCharFormat m_builtInFunctionFormat;
    QTextCharFormat m_preprocessorFormat;
    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
};
