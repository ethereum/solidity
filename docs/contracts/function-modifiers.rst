.. index:: ! function;modifier

.. _modifiers:

******************
Function Modifiers
******************

Modifiers can be used to easily change the behaviour of functions.  For example,
they can automatically check a condition prior to executing the function. Modifiers are
inheritable properties of contracts and may be overridden by derived contracts.

::

    pragma solidity ^0.5.0;

    contract owned {
        constructor() public { owner = msg.sender; }
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

    contract mortal is owned {
        // This contract inherits the `onlyOwner` modifier from
        // `owned` and applies it to the `close` function, which
        // causes that calls to `close` only have an effect if
        // they are made by the stored owner.
        function close() public onlyOwner {
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

    contract Register is priced, owned {
        mapping (address => bool) registeredAddresses;
        uint price;

        constructor(uint initialPrice) public { price = initialPrice; }

        // It is important to also provide the
        // `payable` keyword here, otherwise the function will
        // automatically reject all Ether sent to it.
        function register() public payable costs(price) {
            registeredAddresses[msg.sender] = true;
        }

        function changePrice(uint _price) public onlyOwner {
            price = _price;
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

Multiple modifiers are applied to a function by specifying them in a
whitespace-separated list and are evaluated in the order presented.

.. warning::
    In an earlier version of Solidity, ``return`` statements in functions
    having modifiers behaved differently.

Explicit returns from a modifier or function body only leave the current
modifier or function body. Return variables are assigned and
control flow continues after the "_" in the preceding modifier.

Arbitrary expressions are allowed for modifier arguments and in this context,
all symbols visible from the function are visible in the modifier. Symbols
introduced in the modifier are not visible in the function (as they might
change by overriding).
