#include "stdafx.h"
#include "mu_gles2_matrix.h"

MU_Mat4 g_muProjection;
MU_Mat4 g_muView;

GLuint g_muProgram = 0;
GLint g_uProjection = -1;
GLint g_uView = -1;
GLint g_uTexture = -1;
GLint g_uUseTexture = -1;
GLint g_uDiscardBlack = -1;

void MU_Ortho(MU_Mat4& out, float w, float h)
{
    memset(out.m, 0, sizeof(out.m));

    out.m[0] = 2.0f / w;
    out.m[5] = 2.0f / h; 
    out.m[10] = -1.0f;
    out.m[12] = -1.0f;
    out.m[13] = -1.0f;
    out.m[15] = 1.0f;
}

void MU_LoadIdentity(MU_Mat4& out)
{
    memset(out.m, 0, sizeof(out.m));
    out.m[0] = 1.0f;
    out.m[5] = 1.0f;
    out.m[10] = 1.0f;
    out.m[15] = 1.0f;
}

void MU_Perspective(MU_Mat4& out, float fovyDeg, float aspect, float zNear, float zFar)
{
    float rad = fovyDeg * 3.1415926535f / 180.0f;
    float f = 1.0f / tanf(rad * 0.5f);

    memset(out.m, 0, sizeof(out.m));

    out.m[0] = f / aspect;
    out.m[5] = f;
    out.m[10] = (zFar + zNear) / (zNear - zFar);
    out.m[11] = -1.0f;
    out.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
}

void MU_ApplyMatrices()
{
    glUseProgram(g_muProgram);

    if (g_uProjection >= 0)
        glUniformMatrix4fv(g_uProjection, 1, GL_FALSE, g_muProjection.m);

    if (g_uView >= 0)
        glUniformMatrix4fv(g_uView, 1, GL_FALSE, g_muView.m);
}

void UpdateProjection()
{
    if (WindowHeight <= 0)
        WindowHeight = 1;

    glViewport(0, 0, WindowWidth, WindowHeight);

    MU_Perspective(
        g_muProjection,
        45.0f,
        (float)WindowWidth / (float)WindowHeight,
        0.1f,
        1000.0f
    );
}

GLuint CompileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // check compile error
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        OutputDebugStringA("[SDL-DEBUG] Shader compile error: %s");
    }

    return shader;
}

void InitShader()
{
    const char* vs_src =
        "attribute vec3 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "attribute vec4 aColor;\n"
        "uniform mat4 uProjection;\n"
        "uniform mat4 uView;\n"
        "varying vec2 vTex;\n"
        "varying vec4 vColor;\n"
        "void main(){\n"
        " gl_Position = uProjection * uView * vec4(aPosition, 1.0);\n"
        " vTex = aTexCoord;\n"
        " vColor = aColor;\n"
        "}";

    const char* fs_src =
        "precision mediump float;\n"
        "varying vec2 vTex;\n"
        "varying vec4 vColor;\n"
        "uniform sampler2D uTexture;\n"
        "uniform int uUseTexture;\n"
        "uniform int uDiscardBlack;\n"
        "void main(){\n"
        " vec4 tex = texture2D(uTexture, vTex);\n"
        " if(uDiscardBlack == 1 && tex.r < 0.03 && tex.g < 0.03 && tex.b < 0.03)\n"
        "     discard;\n"
        " vec4 color = vColor;\n"
        " if(uUseTexture == 1)\n"
        "     color *= tex;\n"
        " gl_FragColor = color;\n"
        "}";


    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fs_src);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // bind attribute BEFORE linking
    glBindAttribLocation(program, 0, "aPosition");
    glBindAttribLocation(program, 1, "aTexCoord");
    glBindAttribLocation(program, 2, "aColor");

    glLinkProgram(program);

    // check link error
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
    }

    // shaders no longer needed after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    g_muProgram = program;

    g_uTexture = glGetUniformLocation(g_muProgram, "uTexture");
    g_uUseTexture = glGetUniformLocation(g_muProgram, "uUseTexture");
    g_uDiscardBlack = glGetUniformLocation(g_muProgram, "uDiscardBlack");

    glUseProgram(g_muProgram);

   // GLint uTexture = glGetUniformLocation(g_muProgram, "uTexture");
    if (g_uTexture >= 0)
        glUniform1i(g_uTexture, 0);

    g_uProjection = glGetUniformLocation(g_muProgram, "uProjection");
    g_uView = glGetUniformLocation(g_muProgram, "uView");
}