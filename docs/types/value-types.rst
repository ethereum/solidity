.. index:: ! value type, ! type;value
.. _value-types:

值类型
========

以下类型也称为值类型，因为这些类型的变量将始终按值来传递。
也就是说，当这些变量被用作函数参数或者用在赋值语句中时，总会进行值拷贝。

.. index:: ! bool, ! true, ! false

布尔类型
--------

``bool`` ：可能的取值为常数值 ``true`` 和 ``false``。

运算符：

*  ``!`` (逻辑非)
*  ``&&`` (逻辑与, "and")
*  ``||`` (逻辑或, "or")
*  ``==`` (等于)
*  ``!=`` (不等于)

运算符 ``||`` 和 ``&&`` 都遵循同样的短路（ short-circuiting ）规则。
就是说在表达式 ``f(x) || g(y)`` 中， 如果 ``f(x)`` 的值为 ``true`` ，
那么 ``g(y)`` 就不会被执行，即使会出现一些副作用。

.. index:: ! uint, ! int, ! integer
.. _integers:

整型
------

``int`` / ``uint``: 分别表示有符号和无符号的不同位数的整型变量。
关键字 ``uint8`` 到 ``uint256`` （无符号整型，从 8 位到 256 位）以及 ``int8`` 到 ``int256``，
以 8 位为步长递增。 ``uint`` 和 ``int`` 分别是 ``uint256`` 和 ``int256`` 的别名。

运算符：

* 比较运算符： ``<=``， ``<``， ``==``， ``!=``， ``>=``， ``>`` （返回布尔值）
* 位运算符： ``&``， ``|``， ``^`` (异或)， ``~`` (位取反)
* 移位运算符： ``<<`` （左移）， ``>>`` （右移）
* 算数运算符： ``+``， ``-``， 一元运算 ``-`` （只适用于有符号的整数）， ``*``， ``/``， ``%`` (取余)， ``**`` (幂)

对于一个整数类型 ``X``，您可以使用 ``type(X).min`` 和 ``type(X).max`` 来访问该类型代表的最小值和最大值。

.. warning::

  Solidity中的整数被限制在一个特定的范围内。例如，对于 ``uint32``，这是 ``0`` 到 ``2**32 - 1``。
  有两种模式在这些类型上进行算术。"包装" 或 "未检查" 模式和 "检查" 模式。
  默认情况下，算术总是 "检查" 模式的，这意味着如果一个操作的结果超出了该类型的值范围，
  调用将通过一个 :ref:`失败的断言 <assert-and-require>` 而被恢复。
  您可以用 ``unchecked { ... }``。 更多的细节可以在关于 :ref:`未检查 <unchecked>` 的章节中找到。

比较运算
^^^^^^^^^^^

比较的值是通过比较整数值得到的值。

位运算
^^^^^^^^^^^^^^

位操作是在数字的二进制补码表示上进行的。
这意味着，例如 ``~int256(0) == int256(-1)``。

移位运算
^^^^^^^^^^^

移位操作的结果具有左操作数的类型，将结果截断以符合类型。
右操作数必须是无符号类型，试图对有符号类型进行移位会产生一个编译错误。

移位可以通过以下方式用2的幂的乘法来 "模拟"。
请注意，对左边操作数类型的截断总是在最后进行，但没有明确提及。

- ``x << y`` 等同于数学表达式 ``x * 2**y``。
- ``x >> y`` 等同于数学表达式 ``x / 2**y``，向负无穷远的方向取整。

.. warning::
    在 ``0.5.0`` 版本之前，负数 ``x`` 的右移 ``x >> y`` 相当于数学表达式 ``x / 2**y`` 向零舍入，
    即右移使用向上舍入（向零舍入）而不是向下舍入（向负无穷大）。

.. note::
    就像对算术操作那样，对移位操作从不进行溢出检查。相反，结果总是被截断的。

加法、减法和乘法
^^^^^^^^^^^^^^^^^^

加法、减法和乘法具有通常的语义，在上溢和下溢方面有两种不同的模式：

默认情况下，所有的算术都会被检查是否有下溢或上溢，但这可以用 :ref:`未检查限制 <unchecked>` 来禁用。
这会导致包装的算术。更多细节可以在那一节中找到。

表达式 ``-x`` 等同于 ``(T(0) - x)``，其中
``T`` 是 ``x`` 的类型。它只能用于有符号的类型。
如果 ``x`` 是负的， ``-x`` 的值就是正的。
还有一个注意事项也是由二进制补码表示产生的：

如果您有（这样的表达式） ``int x = type(int).min;``，那么 ``-x`` 就不符合正数范围。
这意味着 ``unchecked { assert(-x == x); }`` 可以工作，
而表达式 ``-x`` 在检查模式下使用时将导致断言失败。

