.. index:: ! functions

.. _functions:

*********
函数
*********

可以在合约内部和外部定义函数。

合约之外的函数，也称为 "自由函数"，总是隐含着 ``internal`` 的 :ref:`可见性 <visibility and-getters>`。
它们的代码包含在所有调用它们的合约中，类似于内部库函数。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    function sum(uint[] memory _arr) pure returns (uint s) {
        for (uint i = 0; i < _arr.length; i++)
            s += _arr[i];
    }

    contract ArrayExample {
        bool found;
        function f(uint[] memory _arr) public {
            // 这在内部调用自由函数。
            // 编译器会将其代码添加到合约中。
            uint s = sum(_arr);
            require(s >= 10);
            found = true;
        }
    }

.. note::
    在合约之外定义的函数，仍然总是在合约的背景下执行。它们仍然可以访问变量 ``this``，
    可以调用其他合约，向它们发送以太，并销毁调用它们的合约，以及其他事项。
    与合约内定义的函数的主要区别是，自由函数不能直接访问不在其范围内的存储变量和函数。

.. _function-parameters-return-variables:

函数参数和返回变量
====================

与许多其他语言不同, 函数接受类型化的参数作为输入，
也可以返回任意数量的值作为输出。

函数参数
-------------------

函数参数的声明方式与变量相同，未使用的参数名称可以省略。

例如，如果您想让您的合约接受一种带有两个整数的外部调用，您可以使用类似以下的方式：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        uint sum;
        function taker(uint _a, uint _b) public {
            sum = _a + _b;
        }
    }

函数参数可以像任何其他局部变量一样使用，它们也可以被赋值。

.. note::

  一个 :ref:`外部函数 <external-function-calls>` 不能接受一个多维数组作为输入参数。
  如果您通过在源文件中添加 ``pragma abicoder v2;`` 来启用ABI编码器v2，就可以实现这个功能。

  一个 :ref:`内部函数 <external-function-calls>` 可以不启用该功能而接受一个多维数组。

.. index:: return array, return string, array, string, array of strings, dynamic array, variably sized array, return struct, struct

返回的变量
----------------

函数的返回变量在 ``returns`` 关键字之后用同样的语法声明。

例如，假设您想返回两个结果：作为函数参数传递的两个整数的总和和乘积，那么您就使用类似的方法：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint sum, uint product)
        {
            sum = _a + _b;
            product = _a * _b;
        }
    }

返回变量的名字可以被省略。返回变量可以像其他本地变量一样使用，
它们被初始化为相应的 :ref:`默认值 <default-value>`，
并且在它们被（重新）赋值之前拥有这个值。

您可以明确地赋值给返回变量，然后像上面那样结束函数，
或者您可以用 ``return`` 语句直接提供返回值（单个或 :ref:`多个返回值 <multi return>`）。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint sum, uint product)
        {
            return (_a + _b, _a * _b);
        }
    }

如果您过早使用 ``return`` 来结束一个有返回变量的函数，您必须在返回语句中同时提供返回值。

.. note::
    您不能从非内部函数返回某些类型，特别是多维动态数组和结构。
    如果您通过在源文件中添加 ``pragma abicoder v2;`` 来启用ABI编码器v2，
    那么就会有更多的类型可用，但 ``映射（mapping）`` 类型仍然被限制在单个合约内，您不能转移它们。

.. _multi-return:

返回多个值
-------------------------

当一个函数有多个返回类型时，语句 ``return (v0, v1, ..., vn)`` 可以用来返回多个值。
声明的数量必须与返回变量的数量相同，并且它们的类型必须匹配，
有可能是经过 :ref:`隐式转换 <types conversion-elementary-types>`。

.. _state-mutability:

状态可变性
================

.. index:: ! view function, function;view

.. _view-functions:

View 函数
--------------

函数可以被声明为 ``view``，在这种情况下，它们承诺不修改状态。

