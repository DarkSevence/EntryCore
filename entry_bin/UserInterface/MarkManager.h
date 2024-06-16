#ifndef METIN2_CLIENT_RUN_MARKMANAGER_HPP
#define METIN2_CLIENT_RUN_MARKMANAGER_HPP

#include "MarkImage.h"
#include <unordered_set>

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
        uint32_t crc;
        std::vector<uint8_t> raw;
    };

    CGuildMarkManager();
    ~CGuildMarkManager();

    const TGuildSymbol* GetGuildSymbol(uint32_t GID);
    bool LoadSymbol(const char* filename);
    void SaveSymbol(const char* filename);
    void UploadSymbol(uint32_t guildID, int iSize, const uint8_t* pbyData);
    //
    // Mark
    //
    void SetMarkPathPrefix(const char* prefix);

    bool LoadMarkIndex(); // ?? ??? ???? (????? ??)
    bool SaveMarkIndex(); // ?? ??? ????

    void LoadMarkImages();               // ?? ?? ???? ????
    void SaveMarkImage(uint32_t imgIdx); // ?? ??? ??

    bool GetMarkImageFilename(uint32_t imgIdx, std::string& path) const;
    bool AddMarkIDByGuildID(uint32_t guildID, uint32_t markID);
    uint32_t GetMarkImageCount() const;
    uint32_t GetMarkCount() const;
    uint32_t GetMarkID(uint32_t guildID);

    bool SaveBlockFromCompressedData(uint32_t imgIdx, uint32_t idBlock, const uint8_t* pbBlock, uint32_t dwSize);
    bool GetBlockCRCList(uint32_t imgIdx, uint32_t* crcList);
    void ReloadGuildMarkImage(uint32_t imgIdx);

private:
    //
    // Mark
    //

    uint32_t __AllocMarkID(uint32_t guildID);

    CGuildMarkImage* __GetImage(uint32_t imgIdx);
    CGuildMarkImage* __GetImagePtr(uint32_t idMark);

    // index = image index
    std::unordered_map<uint32_t, std::unique_ptr<CGuildMarkImage>> m_mapIdx_Image;

    // index = guild id
    std::unordered_map<uint32_t, uint32_t> m_mapGID_MarkID;

    std::unordered_set<uint32_t> m_setFreeMarkID;
    std::string m_pathPrefix;

    std::unordered_map<uint32_t, TGuildSymbol> m_mapSymbol;
};
#endif