除法
^^^^^^^^

由于运算结果的类型总是操作数之一的类型，整数除法的结果总是一个整数。
在Solidity中，除法是向零进位的。这意味着 ``int256(-5) / int256(2) == int256(-2)``。

请注意，与此相反，在 :ref:`字面上 <rational_literals>` 的除法会产生任意精度的分数值。

.. note::
  除以0会导致 :ref:`异常 <assert-and-require>`。这个检查 **不能** 通过 ``unchecked { ... }`` 禁用。

.. note::
  表达式 ``type(int).min / (-1)`` 是除法导致溢出的唯一情况。
  在检查算术模式下，这将导致一个失败的断言，
  而在包装模式下，值将是 ``type(int).min``。

取余
^^^^^^

模数运算 ``a % n`` 是操作数 ``a`` 除以操作数 ``n`` 后产生余数 ``r``，
其中 ``q = int(a / n)`` 和 ``r = a - (n * q)``。
这意味着模数运算的结果与它的左边操作数（或零）相同，
``a % n == -(-a % n)`` 对负的 ``a`` 来说成立。

* ``int256(5) % int256(2) == int256(1)``
* ``int256(5) % int256(-2) == int256(1)``
* ``int256(-5) % int256(2) == int256(-1)``
* ``int256(-5) % int256(-2) == int256(-1)``

.. note::
  对0取余会导致 :ref:`异常 <assert-and-require>`。这个检查 **不能** 通过 ``unchecked { ... }`` 禁用。

幂运算
^^^^^^

幂运算只适用于指数中的无符号类型。幂运算的结果类型总是等于基数的类型。
请注意，它要足够大以容纳结果，并为潜在的断言失败或包装行为做好准备。

.. note::
  在检查模式下，幂运算只对小基数使用相对便宜的 ``exp`` 操作码。
  对于 ``x**3`` 的情况，表达式 ``x*x*x`` 可能更便宜。
  在任何情况下，气体成本测试和使用优化器都是可取的。

.. note::
  请注意，``0**0`` 被EVM定义为 ``1``。

.. index:: ! ufixed, ! fixed, ! fixed point number

定长浮点型
------------

.. warning::
    Solidity 还没有完全支持定长浮点型。可以声明定长浮点型的变量，
    但不能给它们赋值或把它们赋值给其他变量。

``fixed`` / ``ufixed``：表示各种大小的有符号和无符号的定长浮点型。
在关键字 ``ufixedMxN`` 和 ``fixedMxN`` 中， ``M`` 表示该类型占用的位数，
``N`` 表示可用的小数位数。 ``M`` 必须能整除 8，即 8 到 256 位。
``N`` 则可以是从 0 到 80 之间的任意数。 ``ufixed`` 和 ``fixed`` 分别是 ``ufixed128x18`` 和 ``fixed128x18`` 的别名。

运算符：

* 比较运算符： ``<=``， ``<``， ``==``， ``!=``， ``>=``， ``>`` （返回值是布尔型）
* 算术运算符： ``+``， ``-``， 一元运算 ``-``， ``*``， ``/``， ``%`` (取余数)

.. note::
    浮点型（在许多语言中的 ``float`` 和 ``double`` ，更准确地说是 IEEE 754 类型）和定长浮点型之间最大的不同点是，
    在前者中整数部分和小数部分（小数点后的部分）需要的位数是灵活可变的，而后者中这两部分的长度受到严格的规定。
    一般来说，在浮点型中，几乎整个空间都用来表示数字，但只有少数的位来表示小数点的位置。

.. index:: address, balance, send, call, delegatecall, staticcall, transfer

.. _address:

地址类型
---------

地址类型有两种，大体上是相同的：

- ``address``: 保存一个20字节的值（一个以太坊地址的大小）。
- ``address payable``: 与 ``address`` 类型相同，但有额外的方法 ``transfer`` 和 ``send``。

这种区别背后的想法是， ``address payable`` 是一个您可以发送以太币的地址，
而您不应该发送以太币给一个普通的 ``address``，例如，因为它可能是一个智能合约，
而这个合约不是为接受以太币而建立的。

类型转换：

允许从 ``address payable`` 到 ``address`` 的隐式转换，
而从 ``address`` 到 ``address payable`` 的转换必须通过 ``payable(<address>)`` 来明确。

对于 ``uint160``、整数、 ``bytes20`` 和合约类型，允许对 ``address`` 进行明确的转换和输出。

