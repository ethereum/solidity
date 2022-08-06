********************************
Solidity v0.8.0 突破性变化
********************************

本节强调了 Solidity 0.8.0 版本中引入的主要突破性变化。对于完整的列表，请查看
`版本更新日志 <https://github.com/ethereum/solidity/releases/tag/v0.8.0>`_。

语义的微小变化
===============================

本节列出了现有代码在编译器没有通知您的情况下改变其行为的更改。

* 算术操作在下溢和溢出时都会恢复。您可以使用 ``unchecked { ... }`` 来使用以前的包装行为。

  溢出的检查是非常普遍的，所以我们把它作为默认的检查，
  以增加代码的可读性，即使它是以略微增加gas成本为代价的。

* ABI编码器v2默认是激活的。

  您可以使用 ``pragma abicoder v1;`` 来选择使用旧的行为。
  语句 ``pragma experimental ABIEncoderV2;`` 仍然有效，但它已被废弃，没有效果。
  如果您想显示使用，请使用 ``pragma abicoder v2;`` 代替。

  请注意，ABI coder v2比v1支持更多的类型，并对输入进行更多的合理性检查。
  ABI coder v2使一些函数调用更加昂贵，而且当合约中包含不符合参数类型的数据时，它还会使合约调用回退，
  而在ABI coder v1中则没有回退。

* 指数是右联的，也就是说，表达式 ``a**b**c`` 被解析为 ``a**（b**c）``。
  在0.8.0之前，它被解析为 ``(a**b)**c``。

  这是解析指数运算符的常用方法。

* 失败的断言和其他内部检查，如除以零或算术溢出，不使用无效的操作码，而是使用恢复操作码。
  更具体地说，它们将使用等于对 ``Panic(uint256)`` 的函数调用的错误数据，其错误代码是针对具体情况的。

  这将节省错误的gas，同时它仍然允许静态分析工具将这些情况与无效输入的恢复区分开来，
  比如一个失败的 ``require``。

* 如果访问存储中的一个字节数组，其长度被错误地编码，就会引起panic错误。
  合约不会出现这种情况，除非使用内联汇编来修改存储字节数组的原始表示。

* 如果常数被用于数组长度表达式中，Solidity 的先前版本将在评估树的所有分支中使用任意精度。
  现在，如果常量变量被用作中间表达式，它们的值将以与它们在运行时表达式中使用时相同的方式被正确舍入。

* 类型 ``byte`` 已经被删除。它是 ``bytes1`` 的别名。

新的限制条件
================

本节列出了可能导致现有合约不再编译的变化。

* 有一些与字面常量的显式转换有关的新限制。以前在以下情况下的行为可能是模糊的：

  1. 不允许从负数字段和大于 ``type(uint160).max`` 的字段显式转换为 ``address``。
  2. 只有当字面常量位于 ``type(T).min`` 和 ``type(T).max`` 之间时，
     才允许字面常量与整数类型 ``T`` 之间的明确转换。
     特别的是，用 ``type(uint).max`` 代替 ``uint(-1)`` 的使用。
  3. 只有当字面常量能够代表枚举中的一个值时，才允许字面常量和枚举之间的显式转换。
  4. 字面常量和 ``address`` 类型之间的显式转换（例如， ``address(literal)``）是 ``address`` 类型，
     而不是 ``address payable`` 类型。通过使用显式转换，即 ``payable(literal)``，
     可以得到一个payable类型的地址类型。

* :ref:`地址字面常量 <address_literals>` 的类型是 ``address``，而不是 ``address payable``。
  它们可以通过显式的转换转换为 ``address payable`` 类型，
  例如： ``payable(0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF)``。

* 对显式类型转换有新的限制。只有当符号，宽度或类型类别（ ``int``， ``address``， ``bytesNN`` 等）
  有最多一次变化时，才允许进行转换。要执行多个变化，请使用多个转换。

  让我们使用符号 ``T(S)`` 来表示显式转换 ``T(x)``，其中， ``T`` 和 ``S`` 是类型，
  ``x`` 是 ``S`` 类型的任何任意变量。这种不允许的转换的例子是 ``uint16(int8)``，
  因为它同时改变了宽度（8位到16位）和符号（有符号整数到无符号整数）。为了进行转换，我们必须通过一个中间类型。
  在前面的例子中，这将是 ``uint16(uint8(int8))`` 或者 ``uint16(int16(int8))``。
  请注意，这两种转换方式将产生不同的结果，例如，对于 ``-1``。下面是这个规则不允许的一些转换的例子。

  - ``address(uint)`` 和 ``uint(address)``：同时转换类型和宽度。
    分别用 ``address(uint160(uint))`` 和 ``uint(uint160(address))`` 代替。
  - ``payable(uint160)``， ``payable(bytes20)`` 和 ``payable(integer-literal)``： 同时转换了类型和状态可变性。
    分别用 ``payable(address(uint160))``， ``payable(address(bytes20))`` 和
    ``payable(address(integer-literal))`` 代替。请注意， ``payable(0)`` 是有效的，是规则的例外。
  - ``int80(bytes10)`` 和 ``bytes10(int80)``：同时转换了类型和符号。
    分别用 ``int80(uint80(bytes10))`` 和 ``bytes10(uint80(int80))`` 代替。
  - ``Contract(uint)``: 同时转换类型和宽度。用 ``Contract(address(uint160(uint)))`` 代替。

  这些转换是不允许的，以避免歧义。例如，在表达式 ``uint16 x = uint16(int8(-1))`` 中，
  ``x`` 的值取决于是先应用符号还是宽度转换。

