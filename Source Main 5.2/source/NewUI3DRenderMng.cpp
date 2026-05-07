// NewUI3DRenderMng.cpp: implementation of the CNewUI3DRenderMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUI3DRenderMng.h"
#include "NewUIManager.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUI3DCamera::CNewUI3DCamera() 
{

}

SEASON3B::CNewUI3DCamera::~CNewUI3DCamera() 
{ 
	Release(); 
}

bool SEASON3B::CNewUI3DCamera::Create(int iCameraIndex, UINT uiWidth, UINT uiHeight, float fZOrder)
{
	Release();

	m_iCameraIndex = iCameraIndex;
	m_uiWidth = uiWidth;
	m_uiHeight = uiHeight;
	m_fZOrder = fZOrder;

	return true;
}

void SEASON3B::CNewUI3DCamera::Release()
{
	RemoveAll3DRenderObjs();
	m_deque2DEffects.clear();
}

bool SEASON3B::CNewUI3DCamera::IsEmpty()
{ 
	return m_list3DObjs.empty(); 
}

void SEASON3B::CNewUI3DCamera::Add3DRenderObj(INewUI3DRenderObj* pObj)
{
	if(std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj) == m_list3DObjs.end())
	{
		m_list3DObjs.push_back(pObj);
	}
}

void SEASON3B::CNewUI3DCamera::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
	type_list_3dobj::iterator vi = std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj);
	if(vi != m_list3DObjs.end())
	{
		m_list3DObjs.erase(vi);
	}
}

void SEASON3B::CNewUI3DCamera::RemoveAll3DRenderObjs()
{
	m_list3DObjs.clear();
}

void SEASON3B::CNewUI3DCamera::RenderUI2DEffect(UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	UI_2DEFFECT_INFO UI2DEffectInfo;
	UI2DEffectInfo.pCallbackFunc = pCallbackFunc;
	UI2DEffectInfo.pClass = pClass;
	UI2DEffectInfo.dwParamA = dwParamA;
	UI2DEffectInfo.dwParamB = dwParamB;

	m_deque2DEffects.push_back(UI2DEffectInfo);
}

void SEASON3B::CNewUI3DCamera::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
	type_deque_2deffect::iterator di = m_deque2DEffects.begin();
	for(; di != m_deque2DEffects.end(); di++)
	{
		if((*di).pCallbackFunc == pCallbackFunc)
		{
			m_deque2DEffects.erase(di);
			break;
		}
	}
}

int SEASON3B::CNewUI3DCamera::GetCameraIndex() const
{ 
	return m_iCameraIndex; 
}

float SEASON3B::CNewUI3DCamera::GetLayerDepth()
{
	//. fZOrder == fLayerDepth
	return m_fZOrder;
}

bool SEASON3B::CNewUI3DCamera::Render()
{
	if (m_list3DObjs.empty())
		return true;

	// 1. Exit 2D Mode
	EndBitmap();

	// 2. PROJECTION RESET (New 3D Context)
	// Using m_uiWidth/Height for the sub-window/sub-scene size
	projectionStack.push_back(glm::mat4(1.0f));
	glViewport(0, 0, m_uiWidth, m_uiHeight);

	float aspect = (float)m_uiWidth / (float)m_uiHeight;
	// gluPerspective2(1.f, ...) -> Intense telephoto zoom
	projectionStack.back() = glm::perspective(glm::radians(1.0f), aspect, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);

	// 3. MODELVIEW RESET
	modelViewStack.push_back(glm::mat4(1.0f));
	GetOpenGLMatrix(CameraMatrix); // Snapshot Identity

	// 4. HARDWARE STATES
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT); // Ensure 3D objects don't clip with previous world depth

	// 5. SHADER SYNC
	myShader.use();
	myShader.setMat4(g_uMvpLoc, projectionStack.back() * modelViewStack.back());
	myShader.setBool(g_uFogEnabledLoc, false); // No fog for UI 3D objects

	// 6. RENDER OBJECT LIST
	for (auto li = m_list3DObjs.begin(); li != m_list3DObjs.end(); ++li)
	{
		if ((*li)->IsVisible())
		{
			// Inside Render3D, ensure it updates u_mvpMatrix if it moves/rotates
			(*li)->Render3D();
		}
	}

	// 7. UPDATE MOUSE
	UpdateMousePositionn();

	// 8. RESTORE MATRICES (glPopMatrix equivalents)
	if (modelViewStack.size() > 1) modelViewStack.pop_back();
	if (projectionStack.size() > 1) projectionStack.pop_back();

	// 9. RE-ENTER 2D MODE
	BeginBitmap();
	// Ensure BeginBitmap resets the viewport to WindowWidth/Height if needed


	while (!m_deque2DEffects.empty())
	{
		UI_2DEFFECT_INFO& UI2DEffectInfo = m_deque2DEffects.front();
		if (UI2DEffectInfo.pCallbackFunc)
		{
			(*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
		}
		m_deque2DEffects.pop_front();
	}

	return true;
}

