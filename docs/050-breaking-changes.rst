********************************
Solidity v0.5.0 突破性变化
********************************

本节强调了 Solidity 0.5.0 版本中引入的主要突破性变化，
以及这些变化背后的原因和如何更新受影响的代码。
对于完整的列表，请查看 `版本更新日志 <https://github.com/ethereum/solidity/releases/tag/v0.5.0>`_。

.. note::
   用 Solidity v0.5.0 编译的合约仍然可以与合约甚至用旧版本编译的库对接，
   而无需重新编译或重新部署。
   将接口更改为包含数据位置，可见性和可变性说明符就足够了。
   参见下面的 :ref:`与旧合约的互操作性 <interoperability>` 部分。

仅有语义上的变化
=====================

本节仅列出了语义的变化，因此有可能在现有代码中隐藏新的且不同的行为。

* 有符号的右移现在使用正确的算术移位，即向负无穷大取整，而不是向零取整。
  有符号和无符号移位在 君士坦丁堡（Constantinople）版本将有专门的操作码，
  目前由Solidity模拟。

* 在 ``do...while`` 循环中的 ``continue`` 语句现在跳转到条件，这是在这种情况下的常见行为。
  以前是跳到循环主体。因此，如果条件是假的，循环就终止了。

* 函数 ``.call()``， ``.delegatecall()`` 和 ``.staticcall()`` 在给定一个 ``bytes`` 参数时，
  不再进行填充。

* 如果EVM的版本是 拜占庭（Byzantium） 或更高版本，
  现在调用 pure 和 view 函数时使用操作码 ``STATICCALL`` 而不是 ``CALL``。
  这不允许在EVM层面上改变状态。

* 当在外部函数调用和 ``abi.encode`` 中使用时，
  ABI编码器现在可以正确地对来自 calldata（ ``msg.data`` 和外部函数参数）的字节数组和字符串进行填充。
  对于未填充的编码，请使用 ``abi.encodePacked``。

* 如果传入的 calldata 太短或指向界外，ABI解码器会在函数的开头和 ``abi.decode()`` 中回退。
  注意，脏的高阶位仍然会被忽略。

* 从蜜桔前哨（Tangerine Whistle）开始，用外部功能调用转发所有可用气体。

语义和语法的变化
==============================

本节重点介绍影响语法和语义的变化。

* 函数 ``.call()``， ``.delegatecall()``， ``staticcall()``， ``keccak256()``， ``sha256()``
  和 ``ripemd160()`` 现在只接受一个 ``bytes`` 参数。此外，该参数没有被填充。
  这样做是为了使参数的连接方式更加明确和清晰。
  将每个 ``.call()`` (和家族)改为 ``.call("")``，
  将每个 ``.call(signature, a,b, c)`` 改为 ``.call(abi.encodeWithSignature(signature, a, b, c))``
  （最后一项只对值类型有效）。
  将每个 ``keccak256(a, b, c)`` 改为 ``keccak256(abi.encodePacked(a, b, c))``。
  尽管这不是一个突破性的改变，建议开发者将 ``x.call(bytes4(keccak256("f(uint256)")), a, b)``
  改为 ``x.call(abi.encodeWithSignature("f(uint256)", a, b))``。

* 函数 ``.call()``， ``.delegatecall()`` 和 ``.staticcall()``
  现在返回 ``(bool, bytes memory)`` 以提供对返回数据的访问。
  将 ``bool success = otherContract.call("f")`` 改为
  ``(bool success, bytes memory data) = otherContract.call("f")``。

* Solidity 现在为函数局部变量实现了C99风格的范围规则，
  也就是说，变量只能在它们被声明后使用，并且只能在相同或嵌套的范围内使用。
  在 ``for`` 循环的初始化块中声明的变量在循环内部的任何一点都是有效的。

明确性要求
=========================

本节列出了现在的代码需要更加明确的变化。
对于大多数的主题，编译器会提供建议。

* 明确的函数可见性现在是强制性的。 在每个函数和构造函数中添加 ``public``，
  在每个未指定可见性的回退或接口函数中添加 ``external``。

* 所有结构，数组或映射类型的变量的明确数据位置现在是强制性的。
  这也适用于函数参数和返回变量。 例如，将 ``uint[] x = z`` 改为 ``uint[] storage x = z``，
  将 ``function f(uint[] [] x)`` 改为 ``function f(uint[] [] memory x)``，
  其中 ``memory`` 是数据位置，可以相应地替换为 ``storage`` 或 ``calldata``。
  注意， ``external`` 函数要求参数的数据位置为 ``calldata``。

