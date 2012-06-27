/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
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
#include <iostream>
#include <fstream>

#include "tgt/config.h"
#include "tgt/manager.h"
#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/tgt_gl.h"
#include "tgt/logmanager.h"

namespace tgt {

/**
 * ShaderObject, can be vertex or fragment shader
 */
enum ShaderType
{
    VERTEX_SHADER = GL_VERTEX_SHADER,
    FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    GEOMETRY_SHADER = GL_GEOMETRY_SHADER_EXT
                      
};

//------------------------------------------------------------------------------

class ShaderObject
{
    friend class Shader;

    std::string filename_;
    ShaderType shaderType_;
    static const std::string loggerCat_;

    GLuint id_;
    GLchar* source_;
    GLchar* header_;
    bool isCompiled_;
	GLint inputType_;
	GLint outputType_;
	GLint verticesOut_;

public:
    /**
     * Creates a shader object of the specified type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
     */
    ShaderObject(const std::string& filename, ShaderType type = VERTEX_SHADER);

    /**
     * Deletes the shader an source
     */
    ~ShaderObject();

    bool loadSourceFromFile(const std::string &filename);

    void uploadSource();

	
	/**
	 * Searches for #include preprocessor directives and replaces them by
	 * the content of the referenced file.
	 * returns the number of the replaced directives
	 */
    int replaceIncludes();

	/**
	*	Scan for geometry shader directives in shader source.
	*	
	*	Accepted directives:
	*	GL_GEOMETRY_INPUT_TYPE_EXT(GL_POINTS | GL_LINES | GL_LINES_ADJACENCY_EXT | GL_TRIANGLES | GL_TRIANGLES_ADJACENCY_EXT)
	*	GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_POINTS | GL_LINE_STRIP | GL_TRIANGLE_STRIP)
	*	GL_GEOMETRY_VERTICES_OUT_EXT(<int>)
	*	No newline or space allowed between each pair of brackets.
	*
	*	Example geometry shader header:
	*	#version 120 
	*	#extension GL_EXT_geometry_shader4 : enable
	*
	*	//GL_GEOMETRY_INPUT_TYPE_EXT(GL_LINES)
	*	//GL_GEOMETRY_OUTPUT_TYPE_EXT(GL_LINE_STRIP)
	*	//GL_GEOMETRY_VERTICES_OUT_EXT(42)
	*	[...]
	*/
	bool scanDirectives();

	///Set directives using glProgramParameteriEXT(...)
	///Call before linking.
	///@param id Set the directives for this shader
	void setDirectives(GLuint id);

    bool compileShader();

    bool isCompiled() const { return isCompiled_; }

    std::string getCompilerLog();

    bool rebuildFromFile();

    /**
     * Use h as header for shadersource (copies h)
     */
    void setHeader(const std::string& h);

    /**
     * Generates (and sets) header from defines, seperated by blanks.
     * Example: "a b" =>
     * #define a
     * #define b
     */
    void generateHeader(const std::string& defines);

    ShaderType getType() { return shaderType_; }

	///Set geometry shader input type.
	///For this change to take effect call setDirectives()
	///re-link already linked shaders.
	///Now (30.4.2008) only  GL_POINTS, GL_LINES, GL_LINES_ADJACENCY_EXT, GL_TRIANGLES or GL_TRIANGLES_ADJACENCY_EXT can be used.
	void setInputType(GLint inputType) { inputType_ = inputType; }
	GLint getInputType() { return inputType_; }

	///Set geometry shader output type.
	///For this change to take effect call setDirectives()
	///re-link already linked shaders.
	///Now (30.4.2008) only GL_POINTS, GL_LINE_STRIP or GL_TRIANGLE_STRIP can be used.
	void setOutputType(GLint outputType) { outputType_ = outputType; }
	GLint getOuputType() { return outputType_; }

	///Set max number a geometry shader can create.
	///For this change to take effect call setDirectives()
	///re-link already linked shaders.
	///Limited by GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT
	void setVerticesOut(GLint verticesOut) { verticesOut_ = verticesOut; }
	GLint getVerticesOut() { return verticesOut_; }

protected:
	std::string getDirective(std::string d);
};

//------------------------------------------------------------------------------

/**
 * Shader, consists of compiled ShaderObjects, linked together
 */
class Shader
{
    typedef std::list<ShaderObject*> ShaderObjects;

