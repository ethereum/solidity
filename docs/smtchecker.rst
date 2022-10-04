.. _formal_verification:

##################################
SMT检查器和形式化验证
##################################

使用形式化验证，有可能进行自动数学证明，
证明您的源代码符合某种形式化规范。
该规范仍然是正式的（就像源代码一样），但通常要简单得多。

请注意，形式化验证本身只能帮助您理解您所做的（规范）和您如何做的（实际实现）之间的区别。
您仍然需要检查规范是否是您想要的，以及您没有遗漏任何意想不到的效果。

Solidity 实现了基于 `SMT（可满足性模型理论（Satisfiability Modulo Theories） <https://en.wikipedia.org/wiki/Satisfiability_modulo_theoris>`_
和 `Horn <https://en.wikipedia.org/wiki/Horn-satisfiability>`_ 解决的形式验证方法。
SMT检查器模块自动尝试证明代码满足由 ``require`` 和 ``assert`` 语句给出的规范。
也就是说，它把 ``require`` 语句视为假设，并试图证明 ``assert`` 语句中的条件总是真的。
如果发现断言失败，则可以向用户提供一个反例，说明断言是如何被违反的。
如果 SMT 检查器对某一属性没有给出警告，这意味着该属性是安全的。

SMT 检查器在编译时检查的其他验证目标有：

- 算术上的下溢和溢出。
- 除以0的除法。
- 无用的条件和无法访问的代码。
- 弹出一个空数组。
- 超出界限的索引访问。
- 转账资金不足。

如果所有检查引擎都被启用，上述所有目标都被默认为自动检查，
除了 Solidity >=0.8.7 的下溢和溢出。

SMT 检查器所报告的潜在警告是：

- ``<失败的属性> 发生在这里``。这意味着 SMT 检查器证明了某一属性失败。可能会给出一个反例，但是在复杂的情况下，也可能不会显示反例。在某些情况下，当 SMT 编码为 Solidity 代码添加了难以表达或无法表达的抽象时，这个结果也可能是一个假阳性。
- ``<失败的属性> 可能发生在这里``。这意味着求解器无法在给定的超时时间内证明两种情况。由于结果是未知的，SMT 检查器会报告潜在的健全性失败。这可以通过增加查询超时时间来解决，但问题也可能只是对引擎来说太难解决。

要启用SMT检查器，您必须选择 :ref:`应该运行哪一个引擎 <smtchecker_engines>`，
其中默认的是没有引擎。选择引擎可以在所有文件上启用SMT检查器。

.. note::

    在 Solidity 0.8.4 之前，启用SMT检查器的默认方式是通过 ``pragma experimental SMTChecker;``
    并且只有包含 pragma 的合约才会被分析。该 pragma 已被弃用，
    尽管它仍能使SMT检查器向后兼容，但它将在 Solidity 0.9.0 中被移除。
    还要注意的是，现在即使在一个文件中使用 pragma，也会对所有文件启用SMT检查器。

.. note::

    假设SMT检查器和底层求解器中没有错误，
    那么验证目标没有警告就代表了一个无可争议的正确性数学证明。
    请记住，这些问题在一般情况下是 *很难* 的，有时是 *不可能* 自动解决的。
    因此，有几个属性可能无法解决，或者可能导致大型合约的假阳性。
    每一个被证明的属性都应该被看作是一个重要的成就。
    对于高级用户，请参阅 :ref:`SMT检查器 调优 <smtchecker_options>`
    来了解一些可能有助于证明更复杂属性的选项。

********
教程
********

溢出
========

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Overflow {
        uint immutable x;
        uint immutable y;

        function add(uint _x, uint _y) internal pure returns (uint) {
            return _x + _y;
        }

        constructor(uint _x, uint _y) {
            (x, y) = (_x, _y);
        }

        function stateAdd() public view returns (uint) {
            return add(x, y);
        }
    }

