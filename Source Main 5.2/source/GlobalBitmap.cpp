// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GlobalBitmap.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "mu_sdl.h"
#include "wt.h"

#ifndef j_boolean
#define j_boolean int
#endif

#undef SetTimer

std::vector<_TexScaleMap> g_mapTexScale = {
	{0, 0, false, false, "Default"},
#ifdef __ANDROID__
	{BITMAP_LOG_IN, 480.0f, true, true, "Select Server"},
	{BITMAP_LOG_IN + 1, 480.0f, true, true, "Select Server"},

	{BITMAP_LOG_IN + 7, 480.0f, true, true, "Login-Box"},
	{BITMAP_LOG_IN + 8, 480.0f, true, true, "Login-User/Pass"},
	{BITMAP_BUTTON, 480.0f, true, true, "Login Window"},
	{BITMAP_BUTTON + 1, 480.0f, true, true, "Login Window"},
#endif
};

_TexScaleMap getTexScale(GLuint TexID) {
	for (auto iter = g_mapTexScale.begin(); iter != g_mapTexScale.end(); iter++) {
		if (iter->TexID == TexID) {
			return *iter;
		}
	}
	return g_mapTexScale[0];
}

GLuint getScaleTexID(GLuint TexID) {
	for (auto iter = g_mapTexScale.begin(); iter != g_mapTexScale.end(); iter++) {
		if (iter->TexID == TexID) {
			if (iter->isscale) {
				return (TexID + BITMAP_SCALED);
			}
		}
	}
	return TexID;
}

int32_t getScaleNewSize(GLuint TexID, int32_t size) {
	for (auto iter = g_mapTexScale.begin(); iter != g_mapTexScale.end(); iter++) {
		if (iter->TexID == TexID) {
			if (iter->isscale) {
				return (size * WindowHeight / iter->scale);
			}
		}
	}
	return size;
}


CBitmapCache::CBitmapCache() 
{
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}
CBitmapCache::~CBitmapCache() { Release(); }

bool CBitmapCache::Create()
{
	Release();

	DWORD dwRange = 0;

	dwRange =  BITMAP_MAPTILE_END - BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMin = BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMax = BITMAP_MAPTILE_END;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange =  BITMAP_MAPGRASS_END - BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMin = BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMax = BITMAP_MAPGRASS_END;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange =  BITMAP_WATER_END - BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMin = BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMax = BITMAP_WATER_END;
	m_QuickCache[QUICK_CACHE_WATER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_WATER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_WATER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_CURSOR_END - BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMin = BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMax = BITMAP_CURSOR_END;
	m_QuickCache[QUICK_CACHE_CURSOR].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_FONT_END - BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMin = BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMax = BITMAP_FONT_END;
	m_QuickCache[QUICK_CACHE_FONT].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_FONT].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_FONT].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_INTERFACE_NEW_MAINFRAME_END - BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_MAINFRAME_END;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	dwRange = BITMAP_INTERFACE_NEW_SKILLICON_END - BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_SKILLICON_END;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));
	
	dwRange = BITMAP_PLAYER_TEXTURE_END - BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMin = BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMax = BITMAP_PLAYER_TEXTURE_END;
	m_QuickCache[QUICK_CACHE_PLAYER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	m_pNullBitmap = new BITMAP_t;
	memset(m_pNullBitmap, 0, sizeof(BITMAP_t));

	m_ManageTimer.SetTimer(1500);

	return true;
}
void CBitmapCache::Release()
{
	SAFE_DELETE(m_pNullBitmap);

	RemoveAll();

	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(m_QuickCache[i].ppBitmap)
			delete [] m_QuickCache[i].ppBitmap;
	}
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(pBitmap)
				m_QuickCache[i].ppBitmap[dwVI] = pBitmap;
			else
				m_QuickCache[i].ppBitmap[dwVI] = m_pNullBitmap;
			return;
		}
	}
	if(pBitmap)
	{
		if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
			m_mapCachePlayer.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheInterface.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheEffect.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else
			m_mapCacheMain.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
	}
}
void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			m_QuickCache[i].ppBitmap[dwVI] = NULL;
			return;
		}
	}

	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
			m_mapCachePlayer.erase(mi);
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
			m_mapCacheInterface.erase(mi);
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
			m_mapCacheEffect.erase(mi);
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
			m_mapCacheMain.erase(mi);
	}
}
void CBitmapCache::RemoveAll()
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		memset(m_QuickCache[i].ppBitmap, 0, m_QuickCache[i].dwRange*sizeof(BITMAP_t*));
	}
	m_mapCacheMain.clear();
	m_mapCachePlayer.clear();
	m_mapCacheInterface.clear();
	m_mapCacheEffect.clear();
}

size_t CBitmapCache::GetCacheSize()
{ 
	return m_mapCacheMain.size() + m_mapCachePlayer.size() + 
		m_mapCacheInterface.size() + m_mapCacheEffect.size(); 
}

