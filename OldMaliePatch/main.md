# 旧版malie引擎汉化补丁制作

开局一张图，内容全靠编（

![1](.\\img\\1.png)

上一讲基本介绍了文本提取的办法。本章接着讲（

首先是文本（包括有声文本和无声文本，这两种文本在游戏里叫做RichString）。

还记得我们最开始下断文本，他在RichString_Copy吗，那个函数出来就是一个0x6的body。

![2](.\\img\\2.png)

注意ScenarioObject+0x24，这个值指向当前脚本位置。之所以叫他ScenarioObject是因为这里还包含其他的内容，同时ScenarioProcessor_Step也在维护这个object：

![3](.\\img\\3.png)

这样来看，我们很好算脚本的index，即当前位置减去起始位置

接下来，还是回到ScenarioObject+0x24那，我们看看汇编是怎么说的：

![4](.\\img\\4.png)

可以看到，他把当前脚本位置传到ecx再push进去，以便函数调用。那就很简单，我们在Push str哪里下hook，强行把ecx换掉。

```C++
PVOID pHookPushNormalStr = (PVOID)0x00424B62;
__declspec(naked)void gHookPushNormalStr()
{
    DWORD ret;
    _asm
    {
        call ReplaceText;
        mov ecx, eax;
        jmp pHookPushNormalStr
    }
}
```

然后是人名。

如果你足够细心你会发现上面我注释了一个NameBox的变量。这里面存的就是人名。但是我们不能在这里hook，因为他会无限循环第一个人名。因此我们需要找到他复制到NameBox的位置。

我们对NameBox按x，然后一个一个看，很快我们可以看到这个函数：
![5](.\\img\\5.png)

这个函数很明显就复制一个字符串过去，我们可以在这里打个断点检查一下copy什么。

很快你就发现，他不仅复制人名，而且是在有人名的时候才会调用。这很符合我们的理想。

直接hook掉这个函数即可。