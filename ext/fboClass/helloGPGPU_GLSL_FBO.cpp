//---------------------------------------------------------------------------
//                               www.GPGPU.org
//                                Sample Code
//---------------------------------------------------------------------------
// Copyright (c) 2004 Mark J. Harris and GPGPU.org
// Copyright (c) 2004 3Dlabs Inc. Ltd.
//---------------------------------------------------------------------------
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you
//    must not claim that you wrote the original software. If you use
//    this software in a product, an acknowledgment in the product
//    documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and
//    must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//---------------------------------------------------------------------------
// Author: Mark Harris (harrism@gpgpu.org) - original helloGPGPU
// Author: Mike Weiblen (mike.weiblen@3dlabs.com) - GLSL version
// Author: Martin Dvh (nldudok1 olifantasia.com) - FBO (Framebuffer Object) version
// Author: Aaron Lefohn  - Framebuffer Object (FBO) and Renderbuffer class version
//---------------------------------------------------------------------------
// GPGPU Lesson 0: "helloGPGPU_GLSL" (a GLSL version of "helloGPGPU")
//---------------------------------------------------------------------------
//
// GPGPU CONCEPTS Introduced:
//
//      1.) Texture = Array
//      2.) Fragment Program = Computational Kernel.
//      3.) One-to-one Pixel to Texel Mapping:
//          a) Data-Dimensioned Viewport, and
//          b) Orthographic Projection.
//      4.) Viewport-Sized Quad = Data Stream Generator.
//      5.) Copy To Texture = feedback.
//
//      For details of each of these concepts, see the explanations in the
//      inline "GPGPU CONCEPT" comments in the code below.
//
// APPLICATION Demonstrated: A simple post-process edge detection filter.
//
//---------------------------------------------------------------------------
// Notes regarding this "helloGPGPU_GLSL_FBO" source code.
// This example was derived from the "helloGPGPU_GLSL" sourcecode which in its turn was based on "helloGPGPU.cpp" v1.0.1
// The major modification is that now this example uses Framebuffer objects to implement render-to-texture
// This is a new opengl feature and is easier to use then using pbuffers.
// It is also much faster then copy-to-texture and also faster then using pbuffers for doing render-to-texture
// This is because context switches are not neccesary, even when using more then 2 textures/surfaces.
// I used the concepts from the Nvidia Opengl Framebuffer Object presentation document and
// the framebuffer object extension specification at the opengl extension registry.
// For this example to work you need a videodriver which supports the FBO extension
// At the time of writing (19 april 2005) I only know of FBOs being supported in
// NVIDIA Beta drivers for windows version 75.x or greater.(I don't know about other vendors)
//
// I also included visual studio 7.1 and visual studio 6 project and solution files.
// The visual studio 6 project file is not recently tested.
// There is also still a linux makefile. This will be usefull as soon as drivers for linux appear which include FBO support.
// Note that the example requires a recent glew.h and glew32s.lib, available at
// http://glew.sourceforge.net. You need version 1.31 or higher
//
// References:
//   http://gpgpu.sourceforge.net/
//   http://oss.sgi.com/projects/ogl-sample/registry/EXT/framebuffer_object.txt
//   http://download.nvidia.com/developer/presentations/2005/GDC/OpenGL_Day/OpenGL_FrameBuffer_Object.pdf
//   http://glew.sourceforge.net
//
//  Thanks to Mark and Mike for making the original example
//  Also Thanks for the opengl ARB and Nvidia for defining and implementing framebufferobject support
// (Please also implement this for linux soon)
//
//  -- Martin Dvh, April 2005
//---------------------------------------------------------------------------
// Notes regarding the "helloGPGPU_GLSL" source code which was the base for "helloGPGPU_GLSL_FBO":
//
// This example was derived from the original "helloGPGPU.cpp" v1.0.1
// by Mark J. Harris.  It demonstrates the same simple post-process edge
// detection filter, but instead implemented using the OpenGL Shading Language
// (also known as "GLSL"), an extension of the OpenGL v1.5 specification.
// Because the GLSL compiler is an integral part of a vendor's OpenGL driver,
// no additional GLSL development tools are required.
// This example was developed/tested on 3Dlabs Wildcat Realizm.
//
// I intentionally minimized changes to the structure of the original code
// to support a side-by-side comparison of the implementations.
//
// Thanks to Mark for making the original helloGPGPU example available!
//
// -- Mike Weiblen, May 2004
//
//
// [MJH:]
// This example has also been tested on NVIDIA GeForce FX and GeForce 6800 GPUs.
//
// Note that the example requires glew.h and glew32s.lib, available at
// http://glew.sourceforge.net.
//
// Thanks to Mike for modifying the example.  I have actually changed my
// original code to match; for example the inline Cg code instead of an
// external file.
//
// -- Mark Harris, June 2004
//
// References:
//    http://gpgpu.sourceforge.net/
//    http://glew.sourceforge.net/
//    http://www.xmission.com/~nate/glut.html
//
//
// [AEL]
// This example encapsulates the FBO code written by Mike Weiblen in an FBO
// class that is now included in the visual studio project. This is intended
// to serve as a simple example of our Framebuffer object and renderbuffer classes.
//
// -- Aaron Lefohn, June 2005
//---------------------------------------------------------------------------

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
//define GLEW_STATIC 1
#include <GL/glew.h>
#include <GL/glut.h>
#include "framebufferObject.h"
#include "renderbuffer.h"
#include "glErrorUtil.h"

