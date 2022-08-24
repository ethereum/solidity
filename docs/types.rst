.. index:: type

.. _types:

*****
类型
*****

Solidity 是一种静态类型语言，这意味着每个变量（状态变量和局部变量）都需要被指定。
Solidity 提供了几种基本类型，可以用来组合出复杂类型。

除此之外，各个类型之间可以在包含运算符号的表达式中进行交互。
关于各种运算符的快速参考，可以参考 :ref:`order`。

Solidity中不存在"未定义"或"空"值的概念，
但新声明的变量总是有一个取决于其类型的 :ref:`默认值 <default-value>`。
为了处理任何意外的值，您应该使用 :ref:`revert 函数 <assert-and-require>` 来恢复整个事务，
或者返回一个带有第二个 ``bool`` 值的元组来表示成功。


.. include:: types/value-types.rst

.. include:: types/reference-types.rst

.. include:: types/mapping-types.rst

.. include:: types/operators.rst

.. include:: types/conversion.rst
