#######
杂项
#######

*******
存储中状态变量的布局
*******

静态尺寸大小的变量（除了映射和动态尺寸大小的数组类型（的其他类型变量））在存储中，是从位置0连续存储。如果可能的话，不足32个字节的多个条目被紧凑排列在一个单一的存储块，参见以下规则：

- 在存储块中的第一项是存储低阶对齐的。

- 基本类型只使用了正好存储它们的字节数。

- 如果一个基本类型不适合存储块的剩余部分，则移动到下一个存储块中。

- 结构和数组的数据总是开始一个新的块并且占整个块（根据这些规则，结构或数组项都是紧凑排列的）。

结构和数组元素是一个接着一个存储排列的，就如当初它们被声明的次序。

由于无法预知的分配的大小，映射和动态尺寸大小的数组类型（这两种类型）是使用sha3 计算来找到新的起始位置，来存放值或者数组数据。这些起始位置总是满栈块。

根据上述规则，映射或动态数组本身存放在（没有填满）的存储块位置p（或从映射到映射或数组递归应用此规则）。对于一个动态数组，存储块存储了数组元素的数目（字节数组和字符串是一个例外，见下文）。对于映射，存储块是未使用的（但它是需要的，因此，前后相邻的两个相同的映射，将使用一个不同的hash分布）。数组数据位于sha3(p)， 对应于一个映射key值k位于 sha3(k . p)  （这里 . 是连接符）。如果该值又是一个非基本类型，位置的偏移量是sha3(k . p)。

如果bytes 和 string是短类型的，它们将和其长度存储在同一个存储块里。特别是：如果数据最长31字节，它被存储在高阶字节（左对齐）, 低字节存储length* 2。 如果是长类型，主存储块存储 length* 2 + 1,  数据存储在sha3(shot)。

因此，本合约片段如下：

::

    contract c {
      struct S { uint a; uint b; }
      uint x;
      mapping(uint => mapping(uint => S)) data;
    }

*******
深奥的特点
*******

在Solidity的类型系统中，有一些在语法中没有对应的类型。其中就有函数的类型。但若使用 var （这个关键字），该函数就被认为是这个类型的局部变量：

::

    contract FunctionSelector {
     function select(bool useB, uint x) returns (uint z) {
        var f = a;
        if (useB) f = b;
        return f(x);
      }
      function a(uint x) returns (uint z) {
        return x * x;
      }
      function b(uint x) returns (uint z) {
        return 2 * x;
      }
    }

（在上面的程序片段中）

若调用select(false, x)，  就会计算 x * x 。若调用select(true, x)）就会计算 2 * x。

*******
内部-优化器
*******

Solidity 优化器是在汇编级别上的操作，所以它也可以同时被其他语言所使用。它将指令的（执行）次序，在JUMP 和 JUMPDEST上分成基本的块。在这些块中，指令被解析 。 堆栈、内存或存储上的每一次修改，都将作为表达式被记录。该表达式包括一条指令以及指向其他表达式的一系列参数的一个指针。现在的主要意思是要找到相等的表达式（在每次输入），做成了表达式的类。优化器首先在已知的表达式列表里找，若找不到的话，就根据constant + constant = sum_of_constants  或 X * 1 = X  来简化。 因为这样做是递归的，如果第二个因子是一个更复杂的表达式，我们也可以应用latter规则来计算。存储和内存位置的修改，是不知道存储和内存的位置的区别。如果我们先写到的位置x，再写到位置y , x,y均是输入变量。第二个可以覆写第一个，所以我们不知道x是存放在y之后的。另一方面，如果一个简化的表达式 x-y 能计算出一个非零常数，我们就知道x存放的内容。

在这个过程结束时，我们知道，表达式必须在堆栈中结尾，并有一系列对内存和存储的修改。这些信息存储在block上，并链接这些block。此外，有关堆栈，存储和内存配置的信息会转发到下一个block。如果我们知道所有的 JUMP 和 JUMPI 指令，我们可以建立一个完整的程序控制流图。如果我们不知道目标块（原则上，跳转目标是从输入里得到的），我们必须清除所有输入状态的存储块上的信息，（因为它的目标块未知）。如果条件计算的结果为一个常量，它转化为一个无条件jump。

作为最后一步，在每个块中的代码完全可以重新生成。从堆栈里block的结尾表达式开始，创建一个依赖关系图。每个不是这个图上的操作将舍弃。现在能按照原来代码的顺序，生成对存储和内存修改的代码（舍弃不必要的修改）。最后，在正确位置的堆栈上，生成所有的值。

这些步骤适用于每一个基本块， 如果它是较小的，用新生成的代码来替换。如果一个基本块在JUMPI上进行分割，在分析过程中，条件表达式的结果计算为一个常数，JUMP就用常量值进行替换。代码如下

::

    var x = 7;
    data[7] = 9;
    if (data[x] != x + 2)
      return 2;
    else
      return 1;


简化成下面可以编译的形式

::

    data[7] = 9;
    return 1;

即使在开始处包含有jump指令

*******
使用命令行编译器
*******

一个 Solidity 库构建的目标是solc， Solidity命令行编译器。使用solc –help  为您提供所有选项的解释。编译器可以产生不同的输出，从简单的二进制文件，程序集的抽象语法树（解析树）到gas使用的估量。如果你只想编译一个文件，你运行solc –bin sourceFile.sol ， 将会打印出二进制。你部署你的合约之前，使用solc –optimize –bin sourceFile.sol 来激活优化器。如果你想获得一些solc更进一步的输出变量，可以使用solc -o outputDirectory –bin –ast –asm sourceFile.sol，（这条命令）将通知编译器输出结果到单独的文件中。