    static const std::string loggerCat_;

    GLuint id_;
    bool isLinked_;

    ShaderObjects objects_;

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
    void detachObjectsByType(ShaderType type);

    /**
     * Link all shader objects to one shader.
	 * Will re-link already linked shaders.
     *   @return true for success
     */
    bool linkProgram();

    bool rebuild();
    bool rebuildFromFile();

    void setHeaders(const std::string& customHeader, bool processHeader = true);

    /**
     * Activates the shader with all textures
     */
    void activate();
    
    static void deactivate()
        { glUseProgram(0); }

    static GLint getCurrentProgram();

    static void activate(GLint id)
        { glUseProgram(id); }

    /// Returns whether the Shader is currently active
    bool isActivated();

    bool isLinked() { return isLinked_; }

    std::string getLinkerLog();
    std::string getSource(int i);

    //
    // Uniform stuff
    //

    /**
     * Returns uniform location, or -1 on failure
     */
    GLint getUniformLocation(const std::string &name, bool ignoreError = false);
    
// Floats
    bool setUniform(const std::string &name, GLfloat value);
    bool setUniform(const std::string &name, GLfloat v1, GLfloat v2);
    bool setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3);
    bool setUniform(const std::string &name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4);
    bool setUniform(const std::string &name, int count, GLfloat* v);

// Integers
    bool setUniform(const std::string &name, GLint value);
    bool setUniform(const std::string &name, GLint v1, GLint v2);
    bool setUniform(const std::string &name, GLint v1, GLint v2, GLint v3);
    bool setUniform(const std::string &name, GLint v1, GLint v2, GLint v3, GLint v4);
    bool setUniform(const std::string &name, int count, GLint* v);

#ifdef __APPLE__
	// Glew (1.4.0) defines 'GLint' as 'long' on Apple, so these wrappers are necessary.
	// On all other platforms 'GLint' is defined as 'int' instead.
	bool setUniform(const std::string &name, int value);
    bool setUniform(const std::string &name, int v1, int v2);
    bool setUniform(const std::string &name, int v1, int v2, int v3);
    bool setUniform(const std::string &name, int v1, int v2, int v3, int v4);
    bool setUniform(const std::string &name, int count, int* v);
#endif

// Booleans
    bool setUniform(const std::string &name, bool value);
    bool setUniform(const std::string &name, bool v1, bool v2);
    bool setUniform(const std::string &name, bool v1, bool v2, bool v3);
    bool setUniform(const std::string &name, bool v1, bool v2, bool v3, bool v4);
    bool setUniform(const std::string &name, int count, GLboolean* v);

// Vectors
    bool setUniform(const std::string &name, Vector2f value, GLsizei count = 1);
    bool setUniform(const std::string &name, Vector2f* vectors, GLsizei count = 1);
    bool setUniform(const std::string &name, Vector3f value, GLsizei count = 1);
    bool setUniform(const std::string &name, Vector3f* vectors, GLsizei count = 1);
    bool setUniform(const std::string &name, Vector4f value, GLsizei count = 1);
    bool setUniform(const std::string &name, Vector4f* vectors, GLsizei count = 1);
    bool setUniform(const std::string &name, int count, Vector4f* vectors);
    bool setUniform(const std::string &name, ivec2 value, GLsizei count = 1);
    bool setUniform(const std::string &name, ivec2* vectors, GLsizei count = 1);
    bool setUniform(const std::string &name, ivec3 value, GLsizei count = 1);
    bool setUniform(const std::string &name, ivec3* vectors, GLsizei count = 1);
    bool setUniform(const std::string &name, ivec4 value, GLsizei count = 1);
    bool setUniform(const std::string &name, ivec4* vectors, GLsizei count = 1);

    /**
     * Matrix is transposed by opengl!
     */
    bool setUniform(const std::string &name, const Matrix2f &value, bool transpose = false,
                    GLsizei count = 1);

    bool setUniform(const std::string &name, const Matrix3f &value, bool transpose = false,
                    GLsizei count = 1);

