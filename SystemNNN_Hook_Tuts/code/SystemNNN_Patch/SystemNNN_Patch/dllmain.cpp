// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
CHAR TextBox[0x1000] = { 0 };
int(__fastcall* sub_425FA0)(DWORD* pThis, int null_ptr, int mesNum);
int __fastcall GetMessagePtr(DWORD* pThis, int null_ptr, int mesNum)
{
    /*
    int ret = pThis[0x242] + 4 * *(DWORD*)(pThis[0x242] + 4 * (mesNum + pThis[0x245]));
    cout << "mes: ";
    cout << dec << mesNum; //当前文本编号
    cout << " | 0x" << hex << 4 * *(DWORD*)(pThis[0x242] + 4 * (mesNum + pThis[0x245])); //当前文本offset
    cout << " | 0x" << hex << pThis[0x243]; // 当前脚本大小
    cout << " | " << wtoc(ctow((LPSTR)ret, 932), 936);
    cout << endl;
    */
    ///*
    if (mesNum == 0)
    {
        memset(TextBox, 0, 0x1000);
        lstrcpyA(TextBox, "？？？\r\n这是我的中文测试１２３ＡＢＣａｂｃ\r\n这是我的中文测试１２３ＡＢＣａｂｃ\r\n");
        return (int)&TextBox;
    }
    //*/
    return sub_425FA0(pThis, null_ptr, mesNum);
}

int(__fastcall* sub_425FC0)(DWORD* pThis, int null_ptr, int strNum);
int __fastcall GetStringPtr(DWORD* pThis, int null_ptr, int strNum)
{
    /*
    int ret = pThis[0x242] + 4 * *(DWORD*)(pThis[0x242] + 4 * (strNum + pThis[0x246]));
    cout << "str: ";
    cout << dec << strNum; //当前文本编号
    cout << " | 0x" << hex << 4 * *(DWORD*)(pThis[0x242] + 4 * (strNum + pThis[0x246])); //当前文本offset
    cout << " | 0x" << hex << pThis[0x243]; // 当前脚本大小
    cout << " | " << wtoc(ctow((LPSTR)ret, 932), 936);
    cout << endl;
    */
    return sub_425FC0(pThis, null_ptr, strNum);
}


/*
// 方法1：需要适配对应的dwq

typedef FILE* (_cdecl* pFopen_t)(const char* FileName, const char* Mode);
auto pFopen = (pFopen_t)(BaseAddr + 0x3596);

FILE* fp = nullptr;
CHAR PicFileName[MAX_PATH] = { 0 };
FILE* (__fastcall* sub_40BB00)(DWORD* pThis, int null_ptr, LPSTR fullFileName, LPSTR filename);
FILE* __fastcall OpenDWQFile(DWORD* pThis, int null_ptr, LPSTR fullFileName, LPSTR filename)
{
    wsprintfA(PicFileName, "_pic\\%s.dwq\0", filename);
    //cout << PicFileName << endl;
    fp = pFopen(PicFileName, "rb");
    if (fp)
    {
        pThis[26] = 0;
        return fp;
    }
    return sub_40BB00(pThis, null_ptr, fullFileName, filename);
}
*/

/*
// 方法2：针对每一种不同的格式的dib数据进行处理
CHAR PicFileName[MAX_PATH] = { 0 };

FILE* (__fastcall* sub_40BB00)(DWORD* pThis, int null_ptr, LPSTR fullFileName, LPSTR filename);
FILE* __fastcall OpenDWQFile(DWORD* pThis, int null_ptr, LPSTR fullFileName, LPSTR filename)
{
    wsprintfA(PicFileName, "_pic\\%s.png\0", filename);
    return sub_40BB00(pThis, null_ptr, fullFileName, filename);
}

DWORD dst_addr = 0;
void __stdcall CopyDib(BYTE* dst)
{
    if (dst_addr != 0)
    {
        FILE* fp = fopen(PicFileName, "rb");
        if (fp)
        {
            cout << PicFileName << endl;
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
            if (bit_depth == 24)
                dib = Dib24To32(width, height, dib);
            BMP_TO_DIB((PBYTE)dib.Get(), width, height, 32);

            cout << "start copy" << endl;
            memcpy(dst, dib.Get(), width * height * 4);
            dst_addr = 0;
            cout << "end" << endl;
        }
    }
}

void __stdcall PrintAddr(DWORD Addr)
{
    cout << "addr:0x" << hex << Addr << endl;
}


// jpg
PVOID pGetDibAddr = (PVOID)(BaseAddr + 0xC4E9);
__declspec(naked)void GetDibAddr()
{
    _asm
    {
        mov dst_addr, eax;
        jmp pGetDibAddr;
    }
}

PVOID pDoCopy = (PVOID)(BaseAddr + 0xC3AA);
__declspec(naked)void DoCopy()
{
    _asm
    {
        pushad;
        pushfd;
        push dst_addr;
        call CopyDib;
        popfd;
        popad;
        jmp pDoCopy
    }
}

// PACKBMP
PVOID pGetDibAddr2 = (PVOID)(BaseAddr + 0x9499);
__declspec(naked)void GetDibAddr2()
{
    _asm
    {
        pushad;
        pushfd;
        //push[EBP + 0x20];
        //call PrintAddr;
        MOV EAX, DWORD PTR SS : [EBP + 0x20] ;
        MOV dst_addr, eax;
        popfd;
        popad;
        jmp pGetDibAddr2
    }
}

PVOID pDoCopy2 = (PVOID)(BaseAddr + 0x95D0);
__declspec(naked)void DoCopy2()
{
    _asm
    {
        pushad;
        pushfd;
        push dst_addr;
        call CopyDib;
        popfd;
        popad;
        jmp pDoCopy2
    }
}
*/