void CBitmapCache::Update()
{
	m_ManageTimer.UpdateTime();

	if(m_ManageTimer.IsTime())
	{
		type_cache_map::iterator mi = m_mapCacheMain.begin();
		for(; mi != m_mapCacheMain.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheMain.erase(mi);
			}
		}

		mi = m_mapCachePlayer.begin();
		for(; mi != m_mapCachePlayer.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;

			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCachePlayer.erase(mi);
			}
		}

		mi = m_mapCacheInterface.begin();
		for(; mi != m_mapCacheInterface.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheInterface.erase(mi);
			}
		}

		mi = m_mapCacheEffect.begin();
		for(; mi != m_mapCacheEffect.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				mi = m_mapCacheEffect.erase(mi);
			}
		}

#ifdef DEBUG_BITMAP_CACHE
		g_ConsoleDebug->Write(MCD_NORMAL, "M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(), 
			m_mapCachePlayer.size(), m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
	}
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && 
			uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(m_QuickCache[i].ppBitmap[dwVI])
			{
				if(m_QuickCache[i].ppBitmap[dwVI] == m_pNullBitmap)
					*ppBitmap = NULL;
				else
				{
					*ppBitmap = m_QuickCache[i].ppBitmap[dwVI];
				}
				return true;
			}
			return false;
		}
	}

	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	return false;
}

CGlobalBitmap::CGlobalBitmap() 
{ 
	Init();
	m_BitmapCache.Create();

#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.SetTimer(5000);
#endif // DEBUG_BITMAP_CACHE
}
CGlobalBitmap::~CGlobalBitmap() 
{ 
	UnloadAllImages();
}
void CGlobalBitmap::Init()
{
	m_uiAlternate = 0;
	m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
	m_dwUsedTextureMemory = 0;
}

GLuint CGlobalBitmap::LoadImage(const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	BITMAP_t* pBitmap = FindTexture(filename);

	if(pBitmap)
	{
		if(pBitmap->Ref > 0)
		{
			if(0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;

				return pBitmap->BitmapIndex;
			}
		}
	}
	else
	{
		GLuint uiNewTextureIndex = GenerateTextureIndex();
		if(true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode))
		{
			m_listNonamedIndex.push_back(uiNewTextureIndex);

			return uiNewTextureIndex;
		}
	}

	//char t[100] = { 0 };
	//sprintf(t, "[SDL-DEBUG] CGlobalBitmap::LoadImage, %s load failed.", filename.c_str());
	//OutputDebugString(t);

	return BITMAP_UNKNOWN;
}
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	unsigned int UICLAMP	= GL_CLAMP_TO_EDGE;
	unsigned int UIREPEAT	= GL_REPEAT;

	if( uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT )
	{
#ifdef _DEBUG
		static unsigned int	uiCnt2 = 0;
		int			iBuff;	iBuff = 0;
		
		char		szDebugOutput[256];
		
		iBuff = iBuff + sprintf( iBuff + szDebugOutput, "%d. Call No CLAMP & No REPEAT. \n", uiCnt2++);
		OutputDebugString( szDebugOutput );
#endif
	}

	_TexScaleMap _tex = getTexScale(uiBitmapIndex);

	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);

	if (mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;
		if (pBitmap->Ref > 0)
		{
			if (0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;
				if (!_tex.isscale || !_tex.iscopy)
					return true;
			}
			else
			{
				g_ErrorReport.Write("File not found %s (%d)->%s\r\n", pBitmap->FileName, uiBitmapIndex, filename.c_str());
				UnloadImage(uiBitmapIndex, true);
			}
		}
	}

	if (_tex.iscopy) {

		type_bitmap_map::iterator mi = m_mapBitmap.find(_tex.TexID + BITMAP_SCALED);

		if (mi != m_mapBitmap.end())
		{
			BITMAP_t* pBitmap = (*mi).second;
			if (pBitmap->Ref > 0)
			{
				if (0 == _stricmp(pBitmap->FileName, filename.c_str()))
				{
					pBitmap->Ref++;
					return true;
				}
				else
				{
					g_ErrorReport.Write("File not found %s (%d)->%s\r\n", pBitmap->FileName, uiBitmapIndex, filename.c_str());
					UnloadImage(uiBitmapIndex, true);
				}
			}
		}
	}
	
	std::string ext;
	SplitExt(filename, ext, false);


	if (_tex.isscale) {
		bool _res = false;
		if (0 == _stricmp(ext.c_str(), "jpg"))
			_res = OpenJpegScaledCopy(_tex.TexID + BITMAP_SCALED, filename, uiFilter, uiWrapMode, WindowHeight / _tex.scale);
		else if (0 == _stricmp(ext.c_str(), "tga"))
			_res = OpenTgaScaledCopy(_tex.TexID + BITMAP_SCALED, filename, uiFilter, uiWrapMode, WindowHeight / _tex.scale);
		if (!_res)
			return false;
		if (!_tex.iscopy)
			return true;
	} 

	if (0 == _stricmp(ext.c_str(), "jpg"))
		return OpenJpeg(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	else if (0 == _stricmp(ext.c_str(), "tga"))
		return OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	
	return false;
}
void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if(mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;

		if(--pBitmap->Ref == 0 || bForce)
		{
			glDeleteTextures( 1, &(pBitmap->TextureNumber));

			m_dwUsedTextureMemory -= (DWORD)(pBitmap->Width * pBitmap->Height * pBitmap->Components);

			delete [] pBitmap->Buffer;
			delete pBitmap;
			m_mapBitmap.erase(mi);
			
			if(uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
			{
				m_listNonamedIndex.remove(uiBitmapIndex);
			}
			m_BitmapCache.Remove(uiBitmapIndex);
		}
	}
}
void CGlobalBitmap::UnloadAllImages()
{
#ifdef _DEBUG
	if(!m_mapBitmap.empty())
		g_ErrorReport.Write("Unload Images\r\n");
#endif // _DEBUG
	
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;

#ifdef _DEBUG
		if(pBitmap->Ref > 1)
		{
			g_ErrorReport.Write("Bitmap %s(RefCount= %d)\r\n", pBitmap->FileName, pBitmap->Ref);
		}
#endif // _DEBUG
		delete [] pBitmap->Buffer;
		delete pBitmap;
	}

	m_mapBitmap.clear();
	m_listNonamedIndex.clear();
	m_BitmapCache.RemoveAll();

	Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	if(NULL == pBitmap)
	{
		static BITMAP_t s_Error;
		memset(&s_Error, 0, sizeof(BITMAP_t));
		strcpy(s_Error.FileName, "CGlobalBitmap::GetTexture Error!!!");
		pBitmap = &s_Error;
	}
	return pBitmap;
}
BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		if(pBitmap != NULL)
			m_BitmapCache.Add(uiBitmapIndex, pBitmap);
	}
	return pBitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		if(0 == stricmp(filename.c_str(), pBitmap->FileName))
			return pBitmap;
	}
	return NULL;
}

BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();

	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;

		//std::string texname="";

        std::string _textname = "";

		SplitFileName(pBitmap->FileName, _textname, true);

		if(0 == stricmp(_textname.c_str(), name.c_str()))
			return pBitmap;
	}
	return NULL;
}

DWORD CGlobalBitmap::GetUsedTextureMemory() const
{
	return m_dwUsedTextureMemory;
}
size_t CGlobalBitmap::GetNumberOfTexture() const
{
	return m_mapBitmap.size();
}

void CGlobalBitmap::Manage()
{
#ifdef DEBUG_BITMAP_CACHE
	m_DebugOutputTimer.UpdateTime();
	if(m_DebugOutputTimer.IsTime())
	{
		g_ConsoleDebug->Write(MCD_NORMAL, "CacheSize=%d(NumberOfTexture=%d)", m_BitmapCache.GetCacheSize(), GetNumberOfTexture());
	}
#endif // DEBUG_BITMAP_CACHE
	m_BitmapCache.Update();
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
	GLuint uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	if(uiAvailableTextureIndex >= BITMAP_NONAMED_TEXTURES_END)
	{
		m_uiAlternate++;
		m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
		uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	}
	return m_uiTextureIndexStream = uiAvailableTextureIndex;
}
GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
	if(m_uiAlternate > 0)
	{
		type_index_list::iterator li = std::find(m_listNonamedIndex.begin(), m_listNonamedIndex.end(), uiSeed+1);
		if(li != m_listNonamedIndex.end())
			return FindAvailableTextureIndex(uiSeed+1);
	}
	return uiSeed+1;
}

#ifdef MU_USE_SDL
struct jpeg_memory_source
{
	jpeg_source_mgr pub;
	const JOCTET* data;
	size_t len;
	j_boolean start_of_file;
};

static int MU_NextPowerOfTwo(int v)
{
	int r = 1;
	while (r < v)
		r <<= 1;
	return r;
}

static void jpeg_mem_init_source(j_decompress_ptr cinfo)
{
	jpeg_memory_source* src = (jpeg_memory_source*)cinfo->src;
	src->start_of_file = TRUE;
}

static j_boolean jpeg_mem_fill_input_buffer(j_decompress_ptr cinfo)
{
	static const JOCTET fake_eoi[2] = { 0xFF, JPEG_EOI };

	jpeg_memory_source* src = (jpeg_memory_source*)cinfo->src;
	src->pub.next_input_byte = fake_eoi;
	src->pub.bytes_in_buffer = 2;

	return TRUE;
}

static void jpeg_mem_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	jpeg_memory_source* src = (jpeg_memory_source*)cinfo->src;

	if (num_bytes > 0)
	{
		if ((size_t)num_bytes > src->pub.bytes_in_buffer)
		{
			src->pub.next_input_byte += src->pub.bytes_in_buffer;
			src->pub.bytes_in_buffer = 0;
		}
		else
		{
			src->pub.next_input_byte += num_bytes;
			src->pub.bytes_in_buffer -= num_bytes;
		}
	}
}