    bool setUniform(const std::string &name, const Matrix4f &value, bool transpose = false,
                    GLsizei count = 1);

    // no location lookup:
    static void setUniform(GLint l, GLfloat value)
        { glUniform1f(l, value); }
    
    static void setUniform(GLint l, GLfloat v1, GLfloat v2)
        { glUniform2f(l, v1, v2); }

    static void setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3)
        { glUniform3f(l, v1, v2, v3); }

    static void setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
        { glUniform4f(l, v1, v2, v3, v4); }

    // integers:
    static void setUniform(GLint l, GLint value)
        { glUniform1i(l, value); }

    static void setUniform(GLint l, GLint v1, GLint v2)
        { glUniform2i(l, v1, v2); }

    static void setUniform(GLint l, GLint v1, GLint v2, GLint v3)
        { glUniform3i(l, v1, v2, v3); }

    static void setUniform(GLint l, GLint v1, GLint v2, GLint v3, GLint v4)
        { glUniform4i(l, v1, v2, v3, v4); }

    // vectors...
    static void setUniform(GLint l, Vector2f value, GLsizei count = 1)
        { glUniform2fv(l, count, value.elem); }

    static void setUniform(GLint l, Vector3f value, GLsizei count = 1)
        { glUniform3fv(l, count, value.elem); }

    static void setUniform(GLint l, Vector4f value, GLsizei count = 1)
        { glUniform4fv(l, count, value.elem); }

    static void setUniform(GLint l, ivec2 value, GLsizei count = 1)
        { glUniform2iv(l, count,  (GLint *) value.elem); }

    static void setUniform(GLint l, ivec3 value, GLsizei count = 1)
        { glUniform3iv(l, count,  (GLint *) value.elem); }

    static void setUniform(GLint l, ivec4 value, GLsizei count = 1)
        { glUniform4iv(l, count,  (GLint *) value.elem); }

    static void setUniform(GLint l, Matrix2f value, bool transpose = false, GLsizei count = 1)
        { glUniformMatrix2fv(l, count, !transpose, value.elem); }

    static void setUniform(GLint l, Matrix3f value, bool transpose = false, GLsizei count = 1)
        { glUniformMatrix3fv(l, count, !transpose, value.elem); }

    static void setUniform(GLint l, Matrix4f value, bool transpose = false, GLsizei count = 1)
        { glUniformMatrix4fv(l, count, !transpose, value.elem); }
    
    // Attributes
    GLint getAttributeLocation(const std::string &name);

    // 1
    static void setAttribute(GLint index, GLfloat v1)
        { glVertexAttrib1f(index, v1); }

    static void setAttribute(GLint index, GLshort v1)
        { glVertexAttrib1s(index, v1); }

    static void setAttribute(GLint index, GLdouble v1)
        { glVertexAttrib1d(index, v1); }

    // 2
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2)
        { glVertexAttrib2f(index, v1, v2); }

    static void setAttribute(GLint index, GLshort v1, GLshort v2)
        { glVertexAttrib2s(index, v1, v2); }

    static void setAttribute(GLint index, GLdouble v1, GLdouble v2)
        { glVertexAttrib2d(index, v1, v2); }

    // 3
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3)
        { glVertexAttrib3f(index, v1, v2, v3); }

    static void setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3)
        { glVertexAttrib3s(index, v1, v2, v3); }

    static void setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3)
        { glVertexAttrib3d(index, v1, v2, v3); }

    // 4
    static void setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
        { glVertexAttrib4f(index, v1, v2, v3, v4); }

    static void setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3, GLshort v4)
        { glVertexAttrib4s(index, v1, v2, v3, v4); }

    static void setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4)
        { glVertexAttrib4d(index, v1, v2, v3, v4); }

    // for vectors:
    static void setAttribute(GLint index, const Vector2f &v)
        { glVertexAttrib2fv(index, v.elem); }
    static void setAttribute(GLint index, const Vector3f &v)
        { glVertexAttrib3fv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4f &v)
        { glVertexAttrib4fv(index, v.elem); }

    static void setAttribute(GLint index, const Vector2d &v)
        { glVertexAttrib2dv(index, v.elem); }
    static void setAttribute(GLint index, const Vector3d &v)
        { glVertexAttrib3dv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4d &v)
        { glVertexAttrib4dv(index, v.elem); }

    static void setAttribute(GLint index, const Vector2<GLshort> &v)
        { glVertexAttrib2sv(index, v.elem); }
    static void setAttribute(GLint index, const Vector3<GLshort> &v)
        { glVertexAttrib3sv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4<GLshort> &v)
        { glVertexAttrib4sv(index, v.elem); }

    static void setAttribute(GLint index, const Vector4<GLint> &v)
        { glVertexAttrib4iv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4<GLbyte> &v)
        { glVertexAttrib4bv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4<GLubyte> &v)
        { glVertexAttrib4ubv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4<GLushort> &v)
        { glVertexAttrib4usv(index, v.elem); }
    static void setAttribute(GLint index, const Vector4<GLuint> &v)
        { glVertexAttrib4uiv(index, v.elem); }

    // Attribute locations

    void setAttributeLocation(GLuint index, const std::string name)
        { glBindAttribLocation(id_, index, name.c_str()); }

    // normalized attributes
    static void setNormalizedAttribute(GLint index, GLubyte v1, GLubyte v2, GLubyte v3, GLubyte v4)
        { glVertexAttrib4Nub(index, v1, v2, v3, v4); }

    static void setNormalizedAttribute(GLint index, const Vector4<GLbyte> &v)
        { glVertexAttrib4Nbv(index, v.elem); }
    static void setNormalizedAttribute(GLint index, const Vector4<GLshort> &v)
        { glVertexAttrib4Nsv(index, v.elem); }
    static void setNormalizedAttribute(GLint index, const Vector4<GLint> &v)
        { glVertexAttrib4Niv(index, v.elem); }

    // unsigned version
    static void setNormalizedAttribute(GLint index, const Vector4<GLubyte> &v)
        { glVertexAttrib4Nubv(index, v.elem); }
    static void setNormalizedAttribute(GLint index, const Vector4<GLushort> &v)
        { glVertexAttrib4Nusv(index, v.elem); }
    static void setNormalizedAttribute(GLint index, const Vector4<GLuint> &v)
        { glVertexAttrib4Nuiv(index, v.elem); }

    /**
     * Load filename.vert and filename.frag (vertex and fragment shader) and link shader
     * Optional header is added @see ShaderObject::generateHeader
     * @param processHeader if true header is not generated but set
     */
    bool load(const std::string& filename, const std::string& customHeader = "",
              bool processHeader = true);
    bool loadSeparate(const std::string& vert_filename, const std::string& frag_filename,
		const std::string& customHeader = "", bool processHeader = true, const std::string& geom_filename = "");
};

