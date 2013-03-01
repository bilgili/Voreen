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

#include "tgt/shadermanager.h"

#include "tgt/gpucapabilities.h"
#include "tgt/texturemanager.h"

#include <iostream>
#include <fstream>

using std::string;

namespace tgt {

//------------------------------------------------------------------------------

namespace {

/**
 * Resolve the line number to take into account the include directives.
 * Returns a string containing file name and line number in that file.
 */
std::string resolveLineNumber(int number, const std::vector<ShaderObject::LineInfo>& lineTracker) {
    int found = static_cast<int>(lineTracker.size()) - 1;
    for (int i = static_cast<int>(lineTracker.size()) - 1; i >= 0 ; i--) {
        if (lineTracker[i].lineNumber_ <= number) {
            found = i;
            break;
        }
    }

    std::ostringstream result;
    //if (found >= 0) { //found is unsigned...
        result << FileSystem::fileName(lineTracker[found].filename_) << ":"
               << (number - lineTracker[found].lineNumber_ + lineTracker[found].sourceLineNumber_);
    //}
    return result.str();
}

} // namespace

const std::string ShaderPreprocessor::loggerCat_("tgt.Shader.ShaderPreprocessor");

ShaderPreprocessor::ShaderPreprocessor(ShaderObject* obj, Mode /*mode*/)
    : shd_(obj), lineTracker_(obj->lineTracker_), inputType_(0), outputType_(0), verticesOut_(0)
{
    parse();
}

void ShaderPreprocessor::parsePart(const std::string& input, const std::string& name) {
    std::istringstream source(input);
    int locallinenumber = 0;

    lineTracker_.push_back(ShaderObject::LineInfo(activeLine_, name, 1));

    string line;
    while (std::getline(source, line)) {
        locallinenumber++;
        string::size_type pos = line.find("#include");

        // At least partly support for comments.
        // "/*"-style comments starting on previous lines are not detected.
        string::size_type comment1 = line.find("//");
        string::size_type comment2 = line.find("/*");
        bool inComment = (comment1 != string::npos && comment1 < pos) || (comment2 != string::npos && comment2 < pos);

        if (pos != string::npos && !inComment) {
            pos = line.find("\"", pos + 1);
            string::size_type end = line.find("\"", pos + 1);
            string filename(line, pos + 1, end - pos - 1);
            filename = ShdrMgr.completePath(filename);

            File* file = FileSys.open(filename);
            string content;
            if ((!file) || (!file->isOpen())) {
                LERROR("Cannot open shader include '" << filename << "' in " << resolveLineNumber(activeLine_, lineTracker_));
            }
            else {
                size_t len = file->size();
                // check if file is empty
                if (len == 0)
                    content = "";
                else
                    content = file->getAsString();
                file->close();

                outputComment("BEGIN INCLUDE " + filename, "BEGIN");

                if (!content.empty() && content[content.size() - 1] != '\n')
                    content += "\n";

                parsePart(content, filename);

                outputComment("END INCLUDE " + filename, "END");

                lineTracker_.push_back(ShaderObject::LineInfo(activeLine_, name, locallinenumber + 1));
            }
            delete file;
        } else {
            result_ << line << "\n";
            activeLine_++;
        }
    }
}

void ShaderPreprocessor::parse() {
    activeLine_ = 1;
    lineTracker_.clear();
    result_.clear();

    if (!shd_->header_.empty()) {
        outputComment("BEGIN HEADER");
        parsePart(shd_->header_, "HEADER");
        outputComment("END HEADER");
    }

    parsePart(shd_->unparsedSource_, shd_->filename_);
}

void ShaderPreprocessor::outputComment(const std::string& comment, const std::string& type) {
    result_ << "// " << comment << "\n";
    lineTracker_.push_back(ShaderObject::LineInfo(activeLine_, type, 0));
    activeLine_++;
}

bool ShaderPreprocessor::scanGeomShaderDirectives() {
    LDEBUG("Scanning for geometry shader compile directives...");
    std::string input = getGeomShaderDirective("GL_GEOMETRY_INPUT_TYPE_EXT");

    if (input == "GL_POINTS")
        inputType_ = GL_POINTS;
    else if (input == "GL_LINES")
        inputType_ = GL_LINES;
    else if (input == "GL_LINES_ADJACENCY_EXT")
        inputType_ = GL_LINES_ADJACENCY_EXT;
    else if (input == "GL_TRIANGLES")
        inputType_ = GL_TRIANGLES;
    else if (input == "GL_TRIANGLES_ADJACENCY_EXT")
        inputType_ = GL_TRIANGLES_ADJACENCY_EXT;
    else {
        LERROR("Unknown input type: " << input);
        return false;
    };

    std::string output = getGeomShaderDirective("GL_GEOMETRY_OUTPUT_TYPE_EXT");
    if (output == "GL_POINTS")
        outputType_ = GL_POINTS;
    else if (output == "GL_LINE_STRIP")
        outputType_ = GL_LINE_STRIP;
    else if (output == "GL_TRIANGLE_STRIP")
        outputType_ = GL_TRIANGLE_STRIP;
    else {
        LERROR("Unknown output type: " << output);
        return false;
    };

    std::string verticesOut = getGeomShaderDirective("GL_GEOMETRY_VERTICES_OUT_EXT");

    std::istringstream myStream(verticesOut);
    if (myStream >> verticesOut_) {
        LDEBUG("VERTICES_OUT: " << verticesOut_);
    }
    else {
        LERROR("Failed to parse argument(" << verticesOut << ") as integer for directive GL_GEOMETRY_VERTICES_OUT_EXT.");
        return false;
    }

    return true;
}

std::string ShaderPreprocessor::getGeomShaderDirective(const std::string& d) {
    string sourceStr(shd_->source_);
    string::size_type curPos = sourceStr.find(d + "(", 0);
    string::size_type length = d.length() + 1;
    if (curPos != string::npos) {
        string::size_type endPos = sourceStr.find(")", curPos);

        if (endPos != string::npos) {
            std::string ret = sourceStr.substr(curPos + length, endPos - curPos - length);
            // test for space, newline:
            if ((ret.find(" ", 0) == string::npos) && (ret.find("\n", 0) == string::npos) ) {
                LINFO("Directive " << d << ": " << ret);
                return ret;
            }
            else {
                LERROR("No spaces/newlines allowed inbetween directive brackets! Directive: " << d);
                return "";
            }
        }
        LERROR("Missing ending bracket for directive " << d);
        return "";
    }
    else {
        LWARNING("Could not locate directive " << d << "!");
        return "";
    }
}

std::string ShaderPreprocessor::getResult() const {
    return result_.str();
}

GLint ShaderPreprocessor::getGeomShaderOutputType() const {
    return outputType_;
}

GLint ShaderPreprocessor::getGeomShaderInputType() const {
    return inputType_;
}

GLint ShaderPreprocessor::getGeomShaderVerticesOut() const {
    return verticesOut_;
}

//------------------------------------------------------------------------------

const string ShaderObject::loggerCat_("tgt.Shader.ShaderObject");

ShaderObject::ShaderObject(const string& filename, ShaderType type)
    : filename_(filename)
    , shaderType_(type)
    , isCompiled_(false)
    , inputType_(GL_TRIANGLES)
    , outputType_(GL_TRIANGLE_STRIP)
    , verticesOut_(16)
{
    id_ = glCreateShader(shaderType_);
    if (id_ == 0)
        LERROR("ShaderObject(" + filename + ")::glCreateShader() returned 0");
}

ShaderObject::~ShaderObject() {
    glDeleteShader(id_);
}

void ShaderObject::loadSourceFromFile(const string& filename) throw (Exception) {
    LDEBUG("Loading " << filename);
    File* file = FileSys.open(filename);

    // check if file is open
    if (!file || !file->isOpen()) {
        LDEBUG("File not found: " << filename);
        delete file;
        throw FileNotFoundException("", filename);
    }

    filename_ = filename;
    unparsedSource_ = file->getAsString();
    source_ = unparsedSource_;

    file->close();
    delete file;
}

void ShaderObject::uploadSource() {
    const GLchar* s = source_.c_str();
    glShaderSource(id_, 1,  &s, 0);
}

void ShaderObject::setDirectives(GLuint id) {
    glProgramParameteriEXT(id, GL_GEOMETRY_INPUT_TYPE_EXT, inputType_);
    glProgramParameteriEXT(id, GL_GEOMETRY_OUTPUT_TYPE_EXT, outputType_);
    glProgramParameteriEXT(id, GL_GEOMETRY_VERTICES_OUT_EXT, verticesOut_);
}

bool ShaderObject::compileShader() {
    isCompiled_ = false;

    ShaderPreprocessor p(this);
    source_ = p.getResult();

    if (shaderType_ == GEOMETRY_SHADER) {
        if (p.getGeomShaderInputType())
            inputType_ = p.getGeomShaderInputType();
        if (p.getGeomShaderOutputType())
            outputType_ = p.getGeomShaderOutputType();
        if (p.getGeomShaderVerticesOut())
            verticesOut_ = p.getGeomShaderVerticesOut();
    }

    uploadSource();

    glCompileShader(id_);
    GLint check = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &check);
    isCompiled_ = (check == GL_TRUE);
    return isCompiled_;
}