只有 ``address`` 类型和合约类型的表达式可以通过 ``payable(...)`` 显式转换为 ``address payable`` 类型。
对于合约类型，只有在合约可以接收以太的情况下才允许这种转换，也就是说，
合约要么有一个 :ref:`receive <receive-ether function>` 函数，要么有一个 payable 类型的 fallback 的函数。
请注意， ``payable(0)`` 是有效的，是这个规则的例外。

.. note::
    如果您需要一个 ``address`` 类型的变量，并计划向其发送以太，那么就将其类型声明为 ``address payable``，
    以使这一要求可行。另外，尽量尽早地进行这种区分或转换。

运算符：

* ``<=``, ``<``, ``==``, ``!=``, ``>=`` 和 ``>``

.. warning::
    如果您使用较大字节的类型转换为 ``address``，例如 ``bytes32``，那么 ``address`` 就被截断了。
    为了减少转换的模糊性，0.4.24及以上版本的编译器强迫你在转换中明确截断。以32字节的值
    ``0x111122333344556677888899AAAABBBBCCCCDDDDEEFFFFCCCC`` 为例。

    您可以使用 ``address(uint160(bytes20(b)))``，结果是 ``0x111122223333444455556666777788889999aAaa``，
    或者您可以使用 ``address(uint160(uint256(b)))``，结果是 ``0x777788889999AaAAbBbbCcccddDdeeeEfFFfCcCc``。

.. note::
    ``address`` 和 ``address payable`` 之间的区别是在0.5.0版本中引入的。
    同样从该版本开始，合约不从地址类型派生，但仍然可以明确转换为 ``address`` 或 ``address payable``，
    如果它们有一个 receive 或 payable 类型的 fallback 函数。

.. _members-of-addresses:

地址类型成员变量
^^^^^^^^^^^^^^^^^^^^

快速参考，请见 :ref:`地址相关`。

* ``balance`` 和 ``transfer``

可以使用 ``balance`` 属性来查询一个地址的以太币余额，
也可以使用 ``transfer`` 函数向一个地址发送以太币（以 wei 为单位）：

.. code-block:: solidity
    :force:

    address payable x = payable(0x123);
    address myAddress = address(this);
    if (x.balance < 10 && myAddress.balance >= 10) x.transfer(10);

如果当前合约的余额不足，或者以太币转账被接收账户拒绝，那么 ``transfer`` 功能就会失败。
``transfer`` 功能在失败后会被还原。

.. note::
    如果 ``x`` 是一个合约地址，它的代码（更具体地说：它的 :ref:`接收以太币函数`，如果有的话，
    或者它的 :ref:`fallback 函数`，如果有的话）将与 ``transfer`` 调用一起执行（这是EVM的一个特性，无法阻止）。
    如果执行过程中耗尽了气体或出现了任何故障，以太币的转移将被还原，当前的合约将以异常的方式停止。

* ``send``

Send是 ``transfer`` 的低级对应部分。如果执行失败，当前的合约不会因异常而停止，但 ``send`` 会返回 ``false``。

.. warning::
    使用 ``send`` 有一些危险：如果调用堆栈深度为1024，传输就会失败（这可以由调用者强制执行），
    如果接收者的气体耗尽，也会失败。因此，为了安全地进行以太币转账，
    一定要检查 ``send`` 的返回值，或者使用 ``transfer``，甚至使用更好的方式：
    使用收款人提款的模式。

* ``call``, ``delegatecall`` 和 ``staticcall``

为了与不遵守ABI的合约对接，或者为了更直接地控制编码，
我们提供了 ``call``, ``delegatecall`` 和 ``staticcall`` 函数。
它们都接受一个 ``bytes memory`` 参数，并返回成功条件（作为一个 ``bool``）
和返回的数据（ ``bytes memory``）。
函数 ``abi.encode``, ``abi.encodePacked``, ``abi.encodeWithSelector``
和 ``abi.encodeWithSignature`` 可以用来编码结构化的数据。

示例：

.. code-block:: solidity

    bytes memory payload = abi.encodeWithSignature("register(string)", "MyName");
    (bool success, bytes memory returnData) = address(nameReg).call(payload);
    require(success);

.. warning::
    所有这些函数都是低级别的函数，应该谨慎使用。
    具体来说，任何未知的合约都可能是恶意的，如果您调用它，
    您就把控制权交给了该合约，而该合约又可能回调到您的合约中，
    所以要准备好在调用返回时改变您合约的状态变量。
    与其他合约互动的常规方法是在合约对象上调用一个函数（ ``x.f()``）。

.. note::
    以前的 Solidity 版本允许这些函数接收任意的参数，
    并且也会以不同的方式处理 ``bytes4`` 类型的第一个参数。
    这些边缘情况在0.5.0版本中被移除。

可以用 ``gas`` 修饰器来调整所提供的气体：