.. note::
  如果编译器的EVM版本是Byzantium或更新的（默认），
  当调用 ``view`` 函数时，会使用操作码 ``STATICCALL``，这使得状态作为EVM执行的一部分保持不被修改。
  对于库合约的 ``view`` 函数，会使用 ``DELEGATECALL``，
  因为没有组合的 ``DELEGATECALL`` 和 ``STATICCALL``。
  这意味着库合约中的 ``view`` 函数没有防止状态修改的运行时的检查。
  这应该不会对安全产生负面影响，因为库合约的代码通常在编译时就知道了，
  而且静态检查器也会进行编译时检查。

以下声明被认为是修改状态：

#. 修改状态变量。
#. :ref:`产生事件 <events>`。
#. :ref:`创建其它合约 <creating-contracts>`。
#. 使用 ``selfdestruct``。
#. 通过调用发送以太币。
#. 调用任何没有标记为 ``view`` 或者 ``pure`` 的函数。
#. 使用低级调用。
#. 使用包含特定操作码的内联汇编。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public view returns (uint) {
            return a * (b + 42) + block.timestamp;
        }
    }

.. note::
  函数上的 ``constant`` 曾经是 ``view`` 的别名，但在0.5.0版本中被取消。

.. note::
  Getter方法被自动标记为 ``view``。

.. note::
  在0.5.0版本之前，编译器没有为 ``view`` 函数使用 ``STATICCALL`` 操作码。
  这使得 ``view`` 函数通过使用无效的显式类型转换进行状态修改。
  通过对 ``view`` 函数使用 ``STATICCALL``，在EVM层面上防止了对状态的修改。

.. index:: ! pure function, function;pure

.. _pure-functions:

Pure 函数
--------------

函数可以被声明为 ``pure``，在这种情况下，它们承诺不读取或修改状态。
特别是，应该可以在编译时评估一个 ``pure`` 函数，只给它的输入和 ``msg.data``，
但不知道当前区块链状态。这意味着读取 ``immutable`` 的变量可以是一个非标准pure的操作。

.. note::
  如果编译器的EVM版本是Byzantium或更新的（默认），则使用操作码 ``STATICCALL``，
  这并不能保证不读取状态，但至少不能修改。

除了上面解释的状态修改语句列表外，以下内容被认为是从状态中读取的：

#. 读取状态变量。
#. 访问 ``address(this).balance`` 或者 ``<address>.balance``。
#. 访问 ``block``， ``tx``， ``msg`` 中任意成员 （除 ``msg.sig`` 和 ``msg.data`` 之外）。
#. 调用任何未标记为 ``pure`` 的函数。
#. 使用包含某些操作码的内联汇编。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

当一个 :ref:`错误发生 <assert-and-require>` 时，
Pure 函数能够使用 ``revert()`` 和 ``require()`` 函数来恢复潜在的状态变化。

恢复一个状态变化不被认为是 "状态修改"，
因为只有之前在没有 ``view`` 或 ``pure`` 限制的代码中对状态的改变才会被恢复，
并且该代码可以选择捕捉 ``revert`` 而不传递给它。

这种行为也与 ``STATICCALL`` 操作码一致。

.. warning::
  在EVM层面不可能阻止函数读取状态，只可能阻止它们写入状态
  （即只有 ``view`` 可以在EVM层面执行， ``pure`` 不可以）。

.. note::
  在0.5.0版本之前，编译器没有为 ``pure`` 函数使用 ``STATICCALL`` 操作码。
  这使得在 ``pure`` 函数中通过使用无效的显式类型转换进行状态修改。
  通过对 ``pure`` 函数使用 ``STATICCALL``，在EVM层面防止了对状态的修改。

.. note::
  在0.4.17版本之前，编译器并没有强制要求 ``pure`` 不读取状态。
  这是一个编译时的类型检查，可以规避在合约类型之间做无效的显式转换，
  因为编译器可以验证合约的类型不做改变状态的操作，
  但它不能检查将在运行时被调用的合约是否真的属于该类型。

.. _special-functions:

特殊的函数
=================

.. index:: ! receive ether function, function;receive ! receive

.. _receive-ether-function:

接收以太的函数
----------------------

