///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "zzzObject.h"
#include "zzzcharacter.h"
#include "Zzzinfomation.h"
#include "NewUISystem.h"
#include "mu_gles2_matrix.h"

int     OpenglWindowX;
int     OpenglWindowY;
int     OpenglWindowWidth;
int     OpenglWindowHeight;
bool    CameraTopViewEnable = false;
float   CameraViewNear = 20.f;
float   CameraViewFar = 2000.f;
float   CameraFOV = 55.f;
vec3_t  CameraPosition;
vec3_t  CameraAngle;
float   CameraMatrix[3][4];
vec3_t  MousePosition;
vec3_t  MouseTarget;
float   g_fCameraCustomDistance = 0.f;
bool    FogEnable = false;
GLfloat FogDensity = 0.0004f;
GLfloat FogColor[4] = { 30 / 256.f,20 / 256.f,10 / 256.f, };

unsigned int WindowWidth = 1024;
unsigned int WindowHeight = 768;
int          MouseX = WindowWidth / 2;
int          MouseY = WindowHeight / 2;
int          BackMouseX = MouseX;
int          BackMouseY = MouseY;
bool         MouseLButton;
bool 		 MouseLButtonPop;
bool 		 MouseLButtonPush;
bool         MouseRButton;
bool 		 MouseRButtonPop;
bool 		 MouseRButtonPush;
bool 	   	 MouseLButtonDBClick;
bool         MouseMButton;
bool         MouseMButtonPop;
bool         MouseMButtonPush;
int          MouseWheel;
DWORD		 MouseRButtonPress = 0;

// Simple state tracker
static GLboolean attr_enabled[3] = { GL_FALSE, GL_FALSE, GL_FALSE };

void OpenExploper(char* Name, char* para)
{
#ifdef _WIN32
	ShellExecute(NULL, "open", Name, para, "", SW_SHOW);
#endif
}

bool CheckID_HistoryDay(char* Name, WORD day)
{
	typedef struct  __day_history__
	{
		char ID[MAX_ID_SIZE + 1];
		WORD date;
	}dayHistory;

	FILE* fp;
	dayHistory days[100];
	int   count = 0;
	WORD  num = 0;
	bool  sameName = false;
	bool  update = true;

	if ((fp = fopen("dconfig.ini", "rb")) != NULL)
	{
		fread(&num, sizeof(WORD), 1, fp);

		if (num > 100)
		{
			num = 0;
		}
		else
		{
			for (int i = 0; i < num; ++i)
			{
				fread(days[i].ID, sizeof(char), MAX_ID_SIZE + 1, fp);
				fread(&days[i].date, sizeof(WORD), 1, fp);

				if (!strcmp(days[i].ID, Name))
				{
					sameName = true;
					if (days[i].date == day)
					{
						update = false;
						break;
					}
					days[i].date = day;
				}
				count++;
			}
		}
		fclose(fp);
	}

	if (update)
	{
		if (!sameName)
		{
			memcpy(days[num].ID, Name, (MAX_ID_SIZE + 1) * sizeof(char));
			days[num].date = day;

			num++;
		}

		fp = fopen("dconfig.ini", "wb");

		fwrite(&num, sizeof(WORD), 1, fp);
		for (int i = 0; i < num; ++i)
		{
			fwrite(days[i].ID, sizeof(char), MAX_ID_SIZE + 1, fp);
			fwrite(&days[i].date, sizeof(WORD), 1, fp);
		}

		fclose(fp);
	}

	//    showShoppingMall = update;

	return  update;
}

bool GrabEnable = false;
char GrabFileName[MAX_PATH];
int  GrabScreen = 0;
bool GrabFirst = false;

void SaveScreen()
{
	GrabFirst = true;

	/*if(!GrabFirst)
	{
		GrabFirst = true;
		for(int i=0;i<10000;i++)
		{
			GrabScreen = i;
			if(GrabScreen<10)
				sprintf(GrabFileName,"Screen000%d",GrabScreen);
			else if(GrabScreen<100)
				sprintf(GrabFileName,"Screen00%d",GrabScreen);
			else if(GrabScreen<1000)
				sprintf(GrabFileName,"Screen0%d",GrabScreen);
			else
				sprintf(GrabFileName,"Screen%d",GrabScreen);

			strcat( GrabFileName, lpszFileName);
			FILE *fp = fopen(GrabFileName,"rb");
			if(fp==NULL)
				break;
			else
				fclose(fp);
		}
	}
	else
	{
		if(GrabScreen<10)
			sprintf(GrabFileName,"Screen000%d",GrabScreen);
		else if(GrabScreen<100)
			sprintf(GrabFileName,"Screen00%d",GrabScreen);
		else if(GrabScreen<1000)
			sprintf(GrabFileName,"Screen0%d",GrabScreen);
		else
			sprintf(GrabFileName,"Screen%d",GrabScreen);

		strcat( GrabFileName, lpszFileName);
	}*/

	unsigned char* Buffer = new unsigned char[(int)WindowWidth * (int)WindowHeight * 3];
	glReadPixels(0, 0, (int)WindowWidth, (int)WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, Buffer);
	WriteJpeg(GrabFileName, (int)WindowWidth, (int)WindowHeight, Buffer, 100);

	SAFE_DELETE_ARRAY(Buffer);

	GrabScreen++;
	GrabScreen %= 10000;
}

float PerspectiveX;
float PerspectiveY;
int   ScreenCenterX;
int   ScreenCenterY;
int   ScreenCenterYFlip;

void GetOpenGLMatrix(float Matrix[3][4])
{
	const glm::mat4& m = modelViewStack.back();

	for (int i = 0; i < 3; i++) // Rows
	{
		for (int j = 0; j < 4; j++) // Columns
		{
			Matrix[i][j] = m[j][i];
		}
	}
}


void safe_disable_attr(GLuint index, bool set, float x, float y, float z, float w) {
	if (attr_enabled[index] || (index == g_aColorLoc && set)) {
		glDisableVertexAttribArray(index);
		attr_enabled[index] = GL_FALSE;
	}
	if (g_aColorLoc == index && set) {
		glVertexAttrib4f(g_aColorLoc, x, y, z, w);
	}
}

void safe_enable_attr(GLuint index) {
	if (!attr_enabled[index]) {
		glEnableVertexAttribArray(index);
		attr_enabled[index] = GL_TRUE;
	}
	if (g_aTexLoc == index) {
		myShader.use();
		myShader.setFloat(g_uTexEnabledLoc, 1.0f);
	}
}