// forward declarations
class HelloGPGPU;
void reshape(int w, int h);

// globals
HelloGPGPU  *g_pHello;

// This shader performs a 9-tap Laplacian edge detection filter.
// (converted from the separate "edges.cg" file to embedded GLSL string)
static const char *edgeFragSource = {
  "uniform sampler2D texUnit;"
    "void main(void)"
    "{"
    "   const float offset = 1.0 / 512.0;"
    "   vec2 texCoord = gl_TexCoord[0].xy;"
    "   vec4 c  = texture2D(texUnit, texCoord);"
    "   vec4 bl = texture2D(texUnit, texCoord + vec2(-offset, -offset));"
    "   vec4 l  = texture2D(texUnit, texCoord + vec2(-offset,     0.0));"
    "   vec4 tl = texture2D(texUnit, texCoord + vec2(-offset,  offset));"
    "   vec4 t  = texture2D(texUnit, texCoord + vec2(    0.0,  offset));"
    "   vec4 ur = texture2D(texUnit, texCoord + vec2( offset,  offset));"
    "   vec4 r  = texture2D(texUnit, texCoord + vec2( offset,     0.0));"
    "   vec4 br = texture2D(texUnit, texCoord + vec2( offset,  offset));"
    "   vec4 b  = texture2D(texUnit, texCoord + vec2(    0.0, -offset));"
    "   gl_FragColor = 8.0 * (c + -0.125 * (bl + l + tl + t + ur + r + br + b));"
    "}"
};

// This class encapsulates all of the GPGPU functionality of the example.
class HelloGPGPU
{
public: // methods
  HelloGPGPU(int w, int h)
    : _rAngle(0),
    _iWidth(w),
    _iHeight(h),
    _fbo(),// create a new frame buffer object
    _rb()  // optional: create a new render buffer object
  {
    // Create a simple 2D texture.
    // This example uses render to texture  by using the new Framebuffer Objects.
    //At the time of writing (1 july 2005) these are only implemented
    //in NVIDIA drivers for windows/linux.(I don't know about other vendors)
    //glew version 1.31 or newer is required.

    // GPGPU CONCEPT 1: Texture = Array.
    // Textures are the GPGPU equivalent of arrays in standard
    // computation. Here we allocate a texture large enough to fit our
    // data (which is arbitrary in this example).

    // OpenGL Good Programming Practice : Check errors
    // - OpenGL errors accumulate silently until checked.
    //   This utility checks the accumulated errors and reports
    //   them (to cerr by default). The error checking disappears
    //   with release builds by defining NDEBUG.
    CheckErrorsGL("BEGIN : Creating textures");
    glGenTextures(2, _iTexture); // create (reference to) a new texture

    // initialize the two textures
    // The first is used as rendering target for the sample scene
    //This one is used as reading source for the filter
    //The destination of the filter is the second texture
    //This second texture is the source for the final rendering display to the screen
    for(int i=0;i<2;i++)
    {
      glBindTexture(GL_TEXTURE_2D, _iTexture[i]);
      // (set texture parameters here)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      //create the texture
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _iWidth, _iHeight,
        0, GL_RGB, GL_FLOAT, 0);
    }
    CheckErrorsGL("END : Creating textures");

    _fbo.Bind(); // Bind framebuffer object.
    CheckErrorsGL("BEGIN : Configuring FBO");

    // Attach texture to framebuffer color buffer
    _fbo.AttachTexture(GL_TEXTURE_2D, _iTexture[0], GL_COLOR_ATTACHMENT0_EXT);
    _fbo.AttachTexture(GL_TEXTURE_2D, _iTexture[1], GL_COLOR_ATTACHMENT1_EXT);