* 合约类型不再包括 ``address`` 成员，以便分离命名空间。
  因此，现在有必要在使用 ``address`` 成员之前，明确地将合约类型的值转换为地址。
  例如：如果 ``c`` 是一个合约，把 ``c.transfer(...)`` 改为 ``address(c).transfer(...)``，
  把  ``c.balance`` 改为 ``address(c).balance``。

* 现在不允许在不相关的合约类型之间进行显式的转换。您只能从一个合约类型转换到它的一个基础或祖先类型。
  如果您确定一个合约与您想转换的合约类型是兼容的，尽管它没有继承它，
  您可以通过先转换为 ``address`` 来解决这个问题。
  例如：如果 ``A`` 和 ``B`` 是合约类型， ``B`` 不继承 ``A``，而 ``b`` 是 ``B`` 类型的合约，
  您仍然可以用 ``A(address(b))`` 将 ``b`` 转换成 ``A`` 类型。
  请注意，您仍然需要注意匹配的payable修饰的回退函数，如下文所述。

* ``address`` 类型被分成 ``address`` 和 ``address payable``，
  其中只有 ``address payable`` 提供 ``transfer`` 功能。
  一个 ``address payable`` 可以直接转换为 ``address``，
  但不允许以其他方式转换。将 ``address`` 转换为 ``address payable`` 是可以通过 ``uint160`` 转换的。
  如果 ``c`` 是一个合约， 只有当 ``c`` 有一个 payable 修饰的回退函数时，
  ``address(c)`` 的结果是 ``address payable``。
  如果您使用 :ref:`取回模式 <withdrawal_pattern>`，您很可能不必改变您的代码，
  因为 ``transfer`` 只用于 ``msg.sender`` 而不是存储地址，
  而且 ``msg.sender`` 是一个 ``address payable`` 类型。

* 现在不允许不同位数的 ``bytesX`` 和 ``uintY`` 之间的转换了，
  因为 ``bytesX`` 会在右侧填充， ``uintY`` 会在左侧填充，这可能导致意外的转换结果。
  现在在转换前必须在类型内调整位数。 例如，
  您想要将 ``bytes4`` （4字节）转换为 ``uint64`` （8字节），
  首先将 ``bytes4`` 变量转换为 ``bytes8``，然后再转换为 ``uint64``。
  当通过 ``uint32`` 转换时，您会得到相反的填充结果。
  在v0.5.0之前，任何 ``bytesX`` 和 ``uintY`` 之间的转换都要通过 ``uint8X``。
  例如， ``uint8(bytes3(0x291807))`` 将被转换为 ``uint8(uint24(bytes3(0x291807)))``
  （结果是 ``0x07``）。

* 在非payable函数中使用 ``msg.value`` （或通过修改器引入）是不允许的，因为这是一个安全特性。
  将该函数变成 ``payable``，或为程序逻辑创建一个新的内部函数，使用 ``msg.value``。

* 为了清晰起见，如果使用标准输入作为源，命令行界面现在需要 ``-``。

废弃的元素
===================

这一节列出了废弃以前的功能或语法的变化。 请注意，其中许多变化已经在实验模式 ``v0.5.0`` 中启用。

命令行和JSON接口
--------------------------------

* 命令行选项 ``--formal`` （用于生成Why3输出以进一步形式化验证）已被废弃，现在已被删除。
  一个新的形式化验证模块，SMTChecker，可以通过 ``pragma experimental SMTChecker;`` 启用。

* 由于中间语言 ``Julia`` 更名为 ``Yul``，命令行选项 ``--julia`` 被更名为 ``--yul``。

* 删除了 ``--clone-bin`` 和 ``--combined-json clone-bin`` 命令行选项。

* 不允许使用空前缀的重映射。

* JSON AST字段 ``constant`` 和 ``payable`` 被删除。
  这些信息现在出现在 ``stateMutability`` 字段中。

* ``FunctionDefinition`` 节点的JSON AST字段 ``isConstructor`` 被一个名为 ``kind`` 的字段取代，
  该字段的值可以是  ``"constructor"``， ``"fallback"`` 或 ``"function"``。

* 在非链接的二进制十六进制文件中，库地址占位符现在是完全等同的库名的keccak256哈希值的前36个十六进制字符，
  用 ``$...$`` 包围。以前，只使用完全等同的库名。这减少了碰撞的机会，特别是在使用长路径的时候。
  二进制文件现在也包含一个从这些占位符到完全等同名称的映射列表。

构造函数
------------

* 现在必须使用 ``constructor`` 关键字来定义构造函数。

* 现在不允许在没有括号的情况下调用基本构造函数。

* 现在不允许在同一继承层次中多次指定基本构造函数参数。

* 现在不允许调用有参数但参数个数错误的构造函数。
  如果您只是想指定一个继承关系而不是给参数，完全不要提供括号。

函数
---------