namespace {

int parseLogLineNumberNVIDIA(const std::string& message) {
    // Errors look like this:
    // 0(1397) : error C0000: syntax error, unexpected '=' at token "="
    std::istringstream ls(message);
    int id; // first number (probably used when multiple sources are bound), ignored
    if (ls >> id) {
        char c = 0; // should be an opening parenthesis
        if (ls >> c &&c == '(') {
            int num; // line number
            if (ls >> num)
                return num;
        }
    }
    return 0;
}

int parseLogLineNumberATI(const std::string& message) {
    // Errors look like this:
    // ERROR: 0:2785: 'frontPos' : undeclared identifier
    std::istringstream ls(message);
    std::string s;
    if (ls >> s && s == "ERROR:") {
        int id; // first number (probably used when multiple sources are bound), ignored
        if (ls >> id) {
            char c = 0; // should be a colon
            if (ls >> c && c == ':') {
                int num; // line number
                if (ls >> num)
                    return num;
            }
        }
    }

    return 0;
}

/**
 * Tries to extract the line number for a given compile messages.
 * Returns 0 if no line number was found.
 */
int parseLogLineNumber(const std::string& message) {
    int n = 0;

    n = parseLogLineNumberNVIDIA(message);
    if (n > 0)
        return n;

    n = parseLogLineNumberATI(message);
    if (n > 0)
        return n;

    return 0;
}

} // namespace

