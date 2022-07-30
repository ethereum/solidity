.. index:: ! contract

.. _contracts:

##########
合约
##########

Solidity中的合约类似于面向对象语言中的类。
它们在状态变量中包含持久的数据，以及可以修改这些变量的函数。
在不同的合约（实例）上调用一个函数将执行一个EVM函数调用，
从而切换上下文，使调用合约中的状态变量无法访问。
一个合约和它的函数需要被调用才会发生。
在以太坊中没有 "cron" 的概念，在特定的事件中自动调用一个函数。

.. include:: contracts/creating-contracts.rst

.. include:: contracts/visibility-and-getters.rst

.. include:: contracts/function-modifiers.rst

.. include:: contracts/constant-state-variables.rst
.. include:: contracts/functions.rst

.. include:: contracts/events.rst
.. include:: contracts/errors.rst

.. include:: contracts/inheritance.rst

.. include:: contracts/abstract-contracts.rst
.. include:: contracts/interfaces.rst

.. include:: contracts/libraries.rst

.. include:: contracts/using-for.rst
