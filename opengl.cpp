#pragma runtime_checks("", off)
#pragma comment(linker, "/nodefaultlib /subsystem:windows")

#include <cstdlib>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <GL/glew.h>

#define kWidth 1280
#define kHeight 720


extern "C" int _fltused = 0;

namespace {
    struct {
        GLuint(APIENTRY *createProgram)();
        GLuint(APIENTRY *createShader)(GLenum);
        void (APIENTRY *shaderSource)(GLuint, GLsizei, const GLchar **, const GLint *);
        void (APIENTRY *compileShader)(GLuint);
        void (APIENTRY *attachShader)(GLuint, GLuint);
        void (APIENTRY *linkProgram)(GLuint);
        void (APIENTRY *useProgram)(GLuint);
        void (APIENTRY *uniform1f)(GLint, GLfloat);
        GLint(APIENTRY *uniformLocation)(GLuint, const GLchar *);
        void (APIENTRY *genBuffers)(GLsizei, GLuint *);
        void (APIENTRY *bindBuffer)(GLenum, GLuint);
        void (APIENTRY *bindBufferBase)(GLenum, GLuint, GLuint);
        void (APIENTRY *bufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum);
        void (APIENTRY *getBufferSubData)(GLenum, GLintptr, GLsizeiptr, GLvoid *);
        void (APIENTRY *transformFeedbackVaryings)(GLuint, GLsizei, const GLchar **, GLenum);
        void (APIENTRY *beginTransformFeedback)(GLenum);
        void (APIENTRY *endTransformFeedback)();
        void (APIENTRY *enableVertexAttribArray)(GLuint);
        void (APIENTRY *vertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
    } gl;

    const char *const procNames =
        "glCreateProgram\0"
        "glCreateShader\0"
        "glShaderSource\0"
        "glCompileShader\0"
        "glAttachShader\0"
        "glLinkProgram\0"
        "glUseProgram\0"
        "glUniform1f\0"
        "glGetUniformLocation\0"
        "glGenBuffers\0"
        "glBindBuffer\0"
        "glBindBufferBase\0"
        "glBufferData\0"
        "glGetBufferSubData\0"
        "glTransformFeedbackVaryings\0"
        "glBeginTransformFeedback\0"
        "glEndTransformFeedback\0"
        "glEnableVertexAttribArray\0"
        "glVertexAttribPointer\0";

    void WINAPI compile(GLenum type, const char *source, GLuint program) {
        const GLuint shader = gl.createShader(type);
        gl.shaderSource(shader, 1, &source, 0);
        gl.compileShader(shader);
        gl.attachShader(program, shader);
    }

    const char vertex_shader[] = "uniform float time; /* write your code here */";
    const char pixel_shader[] = "uniform float time; /* write your code here */";
    const char sound_shader[] = "/* write your code here */";
}

void WINAPI WinMainCRTStartup()
{
#ifdef FINAL
    // enter to full screen & hide cursor
    static DEVMODEA dm = {
        { 0 }, 0, 0, sizeof(DEVMODEA), 0, DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT,
        { 0 }, 0, 0, 0, 0, 0,{ 0 }, 0, 32, kWidth, kHeight
    };
    ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
    ShowCursor(FALSE);
#else
#endif

    const HDC hdc = GetDC(CreateWindowExA(0, "static", 0, WS_POPUP | WS_VISIBLE, 0, 0, kWidth, kHeight, 0, 0, 0, 0));
    static const PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32
    };
    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
    wglMakeCurrent(hdc, wglCreateContext(hdc));

    // get OpenGL functions
    const char *name = procNames;
    for (PROC *proc = reinterpret_cast<PROC *>(&gl), *const end = &proc[sizeof(gl) / sizeof(*proc)]; proc < end; proc++) {
        *proc = wglGetProcAddress(name);
        while (*name++) {}
    }

    // create shader
    static GLuint graphics_program;
    graphics_program = gl.createProgram();
    compile(GL_FRAGMENT_SHADER, pixel_shader, graphics_program);
    compile(GL_VERTEX_SHADER, vertex_shader, graphics_program);
    gl.linkProgram(graphics_program);

    // create sound shader
    static GLuint sound_program;
    static const char *varying = "time";
    compile(GL_VERTEX_SHADER, sound_shader, sound_program = gl.createProgram());
    gl.transformFeedbackVaryings(sound_program, 1, &varying, GL_INTERLEAVED_ATTRIBS);
    gl.linkProgram(sound_program);

    // make sound data
    static char feedback[0x03000000];
    GLuint buffers[2];
    gl.genBuffers(2, buffers);
    gl.bindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(feedback), 0, GL_DYNAMIC_COPY);
    gl.bindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(feedback), 0, GL_STATIC_DRAW);
    gl.enableVertexAttribArray(0);
    gl.vertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    gl.useProgram(sound_program);
    gl.bindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[0]);
    glEnable(GL_RASTERIZER_DISCARD);
    gl.beginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 0x00600000);
    gl.endTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
    gl.getBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

    // begin output sound
    static const WAVEFORMATEX wfx = { WAVE_FORMAT_IEEE_FLOAT, 2, 48000, 8 * 48000, 8, 32, 0 };
    static WAVEHDR wh = { feedback, sizeof(feedback) };
    HWAVEOUT hwo;
    waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, 0);
    waveOutPrepareHeader(hwo, &wh, sizeof(wh));
    waveOutWrite(hwo, &wh, sizeof(wh));

    static MMTIME mmt = { TIME_SAMPLES };
    gl.useProgram(graphics_program);
    do {
        MSG msg;
        PeekMessage(&msg, 0, 0, 0, TRUE);
        waveOutGetPosition(hwo, &mmt, sizeof(mmt));

        gl.uniform1f(gl.uniformLocation(graphics_program, varying), GLfloat(mmt.u.sample) * GLfloat(1.0 / 48000.0));
        glRects(-1, -1, 1, 1);
        wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
    } while (!GetAsyncKeyState(VK_ESCAPE) && (mmt.u.sample < 0x00600000));

    ExitProcess(0);
}
