############
贡献方式
############

对于大家的帮助，我们一如既往地感激。而且有很多选择，您可以为 Solidity 做出贡献。

特别是，我们感谢在以下领域的支持:

* 报告问题。
* 修复和回应 `Solidity的GitHub问题
  <https://github.com/ethereum/solidity/issues>`_，特别是那些被标记为
  `"很好的第一个问题" <https://github.com/ethereum/solidity/labels/good%20first%20issue>`_，这是
  作为外部贡献者的介绍性问题。
* 完善文档。
* 将文档翻译成更多的语言。
* 在 `StackExchange <https://ethereum.stackexchange.com>`_ 和
  `Solidity Gitter Chat <https://gitter.im/ethereum/solidity>`_ 上回答其他用户的问题。
* 通过在 `Solidity论坛 <https://forum.soliditylang.org/>`_ 上提出语言修改或新功能并提供反馈，参与语言设计过程。

为了开始参与，您可以尝试 :ref:`building-from-source`，以熟悉 Solidity 的组件和构建过程。
此外，精通在 Solidity 中编写智能合约可能是有用的。

请注意，本项目发布时有一个 `贡献者行为准则 <https://raw.githubusercontent.com/ethereum/solidity/develop/CODE_OF_CONDUCT.md>`_。通过参与这个项目--在问题、拉动请求或Gitter频道-您同意遵守其条款。

团队电话会议
============

如果您有问题或拉动请求要讨论，或有兴趣听听团队和贡献者正在做什么，您可以加入我们的公共团队电话会议：

- 每周一下午3点，欧洲中部/中部时间。
- 每周三下午2点，欧洲中部/西部时间。

这两个会议都在 `Jitsi <https://meet.ethereum.org/solidity>`_ 举行。

如何报告问题
====================

要报告一个问题，请使用
`GitHub问题跟踪器 <https://github.com/ethereum/solidity/issues>`_。
当报告问题时，请提及以下细节：

* Solidity版本。
* 源代码（如果可以的话）。
* 操作系统。
* 重现该问题的步骤。
* 实际行为与预期行为。

将导致问题的源代码减少到最低限度总是非常有帮助的，有时甚至可以澄清一个误解。

Pull Request 的工作流
==========================

为了进行贡献，请 fork 一个 ``develop`` 分支并在那里进行修改。
除了您 *做了什么* 之外，您还需要在提交信息中说明，
您 *为什么* 做这些修改（除非只是个微小的改动）。

在进行了 fork 之后，如果您还需要从 ``develop`` 分支 pull 任何变更的话
（例如，为了解决潜在的合并冲突），请避免使用 ``git rebase`` ，
而是用 ``git rebase`` 您的分支。

此外，如果您正在编写一个新的功能，请确保您在 ``test/`` 下添加适当的测试案例（见下文）。

但是，如果您在进行一个更大的变更，请先与
`Solidity Development Gitter channel <https://gitter.im/ethereum/solidity-dev>`_
进行商量（与上文提到的那个功能不同，这个变更侧重于编译器和编程语言开发，而不是编程语言的使用）。

新的特性和 bug 修复会被添加到 ``Changelog.md`` 文件中：使用的时候请遵循上述方式。

最后，请确保您遵守了这个项目的 `编码风格 <https://github.com/ethereum/solidity/blob/develop/CODING_STYLE.md>`_ 。
还有，虽然我们采用了持续集成测试，但是在提交 pull request 之前，请测试您的代码并确保它能在本地进行编译。

感谢您的帮助！

运行编译器测试
==========================

先决条件
-------------

为了运行所有的编译器测试，您可能想选择性地安装一些依赖项
（ `evmone <https://github.com/ethereum/evmone/releases>`_，
`libz3 <https://github.com/Z3Prover/z3>`_， 和
`libhera <https://github.com/ewasm/hera>`_）。

在macOS上，一些测试脚本需要安装 GNU 核心工具。
这可以用Homebrew来完成： ``brew install coreutils``。

在Windows系统上，确保您有创建符号链接的权限，否则一些测试可能会失败。
管理员应该有这个权限，但您也可以
`将其授予其他用户 <https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/create-symbolic-links#policy-management>`_
或 `启用开发者模式 <https://docs.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development>`_。

运行测试
-----------------

Solidity包括不同类型的测试，其中大部分捆绑在
`Boost C++测试框架 <https://www.boost.org/doc/libs/release/libs/test/doc/html/index.html>`_ 应用程序 ``soltest``。
运行 ``build/test/soltest`` 或其包装器 ``scripts/soltest.sh`` 对大多数变化来说是足够的。

