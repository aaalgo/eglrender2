#include "eglrender.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdexcept>

#define ES_WINDOW_RGB           0
#define ES_WINDOW_ALPHA         1
#define ES_WINDOW_DEPTH         2
#define ES_WINDOW_STENCIL       4
#define ES_WINDOW_MULTISAMPLE   8

static Display *x_display = NULL;

EglRenderCtx::EglRenderCtx (GLint w, GLint h): width(w), height(h) {
    int flags = 0;
    EGLDisplay  eglDisplay;
    EGLContext  eglContext;
    EGLSurface  eglSurface;
    XSetWindowAttributes swa;
    XSetWindowAttributes xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
 
    if (x_display) throw std::runtime_error("EGL already initialized");
    x_display = XOpenDisplay(NULL);
    if (!x_display) throw std::runtime_error("Failed to open display");
 
    Window root = DefaultRootWindow(x_display);
 
    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    Window win = XCreateWindow(
                x_display, root,
                0, 0, width, height, 0,
                CopyFromParent, InputOutput,
                CopyFromParent, CWEventMask,
                &swa);
 
    xattr.override_redirect = 0;
    XChangeWindowAttributes (x_display, win, CWOverrideRedirect, &xattr );
 
    hints.input = 1;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);
 
    /*
    XMapWindow (x_display, win);
    XStoreName (x_display, win, "");
    */
 
    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", 0);
 
    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = 0;
    XSendEvent (
        x_display,
        DefaultRootWindow ( x_display ),
        0,
        SubstructureNotifyMask,
        &xev );
 
    eglDisplay = eglGetDisplay((EGLNativeDisplayType)x_display);
    if (eglDisplay == EGL_NO_DISPLAY) throw std::runtime_error("Failed to get EGL display.");
 
    EGLConfig config;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
 
    // Initialize EGL
    if (!eglInitialize (eglDisplay, &majorVersion, &minorVersion ) )
    {
        throw std::runtime_error("Failed to initialize EGL.");
    }
 
    {
       EGLint numConfigs = 0;
       EGLint attribList[] =
       {
          EGL_RED_SIZE,       5,
          EGL_GREEN_SIZE,     6,
          EGL_BLUE_SIZE,      5,
          EGL_ALPHA_SIZE,     ( flags & ES_WINDOW_ALPHA ) ? 8 : EGL_DONT_CARE,
          EGL_DEPTH_SIZE,     ( flags & ES_WINDOW_DEPTH ) ? 8 : EGL_DONT_CARE,
          EGL_STENCIL_SIZE,   ( flags & ES_WINDOW_STENCIL ) ? 8 : EGL_DONT_CARE,
          EGL_SAMPLE_BUFFERS, ( flags & ES_WINDOW_MULTISAMPLE ) ? 1 : 0,
          // if EGL_KHR_create_context extension is supported, then we will use
          // EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT in the attribute list
          EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
          EGL_NONE
       };
 
       // Choose config
       if ( !eglChooseConfig (eglDisplay, attribList, &config, 1, &numConfigs ) || (numConfigs < 1))
       {
           throw std::runtime_error("Failed to choose config.");
       }
    }
 
    // Create a surface
    eglSurface = eglCreateWindowSurface (eglDisplay, config,  (EGLNativeWindowType)win, NULL );

    if (eglSurface == EGL_NO_SURFACE) throw std::runtime_error("Failed to create surface.");
 
    // Create a GL context
    eglContext = eglCreateContext (eglDisplay, config,  EGL_NO_CONTEXT, contextAttribs );
 
    if (eglContext == EGL_NO_CONTEXT ) throw std::runtime_error("Failed to create context.");
 
    if ( !eglMakeCurrent ( eglDisplay, eglSurface, 
                           eglSurface, eglContext ))
        throw std::runtime_error("Failed to make context current.");
}

cv::Mat EglRenderCtx::render () {
    cv::Mat image(height, width, CV_8UC4);
    //glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.ptr<uint8_t>(0));
    cv::Mat out;
    cv::cvtColor(image, out, CV_RGBA2BGR);
    return out;
}

