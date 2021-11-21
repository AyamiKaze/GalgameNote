# CMVS中文化支持——以天津罪为例，用hook汉化所有内容

_站在巨人的肩膀上，就没有跨不过去的坎_

![0](img\0.png)

这次拿天津罪来说说如何用全hook的方式，完整汉化一个游戏。

## 一、准备工作

### 文本

天津罪的文本可以用xmoe大佬的工具（https://github.com/xmoeproject/CMVS-Engine/tree/master/Ps3TextDumper）搞定，不过我们提取的时候做点改动。

其实观察ps3脚本不难发现，所有的文本都在文件的最后面，也就是说他有一个区域专门存放文本，那在虚拟机代码中必然有一个跳转的offset与之对应。他可以是相对的，也可以是绝对的（这里是相对于资源区）。我们只需要把这个offset拿出来，在游戏执行这个offset跳转的时候偷换掉offset，改成我们自己存放汉化文本的地方的offset即可。

所以：

```C++
MultiByteToWideChar(932, 0, String, lstrlenA(String), WideChar, 2048);
WideCharToMultiByte(CP_UTF8, 0, WideChar, lstrlenW(WideChar), UTF8Char, 4096, nullptr, nullptr);
fprintf(out, "[0x%08x]%s\r\n", StringOffset + StringInfo, UTF8Char);
fprintf(out, ";[0x%08x]\r\n\r\n", StringOffset + StringInfo);
```

这一部分改成

```C++
fprintf(out, "[0x%08x]%s\r\n", StringInfo, String);
```

这样我们导出的文本就是这样：

```tex
[0x00000980]- こころ１ -
[0x0000098d]「……卵」
[0x00000998]
[0x00000999]　誠　
[0x000009a0]そう、卵だ。
[0x000009ad]
[0x000009ae]
[0x000009af]夏の炎天下にさらされた地面は熱い。
[0x000009d2]
[0x000009d3]
[0x000009d4]どのくらい熱いかといえば、卵を割って落とせば、そのまま目玉焼きが作れそうな熱さだった。
[0x00000a2b]
[0x00000a2c]
[0x00000a2d]つまり、行き倒れて地面に転がっている僕の頭が、卵だ。
[0x00000a62]
[0x00000a63]
[0x00000a64]卵。
[0x00000a69]
[0x00000a6a]
[0x00000a6b]このまま温めると、なにが生まれるのだろう？
[0x00000a96]
[0x00000a97]
[0x00000a98]ヒヨコだろうか？
[0x00000aa9]
[0x00000aaa]
[0x00000aab]神様だろうか？
[0x00000aba]
[0x00000abb]
[0x00000abc]卵が先か鶏が先か、という問答がある。
[0x00000ae1]
[0x00000ae2]
[0x00000ae3]だけど、それらを生みだす神という存在の前に、神様を生みだすための卵があるのかもしれない。
[0x00000b3c]
[0x00000b3d]
[0x00000b3e]もしくは、卵の中に、また別の卵が……。
[0x00000b65]
```

方括号内就是文本跳转的offset，后面就是文本。这是为了方便后期我们拿文本直接hook回封时方便使用

### 图片

图片拿GARbro就可以直接解压成png，留存备用。

### 其他

建议准备好CheatEngine，这个工具在搜索内存的时候非常管用，方便我们直接定位文本。

然后是IDA，这对于你对当前代码的整体逻辑掌握有很大的帮助。

## 二、脚本回封

### 定位脚本初始状态

首先我们要定位到脚本的初始状态，也就是刚刚读入（或者解密/解压），还没有执行任何脚本代码的状态。

首先我们要找到从封包加载脚本的地方，要找到这个，首先我们要找到传入脚本名的地方。

调试器加载程序，让程序先跑一会儿，随后下断ReadFile，就会变成这样：

![1](.\img\1.png)

然后翻一下栈，就是右下角那个区域。看看有没有封包内文件名的调用。

![2](.\img\2.png)