上面的合约显示了一个溢出检查的例子。
对于 Solidity >=0.8.7，SMT检查器默认不检查下溢和溢出，
所以我们需要使用命令行选项 ``--model-checker-targets "underflow,overflow"``
或者JSON选项 ``settings.modelChecker.targets = ["underflow", "overflow"]``。
参见 :ref:`本节的目标配置 <smtchecker_targets>`。此处，它报告如下：

.. code-block:: text

    Warning: CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
    Counterexample:
    x = 1, y = 115792089237316195423570985008687907853269984665640564039457584007913129639935
     = 0

    Transaction trace:
    Overflow.constructor(1, 115792089237316195423570985008687907853269984665640564039457584007913129639935)
    State: x = 1, y = 115792089237316195423570985008687907853269984665640564039457584007913129639935
    Overflow.stateAdd()
        Overflow.add(1, 115792089237316195423570985008687907853269984665640564039457584007913129639935) -- internal call
     --> o.sol:9:20:
      |
    9 |             return _x + _y;
      |                    ^^^^^^^

如果我们添加了过滤掉溢出情况的 ``require`` 语句，
SMT检查器就会证明没有溢出是可以达到的（会通过不报告警告表现出来）。

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Overflow {
        uint immutable x;
        uint immutable y;

        function add(uint _x, uint _y) internal pure returns (uint) {
            return _x + _y;
        }

        constructor(uint _x, uint _y) {
            (x, y) = (_x, _y);
        }

        function stateAdd() public view returns (uint) {
            require(x < type(uint128).max);
            require(y < type(uint128).max);
            return add(x, y);
        }
    }


断言
======

断言表示代码中的一个不变量： *对于所有的事务，包括所有的输入和存储值*，
一个属性必须为真，否则就会出现错误。

下面的代码定义了一个保证没有溢出的函数 ``f``。
函数 ``inv`` 定义了 ``f`` 是单调递增的规范：
对于每个可能的数值对 ``(_a, _b)``，如果 ``_b > _a``，那么 ``f(_b) > f(_a)``。
由于 ``f`` 确实是单调增长的，SMT检查器证明了我们的属性是正确的。
我们鼓励您试试这个属性和函数定义，看看会有什么样的结果!

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Monotonic {
        function f(uint _x) internal pure returns (uint) {
            require(_x < type(uint128).max);
            return _x * 42;
        }

        function inv(uint _a, uint _b) public pure {
            require(_b > _a);
            assert(f(_b) > f(_a));
        }
    }

我们还可以在循环中添加断言，以验证更多的复杂的属性。
下面的代码搜索一个不受限制的数字数组的最大元素，
并断言找到的元素必须大于或等于数组中的每个元素的属性。

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Max {
        function max(uint[] memory _a) public pure returns (uint) {
            uint m = 0;
            for (uint i = 0; i < _a.length; ++i)
                if (_a[i] > m)
                    m = _a[i];

            for (uint i = 0; i < _a.length; ++i)
                assert(m >= _a[i]);

            return m;
        }
    }

注意，在这个例子中，SMT检查器将自动尝试证明三个属性：

1. 第一个循环中的 ``++i`` 不会溢出。
2. 第二个循环中的 ``++i`` 不会溢出。
3. 该断言始终是正确的。

.. note::

    这些属性涉及到循环，这使得它比前面的例子 *更加* 难了，所以要当心循环的问题！

所有的属性都被正确证明是安全的。
可以随意改变属性和/或在数组上添加限制，以看到不同的结果。例如，将代码改为

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Max {
        function max(uint[] memory _a) public pure returns (uint) {
            require(_a.length >= 5);
            uint m = 0;
            for (uint i = 0; i < _a.length; ++i)
                if (_a[i] > m)
                    m = _a[i];

            for (uint i = 0; i < _a.length; ++i)
                assert(m > _a[i]);

            return m;
        }
    }

我们得到的结果：

.. code-block:: text

    Warning: CHC: Assertion violation happens here.
    Counterexample:

    _a = [0, 0, 0, 0, 0]
     = 0

    Transaction trace:
    Test.constructor()
    Test.max([0, 0, 0, 0, 0])
      --> max.sol:14:4:
       |
    14 |            assert(m > _a[i]);


