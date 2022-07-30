.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

***********
继承
***********

Solidity支持多重继承，包括多态性。

多态性意味着函数调用（内部和外部）总是执行继承层次结构中最新继承的合约中的同名函数（和参数类型）。
但必须使用 ``virtual`` 和 ``override`` 关键字在层次结构中的每个函数上明确启用。
参见 :ref:`函数重载 <function-overriding>` 以了解更多细节。

通过使用 ``ContractName.functionName()`` 明确指定合约，
可以在内部调用继承层次结构中更高的函数。
或者如果您想在扁平化的继承层次中调用高一级的函数（见下文），
可以使用 ``super.functionName()``。

当一个合约继承自其他合约时，在区块链上只创建一个单一的合约，
所有基础合约的代码被编译到创建的合约中。
这意味着对基础合约的所有内部函数的调用也只是使用内部函数调用
（ ``super.f(..)`` 将使用 JUMP 而不是消息调用）。

状态变量的阴影被认为是一个错误。
一个派生合约只能声明一个状态变量 ``x``，
如果在它的任何基类中没有相同名称的可见状态变量。

总的来说，Solidity 的继承系统与 `Python的继承系统 <https://docs.python.org/3/tutorial/classes.html#inheritance>`_
非常相似，特别是关于多重继承方面，但也有一些 :ref:`不同之处 <multi-inheritance>`。

详细情况见下面的例子。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;


    contract Owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }


    // 使用 `is` 从另一个合约派生。派生合约可以访问所有非私有成员，
    // 包括内部函数和状态变量，但无法通过 `this` 来外部访问。
    contract Destructible is Owned {
        // 关键字 `virtual` 意味着该函数可以在派生类中改变其行为（"重载"）。
        function destroy() virtual public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }


    // 这些抽象合约仅用于给编译器提供接口。
    // 注意函数没有函数体。
    // 如果一个合约没有实现所有函数，则只能用作接口。
    abstract contract Config {
        function lookup(uint id) public virtual returns (address adr);
    }


    abstract contract NameReg {
        function register(bytes32 name) public virtual;
        function unregister() public virtual;
    }


    // 可以多重继承。请注意， `owned` 也是 `Destructible` 的基类，
    // 但只有一个 `owned` 实例（就像 C++ 中的虚拟继承）。
    contract Named is Owned, Destructible {
        constructor(bytes32 name) {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // 函数可以被另一个具有相同名称和相同数量/类型输入的函数重载。
        // 如果重载函数有不同类型的输出参数，会导致错误。
        // 本地和基于消息的函数调用都会考虑这些重载。
        // 如果您想重载这个函数，您需要使用 `override` 关键字。
        // 如果您想让这个函数再次被重载，您需要再指定 `virtual` 关键字。
        function destroy() public virtual override {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
                NameReg(config.lookup(1)).unregister();
                // 仍然可以调用特定的重载函数。
                Destructible.destroy();
            }
        }
    }


    // 如果构造函数接受参数，
    // 则需要在声明（合约的构造函数）时提供，
    // 或在派生合约的构造函数位置以修饰器调用风格提供（见下文）。
    contract PriceFeed is Owned, Destructible, Named("GoldFeed") {
        function updateInfo(uint newInfo) public {
            if (msg.sender == owner) info = newInfo;
        }

        // 在这里，我们只指定了 `override` 而没有 `virtual`。
        // 这意味着从 `PriceFeed` 派生出来的合约不能再改变 `destroy` 的行为。
        function destroy() public override(Destructible, Named) { Named.destroy(); }
        function get() public view returns(uint r) { return info; }

        uint info;
    }

