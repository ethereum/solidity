.. index:: Bugs

.. _known_bugs:

##################
已知bug列表
##################

下面，您可以找到一个JSON格式的列表，其中包括Solidity编译器中一些已知的与安全有关的错误。
该文件本身托管在 `Github 仓库 <https://github.com/ethereum/solidity/blob/develop/docs/bugs.json>`_。
该列表最早可以追溯到0.3.0版本，只有在此之前的版本中已知的错误没有列出。

还有一个文件叫
`bugs_by_version.json <https://github.com/ethereum/solidity/blob/develop/docs bugs_by_version.json>`_，
它可以用来检查哪些bug影响到特定版本的编译器。

合约源码验证工具以及其他与合约交互的工具应根据以下标准查阅此列表：

- 如果合约是用夜间编译器版本而不是发布版本编译的，那就有点可疑了。
  此列表不跟踪未发布或夜间版本。
- 如果合约的编译版本不是合约创建时的最新版本，则也有点可疑。
  对于从其他合约创建的合约，您必须按照创建链返回到事务，并使用该事务的日期作为创建日期。
- 如果合约是用包含已知bug的编译器编译的，并且合约是在已发布包含修复程序的较新编译器版本时创建的，
  则这是高度可疑的。

下面的已知错误的JSON文件是一个对象数组，每个错误都有一个对象，其键值如下：

uid
    以 ``SOL-<year>-<number>`` 的形式给予该错误的唯一标识符。
    有可能存在多个具有相同uid的条目。
    这意味着多个版本范围受到同一错误的影响。
name
    给予该错误的唯一名称
summary
    对该错误的简短描述
description
    该错误的详细描述
link
    有更多详细信息的网站的URL，可选
introduced
    第一个包含该错误的发布的编译器版本，可选
fixed
    第一个不再包含该错误的发布的编译器版本
publish
    bug公开的日期，可选
severity
    bug的严重程度：非常低，低，中，高。
    考虑合约测试中的可发现性，发生的可能性和错误造成的潜在损害。
conditions
    必须满足的条件才能触发该错误。可以使用以下键：
    ``optimizer``, 布尔值，表示优化器必须打开才会出现该错误。
    ``evmVersion``, 一个字符串，表示哪个EVM版本的编译器设置触发了该错误。
    这个字符串可以包含比较运算符。例如， ``">=constantinople"`` 表示
    当EVM版本设置为 ``constantinople`` 或更高时，该错误就会出现。
    如果没有给出条件，则假定该错误存在。
check
    这个字段包含不同的检查，报告智能合约是否包含错误。
    第一种类型的检查是Javascript正则表达式，如果存在该错误，将与源代码（“source-regex”）进行匹配。
    如果没有匹配，那么该漏洞很可能不存在。如果有一个匹配，则该错误可能存在。
    为了提高准确性，检查应该在剥离注释后应用于源代码。
    第二种类型的检查是在Solidity程序的紧凑AST上检查的模式（“ast-compact-json path”）。
    指定的搜索查询是一个 `JsonPath <https://github.com/json-path/JsonPath>`_ 表达式。
    如果Solidity AST中至少有一个路径与该查询相匹配，则该错误可能存在。

.. literalinclude:: bugs.json
   :language: js