状态属性
================

到目前为止，这些例子只展示了SMT检查器在纯代码上的使用，
证明了关于特定操作或算法的属性。
智能合约中常见的属性类型是涉及合约状态的属性。
对于这样的属性，可能需要多个交易来使断言失效。

举一个例子，考虑一个二维网格，其中两个轴的坐标都在（-2^128, 2^128 - 1）范围内。
让我们在位置（0，0）放置一个机器人。该机器人只能在对角线上移动，一次只能走一步，
不能在网格外移动。机器人的状态机可以用下面的智能合约来表示。

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Robot {
        int x = 0;
        int y = 0;

        modifier wall {
            require(x > type(int128).min && x < type(int128).max);
            require(y > type(int128).min && y < type(int128).max);
            _;
        }

        function moveLeftUp() wall public {
            --x;
            ++y;
        }

        function moveLeftDown() wall public {
            --x;
            --y;
        }

        function moveRightUp() wall public {
            ++x;
            ++y;
        }

        function moveRightDown() wall public {
            ++x;
            --y;
        }

        function inv() public view {
            assert((x + y) % 2 == 0);
        }
    }

函数 ``inv`` 代表状态机的一个不变量，即 ``x + y`` 必须是偶数。
SMT检查器设法证明，无论我们给机器人多少条命令，
即使是无限多的命令，这个不变量都 *不会* 失败。
有兴趣的读者可能也想手动证明这个事实。 提示：这个不变量是归纳性的。

我们也可以欺骗SMT检查器，让它给我们提供一条通往某个我们认为可能是可访问的位置的路径。
我们可以通过添加以下函数，来增加(2, 4)是 *不* 可访问的属性。

.. code-block:: Solidity

    function reach_2_4() public view {
        assert(!(x == 2 && y == 4));
    }

这个属性是假的，在证明这个属性是假的同时，
SMT检查器准确地告诉我们 *如何* 访问到(2, 4)。

.. code-block:: text

    Warning: CHC: Assertion violation happens here.
    Counterexample:
    x = 2, y = 4

    Transaction trace:
    Robot.constructor()
    State: x = 0, y = 0
    Robot.moveLeftUp()
    State: x = (- 1), y = 1
    Robot.moveRightUp()
    State: x = 0, y = 2
    Robot.moveRightUp()
    State: x = 1, y = 3
    Robot.moveRightUp()
    State: x = 2, y = 4
    Robot.reach_2_4()
      --> r.sol:35:4:
       |
    35 |            assert(!(x == 2 && y == 4));
       |            ^^^^^^^^^^^^^^^^^^^^^^^^^^^

请注意，上面的路径不一定是确定的，
因为还有其他路径可以访问（2，4）。
选择哪条路径可能会根据所使用的解算器，其使用版本，或者只是随机地改变。

外部调用和重入
=============================

每个外部调用都被SMT检查器视为对未知代码的调用。
这背后的原因是，即使被调用合约的代码在编译时是可用的，
也不能保证部署的合约确实与编译时接口所在的合约相同。

在某些情况下，有可能在状态变量上自动推断出属性，
即使外部调用的代码可以做任何事情，包括重新进入调用者合约，
这些属性仍然是真的。

.. code-block:: Solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    interface Unknown {
        function run() external;
    }

    contract Mutex {
        uint x;
        bool lock;

        Unknown immutable unknown;

        constructor(Unknown _u) {
            require(address(_u) != address(0));
            unknown = _u;
        }

        modifier mutex {
            require(!lock);
            lock = true;
            _;
            lock = false;
        }

        function set(uint _x) mutex public {
            x = _x;
        }

        function run() mutex public {
            uint xPre = x;
            unknown.run();
            assert(xPre == x);
        }
    }

上面的例子显示了一个使用互斥标志来禁止重入的合约。
解算器能够推断出，当 ``unknown.run()`` 被调用时，合约已经被 “锁定”，
所以无论未知的调用代码做什么，都不可能改变 ``x`` 的值。