注意，在上面，我们调用 ``Destructible.destroy()`` 来 "转发" 销毁请求。
这样做的方式是有问题的，从下面的例子中可以看出：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }

    contract Destructible is owned {
        function destroy() public virtual {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is Destructible {
        function destroy() public virtual override { /* 清除操作 1 */ Destructible.destroy(); }
    }

    contract Base2 is Destructible {
        function destroy() public virtual override { /* 清除操作 2 */ Destructible.destroy(); }
    }

    contract Final is Base1, Base2 {
        function destroy() public override(Base1, Base2) { Base2.destroy(); }
    }

调用 ``Final.destroy()`` 时会调用最后的派生重载函数 ``Base2.destroy``，
但是会绕过 ``Base1.destroy``， 解决这个问题的方法是使用 ``super``：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }

    contract Destructible is owned {
        function destroy() virtual public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is Destructible {
        function destroy() public virtual override { /* 清除操作 1 */ super.destroy(); }
    }


    contract Base2 is Destructible {
        function destroy() public virtual override { /* 清除操作 2 */ super.destroy(); }
    }

    contract Final is Base1, Base2 {
        function destroy() public override(Base1, Base2) { super.destroy(); }
    }

如果 ``Base2`` 调用 ``super`` 的函数，它不会简单在其基类合约上调用该函数。
相反，它在最终的继承关系图谱的上一个基类合约中调用这个函数，
所以它会调用 ``Base1.destroy()``
（注意最终的继承序列是——从最远派生合约开始：Final, Base2, Base1, Destructible, ownerd）。
在类中使用 super 调用的实际函数在当前类的上下文中是未知的，尽管它的类型是已知的。
这与普通的虚拟方法查找类似。

.. index:: ! overriding;function

.. _function-overriding:

函数重载
===================

如果基函数被标记为 ``virtual``，则可以通过继承合约来改变其行为。
被重载的函数必须在函数头中使用 ``override`` 关键字。
重载函数只能将被重载函数的可见性从 ``external`` 改为 ``public``。
可变性可以按照以下顺序改变为更严格的可变性。
``nonpayable`` 可以被 ``view`` 和 ``pure`` 重载。
``view`` 可以被 ``pure`` 重写。 ``payable`` 是一个例外，不能被改变为任何其他可变性。

下面的例子演示了改变函数可变性和可见性：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base
    {
        function foo() virtual external view {}
    }

    contract Middle is Base {}

    contract Inherited is Middle
    {
        function foo() override public pure {}
    }

对于多重继承，必须在 ``override`` 关键字后明确指定定义同一函数的最多派生基类合约。
换句话说，您必须指定所有定义同一函数的基类合约，
并且还没有被另一个基类合约重载（在继承图的某个路径上）。
此外，如果一个合约从多个（不相关的）基类合约上继承了同一个函数，必须明确地重载它。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base1
    {
        function foo() virtual public {}
    }

    contract Base2
    {
        function foo() virtual public {}
    }

    contract Inherited is Base1, Base2
    {
        // 派生自多个定义 foo() 函数的基类合约，
        // 所以我们必须明确地重载它
        function foo() public override(Base1, Base2) {}
    }

如果函数被定义在一个共同的基类合约中，
或者在一个共同的基类合约中有一个独特的函数已经重载了所有其他的函数，
则不需要明确的函数重载指定符。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract A { function f() public pure{} }
    contract B is A {}
    contract C is A {}
    // 无需明确的重载
    contract D is B, C {}

更准确地说，如果有一个基类合约是该签名的所有重载路径的一部分，
并且（1）该基类合约实现了该函数，并且从当前合约到该基类合约的任何路径都没有提到具有该签名的函数，
或者（2）该基类合约没有实现该函数，并且从当前合约到该基类合约的所有路径中最多只有一个提到该函数，
那么就不需要重载从多个基类合约继承的函数（直接或间接）。

在这个意义上，一个签名的重载路径是一条继承图的路径，
它从所考虑的合约开始，到提到具有该签名的函数的合约结束，
而该签名没有重载。

如果您不把一个重载的函数标记为 ``virtual``，派生合约就不能再改变该函数的行为。

.. 注解::

  具有 ``private`` 可见性的函数不能是 ``virtual``。

.. 注解::

  在接口合约之外，没有实现的函数必须被标记为 ``virtual``。
  在接口合约中，所有的函数都被自动视为 ``virtual``。

.. 注解::

  从Solidity 0.8.8开始，当重载一个接口函数时，
  不需要 ``override`` 关键字，除非该函数被定义在多个基础上。


如果函数的参数和返回类型与变量的getter函数匹配，公共状态变量可以重载为外部函数。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract A
    {
        function f() external view virtual returns(uint) { return 5; }
    }

    contract B is A
    {
        uint public override f;
    }

.. 注解::

  虽然公共状态变量可以重载外部函数，但它们本身不能被重载。

.. index:: ! overriding;modifier

.. _modifier-overriding:

修饰器重载
===================

函数修改器可以相互重载。
这与 :ref:`函数重载 <function-overriding>` 的工作方式相同（除了对修改器没有重载）。
``virtual`` 关键字必须用在被重载的修改器上， ``override`` 关键字必须用在重载的修改器上：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base
    {
        modifier foo() virtual {_;}
    }

    contract Inherited is Base
    {
        modifier foo() override {_;}
    }


在多重继承的情况下，必须明确指定所有的直接基类合约。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base1
    {
        modifier foo() virtual {_;}
    }

    contract Base2
    {
        modifier foo() virtual {_;}
    }

    contract Inherited is Base1, Base2
    {
        modifier foo() override(Base1, Base2) {_;}
    }



.. index:: ! constructor

.. _constructor:

构造函数
============

构造函数是一个用 ``constructor`` 关键字声明的可选函数，
它在合约创建时被执行，您可以在这里运行合约初始化代码。

在构造函数代码执行之前，如果您用内联编程的方式初始化状态变量，则将其初始化为指定的值；
如果您不用内联编程的方式来初始化，则将其初始化为 :ref:`默认值 <default-value>`。

构造函数运行后，合约的最终代码被部署到区块链上。
部署代码的gas花费与代码长度成线性关系。
这段代码包括属于公共接口的所有函数，以及所有通过函数调用可以到达的函数。
但不包括构造函数代码或只从构造函数中调用的内部函数。

如果没有构造函数，合约将假定默认的构造函数，
相当于 ``constructor() {}``。比如说：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    abstract contract A {
        uint public a;

        constructor(uint _a) {
            a = _a;
        }
    }

    contract B is A(1) {
        constructor() {}
    }

您可以在构造函数中使用内部参数（例如，存储指针）。
在这种情况下，合约必须被标记为 :ref:`abstract <abstract-contract>`，
因为这些参数不能从外部分配有效的值，只能通过派生合约的构造函数来赋值。


.. 警告 ::
    在0.4.22版本之前，构造函数被定义为与合约同名的函数。
    这种语法已被废弃，在0.5.0版本中不再允许。

.. 警告 ::
    在0.7.0版本之前，您必须指定构造函数的可见性为 ``internal`` 或 ``public``。


.. index:: ! base;constructor

基本构造函数的参数
===============================

所有基类合约的构造函数将按照下面解释的线性化规则被调用。
如果基类合约构造函数有参数，派生合约需要指定所有的参数。
这可以通过两种方式实现：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base {
        uint x;
        constructor(uint _x) { x = _x; }
    }

    // 要么直接在继承列表中指定...
    contract Derived1 is Base(7) {
        constructor() {}
    }

    // 或者通过派生构造函数的一个 "修改器"。
    contract Derived2 is Base {
        constructor(uint _y) Base(_y * _y) {}
    }

一种方式是直接在继承列表中给出（ ``is Base(7)`` ）。
另一种是通过修改器作为派生构造函数的一部分被调用的方式（ ``Base(_y * _y)`` ）。
如果构造函数参数是一个常量，并且定义了合约的行为或描述了它，那么第一种方式更方便。
如果基类合约的构造函数参数依赖于派生合约的参数，则必须使用第二种方式。
参数必须在继承列表中或在派生构造函数中以修饰器的形式给出。
在两个地方都指定参数是一个错误。

如果一个派生合约没有指定其所有基类合约的构造函数的参数，它将是抽象的合约。

.. index:: ! inheritance;multiple, ! linearization, ! C3 linearization

.. _multi-inheritance:

多重继承与线性化
======================================

编程语言实现多重继承需要解决几个问题。
一个问题是 `钻石问题 <https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem>`_ 。
Solidity 借鉴了 Python 的方式并且使用 "`C3 线性化 <https://en.wikipedia.org/wiki/C3_linearization>`_"
强制一个由基类构成的 DAG（有向无环图）保持一个特定的顺序。
这最终实现我们所希望的唯一化的结果，但也使某些继承方式变为无效。
尤其是，基类在 ``is`` 后面的顺序很重要。 在下面的代码中，
您必须按照从 “最接近的基类”（most base-like）到 “最远的继承”（most derived）的顺序来指定所有的基类。
注意，这个顺序与Python中使用的顺序相反。

另一种简化的解释方式是，当一个函数被调用时，
它在不同的合约中被多次定义，给定的基类以深度优先的方式从右到左（Python中从左到右）进行搜索，
在第一个匹配处停止。如果一个基类合约已经被搜索过了，它就被跳过。

在下面的代码中，Solidity 会给出 “Linearization of inheritance graph impossible” 这样的错误。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract X {}
    contract A is X {}
    // 这段代码不会编译
    contract C is A, X {}

代码编译出错的原因是 ``C`` 要求 ``X`` 重写 ``A``
（因为定义的顺序是 ``A, X`` ）， 但是 ``A`` 本身要求重写 ``X``，
这是一种无法解决的冲突。

由于您必须明确地重载一个从多个基类合约继承的函数，
而没有唯一的重载，C3线性化在实践中不是太重要。

继承的线性化特别重要的一个领域是，当继承层次中存在多个构造函数时，也许不那么清楚。
构造函数将总是按照线性化的顺序执行，而不管它们的参数在继承合约的构造函数中是以何种顺序提供的。 比如说：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base1 {
        constructor() {}
    }

    contract Base2 {
        constructor() {}
    }

    // 构造函数按以下顺序执行：
    //  1 - Base1
    //  2 - Base2
    //  3 - Derived1
    contract Derived1 is Base1, Base2 {
        constructor() Base1() Base2() {}
    }

    // 构造函数按以下顺序执行：
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived2
    contract Derived2 is Base2, Base1 {
        constructor() Base2() Base1() {}
    }

    // 构造函数仍按以下顺序执行：
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived3
    contract Derived3 is Base2, Base1 {
        constructor() Base1() Base2() {}
    }


继承有相同名字的不同类型成员
======================================================

由于继承的原因，当合约有以下任何一对具有相同的名称时，这是一个错误：
  - 函数和修饰器
  - 函数和事件
  - 事件和修饰器

有一种例外情况，状态变量的 getter 可以重载一个外部函数。
