// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"

DWORD str_off = 0;
CHAR CopyStr[0x1000] = { 0 };
CHAR* FileName = nullptr;

PVOID HookAddr_pGetFileNameInPackage= (PVOID)0x402060;
__declspec(naked)void ASM_GetFileNameInPackage()
{
    __asm
    {
        mov FileName, eax;
        jmp HookAddr_pGetFileNameInPackage;
    }
}


void __stdcall ProcessPushStr(UINT str, UINT off)
{
    str_off = off;
    //cout << "0x" << hex << off << "|" << wtoc(ctow(str + off, 932), 936) << endl;
    ///*
    if (off == 0x980)
    {
        lstrcpyA(CopyStr, "- 心１ -");
        str_off = (UINT)CopyStr - str;
    }
    else if (off == 0x98D)
    {
        lstrcpyA(CopyStr, "中文测试１２３ＡＢＣａｂｃ");
        str_off = (UINT)CopyStr - str;
    }
    else if (off == 0x999)
    {
        lstrcpyA(CopyStr, "中文测试");
        str_off = (UINT)CopyStr - str;
    }
    else if (off == 0x15F)
    {
        lstrcpyA(CopyStr, "【114514汉化组】天津罪");
        str_off = (UINT)CopyStr - str;
    }
   // */
    cout << "0x" << hex << str_off << " | " << (CHAR*)(str + off) << endl;
    return;
}

PVOID HookAddr_pPorcessPushStr = (PVOID)0x45AF82;
PVOID JmpOut_pPorcessPushStr = (PVOID)0x45AF89;
__declspec(naked)void ASM_ProcessPushStr()
{
    __asm
    {
        mov ecx, dword ptr ds : [esi + edx * 4 + 0x3AFC] ;
        pushad;
        pushfd;
        push eax;
        push ecx;
        call ProcessPushStr;
        popfd;
        popad;
        mov eax, str_off;
        jmp JmpOut_pPorcessPushStr
    }
}


PVOID HookAddr_pPorcessPushStr2 = (PVOID)0x461260;
PVOID JmpOut_pPorcessPushStr2 = (PVOID)0x461267;
__declspec(naked)void ASM_ProcessPushStr2()
{
    __asm
    {
        mov eax, dword ptr ds : [ecx + eax * 4 + 0x3AFC] ;
        pushad;
        pushfd;
        push edx;
        push eax;
        call ProcessPushStr;
        popfd;
        popad;
        mov edx, str_off;
        jmp JmpOut_pPorcessPushStr2
    }
}

void BMP_TO_DIB(PBYTE data, int width, int height, int BitCount)
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

void __stdcall ProcessPic(BYTE* dst_dib, WORD nWidth, WORD nHeight)
{
    ///*
    if (FileName != nullptr)
    {
        UINT szFileName = strlen(FileName);
        if (FileName[szFileName - 3] == 'p' && FileName[szFileName - 2] == 'b' && FileName[szFileName - 1] == '3')
        {
            string fnm = string(FileName);
            string dir = "_pic\\";
            string fmtFnm = dir + fnm + ".png";
            
            // dump
            //cout << "dump:" << fmtFnm << "|" << dec << width << "x" << height << endl;
            //write_bmp(fmtFnm.c_str(), dib, width * height * 4, width, -height, 32);

            // copy
            /*
            FILE* fp = fopen(fmtFnm.c_str(), "rb");
            if (!fp)
                return;
            fseek(fp, 0, SEEK_SET);

            BITMAPFILEHEADER bmf;
            BITMAPINFOHEADER bmi;
            fread(&bmf, sizeof(BITMAPFILEHEADER), 1, fp);
            fread(&bmi, sizeof(BITMAPINFOHEADER), 1, fp);

            DWORD DIBSize = bmf.bfSize - bmf.bfOffBits;
            BYTE* DIB = new BYTE[DIBSize];
            fread(DIB, DIBSize, 1, fp);
            fclose(fp);

            memcpy(dib, DIB, DIBSize);
            delete[] DIB;
            cout << "copy:" << fmtFnm << "|" << dec << width << "x" << height << endl;
            */

            // copy2
            FILE* fp = nullptr;
            auto err = fopen_s(&fp, fmtFnm.c_str(), "rb");
            if (!fp)
                return;
            fseek(fp, 0, SEEK_END);
            DWORD size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            BYTE* buff = new BYTE[size];
            fread(buff, size, 1, fp);
            fclose(fp);

            int width, height, bit_depth;
            NakedMemory dib;
            if (!ReadPngToBmp(buff, size, &width, &height, &bit_depth, dib))
            {
                delete[] buff;
                return;
            }
            delete[] buff;

            if (width != nWidth || height != nHeight)
            {
                MessageBox(NULL, L"图片信息不匹配", L"Info", MB_OK | MB_ICONERROR);
                return;
            }

            BMP_TO_DIB((PBYTE)dib.Get(), width, height, bit_depth);
            memcpy(dst_dib, dib.Get(), dib.GetSize());
            cout << "copy:" << fmtFnm << "|" << dec << width << "x" << height << endl;
        }
    }
    //*/
    return;
}

