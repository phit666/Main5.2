#include "stdafx.h"
#include "mu_gles2_matrix.h"
#include "Utilities/Log/ErrorReport.h"

Shader myShader;

// Manual stacks to replace glPushMatrix/glPopMatrix
std::vector<glm::mat4> projectionStack = { glm::mat4(1.0f) };
std::vector<glm::mat4> modelViewStack = { glm::mat4(1.0f) };

MU_Mat4 g_muProjection;
MU_Mat4 g_muView;

GLuint g_muProgram = 0;
GLint g_uProjection = -1;
GLint g_uView = -1;
GLint g_uTexture = -1;
GLint g_uUseTexture = -1;
GLint g_uDiscardBlack = -1;
GLint g_uMinLight = -1;

GLint g_aPosLoc = -1;
GLint g_aTexLoc = -1;
GLint g_aColorLoc = -1;
// Global uniform locations (you should initialize these after linking your shader)
GLint g_uTexEnabledLoc = -1, g_uAlphaTestLoc = -1, g_uFogEnabledLoc = -1, g_uFogColorLoc = -1, g_uFogDensityLoc = -1, g_uAlphaThresholdLoc = -1;
GLint g_uFogStartLoc = -1, g_uFogEndLoc = -1;
GLint g_uColorLoc = -1;
GLint g_uMvLoc = -1;
GLint g_uMvpLoc = -1;

MU_Mat4 g_savedViewForSprite;

float g_CurrentColor[4] = { 1.f, 1.f, 1.f, 1.f };


void MU_glColor4f(float r, float g, float b, float a)
{
    g_CurrentColor[0] = r;
    g_CurrentColor[1] = g;
    g_CurrentColor[2] = b;
    g_CurrentColor[3] = a;

    myShader.setVec4(g_uColorLoc, r, g, b, a);
    //glVertexAttrib4f(g_aColorLoc, r, g, b, a);

}

void MU_glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    g_CurrentColor[0] = r / 255.0f;
    g_CurrentColor[1] = g / 255.0f;
    g_CurrentColor[2] = b / 255.0f;
    g_CurrentColor[3] = a / 255.0f;
    //glDisableVertexAttribArray(g_aColorLoc);
    //glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    myShader.setVec4(g_uColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    //glVertexAttrib4f(g_aColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

}