// 方法3：执行完函数后替换dib
CHAR PicFileName[MAX_PATH] = { 0 };
void __stdcall CopyDib(BYTE* dst)
{
    if (dst != 0)
    {
        FILE* fp = fopen(PicFileName, "rb");
        if (fp)
        {
            cout << PicFileName << endl;
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
            if (bit_depth == 24)
                dib = Dib24To32(width, height, dib);
            BMP_TO_DIB((PBYTE)dib.Get(), width, height, 32);

            cout << "start copy" << endl;
            memcpy(dst, dib.Get(), width * height * 4);
            cout << "end" << endl;
        }
    }
}

BOOL(__fastcall* sub_409900)(int pThis, int null_ptr, LPSTR filename, BOOL b256Flag, int a4);
BOOL __fastcall LoadDWQ(int pThis, int null_ptr, LPSTR filename, BOOL b256Flag, int a4)
{
    wsprintfA(PicFileName, "_pic\\%s.png\0", filename);
    int m_pic = pThis + 8 * 4;
    //cout << "addr:0x" << hex << addr << endl;
    BOOL ret = sub_409900(pThis, null_ptr, filename, b256Flag, a4);
    if (ret) 
    {
        BYTE* DibPtr = (BYTE*)*(int*)m_pic;
        cout << "point:0x" << hex << (int)DibPtr << endl;
        CopyDib(DibPtr);
    }
    return ret;
}

void InlineHook()
{
    *(DWORD*)&sub_425FA0 = BaseAddr + 0x25FA0;
    *(DWORD*)&sub_425FC0 = BaseAddr + 0x25FC0;
    /* pic方法1&2
    (DWORD*)&sub_40BB00 = BaseAddr + 0xBB00;
    */
    *(DWORD*)&sub_409900 = BaseAddr + 0x9900;
    DetourTransactionBegin();
    // Text
    DetourAttach((void**)&sub_425FA0, GetMessagePtr);
    DetourAttach((void**)&sub_425FC0, GetStringPtr);
    // Pic
    /* 方法1&2
    DetourAttach((void**)&sub_40BB00, OpenDWQFile);
    DetourAttach(&pGetDibAddr, GetDibAddr);
    DetourAttach(&pDoCopy, DoCopy);

    DetourAttach(&pGetDibAddr2, GetDibAddr2);
    DetourAttach(&pDoCopy2, DoCopy2);
    */
    // 方法3
    DetourAttach((void**)&sub_409900, LoadDWQ);

    if (DetourTransactionCommit() != NO_ERROR)
    {
        MessageBox(NULL, L"InlinePatch Error.", L"InlinePatch", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
    }
}

PVOID g_pOldCreateFontA = CreateFontA;
typedef HFONT(WINAPI* PfuncCreateFontA)(int nHeight,
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

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI* pfuncCreateWindowExA)(
    DWORD dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
    DWORD dwStyle,
    int x,
    int y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
HWND WINAPI HookCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    const char* szWndName = "【114514汉化组】梦幻廻廊 中文化演示";

    return ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, (LPCTSTR)szWndName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

void Init()
{
    DetourTransactionBegin();
    DetourAttach(&g_pOldCreateFontA, HookCreateFontA);
    DetourAttach(&g_pOldCreateWindowExA, HookCreateWindowExA);
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
        InlineHook();
        Init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

__declspec(dllexport)void WINAPI AyamiKaze()
{
}