如果我们 “忘记” 在函数 ``set`` 上使用 ``mutex`` 修饰符，
SMT检查器就能合成外部调用代码的行为，从而使断言失败。

.. code-block:: text

    Warning: CHC: Assertion violation happens here.
    Counterexample:
    x = 1, lock = true, unknown = 1

    Transaction trace:
    Mutex.constructor(1)
    State: x = 0, lock = false, unknown = 1
    Mutex.run()
        unknown.run() -- untrusted external call, synthesized as:
            Mutex.set(1) -- reentrant call
      --> m.sol:32:3:
       |
    32 | 		assert(xPre == x);
       | 		^^^^^^^^^^^^^^^^^


.. _smtchecker_options:

*****************************
SMT检查器选项和调试
*****************************

超时
=======

SMT检查器使用了一个硬编码的资源限制（ ``rlimit`` ），
这个限制是根据每个求解器选择的，与时间没有确切的关系。
我们选择 ``rlimit`` 选项作为默认值，因为它比求解器内部的时间提供了更多的确定性保证。

这个选项大致转化为每个查询 “几秒钟超时”。
当然，许多属性非常复杂，需要大量的时间来解决，而决定并不重要。
如果SMT检查器不能用默认的 ``rlimit`` 选项处理合约属性，
则可以通过CLI选项 ``--model-checker-timeout <time>`` 或
JSON选项 ``settings.modelChecker.timeout=<time>`` 给出以毫秒为单位的超时。
其中0表示不超时。

.. _smtchecker_targets:

验证目标
====================

SMT检查器创建的验证目标的类型也可以通过CLI选项 ``--model-checker-target <targets>``
或JSON选项 ``settings.modelChecker.targets=<targets>`` 来定制。
在CLI情况下， ``<targets>`` 是一个没有空格的逗号分隔的一个或多个验证目标的列表，
在JSON输入中是一个或多个作为字符串的目标数组。
代表目标的关键词是：

- 断言： ``assert``。
- 算术下溢： ``underflow``。
- 算术溢出： ``overflow``。
- 除以零： ``divByZero``。
- 无用的条件和无法访问的代码： ``constantCondition``。
- 弹出一个空数组： ``popEmptyArray``。
- 越界的数组/固定字节索引访问： ``outOfBounds``。
- 转账资金不足： ``balance``。
- 以上都是： ``default`` （仅适用CLI）。

一个常见的目标子集可能是，例如： ``--model-checker-targets assert,overflow``。

所有目标都被默认检查，除了Solidity >=0.8.7的下溢和溢出。

关于如何以及何时分割验证目标，没有精确的指导方法。
但在处理大型合约时，它可能是有用的。

未验证的目标
================

如果有任何未验证的目标，SMT检查器会发出一个警告，
说明有多少个未验证的目标。如果用户希望看到所有具体的未验证的目标，
可以使用CLI选项 ``--model-checker-show-unproved``
和JSON选项 ``settings.modelChecker.showUnproved = true``。

已验证过的合约
==================

默认情况下，给定来源中的所有可部署合约都会被单独分析，正如将被部署的那一个合约一样。
这意味着，如果一个合约有许多直接和间接的继承父类，所有这些都将被单独分析，
尽管只有最终派生的合约可以在区块链上被直接访问。
这给SMT检查器和求解器造成了不必要的负担。
为了帮助缓解这样的情况，用户可以指定哪些合约应该作为部署的合约进行分析。
当然，基类合约仍然被分析，但只是在分析最终派生的合约的情况下才进行，
这可以减少编码和生成查询的复杂性。
请注意，抽象合约在默认情况下不会被SMT检查器分析为最终派生的合约。

选择的合约可以通过CLI，用 <source>:<contract> 形式的键值对，以逗号分隔的列表（不允许有空格）给出：
``--model-checker-contracts "<source1.sol:contract1>,<source2.sol:contract2>,<source2.sol:contract3>"``，
以及通过 :ref:`JSON 输入<compiler-api>` 中的对象 ``settings.modelChecker.contracts``，它有如下格式：

