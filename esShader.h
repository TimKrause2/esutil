// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
//
/// \file ESUtil.h
/// \brief A utility library for OpenGL ES.  This library provides a
///        basic common framework for the example applications in the
///        OpenGL ES 3.0 Programming Guide.
//
#ifndef ESUTIL_H
#define ESUTIL_H

///
//  Includes
//
#include <stdlib.h>

//#ifdef __APPLE__
//#include <OpenGLES/ES3/gl.h>
//#else
//#include <GLES3/gl3.h>
//#endif'

#include <GL/glew.h>

#ifdef __cplusplus

extern "C" {
#endif


///
//  Macros
//
#ifdef WIN32
#define ESUTIL_API  __cdecl
#define ESCALLBACK  __cdecl
#else
#define ESUTIL_API
#define ESCALLBACK
#endif


/// esCreateWindow flag - RGB color buffer
#define ES_WINDOW_RGB           0
/// esCreateWindow flag - ALPHA color buffer
#define ES_WINDOW_ALPHA         1
/// esCreateWindow flag - depth buffer
#define ES_WINDOW_DEPTH         2
/// esCreateWindow flag - stencil buffer
#define ES_WINDOW_STENCIL       4
/// esCreateWindow flat - multi-sample buffer
#define ES_WINDOW_MULTISAMPLE   8


///
// Types
//
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


//
///
/// \brief Load a shader, check for compile errors, print error messages to output log
/// \param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
/// \param shaderSrc Shader source string
/// \return A new shader object on success, 0 on failure
//
GLuint ESUTIL_API esLoadShader ( GLenum type, const char *shaderSrc );

GLuint esLoadShaderFile(GLenum type, const char *shaderFile);

//
///
/// \brief Load a vertex and fragment shader, create a program object, link program.
///        Errors output to log.
/// \param vertShaderSrc Vertex shader source code
/// \param fragShaderSrc Fragment shader source code
/// \return A new program object linked with the vertex/fragment shader pair, 0 on failure
//
GLuint ESUTIL_API esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc );

GLuint esLoadProgramFiles(const char *vertShaderFile, const char *fragShaderFile);

#ifdef __cplusplus
}
#endif

#endif // ESUTIL_H