* 函数调用选项只能给出一次，即 ``c.f{gas: 10000}{value: 1}()`` 是无效的，
  必须改成 ``c.f{gas: 10000, value: 1}()``。

* 全局函数 ``log0``， ``log1``， ``log2``， ``log3`` 和 ``log4`` 已被删除。

  这些都是低级别的函数，基本上没有被使用过。它们的行为可以通过内联汇编访问。

* ``enum`` 定义包含的成员不能超过256个。

  这将使我们可以安全地假设ABI中的底层类型总是 ``uint8``。

* 除了公共函数和事件之外，不允许使用 ``this``， ``super`` 和 ``_`` 的名称进行声明。
  这个例外是为了使声明用Solidity以外的语言实现的合约的接口成为可能，这些语言确实允许这种函数名称。

* 移除对代码中的 ``\b``， ``\f`` 和 ``\v`` 转义序列的支持。
  它们仍然可以通过十六进制转义插入，例如：分别是 ``\x08``， ``\x0c``， 和 ``\x0b``。

* 全局变量 ``tx.origin`` 和 ``msg.sender`` 的类型是 ``address`` 而不是 ``address payable``。
  我们可以通过显式转换将它们转换为 ``address payable`` 类型，
  即 ``payable(tx.origin)`` 或 ``payable(msg.sender)``。

  做这个改变是因为编译器不能确定这些地址是否可以支付，所以现在需要一个明确的转换来使这个要求可见。

* 显式转换为 ``address`` 类型总是返回一个非-payable类型的 ``address``。
  特别是，以下显式转换的类型是 ``address`` 而不是 ``address payable``：

  - ``address(u)`` 其中 ``u`` 是一个 ``uint160`` 类型的变量。
    我们可以通过两个显式转换将 ``u`` 转换为 ``address payable`` 类型，即 ``payable(address(u))``。
  - ``address(b)`` 其中 ``b`` 是一个 ``bytes20`` 类型的变量。
    我们可以通过两个显式转换将 ``b`` 转换为 ``address payable`` 类型，即 ``payable(address(b))``。
  - ``address(c)`` 其中 ``c`` 是一个合约。以前，这种转换的返回类型取决于合约是否可以接收以太
    （要么有一个receive函数，要么有一个payable类型的fallback函数）。
    转换 ``payable(c)`` 的类型为 ``address payable``，只有当合约 ``c`` 可以接收以太时才允许。
    一般来说，人们总是可以通过使用以下显式转换将 ``c`` 转换为 ``address payable`` 的类型：
    ``payable(address(c))``。请注意， ``address(this)`` 与 ``address(c)`` 属于同一类别，
    同样的规则也适用于它。

* 内联汇编中的 ``chainid`` 现在被认为是 ``view`` 而不是 ``pure``。

* 一元求反不能再用于无符号整数，只能用于有符号整数。

接口变化
=================

* ``--combined-json`` 的输出已经改变。JSON字段 ``abi``, ``devdoc``, ``userdoc`` 和
  ``storage-layout`` 现在是子对象。在0.8.0之前，它们曾被序列化为字符串。

* “传统AST“ 已被删除（ ``--ast-json`` 在命令行界面， ``legacyAST`` 用于标准JSON）。
  使用 “紧凑型AST”（ ``--ast-compact-json`` 参数. ``AST``）作为替代。

* 旧的错误报告器（ ``--old-reporter`` ）已经被删除。


如何更新您的代码
=======================

- 如果您依赖包装算术，请用 ``unchecked { ... }`` 包裹每个操作。
- 可选：如果您使用SafeMath或类似的库，将 ``x.add(y)``  改为 ``x + y``， ``x.mul(y)`` 改为 ``x * y`` 等等。
- 如果您想继续使用旧的ABI编码器，请添加 ``pragma abicoder v1;``。
- 可以选择删除 ``pragma experimental ABIEncoderV2`` 或 ``pragma abicoder v2`` 因为它是多余的。
- 将 ``byte`` 改为 ``bytes1``。
- 如果需要的话，添加中间显式类型转换。
- 将 ``c.f{gas: 10000}{value: 1}()`` 合并为 ``c.f{gas: 10000, value: 1}()``。
- 将 ``msg.sender.transfer(x)`` 改为 ``payable(msg.sender).transfer(x)``
  或者使用 ``address payable`` 类型的存储变量。
- 将 ``x**y**z`` 改为 ``(x**y)**z``。
- Use inline assembly as a replacement for ``log0``, ..., ``log4``.
- 使用内联汇编作为 ``log0``， ...， ``log4`` 的替代。
- 通过从某类型的最大值中减去该值并加上1来否定该无符号整数
  （例如， ``type(uint256).max - x + 1``，同时确保 `x` 不是零）。