``./scripts/tests.sh`` 脚本自动执行大多数Solidity测试，
包括那些捆绑在 `Boost C++测试框架 <https://www.boost.org/doc/libs/release/libs/test/doc/html/index.html>`_ 应用程序 ``soltest``
（或其包装器 ``scripts/soltest.sh``）中的测试，以及命令行测试和编译测试。

测试系统会自动尝试发现 `evmone <https://github.com/ethereum/evmone/releases>`_ 的位置，以运行语义测试。

``evmone`` 库必须位于 ``deps`` 或 ``deps/lib`` 目录下，相对于当前工作目录，
其父目录或其父的父目录。另外，可以通过 ``ETH_EVMONE`` 环境变量指定 ``evmone`` 共享对象的明确位置。

``evmone`` 主要用于运行语义和gas测试。
如果您没有安装它，您可以通过向 ``scripts/soltest.sh`` 传递 ``--no-semantic-tests`` 标志来跳过这些测试。

运行Ewasm测试默认是禁用的，可以通过 ``./scripts/soltest.sh --ewasm`` 明确启用，
要求 `hera <https://github.com/ewasm/hera>`_ 被 ``soltest`` 找到。
定位 ``hera`` 库的机制与 ``evmone`` 相同，只是用于指定明确位置的变量被称为 ``ETH_HERA``。

``evmone`` 和 ``hera`` 库的文件名后缀都应该
是Linux上的 ``.so``，Windows系统上的 ``.dll``，MacOS上的 ``.dylib``。

为了运行SMT测试， ``libz3`` 库必须被安装，并在编译器配置阶段被 ``cmake`` 可以找到。

如果您的系统没有安装 ``libz3`` 库，您应该在运行 ``./scripts/tests.sh`` 或 ``./scripts/soltest.sh --no-smt`` 之前，
通过导出 ``SMT_FLAGS=--no-smt`` 来禁用SMT测试。
这些测试是 ``libsolidity/smtCheckerTests`` 和 ``libsolidity/smtCheckerTestsJSON``。

.. note::

    要获得Soltest运行的所有单元测试的列表，请运行 ``./build/test/soltest --list_content=HRF``。

为了获得更快的结果，您可以运行一个子集，或特定的测试。

要运行测试的一个子集，可以使用过滤器：
``./scripts/soltest.sh -t TestSuite/TestName``,
其中 ``TestName`` 可以是通配符 ``*``。

或者，举例来说，运行yul 消歧义器的所有测试：
``./scripts/soltest.sh -t "yulOptimizerTests/disambiguator/*" --no-smt``。

``./build/test/soltest --help`` 有关于所有可用选项的广泛帮助。

尤其是可以查看：

- `show_progress (-p) <https://www.boost.org/doc/libs/release/libs/test/doc/html/boost_test/utf_reference/rt_param_reference/show_progress.html>`_ 来显示测试完成。
- `run_test (-t) <https://www.boost.org/doc/libs/release/libs/test/doc/html/boost_test/utf_reference/rt_param_reference/run_test.html>`_ 来运行特定的测试案例，以及
- `report-level (-r) <https://www.boost.org/doc/libs/release/libs/test/doc/html/boost_test/utf_reference/rt_param_reference/report_level.html>`_ 给出一个更详细的报告。

..  note::

    那些在Windows环境下使用的人，想在没有libz3的情况下运行上述基本集，可以使用Git Bash，
    使用命令为： ``./build/test/Release/soltest.exe -- --no-smt``。
    如果您在普通的命令提示符下运行，使用 ``.\build\test\Release\soltest.exe -- --no-smt``。

如果您想使用GDB进行调试，确保您的构建方式与 “通常” 不同。
例如，您可以在您的 ``build`` 文件夹中运行以下命令：

.. code-block:: bash

   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make

这会创建了一些符号，所以当您使用 ``--debug`` 标志调试测试时，
您可以访问其中的函数和变量，您可以用它来中断或打印。

CI运行额外的测试（包括 ``solc-js`` 和测试第三方Solidity框架），需要编译 Emscripten 目标。

编写和运行语法测试
--------------------------------

语法测试检查编译器是否对无效的代码产生正确的错误信息，并正确接受有效的代码。
它们被保存在 ``tests/libsolidity/syntaxTests`` 文件夹下的单个文件中。
这些文件必须包含注释，说明各自测试的预期结果。
测试套件会根据给定的期望值进行编译和检查。