static void jpeg_mem_term_source(j_decompress_ptr)
{
}

static void jpeg_mem_src_compat(j_decompress_ptr cinfo, const unsigned char* data, size_t len)
{
	jpeg_memory_source* src;

	if (cinfo->src == NULL)
	{
		cinfo->src = (jpeg_source_mgr*)
			(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(jpeg_memory_source));
	}

	src = (jpeg_memory_source*)cinfo->src;

	src->data = data;
	src->len = len;
	src->start_of_file = TRUE;

	src->pub.init_source = jpeg_mem_init_source;
	src->pub.fill_input_buffer = jpeg_mem_fill_input_buffer;
	src->pub.skip_input_data = jpeg_mem_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = jpeg_mem_term_source;
	src->pub.bytes_in_buffer = len;
	src->pub.next_input_byte = data;
}


std::string MU_NormalizePath2(const char* path)
{
	if (!path)
		return "";

	std::string s(path);

	// replace backslashes
	for (char& c : s)
	{
		if (c == '\\')
			c = '/';
	}

	return s;
}

bool CGlobalBitmap::OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozj;
	ExchangeExt(filename, "ozj", filename_ozj);


	MU_FILE* infile = MU_fopen(filename_ozj.c_str(), "rb");

	if (infile == NULL) {
        g_ErrorReport.Write("> OpenJpeg %s, failed to open.",filename_ozj.c_str());
        return false;
    }

	MU_fseek(infile, 0, SEEK_END);
	long fileSize = MU_ftell(infile);
	MU_fseek(infile, 0, SEEK_SET);

	if (fileSize <= 24)
	{
        g_ErrorReport.Write("> OpenJpeg %s, fileSize <= 24.",filename_ozj.c_str());
        MU_fclose(infile);
		return false;
	}

	BYTE* fileBuffer = new BYTE[fileSize];
	size_t readSize = MU_fread(fileBuffer, 1, fileSize, infile);
	MU_fclose(infile);

	if (readSize != (size_t)fileSize)
	{
        g_ErrorReport.Write("> OpenJpeg %s, readSize != (size_t)fileSize.",filename_ozj.c_str());
		delete[] fileBuffer;
		return false;
	}

	BYTE* jpegData = fileBuffer + 24;          // skip OZJ dump header
	size_t jpegSize = fileSize - 24;

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
        g_ErrorReport.Write("> OpenJpeg %s, setjmp(jerr.setjmp_buffer).",filename_ozj.c_str());
		jpeg_destroy_decompress(&cinfo);
		delete[] fileBuffer;
		return false;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src_compat(&cinfo, jpegData, jpegSize);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	if (cinfo.output_width <= MAX_WIDTH && cinfo.output_height <= MAX_HEIGHT)
	{
		int Width = 1;
		int Height = 1;

		while (Width < (int)cinfo.output_width)
			Width <<= 1;

		while (Height < (int)cinfo.output_height)
			Height <<= 1;

		BITMAP_t* pNewBitmap = new BITMAP_t;
		memset(pNewBitmap, 0, sizeof(BITMAP_t));

		pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
		filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
		ndk_copy_s(MU_NormalizePath2(filename.c_str()), pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif

		pNewBitmap->Width = (float)Width;
		pNewBitmap->Height = (float)Height;
		pNewBitmap->Components = 3;
		pNewBitmap->Ref = 1;

		size_t BufferSize = Width * Height * pNewBitmap->Components;
		pNewBitmap->Buffer = new BYTE[BufferSize];
		memset(pNewBitmap->Buffer, 0, BufferSize);

		m_dwUsedTextureMemory += BufferSize;

		int row_stride = cinfo.output_width * cinfo.output_components;

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)(
			(j_common_ptr)&cinfo,
			JPOOL_IMAGE,
			row_stride,
			1
			);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			BYTE* dst = pNewBitmap->Buffer + (cinfo.output_scanline - 1) * Width * 3;
			memcpy(dst, buffer[0], row_stride);
		}

        m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

		glGenTextures(1, &(pNewBitmap->TextureNumber));
		glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		bool isPOT = ((Width & (Width - 1)) == 0) && ((Height & (Height - 1)) == 0);

		GLuint wrap = uiWrapMode;

		if (wrap == 0x2900) { // Legacy GL_CLAMP
			wrap = GL_CLAMP_TO_EDGE;
		}

		// we MUST use CLAMP_TO_EDGE even if the engine requested REPEAT.
		if (!isPOT) {
			wrap = GL_CLAMP_TO_EDGE;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	delete[] fileBuffer;

	return true;
}

