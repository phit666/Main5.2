//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoginWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "UIControls.h"
#include "ZzzScene.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "ProtocolSend.h"
#include "ServerListManager.h"
#include "mu_sdl.h"
#include "wt.h"

#define	LIW_ACCOUNT		0
#define	LIW_PASSWORD	1

#define LIW_OK			0
#define LIW_CANCEL		1

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int g_iChatInputType;


CLoginWin::CLoginWin()
{
	m_pIDInputBox = NULL;
	m_pPassInputBox = NULL;
}

CLoginWin::~CLoginWin()
{
	SAFE_DELETE(m_pIDInputBox);
	SAFE_DELETE(m_pPassInputBox);
}

void CLoginWin::Create()
{
	CWin::Create(getScaleNewSize(BITMAP_LOG_IN + 7, 329), getScaleNewSize(BITMAP_LOG_IN + 7, 245), getScaleTexID(BITMAP_LOG_IN + 7));

	m_asprInputBox[LIW_ACCOUNT].Create(getScaleNewSize(BITMAP_LOG_IN + 8, 156), getScaleNewSize(BITMAP_LOG_IN + 8, 23), getScaleTexID(BITMAP_LOG_IN + 8));
	m_asprInputBox[LIW_PASSWORD].Create(getScaleNewSize(BITMAP_LOG_IN + 8, 156), getScaleNewSize(BITMAP_LOG_IN + 8, 23), getScaleTexID(BITMAP_LOG_IN + 8));

	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(getScaleNewSize(BITMAP_BUTTON + i, 54), getScaleNewSize(BITMAP_BUTTON + i, 30), getScaleTexID(BITMAP_BUTTON + i), 3, 2, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

	SAFE_DELETE(m_pIDInputBox);

	m_pIDInputBox = new CUITextInputBox;
	m_pIDInputBox->Init(g_hWnd, getScaleNewSize(BITMAP_LOG_IN + 8, 140), getScaleNewSize(BITMAP_LOG_IN + 8, 14), MAX_ID_SIZE);
	m_pIDInputBox->SetBackColor(0, 0, 0, 255);
	m_pIDInputBox->SetTextColor(255, 255, 230, 210);
	m_pIDInputBox->SetFont(g_hFixFont);
	m_pIDInputBox->SetState(UISTATE_NORMAL);
	m_pIDInputBox->SetText(m_ID);
	m_pIDInputBox->SetTitle("login-username");

	SAFE_DELETE(m_pPassInputBox);

	m_pPassInputBox = new CUITextInputBox;
	m_pPassInputBox->Init(g_hWnd, getScaleNewSize(BITMAP_LOG_IN + 8, 140), getScaleNewSize(BITMAP_LOG_IN + 8, 14), MAX_PASSWORD_SIZE, TRUE);
	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);
	m_pPassInputBox->SetTitle("login-password");

	m_pIDInputBox->SetTabTarget(m_pPassInputBox);
	m_pPassInputBox->SetTabTarget(m_pIDInputBox);

	m_pIDInputBox->SetText("lorelie");
	m_pPassInputBox->SetText("redzone");

	
	this->FirstLoad = 1;
}

void CLoginWin::PreRelease()
{
	for (int i = 0; i < 2; ++i)
		m_asprInputBox[i].Release();
}

void CLoginWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	m_asprInputBox[LIW_ACCOUNT].SetPosition(nXCoord + getScaleNewSize(BITMAP_LOG_IN + 8, 109), nYCoord + getScaleNewSize(BITMAP_LOG_IN + 8, 106));
	m_asprInputBox[LIW_PASSWORD].SetPosition(nXCoord + getScaleNewSize(BITMAP_LOG_IN + 8, 109), nYCoord + getScaleNewSize(BITMAP_LOG_IN + 8, 131));

	if (g_iChatInputType == 1)
	{
		m_pIDInputBox->SetPosition(int((m_asprInputBox[LIW_ACCOUNT].GetXPos() + 6) / g_fScreenRate_x),
			int((m_asprInputBox[LIW_ACCOUNT].GetYPos() + 6) / g_fScreenRate_y));

		m_pPassInputBox->SetPosition(int((m_asprInputBox[LIW_PASSWORD].GetXPos() + 6) / g_fScreenRate_x),
			int((m_asprInputBox[LIW_PASSWORD].GetYPos() + 6) / g_fScreenRate_y));
	}

	m_aBtn[LIW_OK].SetPosition(nXCoord + getScaleNewSize(BITMAP_BUTTON + 0, 150), nYCoord + getScaleNewSize(BITMAP_BUTTON + 0, 178));
	m_aBtn[LIW_CANCEL].SetPosition(nXCoord + getScaleNewSize(BITMAP_BUTTON + 1, 211), nYCoord + getScaleNewSize(BITMAP_BUTTON + 1, 178));
}

void CLoginWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < 2; ++i)
	{
		m_asprInputBox[i].Show(bShow);
		m_aBtn[i].Show(bShow);


	}
}

bool CLoginWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	}

	return CWin::CursorInWin(nArea);
}

void CLoginWin::UpdateWhileActive(double dDeltaTick)
{
	CInput& rInput = CInput::Instance();

	if (m_aBtn[LIW_OK].IsClick())
		RequestLogin();
	else if (m_aBtn[LIW_CANCEL].IsClick())
		CancelLogin();
	else if (CInput::Instance().IsKeyDown(VK_RETURN))
	{
		::PlayBuffer(SOUND_CLICK01);
		RequestLogin();
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
}

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
	m_pIDInputBox->DoAction();
	m_pPassInputBox->DoAction();
}

void CLoginWin::RenderControls()
{

	if (this->FirstLoad == 1)
	{
		if (strlen(m_ID) > 0)
			CUIMng::Instance().m_LoginWin.GetPassInputBox()->GiveFocus();
		else
			CUIMng::Instance().m_LoginWin.GetIDInputBox()->GiveFocus();
		this->FirstLoad = 0;
	}

	CWin::RenderButtons();

	for (int i = 0; i < 2; ++i)
		m_asprInputBox[i].Render();

	m_pIDInputBox->Render();
	m_pPassInputBox->Render();

	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->RenderText(int((CWin::GetXPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 30)) / g_fScreenRate_x),
		int((CWin::GetYPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 113)) / g_fScreenRate_y), GlobalText[450]);
	g_pRenderText->RenderText(int((CWin::GetXPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 30)) / g_fScreenRate_x),
		int((CWin::GetYPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 139)) / g_fScreenRate_y), GlobalText[451]);

	unicode::t_char szServerName[MAX_TEXT_LENGTH];

	const char* apszGlobalText[4]
		= { GlobalText[461], GlobalText[460], GlobalText[3130], GlobalText[3131] };
	sprintf(szServerName, apszGlobalText[g_ServerListManager->GetNonPVPInfo()],
		g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());

	g_pRenderText->RenderText(int((CWin::GetXPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 111)) / g_fScreenRate_x),
		int((CWin::GetYPos() + getScaleNewSize(BITMAP_LOG_IN + 8, 80)) / g_fScreenRate_y), szServerName);
}

void CLoginWin::RequestLogin()
{
	if (CurrentProtocolState == REQUEST_JOIN_SERVER)
		return;

	CUIMng::Instance().HideWin(this);

	char szID[MAX_ID_SIZE+1] = { 0, };
	char szPass[MAX_PASSWORD_SIZE+1] = {0, };
	m_pIDInputBox->GetText(szID, MAX_ID_SIZE+1);
	m_pPassInputBox->GetText(szPass, MAX_PASSWORD_SIZE+1);
	
	if (unicode::_strlen(szID) <= 0)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
	else if (unicode::_strlen(szPass) <= 0)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
	else
	{
		if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
		{
			g_ConsoleDebug->Write( MCD_NORMAL, "Login with the following account: %s", szID);

			g_ErrorReport.Write("> Login Request.\r\n");
			g_ErrorReport.Write("> Try to Login \"%s\"\r\n", szID);
			// SendRequestLogIn()

			#ifdef NEW_PROTOCOL_SYSTEM
				gProtocolSend.SendRequestLogInNew(szID, szPass);
			#else
				SendRequestLogIn(szID, szPass);
			#endif
		
		}
	}
}

void CLoginWin::CancelLogin()
{
	ConnectConnectionServer();
	CUIMng::Instance().HideWin(this);
}

void CLoginWin::ConnectConnectionServer()
{
	#ifdef NEW_PROTOCOL_SYSTEM
		gProtocolSend.DisconnectServer();
	#endif

	LogIn = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
	CreateSocket(szServerIpAddress, g_ServerPort);
}