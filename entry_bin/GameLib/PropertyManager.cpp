#include "StdAfx.h"
#include "../eterPack/EterPackManager.h"

#include "PropertyManager.h"
#include "Property.h"

CPropertyManager::CPropertyManager() : m_isFileMode(true)
{
}

CPropertyManager::~CPropertyManager()
{
	Clear();
}

bool CPropertyManager::Initialize(const char* c_pszPackFileName)
{
	if (c_pszPackFileName)
	{
		if (!m_pack.Create(m_fileDict, c_pszPackFileName, "", true))
		{
			TraceError("Cannot open property pac file: %s", c_pszPackFileName);
			return false;
		}
		m_isFileMode = false;

		TDataPositionMap& indexMap = m_pack.GetIndexMap();
		TDataPositionMap::iterator itor = indexMap.begin();

		int i = 0;
		while (indexMap.end() != itor)
		{
			TEterPackIndex* pIndex = itor->second;
			++itor;

			if (!stricmp("property/reserve", pIndex->filename))
			{
				LoadReservedCRC(pIndex->filename);
				continue;
			}

			if (!Register(pIndex->filename))
				continue;

			++i;
		}
	}
	else
	{
		m_isFileMode = true;
	}

	return true;
}

bool CPropertyManager::BuildPack()
{
	if (!m_pack.Create(m_fileDict, "property", ""))
		return false;

	WIN32_FIND_DATA fdata;
	HANDLE hFind = FindFirstFile("property\\*", &fdata);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		char szSourceFileName[256 + 1];
		_snprintf(szSourceFileName, sizeof(szSourceFileName), "property\\%s", fdata.cFileName);

		m_pack.Put(fdata.cFileName, szSourceFileName,COMPRESSED_TYPE_NONE,"");
	}
	while (FindNextFile(hFind, &fdata));

	FindClose(hFind);
	return true;
}

bool CPropertyManager::LoadReservedCRC(const char * c_pszFileName)
{
	CMappedFile file;
	LPCVOID c_pvData;

	if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
		return false;

	//TraceError("[INFO] �adowanie zarezerwowanych CRC z pliku: %s", c_pszFileName);
	
	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(file.Size(), c_pvData);

	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		const char * pszLine = textFileLoader.GetLineString(i).c_str();

		if (!pszLine || !*pszLine)
			continue;

		ReserveCRC(atoi(pszLine));
	}

	return true;
}

void CPropertyManager::ReserveCRC(DWORD dwCRC)
{
	m_ReservedCRCSet.insert(dwCRC);
}

DWORD CPropertyManager::GetUniqueCRC(const char * c_szSeed)
{
	std::string stTmp = c_szSeed;

	while (1)
	{
		DWORD dwCRC = GetCRC32(stTmp.c_str(), stTmp.length());

		if (m_ReservedCRCSet.find(dwCRC) == m_ReservedCRCSet.end() &&
			m_PropertyByCRCMap.find(dwCRC) == m_PropertyByCRCMap.end())
			return dwCRC;

		char szAdd[2];
		_snprintf(szAdd, sizeof(szAdd), "%d", random() % 10);
		stTmp += szAdd;
	}
}
bool CPropertyManager::Register(const char* c_pszFileName, CProperty** ppProperty)
{
	CMappedFile file;
	LPCVOID c_pvData;

	if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
	{
		//TraceError("[ERROR] Nie mo�na za�adowa� property z pliku: %s", c_pszFileName);
		return false;
	}

	CProperty* pProperty = new CProperty(c_pszFileName);

	if (!pProperty->ReadFromMemory(c_pvData, file.Size(), c_pszFileName))
	{
		//TraceError("[ERROR] Nie uda�o si� przeczyta� property z pami�ci dla pliku: %s", c_pszFileName);
		delete pProperty;
		return false;
	}


	DWORD dwCRC = pProperty->GetCRC();
	RegisterCRCtoFileName(dwCRC, c_pszFileName);


	TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);
	if (m_PropertyByCRCMap.end() != itor)
	{
		Tracef("[WARN] Property ju� zarejestrowane, zast�powanie %s przez %s",
			itor->second->GetFileName(), c_pszFileName);

		delete itor->second;
		itor->second = pProperty;
	}
	else
	{
		m_PropertyByCRCMap.insert(TPropertyCRCMap::value_type(dwCRC, pProperty));
	}

	if (ppProperty)
		*ppProperty = pProperty;

	return true;
}


bool CPropertyManager::Get(const char * c_pszFileName, CProperty ** ppProperty)
{
	return Register(c_pszFileName, ppProperty);
}

bool CPropertyManager::Get(DWORD dwCRC, CProperty ** ppProperty)
{
	TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() == itor)
		return false;

	*ppProperty = itor->second;
	return true;
}

bool CPropertyManager::Put(const char * c_pszFileName, const char * c_pszSourceFileName)
{
	if (!CopyFile(c_pszSourceFileName, c_pszFileName, FALSE))
		return false;

	if (!m_isFileMode)	// �� ���Ͽ��� ����
	{
		if (!m_pack.Put(c_pszFileName, NULL, COMPRESSED_TYPE_NONE,""))
		{
			assert(!"CPropertyManager::Put cannot write to pack file");
			return false;
		}
	}

	Register(c_pszFileName);
	return true;
}

bool CPropertyManager::Erase(DWORD dwCRC)
{
	TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() == itor)
		return false;

	CProperty * pProperty = itor->second;
	m_PropertyByCRCMap.erase(itor);

	DeleteFile(pProperty->GetFileName());
	ReserveCRC(pProperty->GetCRC());

	if (!m_isFileMode)	// ���� ��尡 �ƴϸ� �ѿ����� ����
		m_pack.Delete(pProperty->GetFileName());

	FILE * fp = fopen("property/reserve", "a+");

	if (!fp)
		LogBox("���� CRC ������ �� �� �����ϴ�.");
	else
	{
		char szCRC[64 + 1];
		_snprintf(szCRC, sizeof(szCRC), "%u\r\n", pProperty->GetCRC());

		fputs(szCRC, fp);
		fclose(fp);
	}

	delete pProperty;
	return true;
}

bool CPropertyManager::Erase(const char * c_pszFileName)
{
	CProperty * pProperty = NULL;

	if (Get(c_pszFileName, &pProperty))
		return Erase(pProperty->GetCRC());
	
	return false;
}

void CPropertyManager::Clear()
{
	for (auto itor = m_PropertyByCRCMap.begin(); itor != m_PropertyByCRCMap.end(); ++itor)
	{
		delete itor->second;
	}
	m_PropertyByCRCMap.clear();
}