bool CGlobalBitmap::OpenJpegScaledCopy(
	GLuint uiBitmapIndex,
	const std::string& filename,
	GLuint uiFilter,
	GLuint uiWrapMode,
	float scale)
{
	if (scale <= 0.0f)
		scale = 1.0f;

	std::string filename_ozj;
	ExchangeExt(filename, "ozj", filename_ozj);

	MU_FILE* infile = MU_fopen(filename_ozj.c_str(), "rb");

	if (infile == NULL)
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, failed to open.", filename_ozj.c_str());
		return false;
	}

	MU_fseek(infile, 0, SEEK_END);
	long fileSize = MU_ftell(infile);
	MU_fseek(infile, 0, SEEK_SET);

	if (fileSize <= 24)
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, fileSize <= 24.", filename_ozj.c_str());
		MU_fclose(infile);
		return false;
	}

	BYTE* fileBuffer = new BYTE[fileSize];

	size_t readSize = MU_fread(fileBuffer, 1, fileSize, infile);
	MU_fclose(infile);

	if (readSize != (size_t)fileSize)
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, readSize != fileSize.", filename_ozj.c_str());
		delete[] fileBuffer;
		return false;
	}

	BYTE* jpegData = fileBuffer + 24;
	size_t jpegSize = fileSize - 24;

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, jpeg error.", filename_ozj.c_str());
		jpeg_destroy_decompress(&cinfo);
		delete[] fileBuffer;
		return false;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src_compat(&cinfo, jpegData, jpegSize);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	if (cinfo.output_width > MAX_WIDTH || cinfo.output_height > MAX_HEIGHT)
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, size too large.", filename_ozj.c_str());

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		delete[] fileBuffer;

		return false;
	}

	int srcW = (int)cinfo.output_width;
	int srcH = (int)cinfo.output_height;
	int srcComponents = (int)cinfo.output_components;

	if (srcComponents != 3)
	{
		g_ErrorReport.Write("> OpenJpegScaledCopy %s, output_components != 3.", filename_ozj.c_str());

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		delete[] fileBuffer;

		return false;
	}

	size_t srcBufferSize = srcW * srcH * srcComponents;
	BYTE* srcBuffer = new BYTE[srcBufferSize];
	memset(srcBuffer, 0, srcBufferSize);

	int row_stride = srcW * srcComponents;

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)(
		(j_common_ptr)&cinfo,
		JPOOL_IMAGE,
		row_stride,
		1
		);

	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);

		BYTE* dst = srcBuffer + (cinfo.output_scanline - 1) * row_stride;
		memcpy(dst, buffer[0], row_stride);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	delete[] fileBuffer;

	int scaledW = (int)((float)srcW * scale + 0.5f);
	int scaledH = (int)((float)srcH * scale + 0.5f);

	if (scaledW <= 0)
		scaledW = 1;

	if (scaledH <= 0)
		scaledH = 1;

	int Width = MU_NextPowerOfTwo(scaledW);
	int Height = MU_NextPowerOfTwo(scaledH);

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));

	pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
	filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
	ndk_copy_s(MU_NormalizePath2(filename.c_str()), pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif

	pNewBitmap->Width = (float)Width;
	pNewBitmap->Height = (float)Height;
	pNewBitmap->Components = 3;
	pNewBitmap->Ref = 1;

	size_t BufferSize = Width * Height * pNewBitmap->Components;
	pNewBitmap->Buffer = new BYTE[BufferSize];
	memset(pNewBitmap->Buffer, 0, BufferSize);

	m_dwUsedTextureMemory += BufferSize;

	for (int y = 0; y < scaledH; y++)
	{
		int srcY = (int)((float)y * (float)srcH / (float)scaledH);

		BYTE* dst = pNewBitmap->Buffer + y * Width * 3;

		for (int x = 0; x < scaledW; x++)
		{
			int srcX = (int)((float)x * (float)srcW / (float)scaledW);

			BYTE* src = srcBuffer + ((srcY * srcW + srcX) * 3);

			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];

			dst += 3;
		}
	}

	delete[] srcBuffer;

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

	glGenTextures(1, &(pNewBitmap->TextureNumber));
	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		Width,
		Height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		pNewBitmap->Buffer
	);

	GLuint wrap = uiWrapMode;

	if (wrap == 0x2900)
		wrap = GL_CLAMP_TO_EDGE;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozt;
	ExchangeExt(filename, "ozt", filename_ozt);

	MU_FILE* fp = MU_fopen(filename_ozt.c_str(), "rb");

	if (fp == NULL) {
		g_ErrorReport.Write("> OpenTga %s, failed to open.", filename_ozt.c_str());
		return false;
	}

	MU_fseek(fp, 0, SEEK_END);
	int Size = (int)MU_ftell(fp);
	MU_fseek(fp, 0, SEEK_SET);

	if (Size <= 0)
	{
		g_ErrorReport.Write("> OpenTga %s, Size <= 0.",filename_ozt.c_str());
		MU_fclose(fp);
		return false;
	}

	unsigned char* PakBuffer = new unsigned char[Size];

	size_t readSize = MU_fread(PakBuffer, 1, Size, fp);
	MU_fclose(fp);

	if (readSize != (size_t)Size)
	{
		g_ErrorReport.Write("> OpenTga %s, readSize != (size_t)Size.",filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int index = 12;
	index += 4;

	if (index + 6 > Size)
	{
		g_ErrorReport.Write("> OpenTga %s, index + 6 > Size.",filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	short nx = *((short*)(PakBuffer + index)); index += 2;
	short ny = *((short*)(PakBuffer + index)); index += 2;
	char bit = *((char*)(PakBuffer + index)); index += 1;
	index += 1;

	if (bit != 32 || nx > MAX_WIDTH || ny > MAX_HEIGHT || nx <= 0 || ny <= 0)
	{
		g_ErrorReport.Write("> OpenTga %s, bit != 32 || nx > MAX_WIDTH...",filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	if (index + nx * ny * 4 > Size)
	{
		g_ErrorReport.Write("> OpenTga %s, index + nx * ny * 4 > Size",filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int Width = 1;
	int Height = 1;

	while (Width < nx)
		Width <<= 1;

	while (Height < ny)
		Height <<= 1;

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));

	pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
	filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
    ndk_copy_s(MU_NormalizePath2(filename.c_str()),pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif

	pNewBitmap->Width = (float)Width;
	pNewBitmap->Height = (float)Height;
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;

	size_t BufferSize = Width * Height * pNewBitmap->Components;
	pNewBitmap->Buffer = new BYTE[BufferSize];
	memset(pNewBitmap->Buffer, 0, BufferSize);

	m_dwUsedTextureMemory += BufferSize;

	for (int y = 0; y < ny; y++)
	{
		unsigned char* src = &PakBuffer[index];
		index += nx * 4;

		unsigned char* dst =
			&pNewBitmap->Buffer[(ny - 1 - y) * Width * pNewBitmap->Components];

		for (int x = 0; x < nx; x++)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];

			src += 4;
			dst += pNewBitmap->Components;
		}
	}

	SAFE_DELETE_ARRAY(PakBuffer);

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

	glGenTextures(1, &(pNewBitmap->TextureNumber));
	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		Width,
		Height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pNewBitmap->Buffer
	);

	GLuint wrap = uiWrapMode;
	if (wrap == 0x2900) wrap = GL_CLAMP_TO_EDGE; // Handle legacy GL_CLAMP

	bool isPOT = ((Width & (Width - 1)) == 0) && ((Height & (Height - 1)) == 0);
	if (!isPOT) {
		wrap = GL_CLAMP_TO_EDGE;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	return true;
}

bool CGlobalBitmap::OpenTgaScaledCopy(
	GLuint uiBitmapIndex,
	const std::string& filename,
	GLuint uiFilter,
	GLuint uiWrapMode,
	float scale)
{
	if (scale <= 0.0f)
		scale = 1.0f;

	std::string filename_ozt;
	ExchangeExt(filename, "ozt", filename_ozt);

	MU_FILE* fp = MU_fopen(filename_ozt.c_str(), "rb");

	g_ErrorReport.Write("> OpenTgaScaledCopy %s", filename_ozt.c_str());

	if (fp == NULL)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, failed to open.", filename_ozt.c_str());
		return false;
	}

	MU_fseek(fp, 0, SEEK_END);
	int Size = (int)MU_ftell(fp);
	MU_fseek(fp, 0, SEEK_SET);

	if (Size <= 0)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, Size <= 0.", filename_ozt.c_str());
		MU_fclose(fp);
		return false;
	}

	unsigned char* PakBuffer = new unsigned char[Size];

	size_t readSize = MU_fread(PakBuffer, 1, Size, fp);
	MU_fclose(fp);

	if (readSize != (size_t)Size)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, readSize != Size.", filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int index = 12;
	index += 4;

	if (index + 6 > Size)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, index + 6 > Size.", filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	short nx = *((short*)(PakBuffer + index)); index += 2;
	short ny = *((short*)(PakBuffer + index)); index += 2;
	char bit = *((char*)(PakBuffer + index)); index += 1;
	index += 1;

	if (bit != 32 || nx > MAX_WIDTH || ny > MAX_HEIGHT || nx <= 0 || ny <= 0)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, invalid size/bit.", filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	if (index + nx * ny * 4 > Size)
	{
		g_ErrorReport.Write("> OpenTgaScaledCopy %s, image data out of range.", filename_ozt.c_str());
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int srcDataIndex = index;

	int scaledNx = (int)((float)nx * scale + 0.5f);
	int scaledNy = (int)((float)ny * scale + 0.5f);

	if (scaledNx <= 0)
		scaledNx = 1;

	if (scaledNy <= 0)
		scaledNy = 1;

	int Width = MU_NextPowerOfTwo(scaledNx);
	int Height = MU_NextPowerOfTwo(scaledNy);

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));

	pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
	filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
	ndk_copy_s(MU_NormalizePath2(filename.c_str()), pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif

	pNewBitmap->Width = (float)Width;
	pNewBitmap->Height = (float)Height;
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;

	size_t BufferSize = Width * Height * pNewBitmap->Components;
	pNewBitmap->Buffer = new BYTE[BufferSize];
	memset(pNewBitmap->Buffer, 0, BufferSize);

	m_dwUsedTextureMemory += BufferSize;

	for (int y = 0; y < scaledNy; y++)
	{
		int srcY = (int)((float)y * (float)ny / (float)scaledNy);

		unsigned char* dst =
			&pNewBitmap->Buffer[(scaledNy - 1 - y) * Width * pNewBitmap->Components];

		for (int x = 0; x < scaledNx; x++)
		{
			int srcX = (int)((float)x * (float)nx / (float)scaledNx);

			unsigned char* src =
				&PakBuffer[srcDataIndex + ((srcY * nx + srcX) * 4)];

			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];

			dst += pNewBitmap->Components;
		}
	}

	g_ErrorReport.Write("> OpenTgaScaledCopy %s done.", filename_ozt.c_str());


	SAFE_DELETE_ARRAY(PakBuffer);

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

	glGenTextures(1, &(pNewBitmap->TextureNumber));
	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		Width,
		Height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pNewBitmap->Buffer
	);

	GLuint wrap = uiWrapMode;

	if (wrap == 0x2900) // legacy GL_CLAMP
		wrap = GL_CLAMP_TO_EDGE;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

	return true;
}