.. code-block:: json

    "contracts": {
        "source1.sol": ["contract1"],
        "source2.sol": ["contract2", "contract3"]
    }

报告推断的归纳变量
======================================

于那些被CHC引擎证明为安全的属性，
SMT检查器可以检索由Horn求解器推断出的归纳不变性，作为证明的一部分。
目前有两种类型的不变量可以报告给用户：

- 合约不变量：这些是合约的状态变量的属性，在合约可能运行的每一个可能的事务之前和之后都是真的。
  例如， ``x >= y``，其中 ``x`` 和 ``y`` 是一个合约的状态变量。
- 可重入性属性：它们代表了合约在存在对未知代码的外部调用时的行为。
  这些属性可以表达外部调用前后状态变量的值之间的关系，
  其中外部调用可以自由地做任何事情，包括对分析的合约进行可重入调用。
  导数变量代表所述外部调用后的状态变量的值。例如： ``lock -> x = x'``。

用户可以使用CLI选项 ``--model-checker-invariants "contract,reentrancy"`` 来选择要报告的不变量类型，
或者在 :ref:`JSON 输入<compiler-api>` 中的字段 ``settings.modelChecker.invariants`` 中作为数组。
默认情况下，SMT检查器不报告不变量。

有松弛变量的除法和模数运算
========================================

Spacer是SMT检查器使用的默认Horn求解器，它通常不喜欢Horn规则中的除法和模数操作。
正因为如此，默认情况下，Solidity的除法和模运算是用约束条件 ``a = b * d + m`` 来编码的，
其中 ``d = a / b`` 和 ``m = a % b``。
然而，对于其他求解器，如Eldarica，更喜欢语法上的精确操作。
命令行标志 ``--model-checker-div-mod-no-slacks`` 和
JSON选项 ``settings.modelChecker.divModNoSlacks`` 可以用来切换编码，
这取决于所用求解器的偏好。

Natspec标签函数抽象化
============================

某些函数包括常见的数学方法，如 ``pow`` 和 ``sqrt``，
可能它们过于复杂，无法用完全自动化的方式进行分析。
这些函数可以用Natspec标签进行注释，向SMT检查器表明这些函数应该被抽象化。
这意味着在调用此函数时，不会使用函数的主体，函数将：

- 返回一个非决定性的值，如果抽象函数是 view/pure 类型的，则保持状态变量不变，
  否则会将状态变量设置为非决定性的值。
  可以通过注解 ``//@custom:smtchecker abstract-function-nondet`` 来使用。
- 作为一个未被解释的函数。这意味着函数的语义（由主体给出）会被忽略，
  这个函数的唯一属性是，给定相同的输入，它保证有相同的输出。
  这一点目前正在开发中，并将通过注解 ``//@custom:smtchecker abstract-function-uf`` 来使用。

.. _smtchecker_engines:

模型检查引擎
======================

SMT检查器模块实现了两个不同的推理引擎，一个是有界模型检查器（Bounded Model Checker， BMC），
一个是约束角条款（Constrained Horn Clauses， CHC）系统。
这两个引擎目前都在开发中，并且有不同的特点。
这两个引擎是独立的，每一个属性警告都说明它来自哪个引擎。
请注意，上面所有带有反例的例子都是由CHC这个更强大的引擎报告的。

默认情况下，两个引擎都会被使用，其中首先运行CHC，
每一个没有被证明的属性都被传递给BMC。
您可以通过CLI选项 ``--model-checker-engine {all,bmc,chc,none}`` 或
JSON选项 ``settings.modelChecker.engine {all,bmc,chc,none}`` 来选择一个特定的引擎。

有界模型检查器 （BMC）
---------------------------

BMC引擎单独地分析函数，也就是说，它在分析每个函数时不会考虑合约在多个交易中的整体行为。
目前在这个引擎中循环也会被忽略了。
只要不是直接或间接的递归，内部函数调用是内联的。
如果可能的话，外部函数调用是内联的。
有可能受重入影响的理论在此被忽略。