例如： ``./test/libsolidity/syntaxTests/double_stateVariable_declaration.sol``

.. code-block:: solidity

    contract test {
        uint256 variable;
        uint128 variable;
    }
    // ----
    // 声明错误：（36-52）。标识符已被声明。

语法测试必须至少包含被测合约本身，后面是分隔符 ``//----``。
分隔符后面的注释是用来描述预期的编译器错误或警告的。
数字范围表示错误发生在源代码中的位置。
如果您希望合约在编译时没有任何错误或警告，您可以不使用分隔符和后面的注释。

在上面的例子中，状态变量 ``variable`` 被声明了两次，这是不允许的。这导致了一个 ``声明错误``，说明标识符已经被声明。

用来进行那些测试的工具叫做 ``isoltest``，可以在 ``./build/test/tools/`` 下找到。
它是一个交互工具，允许您使用您喜欢的文本编辑器编辑失败的合约。
让我们把第二个 ``variable`` 的声明去掉来使测试失败：

.. code-block:: solidity

    contract test {
        uint256 variable;
    }
    // ----
    // 声明错误：（36-52）。标识符已被声明。

再次运行 ``./build/test/tools/isoltest`` 就会得到一个失败的测试：

.. code-block:: text

    syntaxTests/double_stateVariable_declaration.sol: FAIL
        Contract:
            contract test {
                uint256 variable;
            }

        Expected result:
            DeclarationError: (36-52): Identifier already declared.
        Obtained result:
            Success


``isoltest`` 在获得的结果旁边打印出预期的结果，
还提供了一个编辑，更新，跳过当前合约文件或退出应用程序的办法。

它为失败的测试提供了几种选择：

- ``edit``：  ``isoltest`` 试图在一个编辑器中打开合约，以便您可以调整它。它或者使用命令行上给出的编辑器（如 ``isoltest --editor /path/to/editor``），或者在环境变量 ``EDITOR`` 中，或者只是 ``/usr/bin/editor`` （按这个顺序）。
- ``update``： 更新测试中的合约。这将会移除包含了不匹配异常的注解，或者增加缺失的预想结果。然后测试会重新开始。
- ``skip``： 跳过这一特定测试的执行。
- ``quit``： 退出 ``isoltest``。

所有这些选项都适用于当前的合约，除了 ``quit``，它可以停止整个测试过程。

在上边的情况自动更新合约会把它变为

.. code-block:: solidity

    contract test {
        uint256 variable;
    }
    // ----

并重新运行测试。它将会通过：

.. code-block:: text

    Re-running test case...
    syntaxTests/double_stateVariable_declaration.sol: OK


.. note::

    为合约文件选择一个能解释其测试内容的名字，例如： ``double_variable_declaration.sol``。
    不要把一个以上的合约放在一个文件中，除非您在测试继承或跨合约的调用。
    每个文件应该测试您的新功能的一个方面。


通过 AFL 运行 Fuzzer
==========================

Fuzzing 是一种测试技术，它可以通过运行多少不等的随机输入来找出异常的执行状态（片段故障、异常等等）。
现代的 fuzzer 已经可以很聪明地在输入中进行直接的查询。
我们有一个专门的程序叫做 ``solfuzzer``，它可以将源代码作为输入，
当发生一个内部编译错误，片段故障或者类似的错误时失败，但当代码包含错误的时候则不会失败。
通过这种方法，fuzzing 工具可以找到那些编译级别的内部错误。

我们主要使用 `AFL <https://lcamtuf.coredump.cx/afl/>`_ 来进行 fuzzing 测试。
您需要手工下载和构建 AFL。然后用 AFL 作为编译器来构建 Solidity（或只是 ``solfuzzer`` 二进制文件）：

.. code-block:: bash

    cd build
    # 如果需要的话
    make clean
    cmake .. -DCMAKE_C_COMPILER=path/to/afl-gcc -DCMAKE_CXX_COMPILER=path/to/afl-g++
    make solfuzzer

在这个阶段，您应该能够看到类似以下的信息：

.. code-block:: text

    Scanning dependencies of target solfuzzer
    [ 98%] Building CXX object test/tools/CMakeFiles/solfuzzer.dir/fuzzer.cpp.o
    afl-cc 2.52b by <lcamtuf@google.com>
    afl-as 2.52b by <lcamtuf@google.com>
    [+] Instrumented 1949 locations (64-bit, non-hardened mode, ratio 100%).
    [100%] Linking CXX executable solfuzzer

