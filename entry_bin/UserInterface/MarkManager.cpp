#include "stdafx.h"
#include "MarkManager.h"

#include <iterator>

CGuildMarkManager::CGuildMarkManager()
{
    CreateDirectoryA("mark", NULL);

    // ?? mark id ?? ???. (???)
    for (uint32_t i = 0; i < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT; ++i)
        m_setFreeMarkID.insert(i);
}

CGuildMarkManager::~CGuildMarkManager()
{
    m_mapIdx_Image.clear();
}

bool CGuildMarkManager::GetMarkImageFilename(uint32_t imgIdx, std::string& path) const
{
    if (imgIdx >= MAX_IMAGE_COUNT)
        return false;

    char buf[256];
    snprintf(buf, sizeof(buf), "mark/%s_%lu.tga", m_pathPrefix.c_str(), imgIdx);

    path = buf;
    return true;
}

void CGuildMarkManager::SetMarkPathPrefix(const char* prefix)
{
    m_pathPrefix = prefix;
}

// ?? ??? ???? (????? ??)
bool CGuildMarkManager::LoadMarkIndex()
{
    TraceError("LoadMarkIndex: Loading mark index");

    char buf[256];
    snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());

    FILE* fp = fopen(buf, "r");
    if (!fp) {
        TraceError("LoadMarkIndex: Failed to open mark index file");
        return false;
    }

    uint32_t guildID;
    uint32_t markID;

    char line[256];
    while (fgets(line, sizeof(line) - 1, fp))
    {
        sscanf(line, "%lu %lu", &guildID, &markID);
        line[0] = '\0';

        TraceError("LoadMarkIndex: Read line guildID {0}, markID {1}", guildID, markID);
        AddMarkIDByGuildID(guildID, markID);
    }

    LoadMarkImages();

    fclose(fp);
    TraceError("LoadMarkIndex: Successfully loaded mark index");
    return true;
}

bool CGuildMarkManager::SaveMarkIndex()
{
    TraceError("SaveMarkIndex: Saving mark index");

    char buf[256];
    snprintf(buf, sizeof(buf), "mark/%s_index", m_pathPrefix.c_str());

    FILE* fp = fopen(buf, "w");
    if (!fp)
    {
        TraceError("SaveMarkIndex: Cannot open index file.");
        return false;
    }

    for (auto& it : m_mapGID_MarkID) {
        fprintf(fp, "%d %d\n", it.first, it.second);
        TraceError("SaveMarkIndex: Writing guildID {0}, markID {1} to file", it.first, it.second);
    }

    fclose(fp);
    TraceError("SaveMarkIndex: Successfully saved mark index, count {0}", m_mapGID_MarkID.size());
    return true;
}

