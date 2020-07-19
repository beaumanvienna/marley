/*
 *	Copyright (C) 2007-2012 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSWndOGL.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>

#if defined(__unix__)
GSWndOGL::GSWndOGL()
	: m_NativeWindow(0), m_NativeDisplay(nullptr), m_context(0), m_has_late_vsync(false), m_swapinterval_ext(nullptr), m_swapinterval_mesa(nullptr)
{
}

static bool ctxError = false;
static int  ctxErrorHandler(Display *dpy, XErrorEvent *ev)
{
	ctxError = true;
	return 0;
}
extern SDL_Window* gWindow;
namespace GLLoader {
	void check_gl_version(int major,int minor);
}
SDL_GLContext SDLcontext;
void GSWndOGL::CreateContext(int major, int minor)
{
	if ( !m_NativeDisplay || !m_NativeWindow )
	{
		fprintf( stderr, "Wrong X11 display/window\n" );
		throw GSDXRecoverableError();
	}
    
    SDL_GLContext SDLcontext = SDL_GL_CreateContext(gWindow);
    #define immediate_updates 0
    #define updates_synchronized 1
    SDL_GL_SetSwapInterval(updates_synchronized);

    m_context = glXGetCurrentContext();

	// Get latest error
	XSync( m_NativeDisplay, false);

	if (!m_context) {
		fprintf(stderr, "Failed to create the opengl context. Check your drivers support openGL %d.%d. Hint: opensource drivers don't\n", major, minor );
		throw GSDXRecoverableError();
	}
}

void GSWndOGL::AttachContext()
{
	if (!IsContextAttached()) {
		glXMakeCurrent(m_NativeDisplay, m_NativeWindow, m_context);
		m_ctx_attached = true;
	}
}

void GSWndOGL::DetachContext()
{
	if (IsContextAttached()) {
		//glXMakeCurrent(m_NativeDisplay, None, NULL);
		m_ctx_attached = false;
	}
}

void GSWndOGL::PopulateWndGlFunction()
{
	m_swapinterval_ext  = (PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddress((const GLubyte*) "glXSwapIntervalEXT");
	m_swapinterval_mesa = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");

	const char* ext = glXQueryExtensionsString(m_NativeDisplay, DefaultScreen(m_NativeDisplay));
	m_has_late_vsync = m_swapinterval_ext && ext && strstr(ext, "GLX_EXT_swap_control");
}
extern Display* XDisplay;
bool GSWndOGL::Attach(void* handle, bool managed)
{
	m_NativeWindow = *(Window*)handle;
	m_managed = managed;

	m_NativeDisplay = XDisplay;

	FullContextInit();

	return true;
}

void GSWndOGL::Detach()
{
	// Actually the destructor is not called when there is only a GSclose/GSshutdown
	/*// The window still need to be closed
	DetachContext();
	if (m_context) glXDestroyContext(m_NativeDisplay, m_context);

	if (m_NativeDisplay) {
		XCloseDisplay(m_NativeDisplay);
		m_NativeDisplay = NULL;
	}*/
}

bool GSWndOGL::Create(const std::string& title, int w, int h)
{
	if(m_NativeWindow)
		throw GSDXRecoverableError();

	if(w <= 0 || h <= 0) {
		w = theApp.GetConfigI("ModeWidth");
		h = theApp.GetConfigI("ModeHeight");
	}

	m_managed = true;

	// note this part must be only executed when replaying .gs debug file
	m_NativeDisplay = XOpenDisplay(NULL);

	m_NativeWindow = XCreateSimpleWindow(m_NativeDisplay, DefaultRootWindow(m_NativeDisplay), 0, 0, w, h, 0, 0, 0);
	XMapWindow (m_NativeDisplay, m_NativeWindow);

	if (m_NativeWindow == 0)
		throw GSDXRecoverableError();

	FullContextInit();

	return true;
}

void* GSWndOGL::GetProcAddress(const char* name, bool opt)
{
	void* ptr = (void*)glXGetProcAddress((const GLubyte*)name);
	if (ptr == NULL) {
		if (theApp.GetConfigB("debug_opengl"))
			fprintf(stderr, "Failed to find %s\n", name);

		if (!opt)
			throw GSDXRecoverableError();
	}
	return ptr;
}

void* GSWndOGL::GetDisplay()
{
	// note this part must be only executed when replaying .gs debug file
	return (void*)m_NativeDisplay;
}
extern SDL_Window* gWindow;
extern Display* XDisplay;
extern Window Xwindow;
GSVector4i GSWndOGL::GetClientRect()
{
	unsigned int h = 480;
	unsigned int w = 640;

	unsigned int borderDummy;
	unsigned int depthDummy;
	Window winDummy;
    int xDummy;
    int yDummy;
	
	XGetGeometry(m_NativeDisplay, m_NativeWindow, &winDummy, &xDummy, &yDummy, &w, &h, &borderDummy, &depthDummy);

	return GSVector4i(0, 0, (int)w, (int)h);
}

// Returns FALSE if the window has no title, or if th window title is under the strict
// management of the emulator.

bool GSWndOGL::SetWindowText(const char* title)
{
	if (!m_managed) return true;

	XTextProperty prop;

	memset(&prop, 0, sizeof(prop));

	char* ptitle = (char*)title;
	if (XStringListToTextProperty(&ptitle, 1, &prop)) {
		XSetWMName(m_NativeDisplay, m_NativeWindow, &prop);
	}

	XFree(prop.value);
	XFlush(m_NativeDisplay);

	return true;
}

void GSWndOGL::SetSwapInterval()
{
	// m_swapinterval uses an integer as parameter
	// 0 -> disable vsync
	// n -> wait n frame
	if      (m_swapinterval_ext)  m_swapinterval_ext(m_NativeDisplay, m_NativeWindow, m_vsync);
	else if (m_swapinterval_mesa) m_swapinterval_mesa(m_vsync);
	else						 fprintf(stderr, "Failed to set VSync\n");
}

void GSWndOGL::Flip()
{
	if (m_vsync_change_requested.exchange(false))
		SetSwapInterval();

	glXSwapBuffers(m_NativeDisplay, m_NativeWindow);
}

void GSWndOGL::Show()
{
	XMapRaised(m_NativeDisplay, m_NativeWindow);
	XFlush(m_NativeDisplay);
}

void GSWndOGL::Hide()
{
	XUnmapWindow(m_NativeDisplay, m_NativeWindow);
	XFlush(m_NativeDisplay);
}

void GSWndOGL::HideFrame()
{
	// TODO
}

#endif