#else
bool CGlobalBitmap::OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozj;
	ExchangeExt(filename, "OZJ", filename_ozj);

	FILE* infile = fopen(filename_ozj.c_str(), "rb");
	if(infile == NULL) 
	{
		return false;
	}

	fseek(infile,24,SEEK_SET);	//. Skip Dump
	
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) 
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	if(cinfo.output_width<=MAX_WIDTH && cinfo.output_height<=MAX_HEIGHT)
	{
		int Width, Height;
		for(int i=1;i<=MAX_WIDTH;i<<=1)
		{
			Width = i;
			if(i >= (int)cinfo.output_width) break;
		}
		for(int i=1;i<=MAX_HEIGHT;i<<=1)
		{
			Height = i;
			if(i >= (int)cinfo.output_height) break;
		}

		BITMAP_t* pNewBitmap = new BITMAP_t;
		memset(pNewBitmap, 0, sizeof(BITMAP_t));

		pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
		filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
        ndk_copy_s(filename,pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif
		pNewBitmap->Width      = (float)Width;
		pNewBitmap->Height     = (float)Height;
		pNewBitmap->Components = 3;
		pNewBitmap->Ref = 1;
		
		size_t BufferSize = Width*Height*pNewBitmap->Components;
		pNewBitmap->Buffer = new BYTE[BufferSize];
		m_dwUsedTextureMemory += BufferSize;
		
		int offset=0;
		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		while (cinfo.output_scanline < cinfo.output_height) 
		{
			if(offset+row_stride > (int)BufferSize)
				break;

			(void) jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(pNewBitmap->Buffer+(cinfo.output_scanline-1)*Width*3,buffer[0],row_stride);
			offset += row_stride;
		}

		m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));
		
		glGenTextures( 1, &(pNewBitmap->TextureNumber));

		glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

        glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	std::string filename_ozt;
	ExchangeExt(filename, "OZT", filename_ozt);

	MU_FILE* fp = MU_fopen(filename_ozt.c_str(), "rb");
	if (fp == NULL)
	{
		return false;
	}

	MU_fseek(fp, 0, SEEK_END);
	int Size = MU_ftell(fp);
	MU_fseek(fp, 0, SEEK_SET);

	unsigned char* PakBuffer = new unsigned char[Size];
	MU_fread(PakBuffer, 1, Size, fp);
	MU_fclose(fp);

	int index = 12;
	index += 4;
	short nx = *((short*)(PakBuffer + index)); index += 2;
	short ny = *((short*)(PakBuffer + index)); index += 2;
	char bit = *((char*)(PakBuffer + index)); index += 1;
	index += 1;

	if (bit != 32 || nx > MAX_WIDTH || ny > MAX_HEIGHT)
	{
		SAFE_DELETE_ARRAY(PakBuffer);
		return false;
	}

	int Width = 0, Height = 0;
	for (int i = 1; i <= MAX_WIDTH; i <<= 1)
	{
		Width = i;
		if (i >= nx) break;
	}
	for (int i = 1; i <= MAX_HEIGHT; i <<= 1)
	{
		Height = i;
		if (i >= ny) break;
	}

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));

	pNewBitmap->BitmapIndex = uiBitmapIndex;

