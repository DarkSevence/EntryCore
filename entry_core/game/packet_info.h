#pragma once 

#include "packet.h"

struct TPacketElement 
{
	int iSize;
	std::string stName;
	int iCalled;
	uint dwLoad;
};

class CPacketInfo
{
	public:
		CPacketInfo();
		virtual ~CPacketInfo() = default;

		void Set(int header, int iSize, const char* c_pszName);
		bool Get(int header, int* size, const char** c_ppszName);

		void Start();
		void End();

		void Log(const char* c_pszFileName);
		
	private:
		TPacketElement* GetElement(int header);
		std::unordered_map<int, std::unique_ptr<TPacketElement>> m_pPacketMap;

	protected:
		TPacketElement* m_pCurrentPacket;
		DWORD m_dwStartTime;
};

class CPacketInfoCG : public CPacketInfo
{
	public:
		CPacketInfoCG();
		virtual ~CPacketInfoCG();
};

class CPacketInfoGG : public CPacketInfo
{
	public:
		CPacketInfoGG();
		virtual ~CPacketInfoGG();
};