bool SEASON3B::CNewUI3DCamera::Update()
{
	//. DOING NOTHING
	return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateMouseEvent()
{
	//. DOING NOTHING
	return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateKeyEvent()
{
	//. DOING NOTHING
	return true;
}

SEASON3B::CNewUI3DRenderMng::CNewUI3DRenderMng() 
{

}

SEASON3B::CNewUI3DRenderMng::~CNewUI3DRenderMng() 
{ 
	Release(); 
}

bool SEASON3B::CNewUI3DRenderMng::Create(CNewUIManager* pNewUIMng)
{
	m_pNewUIMng = pNewUIMng;
	return true;
}

void SEASON3B::CNewUI3DRenderMng::Release()
{ 
	RemoveAll3DRenderObjs(); 
}

void SEASON3B::CNewUI3DRenderMng::Add3DRenderObj(INewUI3DRenderObj* pObj, float fZOrder/* = INFORMATION_CAMERA_Z_ORDER*/)
{
	CNewUI3DCamera* pCamera = FindCamera(fZOrder);
	if(NULL == pCamera)
	{
		int iAvailableCameraIndex = FindAvailableCameraIndex();
		if(-1 != iAvailableCameraIndex)
		{
			pCamera = new CNewUI3DCamera;
			pCamera->Create(iAvailableCameraIndex, WindowWidth, WindowHeight, fZOrder);
			pCamera->Add3DRenderObj(pObj);
			m_pNewUIMng->AddUIObj(iAvailableCameraIndex, pCamera);
			m_listCamera.push_back(pCamera);
		}
		else
		{
#ifdef _DEBUG
			__asm { int 3 };
#endif // _DEBUG
		}
	}
	else
	{
		pCamera->Add3DRenderObj(pObj);
	}	
}
void SEASON3B::CNewUI3DRenderMng::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
	type_list_camera::iterator li = m_listCamera.begin();
	for(; li != m_listCamera.end(); li++)
	{
		(*li)->Remove3DRenderObj(pObj);
		if((*li)->IsEmpty())
		{
			m_pNewUIMng->RemoveUIObj(*li);
			delete (*li);
			m_listCamera.erase(li);
			break;
		}
	}
}

void SEASON3B::CNewUI3DRenderMng::RemoveAll3DRenderObjs()
{
	type_list_camera::iterator li = m_listCamera.begin();
	for(; li != m_listCamera.end(); li++)
	{
		delete (*li);
		m_pNewUIMng->RemoveUIObj(*li);
	}
	m_listCamera.clear();
}

void SEASON3B::CNewUI3DRenderMng::RenderUI2DEffect(float fZOrder, UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	CNewUI3DCamera* pCamera = FindCamera(fZOrder);
	if(pCamera)
		pCamera->RenderUI2DEffect(pCallbackFunc, pClass, dwParamA, dwParamB);
}

void SEASON3B::CNewUI3DRenderMng::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
	type_list_camera::iterator li = m_listCamera.begin();
	for(; li != m_listCamera.end(); li++)
		(*li)->DeleteUI2DEffectObject(pCallbackFunc);
}

CNewUI3DCamera* SEASON3B::CNewUI3DRenderMng::FindCamera(float fZOrder)
{
	type_list_camera::iterator li = m_listCamera.begin();
	for(; li != m_listCamera.end(); li++)
		if((*li)->GetLayerDepth() == fZOrder)
			return (*li);
	return NULL;
}

int SEASON3B::CNewUI3DRenderMng::FindAvailableCameraIndex()
{
	for(int iIndex=INTERFACE_3DRENDERING_CAMERA_BEGIN; iIndex<INTERFACE_3DRENDERING_CAMERA_END; iIndex++)
	{
		type_list_camera::iterator li = m_listCamera.begin();
		for(; li != m_listCamera.end(); li++)
		{
			if((*li)->GetCameraIndex() == iIndex)
				break;
		}
		if(li == m_listCamera.end())
			return iIndex;
	}
	return -1;
}