#ifdef _WIN32
    filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#else
    ndk_copy_s(filename,pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
#endif

	pNewBitmap->Width = (float)Width;
	pNewBitmap->Height = (float)Height;
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;

	size_t BufferSize = Width * Height * pNewBitmap->Components;
	pNewBitmap->Buffer = (unsigned char*)new BYTE[BufferSize];

	m_dwUsedTextureMemory += BufferSize;

	for (int y = 0; y < ny; y++)
	{
		unsigned char* src = &PakBuffer[index];
		index += nx * 4;
		unsigned char* dst = &pNewBitmap->Buffer[(ny - 1 - y) * Width * pNewBitmap->Components];

		for (int x = 0; x < nx; x++)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += pNewBitmap->Components;
		}
	}
	SAFE_DELETE_ARRAY(PakBuffer);

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));

	glGenTextures(1, &(pNewBitmap->TextureNumber));

	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);

	return true;
}
#endif
#include "wt.h"

inline void MU_splitpath2(
        const char* path,
        char* drive,
        char* dir,
        char* fname,
        char* ext)
{
    namespace fs = std::filesystem;

    if (!path)
        return;

    fs::path p(path);

    // DRIVE
    if (drive)
    {
#ifdef _WIN32
        std::string root = p.root_name().string();
        std::snprintf(drive, _MAX_DRIVE, "%s", root.c_str());
#else
        drive[0] = '\0';
#endif
    }

    // DIRECTORY
    if (dir)
    {
        std::string d = p.parent_path().string();

        // mimic old _splitpath behavior
        if (!d.empty())
        {
            char last = d[d.size() - 1];

            if (last != '/' && last != '\\')
                d += "/";
        }

        std::snprintf(dir, _MAX_DIR, "%s", d.c_str());
    }


    // FILENAME
    if (fname)
    {
        std::string f = p.stem().string();
        std::snprintf(fname, _MAX_FNAME, "%s", f.c_str());
        //g_ErrorReport.Write("> MU_splitpath2, %s", p.filename().c_str());
    }

    // EXTENSION
    if (ext)
    {
        std::string e = p.extension().string();
        std::snprintf(ext, _MAX_EXT, "%s", e.c_str());
    }



}