string ShaderObject::getCompilerLog() const {
    GLint len;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH , &len);

    if (len > 1) {
        GLchar* log = new GLchar[len];
        if (log == 0)
            return "Memory allocation for log failed!";
        GLsizei l;  // length returned
        glGetShaderInfoLog(id_, len, &l, log);
        std::istringstream str(log);
        delete[] log;

        std::ostringstream result;

        string line;
        while (getline(str, line)) {
            result << line;

            int num = parseLogLineNumber(line);
            if (num > 0)
                result << " [" << resolveLineNumber(num, lineTracker_) << "]";

            result << '\n';
        }

        return result.str();
    } else {
        return "";
    }
}

void ShaderObject::setHeader(const string& h) {
    header_ = h;
}

bool ShaderObject::rebuildFromFile() {
    try {
        loadSourceFromFile(filename_);
    }
    catch (const Exception& e) {
        LWARNING("Failed to load shader " << filename_ << ": " << e.what());
        return false;
    }

    uploadSource();

    if (!compileShader()) {
        LERROR("Failed to compile shader object " << filename_);
        LERROR("Compiler Log: \n" << getCompilerLog());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

const string Shader::loggerCat_("tgt.Shader.Shader");

Shader::Shader()
    : isLinked_(false)
    , ignoreError_(false)
{
    id_ = glCreateProgram();
    if (id_ == 0)
        LERROR("Shader(): glCreateProgram() returned 0");
}

Shader::~Shader() {
    for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
        glDetachShader(id_, (*iter)->id_);
        delete (*iter);
    }
    glDeleteProgram(id_);
}

void Shader::attachObject(ShaderObject* obj) {
    glAttachShader(id_, obj->id_);
    objects_.push_back(obj);
}

void Shader::detachObject(ShaderObject* obj) {
    glDetachShader(id_, obj->id_);
    objects_.remove(obj);
    isLinked_ = false;
}

void Shader::activate() {
    if (isLinked_)
        glUseProgram(id_);
}

void Shader::activate(GLint id) {
    glUseProgram(id);
}

void Shader::deactivate() {
    glUseProgram(0);
}

GLint Shader::getCurrentProgram() {
    GLint id;
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);
    return id;
}

bool Shader::isActivated() {
    GLint shader_nr;
    glGetIntegerv(GL_CURRENT_PROGRAM, &shader_nr);
    return (id_ == static_cast<GLuint>(shader_nr));
}

void Shader::detachObjectsByType(ShaderObject::ShaderType type) {
    for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
        if ((*iter)->getType() == type)
            detachObject(*iter);
        delete (*iter);
    }
    isLinked_ = false;
}

