#ifndef __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__
#define __INC_METIN_II_GUILDLIB_MARK_MANAGER_H__

#include "MarkImage.h"
#include <unordered_map>
#include <set>
#include <vector>
class CGuildMarkManager : public singleton<CGuildMarkManager>
{
	public:
		enum
		{
			MAX_IMAGE_COUNT = 5,
			INVALID_MARK_ID = 0xffffffff,
		};

		// Symbol
		struct TGuildSymbol
		{
			DWORD crc;
			std::vector<BYTE> raw;
		};

		CGuildMarkManager();
		virtual ~CGuildMarkManager();

		const TGuildSymbol * GetGuildSymbol(DWORD GID);
		bool LoadSymbol(const char* filename);
		void SaveSymbol(const char* filename);
		void UploadSymbol(DWORD guildID, int iSize, const BYTE* pbyData);

		//
		// Mark
		//
		void SetMarkPathPrefix(const char * prefix);

		bool LoadMarkIndex(); // 마크 인덱스 불러오기 (서버에서만 사용)
		bool SaveMarkIndex(); // 마크 인덱스 저장하기

		void LoadMarkImages(); // 모든 마크 이미지를 불러오기
		void SaveMarkImage(DWORD imgIdx); // 마크 이미지 저장

		bool GetMarkImageFilename(DWORD imgIdx, std::string & path) const;
		bool AddMarkIDByGuildID(DWORD guildID, DWORD markID);
		DWORD GetMarkImageCount() const;
		DWORD GetMarkCount() const;
		DWORD GetMarkID(DWORD guildID);

		// SERVER
		void CopyMarkIdx(char * pcBuf) const;
    DWORD SaveMark(LPCHARACTER pkChrCauser, DWORD guildID, BYTE * pbMarkImage);
    void UpdatePlayerMarkCooldown(DWORD playerID);
    bool IsPlayerMarkCooldownActive(DWORD playerID);
    void SendCooldownMessage(LPCHARACTER pkChrCauser);
		void DeleteMark(DWORD guildID);
		void GetDiffBlocks(uint32_t imgIdx, const uint32_t *crcList, std::unordered_map<uint8_t, const SGuildMarkBlock *> &mapDiffBlocks);

		// CLIENT
		bool SaveBlockFromCompressedData(DWORD imgIdx, DWORD idBlock, const BYTE * pbBlock, DWORD dwSize);
		bool GetBlockCRCList(DWORD imgIdx, DWORD * crcList);

	private:
		// 
		// Mark
		//
		CGuildMarkImage * __NewImage();
		void __DeleteImage(CGuildMarkImage * pkImgDel);

		DWORD __AllocMarkID(DWORD guildID);

		CGuildMarkImage * __GetImage(DWORD imgIdx);
		CGuildMarkImage * __GetImagePtr(DWORD idMark);

		std::map<DWORD, CGuildMarkImage *> m_mapIdx_Image; // index = image index
		std::map<DWORD, DWORD> m_mapGID_MarkID; // index = guild id

		std::set<DWORD> m_setFreeMarkID;
		std::string		m_pathPrefix;

	private:
		//
		// Symbol
		//
		std::map<DWORD, TGuildSymbol> m_mapSymbol;
		    std::unordered_map<DWORD, time_t> m_playerMarkCooldowns; // Mapa do ?ledzenia cooldownow dla graczy

};

#endif