PVOID HookAddr_pProcessPic = (PVOID)0x510080;
__declspec(naked)void ASM_ProcessPic()
{
    __asm
    {
        pushad;
        pushfd;
        push [ebp + 0x20] ;
        push [ebp + 0x1E] ;
        push ebx;
        call ProcessPic;
        popfd;
        popad;
        jmp HookAddr_pProcessPic
    }
}

void InlinePatch()
{
    DetourTransactionBegin();

    DetourAttach(&HookAddr_pGetFileNameInPackage, ASM_GetFileNameInPackage);

    DetourAttach(&HookAddr_pPorcessPushStr, ASM_ProcessPushStr);
    DetourAttach(&HookAddr_pPorcessPushStr2, ASM_ProcessPushStr2);

    DetourAttach(&HookAddr_pProcessPic, ASM_ProcessPic);
    if (DetourTransactionCommit() != NOERROR)
    {
        MessageBox(NULL, L"Patch hook error", L"Patch", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
    }
}

PVOID g_pOldCreateFontA = CreateFontA;
typedef HFONT (WINAPI* PfuncCreateFontA)(int nHeight,
    int nWidth,
    int nEscapement,
    int nOrientation,
    int fnWeight,
    DWORD fdwltalic,
    DWORD fdwUnderline,
    DWORD fdwStrikeOut,
    DWORD fdwCharSet,
    DWORD fdwOutputPrecision,
    DWORD fdwClipPrecision,
    DWORD fdwQuality,
    DWORD fdwPitchAndFamily,
    LPCTSTR lpszFace);
HFONT WINAPI HookCreateFontA(int nHeight,
    int nWidth,
    int nEscapement,
    int nOrientation,
    int fnWeight,
    DWORD fdwltalic,
    DWORD fdwUnderline,
    DWORD fdwStrikeOut,
    DWORD fdwCharSet,
    DWORD fdwOutputPrecision,
    DWORD fdwClipPrecision,
    DWORD fdwQuality,
    DWORD fdwPitchAndFamily,
    LPCTSTR lpszFace)
{
    return CreateFontW(nHeight,
        nWidth,
        nEscapement,
        nOrientation,
        fnWeight,
        fdwltalic,
        fdwUnderline,
        fdwStrikeOut,
        GB2312_CHARSET,
        fdwOutputPrecision,
        fdwClipPrecision,
        fdwQuality,
        fdwPitchAndFamily,
        L"SimHei");
}

PVOID g_pOldSetWindowTextA = SetWindowTextA;
typedef bool (WINAPI* PfuncSetWindowTextA)(HWND hWnd, LPCSTR lpString);
bool WINAPI HookSetWindowTextA(HWND hw, LPCSTR lpString)
{
    //cout << lpString << endl;
    for (int i = 0; i < lstrlenA(lpString);)
    {
        UINT c1 = lpString[i] & 0xFF;
        if (c1 == 0x81 && (UINT)lpString[i + 1] == 0x40)
        {
            memcpy((void*)(lpString + i), "\xA1\xA1", 2);
            i += 2;
        }
        else
            i++;
    }
    return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hw, lpString);
}

void Init()
{
    DetourTransactionBegin();
    DetourAttach(&g_pOldCreateFontA, HookCreateFontA);
    DetourAttach(&g_pOldSetWindowTextA, HookSetWindowTextA);
    DetourTransactionCommit();
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        make_console();
        InlinePatch();
        Init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