bool Shader::linkProgram() {
    if (isLinked_) {
        // program is already linked: detach and re-attach everything
        for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
            glDetachShader(id_, (*iter)->id_);
            glAttachShader(id_, (*iter)->id_);
            if ((*iter)->getType() == ShaderObject::GEOMETRY_SHADER)
                (*iter)->setDirectives(id_);
        }
    } else {
        for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
            if ((*iter)->getType() == ShaderObject::GEOMETRY_SHADER)
                (*iter)->setDirectives(id_);
        }
    }

    isLinked_ = false;
    glLinkProgram(id_);
    GLint check = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &check);
    if (check)
        isLinked_ = true;

    return isLinked_;
}

string Shader::getLinkerLog() const {
    GLint len;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH , &len);

    if (len > 1) {
        GLchar* log = new GLchar[len];
        if (log == 0)
            return "Memory allocation for log failed!";
        GLsizei l;  // length returned
        glGetProgramInfoLog(id_, len, &l, log);
        string retStr(log);
        delete[] log;
        return retStr;
    }

    return "";
}

bool Shader::rebuild() {
    if (isLinked_) {
        // program is already linked: detach and re-attach everything
        for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
            glDetachShader(id_, (*iter)->id_);
            (*iter)->uploadSource();
            if (!(*iter)->compileShader()) {
                LERROR("Failed to compile shader object.");
                LERROR("Compiler Log: \n" << (*iter)->getCompilerLog());
                return false;
            }

            glAttachShader(id_, (*iter)->id_);
        }
    }
    isLinked_ = false;
    glLinkProgram(id_);
    GLint check = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &check);

    if (check) {
        isLinked_ = true;
        return true;
    } else {
        LERROR("Shader::rebuild(): Failed to link shader." );
        LERROR("Linker Log: \n" << getLinkerLog());
        return false;
    }
}

bool Shader::rebuildFromFile() {
    bool result = true;

    for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter)
        result &= (*iter)->rebuildFromFile();

    result &= rebuild();

    return result;
}

void Shader::setHeaders(const string& customHeader) {
    for (ShaderObjects::iterator iter = objects_.begin(); iter != objects_.end(); ++iter) {
        (*iter)->setHeader(customHeader);
    }
}

void Shader::bindFragDataLocation(GLuint colorNumber, std::string name) {
    if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        glBindFragDataLocation(id_, colorNumber, name.c_str()); /* was ...EXT */
    }
}

void Shader::load(const string& filename, const string& customHeader) throw (Exception) {
    return loadSeparate(filename + ".vert", "", filename + ".frag", customHeader);
}