    // Optional: initialize depth renderbuffer
    _rb.Set( GL_DEPTH_COMPONENT24, _iWidth, _iHeight );
    _fbo.AttachRenderBuffer( _rb.GetId(), GL_DEPTH_ATTACHMENT_EXT );

    // Validate the FBO after attaching textures and render buffers
    _fbo.IsValid();

    // Disable FBO rendering for now...
    FramebufferObject::Disable();
    CheckErrorsGL("END : Configuring FBO");

    // GPGPU CONCEPT 2: Fragment Program = Computational Kernel.
    // A fragment program can be thought of as a small computational
    // kernel that is applied in parallel to many fragments
    // simultaneously.  Here we load a kernel that performs an edge
    // detection filter on an image.
    _programObject = glCreateProgramObjectARB();
    CheckErrorsGL("BEGIN : Configuring Shader");

    // Create the edge detection fragment program
    _fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    glShaderSourceARB(_fragmentShader, 1, &edgeFragSource, NULL);
    glCompileShaderARB(_fragmentShader);
    glAttachObjectARB(_programObject, _fragmentShader);

    // Link the shader into a complete GLSL program.
    glLinkProgramARB(_programObject);
    GLint progLinkSuccess;
    glGetObjectParameterivARB(_programObject, GL_OBJECT_LINK_STATUS_ARB,
      &progLinkSuccess);
    if (!progLinkSuccess)
    {
      fprintf(stderr, "Filter shader could not be linked\n");
      exit(1);
    }

    // Get location of the sampler uniform
    _texUnit = glGetUniformLocationARB(_programObject, "texUnit");
    CheckErrorsGL("END : Configuring Shader");
  }

  ~HelloGPGPU()
  {}

  // This method updates the texture by rendering the geometry (a teapot
  // and 3 rotating tori) and copying the image to a texture.
  // It then renders a second pass using the texture as input to an edge
  // detection filter.  It copies the results of the filter to the texture.
  // The texture is used in HelloGPGPU::display() for displaying the
  // results.
  void update()
  {
    CheckErrorsGL("BEGIN : HelloGPGPU::update()");
    glGetIntegerv(GL_DRAW_BUFFER, &_currentDrawbuf); // Save the current Draw buffer
    _fbo.Bind(); 									 // Render to the FBO
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);			 // Draw into the first texture
    _rAngle += 0.5f;
    CheckErrorsGL("HelloGPGPU::update() : After FBO setup");

    // store the window viewport dimensions so we can reset them,
    // and set the viewport to the dimensions of our texture
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);

    // GPGPU CONCEPT 3a: One-to-one Pixel to Texel Mapping: A Data-
    //                   Dimensioned Viewport.
    // We need a one-to-one mapping of pixels to texels in order to
    // ensure every element of our texture is processed. By setting our
    // viewport to the dimensions of our destination texture and drawing
    // a screen-sized quad (see below), we ensure that every pixel of our
    // texel is generated and processed in the fragment program.
    glViewport(0, 0, _iWidth, _iHeight);

    // Render a teapot and 3 tori
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glRotatef(-_rAngle, 0, 1, 0.25);
    glutSolidTeapot(0.5);
    glPopMatrix();
    glPushMatrix();
    glRotatef(2.1 * _rAngle, 1, 0.5, 0);
    glutSolidTorus(0.05, 0.9, 64, 64);
    glPopMatrix();
    glPushMatrix();
    glRotatef(-1.5 * _rAngle, 0, 1, 0.5);
    glutSolidTorus(0.05, 0.9, 64, 64);
    glPopMatrix();
    glPushMatrix();
    glRotatef(1.78 * _rAngle, 0.5, 0, 1);
    glutSolidTorus(0.05, 0.9, 64, 64);
    glPopMatrix();
    CheckErrorsGL("HelloGPGPU::update() : After first render pass");

    glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);//Draw into the second texture
    // read from the first texture
    glBindTexture(GL_TEXTURE_2D, _iTexture[0]);
    //No need for copy glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _iWidth, _iHeight);

    // run the edge detection filter over the geometry texture
    // Activate the edge detection filter program
    glUseProgramObjectARB(_programObject);

    // identify the bound texture unit as input to the filter
    glUniform1iARB(_texUnit, 0);

    // GPGPU CONCEPT 4: Viewport-Sized Quad = Data Stream Generator.
    // In order to execute fragment programs, we need to generate pixels.
    // Drawing a quad the size of our viewport (see above) generates a
    // fragment for every pixel of our destination texture. Each fragment
    // is processed identically by the fragment program. Notice that in
    // the reshape() function, below, we have set the frustum to
    // orthographic, and the frustum dimensions to [-1,1].  Thus, our
    // viewport-sized quad vertices are at [-1,-1], [1,-1], [1,1], and
    // [-1,1]: the corners of the viewport.
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0, 0); glVertex3f(-1, -1, -0.5f);
      glTexCoord2f(1, 0); glVertex3f( 1, -1, -0.5f);
      glTexCoord2f(1, 1); glVertex3f( 1,  1, -0.5f);
      glTexCoord2f(0, 1); glVertex3f(-1,  1, -0.5f);
    }
    glEnd();
    CheckErrorsGL("HelloGPGPU::update() : After second render pass");

    // disable the filter
    glUseProgramObjectARB(0);

    // GPGPU CONCEPT 5: Copy To Texture (CTT) = Feedback.
    // We have just invoked our computation (edge detection) by applying
    // a fragment program to a viewport-sized quad. The results are now
    // in the frame buffer. To store them, we copy the data from the
    // frame buffer to a texture.  This can then be fed back as input
    // for display (in this case) or more computation (see
    // more advanced samples.)

    // update the texture again, this time with the filtered scene
    //glBindTexture(GL_TEXTURE_2D, _iTexture[0]);
    //glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _iWidth, _iHeight);

    // restore the stored viewport dimensions
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    CheckErrorsGL("END : HelloGPGPU::update()");
  }

  void display()
  {
    CheckErrorsGL("BEGIN : HelloGPGPU::display()");

    // Disable FBO rendering; Render to the window
    FramebufferObject::Disable();
    glDrawBuffer(_currentDrawbuf);

    // Bind the filtered texture
    glBindTexture(GL_TEXTURE_2D, _iTexture[1]);//read from the second texture
    glEnable(GL_TEXTURE_2D);
    CheckErrorsGL("HelloGPGPU::display() : After FBO/Texture setup");

    // Render a full-screen quad textured with the results of our
    // computation.  Note that this is not part of the computation: this
    // is only the visualization of the results.
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0, 0); glVertex3f(-1, -1, -0.5f);
      glTexCoord2f(1, 0); glVertex3f( 1, -1, -0.5f);
      glTexCoord2f(1, 1); glVertex3f( 1,  1, -0.5f);
      glTexCoord2f(0, 1); glVertex3f(-1,  1, -0.5f);
    }
    glEnd();
    CheckErrorsGL("HelloGPGPU::display() : After render pass");

    glDisable(GL_TEXTURE_2D);
    CheckErrorsGL("END : HelloGPGPU::display()");
  }