如果指示信息没有出现，尝试切换指向AFL的clang二进制文件的cmake标志：

.. code-block:: bash

    # 如果之前失败了
    make clean
    cmake .. -DCMAKE_C_COMPILER=path/to/afl-clang -DCMAKE_CXX_COMPILER=path/to/afl-clang++
    make solfuzzer

否则，在执行时，fuzzer 就会停止，并出现错误，说二进制没有被检测到。

.. code-block:: text

    afl-fuzz 2.52b by <lcamtuf@google.com>
    ... (truncated messages)
    [*] Validating target binary...

    [-] Looks like the target binary is not instrumented! The fuzzer depends on
        compile-time instrumentation to isolate interesting test cases while
        mutating the input data. For more information, and for tips on how to
        instrument binaries, please see /usr/share/doc/afl-doc/docs/README.

        When source code is not available, you may be able to leverage QEMU
        mode support. Consult the README for tips on how to enable this.
        (It is also possible to use afl-fuzz as a traditional, "dumb" fuzzer.
        For that, you can use the -n option - but expect much worse results.)

    [-] PROGRAM ABORT : No instrumentation detected
             Location : check_binary(), afl-fuzz.c:6920


接下来，您需要一些示例源文件。这使得 fuzzer 更容易发现错误。
您可以从语法测试中复制一些文件，或者从文档或其他测试中提取测试文件。

.. code-block:: bash

    mkdir /tmp/test_cases
    cd /tmp/test_cases
    # 从测试中提取：
    path/to/solidity/scripts/isolate_tests.py path/to/solidity/test/libsolidity/SolidityEndToEndTest.cpp
    # 从文件中摘录：
    path/to/solidity/scripts/isolate_tests.py path/to/solidity/docs

AFL 的文档指出，账册（初始的输入文件）不应该太大。
每个文件本身不应该超过 1 kB，并且每个功能最多只能有一个输入文件；
所以最好从少量的输入文件开始。
此外还有一个叫做 ``afl-cmin`` 的工具，
可以将输入文件整理为可以具有近似行为的二进制代码。

现在运行 fuzzer（ ``-m`` 参数将使用的内存大小扩展为 60 MB）：

.. code-block:: bash

    afl-fuzz -m 60 -i /tmp/test_cases -o /tmp/fuzzer_reports -- /path/to/solfuzzer

fuzzer 会将导致失败的源文件创建在 ``/tmp/fuzzer_reports`` 中。
通常它会找到产生相似错误的类似的源文件。
您可以使用 ``scripts/uniqueErrors.sh`` 工具来那些独特的错误。

Whiskers 系统
=============

*Whiskers* 是一个类似于 `Mustache <https://mustache.github.io>`_ 的字符串模板化系统。
它被编译器用在不同的地方，以帮助代码的可读性，从而帮助代码的可维护性和可验证性。

该语法与Mustache有很大区别。模板标记 ``{{`` 和 ``}}`` 被 ``<`` 和 ``>`` 取代，
以帮助解析并避免与 :ref:`yul` 的冲突
（符号 ``<`` 和 ``>`` 在内联汇编中是无效的，而 ``{`` 和 ``}`` 是用来限定块的）。
另一个限制是，列表只能解决一个深度的问题，而且它们不会递归。这在将来可能会改变。

下面是一个粗略的说明：

任何出现的 ``<name>`` 的地方都会被提供的变量 ``name`` 的字符串值替换，没有任何转义，也没有迭代替换。
可以用 ``<#name>...</name>`` 来划定一个区域。
该区域中的内容将进行多次拼接，每次拼接会使用相应变量集中的值替换区域中的 ``<inner>`` 项，
模板系统中提供了多少组变量集，就会进行多少次拼接。顶层变量也可以在这种区域内使用。

还有一些判断条件的表达式 ``<?name <!name>...</name>``，
根据布尔参数 ``name`` 的值，会在第一段或第二段继续递归地替换模板。
如果使用 ``<?+name>...<!+name>...</+name>`` 这种表达式，那么检查的是字符串参数 ``name`` 是否为非空。

.. _documentation-style:

文档风格指南
=========================

在下面的部分，您可以找到专门针对 Solidity 文档贡献的风格建议。

英语
----------------