void Shader::loadSeparate(const string& vert_filename, const string& geom_filename,
                          const string& frag_filename, const string& customHeader)
                          throw (Exception)
{
    ShaderObject* frag = 0;
    ShaderObject* vert = 0;
    ShaderObject* geom = 0;

    if (!vert_filename.empty()) {
        vert = new ShaderObject(vert_filename, ShaderObject::VERTEX_SHADER);

        if (!customHeader.empty()) {
            vert->setHeader(customHeader);
        }

        try {
            vert->loadSourceFromFile(vert_filename);
        }
        catch (const Exception& e) {
            LDEBUG("Failed to load vertex shader " << vert_filename << ": " << e.what());
            delete vert;
            throw Exception("Failed to load vertex shader " + vert_filename + ": " + e.what());
        }

        vert->uploadSource();

        if (!vert->compileShader()) {
            LERROR("Failed to compile vertex shader " << vert_filename);
            LERROR("Compiler Log: \n" << vert->getCompilerLog());
            delete vert;
            throw Exception("Failed to compile vertex shader: " + vert_filename);
        }
    }

    if (!geom_filename.empty()) {
        geom = new ShaderObject(geom_filename, ShaderObject::GEOMETRY_SHADER);

        if (!customHeader.empty()) {
            geom->setHeader(customHeader);
        }

        try {
            geom->loadSourceFromFile(geom_filename);
        }
        catch (const Exception& e) {
            LDEBUG("Failed to load geometry shader " << geom_filename << ": " << e.what());
            delete vert;
            delete geom;
            throw Exception("Failed to load geometry shader " + geom_filename + ": " + e.what());
        }

        geom->uploadSource();
        if (!geom->compileShader()) {
            LERROR("Failed to compile geometry shader " << geom_filename);
            LERROR("Compiler Log: \n" << geom->getCompilerLog());
            delete vert;
            delete geom;
            throw Exception("Failed to compile geometry shader: " + geom_filename);
        }
    }

    if (!frag_filename.empty()) {
        frag = new ShaderObject(frag_filename, ShaderObject::FRAGMENT_SHADER);

        if (!customHeader.empty()) {
            frag->setHeader(customHeader);
        }

        try {
            frag->loadSourceFromFile(frag_filename);
        }
        catch (const Exception& e) {
            LDEBUG("Failed to load fragment shader " << frag_filename);
            delete frag;
            delete geom;
            delete vert;
            throw Exception("Failed to load fragment shader " + frag_filename + ": " + e.what());
        }

        if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130)
            bindFragDataLocation(0, "FragData0");

        frag->uploadSource();

        if (!frag->compileShader()) {
            LERROR("Failed to compile fragment shader " << frag_filename);
            LERROR("Compiler Log: \n" << frag->getCompilerLog());
            delete vert;
            delete geom;
            delete frag;
            throw Exception("Failed to compile fragment shader: " + frag_filename);
        }
    }

    // Attach ShaderObjects, dtor will take care of freeing them
    if (frag)
        attachObject(frag);
    if (vert)
        attachObject(vert);
    if (geom)
        attachObject(geom);

    if (!linkProgram()) {
        LERROR("Failed to link shader (" << vert_filename << ","  << frag_filename << "," << geom_filename << ")");
        if (vert) {
            LERROR(vert->filename_ << " Vertex shader compiler log: \n" << vert->getCompilerLog());
            detachObject(vert);
            delete vert;
        }
        if (geom) {
            LERROR(geom->filename_ << " Geometry shader compiler log: \n" << geom->getCompilerLog());
            detachObject(geom);
            delete geom;
        }
        if (frag) {
            LERROR(frag->filename_ << " Fragment shader compiler log: \n" << frag->getCompilerLog());
            detachObject(frag);
            delete frag;
        }

        LERROR("Linker Log: \n" << getLinkerLog());
        throw Exception("Failed to link shader (" + vert_filename + "," + frag_filename + "," + geom_filename + ")");
    }


    if (vert && vert->getCompilerLog().size() > 1) {
        LDEBUG("Vertex shader compiler log for file '" << vert_filename
               << "': \n" << vert->getCompilerLog());
    }
    if (geom && geom->getCompilerLog().size() > 1) {
        LDEBUG("Geometry shader compiler log for file '" << geom_filename
               << "': \n" << geom->getCompilerLog());
    }
    if (frag && frag->getCompilerLog().size() > 1) {
        LDEBUG("Fragment shader compiler log for file '" << frag_filename
               << "': \n" << frag->getCompilerLog());
    }

    if (getLinkerLog().size() > 1) {
        LDEBUG("Linker log for '" << vert_filename << "' and '"
               << frag_filename << "' and '"
               << geom_filename << "': \n" << getLinkerLog());
    }
}

GLint Shader::getUniformLocation(const string& name) {
    GLint l;
    l = glGetUniformLocation(id_, name.c_str());
    if (l == -1 && !ignoreError_)
        LWARNING("Failed to locate uniform Location: " << name);
    return l;
}

void Shader::setIgnoreUniformLocationError(bool ignoreError) {
    ignoreError_ = ignoreError;
}

bool Shader::getIgnoreUniformLocationError() {
    return ignoreError_;
}

// Floats
bool Shader::setUniform(const string& name, GLfloat value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform1f(l, value);
    return true;
}

bool Shader::setUniform(const string& name, GLfloat v1, GLfloat v2) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform2f(l, v1, v2);
    return true;
}

bool Shader::setUniform(const string& name, GLfloat v1, GLfloat v2, GLfloat v3) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform3f(l, v1, v2, v3);
    return true;
}

bool Shader::setUniform(const string& name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform4f(l, v1, v2, v3, v4);
    return true;
}

bool Shader::setUniform(const string& name, GLfloat* v, int count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform1fv(l, count, v);
    return true;
}

// Integers
bool Shader::setUniform(const string& name, GLint value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform1i(l, value);
    return true;
}

bool Shader::setUniform(const string& name, GLint v1, GLint v2) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform2i(l, v1, v2);
    return true;
}

bool Shader::setUniform(const string& name, GLint v1, GLint v2, GLint v3) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform3i(l, v1, v2, v3);
    return true;
}

