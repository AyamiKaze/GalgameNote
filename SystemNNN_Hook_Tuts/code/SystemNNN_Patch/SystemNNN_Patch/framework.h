// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <unordered_map>
#include <map>
#include <intrin.h>

#include "detours.h"
#include "_Util.h"
#include "zlib.h"
using namespace std;

HMODULE Base = GetModuleHandle(NULL);
DWORD BaseAddr = (DWORD)Base;

static void make_console() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	std::cout << "Open Console Success!" << std::endl;
}

static BOOL IATPatch(LPCSTR szDllName, PROC pfnOrg, PROC pfnNew)
{
	HMODULE hmod;
	LPCSTR szLibName;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pThunk;
	DWORD dwOldProtect, dwRVA;
	PBYTE pAddr;

	hmod = GetModuleHandleW(NULL);
	pAddr = (PBYTE)hmod;
	pAddr += *((DWORD*)&pAddr[0x3C]);
	dwRVA = *((DWORD*)&pAddr[0x80]);

	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)hmod + dwRVA);

	for (; pImportDesc->Name; pImportDesc++)
	{
		szLibName = (LPCSTR)((DWORD)hmod + pImportDesc->Name);
		if (!stricmp(szLibName, szDllName))
		{
			pThunk = (PIMAGE_THUNK_DATA)((DWORD)hmod + pImportDesc->FirstThunk);
			for (; pThunk->u1.Function; pThunk++)
			{
				if (pThunk->u1.Function == (DWORD)pfnOrg)
				{
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
					pThunk->u1.Function = (DWORD)pfnNew;
					VirtualProtect((LPVOID)&pThunk->u1.Function, 4, dwOldProtect, &dwOldProtect);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

static char* wtoc(LPCTSTR str, UINT cp)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(cp, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char* out = new char[dwMinSize];
	WideCharToMultiByte(cp, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

static LPWSTR ctow(char* str, UINT cp)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(cp, 0, str, -1, NULL, 0); //计算长度
	LPWSTR out = new wchar_t[dwMinSize];
	MultiByteToWideChar(cp, 0, str, -1, out, dwMinSize);//转换
	return out;
}

static void BMP_TO_DIB(PBYTE data, int width, int height, int BitCount)
{
	BYTE* TempBuffer = nullptr;
	int i = 0;
	int widthlen = 0;
	int nAlignWidth = 0;
	size_t BufferSize = 0;
	nAlignWidth = (width * 32 + 31) / 32;
	BufferSize = 4 * nAlignWidth * height;
	TempBuffer = (BYTE*)malloc(BufferSize);

	//反转图片,修正图片信息
	widthlen = width * (BitCount / 8); //对齐宽度大小
	for (i = 0; i < height; i++) {
		memcpy(&TempBuffer[(((height - i) - 1) * widthlen)], &data[widthlen * i], widthlen);
	}

	memcpy(data, TempBuffer, BufferSize);

	free(TempBuffer);
}