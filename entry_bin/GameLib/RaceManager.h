#pragma once

#include "RaceData.h"
#include <array>

class CRaceManager : public CSingleton<CRaceManager>
{
	public:
		typedef std::map<DWORD, CRaceData *> TRaceDataMap;
		typedef TRaceDataMap::iterator TRaceDataIterator;

		static constexpr uint32_t MIN_PET_NPC_RACE = 34001;
		static constexpr uint32_t MAX_PET_NPC_RACE = 34099;
		static constexpr uint32_t MIN_MOUNT_NPC_RACE = 20110;
		static constexpr uint32_t MAX_MOUNT_NPC_RACE = 20220;
		static constexpr uint32_t GUILD_RACE_MIN = 14000;
		static constexpr uint32_t GUILD_RACE_MAX = 14999;
		static constexpr uint32_t SPECIAL_GUILD_RACE = 20043;
		static constexpr uint32_t NPC_RACE_START_RANGE = 9000;
		static constexpr uint32_t MONSTER_RACE_MIN = 101;
		static constexpr uint32_t MONSTER_RACE_MAX = 1999;
		static constexpr uint32_t MONSTER_RACE_METIN_MIN = 8001;
		static constexpr uint32_t MONSTER_RACE_METIN_MAX = 8500;
		static constexpr uint32_t MONSTER2_RACE_MIN = 1400;
		static constexpr uint32_t MONSTER2_RACE_MAX = 1700;
		static constexpr uint32_t MONSTER2_RACE_SPECIAL1 = 8507;
		static constexpr uint32_t MONSTER2_RACE_SPECIAL2 = 8510;
		static constexpr uint32_t MONSTER2_RACE_START_RANGE = 2000;

		static constexpr std::string_view NPC_PET_PATH = "d:/ymir work/npc_pet/";
		static constexpr std::string_view NPC_MOUNT_PATH = "d:/ymir work/npc_mount/";
		static constexpr std::string_view NPC_PATH = "d:/ymir work/npc/";
		static constexpr std::string_view MONSTER_PATH = "d:/ymir work/monster/";
		static constexpr std::string_view MONSTER2_PATH = "d:/ymir work/monster2/";
		static constexpr std::string_view GUILD_PATH = "d:/ymir work/guild/";

	public:
		CRaceManager();
		virtual ~CRaceManager();

		void Create();
		void Destroy();
		
		void RegisterRaceName(DWORD dwRaceIndex, const char * c_szName);
		void RegisterRaceSrcName(const char * c_szName, const char * c_szSrcName);

		void SetRaceHeight(int iVnum, float fHeight);
		float GetRaceHeight(int iVnum);

		void SetPathName(const char * c_szPathName);
		const char * GetFullPathFileName(const char* c_szFileName);

		// Handling
		void CreateRace(DWORD dwRaceIndex);
		void SelectRace(DWORD dwRaceIndex);
		CRaceData * GetSelectedRaceDataPointer();
		// Handling

		BOOL GetRaceDataPointer(DWORD dwRaceIndex, CRaceData ** ppRaceData);


	protected:
		CRaceData* __LoadRaceData(DWORD dwRaceIndex);
		bool __LoadRaceMotionList(CRaceData& rkRaceData, const char* pathName, const char* motionListFileName);

		void __Initialize();
		void __DestroyRaceDataMap();

	protected:
		TRaceDataMap					m_RaceDataMap;
		std::map<std::string, std::string> m_kMap_stRaceName_stSrcName;
		std::map<DWORD, std::string>	m_kMap_dwRaceKey_stRaceName;
		std::map<int, float> m_kMap_iRaceKey_fRaceAdditionalHeight;


	private:
		std::string						m_strPathName;
		CRaceData *						m_pSelectedRaceData;
};