bool Shader::setUniform(const string& name, GLint v1, GLint v2, GLint v3, GLint v4) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform4i(l, v1, v2, v3, v4);
    return true;
}

bool Shader::setUniform(const string& name, GLint* v, int count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    glUniform1iv(l, count, v);
    return true;
}


bool Shader::setUniform(const string& name, bool value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, static_cast<GLint>(value));
    return true;
}

bool Shader::setUniform(const string& name, bool v1, bool v2) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, static_cast<GLint>(v1), static_cast<GLint>(v2));
    return true;
}

bool Shader::setUniform(const string& name, bool v1, bool v2, bool v3) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3));
    return true;
}

bool Shader::setUniform(const string& name, bool v1, bool v2, bool v3, bool v4) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3), static_cast<GLint>(v4));
    return true;
}

bool Shader::setUniform(const string& name, GLboolean* v, int count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLint* vector = new GLint[count];
    for (int i=0; i < count; i++)
        vector[i] = static_cast<GLint>( v[i] );
    glUniform1iv(l, count, vector);
    delete[] vector;
    return true;
}

// Vectors
bool Shader::setUniform(const string& name, const Vector2f& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, Vector2f* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    //TODO: use the adress directly, without copying, same below. joerg
    GLfloat* values = new GLfloat[2*count];
    for (int i=0; i < count; i++){
        values[2*i] = vectors[i].x;
        values[2*i+1] = vectors[i].y;
    }
    glUniform2fv(l, count, values);
    delete[] values;
    return true;
}

bool Shader::setUniform(const string& name, const Vector3f& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, Vector3f* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLfloat* values = new GLfloat[3*count];
    for (int i=0; i < count; i++) {
        values[3*i] = vectors[i].x;
        values[3*i+1] = vectors[i].y;
        values[3*i+2] = vectors[i].z;
    }
    glUniform3fv(l, count, values);
    delete[] values;
    return true;
}

bool Shader::setUniform(const string& name, const Vector4f& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, Vector4f* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLfloat* values = new GLfloat[4*count];
    for (int i=0; i < count; i++) {
        values[4*i] = vectors[i].x;
        values[4*i+1] = vectors[i].y;
        values[4*i+2] = vectors[i].z;
        values[4*i+3] = vectors[i].a;
    }
    glUniform4fv(l, count, values);
    delete[] values;
    return true;
}

bool Shader::setUniform(const string& name, const ivec2& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, ivec2* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLint* values = new GLint[2*count];
    for (int i=0; i < count; i++) {
        values[2*i] = vectors[i].x;
        values[2*i+1] = vectors[i].y;
    }
    glUniform2iv(l, count, values);
    delete[] values;
    return true;
}

bool Shader::setUniform(const string& name, const ivec3& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, ivec3* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLint* values = new GLint[3*count];
    for (int i=0; i < count; i++) {
        values[3*i] = vectors[i].x;
        values[3*i+1] = vectors[i].y;
        values[3*i+2] = vectors[i].z;
    }
    glUniform3iv(l, count, values);
    delete[] values;
    return true;
}

bool Shader::setUniform(const string& name, const ivec4& value) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value);
    return true;
}

bool Shader::setUniform(const string& name, ivec4* vectors, GLsizei count) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    GLint* values = new GLint[4*count];
    for (int i=0; i < count; i++) {
        values[4*i] = vectors[i].x;
        values[4*i+1] = vectors[i].y;
        values[4*i+2] = vectors[i].z;
        values[4*i+3] = vectors[i].a;
    }
    glUniform4iv(l, count, values);
    delete[] values;
    return true;
}

// Note: Matrix is transposed by OpenGL
bool Shader::setUniform(const string& name, const Matrix2f& value, bool transpose) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value, transpose);
    return true;
}

bool Shader::setUniform(const string& name, const Matrix3f& value, bool transpose) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value, transpose);
    return true;
}

bool Shader::setUniform(const string& name, const Matrix4f& value, bool transpose) {
    GLint l = getUniformLocation(name);
    if (l == -1)
        return false;
    setUniform(l, value, transpose);
    return true;
}

// No location lookup
//
// Floats
void Shader::setUniform(GLint l, GLfloat value) {
    glUniform1f(l, value);
}

void Shader::setUniform(GLint l, GLfloat v1, GLfloat v2) {
    glUniform2f(l, v1, v2);
}