void gluPerspective2(float Fov, float Aspect, float ZNear, float ZFar)
{
	// 1. Update the manual Projection Stack
	// glm::perspective uses radians, so we convert Fov
	projectionStack.back() = glm::perspective(glm::radians(Fov), Aspect, ZNear, ZFar);

	// 2. Sync the Shader (Ensures the world looks right)
	MU_ApplyMatrices();

	// 3. Keep your Legacy Mouse-to-World Math
	// These variables are critical for your Click-to-Move and Object Picking
	ScreenCenterX = OpenglWindowX + OpenglWindowWidth / 2;
	ScreenCenterY = OpenglWindowY + OpenglWindowHeight / 2;
	ScreenCenterYFlip = WindowHeight - ScreenCenterY; // Fixed: WindowHeight is standard for flipping Y

	float AspectY = (float)(WindowHeight) / (float)(OpenglWindowHeight);

	// This calculates how much the 3D ray expands per pixel away from center
	float tanFov = tanf(Fov * 0.5f * 3.141592f / 180.f);
	PerspectiveX = tanFov / (float)(OpenglWindowWidth / 2) * Aspect;
	PerspectiveY = tanFov / (float)(OpenglWindowHeight / 2) * AspectY;
}


void CreateScreenVector(int sx, int sy, vec3_t Target, bool bFixView)
{
	sx = sx * WindowWidth / 640;
	sy = sy * WindowHeight / 480;
	vec3_t p1, p2;
	if (bFixView)
	{
		p1[0] = (float)(sx - ScreenCenterX) * CameraViewFar * PerspectiveX;
		p1[1] = -(float)(sy - ScreenCenterY) * CameraViewFar * PerspectiveY;
		p1[2] = -CameraViewFar;
	}
	else
	{
		p1[0] = (float)(sx - ScreenCenterX) * RENDER_ITEMVIEW_FAR * PerspectiveX;
		p1[1] = -(float)(sy - ScreenCenterY) * RENDER_ITEMVIEW_FAR * PerspectiveY;
		p1[2] = -RENDER_ITEMVIEW_FAR;
	}

	p2[0] = -CameraMatrix[0][3];
	p2[1] = -CameraMatrix[1][3];
	p2[2] = -CameraMatrix[2][3];
	VectorIRotate(p2, CameraMatrix, MousePosition);
	VectorIRotate(p1, CameraMatrix, p2);
	VectorAdd(MousePosition, p2, Target);
}

void Projection(vec3_t Position, int* sx, int* sy)
{
	vec3_t TrasformPosition;
	VectorTransform(Position, CameraMatrix, TrasformPosition);
	*sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
	*sy = (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
	*sx = *sx * 640 / (int)WindowWidth;
	*sy = *sy * 480 / (int)WindowHeight;
}

void TransformPosition(vec3_t Position, vec3_t WorldPosition, int* x, int* y)
{
	vec3_t Temp;
	VectorSubtract(Position, CameraPosition, Temp);
	VectorRotate(Temp, CameraMatrix, WorldPosition);

	*x = (int)(WorldPosition[0] / PerspectiveX / -WorldPosition[2]) + (ScreenCenterX);
	*y = (int)(WorldPosition[1] / PerspectiveY / -WorldPosition[2]) + (ScreenCenterYFlip);
	//*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (WindowHeight/2);
}

bool TestDepthBuffer(vec3_t Position)
{
	vec3_t WorldPosition;
	int x, y;
	TransformPosition(Position, WorldPosition, &x, &y);
	if (x < OpenglWindowX ||
		y < OpenglWindowY ||
		x >= (int)OpenglWindowX + OpenglWindowWidth ||
		y >= (int)OpenglWindowY + OpenglWindowHeight) return false;

	GLfloat key[3];
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, key);

	float z = 1.f - CameraViewNear / -WorldPosition[2] + CameraViewNear / CameraViewFar;
	if (key[0] >= z) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// opengl render util
///////////////////////////////////////////////////////////////////////////////

int  CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int  AlphaBlendType;
bool TextureStream = false;

extern  int test;

void BindTexture(int tex) {
	if (CachTexture != tex) {
		CachTexture = tex;
		GLuint texID = (tex >= 0) ? Bitmaps[tex].TextureNumber : (GLuint)(-1 * tex);
		glBindTexture(GL_TEXTURE_2D, texID);
		//char t[100] = { 0 };
		//sprintf(t, "[SDL-DEBUG] BindTexture %u", tex);
		//OutputDebugString(t);
	}
	//myShader.setFloat(g_uTexEnabledLoc, 1.0f);
}

// GLES2 does not support glBegin/glEnd. TextureStream must be handled 
// by filling a buffer and calling glDrawArrays at the end.
void BindTextureStream(int tex) {
	if (CachTexture != tex) {
		CachTexture = tex;
		// You must manually flush your vertex buffer here
		// FlushBuffer(); 
		glBindTexture(GL_TEXTURE_2D, Bitmaps[tex].TextureNumber);
		myShader.setFloat(g_uTexEnabledLoc, 1.0f);

	}
}

void EnableDepthTest() {
	if (!DepthTestEnable) {
		DepthTestEnable = true;
		glEnable(GL_DEPTH_TEST);
	}
}

void DisableDepthTest() {
	if (DepthTestEnable) {
		DepthTestEnable = false;
		glDisable(GL_DEPTH_TEST);
	}
}

void EnableDepthMask() {
	if (!DepthMaskEnable) {
		DepthMaskEnable = true;
		glDepthMask(GL_TRUE);
	}
}

void DisableDepthMask() {
	if (DepthMaskEnable) {
		DepthMaskEnable = false;
		glDepthMask(GL_FALSE);
	}
}

void EnableCullFace()
{
	if (!CullFaceEnable)
	{
		CullFaceEnable = true;
		glEnable(GL_CULL_FACE);
	}
}

void DisableCullFace()
{
	if (CullFaceEnable)
	{
		CullFaceEnable = false;
		glDisable(GL_CULL_FACE);
	}
}

void DisableTexture(bool AlphaTest)
{
	// 1. Depth Mask is still a standard hardware call
	EnableDepthMask();

	// 2. Alpha Test Logic (Shader Uniform)
	// We update the local boolean and the shader uniform
	AlphaTestEnable = AlphaTest;
	myShader.setFloat(g_uAlphaTestLoc, AlphaTest ? 1.0f : 0.0f);

	// 3. Texture Logic (Shader Uniform)
	// We tell the shader to stop sampling by setting u_hasTexture to false
	if (TextureEnable)
	{
		TextureEnable = false;
		myShader.setFloat(g_uTexEnabledLoc, 1.0);

		// Note: You don't technically need to call glBindTexture(GL_TEXTURE_2D, 0),
		// because the shader if(u_hasTexture) check will bypass the sampler anyway.
	}
}

void SetShaderTexture(bool enable) {
	TextureEnable = enable;
	myShader.setFloat(g_uTexEnabledLoc, enable ? 1.0f : 0.0f);
}

void SetShaderAlphaTest(bool enable) {
	AlphaTestEnable = enable;
	myShader.setFloat(g_uAlphaTestLoc, enable ? 1.0f : 0.0f);
}

void SetShaderFog(bool enable) {
	myShader.setFloat(g_uFogEnabledLoc, enable ? 1.0f : 0.0f);
}

void DisableAlphaBlend() {

	if (AlphaBlendType != 0) {
		AlphaBlendType = 0;
		glDisable(GL_BLEND);
	}
	EnableCullFace();
	EnableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}

	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}

	if(FogEnable)
		SetShaderFog(true);
}

