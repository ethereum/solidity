.. index:: ! function;modifier

.. _modifiers:

******************
Function Modifiers
******************

Modifiers can be used to change the behavior of functions in a declarative way.
For example,
you can use a modifier to automatically check a condition prior to executing the function.

Modifiers are
inheritable properties of contracts and may be overridden by derived contracts, but only
if they are marked ``virtual``. For details, please see
:ref:`Modifier Overriding <modifier-overriding>`.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;
    // This will report a warning due to deprecated selfdestruct

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;

        // This contract only defines a modifier but does not use
        // it: it will be used in derived contracts.
        // The function body is inserted where the special symbol
        // `_;` in the definition of a modifier appears.
        // This means that if the owner calls this function, the
        // function is executed and otherwise, an exception is
        // thrown.
        modifier onlyOwner {
            require(
                msg.sender == owner,
                "Only owner can call this function."
            );
            _;
        }
    }

    contract destructible is owned {
        // This contract inherits the `onlyOwner` modifier from
        // `owned` and applies it to the `destroy` function, which
        // causes that calls to `destroy` only have an effect if
        // they are made by the stored owner.
        function destroy() public onlyOwner {
            selfdestruct(owner);
        }
    }

    contract priced {
        // Modifiers can receive arguments:
        modifier costs(uint price) {
            if (msg.value >= price) {
                _;
            }
        }
    }

    contract Register is priced, destructible {
        mapping(address => bool) registeredAddresses;
        uint price;

        constructor(uint initialPrice) { price = initialPrice; }

        // It is important to also provide the
        // `payable` keyword here, otherwise the function will
        // automatically reject all Ether sent to it.
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

        /// This function is protected by a mutex, which means that
        /// reentrant calls from within `msg.sender.call` cannot call `f` again.
        /// The `return 7` statement assigns 7 to the return value but still
        /// executes the statement `locked = false` in the modifier.
        function f() public noReentrancy returns (uint) {
            (bool success,) = msg.sender.call("");
            require(success);
            return 7;
        }
    }

If you want to access a modifier ``m`` defined in a contract ``C``, you can use ``C.m`` to
reference it without virtual lookup. It is only possible to use modifiers defined in the current
contract or its base contracts. Modifiers can also be defined in libraries but their use is
limited to functions of the same library.

Multiple modifiers are applied to a function by specifying them in a
whitespace-separated list and are evaluated in the order presented.

Modifiers cannot implicitly access or change the arguments and return values of functions they modify.
Their values can only be passed to them explicitly at the point of invocation.

In function modifiers, it is necessary to specify when you want the function to which the modifier is
applied to be run. The placeholder statement (denoted by a single underscore character ``_``) is used to
denote where the body of the function being modified should be inserted. Note that the
placeholder operator is different from using underscores as leading or trailing characters in variable
names, which is a stylistic choice.

Explicit returns from a modifier or function body only leave the current
modifier or function body. Return variables are assigned and
control flow continues after the ``_`` in the preceding modifier.

.. warning::
    In an earlier version of Solidity, ``return`` statements in functions
    having modifiers behaved differently.

An explicit return from a modifier with ``return;`` does not affect the values returned by the function.
The modifier can, however, choose not to execute the function body at all and in that case the return
variables are set to their :ref:`default values<default-value>` just as if the function had an empty
body.

The ``_`` symbol can appear in the modifier multiple times. Each occurrence is replaced with
the function body, and the function returns the return value of the final occurrence.

Arbitrary expressions are allowed for modifier arguments and in this context,
all symbols visible from the function are visible in the modifier. Symbols
introduced in the modifier are not visible in the function (as they might
change by overriding).