void Shader::setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3) {
    glUniform3f(l, v1, v2, v3);
}

void Shader::setUniform(GLint l, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    glUniform4f(l, v1, v2, v3, v4);
}

// Integers
void Shader::setUniform(GLint l, GLint value) {
    glUniform1i(l, value);
}

void Shader::setUniform(GLint l, GLint v1, GLint v2) {
    glUniform2i(l, v1, v2);
}

void Shader::setUniform(GLint l, GLint v1, GLint v2, GLint v3) {
    glUniform3i(l, v1, v2, v3);
}

void Shader::setUniform(GLint l, GLint v1, GLint v2, GLint v3, GLint v4) {
    glUniform4i(l, v1, v2, v3, v4);
}

// Vectors
void Shader::setUniform(GLint l, const Vector2f& value) {
    glUniform2fv(l, 1, value.elem);
}

void Shader::setUniform(GLint l, const Vector3f& value) {
    glUniform3fv(l, 1, value.elem);
}

void Shader::setUniform(GLint l, const Vector4f& value) {
    glUniform4fv(l, 1, value.elem);
}

void Shader::setUniform(GLint l, const ivec2& value) {
    glUniform2i(l, static_cast<GLint>(value.x), static_cast<GLint>(value.y));
}

void Shader::setUniform(GLint l, const ivec3& value) {
    glUniform3i(l, static_cast<GLint>(value.x), static_cast<GLint>(value.y), static_cast<GLint>(value.z));
}

void Shader::setUniform(GLint l, const ivec4& value) {
    glUniform4i(l, static_cast<GLint>(value.x), static_cast<GLint>(value.y),
                static_cast<GLint>(value.z), static_cast<GLint>(value.w));
}

void Shader::setUniform(GLint l, const Matrix2f& value, bool transpose) {
    glUniformMatrix2fv(l, 1, !transpose, value.elem);
}

void Shader::setUniform(GLint l, const Matrix3f& value, bool transpose) {
    glUniformMatrix3fv(l, 1, !transpose, value.elem);
}

void Shader::setUniform(GLint l, const Matrix4f& value, bool transpose) {
    glUniformMatrix4fv(l, 1, !transpose, value.elem);
}

// Attributes
//
// 1 component
void Shader::setAttribute(GLint index, GLfloat v1) {
    glVertexAttrib1f(index, v1);
}

void Shader::setAttribute(GLint index, GLshort v1) {
    glVertexAttrib1s(index, v1);
}

void Shader::setAttribute(GLint index, GLdouble v1) {
    glVertexAttrib1d(index, v1);
}

// 2 components
void Shader::setAttribute(GLint index, GLfloat v1, GLfloat v2) {
    glVertexAttrib2f(index, v1, v2);
}

void Shader::setAttribute(GLint index, GLshort v1, GLshort v2) {
    glVertexAttrib2s(index, v1, v2);
}

void Shader::setAttribute(GLint index, GLdouble v1, GLdouble v2) {
    glVertexAttrib2d(index, v1, v2);
}

// 3 components
void Shader::setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3) {
    glVertexAttrib3f(index, v1, v2, v3);
}

void Shader::setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3) {
    glVertexAttrib3s(index, v1, v2, v3);
}

void Shader::setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3) {
    glVertexAttrib3d(index, v1, v2, v3);
}

// 4 components
void Shader::setAttribute(GLint index, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    glVertexAttrib4f(index, v1, v2, v3, v4);
}

void Shader::setAttribute(GLint index, GLshort v1, GLshort v2, GLshort v3, GLshort v4) {
    glVertexAttrib4s(index, v1, v2, v3, v4);
}

void Shader::setAttribute(GLint index, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4) {
    glVertexAttrib4d(index, v1, v2, v3, v4);
}

// For vectors
void Shader::setAttribute(GLint index, const Vector2f& v) {
    glVertexAttrib2fv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector3f& v) {
    glVertexAttrib3fv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4f& v) {
    glVertexAttrib4fv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector2d& v) {
    glVertexAttrib2dv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector3d& v) {
    glVertexAttrib3dv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4d& v) {
    glVertexAttrib4dv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector2<GLshort>& v) {
    glVertexAttrib2sv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector3<GLshort>& v) {
    glVertexAttrib3sv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLshort>& v) {
    glVertexAttrib4sv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLint>& v) {
    glVertexAttrib4iv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLbyte>& v) {
    glVertexAttrib4bv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLubyte>& v) {
    glVertexAttrib4ubv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLushort>& v) {
    glVertexAttrib4usv(index, v.elem);
}