* 函数 ``callcode`` 现在被禁止使用（改用 ``delegatecall``）。
  但仍然可以通过内联汇编使用它。

* 现在不允许使用 ``suicide`` （改用 ``selfdestruct``）。

* 现在不允许使用 ``sha3`` （改用 ``keccak256``）。

* 现在不允许使用 ``throw`` （改用 ``revert``， ``require`` 和 ``assert``）。

转换
-----------

* 现在不允许从数字到 ``bytesXX`` 类型的显性和隐性转换。

* 现在不允许从十六进制字数到不同大小的 ``bytesXX`` 类型的显性和隐性转换。

字面常量和后缀
---------------------

* 由于闰年的复杂性和混乱性，现在不允许使用单位名称 ``years``。

* 现在不允许出现后面没有数字的尾部圆点。

* 现在不允许将十六进制数字与单位值相结合（例如： ``0x1e wei``）。

* 十六进制数字的前缀 ``0X`` 是不允许的，只能是 ``0x``。

变量
---------

* 为了清晰起见，现在不允许声明空结构。

* 现在不允许使用 ``var`` 关键字，以利于明确性。

* 现在不允许在具有不同组件数量的元组之间进行分配。

* 不允许使用不属于编译时常量的常量值。

* 现在不允许出现数值不匹配的多变量声明。

* 现在不允许出现未初始化的存储变量。

* 现在不允许使用空元组。

* 检测变量和结构中的循环依赖关系，在递归中被限制为256个。

* 现在不允许长度为零的固定长度数组。

语法
------

* 现在不允许使用 ``constant`` 作为函数状态的可变性修饰符。

* 布尔表达式不能使用算术运算。

* 现在不允许使用单数的 ``+`` 操作符。

* 如果没有事先转换为明确的类型，字面量不能再使用 ``abi.encodePacked``。

* 现在不允许有一个或多个返回值的函数的空返回语句。

* 现在完全不允许使用 "松散汇编" 语法，也就是说，
  不能再使用跳转标签，跳转和非功能指令。使用新的 ``while``， ``switch`` 和 ``if`` 结构代替。

* 没有实现的函数不能再使用修改器。

* 现在不允许具有命名返回值的函数类型。

* 现在不允许在不是程序块的 if/while/for 语句体中进行单语句变量声明。

* 新的关键字： ``calldata`` 和 ``constructor``。

* 新的保留关键字： ``alias``， ``apply``， ``auto``， ``copyof``，
  ``define``， ``immutable``， ``implements``， ``macro``， ``mutable``，
  ``override``， ``partial``， ``promise``， ``reference``， ``sealed``，
  ``sizeof``， ``supports``， ``typedef`` 和 ``unchecked``。

.. _interoperability:

与旧合约的互操作性
=====================================

通过为它们定义接口，仍然可以与为0.5.0之前的Solidity版本编写的合于对接（或者反过来）。
考虑到您已经部署了以下0.5.0之前的合约：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.4.25;
    // 在0.4.25版本的编译器之前，这将报告一个警告
    // 这在0.5.0之后将无法编译。
    contract OldContract {
        function someOldFunction(uint8 a) {
            //...
        }
        function anotherOldFunction() constant returns (bool) {
            //...
        }
        // ...
    }

这将不再在Solidity 0.5.0版本中进行编译。然而，您可以为它定义一个兼容的接口：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    interface OldContract {
        function someOldFunction(uint8 a) external;
        function anotherOldFunction() external returns (bool);
    }

请注意，我们没有声明 ``anotherOldFunction`` 是 ``view``，尽管它在原始合约中被声明为 ``constant``。
这是由于从Solidity 0.5.0版本开始，``staticcall`` 被用来调用 ``view`` 函数。
在 0.5.0 版本之前， ``constant`` 关键字没有被强制执行，
所以用 ``staticcall`` 调用一个被声明为 ``constant`` 的函数仍然可能被还原，
因为 ``constant`` 函数仍然可能试图修改存储。因此，当为旧合约定义接口时，
您应该只使用 ``view`` 来代替 ``constant``，以防您绝对确定该函数能与 ``staticcall`` 一起工作。

有了上面定义的接口，您现在可以很容易地使用已经部署的 0.5.0 之前的合约：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    interface OldContract {
        function someOldFunction(uint8 a) external;
        function anotherOldFunction() external returns (bool);
    }

    contract NewContract {
        function doSomething(OldContract a) public returns (bool) {
            a.someOldFunction(0x42);
            return a.anotherOldFunction();
        }
    }

同样，0.5.0以前的库可以通过定义库的功能而不需要实现，
并在连接时提供0.5.0以前的库的地址来使用
（关于如何使用命令行编译器进行连接，请参见 :ref:`commandline-compiler`）。