上述特点使BMC容易报告假阳性，
但它也是轻量级的，应该能够快速找到小的局部bug。

受约束的角条款（Constrained Horn Clauses， CHC）
------------------------------------------------

合约的控制流程图（CFG）被建模为一个Horn条款系统，
其中合约的生命周期由一个可以非确定性地访问每个公共/外部函数的循环表示。
这样，在分析任何函数时都会考虑到整个合约在无限制数量的事务中的行为。
这个引擎完全支持循环。
支持内部函数调用，而外部函数调用假定被调用的代码是未知的，可以做任何事情。

在能够证明的内容方面，CHC引擎要比BMC强大得多，但可能需要更多的计算资源。

SMT和Horn求解器
====================

上面详述的两个引擎使用自动定理证明器作为其逻辑后端。
BMC使用一个SMT求解器，而CHC使用一个Horn求解器。
通常同一个工具可以同时充当这两种工具，如 `z3 <https://github.com/Z3Prover/z3>`_，
它主要是一个SMT求解器，并将 `Spacer <https://spacer.bitbucket.io/>`_
作为一个Horn求解器使用，而 `Eldarica <https://github.com/uuverifiers/eldarica>`_
则同时做这两种工作。

如果求解器可用的话，用户可以通过CLI选项 ``--model-checker-solvers {all,cvc4,smtlib2,z3}``
或JSON选项 ``settings.modelChecker.solvers=[smtlib2,z3]`` 来选择应该使用哪个求解器，
其中：

- ``cvc4`` 仅在使用 ``solc`` 编译二进制文件时可用。并且只有BMC使用 ``cvc4``。
- ``smtlib2`` 以 `smtlib2 <http://smtlib.cs.uiowa.edu/>`_ 格式输出 SMT/Horn 查询。
  这些可以和编译器的 `回调机制 <https://github.com/ethereum/solc-js>`_ 一起使用，
  这样就可以采用系统中的任何求解器二进制来同步返回查询的结果给编译器。
  例如，这是目前使用Eldarica的唯一方法，因为它没有C++ API。
  根据调用哪个求解器，BMC和CHC都可以使用此方法。
- ``z3`` 是可用的

  - 如果 ``solc`` 与它一起被编译的话。
  - 如果Linux系统中安装了4.8.x版本的动态 ``z3`` 库（从Solidity 0.7.6开始）。
  - 在 ``soljson.js`` （从Solidity 0.6.9开始）中静态的，也就是编译器的Javascript二进制。

由于BMC和CHC都使用 ``z3``，而且 ``z3`` 可以在更多的环境中使用，包括在浏览器中，
大多数用户几乎不需要关心这个选项。更高级的用户可能会应用这个选项，在更复杂的问题上尝试其他求解器。

请注意，所选择的引擎和求解器的某些组合将导致SMT检查器不做任何事情，例如选择CHC和 ``cvc4``。

*******************************
抽象和假阳性结果
*******************************

SMT检查器以一种不完整但健全的方式实现了抽象：
如果报告了一个bug，它可能是由抽象引入的假阳性（由于删除了知识或使用了非精确类型）。
如果它确定一个验证目标是安全的，那么它确实是安全的，也就是说，
不存在假阴性（除非SMT检查器中存在一个bug）。

如果一个目标不能被证明，您可以尝试通过使用上一节中的调整选项来帮助求解器。
如果您确定是假阳性，在代码中加入有更多信息的 ``require`` 语句也可能给求解器带来一些更多的帮助。

SMT的编码和类型
======================

SMT检查器编码试图尽可能精确，
将Solidity类型和表达式映射到它们最接近的 `SMT-LIB <http://smtlib.cs.uiowa.edu/>`_ 表示法上，
正如下表所示。