void Shader::setAttribute(GLint index, const Vector4<GLuint>& v) {
    glVertexAttrib4uiv(index, v.elem);
}

// Attribute locations
void Shader::setAttributeLocation(GLuint index, const std::string& name) {
    glBindAttribLocation(id_, index, name.c_str());
}

GLint Shader::getAttributeLocation(const string& name) {
    GLint l;
    l = glGetAttribLocation(id_, name.c_str());
    if (l == -1)
        LERROR("Failed to locate attribute Location: " << name);
    return l;
}

// Normalized attributes
void Shader::setNormalizedAttribute(GLint index, GLubyte v1, GLubyte v2, GLubyte v3, GLubyte v4) {
    glVertexAttrib4Nub(index, v1, v2, v3, v4);
}

void Shader::setNormalizedAttribute(GLint index, const Vector4<GLbyte>& v) {
    glVertexAttrib4Nbv(index, v.elem);
}

void Shader::setNormalizedAttribute(GLint index, const Vector4<GLshort>& v) {
    glVertexAttrib4Nsv(index, v.elem);
}

void Shader::setNormalizedAttribute(GLint index, const Vector4<GLint>& v) {
    glVertexAttrib4Niv(index, v.elem);
}

// Unsigned version
void Shader::setNormalizedAttribute(GLint index, const Vector4<GLubyte>& v) {
    glVertexAttrib4Nubv(index, v.elem);
}

void Shader::setNormalizedAttribute(GLint index, const Vector4<GLushort>& v) {
    glVertexAttrib4Nusv(index, v.elem);
}

void Shader::setNormalizedAttribute(GLint index, const Vector4<GLuint>& v) {
    glVertexAttrib4Nuiv(index, v.elem);
}

//------------------------------------------------------------------------------

const string ShaderManager::loggerCat_("tgt.Shader.Manager");

ShaderManager::ShaderManager()
  : ResourceManager<Shader>(false)
{}

Shader* ShaderManager::load(const string& filename, const string& customHeader,
                            bool activate)
                            throw (Exception)
{
    return loadSeparate(filename + ".vert", filename + ".frag", customHeader, activate);
}

Shader* ShaderManager::loadSeparate(const string& vert_filename, const string& frag_filename,
                                    const string& customHeader, bool activate)
                                    throw (Exception)
{
    return loadSeparate(vert_filename, "", frag_filename, customHeader, activate);
}

Shader* ShaderManager::loadSeparate(const string& vert_filename, const string& geom_filename,
                                    const string& frag_filename,
                                    const string& customHeader, bool activate)
                                    throw (Exception)
{
    LDEBUG("Loading files " << vert_filename << " and " << frag_filename);
    if (!GpuCaps.areShadersSupported()) {
        LERROR("Shaders are not supported.");
        throw Exception("Shaders are not supported.");
    }

    // create a somewhat unique identifier for this shader triple
    string identifier = vert_filename + "#" +  frag_filename + "#" + geom_filename;

    if (isLoaded(identifier)) {
        LDEBUG("Shader already loaded. Increase usage count.");
        increaseUsage(identifier);
        return get(identifier);
    }

    Shader* shdr = new Shader();

    // searching in all paths for every shader
    string vert_completeFilename;
    if (!vert_filename.empty())
        vert_completeFilename = completePath(vert_filename);

    string geom_completeFilename;
    if (!geom_filename.empty())
        geom_completeFilename = completePath(geom_filename);

    string frag_completeFilename;
    if (!frag_filename.empty())
        frag_completeFilename = completePath(frag_filename);

    // loading and linking found shaders
    try {
        shdr->loadSeparate(vert_completeFilename, geom_completeFilename,
                           frag_completeFilename, customHeader);
        // register even when caching is disabled, needed for rebuildFromFile()
        reg(shdr, identifier);

        if (activate)
            shdr->activate();

        return shdr;
    }
    catch (const Exception& /*e*/) {
        delete shdr;
        throw;
    }
}

bool ShaderManager::rebuildAllShadersFromFile() {
    bool result = true;

    for (std::map<Shader*, ResourceManager<Shader>::Resource*>::iterator iter = resourcesByPtr_.begin();
         iter != resourcesByPtr_.end(); ++iter)
    {
        result &= iter->first->rebuildFromFile();
    }

    return result;
}

} // namespace
