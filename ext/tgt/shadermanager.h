/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_SHADERMANAGER_H
#define TGT_SHADERMANAGER_H

#include <list>
#include <string>

#include "tgt/exception.h"
#include "tgt/manager.h"
#include "tgt/matrix.h"
#include "tgt/tgt_gl.h"
#include "tgt/types.h"
#include "tgt/vector.h"

namespace tgt {

/**
 * Type of a shader object, can be vertex, fragment or geometry shader
 *
 * #include statements are allowed.
 *
 * Geometry shaders can be controled using directives in shader source.
 * Accepted directives:
 * GL_GEOMETRY_INPUT_TYPE_EXT(GL_POINTS | GL_LINES | GL_LINES_ADJACENCY_EXT | GL_TRIANGLES | GL_TRIANGLES_ADJACENCY_EXT)
 * GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_POINTS | GL_LINE_STRIP | GL_TRIANGLE_STRIP)
 * GL_GEOMETRY_VERTICES_OUT_EXT(<int>)
 * No newline or space allowed between each pair of brackets.
 *
 * Example geometry shader header:
 * #version 120
 * #extension GL_EXT_geometry_shader4 : enable
 * //GL_GEOMETRY_INPUT_TYPE_EXT(GL_LINES)
 * //GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_LINE_STRIP)
 * //GL_GEOMETRY_VERTICES_OUT_EXT(42)
 * [...]
 */
class TGT_API ShaderObject {
public:
    friend class Shader;
    friend class ShaderPreprocessor;

    enum ShaderType {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        GEOMETRY_SHADER = GL_GEOMETRY_SHADER_EXT
    };

    // Helper for resolving line number when includes are used in shader files
    struct LineInfo {
        LineInfo(int n, std::string s, int sn)
            : lineNumber_(n), filename_(s), sourceLineNumber_(sn) {}

        int lineNumber_;          //< line number in preprocessed file
        std::string filename_;    //< filename of included file
        int sourceLineNumber_;    //< line number in included file (needed when it itself
                                  //< includes another file)
    };

    /**
     * Creates a shader object of the specified type
     */
    ShaderObject(const std::string& filename, ShaderType type = VERTEX_SHADER);

    /**
     * Deletes the shader and source
     */
    ~ShaderObject();

    /**
     * Loads the shader source from the specified file.
     *
     * @throw Exception if loading failed.
     */
    void loadSourceFromFile(const std::string& filename)
        throw (Exception);

    /**
     * Set directives using glProgramParameteriEXT(...), used for geometry shaders.
     * Call before compiling.
     * @param id Set the directives for this shader
     */
    void setDirectives(GLuint id);

    bool compileShader();

    bool isCompiled() const { return isCompiled_; }

    std::string getCompilerLog() const;

    bool rebuildFromFile();

    /**
     * Use h as header for shadersource (copies h)
     */
    void setHeader(const std::string& h);

    ShaderType getType() { return shaderType_; }

    void setSource(std::string source) {
        source_ = source;
        unparsedSource_ = source;
    }
    const std::string getSource() { return unparsedSource_; }

    /**
     * Set geometry shader input type. For this change to take effect call setDirectives() and
     * re-link already linked shaders. Currently only GL_POINTS, GL_LINES,
     * GL_LINES_ADJACENCY_EXT, GL_TRIANGLES or GL_TRIANGLES_ADJACENCY_EXT can be used.
     */
    void setInputType(GLint inputType) { inputType_ = inputType; }
    GLint getInputType() const { return inputType_; }

    /**
     * Set geometry shader output type. For this change to take effect call setDirectives() and
     * re-link already linked shaders. Currently only GL_POINTS, GL_LINE_STRIP or
     * GL_TRIANGLE_STRIP can be used.
     */
    void setOutputType(GLint outputType) { outputType_ = outputType; }
    GLint getOuputType() const { return outputType_; }

    /**
     * Set maximum number of primitives a geometry shader can create.
     * For this change to take effect call setDirectives()
     * re-link already linked shaders. Limited by GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT.
     */
    void setVerticesOut(GLint verticesOut) { verticesOut_ = verticesOut; }
    GLint getVerticesOut() const { return verticesOut_; }

protected:
    void uploadSource();

    std::string filename_;
    ShaderType shaderType_;