+------------------------+----------------------------------+------------------------+
|     Solidity 类型      |             SMT 类别             |         理论值         |
+========================+==================================+========================+
| Boolean                | Bool                             | Bool                   |
+------------------------+----------------------------------+------------------------+
| intN, uintN, address,  | Integer                          | LIA, NIA               |
| bytesN, enum, contract |                                  |                        |
+------------------------+----------------------------------+------------------------+
| array, mapping, bytes, | Tuple                            | Datatypes, Arrays, LIA |
| string                 | (Array elements, Integer length) |                        |
+------------------------+----------------------------------+------------------------+
| struct                 | Tuple                            | Datatypes              |
+------------------------+----------------------------------+------------------------+
| 其他类型               | Integer                          | LIA                    |
+------------------------+----------------------------------+------------------------+

尚不支持的类型由一个256位无符号整数抽象出来，其不支持的操作被忽略。

关于SMT编码的内部工作方式的更多细节，请参见论文
`基于SMT的Solidity智能合约验证 <https://github.com/leonardoalt/text/blob/master/solidity_isola_2018/main.pdf>`_。

函数调用
==============

在BMC引擎中，当可能时，即当它们的实现可用时，对相同合约（或基础合约）的函数调用被内联。
对其他合约中的函数的调用不被内联，即使它们的代码是可用的，因为我们不能保证实际部署的代码是相同的。

CHC引擎创建了非线性的Horn选项，使用被调用函数的摘要来支持内部函数调用。
外部函数调用被视为对未知代码的调用，包括潜在的可重入调用。

复杂的纯函数是由参数上的未转译函数（UF）抽象出来的。

+------------------------------------+------------------------------------------+
|                方法                |             BMC/CHC 运行方式             |
+====================================+==========================================+
| ``assert``                         | 验证目标。                               |
+------------------------------------+------------------------------------------+
| ``require``                        | 假设。                                   |
+------------------------------------+------------------------------------------+
| 内部调用                           | BMC: 内联函数调用。                      |
|                                    | CHC：函数摘要。                          |
+------------------------------------+------------------------------------------+
| 对已知代码的外部调用               | BMC: 内联函数调用或                      |
|                                    | 抹去关于状态变量的记忆                   |
|                                    | 和本地存储引用。                         |
|                                    | CHC: 假设被调用的代码是未知的。          |
|                                    | 试图推断出在调用返回后仍然成立的不变性。 |
+------------------------------------+------------------------------------------+
| 存储数组的压栈和出栈               | 精确地支持                               |
|                                    | 检查是否从一个空数组弹出。               |
+------------------------------------+------------------------------------------+
| ABI 函数                           | 用UF函数进行抽象                         |
+------------------------------------+------------------------------------------+
| ``addmod``, ``mulmod``             | 精确地支持                               |
+------------------------------------+------------------------------------------+
| ``gasleft``, ``blockhash``,        | 用UF函数进行抽象                         |
| ``keccak256``, ``ecrecover``       |                                          |
| ``ripemd160``                      |                                          |
+------------------------------------+------------------------------------------+
| 无执行动作的纯函数（外部或复杂）。 | 用UF函数进行抽象                         |
+------------------------------------+------------------------------------------+
| 无执行动作的外部函数               | BMC：擦除状态记忆并假定结果是不确定的。  |
|                                    | CHC：不确定的摘要。                      |
|                                    | 试图推断出在调用返回后仍然成立的不变性。 |
+------------------------------------+------------------------------------------+
| transfer                           | BMC：检查合约的余额是否足够。            |
|                                    | CHC：还不执行检查。                      |
+------------------------------------+------------------------------------------+
| 其他调用                           | 目前不支持                               |
+------------------------------------+------------------------------------------+

使用抽象意味着失去精确的知识，但在许多情况下，这并不意味着失去证明力。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Recover
    {
        function f(
            bytes32 hash,
            uint8 _v1, uint8 _v2,
            bytes32 _r1, bytes32 _r2,
            bytes32 _s1, bytes32 _s2
        ) public pure returns (address) {
            address a1 = ecrecover(hash, _v1, _r1, _s1);
            require(_v1 == _v2);
            require(_r1 == _r2);
            require(_s1 == _s2);
            address a2 = ecrecover(hash, _v2, _r2, _s2);
            assert(a1 == a2);
            return a1;
        }
    }