void MU_glColor3ub(GLubyte r, GLubyte g, GLubyte b)
{
    g_CurrentColor[0] = r / 255.0f;
    g_CurrentColor[1] = g / 255.0f;
    g_CurrentColor[2] = b / 255.0f;
    g_CurrentColor[3] = 1.0f;
    myShader.setVec4(g_uColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
   // glVertexAttrib4f(g_aColorLoc, r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

void MU_glLoadIdentity() {
    if (g_currentMatrixMode == GL_PROJECTION) {
        projectionStack.back() = glm::mat4(1.0f);
    }
    else {
        modelViewStack.back() = glm::mat4(1.0f);
    }
    // Sync the shader
    myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
}

void MU_glPushMatrix() {
    if (g_currentMatrixMode == GL_PROJECTION) {
        projectionStack.push_back(projectionStack.back());
    }
    else {
        modelViewStack.push_back(modelViewStack.back());
    }
}

void MU_glPopMatrix() {
    if (g_currentMatrixMode == GL_PROJECTION) {
        if (projectionStack.size() > 1) projectionStack.pop_back();
    }
    else {
        if (modelViewStack.size() > 1) modelViewStack.pop_back();
    }
    // Sync the shader
    myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
}

void MU_TransformPoint(const MU_Mat4& m, const vec3_t in, vec3_t out)
{
    out[0] = m.m[0] * in[0] + m.m[4] * in[1] + m.m[8] * in[2] + m.m[12];
    out[1] = m.m[1] * in[0] + m.m[5] * in[1] + m.m[9] * in[2] + m.m[13];
    out[2] = m.m[2] * in[0] + m.m[6] * in[1] + m.m[10] * in[2] + m.m[14];
}

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
    if (g_uMvLoc >= 0)
    {
        glUniformMatrix4fv(g_uMvLoc, 1, GL_FALSE, glm::value_ptr(modelViewStack.back()));
    }
    if (g_uMvpLoc >= 0)
    {
        glm::mat4 mvp = projectionStack.back() * modelViewStack.back();
        glUniformMatrix4fv(g_uMvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
    }
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
        //char t[100] = { 0 };
        //sprintf(t, "[SDL-DEBUG] Shader compile error, type %d %s", (int)type, log);
        //OutputDebugStringA(t);
    }

    return shader;
}

void InitShader()
{
    const char* vertexShaderSource = R"(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

uniform mat4 u_mvpMatrix;
uniform mat4 u_mvMatrix;

varying vec2 v_texCoord;
varying float v_dist;
varying vec4 v_color;

void main()
{
    gl_Position = u_mvpMatrix * a_position;

    v_texCoord = a_texCoord;
    v_color = a_color;

    // Calculate distance from camera for fog
    vec4 viewSpacePos = u_mvMatrix * a_position;
    v_dist = length(viewSpacePos.xyz);
}

)";

    const char* fragmentShaderSource = R"(

precision mediump float;

varying vec2 v_texCoord;
varying float v_dist;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform vec4 u_color;

// Toggles
uniform float u_hasTexture;
uniform float u_alphaTestEnabled;
uniform float u_alphaThreshold;
uniform float u_fogEnabled;

// Fog Params
uniform vec4 u_fogColor;
uniform float u_fogDensity;
uniform float u_fogStart;
uniform float u_fogEnd;

void main()
{
    vec4 finalColor;

    if (u_hasTexture > 0.5)
    {
        finalColor =
            texture2D(u_texture, v_texCoord) *
            v_color *
            u_color;
    }
    else
    {
        finalColor = v_color * u_color;
    }

    if (u_alphaTestEnabled > 0.5 &&
        finalColor.a <= u_alphaThreshold)
    {
        discard;
    }

    if (u_fogEnabled > 0.5)
    {
        float fogFactor = 1.0;

        if (u_fogDensity > 0.0)
        {
            fogFactor = 1.0 / exp(v_dist * u_fogDensity);
        }
        else
        {
            fogFactor =
                (u_fogEnd - v_dist) /
                (u_fogEnd - u_fogStart);
        }

        fogFactor = clamp(fogFactor, 0.0, 1.0);

        gl_FragColor = vec4(
            mix(u_fogColor.rgb, finalColor.rgb, fogFactor),
            finalColor.a
        );
    }
    else
    {
        gl_FragColor = finalColor;
    }
}

)";


    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // bind attribute BEFORE linking
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_texCoord");
    glBindAttribLocation(program, 2, "a_color");

    glLinkProgram(program);

    // check link error
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[512] = { 0 };
        glGetProgramInfoLog(program, 512, NULL, log);
        // need to report later
    }

    // shaders no longer needed after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    g_muProgram = program;

    myShader.init(program);

    g_uTexEnabledLoc = glGetUniformLocation(g_muProgram, "u_hasTexture");
    g_uAlphaTestLoc = glGetUniformLocation(g_muProgram, "u_alphaTestEnabled");
    g_uAlphaThresholdLoc = glGetUniformLocation(g_muProgram, "u_alphaThreshold");
    g_uFogEnabledLoc = glGetUniformLocation(g_muProgram, "u_fogEnabled");
    g_uFogColorLoc = glGetUniformLocation(g_muProgram, "u_fogColor");
    g_uFogDensityLoc = glGetUniformLocation(g_muProgram, "u_fogDensity");
    g_uColorLoc = glGetUniformLocation(g_muProgram, "u_color");

    g_uFogStartLoc = glGetUniformLocation(g_muProgram, "u_fogStart");
    g_uFogEndLoc = glGetUniformLocation(g_muProgram, "u_fogEnd");

    g_aPosLoc = glGetAttribLocation(g_muProgram, "a_position");
    g_aTexLoc = glGetAttribLocation(g_muProgram, "a_texCoord");
    g_aColorLoc = glGetAttribLocation(g_muProgram, "a_color");
    g_uMvpLoc = glGetUniformLocation(g_muProgram, "u_mvpMatrix");
    g_uMvLoc = glGetUniformLocation(g_muProgram, "u_mvMatrix");

    glUseProgram(g_muProgram);
    GLint texSamplerLoc = glGetUniformLocation(g_muProgram, "u_texture");
    glUniform1i(texSamplerLoc, 0); // Always use Texture Unit 0

}


Shader::Shader() {
    ID = -1;
    uColor.m[0] = 0.0f;
    uColor.m[1] = 0.0f;
    uColor.m[2] = 0.0f;
    uColor.m[3] = 0.0f;
}

Shader::~Shader() {

}

void Shader::init(GLuint iID) {
    ID = iID;
}

void Shader::use() { glUseProgram(ID); }

void Shader::setBool(GLuint iID, bool value){
    glUniform1i(iID, (int)value);
}
void Shader::setFloat(GLuint iID, float value){
    glUniform1f(iID, value);
}
void Shader::setVec4(GLuint iID, float x, float y, float z, float w){

    //if (g_uColorLoc == iID) {
        //if (uColor.m[0] == x && uColor.m[1] == y && uColor.m[2] == z && uColor.m[3] == w) {
           // return;
        //}
   // }

    glUniform4f(iID, x, y, z, w);

    if (g_uColorLoc == iID) {
        uColor.m[0] = x;
        uColor.m[1] = y;
        uColor.m[2] = z;
        uColor.m[3] = w;
    }
}
void Shader::setMat4(GLuint iID, const glm::mat4& mat){
    glUniformMatrix4fv(iID, 1, GL_FALSE, glm::value_ptr(mat));
}

void testprogram() {
    GLfloat aColor[4] = { 0 };
    GLfloat uColor[4] = { 0 };
    GLint enabled = 0;

    glGetVertexAttribiv(g_aColorLoc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    glGetVertexAttribfv(g_aColorLoc, GL_CURRENT_VERTEX_ATTRIB, aColor);

    glGetUniformfv(g_muProgram, g_uColorLoc, uColor);

    g_ErrorReport.Write("> [debugcolor] aColor enabled: %d\n", enabled);
    g_ErrorReport.Write("> [debugcolor] aColor current: %.3f %.3f %.3f %.3f\n",
        aColor[0], aColor[1], aColor[2], aColor[3]);

    g_ErrorReport.Write("> [debugcolor] uColor: %.3f %.3f %.3f %.3f\n",
        uColor[0], uColor[1], uColor[2], uColor[3]);
}