.. code-block:: solidity

    // 这在0.6.0版本之后将无法编译。
    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.5.0;

    library OldLibrary {
        function someFunction(uint8 a) public returns(bool);
    }

    contract NewContract {
        function f(uint8 a) public returns (bool) {
            return OldLibrary.someFunction(a);
        }
    }


示例
=======

下面的例子显示了Solidity 0.5.0 版本的合约及其更新版本，其中包括本节中列出的一些变化。

Old version:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.4.25;
    // 这在0.5.0版本之后将无法编译。

    contract OtherContract {
        uint x;
        function f(uint y) external {
            x = y;
        }
        function() payable external {}
    }

    contract Old {
        OtherContract other;
        uint myNumber;

        // 没有提供函数的可变性，不是错误。
        function someInteger() internal returns (uint) { return 2; }

        // 没有提供函数的可见性，不是错误。
        // 没有提供函数的可变性，不是错误。
        function f(uint x) returns (bytes) {
            // 在这个版本中，var是可以使用的。
            var z = someInteger();
            x += z;
            // 在这个版本中，throw是可以使用的。
            if (x > 100)
                throw;
            bytes memory b = new bytes(x);
            y = -3 >> 1;
            // y == -1（错，应该是-2）。
            do {
                x += 1;
                if (x > 10) continue;
                // 'Continue' 会导致无限循环。
            } while (x < 11);
            // 调用只返回一个布尔值。
            bool success = address(other).call("f");
            if (!success)
                revert();
            else {
                // 局部变量可以在其使用后声明。
                int y;
            }
            return b;
        }

        //不需要为'arr'设置明确的数据位置
        function g(uint[] arr, bytes8 x, OtherContract otherContract) public {
            otherContract.transfer(1 ether);

            // 由于uint32（4个字节）小于byte8（8个字节），
            // x的前4个字节将被丢失。
            // 这可能会导致意想不到的行为，因为bytesX是向右填充的。
            uint32 y = uint32(x);
            myNumber += y + msg.value;
        }
    }

新版本：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.5.0;
    // 这在0.6.0版本之后将无法编译。

    contract OtherContract {
        uint x;
        function f(uint y) external {
            x = y;
        }
        function() payable external {}
    }

    contract New {
        OtherContract other;
        uint myNumber;

        // 必须指定函数的可变性。
        function someInteger() internal pure returns (uint) { return 2; }

        // 必须指定函数的可见性。
        // 必须指定函数的可变性。
        function f(uint x) public returns (bytes memory) {
            // 现在必须明确地给出类型。
            uint z = someInteger();
            x += z;
            // 现在不允许使用throw。
            require(x <= 100);
            int y = -3 >> 1;
            require(y == -2);
            do {
                x += 1;
                if (x > 10) continue;
                // 'Continue'跳转到下面的条件。
            } while (x < 11);

            // call返回值为(bool, bytes).
            // 必须指定数据位置。
            (bool success, bytes memory data) = address(other).call("f");
            if (!success)
                revert();
            return data;
        }

        using AddressMakePayable for address;
        // 必须指定'arr'的数据位置
        function g(uint[] memory /* arr */, bytes8 x, OtherContract otherContract, address unknownContract) public payable {
            // 没有提供'otherContract.transfer'。
            // 由于'OtherContract'的代码是已知的，并且具有回退功能，
            // address(otherContract)具有'address payable'类型。
            address(otherContract).transfer(1 ether);

            // 没有提供'unknownContract.transfer'。
            // 没有提供'address(unknownContract).transfer'
            // 因为'address(unknownContract)'不是'address payable'类型。
            // 如果该函数需要一个您想发送资金的'address'类型，
            // 您可以通过'uint160'将其转换为'address payable'类型。
            // 注意：不建议这样做，应尽可能使用明确的'address payable'类型。
            // 为了提高明确性，我们建议使用一个库来进行转换（在这个例子中的合同后面提供）。
            address payable addr = unknownContract.makePayable();
            require(addr.send(1 ether));

            // 由于uint32（4字节）小于bytes8（8字节），
            // 所以不允许进行转换。
            // 我们需要先转换到一个通用的大小：
            bytes4 x4 = bytes4(x); // Padding happens on the right
            uint32 y = uint32(x4); // Conversion is consistent
            // 'msg.value'不能用在'非payable'类型的函数中。
            // 我们需要把函数变成payable类型
            myNumber += y + msg.value;
        }
    }

    // 我们可以定义一个库，将 ``address`` 类型明确转换为 ``address payable`` 类型，作为一种变通方法。
    library AddressMakePayable {
        function makePayable(address x) internal pure returns (address payable) {
            return address(uint160(x));
        }
    }