一个合约最多可以有一个 ``receive`` 函数，
使用 ``receive() external payable { ... }`` 来声明。（没有  ``function`` 关键字）。
这个函数不能有参数，不能返回任何东西，必须具有 ``external`` 的可见性和 ``payable`` 的状态可变性。
它可以是虚拟的，可以重载，也可以有修饰器。

receive 函数是在调用合约时执行的，并带有空的 calldata。
这是在纯以太传输（例如通过 ``.send()`` 或 ``.transfer()`` ）时执行的函数。
如果不存在这样的函数，但存在一个 payable 类型的 :ref:`fallback函数 <fallback-function>`，
这个fallback函数将在纯以太传输时被调用。
如果既没有直接接收以太（receive函数），也没有可接收以太的 fallback 函数，
合约就不能通过常规交易接收以太，并抛出一个异常。

在最坏的情况下， ``receive`` 函数只有2300个气体可用（例如当使用 ``send`` 或 ``transfer`` 时），
除了基本的记录外，几乎没有空间来执行其他操作。以下操作的消耗气体将超过2300气体的规定：

- 写入存储
- 创建合约
- 调用消耗大量 gas 的外部函数
- 发送以太币

.. warning::
    直接接收以太的合约（没有函数调用，即使用 ``send`` 或 ``transfer``），
    但没有定义接收以太的函数或 payable 类型的 fallback 函数，会抛出一个异常，
    将以太送回（这在Solidity v0.4.0之前是不同的）。因此，如果您想让您的合约接收以太，
    您必须实现一个 receive 函数（不建议使用 payable 类型的 fallback 函数来接收以太，
    因为它不会因为接口混乱而失败）。


.. warning::
    没有接收以太币功能的合约可以作为 *coinbase交易*（又称 *矿工区块奖励*）的接收者
    或作为 ``selfdestruct`` 的目的地接收以太币。

    合约不能对这样的以太币转移做出反应，因此也不能拒绝它们。
    这是EVM的一个设计选择，Solidity无法绕过它。

    这也意味着 ``address(this).balance`` 可以高于合约中
    实现的一些手工记帐的总和（即在接收以太函数中更新的累加器）。

下面您可以看到一个使用 ``receive`` 函数的Sink合约的例子。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // 这个合约会保留所有发送给它的以太币，没有办法返还。
    contract Sink {
        event Received(address, uint);
        receive() external payable {
            emit Received(msg.sender, msg.value);
        }
    }

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Fallback 函数
-----------------

一个合约最多可以有一个 ``fallback`` 函数，使用 ``fallback () external [payable]``
或 ``fallback (bytes calldata _input) external [payable] returns (bytes memory _output)``
来声明（都没有 ``function`` 关键字）。
这个函数必须具有 ``external`` 的函数可见性。
一个fallback函数可以是虚拟的，可以重载，也可以有修饰器。

如果其他函数都不符合给定的函数签名，或者根本没有提供数据，
也没有 :ref:`接收以太的函数 <receive-ether-function>`，那么fallback函数将在调用合约时执行。
fallback函数总是接收数据，但为了同时接收以太，它必须被标记为 ``payable``。

如果使用带参数的版本， ``_input``  将包含发送给合约的全部数据（等于 ``msg.data``），
并可以在 ``_output`` 中返回数据。返回的数据将不会被ABI编码。
相反，它将在没有修改的情况下返回（甚至没有填充）。

在最坏的情况下，如果一个可接收以太的fallback函数也被用来代替接收功能，
那么它只有2300气体是可用的
（参见 :ref:`接收以太函数 <receive ether-function>` 对这一含义的简要描述）。

像任何函数一样，只要有足够的气体传递给它，fallback函数就可以执行复杂的操作。

.. warning::
    如果没有 :ref:`receive 函数 <receive ether-function>` 的存在，
    一个标记为 ``payable`` 的fallback函数也会在普通的以太传输时执行。
    如果您已经定义了一个 payable 类型的 fallback 函数，
    我们仍建议您也定义一个 receive 函数接收以太，以区分以太传输和接口混淆的情况。

