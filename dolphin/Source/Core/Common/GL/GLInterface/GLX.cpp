// Copyright 2012 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <array>
#include <sstream>

#include "Common/GL/GLInterface/GLX.h"
#include "Common/Logging/Log.h"
#include "../../../../../../include/gui.h"

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092

typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSPROC)(Display*, GLXFBConfig, GLXContext, Bool,
                                                     const int*);
typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display*, GLXDrawable, int);
typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int);

static PFNGLXCREATECONTEXTATTRIBSPROC glXCreateContextAttribs = nullptr;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXTPtr = nullptr;
static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESAPtr = nullptr;

static PFNGLXCREATEGLXPBUFFERSGIXPROC glXCreateGLXPbufferSGIX = nullptr;
static PFNGLXDESTROYGLXPBUFFERSGIXPROC glXDestroyGLXPbufferSGIX = nullptr;

static bool s_glxError;
//jc static int ctxErrorHandler(Display* dpy, XErrorEvent* ev)
//jc {
//jc   s_glxError = true;
//jc   return 0;
//jc }

GLContextGLX::~GLContextGLX()
{
  #ifdef JC_DEBUGGING  
  printf("jc GLContextGLX::~GLContextGLX() \n");
  #endif
  DestroyWindowSurface();
  if (m_context)
  {
    SDL_GL_DeleteContext(m_context);
//jc    if (glXGetCurrentContext() == m_context)
//jc      glXMakeCurrent(m_display, None, nullptr);
//jc
//jc    glXDestroyContext(m_display, m_context);
  }
}

bool GLContextGLX::IsHeadless() const
{
  return !m_render_window;
}

void GLContextGLX::SwapInterval(int Interval)
{
    #ifdef JC_DEBUGGING
    printf("jc GLContextGLX::SwapInterval() \n");
    #endif
    #define immediate_updates 0
    #define updates_synchronized 1
    SDL_GL_SetSwapInterval(updates_synchronized);
    /*
  if (!m_drawable)
    return;

  // Try EXT_swap_control, then MESA_swap_control.
  if (glXSwapIntervalEXTPtr)
    glXSwapIntervalEXTPtr(m_display, m_drawable, Interval);
  else if (glXSwapIntervalMESAPtr)
    glXSwapIntervalMESAPtr(static_cast<unsigned int>(Interval));
  else
    ERROR_LOG(VIDEO, "No support for SwapInterval (framerate clamped to monitor refresh rate).");
    */
}

void* GLContextGLX::GetFuncAddress(const std::string& name)
{
  return reinterpret_cast<void*>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(name.c_str())));
}

void GLContextGLX::Swap()
{
  //glXSwapBuffers(m_display, m_drawable);
  SDL_GL_SwapWindow(gWindow);
}

