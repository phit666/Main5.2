#include "stdafx.h"
#include "mu_gles2_matrix.h"

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
GLint g_uTexEnabledLoc = -1, g_uAlphaTestLoc = -1, g_uFogEnabledLoc = -1, g_uFogColorLoc = -1, g_uFogDensityLoc = -1;
GLint g_uColorLoc = -1;
GLint g_uMvLoc = -1;
GLint g_uMvpLoc = -1;

MU_Mat4 g_savedViewForSprite;

float g_CurrentColor[4] = { 1.f, 1.f, 1.f, 1.f };

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
    const char* vertexShaderSource = R"(
    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_color;
    uniform mat4 u_mvpMatrix;    // Combined Projection * ModelView
    uniform mat4 u_mvMatrix;     // ModelView only (for distance calc)

    varying vec2 v_texCoord;
    varying float v_dist;
    varying vec4 v_color;

    void main() {
        gl_Position = u_mvpMatrix * a_position;
        v_texCoord = a_texCoord;
        v_color = a_color;
        // Calculate distance from camera for Fog
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
    uniform bool u_hasTexture;       // New: Handle -1 texID logic
    uniform bool u_alphaTestEnabled;
    uniform bool u_fogEnabled;

    // Fog Params
    uniform vec4 u_fogColor;
    uniform float u_fogDensity;

    void main() {
        vec4 finalColor;

        if (u_hasTexture) {
            // Multiply sampled texture by vertex color AND global uniform
            finalColor = texture2D(u_texture, v_texCoord) * v_color * u_color;
        } else {
            // Just vertex color multiplied by global uniform
            finalColor = v_color * u_color;
        }

        // Alpha Test (Replacement for glAlphaFunc)
        if (u_alphaTestEnabled && finalColor.a <= 0.25) {
            discard;
        }

        // Fog Calculation
        if (u_fogEnabled) {
            // Exponential fog: f = 1 / e^(dist * density)
            float fogFactor = 1.0 / exp(v_dist * u_fogDensity);
            fogFactor = clamp(fogFactor, 0.0, 1.0);
            
            // Mix finalColor with FogColor based on distance
            gl_FragColor = vec4(mix(u_fogColor.rgb, finalColor.rgb, fogFactor), finalColor.a);
        } else {
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
    myShader.ID = program;

    g_uTexEnabledLoc = glGetUniformLocation(g_muProgram, "u_hasTexture");
    g_uAlphaTestLoc = glGetUniformLocation(g_muProgram, "u_alphaTestEnabled");
    g_uFogEnabledLoc = glGetUniformLocation(g_muProgram, "u_fogEnabled");
    g_uFogColorLoc = glGetUniformLocation(g_muProgram, "u_fogColor");
    g_uFogDensityLoc = glGetUniformLocation(g_muProgram, "u_fogDensity");
    g_uColorLoc = glGetUniformLocation(g_muProgram, "u_color");

    g_aPosLoc = glGetAttribLocation(g_muProgram, "a_position");
    g_aTexLoc = glGetAttribLocation(g_muProgram, "a_texCoord");
    g_aColorLoc = glGetAttribLocation(g_muProgram, "a_color");
    g_uMvpLoc = glGetUniformLocation(g_muProgram, "u_mvpMatrix");
    g_uMvLoc = glGetUniformLocation(g_muProgram, "u_mvMatrix");

    glUseProgram(g_muProgram);
    GLint texSamplerLoc = glGetUniformLocation(g_muProgram, "u_texture");
    glUniform1i(texSamplerLoc, 0); // Always use Texture Unit 0

}