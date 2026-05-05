/*******************************************************************************
*	�� �� �� : ������
*	�� �� �� : 2009.06.10
*	��    �� : FileDownloader
*				File ���� �ٿ�ε� ��� ����
*******************************************************************************/

#pragma once
#include "mu_sdl.h"
#include "GameShop\ShopListManager\interface\IConnecter.h"
#include "GameShop\ShopListManager\interface\IDownloaderStateEvent.h"

class FileDownloader
{
public:
// Constructor, Destructor

	FileDownloader(IDownloaderStateEvent *	pStateEvent,
				   DownloadServerInfo *		pServerInfo,
				   DownloadFileInfo *		pFileInfo);
	~FileDownloader();


// public Function

	//					�ٿ�ε� ����
	void				Break();
	//					������ ���� �ٿ�ε� ���� : ����, Ŀ��Ʈ, ���� ���� ��� ó��
	WZResult			DownloadFile();


private:
// private Function
	
	//					���� ����
	BOOL				CanBeContinue();
	//					������
	void				Release();

	//					Ŀ���� ����
	IConnecter *		CreateConnecter();
	//					���� ó��
	WZResult 			CreateConnection();
	static unsigned int __stdcall RunConnectThread(LPVOID pParam);
	WZResult 			Connection();

	//					���� ó��
	WZResult 			TransferRemoteFile();

	//					���� ���� ����
	WZResult 			CreateLocalFile();
	//					�ٿ�ε� ���� �б�
	WZResult 			ReadRemoteFile(BYTE* byReadBuffer, DWORD* dwBytesRead);
	//					���� ���� ����
	WZResult 			WriteLocalFile(BYTE* byReadBuffer, DWORD dwBytesRead);

	//					�ٿ�ε� ���� �̺�Ʈ ������
	void				SendStartedDownloadFileEvent(ULONGLONG nFileLength);
	//					�ٿ�ε� �Ϸ� �̺�Ʈ ������
	void				SendCompletedDownloadFileEvent(WZResult wzResult);
	//					�ٿ�ε� ���� ��Ȳ �̺�Ʈ ������ : ��Ŷ ����
	void				SendProgressDownloadFileEvent(ULONGLONG nTotalBytesRead);


// Member Object

	//							�ٿ�ε� ���� �÷���
	volatile BOOL				m_bBreak;
	//							���..
	WZResult 					m_Result;

	//							�ٿ�ε� ���� �̺�Ʈ ���� ��ü
	IDownloaderStateEvent *		m_pStateEvent;
	//							�ٿ�ε� ���� ���� ��ü
	DownloadServerInfo *		m_pServerInfo;
	//							�ٿ�ε� ���� ���� ��ü
	DownloadFileInfo *			m_pFileInfo;
	//							Ŀ����
	IConnecter *				m_pConnecter;

	//							WinINet ���� �ڵ�
	HINTERNET					m_hSession;
	//							WinINet Ŀ���� �ڵ�
	HINTERNET					m_hConnection;
	//							���� ���� �ڵ�
	HINTERNET					m_hRemoteFile;
	//							���� ���� �ڵ�
#ifdef _WIN32
	HANDLE						m_hLocalFile;
#else
	MU_FILE* m_hLocalFile;
#endif
	//							���� ������
	ULONGLONG					m_nFileLength;
};