void EnableAlphaTest(bool DepthMask) {

	if (AlphaBlendType != 2)
	{
		AlphaBlendType = 2;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	DisableCullFace();

	if (DepthMask)
		EnableDepthMask();

	if (!AlphaTestEnable)
	{
		AlphaTestEnable = true;
		SetShaderAlphaTest(true);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(true);
}

void EnableAlphaBlend() { // Additive
	if (AlphaBlendType != 3) {
		AlphaBlendType = 3;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}

	DisableCullFace();
	DisableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(false);
}

void EnableAlphaBlendMinus() // Type 4
{
	if (AlphaBlendType != 4)
	{
		AlphaBlendType = 4;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	}
	DisableCullFace();
	DisableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(false);
}

void EnableAlphaBlend2() // Type 5
{
	if (AlphaBlendType != 5)
	{
		AlphaBlendType = 5;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE);
	}
	DisableCullFace();
	DisableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(true);
}

void EnableAlphaBlend3() // Type 6 (Standard Transparency)
{
	if (AlphaBlendType != 6)
	{
		AlphaBlendType = 6;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	DisableCullFace();
	DisableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(true);

}

void EnableAlphaBlend4() // Type 7
{
	if (AlphaBlendType != 7)
	{
		AlphaBlendType = 7;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}

	DisableCullFace();
	DisableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(true);
}


void EnableLightMap() { // Multiplicative
	if (AlphaBlendType != 1) {
		AlphaBlendType = 1;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	}

	EnableCullFace();
	EnableDepthMask();

	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		SetShaderAlphaTest(false);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		SetShaderTexture(true);
	}
	if (FogEnable)
		SetShaderFog(true);

}

void glViewport2(int x, int y, int Width, int Height)
{
	OpenglWindowX = x;
	OpenglWindowY = y;
	OpenglWindowWidth = Width;
	OpenglWindowHeight = Height;
	glViewport(x, WindowHeight - (y + Height), Width, Height);
}

float ConvertX(float x)
{
	return x * (float)WindowWidth / 640.f;
}

float ConvertY(float y)
{
	return y * (float)WindowHeight / 480.f;
}


void BeginOpengl(int x, int y, int Width, int Height)
{
	// 1. Resolution Scaling
	x = x * WindowWidth / 640;
	y = y * WindowHeight / 480;
	Width = Width * WindowWidth / 640;
	Height = Height * WindowHeight / 480;

	// --- PROJECTION ---
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport2(x, y, Width, Height);

	float aspect = (float)Width / (float)Height;
	projectionStack.back() = glm::perspective(glm::radians(CameraFOV), aspect, CameraViewNear, CameraViewFar * 1.4f);
	gluPerspective2(CameraFOV, (float)Width / (float)Height, CameraViewNear, CameraViewFar * 1.4f);

	// --- MODELVIEW ---
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Camera Transformations
	glRotatef(CameraAngle[1], 0.f, 1.f, 0.f);
	if (CameraTopViewEnable == false)
		glRotatef(CameraAngle[0], 1.f, 0.f, 0.f);
	glRotatef(CameraAngle[2], 0.f, 0.f, 1.f);
	glTranslatef(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);

	// --- UPDATE GLOBAL VARIABLES (Your important state trackers) ---
	AlphaTestEnable = false;
	TextureEnable = true;
	DepthTestEnable = true;
	CullFaceEnable = true;
	DepthMaskEnable = true;

	// --- SYNC SHADER TO GLOBALS ---
	myShader.use();
	myShader.setFloat(g_uAlphaTestLoc, AlphaTestEnable ? 1.0f : 0.0f);
	myShader.setFloat(g_uTexEnabledLoc, TextureEnable ? 1.0f : 0.0f);
	//myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Reset global tint

	// --- HARDWARE STATES ---
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glAlphaFunc(0, 0.25f);  // Wrapper for threshold

	// --- FOG ---
	if (FogEnable)
	{
		myShader.setFloat(g_uFogEnabledLoc, 1.0f);
		myShader.setVec4(g_uFogColorLoc, FogColor[0], FogColor[1], FogColor[2], FogColor[3]);
		myShader.setFloat(g_uFogDensityLoc, 0.0f); // 0.0 triggers Linear Math in our shader
		myShader.setFloat(g_uFogStartLoc, 2000.f);
		myShader.setFloat(g_uFogEndLoc, 2700.f);
	}
	else
	{
		//myShader.setFloat(g_uFogEnabledLoc, 0.0f);
	}

	GetOpenGLMatrix(CameraMatrix); // Fills CameraMatrix for legacy CPU math

	// --- FINAL SYNC ---
	//MU_ApplyMatrices();            // Sends MVP and MV to GPU
}

void EndOpengl()
{
	// Force-disable the common attributes used by UI/Sprites
	safe_disable_attr(g_aPosLoc);
	safe_disable_attr(g_aTexLoc);
	safe_disable_attr(g_aColorLoc);

	// Standard state resets
	//glDisable(GL_SCISSOR_TEST); // Crucial: Nuklear leaves this on!
	//glDisable(GL_BLEND);        // Most 3D shouldn't have UI-blending active
	//glEnable(GL_DEPTH_TEST);    // UI usually turns this off
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void UpdateMousePositionn()
{
	// 1. Calculate the View Matrix (Manual glLoadIdentity + glTranslatef)
	// We create a fresh matrix and translate it by the negative camera position.
	glm::mat4 cameraMat = glm::translate(glm::mat4(1.0f),
		glm::vec3(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]));

	// 2. Extract Translation (Equivalent to -CameraMatrix[i][3])
	// In a column-major glm::mat4, .[3] is the translation column.
	// We take the negative of x, y, z as per your original logic.
	glm::vec3 vPos(-cameraMat[3][0], -cameraMat[3][1], -cameraMat[3][2]);

	// 3. Inverse Rotation (Equivalent to VectorIRotate)
	// VectorIRotate usually means multiplying the vector by the transposed 
	// (inverse) 3x3 rotation part of the matrix.
	glm::mat3 rotationPart = glm::mat3(cameraMat); // Extracts the top-left 3x3
	glm::mat3 inverseRotation = glm::transpose(rotationPart);

	glm::vec3 finalMousePos = inverseRotation * vPos;

	// 4. Store result back into your array
	MousePosition[0] = finalMousePos.x;
	MousePosition[1] = finalMousePos.y;
	MousePosition[2] = finalMousePos.z;
}

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL IsGLExtensionSupported(const char* extension)
{
	const size_t extlen = strlen(extension);
	const char* supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char* (__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return FALSE;

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return FALSE;															// No Match

		if ((p == supported || p[-1] == ' ') && (p[extlen] == '\0' || p[extlen] == ' '))
			return TRUE;															// Match
	}
}

BOOL InitGLMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue, int& OutiPixelFormat)
{
	BOOL bIsGLMultisampleSupported = FALSE;

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// See If The String Exists In WGL!
	if (!IsGLExtensionSupported("WGL_ARB_multisample"))
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Get Our Current Device Context
	HDC hDC = GetDC(hWnd);

	int		valid;
	UINT	numFormats;
	float	fAttributes[] = { 0,0 };

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
			WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,24,
			WGL_ALPHA_BITS_ARB,8,
			WGL_DEPTH_BITS_ARB,16,
			WGL_STENCIL_BITS_ARB,0,
			WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
			WGL_SAMPLES_ARB, iRequestMSAAValue,					// xN MultiSampling (N=4,2,1)
			0,0
	};

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Return The Valid Format
	return  bIsGLMultisampleSupported;
}