    GLuint id_;
    std::string source_;
    std::string unparsedSource_;
    std::string header_;
    bool isCompiled_;
    GLint inputType_;
    GLint outputType_;
    GLint verticesOut_;

    std::vector<LineInfo> lineTracker_; ///< keeps track of line numbers when includes are used
    static const std::string loggerCat_;
};

//------------------------------------------------------------------------------

/**
 * Represents an OpenGL shader program, consisting of linked ShaderObjects.
 *
 * @note Convenient loading of shaders from file is provided by ShaderManager.
 */
class TGT_API Shader {

    friend class ShaderManager;

public:
    Shader();

    /**
     * Detaches all shader objects, deletes them an disposes all textures
     */
    ~Shader();

    /**
     * Attach shader object to Shader
     */
    void attachObject(ShaderObject* obj);
    void detachObject(ShaderObject* obj);
    void detachObjectsByType(ShaderObject::ShaderType type);

    /**
     * Link all shader objects to one shader.
     * Will re-link already linked shaders.
     * @return true for success
     */
    bool linkProgram();

    bool rebuild();
    bool rebuildFromFile();

    void setHeaders(const std::string& customHeader);

    void bindFragDataLocation(GLuint colorNumber, std::string name);

    GLint getID() {return id_;};
    /**
     * Activates the shader
     */
    void activate();

    static void activate(GLint id);

    static void deactivate();

    static GLint getCurrentProgram();

    /**
     * Returns whether the Shader has at least one attached shader object.
     */
    bool hasObjects() const {
        return !objects_.empty();
    }

    /**
     * Returns whether the Shader is currently activated
     */
    bool isActivated();

    bool isLinked() { return isLinked_; }

    std::string getLinkerLog() const;

    //
    // Uniform stuff
    //

    /**
     * Returns uniform location, or -1 on failure
     */
    GLint getUniformLocation(const std::string& name);

    void setIgnoreUniformLocationError(bool ignoreError);
    bool getIgnoreUniformLocationError();

    // Floats
    bool setUniform(const std::string& name, GLfloat value);
    bool setUniform(const std::string& name, GLfloat v1, GLfloat v2);
    bool setUniform(const std::string& name, GLfloat v1, GLfloat v2, GLfloat v3);
    bool setUniform(const std::string& name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);
    bool setUniform(const std::string& name, GLfloat* v, int count);

    // Integers
    bool setUniform(const std::string& name, GLint value);
    bool setUniform(const std::string& name, GLint v1, GLint v2);
    bool setUniform(const std::string& name, GLint v1, GLint v2, GLint v3);
    bool setUniform(const std::string& name, GLint v1, GLint v2, GLint v3, GLint v4);
    bool setUniform(const std::string& name, GLint* v, int count);

    // Booleans
    bool setUniform(const std::string& name, bool value);
    bool setUniform(const std::string& name, bool v1, bool v2);
    bool setUniform(const std::string& name, bool v1, bool v2, bool v3);
    bool setUniform(const std::string& name, bool v1, bool v2, bool v3, bool v4);
    bool setUniform(const std::string& name, GLboolean* v, int count);

    // Vectors
    bool setUniform(const std::string& name, const Vector2f& value);
    bool setUniform(const std::string& name, Vector2f* vectors, GLsizei count = 1);
    bool setUniform(const std::string& name, const Vector3f& value);
    bool setUniform(const std::string& name, Vector3f* vectors, GLsizei count = 1);
    bool setUniform(const std::string& name, const Vector4f& value);
    bool setUniform(const std::string& name, Vector4f* vectors, GLsizei count = 1);
    bool setUniform(const std::string& name, const ivec2& value);
    bool setUniform(const std::string& name, ivec2* vectors, GLsizei count = 1);
    bool setUniform(const std::string& name, const ivec3& value);
    bool setUniform(const std::string& name, ivec3* vectors, GLsizei count = 1);
    bool setUniform(const std::string& name, const ivec4& value);
    bool setUniform(const std::string& name, ivec4* vectors, GLsizei count = 1);

    // Note: Matrix is transposed by OpenGL
    bool setUniform(const std::string& name, const Matrix2f& value, bool transpose = false);
    bool setUniform(const std::string& name, const Matrix3f& value, bool transpose = false);
    bool setUniform(const std::string& name, const Matrix4f& value, bool transpose = false);

