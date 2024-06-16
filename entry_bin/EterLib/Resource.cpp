#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Timer.h"

#include "Resource.h"
#include "ResourceManager.h"

bool CResource::ms_bDeleteImmediately = false;

CResource::CResource(const char* c_szFileName) : me_state(STATE_EMPTY)
{
	SetFileName(c_szFileName);
	Tracef("CResource::CResource Created for file: %s\n", c_szFileName);  // DEBUG
}

CResource::~CResource()
{
	Tracef("CResource::~CResource Destroyed for file: %s\n", GetFileName());  // DEBUG
}

void CResource::SetDeleteImmediately(bool isSet)
{
	ms_bDeleteImmediately = isSet;
}

void CResource::OnConstruct()
{
	Load();
}

void CResource::OnSelfDestruct()
{	
	if (ms_bDeleteImmediately)
		Clear();
	else
		CResourceManager::Instance().ReserveDeletingResource(this);
}

void CResource::Load()
{
	if (me_state != STATE_EMPTY)
		return;

	const char* c_szFileName = GetFileName();

	DWORD dwStart = ELTimer_GetMSec();
	CMappedFile file;
	LPCVOID fileData;

	if (CEterPackManager::Instance().Get(file, c_szFileName, &fileData))
	{
		m_dwLoadCostMiliiSecond = ELTimer_GetMSec() - dwStart;

		if (OnLoad(file.Size(), fileData))
		{
			me_state = STATE_EXIST;
		}
		else
		{
			//Tracef("ERROR: CResource::Load Error %s\n", c_szFileName);  // ERROR
			me_state = STATE_ERROR;
			return;
		}
	}
	else
	{
		//TraceError("ERROR: CResource::Load file not exist %s", c_szFileName);  // ERROR
		if (OnLoad(0, NULL))
			me_state = STATE_EXIST;
		else
		{
			me_state = STATE_ERROR;
		}
	}
}

void CResource::Reload()
{
	Clear();
	TraceError("CResource::Reload %s - Start\n", GetFileName());  // DEBUG

	CMappedFile file;
	LPCVOID fileData;

	if (CEterPackManager::Instance().Get(file, GetFileName(), &fileData))
	{
		TraceError("CResource::Reload %s - File found in EterPack\n", GetFileName());  // DEBUG

		if (OnLoad(file.Size(), fileData))
		{
			me_state = STATE_EXIST;
			TraceError("CResource::Reload %s - Load successful\n", GetFileName());  // DEBUG
		}
		else
		{
			me_state = STATE_ERROR;
			TraceError("CResource::Reload %s - Load failed\n", GetFileName());  // DEBUG
			return;
		}
	}
	else
	{
		TraceError("CResource::Reload %s - File not found in EterPack\n", GetFileName());  // DEBUG

		if (OnLoad(0, NULL))
		{
			me_state = STATE_EXIST;
			TraceError("CResource::Reload %s - Load with empty data successful\n", GetFileName());  // DEBUG
		}
		else
		{
			me_state = STATE_ERROR;
			TraceError("CResource::Reload %s - Load with empty data failed\n", GetFileName());  // DEBUG
		}
	}

	TraceError("CResource::Reload %s - End\n", GetFileName());  // DEBUG
}


CResource::TType CResource::StringToType(const char* c_szType)
{
	return GetCRC32(c_szType, strlen(c_szType));
}

int CResource::ConvertPathName(const char * c_szPathName, char * pszRetPathName, int retLen)
{
	const char * pc;
	int len = 0;

	for (pc = c_szPathName; *pc && len < retLen; ++pc, ++len)
	{
		if (*pc == '/')
			*(pszRetPathName++) = '\\';
		else
			*(pszRetPathName++) = (char) korean_tolower(*pc);
	}

	*pszRetPathName = '\0';
	return len;
}

void CResource::SetFileName(const char* c_szFileName)
{
	// 2004. 2. 1. myevan. 쓰레드가 사용되는 상황에서 static 변수는 사용하지 않는것이 좋다.
	// 2004. 2. 1. myevan. 파일 이름 처리를 std::string 사용
	m_stFileName=c_szFileName;
}

void CResource::Clear()
{
	OnClear();
	me_state = STATE_EMPTY;
}

bool CResource::IsType(TType type)
{
	return OnIsType(type);
}

CResource::TType CResource::Type()
{
	static TType s_type = StringToType("CResource");
	return s_type;
}

bool CResource::OnIsType(TType type)
{
	if (CResource::Type() == type)
		return true;
	
	return false;
}

bool CResource::IsData() const
{
	return me_state != STATE_EMPTY;
}

bool CResource::IsEmpty() const
{
	return OnIsEmpty();
}

bool CResource::CreateDeviceObjects()
{
	return true;
}

void CResource::DestroyDeviceObjects()
{
}