.. code-block:: solidity

    address(nameReg).call{gas: 1000000}(abi.encodeWithSignature("register(string)", "MyName"));

同样，所提供的以太值也可以被控制：

.. code-block:: solidity

    address(nameReg).call{value: 1 ether}(abi.encodeWithSignature("register(string)", "MyName"));

最后，这些修饰器可以合并。它们的顺序并不重要：

.. code-block:: solidity

    address(nameReg).call{gas: 1000000, value: 1 ether}(abi.encodeWithSignature("register(string)", "MyName"));

以类似的方式，可以使用函数 ``delegatecall``：不同的是，它只使用给定地址的代码，
所有其他方面（存储，余额，...）都取自当前的合约。
``delegatecall`` 的目的是为了使用存储在另一个合约中的库代码。
用户必须确保两个合约中的存储结构都适合使用delegatecall。

.. note::
    在 homestead 版本之前，只有一个功能类似但作用有限的 ``callcode`` 的函数可用，
    但它不能获取委托方的 ``msg.sender`` 和 ``msg.value``。这个功能在0.5.0版本中被移除。

从byzantium开始，也可以使用 ``staticcall``。这基本上与 ``call`` 相同，
但如果被调用的函数以任何方式修改了状态，则会恢复。

这三个函数 ``call``， ``delegatecall`` 和 ``staticcall`` 都是非常低级的函数，
只应该作为 *最后的手段* 来使用，因为它们破坏了Solidity的类型安全。

``gas`` 选项在所有三种方法中都可用，而 ``value`` 选项只在 ``call`` 中可用。

.. note::
    最好避免在您的智能合约代码中依赖硬编码的气体值，无论状态是读出还是写入，
    因为这可能有很多隐患。另外，对气体的访问在未来可能会改变。

* ``code`` 和 ``codehash``

您可以查询任何智能合约的部署代码。使用 ``.code`` 获得作为 ``bytes memory`` 的EVM字节码，
这可能是空的。使用 ``.codehash`` 获得该代码的Keccak-256哈希值（作为 ``bytes32``）。
注意，使用 ``addr.codehash`` 比 ``keccak256(addr.code)`` 更便宜。

.. note::
    所有的合约都可以转换为 ``address`` 类型，所以可以用 ``address(this).balance`` 查询当前合约的余额。

.. index:: ! contract type, ! type; contract

.. _contract_types:

合约类型
---------

每个 :ref:`合约 <contracts>` 都定义了自己的类型。
您可以隐式地将一个合约转换为它们所继承的另一个合约。
合约可以显式地转换为 ``address`` 类型，也可以从 ``address`` 类型中转换。

只有在合约类型具有receive或 payable 类型的 fallback 函数的情况下，
才有可能明确转换为 ``address payable`` 类型和从该类型转换。
这种转换仍然使用 ``address(x)`` 进行转换。如果合约类型没有一个 receive 或 payable 类型的 fallback 函数，
可以使用 ``payable(address(x))`` 来转换为 ``address payable`` 。
您可以在 :ref:`地址类型 <address>` 一节中找到更多信息。

.. note::
    在0.5.0版本之前，合约直接从地址类型派生出来，
    并且在 ``address`` 和 ``address payable`` 之间没有区别。

如果您声明了一个本地类型的变量（ ``MyContract c`` ），您可以调用该合约上的函数。
注意要从相同合约类型的地方将其赋值。

您也可以实例化合约（这意味着它们是新创建的）。
您可以在 :ref:`'通过关键字new创建合约' <creating-contracts>` 部分找到更多细节。

合约的数据表示与 ``address`` 类型相同，该类型也用于 :ref:`ABI<ABI>`。

合约不支持任何运算符。

合约类型的成员是合约的外部函数，包括任何标记为 ``public`` 的状态变量。

对于一个合约 ``C``，您可以使用 ``type(C)`` 来访问
关于该合约的 :ref:`类型信息 <meta-type>` 。

.. index:: byte array, bytes32

定长字节数组
------------

值类型 ``bytes1``, ``bytes2``, ``bytes3``, ..., ``bytes32`` 代表从1到32的字节序列。

运算符：

比较运算符：<=， <， ==， !=， >=， > （返回布尔型）

* 比较运算符： ``<=``， ``<``， ``==``， ``!=``， ``>=``， ``>`` (返回 ``bool``)
* 位运算符： ``&``， ``|``， ``^`` （按位异或）， ``~`` （按位取反）
* 移位运算符： ``<<`` （左移位）， ``>>`` （右移位）
* 索引访问： 如果 ``x`` 是 ``bytesI`` 类型，那么当 ``0 <= k < I`` 时， ``x[k]`` 返回第 ``k`` 个字节（只读）。

