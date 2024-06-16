#include "stdafx.h"
#include "MarkImage.h"

CGuildMarkImage::CGuildMarkImage() : m_aakBlock{}, m_apxImage{}, m_uImg(INVALID_HANDLE)
{
}

CGuildMarkImage::~CGuildMarkImage()
{
	Destroy();
}

void CGuildMarkImage::Destroy()
{
	if (m_uImg == INVALID_HANDLE)
	{
		return;
	}

	ilDeleteImages(1, &m_uImg);
	m_uImg = INVALID_HANDLE;
}

void CGuildMarkImage::Create()
{
	if (m_uImg != INVALID_HANDLE)
	{
		return;
	}

	ilGenImages(1, &m_uImg);
}

bool CGuildMarkImage::Save(const char* c_szFileName) const
{
	ilEnable(IL_FILE_OVERWRITE);
	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	if (!ilSave(IL_TGA, static_cast<const ILstring>(c_szFileName)))
	{
		return false;
	}

	return true;
}

bool CGuildMarkImage::Build(const char* c_szFileName)
{
	Destroy();
	Create();

	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	std::vector<uint8_t> data(sizeof(Pixel) * WIDTH * HEIGHT, 0);

	if (!ilTexImage(WIDTH, HEIGHT, 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, data.data()))
	{
		TraceError("CGuildMarkImage: cannot initialize image");
		return false;
	}

	ilEnable(IL_FILE_OVERWRITE);

	if (!ilSave(IL_TGA, static_cast<const ILstring>(c_szFileName)))
	{
		return false;
	}

	return true;
}

bool CGuildMarkImage::Load(const char* c_szFileName) 
{
	Destroy();
	Create();	

	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	if (!ilLoad(IL_TGA, static_cast<const ILstring>(c_szFileName)))
	{
		Build(c_szFileName);
		ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
		
		if (!Load(c_szFileName))
		{
			TraceError("CGuildMarkImage: cannot open file for writing {0}", c_szFileName);
			return false;
		}
	}

	if (ilGetInteger(IL_IMAGE_WIDTH) != WIDTH)	
	{
		return false;
	}

	if (ilGetInteger(IL_IMAGE_HEIGHT) != HEIGHT)
	{
		return false;
	}

	ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);

	BuildAllBlocks();
	return true;
}

void CGuildMarkImage::PutData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void* data) const
{
	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilSetPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);
}

void CGuildMarkImage::GetData(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void* data) const
{
	ilBindImage(m_uImg);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilCopyPixels(x, y, 0, width, height, 1, IL_BGRA, IL_UNSIGNED_BYTE, data);
}

bool CGuildMarkImage::SaveBlockFromCompressedData(uint32_t posBlock, const uint8_t* pbComp, uint32_t dwCompSize)
{
	if (posBlock >= BLOCK_TOTAL_COUNT)
	{
		return false;
	}

	Pixel apxBuf[SGuildMarkBlock::SIZE] = {};
	size_t sizeBuf = sizeof(apxBuf);

	if (LZO_E_OK != lzo1x_decompress_safe(pbComp, dwCompSize, (BYTE *) apxBuf, (lzo_uint*) &sizeBuf, CLZO::Instance().GetWorkMemory()))
	{
		TraceError("CGuildMarkImage::CopyBlockFromCompressedData: cannot decompress, compressed size = %u", dwCompSize);
		return false;
	}

	if (sizeBuf != sizeof(apxBuf))
	{
		TraceError("CGuildMarkImage::CopyBlockFromCompressedData: image corrupted, decompressed size = %u", sizeBuf);
		return false;
	}

	uint32_t rowBlock = posBlock / BLOCK_COL_COUNT;
	uint32_t colBlock = posBlock % BLOCK_COL_COUNT;

	PutData(colBlock * SGuildMarkBlock::WIDTH, rowBlock * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);

	m_aakBlock[rowBlock][colBlock].CopyFrom(pbComp, dwCompSize, GetCRC32(reinterpret_cast<const char*>(apxBuf), sizeof(Pixel) * SGuildMarkBlock::SIZE));
	return true;
}

void CGuildMarkImage::BuildAllBlocks()
{
	Pixel apxBuf[SGuildMarkBlock::SIZE];

	for (uint32_t row = 0; row < BLOCK_ROW_COUNT; ++row)
	{
		for (uint32_t col = 0; col < BLOCK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMarkBlock::WIDTH, row * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);
			m_aakBlock[row][col].Compress(apxBuf);
		}
	}
}

uint32_t CGuildMarkImage::GetEmptyPosition() const
{
	SGuildMark kMark{};

	for (uint32_t row = 0; row < MARK_ROW_COUNT; ++row)
	{
		for (uint32_t col = 0; col < MARK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMark::WIDTH, row * SGuildMark::HEIGHT, SGuildMark::WIDTH, SGuildMark::HEIGHT, kMark.m_apxBuf);

			if (kMark.IsEmpty())
			{
				return (row * MARK_COL_COUNT + col);
			}
		}
	}

	return INVALID_MARK_POSITION;
}

void CGuildMarkImage::GetBlockCRCList(uint32_t* crcList)
{
	for (uint32_t row = 0; row < BLOCK_ROW_COUNT; ++row)
	{
		for (uint32_t col = 0; col < BLOCK_COL_COUNT; ++col)
		{
			*(crcList++) = m_aakBlock[row][col].GetCRC();
		}
	}
}

void SGuildMark::Clear()
{
	for (uint32_t iPixel = 0; iPixel < SIZE; ++iPixel)
	{
		m_apxBuf[iPixel] = 0xff000000ul;
	}
}

bool SGuildMark::IsEmpty() const
{
	for (uint32_t iPixel = 0; iPixel < SIZE; ++iPixel)
	{
		if (m_apxBuf[iPixel] != 0x00000000ul)
		{
			return false;
		}
	}

	return true;
}

uint32_t SGuildMarkBlock::GetCRC() const
{
	return m_crc;
}

void SGuildMarkBlock::CopyFrom(const uint8_t* pbCompBuf, uint32_t dwCompSize, uint32_t crc)
{
	if (dwCompSize > MAX_COMP_SIZE)
	{
		return;
	}

	m_sizeCompBuf = dwCompSize;
	std::memcpy(m_abCompBuf, pbCompBuf, dwCompSize);
	m_crc = crc;
}

void SGuildMarkBlock::Compress(const Pixel* pxBuf)
{
	m_sizeCompBuf = MAX_COMP_SIZE;

	auto result = lzo1x_999_compress(reinterpret_cast<const BYTE*>(pxBuf), sizeof(Pixel) * SGuildMarkBlock::SIZE, m_abCompBuf, reinterpret_cast<lzo_uint*>(&m_sizeCompBuf), CLZO::Instance().GetWorkMemory());

	if (result != LZO_E_OK)
	{
		TraceError("SGuildMarkBlock::Compress: Error! %u > %u", sizeof(Pixel) * SGuildMarkBlock::SIZE, m_sizeCompBuf);
		return;
	}

	m_crc = GetCRC32(reinterpret_cast<const char*>(pxBuf), sizeof(Pixel) * SGuildMarkBlock::SIZE);
}