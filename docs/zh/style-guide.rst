#######
编程规范
#######

*******

本指南用于提供编写Solidity的编码规范，本指南会随着后续需求不断修改演进，可能会增加新的更合适的规范，旧的不适合的规范会被废弃。

当然，很多项目可能有自己的编码规范，如果存在冲突，请参考项目的编码规范。

本指南的结构及规范建议大都来自于python的[pep8编码规范](https://www.python.org/dev/peps/pep-0008/#a-foolish-consistency-is-the-hobgoblin-of-little-minds)。

本指南不是说必须完全按照指南的要求进行solidity编码，而是提供一个总体的一致性要求，这个和pep8的理念相似（译注：pep8的理念大概是：强制的一致性是非常愚蠢的行为，参见：[pep8](https://www.python.org/dev/peps/pep-0008/#a-foolish-consistency-is-the-hobgoblin-of-little-minds)）。

本指南是为了提供编码风格的一致性，因此一致性这一理念是很重要的，在项目中编码风格的一致性更加重要，而在同一个函数或模块中风格的一致性是最重要的。而最最最重要的是：你要知道什么时候需要保持一致性，什么时候不需要保持一致性，因为有时候本指南不一定适用，你需要根据自己的需要进行权衡。可以参考下边的例子决定哪一种对你来说是最合适的。

*******
代码布局
*******

缩进

每行使用4个空格缩进

tab或空格

空格是首选缩进方式

禁止tab和空格混合使用

回车（空行）

两个合约之间增加两行空行

规范的方式：

    contract A {
        ...
    }


    contract B {
        ...
    }


    contract C {
        ...
    }

不规范的方式：

    contract A {
        ...
    }
    contract B {
        ...
    }

    contract C {
        ...
    }

合约内部函数之间需要回车，如果是函数声明和函数实现一起则需要两个回车

规范的方式：


    contract A {
        function spam();
        function ham();
    }


    contract B is A {
        function spam() {
            ...
        }

        function ham() {
            ...
        }
    }

不规范的方式：

    contract A {
        function spam() {
            ...
        }
        function ham() {
            ...
        }
    }


*******
源文件编码方式
*******

首选UTF-8或者ASCII编码


*******
引入
*******

一般在代码开始进行引入声明

规范的方式：

    import "owned";


    contract A {
        ...
    }


    contract B is owned {
        ...
    }

不规范的方式：

    contract A {
        ...
    }


    import "owned";


    contract B is owned {
        ...
    }

表达式中的空格使用方法

以下场景避免使用空格

- 括号、中括号，花括号之后避免使用空格

Yes规范的方式: spam(ham[1], Coin({name: “ham”}));

No不规范的方式: spam( ham[ 1 ], Coin( { name: “ham” } ) );

- 逗号和分号之前避免使用空格

Yes规范的方式: function spam(uint i, Coin coin);

No不规范的方式: function spam(uint i , Coin coin) ;

- 赋值符前后避免多个空格

规范的方式::

    x = 1;
    y = 2;
    long_variable = 3;

不规范的方式：

::

    x             = 1;
    y             = 2;
    long_variable = 3;

控制结构

合约、库。函数、结构体的花括号使用方法：

- 左花括号和声明同一行

- 右括号和左括号声明保持相同缩进位置。

- 左括号后应回车

规范的方式：

::

    contract Coin {
        struct Bank {
            address owner;
            uint balance;
        }
    }

不规范的方式：

::

    contract Coin
    {
        struct Bank {
            address owner;
            uint balance;
        }
    }
以上建议也同样适用于if、else、while、for。

此外，if、while、for条件语句之间必须空行

规范的方式：

::

    if (...) {
        ...
    }

    for (...) {
        ...
    }

不规范的方式：

    if (...)
    {
        ...
    }

    while(...){
    }

    for (...) {
        ...;}

对于控制结构内部如果只有单条语句可以不需要使用括号。

规范的方式：

::

    if (x < 10)
        x += 1;

不规范的方式：

::

    if (x < 10)
        someArray.push(Coin({
            name: 'spam',
            value: 42
        }));

对于if语句如果包含else或者else if语句，则else语句要新起一行。else和else if的内部规范和if相同。

规范的方式：

::

    if (x < 3) {
        x += 1;
    } else if (x > 7) {
        x -= 1;
    } else {
        x = 5;
    }


    if (x < 3)
        x += 1;
    else
        x -= 1;

不规范的方式：

::

    if (x < 3) {
        x += 1;
    }
    else {
        x -= 1;
    }

*******
函数声明
*******

对于简短函数声明，建议将函数体的左括号和函数名放在同一行。

右括号和函数声明保持相同的缩进。

左括号和函数名之间要增加一个空格。

*******
规范的方式：
*******

::

    function increment(uint x) returns (uint) {
        return x + 1;
    }

    function increment(uint x) public onlyowner returns (uint) {
        return x + 1;
    }

不规范的方式：

::

    function increment(uint x) returns (uint)
    {
        return x + 1;
    }

    function increment(uint x) returns (uint){
        return x + 1;
    }

    function increment(uint x) returns (uint) {
        return x + 1;
        }

    function increment(uint x) returns (uint) {
        return x + 1;}

默认修饰符应该放在其他自定义修饰符之前。

规范的方式：

::

    function kill() public onlyowner {
        selfdestruct(owner);
    }

不规范的方式：

::

    function kill() onlyowner public {
        selfdestruct(owner);
    }

对于参数较多的函数声明可将所有参数逐行显示，并保持相同的缩进。函数声明的右括号和函数体左括号放在同一行，并和函数声明保持相同的缩进。

规范的方式：


    function thisFunctionHasLotsOfArguments(
        address a,
        address b,
        address c,
        address d,
        address e,
        address f
    ) {
        doSomething();
    }

不规范的方式：

::

    function thisFunctionHasLotsOfArguments(address a, address b, address c,
        address d, address e, address f) {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(address a,
                                            address b,
                                            address c,
                                            address d,
                                            address e,
                                            address f) {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(
        address a,
        address b,
        address c,
        address d,
        address e,
        address f) {
        doSomething();
    }

如果函数包括多个修饰符，则需要将修饰符分行并逐行缩进显示。函数体左括号也要分行。

规范的方式：

::

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public
        onlyowner
        priced
        returns (address)
    {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(
        address x,
        address y,
        address z,
    )
        public
        onlyowner
        priced
        returns (address)
    {
        doSomething();
    }

不规范的方式：

::

    function thisFunctionNameIsReallyLong(address x, address y, address z)
                                          public
                                          onlyowner
                                          priced
                                          returns (address) {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public onlyowner priced returns (address)
    {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public
        onlyowner
        priced
        returns (address) {
        doSomething();
    }


对于需要参数作为构造函数的派生合约，如果函数声明太长或者难于阅读，建议将其构造函数中涉及基类的构造函数分行独立显示。

规范的方式：

::

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4)
        {
            // do something with param5
        }
    }

不规范的方式：

::

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
        B(param1)
        C(param2, param3)
        D(param4)
        {
            // do something with param5
        }
    }

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4) {
            // do something with param5
        }
    }

对于函数声明的编程规范主要用于提升可读性，本指南不可能囊括所有编程规范，对于不涉及的地方，程序猿可发挥自己的主观能动性。

映射
待完成

变量声明

对于数组变量声明，类型和数组中括号直接不能有空格。

规范的方式: uint[] x; 不规范的方式: uint [] x;

其他建议

- 赋值运算符两边要有一个空格

规范的方式：

```
x = 3;x = 100 / 10;x += 3 + 4;x |= y && z;
```

不规范的方式：

```
x=3;x = 100/10;x += 3+4;x |= y&&z;
```

- 为了显示优先级，优先级运算符和低优先级运算符之间要有空格，这也是为了提升复杂声明的可读性。对于运算符两侧的空格数目必须保持一致。

规范的方式：

```
x = 2**3 + 5;x = 2***y + 3*z;x = (a+b) * (a-**b);
```

不规范的方式：

```
x = 2** 3 + 5;x = y+z;x +=1;
```

命名规范

命名规范是强大且广泛使用的，使用不同的命名规范可以传递不同的信息。

以下建议是用来提升代码的可读性，因此被规范不是规则而是用于帮助更好的解释相关代码。

最后，编码风格的一致性是最重要的。

命名方式

为了防止混淆，以下命名用于说明（描述）不同的命名方式。

- b（单个小写字母）

- B（单个大写字母）

- 小写

- 有下划线的小写

- 大写

- 有下划线的大写

- CapWords规范（首字母大写）

- 混合方式（与CapitalizedWords的不同在于首字母小写!）

- 有下划线的首字母大写（译注：对于python来说不建议这种方式）

注意

当使用CapWords规范（首字母大写）的缩略语时，缩略语全部大写，比如HTTPServerError 比HttpServerError就好理解一点。

避免的命名方式

- l - Lowercase letter el  小写的l


- O - Uppercase letter oh 大写的o


- I - Uppercase letter eye 大写的i

永远不要用字符‘l'(小写字母el(就是读音，下同))，‘O'(大写字母oh)，或‘I'(大写字母eye)作为单字符的变量名。在某些字体中这些字符不能与数字1和0分辨。试着在使用‘l'时用‘L'代替。 

合约及库的命名

合约应该使用CapWords规范命名（首字母大写）。

事件

事件应该使用CapWords规范命名（首字母大写）。

函数命名

函数名使用大小写混合

函数参数命名

当定义一个在自定义结构体上的库函数时，结构体的名称必须具有自解释能力。

局部变量命名

大小写混合

常量命名

常量全部使用大写字母并用下划线分隔。

修饰符命名

功能修饰符使用小写字符并用下划线分隔。

避免冲突

- 单个下划线结尾

当和内置或者保留名称冲突时建议使用本规范。

通用建议

待完成