移位运算符以无符号的整数类型作为右操作数（但返回左操作数的类型），
它表示要移位的位数。有符号类型的移位将产生一个编译错误。

成员变量：

* ``.length`` 表示这个字节数组的长度（只读）.

.. note::
    类型 ``bytes1[]`` 是一个字节数组，但是由于填充规则，它为每个元素浪费了31个字节的空间（在存储中除外）。
    因此最好使用 ``bytes`` 类型来代替。

.. note::
    在0.8.0版本之前， ``byte`` 曾经是 ``bytes1`` 的别名。

变长字节数组
------------

``bytes``:
    变长字节数组，参见 :ref:`数组`。它并不是值类型！
``string``:
    变长 UTF-8 编码字符串类型，参见 :ref:`数组`。并不是值类型！

.. index:: address, literal;address

.. _address_literals:

地址字面常数（Address Literals）
---------------------------------

比如像 ``0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF`` 这样的
通过了地址校验测试的十六进制字属于 ``address`` 类型。
十六进制字数在39到41位之间，并且没有通过校验测试，会产生一个错误。
您可以预加（对于整数类型）或附加（对于bytesNN类型）零来消除该错误。

.. note::
    混合大小写的地址校验和格式定义在 `EIP-55 <https://github.com/ethereum/EIPs/blob/master/EIPS/eip-55.md>`_。

.. index:: literal, literal;rational

.. _rational_literals:

有理数和整数字面常数
-----------------------------

整数字面常数由范围在 0-9 的一串数字组成，表现成十进制。
例如， ``69`` 表示十进制数字 69。 Solidity 中是没有八进制的，因此前置 0 是无效的。

十进制小数字面常数带有一个 ``.``，至少在其一边会有一个数字。 比如： ``1.``, ``.1`` 和 ``1.3``。

也支持 ``2e10`` 形式的科学符号，其中尾数可以是小数，但指数必须是一个整数。
字面的 ``MeE`` 相当于 ``M * 10**E``。
例子包括 ``2e10``, ``-2e10``, ``2e-10``, ``2.5e1``。

下划线可以用来分隔数字字面的数字，以帮助阅读。
例如，十进制 ``123_000``，十六进制 ``0x2eff_abde``，科学十进制 ``1_2e345_678`` 都是有效的。
下划线只允许在两个数字之间，并且只允许一个连续的下划线。
含有下划线的数字字面没有额外的语义，下划线被忽略。

数值字面常数表达式保留任意精度，直到它们被转换为非字面常数类型
（即通过与非字面常数类型一起使用或通过显式转换）。
这意味着在数值常量表达式中，计算不会溢出，除法不会截断。

例如， ``(2**800 + 1) - 2**800`` 的结果是常数 ``1`` （类型 ``uint8``），
尽管中间的结果甚至不符合机器字的大小。此外， ``.5 * 8`` 的结果是整数 ``4`` （尽管中间使用了非整数）。

只要操作数是整数，任何可以应用于整数的操作数也可以应用于数值字面常数表达式。
如果两者中的任何一个是小数，则不允许进行位操作，
如果指数是小数，则不允许进行幂运算（因为这可能导致无理数）。

以数值字面常数表达式为左（或基数）操作数，以整数类型为右（指数）操作数的移位和幂运算，
总是在 ``uint256`` （非负数数值字面常数）或 ``int256`` （负数数值字面常数）类型中进行。
无论右（指数）操作数的类型如何。

.. warning::
    在0.4.0版本之前，Solidity中整数字的除法会被截断，但现在它转换为一个有理数，即 ``5 / 2`` 不等于 ``2``，而是 ``2.5``。

.. note::
    Solidity 对每个有理数都有对应的数值字面常数类型。
    整数字面常数和有理数字面常数都属于数值字面常数类型。
    除此之外，所有的数值字面常数表达式（即只包含数值字面常数和运算符的表达式）都属于数值字面常数类型。
    因此数值字面常数表达式 ``1 + 2`` 和 ``2 + 1`` 的结果跟有理数3的数值字面常数类型相同。

.. note::
    数字字面表达式一旦与非字面表达式一起使用，就会被转换为非字面类型。
    不考虑类型，下面分配给 ``b`` 的表达式的值被评估为一个整数。
    因为 ``a`` 的类型是 ``uint128``，所以表达式 ``2.5 + a`` 必须有一个合适的类型。
    由于 ``2.5`` 和 ``uint128`` 的类型没有共同的类型，Solidity编译器不接受这段代码。

.. code-block:: solidity

    uint128 a = 1;
    uint128 b = 2.5 + a + 0.5;

.. index:: literal, literal;string, string
.. _string_literals:

字符串字面常数和类型
-------------------------