void SetEnableMultisample()
{
	if (TRUE == g_bSupportedMSAA)
	{
		glEnable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

void SetDisableMultisample()
{
	if (TRUE == g_bSupportedMSAA)
	{
		glDisable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

#endif // LDS_ADD_MULTISAMPLEANTIALIASING

///////////////////////////////////////////////////////////////////////////////
// render util
///////////////////////////////////////////////////////////////////////////////

inline void TEXCOORD(float* c, float u, float v)
{
	c[0] = u;
	c[1] = v;
}

void RenderPlane3D(float Width, float Height, float Matrix[3][4])
{
	vec3_t BoundingVertices[4];
	Vector(-Width, -Width, Height, BoundingVertices[3]);
	Vector(Width, Width, Height, BoundingVertices[2]);
	Vector(Width, Width, -Height, BoundingVertices[1]);
	Vector(-Width, -Width, -Height, BoundingVertices[0]);

	vec3_t TransformVertices[4];
	for (int j = 0; j < 4; j++)
	{
		VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
	}

	// 1. Pack the TransformVertices and UVs into your struct
	SpriteVertex3D vao[4];

	// Vertex 0
	vao[0].x = TransformVertices[0][0]; vao[0].y = TransformVertices[0][1]; vao[0].z = TransformVertices[0][2];
	vao[0].u = 0.0f; vao[0].v = 1.0f;

	// Vertex 1
	vao[1].x = TransformVertices[1][0]; vao[1].y = TransformVertices[1][1]; vao[1].z = TransformVertices[1][2];
	vao[1].u = 1.0f; vao[1].v = 1.0f;

	// Vertex 2
	vao[2].x = TransformVertices[2][0]; vao[2].y = TransformVertices[2][1]; vao[2].z = TransformVertices[2][2];
	vao[2].u = 1.0f; vao[2].v = 0.0f;

	// Vertex 3
	vao[3].x = TransformVertices[3][0]; vao[3].y = TransformVertices[3][1]; vao[3].z = TransformVertices[3][2];
	vao[3].u = 0.0f; vao[3].v = 0.0f;

	// 2. Set Attributes
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex3D), &vao[0].x);

	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex3D), &vao[0].u);

	// Disable color attribute if not used, or use glVertexAttrib4f for a constant color
	safe_disable_attr(g_aColorLoc);
	//glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	MU_ApplyMatrices();

	// 3. Draw
	// GL_TRIANGLE_FAN draws 0-1-2 and then 0-2-3, forming the quad.
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void BeginSprite()
{
	myShader.use();
	glPushMatrix();
	glLoadIdentity();
}

void EndSprite()
{
	glPopMatrix();
}

//
void RenderSprite(int Texture, vec3_t Position, float Width, float Height, vec3_t Light, float Rotation, float u, float v, float uWidth, float vHeight)
{
	BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position, CameraMatrix, p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	Width *= 0.5f;
	Height *= 0.5f;

	vec3_t p[4];
	if (Rotation == 0)
	{
		Vector(x - Width, y - Height, z, p[0]);
		Vector(x + Width, y - Height, z, p[1]);
		Vector(x + Width, y + Height, z, p[2]);
		Vector(x - Width, y + Height, z, p[3]);
	}
	else
	{
		vec3_t p2[4];
		Vector(-Width, -Height, z, p2[0]);
		Vector(Width, -Height, z, p2[1]);
		Vector(Width, Height, z, p2[2]);
		Vector(-Width, Height, z, p2[3]);
		vec3_t Angle;
		Vector(0.f, 0.f, Rotation, Angle);
		float Matrix[3][4];
		AngleMatrix(Angle, Matrix);
		for (int i = 0; i < 4; i++)
		{
			VectorRotate(p2[i], Matrix, p[i]);
			p[i][0] += x;
			p[i][1] += y;
		}
	}

	float c[4][2];
	TEXCOORD(c[3], u, v);
	TEXCOORD(c[2], u + uWidth, v);
	TEXCOORD(c[1], u + uWidth, v + vHeight);
	TEXCOORD(c[0], u, v + vHeight);

	// 1. Calculate the Color (replacing the conditional glColor calls)
	float finalAlpha = 1.0f;
	if (Bitmaps[Texture].Components != 3) {
		if (Texture == BITMAP_BLOOD + 1 || Texture == BITMAP_FONT_HIT)
			finalAlpha = 1.0f;
		else
			finalAlpha = Light[0]; // Matches your glColor4f(..., Light[0]) logic
	}

	// Send the color to the shader as a uniform
	myShader.setVec4(g_uColorLoc, Light[0], Light[1], Light[2], finalAlpha);

	// 2. Pack the Vertex Data (4 vertices for the Quad)
	SpriteVertex3D vao[4];
	for (int i = 0; i < 4; i++) {
		vao[i].x = p[i][0];
		vao[i].y = p[i][1];
		vao[i].z = p[i][2];
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 3. Set Attributes
	// Position
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex3D), &vao[0].x);

	// Texture UV
	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex3D), &vao[0].u);

	// Disable per-vertex color attribute (we are using the uniform u_color instead)
	safe_disable_attr(g_aColorLoc);
	//glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Default to white
	MU_ApplyMatrices();
	// 4. Draw
	// GL_TRIANGLE_FAN is the GLES2 replacement for GL_QUADS
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
//
void RenderSpriteUV(int Texture, vec3_t Position, float Width, float Height, float (*UV)[2], vec3_t Light[4], float Alpha)
{
	BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position, CameraMatrix, p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	Width *= 0.5f;
	Height *= 0.5f;
	vec3_t p[4];
	Vector(x - Width, y - Height, z, p[0]);
	Vector(x + Width, y - Height, z, p[1]);
	Vector(x + Width, y + Height, z, p[2]);
	Vector(x - Width, y + Height, z, p[3]);

	// 1. Pack interleaved data (XYZ, UV, RGBA) into your full vertex struct
	SpriteVertexFull vao[4];

	for (int i = 0; i < 4; i++)
	{
		// Position
		vao[i].x = p[i][0];
		vao[i].y = p[i][1];
		vao[i].z = p[i][2];

		// Texture UVs
		vao[i].u = UV[i][0];
		vao[i].v = UV[i][1];

		// Per-vertex Color (Light[i]) + Global Alpha
		vao[i].r = Light[i][0];
		vao[i].g = Light[i][1];
		vao[i].b = Light[i][2];
		vao[i].a = Alpha;
	}

	// 2. Set Attributes
	// Position (3 floats)
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].x);

	// UV (2 floats)
	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].u);

	// Color (4 floats - pulls from the Light[i] data we packed)
	safe_enable_attr(g_aColorLoc);
	glVertexAttribPointer(g_aColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].r);

	MU_ApplyMatrices();
	myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	// 3. Draw
	// Using GL_TRIANGLE_FAN as the quad replacement
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void RenderNumber(vec3_t Position, int Num, vec3_t Color, float Alpha, float Scale)
{
	vec3_t p;
	VectorCopy(Position, p);
	vec3_t Light[4];
	VectorCopy(Color, Light[0]);
	VectorCopy(Color, Light[1]);
	VectorCopy(Color, Light[2]);
	VectorCopy(Color, Light[3]);
	if (Num == -1)
	{
		float UV[4][2];
		TEXCOORD(UV[0], 0.f, 32.f / 32.f);
		TEXCOORD(UV[1], 32.f / 256.f, 32.f / 32.f);
		TEXCOORD(UV[2], 32.f / 256.f, 17.f / 32.f);
		TEXCOORD(UV[3], 0.f, 17.f / 32.f);
		RenderSpriteUV(BITMAP_FONT + 1, p, 45, 20, UV, Light, Alpha);
	}
	else if (Num == -2)
	{
		RenderSprite(BITMAP_FONT_HIT, p, 32 * Scale, 20 * Scale, Light[0], 0.f, 0.f, 0.f, 27.f / 32.f, 15.f / 16.f);
	}
	else
	{
		char Text[32];
		itoa(Num, Text, 10);
		p[0] -= strlen(Text) * 5.f;
		unsigned int Length = strlen(Text);
		p[0] -= Length * Scale * 0.125f;
		p[1] -= Length * Scale * 0.125f;
		for (unsigned int i = 0; i < Length; i++)
		{
			float UV[4][2];
			float u = (float)(Text[i] - 48) * 16.f / 256.f;
			TEXCOORD(UV[0], u, 16.f / 32.f);
			TEXCOORD(UV[1], u + 16.f / 256.f, 16.f / 32.f);
			TEXCOORD(UV[2], u + 16.f / 256.f, 0.f);
			TEXCOORD(UV[3], u, 0.f);
			RenderSpriteUV(BITMAP_FONT + 1, p, Scale, Scale, UV, Light, Alpha);
			p[0] += Scale * 0.5f;
			p[1] += Scale * 0.5f;
		}
	}
}