使用英语，除非使用项目或品牌名称，否则首选英式拼写。
尽量减少使用当地的俚语和参考资料，尽量使您的语言对所有的读者都尽可能清晰。以下是一些参考资料，希望对大家有所帮助：

* `简化技术英语 <https://en.wikipedia.org/wiki/Simplified_Technical_English>`_
* `国际英语 <https://en.wikipedia.org/wiki/International_English>`_
* `英式英语拼写 <https://en.oxforddictionaries.com/spelling/british-and-spelling>`_


.. note::

    虽然官方的 Solidity 文档是用英语写的，但也有社区贡献的其他语言的 :ref:`translations` 可用。
    请参考 `翻译指南 <https://github.com/solidity-docs/translation-guide>`_ 以了解如何为社区翻译作出贡献。

标题的大小写
-----------------------

在标题中使用 `标题大小写 <https://titlecase.com>`_。
这意味着标题中的所有主词都要大写，但不包括冠词，连接词和介词，除非它们是标题的开头。

例如，下列各项都是正确的：

* Title Case for Headings.
* For Headings Use Title Case.
* Local and State Variable Names.
* Order of Layout.

扩写缩写
-------------------

使用扩展的缩略语来表达单词，例如：

* "Do not" 替代 "Don't"。
* "Can not" 替代 "Can't"。

主动和被动语态
------------------------

主动语态通常被推荐用于教程风格的文档，因为它有助于读者理解谁或什么在执行一项任务。
然而，由于 Solidity 文档是教程和参考内容的混合物，被动语态有时更适用。

综上所述：

* 在技术参考方面使用被动语态，例如语言定义和Ethereum虚拟机的内部情况。
* 在描述关于如何应用 Solidity 某方面的建议时，使用主动语态。

例如，下面的内容是被动语态，因为它指定了 Solidity 的一个方面：

  函数可以被声明为 ``pure``，在这种情况下，它们承诺不读取或修改状态。

例如，下面是主动语态，因为它讨论了Solidity的一个应用：

  在调用编译器时，您可以指定如何发现一个路径的第一个元素，也可以指定路径前缀的重映射。

常用术语
------------

* “函数参数“ 和 “返回变量“，而不是输入和输出参数。

代码示例
-------------

CI进程在您创建PR时，使用 ``./test/cmdlineTests.sh`` 脚本测试所有
以 ``pragma solidity``， ``contract``， ``library`` 或 ``interface`` 开头的代码块格式的示例代码。
如果您正在添加新的代码实例，在创建PR之前确保它们能够工作并通过测试。

确保所有的代码实例以 ``pragma`` 版本开始，跨越合约代码有效的最大范围。
例如 ``pragma solidity >=0.4.0 <0.9.0;``。

运行文档测试
---------------------------

通过运行 ``./scripts/docs.sh`` 来确保您的贡献通过我们的文档测试，
它安装了文档所需的依赖，并检查任何问题，如无效的链接或语法问题。

Solidity语言设计
========================

为了积极参与语言设计过程，并分享您关于Solidity未来的想法，请加入 `Solidity论坛 <https://forum.soliditylang.org/>`_。

Solidity论坛作为提出和讨论新的语言功能及其在早期构思阶段的实现或现有功能的修改的一个地方。

一旦提案变得更加具体，
它们的实施也将在 `Solidity GitHub仓库 <https://github.com/ethereum/solidity>`_ 中以问题的形式讨论。

除了论坛和问题讨论之外，我们还定期举办语言设计讨论会议，对选定的主题，问题或功能实现进行详细的辩论。
这些会议的邀请函通过论坛共享。

我们也在论坛中分享反馈调查和其他与语言设计相关的内容。

如果您想知道团队在实施新功能方面的情况，
您可以在 `Solidity Github项目 <https://github.com/ethereum/solidity/projects/43>`_ 中关注实施状况。
设计积压中的问题需要进一步规范，将在语言设计电话会议或常规团队电话会议中讨论。
您可以通过从默认分支（ `develop` ）到 `breaking 分支 <https://github.com/ethereum/solidity/tree/breaking>`_
来查看下一个突破性版本即将发生的变化。

对于特殊情况和问题，您可以通过 `Solidity-dev Gitter 频道 <https://gitter.im/ethereum/solidity-dev>`_ 与我们联系，
这是一个专门的聊天室，用于围绕 Solidity 编译器和语言开发进行对话。

我们很高兴听到你对我们如何改进语言设计过程，使之更加协作和透明的想法。