好像确实有，然后我们回到4021F8的位置，并翻到这个函数头的位置。因为文件名是以参数传入的，我们可以去这个函数头看看栈空间有没有输入脚本名。

定位到函数头，下个断点，取消之前下的ReadFile断点，然后再跑一下，让他停在断点处。然后让我们看看栈的状况：

![3](.\img\3.png)

确实有作为参数传入，再结合刚才readfile的情况，我们可以判定这个函数是加载封包内的文件的。

继续跟踪，当读到ps3脚本时跳出这个函数

然后我们会来到这里：

![4](.\img\4.png)

可以看到下面有个循环，里面有xor和ror之类的操作，盲猜这里是在解密ps3脚本（实际上也是）。

需要注意的是下面有一个LocalAlloc在申请内存，你可能要小心附近有解压的函数。

解压函数往往类似于这种：

```C++
uncompress(BYTE* dst, UINT* dst_size, BYTE* old, UINT old_size);
```

汇编里也就是四个push加一个call。

让我们往下翻一下，有没有类似的。

![5](.\img\5.png)

果然有，迅速做好标记，跟好刚刚申请的内存，然后让他解压完毕。此时内存里面就有解压好的脚本了。

### 跟踪脚本1

让我们用CheatEngine搜一下文本，搜第一个文本，看他断在哪里。

注意，这里搜的时候要勾选CodePage选项，然后搜索的文本应该是乱码文本，日文搜不到。

![6](.\img\6.png)

很快我们找到了地址，然后跟过去，对着文本下内存访问断点，取消之前的CC断点，接着跑游戏，直到触发内存断点：

![7](.\img\7.png)

看样子我们是跟到了lstrcatA里面了，让我们回去看看：

![8](.\img\8.png)

根据lstrcatA的参数来看，我们跟紧ecx，发现不只是从45AFF0这里改变，还有在push ecx这里有个跳转，跟上去看：

```tex
0045AF7C  |> \8B96 90330000 mov edx,dword ptr ds:[esi+0x3390]
0045AF82  |.  8B8C96 FC3A00>mov ecx,dword ptr ds:[esi+edx*4+0x3AFC] ;到资源区
0045AF89  |.  03C8          add ecx,eax ;跳转，eax就是offset
0045AF8B  |.  EB 66         jmp short cmvs32.0045AFF3

```

我们两边断点都打一下，发现他停在0045AF89这一部分，我们就可以开始分析这段代码了。

mov ecx,dword ptr ds:[esi+edx*4+0x3AFC]这段代码直接打开了脚本的资源区，也就是存放文本的位置。而后面的add ecx, eax就直接跳到当前offset的脚本。

至于eax的值是否对得上之前我们导出文本的时候挑出来的offset……我们跑下一句话看看：

```tex
[0x00000980]- こころ１ -
```

![9](.\img\9.png)

看样子是能对上的，那我们直接hook了。

```C++
PVOID HookAddr_pPorcessPushStr = (PVOID)0x45AF82;
PVOID JmpOut_pPorcessPushStr = (PVOID)0x45AF89;
__declspec(naked)void ASM_ProcessPushStr()
{
    __asm
    {
        mov ecx, dword ptr ds : [esi + edx * 4 + 0x3AFC] ;
        pushad;
        pushfd;
        push eax; // offset
        push ecx; // text
        call ProcessPushStr;
        popfd;
        popad;
        mov eax, str_off; // 如果要替换文本就是替换文本的off，如果不是则是原来eax的值
        jmp JmpOut_pPorcessPushStr
    }
}
```

不过这里hook之后你会发现游戏里面并没有替换字符串，还是按照原来的格式输出的，说明主要文本输出的地方不在这。

### 跟踪脚本2

既然跟踪文本我们没跟到，那我们就跟踪offset本身来寻找真正文本输出的地方。

这里的跟踪方法是，在内存里面搜索指令+offset的bytecode，然后对offset下内存访问断点一层一层的跟（也可以下硬件访问断点，4字节的）。这里需要点耐心。