float RenderNumber2D(float x, float y, int Num, float Width, float Height)
{
	char Text[32];
	itoa(Num, Text, 10);
	int Length = (int)strlen(Text);
	x -= Width * Length / 2;
	for (int i = 0; i < Length; i++)
	{
		float u = (float)(Text[i] - 48) * 16.f / 256.f;
		//glColor3fv(Color);
		RenderBitmap(BITMAP_FONT + 1, x, y, Width, Height, u, 0.f, 16.f / 256.f, 16.f / 32.f);
		x += Width * 0.7f;
	}
	return x;
}

void BeginBitmap() {

	myShader.use();

	// 1. Handle Projection Stack
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); // Pushes a copy onto your manual projectionStack
	glLoadIdentity();

	// 2. Hardware Viewport
	glViewport(0, 0, WindowWidth, WindowHeight);

	// 3. Set the Ortho (Matches gluOrtho2D 0, W, 0, H)
	// We skip the perspective math because your legacy code wipes it out anyway
	projectionStack.back() = glm::ortho(0.0f, (float)WindowWidth, 0.0f, (float)WindowHeight, -1.0f, 1.0f);

	// 4. Handle ModelView Stack
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); // Pushes a copy onto your manual modelViewStack
	glLoadIdentity();

	// Apply the Ortho + Identity matrices to the GPU
	MU_ApplyMatrices();

	DisableDepthTest();
}