protected: // data
  int           _iWidth, _iHeight; // The dimensions of our array
  float         _rAngle;           // used for animation

  GLuint			   _iTexture[2]; // The texture used as a data array
  FramebufferObject  _fbo;		 // The framebuffer object used for rendering to the texture
  Renderbuffer	   _rb;	         // Optional: The renderbuffer object used for depth

  GLhandleARB   _programObject;    // the program used to update
  GLhandleARB   _fragmentShader;

  GLint         _texUnit;          // a parameter to the fragment program
  GLint		  _currentDrawbuf;
};

// GLUT idle function
void idle()
{
  glutPostRedisplay();
}

// GLUT display function
void display()
{
  g_pHello->update();  // update the scene and run the edge detect filter
  g_pHello->display(); // display the results
  glutSwapBuffers();
}

// GLUT reshape function
void reshape(int w, int h)
{
  if (h == 0) h = 1;

  glViewport(0, 0, w, h);

  // GPGPU CONCEPT 3b: One-to-one Pixel to Texel Mapping: An Orthographic
  //                   Projection.
  // This code sets the projection matrix to orthographic with a range of
  // [-1,1] in the X and Y dimensions. This allows a trivial mapping of
  // pixels to texels.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// Called at startup
void initialize()
{
  // Initialize the "OpenGL Extension Wrangler" library
  glewInit();

  // Ensure we have the necessary OpenGL Shading Language extensions.
  if (glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
      glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
      glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
      glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)    {
        fprintf(stderr, "Driver does not support OpenGL Shading Language\n");
        exit(1);
  }

  if (glewGetExtension("GL_EXT_framebuffer_object") != GL_TRUE) {
    fprintf(stderr, "Driver does not support Framebuffer Objects (GL_EXT_framebuffer_object)\n");
    exit(1);
  }

  // Create the example object
  g_pHello = new HelloGPGPU(512, 512);
}

// The main function
int main()
{
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(512, 512);
  glutCreateWindow("Hello, GPGPU! (GLSL version)");

  glutIdleFunc(idle);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  initialize();

  glutMainLoop();
  return 0;
}