//------------------------------------------------------------------------------

class ShaderManager : public ResourceManager<Shader>
{
    static const std::string loggerCat_;

public:
    ShaderManager( bool cache = true );

    /**
     *   \brief Load filename.vert and filename.frag (vertex and fragment shader),
     *   link shader and activate it by default
     *
     *   Optional header is added @see ShaderObject::generateHeader
     *   @param processHeader if true header is not generated but set
     *	 @param activate if true shader is activated after loading
     *   @return 0 on failure
     */
    virtual Shader* load(const std::string& filename, const std::string& customHeader = "",
                         bool processHeader = true, bool activate = true);

    /**
     *  \brief Load vert_filename and frag_filename (vertex and fragment shader), link shader
     *  and activate it by default
     *
     *  You have to pass the complete filenames, inclusive file extensions (".vert", ".frag")
     *  Optional header is added @see ShaderObject::generateHeader
     *  @param processHeader if true header is not generated but set
     *  @param activate if true shader is activated after loading
     *  @return 0 on failure
     */
    virtual Shader* loadSeparate(const std::string& vert_filename, const std::string& frag_filename,
                                 const std::string& customHeader = "", bool processHeader = true,
                                 bool activate = true, const std::string& geom_filename = "");

    bool rebuildAllShadersFromFile();
};

} // namespace tgt

#define ShdrMgr tgt::Singleton<tgt::ShaderManager>::getRef()

#endif //TGT_SHADERMANAGER_H