void EndBitmap() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void RenderColor(float x, float y, float Width, float Height, float Alpha, int Flag)
{
	DisableTexture();

	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	float p[4][2];
	y = WindowHeight - y;

	p[0][0] = x; p[0][1] = y;
	p[1][0] = x; p[1][1] = y - Height;
	p[2][0] = x + Width; p[2][1] = y - Height;
	p[3][0] = x + Width; p[3][1] = y;

	// 3. Set Attributes (Position only)
	safe_enable_attr(g_aPosLoc);
	// p is your float p[4][2] array from the original code
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, 0, p);

	// Disable UV and Color attributes as they aren't needed here
	safe_disable_attr(g_aTexLoc);
	safe_disable_attr(g_aColorLoc);

	MU_ApplyMatrices();

	// 1. Set the uniform color based on Flag
	float finalAlpha = (Alpha > 0.0f) ? Alpha : 1.0f;
	if (Flag == 0)
		myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, finalAlpha); // White
	else if (Flag == 1)
		myShader.setVec4(g_uColorLoc, 0.0f, 0.0f, 0.0f, finalAlpha); // Black

	// 4. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void EndRenderColor()
{
	// Reset shader to white and re-enable texture sampling logic
	myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
	myShader.setFloat(g_uTexEnabledLoc, 1.0);
}
//???
void RenderColorBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, unsigned int color)
{
	x = ConvertX(x);
	y = ConvertY(y);

	Width = ConvertX(Width);
	Height = ConvertY(Height);

	BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x; p[0][1] = y;
	p[1][0] = x; p[1][1] = y - Height;
	p[2][0] = x + Width; p[2][1] = y - Height;
	p[3][0] = x + Width; p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	// 1. Convert hex color to 0.0 - 1.0 floats
	float r = (float)(color & 0xff) / 255.0f;
	float g = (float)((color >> 8) & 0xff) / 255.0f;
	float b = (float)((color >> 16) & 0xff) / 255.0f;
	float a = (float)((color >> 24) & 0xff) / 255.0f;

	// 2. Pack the data
	SpriteVertexFull vao[4];
	for (int i = 0; i < 4; i++) {
		vao[i].x = p[i][0];
		vao[i].y = p[i][1];
		vao[i].z = 0.0f;
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
		// Every vertex gets the same color from your hex variable
		vao[i].r = r; vao[i].g = g; vao[i].b = b; vao[i].a = a;
	}

	// 3. Set Attributes
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].x);

	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].u);

	safe_enable_attr(g_aColorLoc);
	glVertexAttribPointer(g_aColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertexFull), &vao[0].r);

	MU_ApplyMatrices();
	myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

	// 4. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

//
void RenderBitmap(int Texture, float x, float y, float Width, float Height,
	float u, float v, float uWidth, float vHeight,
	bool Scale, bool StartScale, float Alpha)
{
	if (StartScale)
	{
		x = ConvertX(x);
		y = ConvertY(y);
	}
	if (Scale)
	{
		Width = ConvertX(Width);
		Height = ConvertY(Height);
	}

	BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x; p[0][1] = y;
	p[1][0] = x; p[1][1] = y - Height;
	p[2][0] = x + Width; p[2][1] = y - Height;
	p[3][0] = x + Width; p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	// 1. Set the global color uniform (replaces glColor4f inside the loop)
	float finalAlpha = (Alpha > 0.0f) ? Alpha : 1.0f;
	myShader.setVec4(g_uColorLoc, 1.0f, 1.0f, 1.0f, finalAlpha);

	// 2. Pack the 4 vertices into a local struct array
	SpriteVertex vao[4];
	for (int i = 0; i < 4; i++) {
		vao[i].x = p[i][0];
		vao[i].y = p[i][1];
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 3. Set Attributes
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].x);

	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].u);

	// Ensure per-vertex color attribute is OFF (we're using the uniform instead)
	safe_disable_attr(g_aColorLoc);



	MU_ApplyMatrices();

	// 4. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}
//
void RenderBitmapRotate(int Texture, float x, float y, float Width, float Height, float Rotate, float u, float v, float uWidth, float vHeight)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
	//x -= Width *0.5f;
	//y -= Height*0.5f;
	BindTexture(Texture);

	vec3_t p[4], p2[4];

	y = WindowHeight - y;

	Vector(-Width * 0.5f, Height * 0.5f, 0.f, p[0]);
	Vector(-Width * 0.5f, -Height * 0.5f, 0.f, p[1]);
	Vector(Width * 0.5f, -Height * 0.5f, 0.f, p[2]);
	Vector(Width * 0.5f, Height * 0.5f, 0.f, p[3]);

	vec3_t Angle;
	Vector(0.f, 0.f, Rotate, Angle);
	float Matrix[3][4];
	AngleMatrix(Angle, Matrix);

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	// 1. Prepare vertex data array
	SpriteVertex vao[4];

	for (int i = 0; i < 4; i++)
	{
		// Apply the legacy rotation math
		VectorRotate(p[i], Matrix, p2[i]);

		// Calculate final screen positions
		vao[i].x = p2[i][0] + x;
		vao[i].y = p2[i][1] + y;

		// Assign UVs
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 2. Set Attributes
	// Position
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].x);

	// Texture UV
	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].u);

	// Ensure per-vertex color is disabled (uses u_color uniform set by previous helpers)
	safe_enable_attr(g_aColorLoc);
	MU_ApplyMatrices();
	// 3. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
//
void RenderBitRotate(int Texture, float x, float y, float Width, float Height, float Rotate)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	BindTexture(Texture);

	vec3_t p[4], p2[4];

	y = Height - y;

	float cx = (Width / 2.f) - (Width - x);
	float cy = (Height / 2.f) - (Height - y);

	float ax = (-Width * 0.5f) + cx;
	float bx = (Width * 0.5f) + cx;
	float ay = (-Height * 0.5f) + cy;
	float by = (Height * 0.5f) + cy;

	Vector(ax, by, 0.f, p[0]);
	Vector(ax, ay, 0.f, p[1]);
	Vector(bx, ay, 0.f, p[2]);
	Vector(bx, by, 0.f, p[3]);

	vec3_t Angle;
	Vector(0.f, 0.f, Rotate, Angle);
	float Matrix[3][4];
	AngleMatrix(Angle, Matrix);

	float c[4][2];
	TEXCOORD(c[0], 0.f, 0.f);
	TEXCOORD(c[3], 1.f, 0.f);
	TEXCOORD(c[2], 1.f, 1.f);
	TEXCOORD(c[1], 0.f, 1.f);

	// 1. Prepare vertex data
	SpriteVertex vao[4];

	for (int i = 0; i < 4; i++)
	{
		// Apply the original rotation math
		VectorRotate(p[i], Matrix, p2[i]);

		// Set screen positions (Relative to screen center)
		vao[i].x = p2[i][0] + (WindowWidth / 2.0f);
		vao[i].y = p2[i][1] + (WindowHeight / 2.0f);

		// Set texture coordinates
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 2. Set Attributes
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].x);

	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].u);

	// Ensure the constant color is white (unless set otherwise by a previous helper)
	safe_disable_attr(g_aColorLoc);
	//glVertexAttrib4f(g_aColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
	MU_ApplyMatrices();
	// 3. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}
