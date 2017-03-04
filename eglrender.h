#ifndef EGLRENDER_HEAD
#define EGLRENDER_HEAD

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <opencv2/opencv.hpp>

class EglRenderCtx {
    GLint width, height;
public:
    EglRenderCtx (GLint w, GLint h);
    cv::Mat render ();
};


#endif