// Create rendering window.
// Call browser: Core.cpp:EmuThread() > main.cpp:Video_Initialize()
bool GLContextGLX::Initialize(const WindowSystemInfo& wsi, bool stereo, bool core)
{
  #warning "JC: modified"
  #ifdef JC_DEBUGGING  
  printf("jc GLContextGLX::Initialize() stereo: %i, core %i\n",stereo,core);
  #endif
  m_display = static_cast<Display*>(wsi.display_connection);
  int screen = SDL_GetWindowDisplayIndex(gWindow);

  // checking glx version
  int glxMajorVersion, glxMinorVersion;
  glXQueryVersion(m_display, &glxMajorVersion, &glxMinorVersion);
  if (glxMajorVersion < 1 || (glxMajorVersion == 1 && glxMinorVersion < 4))
  {
    ERROR_LOG(VIDEO, "glX-Version %d.%d detected, but need at least 1.4", glxMajorVersion,
              glxMinorVersion);
    return false;
  }

  // loading core context creation function
  glXCreateContextAttribs =
      (PFNGLXCREATECONTEXTATTRIBSPROC)GetFuncAddress("glXCreateContextAttribsARB");
  if (!glXCreateContextAttribs)
  {
    ERROR_LOG(VIDEO,
              "glXCreateContextAttribsARB not found, do you support GLX_ARB_create_context?");
    printf("glXCreateContextAttribsARB not found, do you support GLX_ARB_create_context?\n");
    return false;
  }
  
  /*
  // choosing framebuffer
  int visual_attribs[] = {GLX_X_RENDERABLE,
                          True,
                          GLX_DRAWABLE_TYPE,
                          GLX_WINDOW_BIT,
                          GLX_X_VISUAL_TYPE,
                          GLX_TRUE_COLOR,
                          GLX_RED_SIZE,
                          8,
                          GLX_GREEN_SIZE,
                          8,
                          GLX_BLUE_SIZE,
                          8,
                          GLX_DEPTH_SIZE,
                          0,
                          GLX_STENCIL_SIZE,
                          0,
                          GLX_DOUBLEBUFFER,
                          True,
                          GLX_STEREO,
                          stereo ? True : False,
                          None};
  
  //jc int fbcount = 0;
  GLXFBConfig* fbc = glXChooseFBConfig(m_display, screen, visual_attribs, &fbcount);
  if (!fbc || !fbcount)
  {
    ERROR_LOG(VIDEO, "Failed to retrieve a framebuffer config");
    return false;
  }
  m_fbconfig = *fbc;
  XFree(fbc);
  
  s_glxError = false;
  XErrorHandler oldHandler = XSetErrorHandler(&ctxErrorHandler);
  */
  
  // Create a GLX context.
  if (core)
  {
      m_context = SDL_GL_CreateContext(gWindow);
      /*
    for (const auto& version : s_desktop_opengl_versions)
    {
      std::array<int, 9> context_attribs = {
          {GLX_CONTEXT_MAJOR_VERSION_ARB, version.first, GLX_CONTEXT_MINOR_VERSION_ARB,
           version.second, GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
           GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, None}};

      s_glxError = false;
      m_context = glXCreateContextAttribs(m_display, m_fbconfig, 0, True, &context_attribs[0]);
      XSync(m_display, False);
      if (!m_context || s_glxError)
        continue;

      // Got a context.
      INFO_LOG(VIDEO, "Created a GLX context with version %d.%d", version.first, version.second);
      m_attribs.insert(m_attribs.end(), context_attribs.begin(), context_attribs.end());
      break;
    }*/
  }
  
  /*
  // Failed to create any core contexts, try for anything.
  if (!m_context || s_glxError)
  {
    std::array<int, 5> context_attribs_legacy = {
        {GLX_CONTEXT_MAJOR_VERSION_ARB, 1, GLX_CONTEXT_MINOR_VERSION_ARB, 0, None}};
    s_glxError = false;
    m_context = glXCreateContextAttribs(m_display, m_fbconfig, 0, True, &context_attribs_legacy[0]);
    XSync(m_display, False);
    m_attribs.clear();
    m_attribs.insert(m_attribs.end(), context_attribs_legacy.begin(), context_attribs_legacy.end());
  }
  if (!m_context || s_glxError)
  {
    ERROR_LOG(VIDEO, "Unable to create GL context.");
   //jc XSetErrorHandler(oldHandler);
    return false;
  }
  */

  glXSwapIntervalEXTPtr = nullptr;
  glXSwapIntervalMESAPtr = nullptr;
  glXCreateGLXPbufferSGIX = nullptr;
  glXDestroyGLXPbufferSGIX = nullptr;
  m_supports_pbuffer = false;
  
  std::string tmp;
  std::istringstream buffer(glXQueryExtensionsString(m_display, screen));
  while (buffer >> tmp)
  {
    if (tmp == "GLX_SGIX_pbuffer")
    {
      glXCreateGLXPbufferSGIX = reinterpret_cast<PFNGLXCREATEGLXPBUFFERSGIXPROC>(
          GetFuncAddress("glXCreateGLXPbufferSGIX"));
      glXDestroyGLXPbufferSGIX = reinterpret_cast<PFNGLXDESTROYGLXPBUFFERSGIXPROC>(
          GetFuncAddress("glXDestroyGLXPbufferSGIX"));
      m_supports_pbuffer = glXCreateGLXPbufferSGIX && glXDestroyGLXPbufferSGIX;
    }
    else if (tmp == "GLX_EXT_swap_control")
    {
      glXSwapIntervalEXTPtr =
          reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(GetFuncAddress("glXSwapIntervalEXT"));
    }
    else if (tmp == "GLX_MESA_swap_control")
    {
        printf("jc found extension GLX_MESA_swap_control\n");
      glXSwapIntervalMESAPtr =
          reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(GetFuncAddress("glXSwapIntervalMESA"));
    }
  }

  if (!CreateWindowSurface(reinterpret_cast<Window>(wsi.render_surface)))
  {
    ERROR_LOG(VIDEO, "Error: CreateWindowSurface failed\n");
    printf("Error: CreateWindowSurface failed\n");
    //jcXSetErrorHandler(oldHandler);
    return false;
  }

  //jcXSetErrorHandler(oldHandler);
  m_opengl_mode = Mode::OpenGL;
  #ifdef JC_DEBUGGING
  printf("jc GLContextGLX::Initialize() end\n");
  #endif
  return MakeCurrent();
}

