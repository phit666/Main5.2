#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "callstackdmp.h"

using namespace leaf;


//. Implementation

CCallStackDump::CCallStackFrame::CCallStackFrame() : m_pFrameAddr(NULL), m_pReturnAddr(NULL) {
	ZeroMemory(m_pParameter, MAX_PARAMETER_BUFSIZE*sizeof(DWORD));
}
CCallStackDump::CCallStackFrame::~CCallStackFrame() {}

bool CCallStackDump::CCallStackFrame::Create(DWORD* pEbp) {
#ifdef _WIN32
	if(IsBadReadPtr( pEbp, sizeof(DWORD)))
		return false;
	
	m_pFrameAddr = (void*) pEbp;
	m_pReturnAddr = (void*) *(pEbp+1);

	for(int i=0; i<MAX_PARAMETER_BUFSIZE; i++) {
		if(IsBadReadPtr(pEbp+2+i, sizeof(DWORD)))
			break;
		memcpy(m_pParameter+i, (void*)(pEbp+2+i), sizeof(DWORD));
	}
#endif
	return true;
}
void CCallStackDump::CCallStackFrame::Release() {
	m_pFrameAddr = m_pReturnAddr = NULL;
	ZeroMemory(m_pParameter, MAX_PARAMETER_BUFSIZE*sizeof(DWORD));
}

void* CCallStackDump::CCallStackFrame::GetFrameAddr() { return m_pFrameAddr; }
void* CCallStackDump::CCallStackFrame::GetReturnAddr() { return m_pReturnAddr; }
void CCallStackDump::CCallStackFrame::GetParameter(DWORD* pBuf, size_t size) {
#ifdef _WIN32
	if(size > MAX_PARAMETER_BUFSIZE)
		size = MAX_PARAMETER_BUFSIZE;
	memcpy(pBuf, m_pParameter, size*sizeof(DWORD));
#endif
}

CCallStackDump::CCallStackDump() {}
CCallStackDump::~CCallStackDump() { Clear(); }

void CCallStackDump::Dump(const CONTEXT* pContext) {
#ifdef _WIN32
	Clear();
	
	DWORD* pEbp = (DWORD*)pContext->Ebp;
	for(int i=0; i<64; i++) 
	{
		CCallStackFrame* pCallStackFrame = new CCallStackFrame;
		if(!pCallStackFrame->Create(pEbp)) 
		{
			delete pCallStackFrame; break;
		}
		m_listFrame.push_back(pCallStackFrame);

		if(IsBadReadPtr((DWORD*)*pEbp, sizeof(DWORD)))
			break;

		pEbp = (DWORD*)*pEbp;
	}
#endif
}
void CCallStackDump::Dump() 
{
#ifdef _WIN32
	CONTEXT ct;
	ZeroMemory(&ct, sizeof(CONTEXT));
	
	ct.ContextFlags = CONTEXT_FULL;
	::GetThreadContext( GetCurrentThread(), &ct);
	
	Dump(&ct);
#endif
}
void CCallStackDump::Clear() 
{
#ifdef _WIN32
	type_framevect::iterator fvi = m_listFrame.begin();
	for(; fvi != m_listFrame.end(); fvi++)
		delete (*fvi);
	m_listFrame.clear();
#endif
}

size_t CCallStackDump::GetStackDepth() const { return m_listFrame.size(); }

void* CCallStackDump::GetFrameAddr(int index) 
{
#ifdef _WIN32
	if(index >= (int)GetStackDepth()) return NULL;
		return m_listFrame[index]->GetFrameAddr();
#endif
}
void* CCallStackDump::GetReturnAddr(int index) 
{
#ifdef _WIN32
	if(index >= (int)GetStackDepth()) return NULL;
		return m_listFrame[index]->GetReturnAddr();
#else
		return NULL;
#endif
}
void CCallStackDump::GetParameter(int index, DWORD* pBuf, size_t size) 
{
#ifdef _WIN32
	if(index >=0 && index < (int)GetStackDepth()) {
		m_listFrame[index]->GetParameter(pBuf, size);
	}
#endif
}