void CGlobalBitmap::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt) 
{
	char fname[_MAX_FNAME] = {0};
	char ext[_MAX_EXT] = {0};

    MU_splitpath2(filepath.c_str(), NULL, NULL, fname, ext);

    filename = fname;
	if(bIncludeExt)
		filename += ext;
}
void CGlobalBitmap::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot) 
{
	char fext[_MAX_EXT] = {0};
	MU_splitpath2(filepath.c_str(), NULL, NULL, NULL, fext);
	if(bIncludeDot) {
		ext = fext;
	}
	else {
		if((fext[0] == '.') && fext[1])
			ext = fext+1;
	}
}

#include <string>


void CGlobalBitmap::ExchangeExt(const std::string& in_filepath, const std::string& ext, std::string& out_filepath)
{
	// Find the last position of the directory separator
	size_t lastSlash = in_filepath.find_last_of("/\\");

	// Find the last position of the dot (extension)
	size_t lastDot = in_filepath.find_last_of('.');

	// If the dot is part of a directory name (e.g., /path.to/file), ignore it
	if (lastDot != std::string::npos && lastSlash != std::string::npos && lastDot < lastSlash) {
		lastDot = std::string::npos;
	}

	if (lastDot == std::string::npos) {
		// No extension found, just append the new one
		out_filepath = in_filepath + "." + ext;
	}
	else {
		// Strip the old extension and append the new one
		out_filepath = in_filepath.substr(0, lastDot) + "." + ext;
	}
}



bool CGlobalBitmap::Convert_Format(const unicode::t_string& filename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	::_splitpath(filename.c_str(), drive, dir, fname, ext);
	
	std::string strPath = drive; strPath += dir;
	std::string strName = fname;
	
	if(_stricmp(ext, ".jpg") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZJ";
		return Save_Image(filename, strSaveName.c_str(), 24);
	}
	else if(_stricmp(ext, ".tga") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZT";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else if(_stricmp(ext, ".bmp") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZB";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else
	{
	}

	return false;
}

bool CGlobalBitmap::Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader)
{
	MU_FILE* fp = MU_fopen(src.c_str(), "rb");
	if(fp == NULL) 
	{
		return false;
	}
	
	MU_fseek(fp, 0, SEEK_END);
	int size = MU_ftell(fp);
	MU_fseek(fp, 0, SEEK_SET);
	
	char* pTempBuf = new char[size];
	MU_fread(pTempBuf, 1, size, fp);
	MU_fclose(fp);
	
	fp = MU_fopen(dest.c_str(), "wb");
	if(fp == NULL) 
		return false;
	
	MU_fwrite(pTempBuf, 1, cDumpHeader, fp);
	MU_fwrite(pTempBuf, 1, size, fp);
	MU_fclose(fp);
	
	delete [] pTempBuf;
	
	return true;
}

void CGlobalBitmap::my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}