    // No location lookup
    //
    // Floats
    static void setUniform(GLint l, GLfloat value);
    static void setUniform(GLint l, GLfloat v1, GLfloat v2);
    static void setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3);
    static void setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);

    // Integers
    static void setUniform(GLint l, GLint value);
    static void setUniform(GLint l, GLint v1, GLint v2);
    static void setUniform(GLint l, GLint v1, GLint v2, GLint v3);
    static void setUniform(GLint l, GLint v1, GLint v2, GLint v3, GLint v4);

    // Vectors
    static void setUniform(GLint l, const Vector2f& value);
    static void setUniform(GLint l, const Vector3f& value);
    static void setUniform(GLint l, const Vector4f& value);
    static void setUniform(GLint l, const ivec2& value);
    static void setUniform(GLint l, const ivec3& value);
    static void setUniform(GLint l, const ivec4& value);
    static void setUniform(GLint l, const Matrix2f& value, bool transpose = false);
    static void setUniform(GLint l, const Matrix3f& value, bool transpose = false);
    static void setUniform(GLint l, const Matrix4f& value, bool transpose = false);

    // Attributes
    //
    // 1 component
    static void setAttribute(GLint index, GLfloat v1);
    static void setAttribute(GLint index, GLshort v1);
    static void setAttribute(GLint index, GLdouble v1);

    // 2 components
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2);
    static void setAttribute(GLint index, GLshort v1, GLshort v2);
    static void setAttribute(GLint index, GLdouble v1, GLdouble v2);

    // 3 components
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3);
    static void setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3);
    static void setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3);

    // 4 components
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);
    static void setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3, GLshort v4);
    static void setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4);

    // For vectors
    static void setAttribute(GLint index, const Vector2f& v);
    static void setAttribute(GLint index, const Vector3f& v);
    static void setAttribute(GLint index, const Vector4f& v);

    static void setAttribute(GLint index, const Vector2d& v);
    static void setAttribute(GLint index, const Vector3d& v);
    static void setAttribute(GLint index, const Vector4d& v);

    static void setAttribute(GLint index, const Vector2<GLshort>& v);
    static void setAttribute(GLint index, const Vector3<GLshort>& v);
    static void setAttribute(GLint index, const Vector4<GLshort>& v);

    static void setAttribute(GLint index, const Vector4<GLint>& v);
    static void setAttribute(GLint index, const Vector4<GLbyte>& v);
    static void setAttribute(GLint index, const Vector4<GLubyte>& v);
    static void setAttribute(GLint index, const Vector4<GLushort>& v);
    static void setAttribute(GLint index, const Vector4<GLuint>& v);

    // Attribute locations
    void setAttributeLocation(GLuint index, const std::string& name);
    GLint getAttributeLocation(const std::string& name);

    // Normalized attributes
    static void setNormalizedAttribute(GLint index, GLubyte v1, GLubyte v2, GLubyte v3, GLubyte v4);

    static void setNormalizedAttribute(GLint index, const Vector4<GLbyte>& v);
    static void setNormalizedAttribute(GLint index, const Vector4<GLshort>& v);
    static void setNormalizedAttribute(GLint index, const Vector4<GLint>& v);

    // Unsigned version
    static void setNormalizedAttribute(GLint index, const Vector4<GLubyte>& v);
    static void setNormalizedAttribute(GLint index, const Vector4<GLushort>& v);
    static void setNormalizedAttribute(GLint index, const Vector4<GLuint>& v);

protected:
    /**
     * Load filename.vert and filename.frag (vertex and fragment shader) and link shader.
     *
     * @param customHeader Header to be put in front of the shader source.
     *
     * @throw Exception if loading failed
     */
    void load(const std::string& filename, const std::string& customHeader = "")
        throw (Exception);

    /**
     * Load vertex shader \p vertFilename, geometry shader \p geomFilename,
     * fragment shader \p fragFilename.
     *
     * @param customHeader header to be put in front of the shader source
     *
     * @throw Exception if loading failed
     */
    void loadSeparate(const std::string& vertFilename, const std::string& geomFilename,
        const std::string& fragFilename, const std::string& customHeader = "")
        throw (Exception);

    typedef std::list<ShaderObject*> ShaderObjects;
    ShaderObjects objects_;

    GLuint id_;
    bool isLinked_;
    bool ignoreError_;

    static const std::string loggerCat_;
};

//------------------------------------------------------------------------------