std::unique_ptr<GLContext> GLContextGLX::CreateSharedContext()
{
  #warning "JC: modified"
  #ifdef JC_DEBUGGING
  printf("jc GLContextGLX::CreateSharedContext()\n");
  #endif
  s_glxError = false;
  //jcXErrorHandler oldHandler = XSetErrorHandler(&ctxErrorHandler);

  //jcGLXContext new_glx_context =
  //jc    glXCreateContextAttribs(m_display, m_fbconfig, m_context, True, &m_attribs[0]);
  //XSync(m_display, False);

  /*if (!new_glx_context || s_glxError)
  {
    ERROR_LOG(VIDEO, "Unable to create GL context.");
    XSetErrorHandler(oldHandler);
    return nullptr;
  }*/

  std::unique_ptr<GLContextGLX> new_context = std::make_unique<GLContextGLX>();
  new_context->m_context = m_context;
  new_context->m_opengl_mode = m_opengl_mode;
  new_context->m_supports_pbuffer = m_supports_pbuffer;
  new_context->m_display = m_display;
  new_context->m_fbconfig = m_fbconfig;
  new_context->m_is_shared = true;

  if (m_supports_pbuffer && !new_context->CreateWindowSurface(None))
  {
    ERROR_LOG(VIDEO, "Error: CreateWindowSurface failed");
    printf("Error: CreateWindowSurface failed\n");
    //jcXSetErrorHandler(oldHandler);
    return nullptr;
  }

  //jcXSetErrorHandler(oldHandler);
  return new_context;
}

bool GLContextGLX::CreateWindowSurface(Window window_handle)
{
    #warning "JC: modified"
    /*
  if (window_handle)
  {
    // Get an appropriate visual
    XVisualInfo* vi = glXGetVisualFromFBConfig(m_display, m_fbconfig);
    m_render_window = GLX11Window::Create(m_display, window_handle, vi);
    if (!m_render_window)
      return false;

    m_backbuffer_width = m_render_window->GetWidth();
    m_backbuffer_height = m_render_window->GetHeight();
    m_drawable = static_cast<GLXDrawable>(m_render_window->GetWindow());
    XFree(vi);
  }
  else if (m_supports_pbuffer)
  {
    m_pbuffer = glXCreateGLXPbufferSGIX(m_display, m_fbconfig, 1, 1, nullptr);
    if (!m_pbuffer)
      return false;

    m_drawable = static_cast<GLXDrawable>(m_pbuffer);
  }*/
  int w,h;
  SDL_GetWindowSize(gWindow,&w,&h);
  m_backbuffer_width = w;
  m_backbuffer_height = h;
  #ifdef JC_DEBUGGING
  printf("jc GLContextGLX::CreateWindowSurface() width: %i, height: %i\n",m_backbuffer_width,m_backbuffer_height);
  #endif
  
  XVisualInfo* vi = NULL;
  m_render_window = GLX11Window::Create(m_display, window_handle, vi);

  return true;
}

void GLContextGLX::DestroyWindowSurface()
{
  //jc m_render_window.reset();
  if (m_supports_pbuffer && m_pbuffer)
  {
    //jc glXDestroyGLXPbufferSGIX(m_display, m_pbuffer);
    m_pbuffer = 0;
  }
}

bool GLContextGLX::MakeCurrent()
{
  bool ok = ((SDL_GL_MakeCurrent(gWindow, m_context) == 0));
  #ifdef JC_DEBUGGING
  if (ok) 
  {
      printf("jc GLContextGLX::MakeCurrent() ok\n");
  }
  else
  {
      printf("jc GLContextGLX::MakeCurrent() not ok %s\n", SDL_GetError());
  }
  #endif
  //jcreturn glXMakeCurrent(m_display, m_drawable, m_context);
  return true;
}

bool GLContextGLX::ClearCurrent()
{
  #ifdef JC_DEBUGGING
  printf("jc GLContextGLX::ClearCurrent()\n");
  #endif
  //jc return glXMakeCurrent(m_display, None, nullptr);
  return true;
}

void GLContextGLX::Update()
{
    #ifdef JC_DEBUGGING
    printf("jc GLContextGLX::Update()\n");
    #endif
  //jc m_render_window->UpdateDimensions();
  m_backbuffer_width = m_render_window->GetWidth();
  m_backbuffer_height = m_render_window->GetHeight();
}