//
void RenderPointRotate(int Texture, float ix, float iy, float iWidth, float iHeight, float x, float y, float Width, float Height, float Rotate, float Rotate_Loc, float uWidth, float vHeight, int Num)
{
	int i = 0;
	vec3_t p, p2[4], p3, p4[4], Angle;
	float c[4][2], Matrix[3][4];

	ix = ConvertX(ix);
	iy = ConvertY(iy);
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	BindTexture(Texture);

	y = Height - y;
	iy = Height - iy;

	Vector((ix - (Width * 0.5f)) + ((Width / 2.f) - (Width - x)), (iy - (Height * 0.5f)) + ((Height / 2.f) - (Height - y)), 0.f, p);

	Vector(0.f, 0.f, Rotate, Angle);
	AngleMatrix(Angle, Matrix);

	VectorRotate(p, Matrix, p3);

	Vector(-(iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[0]);
	Vector(-(iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[1]);
	Vector((iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[2]);
	Vector((iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[3]);

	Vector(0.f, 0.f, Rotate_Loc, Angle);
	AngleMatrix(Angle, Matrix);

	TEXCOORD(c[0], 0.f, 0.f);
	TEXCOORD(c[3], uWidth, 0.f);
	TEXCOORD(c[2], uWidth, vHeight);
	TEXCOORD(c[1], 0.f, vHeight);


	// 1. Prepare vertex data array
	SpriteVertex vao[4];

	for (int i = 0; i < 4; i++)
	{
		// Apply the original matrix translation and transformation
		Matrix[0][3] = p3[0] + 25;
		Matrix[1][3] = p3[1];
		VectorTransform(p2[i], Matrix, p4[i]);

		// Calculate final screen positions (Center + Transformed Offset)
		vao[i].x = p4[i][0] + (WindowWidth / 2.0f);
		vao[i].y = p4[i][1] + (WindowHeight / 2.0f);

		// Assign UVs
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 2. Set Attributes
	// Position (using global ID)
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].x);

	// Texture UV (using global ID)
	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].u);

	// Ensure per-vertex color is disabled (uses u_color uniform)
	safe_disable_attr(g_aColorLoc);
	MU_ApplyMatrices();
	// 3. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


	if (Num > -1)
	{
		float dx, dy;
		dx = p4[0][0] + (WindowWidth / 2.f);
		dy = p4[0][1] + (WindowHeight / 2.f);
		dx = dx * (float)(640.f / WindowWidth);
		dy = dy * (float)(480.f / WindowHeight);
		if (Num >= 100)
		{
			g_pNewUIMiniMap->SetBtnPos(Num - 100, dx - (iWidth / 2), (480 - dy) - (iHeight / 2), iWidth, iHeight);
		}
		else
		{
			g_pNewUIMiniMap->SetBtnPos(Num, dx, 480 - dy, iWidth / 2, iHeight / 2);
		}
	}
}
// xx
void RenderBitmapLocalRotate(int Texture, float x, float y, float Width, float Height, float Rotate, float u, float v, float uWidth, float vHeight)
{
	BindTexture(Texture);

	vec3_t p[4];
	x = ConvertX(x);
	y = ConvertY(y);
	y = WindowHeight - y;
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	vec3_t vCenter, vDir;
	Vector(x, y, 0, vCenter);
	Vector(Width * 0.5f, -Height * 0.5f, 0, vDir);
	p[0][0] = vCenter[0] + (vDir[0]) * cosf(Rotate);
	p[0][1] = vCenter[1] + (vDir[1]) * sinf(Rotate);
	p[1][0] = vCenter[0] + (vDir[0]) * sinf(Rotate);
	p[1][1] = vCenter[1] - (vDir[1]) * cosf(Rotate);
	p[2][0] = vCenter[0] - (vDir[0]) * cosf(Rotate);
	p[2][1] = vCenter[1] - (vDir[1]) * sinf(Rotate);
	p[3][0] = vCenter[0] - (vDir[0]) * sinf(Rotate);
	p[3][1] = vCenter[1] + (vDir[1]) * cosf(Rotate);

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	MU2DVertex quad[4];

	for (int i = 0; i < 4; ++i)
	{
		quad[i].x = p[i][0];
		quad[i].y = p[i][1];
		quad[i].u = c[i][0];
		quad[i].v = c[i][1];
	}

	MU_DrawBoundTexturedQuad2D(quad, Texture);
}
// xx
void RenderBitmapAlpha(int Texture, float sx, float sy, float Width, float Height)
{
	EnableAlphaTest();
	BindTexture(Texture);

	sy = WindowHeight - sy;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			float p[4][2];
			p[0][0] = sx + ((x)*Width) * 0.25f; p[0][1] = sy - ((y)*Height) * 0.25f;
			p[1][0] = sx + ((x)*Width) * 0.25f; p[1][1] = sy - ((y + 1) * Height) * 0.25f;
			p[2][0] = sx + ((x + 1) * Width) * 0.25f; p[2][1] = sy - ((y + 1) * Height) * 0.25f;
			p[3][0] = sx + ((x + 1) * Width) * 0.25f; p[3][1] = sy - ((y)*Height) * 0.25f;

			float c[4][2];
			TEXCOORD(c[0], (x) * 0.25f, (y) * 0.25f);
			TEXCOORD(c[1], (x) * 0.25f, (y + 1) * 0.25f);
			TEXCOORD(c[2], (x + 1) * 0.25f, (y + 1) * 0.25f);
			TEXCOORD(c[3], (x + 1) * 0.25f, (y) * 0.25f);

			float Alpha[4] = { 1.f,1.f,1.f,1.f };
			if (x == 0) { Alpha[0] = 0.f; Alpha[1] = 0.f; }
			if (x == 3) { Alpha[2] = 0.f; Alpha[3] = 0.f; }
			if (y == 0) { Alpha[0] = 0.f; Alpha[3] = 0.f; }
			if (y == 3) { Alpha[1] = 0.f; Alpha[2] = 0.f; }
			/*if(x==0&&y==0) Alpha[0] = 0.f;
			if(x==0&&y==3) Alpha[1] = 0.f;
			if(x==3&&y==3) Alpha[2] = 0.f;
			if(x==3&&y==0) Alpha[3] = 0.f;*/

			MU2DColorVertex quad[4];

			for (int i = 0; i < 4; ++i)
			{
				quad[i].x = p[i][0];
				quad[i].y = p[i][1];

				quad[i].u = c[i][0];
				quad[i].v = c[i][1];

				quad[i].r = 255;
				quad[i].g = 255;
				quad[i].b = 255;
				quad[i].a = MU_FloatToColorByte(Alpha[i]);
			}

			MU_DrawBoundTexturedColorQuad2D(quad);
		}
	}
}
//
void RenderBitmapUV(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
	BindTexture(Texture);

	float p[4][2];
	y = WindowHeight - y;
	p[0][0] = x; p[0][1] = y;
	p[1][0] = x; p[1][1] = y - Height;
	p[2][0] = x + Width; p[2][1] = y - Height;
	p[3][0] = x + Width; p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v + vHeight * 0.25f);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight - vHeight * 0.25f);

	// 1. Pack the data into your struct array
	SpriteVertex vao[4];
	for (int i = 0; i < 4; i++) {
		vao[i].x = p[i][0];
		vao[i].y = p[i][1];
		vao[i].u = c[i][0];
		vao[i].v = c[i][1];
	}

	// 2. Set Attributes
	// Position
	safe_enable_attr(g_aPosLoc);
	glVertexAttribPointer(g_aPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].x);

	// Texture UV
	safe_enable_attr(g_aTexLoc);
	glVertexAttribPointer(g_aTexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), &vao[0].u);

	// Ensure per-vertex color is disabled (uses u_color uniform)
	safe_disable_attr(g_aColorLoc);
	MU_ApplyMatrices();
	// 3. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a)
{
	if (a < 0.f) return -a;
	return a;
}