class ShaderManager;
#ifdef DLL_TEMPLATE_INST
template class TGT_API Singleton<ShaderManager>;
#endif
#ifdef DLL_TEMPLATE_INST
template class TGT_API ResourceManager<Shader>;
#endif

/**
 * Loads shaders from the file system, managing a shader search path.
 *
 * @see ResourceManager
 */
class TGT_API ShaderManager : public ResourceManager<Shader>, public Singleton<ShaderManager> {
public:

    ShaderManager();

    /**
     * Load filename.vert and filename.frag (vertex and fragment shader), link shader and
     * activate it by default.
     *
     * @param customHeader Header to be put in front of the shader source
     * @param activate activate the shader after loading
     *
     * @return The loaded shader
     *
     * @throw Exception if loading failed
     */
    Shader* load(const std::string& filename, const std::string& customHeader = "",
                 bool activate = true)
                 throw (Exception);

    /**
     * Load vertex shader \p vertFilename and fragment shader \p fragFilename,
     * link shader and activate it by default.
     *
     * You have to pass the complete filenames, inclusive file extensions (".vert", ".frag").
     *
     * @param customHeader header to be put in front of the shader source
     * @param activate activate the shader after loading
     *
     * @return The loaded shader
     *
     * @throw Exception if loading failed
     */
    Shader* loadSeparate(const std::string& vertFilename, const std::string& fragFilename,
                         const std::string& customHeader = "", bool activate = true)
                         throw (Exception);

    /**
     * Load vertex shader \p vertFilename, geometry shader \p geomFilename,
     * fragment shader \p fragFilename, link shader and activate it by default.
     *
     * You have to pass the complete filenames, inclusive file extensions (".vert", ".geom", frag").
     *
     * @param customHeader header to be put in front of the shader source
     * @param activate activate the shader after loading
     *
     * @return The loaded shader
     *
     * @throw Exception if loading failed
     */
    Shader* loadSeparate(const std::string& vertFilename, const std::string& geomFilename,
                         const std::string& fragFilename,
                         const std::string& customHeader, bool activate = true)
                         throw(Exception);

    bool rebuildAllShadersFromFile();

protected:
    static const std::string loggerCat_;
};

/**
 * Parses #include statements and geometry shader settings
 */
class ShaderPreprocessor {
public:
    enum Mode {
        MODE_NONE      = 0,
        MODE_INCLUDE   = 1,
        MODE_GEOMETRY  = 2
    };

    ShaderPreprocessor(ShaderObject* obj, Mode mode = Mode(MODE_INCLUDE | MODE_GEOMETRY));

    // Returns the parsed result
    std::string getResult() const;

    GLint getGeomShaderInputType() const;
    GLint getGeomShaderOutputType() const;
    GLint getGeomShaderVerticesOut() const;

protected:
    void parse();
    void parsePart(const std::string& input, const std::string& name = "");

    void outputComment(const std::string& comment, const std::string& type = "INFO");

    /**
     *    Scan for geometry shader directives in shader source.
     *
     *    Accepted directives:
     *    GL_GEOMETRY_INPUT_TYPE_EXT(GL_POINTS | GL_LINES | GL_LINES_ADJACENCY_EXT | GL_TRIANGLES | GL_TRIANGLES_ADJACENCY_EXT)
     *    GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_POINTS | GL_LINE_STRIP | GL_TRIANGLE_STRIP)
     *    GL_GEOMETRY_VERTICES_OUT_EXT(<int>)
     *    No newline or space allowed between each pair of brackets.
     *
     *    Example geometry shader header:
     *    #version 120
     *    #extension GL_EXT_geometry_shader4 : enable
     *
     *    //GL_GEOMETRY_INPUT_TYPE_EXT(GL_LINES)
     *    //GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_LINE_STRIP)
     *    //GL_GEOMETRY_VERTICES_OUT_EXT(42)
     *    [...]
     */
    bool scanGeomShaderDirectives();

    std::string getGeomShaderDirective(const std::string& d);


    ShaderObject* shd_;
    std::vector<ShaderObject::LineInfo>& lineTracker_; ///< keeps track of line numbers when includes are used
    int activeLine_;
    std::ostringstream result_;

    GLint inputType_;
    GLint outputType_;
    GLint verticesOut_;

    static const std::string loggerCat_;
};

} // namespace tgt

#define ShdrMgr tgt::Singleton<tgt::ShaderManager>::getRef()

#endif //TGT_SHADERMANAGER_H
