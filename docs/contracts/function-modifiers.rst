.. index:: ! function;modifier

.. _modifiers:

******************
函数修饰器
******************

函数修饰器可以用来以声明的方式改变函数的行为。
例如，您可以使用修饰器在执行函数之前自动检查一个条件。

修饰器是合约的可继承属性，可以被派生合约重载，
但只有当它们被标记为 ``virtual`` 时，才能被重载。
详情请见 :ref:`修饰器重载 <modifier-overriding>`。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;

        // 这个合约只定义了一个修饰器，但没有使用它：
        // 它将在派生合约中使用。
        // 修饰器所修饰的函数体会被插入到特殊符号 `_;` 的位置。
        // 这意味着，如果所有者调用这个函数，这个函数就会被执行，
        // 否则就会抛出一个异常。
        modifier onlyOwner {
            require(
                msg.sender == owner,
                "Only owner can call this function."
            );
            _;
        }
    }

    contract destructible is owned {
        // 这个合约从 `owned` 合约继承了 `onlyOwner` 修饰器，
        // 并将其应用于 `destroy` 函数，
        // 只有在合约里保存的 owner 调用 `destroy` 函数，才会生效。
        function destroy() public onlyOwner {
            selfdestruct(owner);
        }
    }

    contract priced {
        // 修饰器可以接受参数：
        modifier costs(uint price) {
            if (msg.value >= price) {
                _;
            }
        }
    }

    contract Register is priced, destructible {
        mapping (address => bool) registeredAddresses;
        uint price;

        constructor(uint initialPrice) { price = initialPrice; }

        // 在这里也使用关键字 `payable` 非常重要，
        // 否则函数会自动拒绝所有发送给它的以太币。
        function register() public payable costs(price) {
            registeredAddresses[msg.sender] = true;
        }

        function changePrice(uint price_) public onlyOwner {
            price = price_;
        }
    }

    contract Mutex {
        bool locked;
        modifier noReentrancy() {
            require(
                !locked,
                "Reentrant call."
            );
            locked = true;
            _;
            locked = false;
        }

        /// 这个函数受互斥量保护，这意味着 `msg.sender.call` 中的重入调用不能再次调用  `f`。
        /// `return 7` 语句指定返回值为 7，但修饰器中的语句 `locked = false` 仍会执行。
        function f() public noReentrancy returns (uint) {
            (bool success,) = msg.sender.call("");
            require(success);
            return 7;
        }
    }

如果您想访问定义在合约 ``C`` 中的修饰器 ``m``，
您可以使用 ``C.m`` 来引用它而不需要虚拟查询。
只能使用定义在当前合约或其基础合约中的修饰器。
修饰器也可以定义在库合约中，但其使用仅限于同一库合约的函数。

如果同一个函数有多个修饰器，它们之间以空格隔开，并按照所呈现的顺序进行评估运算。

修饰器不能隐式地访问或改变它们所修改的函数的参数和返回值。
它们的值只能在调用的时候明确地传递给它们。

修饰器或函数体的显式返回只离开当前修饰器或函数体。
返回变量会被赋值，但整个执行逻辑会从前一个修饰器中定义的 ``_`` 之后继续执行。

.. warning::
    在Solidity的早期版本中，具有修饰器的函数中的 ``return`` 语句会表现的不同。

用 ``return;`` 从修饰器显式返回并不影响函数返回的值。
然而，修饰器可以选择完全不执行函数主体，在这种情况下，
返回变量被设置为 :ref:`默认值 <default-value>`，就像函数有一个空主体一样。

``_`` 符号可以在修饰器中多次出现。每次出现都会被替换成函数体。

允许修饰器参数使用任意表达式，在这种情况下，所有从函数中可见的符号在修饰器中都是可见的。
修饰器中引入的符号在函数中是不可见的（因为它们可能因重载而改变）。
