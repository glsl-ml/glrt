//$ g++ -o gl33_vector_add gl33_vector_add.cpp -lEGL -lGL -ldl
//$ ./dev_shell.sh ./gl33_vector_add

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

// --- Configuration ---
const int WIDTH = 1024;
const int HEIGHT = 1024;
const int NUM_ELEMENTS = WIDTH * HEIGHT; // 1 Million

// --- Definitions ---
#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK
#define EGL_CONTEXT_OPENGL_PROFILE_MASK 0x3098
#endif
#ifndef EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT 0x00000001
#endif
#ifndef EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

// --- Extensions ---
typedef EGLBoolean (*PFNEGLQUERYDEVICESEXTPROC)(EGLint, EGLDeviceEXT*, EGLint*);
typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum, void*, const EGLint*);

// --- Manual Typedefs ---
typedef const GLubyte* (*PFNGLGETSTRINGPROC)(GLenum);

// --- GL Pointers (Extensions Only) ---
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;

// We use STANDARD GL 1.1 functions for Textures/Draw/Viewport
// The linker handles these (-lGL), so we do NOT declare pointers for them.
// This avoids the re-declaration errors.

// Manual pointer for string to avoid conflict
PFNGLGETSTRINGPROC my_glGetString = nullptr;

void loadGLFunctions() {
    // Only load 2.0+ extensions
    glCreateShader = (PFNGLCREATESHADERPROC)eglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)eglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)eglGetProcAddress("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)eglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)eglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)eglGetProcAddress("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)eglGetProcAddress("glUseProgram");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)eglGetProcAddress("glGetShaderiv");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)eglGetProcAddress("glGetProgramiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)eglGetProcAddress("glGetShaderInfoLog");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)eglGetProcAddress("glGetProgramInfoLog");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)eglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)eglGetProcAddress("glUniform1i");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)eglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)eglGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)eglGetProcAddress("glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)eglGetProcAddress("glCheckFramebufferStatus");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)eglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)eglGetProcAddress("glBindVertexArray");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)eglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)eglGetProcAddress("glVertexAttribPointer");
    glGenBuffers = (PFNGLGENBUFFERSPROC)eglGetProcAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)eglGetProcAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)eglGetProcAddress("glBufferData");

    my_glGetString = (PFNGLGETSTRINGPROC)eglGetProcAddress("glGetString");
}

const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec2 position;

void main() {
    // Just draw the quad. We don't calculate texture coords here anymore.
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

const char* fragSrc = R"(
#version 330 core
out vec4 color;

// Note: We still use sampler2D, but we access it differently
uniform sampler2D texA;
uniform sampler2D texB;

void main() {
    // 1. Get ID: Equivalent to gl_GlobalInvocationID.xy
    // gl_FragCoord gives pixel centers (0.5, 1.5, etc.)
    // Casting to int gives us clean indices (0, 1, 2...)
    ivec2 index = ivec2(gl_FragCoord.xy);
    
    // 2. Fetch Data: Equivalent to A[index]
    // texelFetch takes an integer coordinate (no 0.0-1.0 mapping!)
    // The '0' at the end is the mipmap level (detail level).
    float valA = texelFetch(texA, index, 0).r;
    float valB = texelFetch(texB, index, 0).r;
    
    // 3. Output Result
    color = vec4(valA + valB, 0.0, 0.0, 1.0);
}
)";

int main(int argc, const char *argv[]) {
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (!eglQueryDevicesEXT) { std::cerr << "EGL Missing" << std::endl; return 1; }

    EGLint num_devices;
    eglQueryDevicesEXT(0, nullptr, &num_devices);
    std::vector<EGLDeviceEXT> devices(num_devices);
    eglQueryDevicesEXT(num_devices, devices.data(), &num_devices);

    std::cout << "Found " << num_devices << " gpus on the machine\n";

    int device = 0;
    if(argc == 2) {
        device = atoi(argv[1]);
    }
    assert(device < num_devices);
    
    EGLDisplay display = eglGetPlatformDisplayEXT(0x313F, devices[device], nullptr);
    eglInitialize(display, nullptr, nullptr);
    eglBindAPI(EGL_OPENGL_API);

    // --- CRITICAL FIX: EXACT CONFIG FROM WORKING TEST ---
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, // <--- THIS WAS MISSING
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    // ----------------------------------------------------

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        std::cerr << "Failed to find EGL Config." << std::endl;
        return 1;
    }

    EGLContext context = EGL_NO_CONTEXT;

    // Attempt Core 3.3
    std::cout << "Trying GL 3.3 Core..." << std::endl;
    EGLint coreAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, coreAttribs);

    if (context == EGL_NO_CONTEXT) {
        std::cout << "Core failed. Trying Compatibility..." << std::endl;
        EGLint compatAttribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, compatAttribs);
    }

    if (context == EGL_NO_CONTEXT) {
        std::cerr << "FAILURE: Could not create ANY OpenGL 3.3 Context." << std::endl;
        return 1;
    }

    EGLint pbufferAttribs[] = { EGL_WIDTH, 4, EGL_HEIGHT, 4, EGL_NONE };
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    eglMakeCurrent(display, surface, surface, context);
    loadGLFunctions();

    std::cout << "SUCCESS: Context Created!" << std::endl;
    if (my_glGetString) {
        std::cout << "Renderer: " << (const char*)my_glGetString(GL_RENDERER) << std::endl;
    }

    // --- DATA ---
    std::cout << "Generating 1 Million Floats..." << std::endl;
    std::vector<float> dataA(NUM_ELEMENTS);
    std::vector<float> dataB(NUM_ELEMENTS);
    for(int i=0; i<NUM_ELEMENTS; i++) {
        dataA[i] = (float)i;
        dataB[i] = (float)i * 2.0f;
    }

    // --- TEXTURES (Using Standard GL) ---
    GLuint texA, texB, texOut;

    // A
    glGenTextures(1, &texA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT, dataA.data());

    // B
    glGenTextures(1, &texB);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT, dataB.data());

    // Out
    glGenTextures(1, &texOut);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texOut);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);

    // --- FBO ---
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texOut, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO Error: " << std::hex << status << std::endl;
        return 1;
    }

    // --- SHADER ---
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertSrc, nullptr);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragSrc, nullptr);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glUseProgram(prog);

    glUniform1i(glGetUniformLocation(prog, "texA"), 0);
    glUniform1i(glGetUniformLocation(prog, "texB"), 1);

    glViewport(0, 0, WIDTH, HEIGHT);

    // --- DRAW (VBO Required) ---
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float quadVerts[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    std::cout << "Dispatching Render..." << std::endl;
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFinish();

    // --- READBACK ---
    std::cout << "Reading results..." << std::endl;
    std::vector<float> results(NUM_ELEMENTS);
    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, results.data());

    // --- VERIFY ---
    int errors = 0;
    for(int i=0; i<NUM_ELEMENTS; i+=1024) {
        float expected = (float)i * 3.0f;
        if (std::abs(results[i] - expected) > 0.1f) {
            errors++;
            if (errors < 5) std::cout << "Err idx " << i << ": " << results[i] << " != " << expected << std::endl;
        }
    }

    if (errors == 0) {
        std::cout << "SUCCESS: 1,000,000 Elements Processed." << std::endl;
        std::cout << "Sample [Last]: " << results[NUM_ELEMENTS-1] << std::endl;
    } else {
        std::cout << "FAILURE: " << errors << " errors." << std::endl;
    }

    return 0;
}