直到跟到这个地方：

![10](.\img\10.png)

这里和我们之前跟踪到的地方很像，我们可以在这里试着下一个hook看看：

```C++
PVOID HookAddr_pPorcessPushStr2 = (PVOID)0x461260;
PVOID JmpOut_pPorcessPushStr2 = (PVOID)0x461267;
__declspec(naked)void ASM_ProcessPushStr2()
{
    __asm
    {
        mov eax, dword ptr ds : [ecx + eax * 4 + 0x3AFC] ;
        pushad;
        pushfd;
        push edx; // offset
        push eax; // text
        call ProcessPushStr;
        popfd;
        popad;
        mov edx, str_off;
        jmp JmpOut_pPorcessPushStr2
    }
}
```

这个函数在这里会把目标offset的文本的地址传出去，理论上来说，这样可以直接替换文本。

![11](.\img\11.png)

事实上也确实如此。

那我们能不能把第一个hook给删掉，因为他根本没有显示文字呢？

实际上，如果你删掉了，看这个界面是没问题的，但你要是切换到可变的文本框……

![12](.\img\12.png)

就会变成这样。

这样一来我们就清楚了，前面虽然没有显示文字，但是计算了文本框的长度，来囊括所有的文字。

![13](.\img\13.png)

## 三、图片回封

图片回封相对来讲就没有文本那么麻烦，只需要在图片处理完毕后替换dib数据即可。

还是首先，跟踪读取pb3图片的过程，一直来到这里：

![14](.\img\14.png)

跟进去，然后再跟进第一个函数，继续跟

![15](.\img\15.png)

这里会解密pb3图片文件头数据。

之后遇到call就往里面跟，一直跟到50FCD0，然后用IDA F5一下这个函数。

我们首先关注一下函数前面这段代码：

![16](.\img\16.png)

v6有一个4\*x\*x的一个东西，随后这个值就被拿去分配内存了，盲猜这里是最后图片处理完成后dib存放的地方（因为图片dib的大小都是width\*height\*channel，所以可以猜测这里是dib大小，拿去分配内存就是最后的dib）

然后我们一直跟着v69，一直拉到最后，发现他直接返回了这个v69，那我们就可以在返回前动手脚，把dib数据换成我们汉化的数据。

```C++
void __stdcall ProcessPic(BYTE* dst_dib, WORD nWidth, WORD nHeight)
{
    if (FileName != nullptr)
    {
        UINT szFileName = strlen(FileName);
        if (FileName[szFileName - 3] == 'p' && FileName[szFileName - 2] == 'b' && FileName[szFileName - 1] == '3')
        {
            string fnm = string(FileName);
            string dir = "_pic\\";
            string fmtFnm = dir + fnm + ".png";
            
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
    return;
}

PVOID HookAddr_pProcessPic = (PVOID)0x510080;
__declspec(naked)void ASM_ProcessPic()
{
    __asm
    {
        pushad;
        pushfd;
        push [ebp + 0x20] ; // height
        push [ebp + 0x1E] ; // width
        push ebx; // dib
        call ProcessPic;
        popfd;
        popad;
        jmp HookAddr_pProcessPic
    }
}
```

值得一提的是，这里处理的所有图片都要是32位的，游戏在最后有这样一个操作：

![17](.\img\17.png)

他会补齐alpha通道，全部填写FF，这样一来就无所谓之前的bpp位数了，全是32位。所以在最后处理图片时，如果原图片是24位的要变成32位才行（这里代码中没有体现，因为替换的示例是32位图）。

![18](.\img\18.png)

## 四、其他

文本和图片都换进去了，其他剩下的就非常简单。

### 边界

搜索常量9F，把搜到的CMP XX, 9F全部改成FE，顺带下面还有个cmp xx, 1c也改成1e。

然后你有时候会发现有类似于mov xxx，0x8141之类的，找到他对应的中文编码，把它换了。

### 字体

hook CreateFontA，改掉CharSet和字体名（charset必须是GB2312_CHARSET）
