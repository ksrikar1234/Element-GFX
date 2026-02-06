#ifndef _GRIDPRO_GLEW_OPENGL_API_
#define _GRIDPRO_GLEW_OPENGL_API_

#include <stdexcept>

#include <GL/glew.h>

class GLEW_OpenGLFunctions_4_1_Core
{
    public :
    GLEW_OpenGLFunctions_4_1_Core() = default;
   ~GLEW_OpenGLFunctions_4_1_Core() = default;
    
    bool initializeOpenGLFunctions()
    {
        //glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            throw std::runtime_error("Error: " + std::string((const char*)glewGetErrorString(err)));
            return false;
        }
        return true;
    }

    GLenum glGetError()
    {
        return ::glGetError();
    }

    void glClear(GLbitfield mask)
    {
        ::glClear(mask);
    }
    
    void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        ::glClearColor(red, green, blue, alpha);
    }

    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        ::glViewport(x, y, width, height);
    }

    void glEnable(GLenum cap)
    {
        ::glEnable(cap);
    }

    void glDisable(GLenum cap)
    {
        ::glDisable(cap);
    }

    void glDepthFunc(GLenum func)
    {
        ::glDepthFunc(func);
    }

    void glBlendFunc(GLenum sfactor, GLenum dfactor)
    {
        ::glBlendFunc(sfactor, dfactor);
    }

    void glBlendEquation(GLenum mode)
    {
        ::glBlendEquation(mode);
    }

    void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
    {
        ::glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
    {
        ::glBlendEquationSeparate(modeRGB, modeAlpha);
    }

    void glCullFace(GLenum mode)
    {
        ::glCullFace(mode);
    }

    void glFrontFace(GLenum mode)
    {
        ::glFrontFace(mode);
    }

    void glPolygonMode(GLenum face, GLenum mode)
    {
        ::glPolygonMode(face, mode);
    }
    
    void glPolygonOffset(GLfloat factor, GLfloat units)
    {
        ::glPolygonOffset(factor, units);
    }

    void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        ::glScissor(x, y, width, height);
    }

    void glLineWidth(GLfloat width)
    {
        ::glLineWidth(width);
    }

    void glPointSize(GLfloat size)
    {
        ::glPointSize(size);
    }

    void glClearDepth(GLclampd depth)
    {
        ::glClearDepth(depth);
    }

    void glClearStencil(GLint s)
    {
        ::glClearStencil(s);
    }
  
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *data)
    {
        ::glReadPixels(x, y, width, height, format, type, data);
    }

    void glReadBuffer(GLenum mode)
    {
        ::glReadBuffer(mode);
    }

    void glGetIntegerv(GLenum pname, GLint *data)
    {
        ::glGetIntegerv(pname, data);
    }   
 
    void glDrawArrays(GLenum mode, GLint first, GLsizei count)
    {
        ::glDrawArrays(mode, first, count);
    }

    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
    {
        ::glDrawElements(mode, count, type, indices);
    }

    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
    {
        ::glDrawRangeElements(mode, start, end, count, type, indices);
    }

    void glGenVertexArrays(GLsizei n, GLuint *arrays)
    {
        ::glGenVertexArrays(n, arrays);
    }

    void glBindVertexArray(GLuint array)
    {
        ::glBindVertexArray(array);
    }

    void glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
    {
        ::glDeleteVertexArrays(n, arrays);
    }

    bool glIsVertexArray(GLuint array)
    {
        return ::glIsVertexArray(array);
    }
    
    void glGenBuffers(GLsizei n, GLuint *buffers)
    {
        ::glGenBuffers(n, buffers);
    }
    
    bool glIsBuffer(GLuint buffer)
    {
        return ::glIsBuffer(buffer);
    }
    
    void glBindBuffer(GLenum target, GLuint buffer)
    {
        ::glBindBuffer(target, buffer);
    }

    void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
    {
        ::glBufferData(target, size, data, usage);
    }

    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
    {
        ::glBufferSubData(target, offset, size, data);
    }

    void glDeleteBuffers(GLsizei n, const GLuint *buffers)
    {
        ::glDeleteBuffers(n, buffers);
    }

    void glEnableVertexAttribArray(GLuint index)
    {
        ::glEnableVertexAttribArray(index);
    }

    void glDisableVertexAttribArray(GLuint index)
    {
        ::glDisableVertexAttribArray(index);
    }
    
    GLint glGetAttribLocation(GLuint program, const GLchar *name)
    {
        return ::glGetAttribLocation(program, name);
    }

    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
    {
        ::glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    }

    void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
    {
        ::glVertexAttribIPointer(index, size, type, stride, pointer);
    }

    void glVertexAttribDivisor(GLuint index, GLuint divisor)
    {
        ::glVertexAttribDivisor(index, divisor);
    }

    void glGenTextures(GLsizei n, GLuint *textures)
    {
        ::glGenTextures(n, textures);
    }

    void glBindTexture(GLenum target, GLuint texture)
    {
        ::glBindTexture(target, texture);
    }

    void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data)
    {
        ::glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
    }

    void glTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        ::glTexParameteri(target, pname, param);
    }

    void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
    {
        ::glTexParameterf(target, pname, param);
    }

    void glGenerateMipmap(GLenum target)
    {
        ::glGenerateMipmap(target);
    }

    void glDeleteTextures(GLsizei n, const GLuint *textures)
    {
        ::glDeleteTextures(n, textures);
    }

    void glActiveTexture(GLenum texture)
    {
        ::glActiveTexture(texture);
    }

    void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
    {
        ::glGetShaderiv(shader, pname, params);
    }

    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
    {
        ::glGetShaderInfoLog(shader, bufSize, length, infoLog);
    }

    GLint glCreateShader(GLenum type)
    {
       return ::glCreateShader(type);
    }
    
    void glValidateProgram(GLuint program)
    {
       ::glValidateProgram(program);
    }


    void glUseProgram(GLuint program)
    {
        ::glUseProgram(program);
    }

    void glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length)
    {
        ::glShaderSource(shader, count, string, length);
    }

    void glCompileShader(GLuint shader)
    {
        ::glCompileShader(shader);
    }


    void glDeleteShader(GLuint shader)
    {
        ::glDeleteShader(shader);
    }

    GLint glCreateProgram()
    {
        return ::glCreateProgram();
    }

    void glAttachShader(GLuint program, GLuint shader)
    {
        ::glAttachShader(program, shader);
    }

    void glLinkProgram(GLuint program)
    {
        ::glLinkProgram(program);
    }

    void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
    {
        ::glGetProgramiv(program, pname, params);
    }

    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
    {
        ::glGetProgramInfoLog(program, bufSize, length, infoLog);
    }

    void glDeleteProgram(GLuint program)
    {
        ::glDeleteProgram(program);
    }

    void glUniform1i(GLint location, GLint v0)
    {
        ::glUniform1i(location, v0);
    }

    void glUniform1f(GLint location, GLfloat v0)
    {
        ::glUniform1f(location, v0);
    }

    void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
    {
        ::glUniform2f(location, v0, v1);
    }

    void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
    {
        ::glUniform3f(location, v0, v1, v2);
    }

    void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {
        ::glUniform4f(location, v0, v1, v2, v3);
    }

    void glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
    {
        ::glUniform2fv(location, count, value);
    }

    void glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
    {
        ::glUniform3fv(location, count, value);
    }

    void glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
    {
        ::glUniform4fv(location, count, value);
    }

    void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
    {
        ::glUniformMatrix3fv(location, count, transpose, value);
    }

    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
    {
        ::glUniformMatrix4fv(location, count, transpose, value);
    }

    GLint glGetUniformLocation(GLuint program, const GLchar *name)
    {
        return ::glGetUniformLocation(program, name);
    }

};

#endif // _GRIDPRO_GLEW_OPENGL_API_