字符串字面常数是指由双引号或单引号引起来的字符串（ ``"foo"`` 或者 ``'bar'``）。
它们也可以分成多个连续部分（ ``"foo" "bar"`` 相当于 ``"foobar"`` ），这在处理长字符串时很有帮助。
它们不像在 C 语言中那样带有结束符； ``"foo"`` 相当于3个字节而不是4个。
和整数字面常数一样，字符串字面常数的类型也可以发生改变，
但它们可以隐式地转换成 ``bytes1``，……， ``bytes32``，如果合适的话，还可以转换成 ``bytes`` 以及 ``string``。

例如，使用 ``bytes32 samevar = "stringliteral"``，
当分配给 ``bytes32`` 类型时，字符串字面常数被解释成原始字节形式。

字符串字面常数只能包含可打印的ASCII字符，也就是0x20 ... 0x7E之间的字符。

此外，字符串字元还支持以下转义字符：

- ``\<newline>`` （转义一个实际的换行）
- ``\\`` （反斜杠）
- ``\'`` （单引号）
- ``\"`` （双引号）
- ``\n`` （换行）
- ``\r`` （回车键）
- ``\t`` （制表）
- ``\xNN`` （十六进制转义，见下文）
- ``\uNNNN`` （unicode转义，见下文）

``\xNN`` 接收一个十六进制值并插入相应的字节，而 ``\uNNNN`` 接收一个Unicode编码点并插入一个UTF-8序列。

.. note::

    在0.8.0版本之前，有三个额外的转义序列。 ``\b``， ``\f`` 和 ``v``。
    它们在其他语言中通常是可用的，但在实践中很少需要。
    如果您确实需要它们，仍然可以通过十六进制转义插入，
    即分别为 ``\x08``， ``x0c`` 和 ``\x0b``，就像其他ASCII字符一样。

下面例子中的字符串的长度为10个字节。
它以一个换行字节开始，接着是一个双引号，一个单引号，一个反斜杠字符，
然后（没有分隔符）是字符序列 ``abcdef``。

.. code-block:: solidity
    :force:

    "\n\"\'\\abc\
    def"

任何非换行的Unicode行结束符（即LF, VF, FF, CR, NEL, LS, PS）都被认为是字符串字面的结束。
换行只在字符串字面内容前面没有 ``\`` 的情况下终止。

Unicode 字面常数
----------------

普通字符串字面常数只能包含ASCII码，而Unicode字面常数--以关键字 ``unicode`` 为前缀--可以包含任何有效的UTF-8序列。
它们也支持与普通字符串字面意义相同的转义序列。

.. code-block:: solidity

    string memory a = unicode"Hello 😃";

.. index:: literal, bytes

十六进制字面常数
--------------------

十六进制字面常数以关键字 ``hex`` 打头，
后面紧跟着用单引号或双引号引起来的字符串（ ``hex"001122FF"``, ``hex'0011_22_FF'``）。
它们的内容必须是十六进制的数字，可以选择使用一个下划线作为字节边界之间的分隔符。
字面的值将是十六进制序列的二进制表示。

由空格分隔的多个十六进制字面常数被串联成一个字面常数：
``hex"00112233" hex"44556677"`` 相当于 ``hex"0011223344556677"``。

十六进制字面常数的行为与 :ref:`字符串字面常数 <string_literals>` 类似，并有相同的可转换性限制。

.. index:: enum

.. _enums:

枚举类型
---------

枚举是在 Solidity 中创建用户定义类型的一种方式。
它们可以显式地转换为所有整数类型，和从整数类型来转换，但不允许隐式转换。
从整数的显式转换在运行时检查该值是否在枚举的范围内，否则会导致 :ref:`异常<assert-and-require>`。
枚举要求至少有一个成员，其声明时的默认值是第一个成员。
枚举不能有超过256个成员。

数据表示与C语言中的枚举相同。选项由后续的从 ``0`` 开始无符号整数值表示。

使用 ``type(NameOfEnum).min`` 和 ``type(NameOfEnum).max``
您可以得到给定枚举的最小值和最大值。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.8;

    contract test {
        enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
        ActionChoices choice;
        ActionChoices constant defaultChoice = ActionChoices.GoStraight;

        function setGoStraight() public {
            choice = ActionChoices.GoStraight;
        }

        // 由于枚举类型不属于ABI的一部分，因此对于所有来自 Solidity 外部的调用，
        // "getChoice" 的签名会自动被改成 "getChoice() returns (uint8)"。
        function getChoice() public view returns (ActionChoices) {
            return choice;
        }

        function getDefaultChoice() public pure returns (uint) {
            return uint(defaultChoice);
        }

        function getLargestValue() public pure returns (ActionChoices) {
            return type(ActionChoices).max;
        }

        function getSmallestValue() public pure returns (ActionChoices) {
            return type(ActionChoices).min;
        }
    }

