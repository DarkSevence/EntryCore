#ifndef __INC_CRC32_H__
#define __INC_CRC32_H__

#include <Windows.h>
#include <cstdint>

extern DWORD GetCRC32(const char* buffer, size_t count);
extern DWORD GetCaseCRC32(const char* buf, size_t len);
extern DWORD GetHFILECRC32(HANDLE hFile);
extern DWORD GetFileCRC32(const char* c_szFileName);
extern DWORD GetFileSize(const char* c_szFileName);

#endif