void CGuildMarkManager::LoadMarkImages()
{
    bool isMarkExists[MAX_IMAGE_COUNT];
    memset(isMarkExists, 0, sizeof(isMarkExists));

    for (auto& it : m_mapGID_MarkID)
    {
        uint32_t markID = it.second;

        if (markID < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
            isMarkExists[markID / CGuildMarkImage::MARK_TOTAL_COUNT] = true;
    }

    for (uint32_t i = 0; i < MAX_IMAGE_COUNT; ++i)
        if (isMarkExists[i])
            __GetImage(i);
}

void CGuildMarkManager::SaveMarkImage(uint32_t imgIdx)
{
    std::string path;

    if (GetMarkImageFilename(imgIdx, path))
        if (!__GetImage(imgIdx)->Save(path.c_str()))
            TraceError("{0} Save failed", path.c_str());
}

CGuildMarkImage* CGuildMarkManager::__GetImage(uint32_t imgIdx)
{
    const auto it = m_mapIdx_Image.find(imgIdx);
    if (it != m_mapIdx_Image.end())
        return it->second.get();

    if (std::string imagePath; GetMarkImageFilename(imgIdx, imagePath))
    {
        auto&& [val, inserted] = m_mapIdx_Image.emplace(imgIdx, std::make_unique<CGuildMarkImage>());

        val->second->Load(imagePath.c_str());
        return val->second.get();
    }

    return nullptr;
}

bool CGuildMarkManager::AddMarkIDByGuildID(uint32_t guildID, uint32_t markID)
{
    if (markID >= MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
        return false;

    m_mapGID_MarkID.emplace(guildID, markID);
    m_setFreeMarkID.erase(markID);
    return true;
}

uint32_t CGuildMarkManager::GetMarkID(uint32_t guildID)
{
    const auto it = m_mapGID_MarkID.find(guildID);
    if (it == m_mapGID_MarkID.end())
        return INVALID_MARK_ID;

    return it->second;
}

uint32_t CGuildMarkManager::__AllocMarkID(uint32_t guildID)
{
    const auto it = m_setFreeMarkID.lower_bound(0);
    if (it == m_setFreeMarkID.end())
        return INVALID_MARK_ID;

    uint32_t markID = *it;

    uint32_t imgIdx = markID / CGuildMarkImage::MARK_TOTAL_COUNT;
    CGuildMarkImage* pkImage = __GetImage(imgIdx); // ???? ??? ??? ??

    if (pkImage && AddMarkIDByGuildID(guildID, markID))
        return markID;

    return INVALID_MARK_ID;
}

uint32_t CGuildMarkManager::GetMarkImageCount() const
{
    return m_mapIdx_Image.size();
}

uint32_t CGuildMarkManager::GetMarkCount() const
{
    return m_mapGID_MarkID.size();
}

bool CGuildMarkManager::SaveBlockFromCompressedData(uint32_t imgIdx, uint32_t posBlock, const uint8_t* pbBlock,
    uint32_t dwSize)
{
    CGuildMarkImage* pkImage = __GetImage(imgIdx);
    if (pkImage)
        pkImage->SaveBlockFromCompressedData(posBlock, pbBlock, dwSize);

    return false;
}

bool CGuildMarkManager::GetBlockCRCList(uint32_t imgIdx, uint32_t* crcList)
{
    // ??????? ??? ?? ???? ??? ?? ??.
    if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
        return false;

    CGuildMarkImage* p = __GetImage(imgIdx);
    if (p)
        p->GetBlockCRCList(crcList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// Symbol
///////////////////////////////////////////////////////////////////////////////////////
const CGuildMarkManager::TGuildSymbol* CGuildMarkManager::GetGuildSymbol(uint32_t guildID)
{
    const auto it = m_mapSymbol.find(guildID);
    if (it == m_mapSymbol.end())
        return NULL;

    return &it->second;
}

bool CGuildMarkManager::LoadSymbol(const char* filename)
{
    FILE* fp = fopen(filename, "rb");

    if (!fp)
        return true;
    else
    {
        uint32_t symbolCount;
        fread(&symbolCount, 4, 1, fp);

        for (uint32_t i = 0; i < symbolCount; i++)
        {
            uint32_t guildID;
            uint32_t dwSize;
            fread(&guildID, 4, 1, fp);
            fread(&dwSize, 4, 1, fp);

            TGuildSymbol gs;
            gs.raw.resize(dwSize);
            fread(&gs.raw[0], 1, dwSize, fp);
            gs.crc = GetCRC32(reinterpret_cast<const char*>(&gs.raw[0]), dwSize);
            m_mapSymbol.emplace(guildID, gs);
        }
    }

    fclose(fp);
    return true;
}

void CGuildMarkManager::SaveSymbol(const char* filename)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp)
    {
        TraceError("Cannot open Symbol file (name: {0})", filename);
        return;
    }

    uint32_t symbolCount = m_mapSymbol.size();
    fwrite(&symbolCount, 4, 1, fp);

    for (auto it = m_mapSymbol.begin(); it != m_mapSymbol.end(); ++it)
    {
        uint32_t guildID = it->first;
        uint32_t dwSize = it->second.raw.size();
        fwrite(&guildID, 4, 1, fp);
        fwrite(&dwSize, 4, 1, fp);
        fwrite(&it->second.raw[0], 1, dwSize, fp);
    }

    fclose(fp);
}

void CGuildMarkManager::UploadSymbol(uint32_t guildID, int iSize, const uint8_t* pbyData)
{
    TraceError("GuildSymbolUpload: Uploading symbol for guildID {0} with size {1}", guildID, iSize);

    if (m_mapSymbol.find(guildID) == m_mapSymbol.end())
        m_mapSymbol.emplace(guildID, TGuildSymbol());

    auto& rSymbol = m_mapSymbol[guildID];
    rSymbol.raw.clear();

    if (iSize > 0)
    {
        rSymbol.raw.reserve(iSize);
        std::copy(pbyData, (pbyData + iSize), std::back_inserter(rSymbol.raw));
        rSymbol.crc = GetCRC32(reinterpret_cast<const char*>(pbyData), iSize);
    }
    TraceError("GuildSymbolUpload: Successfully uploaded symbol for guildID {0}", guildID);
}

void CGuildMarkManager::ReloadGuildMarkImage(uint32_t imgIdx)
{
    std::string imagePath;
    if (GetMarkImageFilename(imgIdx, imagePath))
    {
        CResource* pResource = CResourceManager::Instance().GetResourcePointer(imagePath.c_str());
        if (pResource && pResource->IsType(CGraphicImage::Type()))
        {
            CGraphicImage* pkGrpImg = static_cast<CGraphicImage*>(pResource);
            pkGrpImg->Reload();
            TraceError("CGuildMarkManager::ReloadGuildMarkImage - Image reloaded successfully [%s]", imagePath.c_str());
        }
        else
        {
            TraceError("CGuildMarkManager::ReloadGuildMarkImage - Resource not found or not a CGraphicImage [%s]", imagePath.c_str());
        }
    }
    else
    {
        TraceError("CGuildMarkManager::ReloadGuildMarkImage - Failed to get image filename for index [%d]", imgIdx);
    }
}