.. note::
    枚举也可以在文件级别上声明，在合约或库定义之外。

.. index:: ! user defined value type, custom type

.. _user-defined-value-types:

用户定义的值类型
-----------------

一个用户定义的值类型允许在一个基本的值类型上创建一个零成本的抽象。
这类似于一个别名，但有更严格的类型要求。

一个用户定义的值类型是用 ``type C is V`` 定义的，其中 ``C`` 是新引入的类型的名称，
``V`` 必须是一个内置的值类型（"底层类型"）。
函数 ``C.wrap`` 被用来从底层类型转换到自定义类型。同样地，
函数 ``C.unwrap`` 用于从自定义类型转换到底层类型。

类型 ``C`` 没有任何运算符或约束成员函数。特别的是，甚至运算符 ``==`` 也没有定义。
也不允许与其他类型进行显式和隐式转换。

这种类型的值的数据表示是从底层类型中继承的，底层类型也被用于ABI中。

下面的例子说明了一个自定义类型 ``UFixed256x18``，
代表一个有18位小数的十进制定点类型和一个最小的库来对该类型做算术运算。


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.8;

    // 使用用户定义的值类型表示一个18位小数，256位宽的定点类型。
    type UFixed256x18 is uint256;

    /// 一个在UFixed256x18上进行定点操作的最小库。
    library FixedMath {
        uint constant multiplier = 10**18;

        /// 将两个UFixed256x18的数字相加。溢出时将返回，依靠uint256的算术检查。
        function add(UFixed256x18 a, UFixed256x18 b) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(UFixed256x18.unwrap(a) + UFixed256x18.unwrap(b));
        }
        /// 将UFixed256x18和uint256相乘。溢出时将返回，依靠uint256的算术检查。
        function mul(UFixed256x18 a, uint256 b) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(UFixed256x18.unwrap(a) * b);
        }
        /// 对一个UFixed256x18类型的数字相下取整。
        /// @return 不超过 `a` 的最大整数。
        function floor(UFixed256x18 a) internal pure returns (uint256) {
            return UFixed256x18.unwrap(a) / multiplier;
        }
        /// 将一个uint256转化为相同值的UFixed256x18。
        /// 如果整数太大，则恢复。
        function toUFixed256x18(uint256 a) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(a * multiplier);
        }
    }

注意 ``UFixed256x18.wrap`` 和 ``FixedMath.toUFixed256x18`` 有相同的签名，
但执行两个非常不同的操作。 ``UFixed256x18.wrap`` 函数返回一个与输入的数据表示相同的 ``UFixed256x18``，
而 ``toUFixed256x18`` 则返回一个具有相同数值的 ``UFixed256x18``。

.. index:: ! function type, ! type; function

.. _function_types:

函数类型
----------

函数类型是一种表示函数的类型。可以将一个函数赋值给另一个函数类型的变量，
也可以将一个函数作为参数进行传递，还能在函数调用中返回函数类型变量。
函数类型有两类：- *内部（internal）* 函数和 *外部（external）* 函数：

内部函数只能在当前合约内被调用（更具体来说，
在当前代码块内，包括内部库函数和继承的函数中），
因为它们不能在当前合约上下文的外部被执行。
调用一个内部函数是通过跳转到它的入口标签来实现的，
就像在当前合约的内部调用一个函数。

外部函数由一个地址和一个函数签名组成，可以通过外部函数调用传递或者返回。

函数类型表示成如下的形式：

.. code-block:: solidity
    :force:

    function (<parameter types>) {internal|external} [pure|view|payable] [returns (<return types>)]

与参数类型相反，返回类型不能为空 —— 如果函数类型不需要返回，
则需要删除整个 ``returns (<return types>)`` 部分。

默认情况下，函数类型是内部函数，所以可以省略 ``internal`` 关键字。
注意，这只适用于函数类型。对于合约中定义的函数，
必须明确指定其可见性，它们没有默认类型。

转换：

当且仅当它们的参数类型相同，它们的返回类型相同，它们的内部/外部属性相同，
并且 ``A`` 的状态可变性比 ``B`` 的状态可变性更具限制性时，
一个函数类型 ``A`` 就可以隐式转换为一个函数类型 ``B``。特别是：

- ``pure`` 函数可以转换为 ``view`` 和 ``non-payable`` 函数
- ``view`` 函数可以转换为 ``non-payable`` 函数
- ``payable`` 函数可以转换为 ``non-payable`` 函数

其他函数类型之间的转换是不可能的。