.. note::
    如果您想对输入数据进行解码，您可以检查前四个字节的函数选择器，
    然后您可以使用 ``abi.decode`` 与数组切片语法一起对ABI编码的数据进行解码。
    ``(c, d) = abi.decode(_input[4:], (uint256, uint256));``
    注意，这只能作为最后的手段，应该使用适当的函数来代替。


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    contract Test {
        uint x;
        // 所有发送到此合约的消息都会调用此函数（没有其他函数）。
        // 向该合约发送以太币将引起异常，
        // 因为fallback函数没有 `payable` 修饰器。
        fallback() external { x = 1; }
    }

    contract TestPayable {
        uint x;
        uint y;
        // 所有发送到此合约的消息都会调用这个函数，
        // 除了普通的以太传输（除了receive函数，没有其他函数）。
        // 任何对该合约的非空的调用都将执行fallback函数（即使以太与调用一起被发送）。
        fallback() external payable { x = 1; y = msg.value; }

        // 这个函数是为纯以太传输而调用的，
        // 即为每一个带有空calldata的调用。
        receive() external payable { x = 2; y = msg.value; }
    }

    contract Caller {
        function callTest(Test test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // 结果是 test.x 等于 1。

            // address(test)将不允许直接调用 ``send``，
            // 因为 ``test`` 没有可接收以太的fallback函数。
            // 它必须被转换为 ``address payable`` 类型，才允许调用 ``send``。
            address payable testPayable = payable(address(test));

            // 如果有人向该合约发送以太币，转账将失败，即这里返回false。
            return testPayable.send(2 ether);
        }

        function callTestPayable(TestPayable test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // 结果是 test.x 等于 1，test.y 等于 0。
            (success,) = address(test).call{value: 1}(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // 结果是 test.x 等于 1，test.y 等于 1。

            // 如果有人向该合约发送以太币，TestPayable的receive函数将被调用。
            // 由于该函数会写入存储空间，它需要的气体比简单的 ``send`` 或 ``transfer`` 要多。
            // 由于这个原因，我们必须要使用一个低级别的调用。
            (success,) = address(test).call{value: 2 ether}("");
            require(success);
            // 结果是 test.x 等于 1，test.y 等于 2 个以太。

            return true;
        }
    }

.. index:: ! overload

.. _overload-function:

函数重载
====================

一个合约可以有多个同名的，但参数类型不同的函数。
这个过程被称为 "重载"，也适用于继承的函数。
下面的例子显示了在合约 ``A`` 范围内对函数 ``f`` 的重载。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint _in) public pure returns (uint out) {
            out = _in;
        }

        function f(uint _in, bool _really) public pure returns (uint out) {
            if (_really)
                out = _in;
        }
    }

重载函数也存在于外部接口中。如果两个外部可见函数仅区别于 Solidity 内的类型而不是它们的外部类型则会导致错误。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    // 这段代码不会编译
    contract A {
        function f(B _in) public pure returns (B out) {
            out = _in;
        }

        function f(address _in) public pure returns (address out) {
            out = _in;
        }
    }

    contract B {
    }


以上两个 ``f`` 函数重载最终都接受ABI的地址类型，尽管它们在Solidity中被认为是不同的。

重载解析和参数匹配
-----------------------------------------

通过将当前范围内的函数声明与函数调用中提供的参数相匹配，可以选择重载函数。
如果所有参数都可以隐式地转换为预期类型，则选择函数作为重载候选项。
如果一个候选都没有，解析失败。

.. note::
    返回参数不作为重载解析的依据。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint8 _in) public pure returns (uint8 out) {
            out = _in;
        }

        function f(uint256 _in) public pure returns (uint256 out) {
            out = _in;
        }
    }

调用 ``f(50)`` 会导致类型错误，因为 ``50`` 既可以被隐式转换为 ``uint8``
也可以被隐式转换为 ``uint256``。 另一方面，调用 ``f(256)`` 则会解析为 ``f(uint256)`` 重载，
因为 ``256`` 不能隐式转换为 ``uint8``。