命令行编译器会自动从文件系统中读取输入文件，但也可以如下列方法，提供重定向路径prefix=path ：

solc github.com/ethereum/dapp-bin/=/usr/local/lib/dapp-bin/ =/usr/local/lib/fallback file.sol

该命令告诉编译器在/usr/local/lib/dapp-bin目录下，寻找以github.com/ethereum/dapp-bin/  开头的文件，如果找不到的话，到usr/local/lib/fallback目录下找（空前缀总是匹配）。

solc不会从remapping目标的外部，或者显式定义的源文件的外部文件系统读取文件，所以要写成 import “/etc/passwd”;     只有增加 =/ 作为remapping,程序才能工作。

如果remapping里找到了多个匹配，则选择有共同的前缀最长的那个匹配。

如果你的合约使用了  libraries ，你会注意到字节码中包含了 form LibraryName 这样的子字符串。你可以在这些地方使用solc 作为链接器，来插入库地址 ：

Either add –libraries “Math:0x12345678901234567890 Heap:0xabcdef0123456”    提供每个库的地址， 或者在文件中存放字符串（每行一个库）

然后运行solc，后面写 –libraries fileName.

如果solc后面接着 –link 选项，所有输入文件将被解释为未链接的二进制文件（十六进制编码）， LibraryName形式如前所述， 库此时被链接（从stdin读取输入，从stdout输出）。在这种情况下，除了–libraries,其他所有的选项都将被忽略（包括 -o)

*******
提示和技巧
*******

- 在数组中使用delete，就是删除数组中的所有元素。

- 使用较短的类型和结构元素，短类型分组在一起进行排序。sstore操作可能合并成一个单一的sstore，这可以降低gas的成本（sstore消耗5000或20000 gas，所以这是你必须优化的原因）。使用天gas的价格估算功能（优化器 enable）进行检查！

- 让你的状态变量公开，编译器会免费创建 getters 。

- 如果你结束了输入或状态的检查条件，请尝试使用函数修饰符。

- 如果你的合约有一个功能send， 但你想使用内置的send功能，请使用 address(contractVariable).send(amount)。

- 如果你不想你的合约通过send接收ether，您可以添加一个抛出回退函数 function() { throw; }.。

- 用单条赋值语句初始化存储结构：x = MyStruct({a: 1, b: 2});

*******
陷阱
*******

不幸的是，还有一些编译器微妙的情况还没有告诉你。

- 在for (var i = 0; i < arrayName.length; i++) { ... },  i的类型是uint8，因为这是存放值0最小的类型。如果数组元素超过255个，则循环将不会终止。

*******
列表
*******

全局变量
---------

- block.coinbase (address):当前块的矿场的地址

- block.difficulty (uint):当前块的难度

- block.gaslimit (uint):当前块的gaslimit

- block.number (uint):当前块的数量

- block.blockhash (function(uint) returns (bytes32)):给定的块的hash值, 只有最近工作的256个块的hash值

- block.timestamp (uint):当前块的时间戳

- msg.data (bytes):完整的calldata

- msg.gas (uint): 剩余gas

- msg.sender (address):消息的发送者（当前调用）

- msg.value (uint):和消息一起发送的wei的数量

- now (uint):当前块的时间戳（block.timestamp的别名）

- tx.gasprice (uint):交易的gas价格

- tx.origin (address):交易的发送者（全调用链）

- sha3(...) returns (bytes32):计算（紧凑排列的）参数的 Ethereum-SHA3  hash值 

- sha256(...) returns (bytes32)计算（紧凑排列的）参数的SHA256 hash值 

- ripemd160(...) returns (bytes20):计算 256个（紧凑排列的）参数的RIPEMD

- ecrecover(bytes32, uint8, bytes32, bytes32) returns (address):椭圆曲线签名公钥恢复

- addmod(uint x, uint y, uint k) returns (uint):计算（x + y）K，加法为任意精度，不以2 ** 256取余

- mulmod(uint x, uint y, uint k) returns (uint):计算（XY）K，乘法为任意精度，不以2 * 256取余

- this (current contract’s type): 当前合约，在地址上显式转换

- super:在层次关系上一层的合约

- selfdestruct(address):销毁当前的合同，将其资金发送到指定address地址

- <address>.balance:address地址中的账户余额（以wei为单位）

- <address>.send(uint256) returns (bool):将一定量wei发送给address地址，若失败返回false。

*******
函数可见性定义符
*******

::

    function myFunction() <visibility specifier> returns (bool) {
        return true;
    }

- public:在外部和内部均可见（创建存储/状态变量的访问者函数）

- private:仅在当前合约中可见

- external: 只有外部可见（仅对函数）- 仅仅在消息调用中（通过this.fun）

- internal: 只有内部可见

Modifiers

- constant for state variables: Disallows assignment (except initialisation), does not occupy storage slot.

- constant for functions: Disallows modification of state - this is not enforced yet.

- anonymous for events: Does not store event signature as topic.

- indexed for event parameters: Stores the parameter as topic.

修饰符

- constant for state variables: 不允许赋值（除了初始化），不占用存储块。

- constant for functions:不允许改变状态- 这个目前不是强制的。

- anonymous for events:不能将topic作为事件指纹进行存储。

- indexed for event parameters: 将topic作为参数存储。