float minf(float a, float b)
{
	if (a > b) return b;
	return a;
}

float maxf(float a, float b)
{
	if (a > b) return a;
	return b;
}

int InsideTest(float x, float y, float z, int n, float* v1, float* v2, float* v3, float* v4, int flag, float type)
{
	if (type > 0.f)
		flag <<= 3;

	int i;
	vec3_t* vtx[4];
	vtx[0] = (vec3_t*)v1;
	vtx[1] = (vec3_t*)v2;
	vtx[2] = (vec3_t*)v3;
	vtx[3] = (vec3_t*)v4;

	int j = n - 1;
	switch (flag)
	{
	case 1:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
			if (d <= 0.f)
				return false;
		}
		break;
	case 2:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
			if (d <= 0.f)
				return false;
		}
		break;
	case 4:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
			if (d <= 0.f)
				return false;
		}
		break;
	case 8:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
			if (d >= 0.f)
				return false;
		}
		break;
	case 16:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
			if (d >= 0.f)
				return false;
		}
		break;
	case 32:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
			if (d >= 0.f)
				return false;
		}
		break;
	}

	return true;
}

float Distance;

void InitCollisionDetectLineToFace()
{
	Distance = 9999999.f;
}

vec3_t CollisionPosition;

bool CollisionDetectLineToFace(vec3_t Position, vec3_t Target, int Polygon, float* v1, float* v2, float* v3, float* v4, vec3_t Normal, bool Collision)
{
	vec3_t Direction;
	VectorSubtract(Target, Position, Direction);
	float a = DotProduct(Direction, Normal);
	if (a >= 0.f) return false;
	float b = DotProduct(Position, Normal) - DotProduct(v1, Normal);
	float t = -b / a;
	if (t >= 0.f && t <= Distance)
	{
		float X = Direction[0] * t + Position[0];
		float Y = Direction[1] * t + Position[1];
		float Z = Direction[2] * t + Position[2];
		int Count = 0;
		float MIN = minf(minf(absf(Direction[0]), absf(Direction[1])), absf(Direction[2]));
		if (MIN == absf(Direction[0]))
		{
			if ((Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1])) &&
				(Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2]))) Count++;
		}
		else if (MIN == absf(Direction[1]))
		{
			if ((Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2])) &&
				(X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0]))) Count++;
		}
		else
		{
			if ((X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0])) &&
				(Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1]))) Count++;
		}
		if (Count == 0) return false;
		Count = 0;
		if (Normal[0] <= -0.5f || Normal[0] >= 0.5f)
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 1, Normal[0]);
		}
		else if (Normal[1] <= -0.5f || Normal[1] >= 0.5f)
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 2, Normal[1]);
		}
		else
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 4, Normal[2]);
		}
		if (Count == 0) return false;
		if (Collision)
		{
			Distance = t;
			Vector(X, Y, Z, CollisionPosition);
		}
		return true;
	}
	return false;
}

bool ProjectLineBox(vec3_t ax, vec3_t p1, vec3_t p2, OBB_t obb)
{
	float P1 = DotProduct(ax, p1);
	float P2 = DotProduct(ax, p2);

	float mx1 = maxf(P1, P2);
	float mn1 = minf(P1, P2);

	float ST = DotProduct(ax, obb.StartPos);
	float Q1 = DotProduct(ax, obb.XAxis);
	float Q2 = DotProduct(ax, obb.YAxis);
	float Q3 = DotProduct(ax, obb.ZAxis);

	float mx2 = ST;
	float mn2 = ST;

	if (Q1 > 0)	mx2 += Q1; else mn2 += Q1;
	if (Q2 > 0)	mx2 += Q2; else mn2 += Q2;
	if (Q3 > 0) mx2 += Q3; else mn2 += Q3;

	if (mn1 > mx2) return false;
	if (mn2 > mx1) return false;

	return true;
}//

bool CollisionDetectLineToOBB(vec3_t p1, vec3_t p2, OBB_t obb)
{
	vec3_t e1;
	vec3_t eq11, eq12, eq13;

	VectorSubtract(p2, p1, e1);

	CrossProduct(e1, obb.XAxis, eq11);
	CrossProduct(e1, obb.YAxis, eq12);
	CrossProduct(e1, obb.ZAxis, eq13);

	if (!ProjectLineBox(eq11, p1, p2, obb)) return false;
	if (!ProjectLineBox(eq12, p1, p2, obb)) return false;
	if (!ProjectLineBox(eq13, p1, p2, obb)) return false;

	if (!ProjectLineBox(obb.XAxis, p1, p2, obb)) return false;
	if (!ProjectLineBox(obb.YAxis, p1, p2, obb)) return false;
	if (!ProjectLineBox(obb.ZAxis, p1, p2, obb)) return false;

	return true;
}