关于 ``payable`` 和 ``non-payable`` 的规则可能有点混乱，
但实质上，如果一个函数是 ``payable``，这意味着
它也接受零以太的支付，所以它也是 ``non-payable``。
另一方面，一个 ``non-payable`` 的函数将拒绝发送给它的以太，
所以 ``non-payable`` 的函数不能被转换为 ``payable`` 的函数。

如果一个函数类型的变量没有被初始化，调用它将导致
会出现 :ref:`异常<assert-and-require>`。如果你在一个函数上使用了 ``delete`` 之后再调用它，
也会发生同样的情况。

如果外部函数类型在Solidity的上下文中被使用，
它们将被视为 ``function`` 类型，它将地址和函数标识符一起编码为一个 ``bytes24`` 类型。

请注意，当前合约的公开函数既可以被当作内部函数也可以被当作外部函数使用。
如果想将一个函数当作内部函数使用，就用 ``f`` 调用，
如果想将其当作外部函数，使用 ``this.f`` 。

一个内部类型的函数可以被分配给一个内部函数类型的变量，而不管它在哪里被定义。
这包括合约和库合约的隐私、内部和公共函数，以及自由函数。
另一方面，外部函数类型只与公共和外部合约函数兼容。
库合约被排除在外，因为它们需要一个 ``delegatecall``，
并且 :ref:`对它们的选择器使用不同的ABI约定 <library-selectors>`。
在接口中声明的函数没有定义，所以指向它们也没有意义。

成员：
外部（或公共）函数有以下成员：

* ``.address`` 返回该函数的合约地址。
* ``.selector`` 返回 :ref:`ABI 函数选择器 <abi_function_selector>`

.. note::
  外部（或公共）函数曾经有额外的成员 ``.gas(uint)`` 和 ``.value(uint)``。
  这些在Solidity 0.6.2中被废弃，并在Solidity 0.7.0中被移除。取而代之的是
  使用 ``{gas: ...}`` 和 ``{value: ...}`` 来分别指定发送到函数的气体量或以太（wei为单位）量。
  参见 :ref:`外部函数调用 <external- function-calls>` 以获得更多信息。

以下例子展示如何使用这些成员：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.4 <0.9.0;

    contract Example {
        function f() public payable returns (bytes4) {
            assert(this.f.address == address(this));
            return this.f.selector;
        }

        function g() public {
            this.f{gas: 10, value: 800}();
        }
    }

以下例子展示如何使用内部函数类型：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    library ArrayUtils {
        // 内部函数可以在内部库函数中使用，因为它们将是同一代码上下文的一部分
        function map(uint[] memory self, function (uint) pure returns (uint) f)
            internal
            pure
            returns (uint[] memory r)
        {
            r = new uint[](self.length);
            for (uint i = 0; i < self.length; i++) {
                r[i] = f(self[i]);
            }
        }

        function reduce(
            uint[] memory self,
            function (uint, uint) pure returns (uint) f
        )
            internal
            pure
            returns (uint r)
        {
            r = self[0];
            for (uint i = 1; i < self.length; i++) {
                r = f(r, self[i]);
            }
        }

        function range(uint length) internal pure returns (uint[] memory r) {
            r = new uint[](length);
            for (uint i = 0; i < r.length; i++) {
                r[i] = i;
            }
        }
    }


    contract Pyramid {
        using ArrayUtils for *;

        function pyramid(uint l) public pure returns (uint) {
            return ArrayUtils.range(l).map(square).reduce(sum);
        }

        function square(uint x) internal pure returns (uint) {
            return x * x;
        }

        function sum(uint x, uint y) internal pure returns (uint) {
            return x + y;
        }
    }

另一个使用外部函数类型的例子：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;


    contract Oracle {
        struct Request {
            bytes data;
            function(uint) external callback;
        }

        Request[] private requests;
        event NewRequest(uint);

        function query(bytes memory data, function(uint) external callback) public {
            requests.push(Request(data, callback));
            emit NewRequest(requests.length - 1);
        }

        function reply(uint requestID, uint response) public {
            // 这里要检查的是调用返回是否来自可信的来源
            requests[requestID].callback(response);
        }
    }


    contract OracleUser {
        Oracle constant private ORACLE_CONST = Oracle(address(0x00000000219ab540356cBB839Cbe05303d7705Fa)); // 已知的合约
        uint private exchangeRate;

        function buySomething() public {
            ORACLE_CONST.query("USD", this.oracleResponse);
        }

        function oracleResponse(uint response) public {
            require(
                msg.sender == address(ORACLE_CONST),
                "Only oracle can call this."
            );
            exchangeRate = response;
        }
    }

.. note::
    Lambda或内联函数是计划中的，但还不支持。