在上面的例子中，SMT检查器的表达能力不足以实际计算 ``ecrecover``，
但通过将函数调用建模为未转译的函数，我们知道在同等参数上调用时返回值是相同的。
这就足以证明上面的断言总是正确的。

对于已知是确定性的函数，可以用UF来抽象一个函数调用，
对于纯函数也很容易做到。
然而，对于一般的外部函数来说，这是很难做到的，
因为它们可能依赖于状态变量。

引用类型和别名
============================

Solidity 为具有相同 :ref:`数据位置 <data-location>` 的引用类型实现了别名。
这意味着可以通过对同一数据区域的引用来修改一个变量。
SMT检查器并不跟踪哪些引用是指向相同的数据。
这意味着每当分配一个局部引用或引用类型的状态变量时，
所有关于相同类型和数据位置的变量的知识都会被抹去。
如果类型是嵌套的，知识删除也包括所有的前缀基础类型。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.0;

    contract Aliasing
    {
        uint[] array1;
        uint[][] array2;
        function f(
            uint[] memory a,
            uint[] memory b,
            uint[][] memory c,
            uint[] storage d
        ) internal {
            array1[0] = 42;
            a[0] = 2;
            c[0][0] = 2;
            b[0] = 1;
            // 删除关于内存引用的记忆不应该删除关于状态变量的记忆。
            assert(array1[0] == 42);
            // 但是，对存储引用的赋值将相应地删除存储记忆。
            d[0] = 2;
            // 由于上面的分配，失败为假阳性。
            assert(array1[0] == 42);
            // 失败，因为 `a == b` 是可能的。
            assert(a[0] == 2);
            // 失败，因为 `c[i] == b` 是可能的。
            assert(c[0][0] == 2);
            assert(d[0] == 2);
            assert(b[0] == 1);
        }
        function g(
            uint[] memory a,
            uint[] memory b,
            uint[][] memory c,
            uint x
        ) public {
            f(a, b, c, array2[x]);
        }
    }

在对 ``b[0]`` 进行赋值后，我们需要清除关于 ``a`` 的知识，
因为它有相同的类型（ ``uint[]`` ）和数据位置（内存）。
我们还需要清除关于 ``c`` 的知识，因为它的基本类型也是一个位于内存中的 ``uint[]``。
这意味着一些 ``c[i]`` 可能与 ``b`` 或 ``a`` 指的是同一个数据。

注意，我们没有清除关于 ``array`` 和 ``d`` 的知识，
因为它们位于存储区，尽管它们也有 ``uint[]`` 类型。
然而，如果 ``d`` 被分配，我们就需要清除关于 ``array`` 的知识，反之亦然。

合约余额
================

如果在部署交易中 ``msg.value`` > 0，则合约可能在部署时被发送资金。
然而，合约的地址在部署前可能已经有了资金，
这些资金由合约保存。
因此，SMT检查器在构造函数中假定 ``address(this).balance >= msg.value``，
以便与EVM规则一致。合约的余额也可能在不触发任何对合约的调用的情况下增加，如果

- ``selfdestruct`` 是由另一个合约执行的，被分析的合约是剩余资金的接收目标。
- 该合约是某个区块的coinbase（即 ``block.coinbase``）。

为了正确建模，SMT检查器假设在每一笔新的交易中，合约的余额可能至少增长 ``msg.value`` 的值。

**********************
现实世界的假设
**********************

有些情况可以在Solidity和EVM中可以表达出，但可能在实践中不会发生。
其中一种情况是动态存储数组的长度在压栈过程中溢出：
如果 ``push`` 操作被应用于一个长度为 2^256 - 1的数组，它的长度会悄悄溢出。
然而，这在实践中不太可能发生，因为将数组增长到这一点所需的操作需要数十亿年的时间来执行。
SMT检查器采取的另一个类似的假设是，一个地址的余额永远不会溢出。

类似的想法在 `EIP-1985 <https://eips.ethereum.org/EIPS/eip-1985>`_ 中提出过。
