.. _security_considerations:

#######################
Security Considerations
#######################

While it is usually quite easy to build software that works as expected,
it is much harder to check that nobody can use it in a way that was **not** anticipated.

In Solidity, this is even more important because you can use smart contracts
to handle tokens or, possibly, even more valuable things. Furthermore, every
execution of a smart contract happens in public and, in addition to that,
the source code is often available.

Of course you always have to consider how much is at stake:
You can compare a smart contract with a web service that is open to the
public (and thus, also to malicious actors) and perhaps even open source.
If you only store your grocery list on that web service, you might not have
to take too much care, but if you manage your bank account using that web service,
you should be more careful.

This section will list some pitfalls and general security recommendations but
can, of course, never be complete.  Also, keep in mind that even if your smart
contract code is bug-free, the compiler or the platform itself might have a
bug. A list of some publicly known security-relevant bugs of the compiler can
be found in the :ref:`list of known bugs<known_bugs>`, which is also
machine-readable. Note that there is a bug bounty program that covers the code
generator of the Solidity compiler.

As always, with open source documentation, please help us extend this section
(especially, some examples would not hurt)!

NOTE: In addition to the list below, you can find more security recommendations and best practices
`in Guy Lando's knowledge list <https://github.com/guylando/KnowledgeLists/blob/master/EthereumSmartContracts.md>`_ and
`the Consensys GitHub repo <https://consensys.github.io/smart-contract-best-practices/>`_.

********
Pitfalls
********

Private Information and Randomness
==================================

Everything you use in a smart contract is publicly visible, even
local variables and state variables marked ``private``.

Using random numbers in smart contracts is quite tricky if you do not want
block builders to be able to cheat.

Re-Entrancy
===========

Any interaction from a contract (A) with another contract (B) and any transfer
of Ether hands over control to that contract (B). This makes it possible for B
to call back into A before this interaction is completed. To give an example,
the following code contains a bug (it is just a snippet and not a
complete contract):

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract Fund {
        /// @dev Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            if (payable(msg.sender).send(shares[msg.sender]))
                shares[msg.sender] = 0;
        }
    }

The problem is not too serious here because of the limited gas as part
of ``send``, but it still exposes a weakness: Ether transfer can always
include code execution, so the recipient could be a contract that calls
back into ``withdraw``. This would let it get multiple refunds and
basically retrieve all the Ether in the contract. In particular, the
following contract will allow an attacker to refund multiple times
as it uses ``call`` which forwards all remaining gas by default:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract Fund {
        /// @dev Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            (bool success,) = msg.sender.call{value: shares[msg.sender]}("");
            if (success)
                shares[msg.sender] = 0;
        }
    }

To avoid re-entrancy, you can use the Checks-Effects-Interactions pattern as
demonstrated below:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Fund {
        /// @dev Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            uint share = shares[msg.sender];
            shares[msg.sender] = 0;
            payable(msg.sender).transfer(share);
        }
    }

The Checks-Effects-Interactions pattern ensures that all code paths through a contract complete all required checks
of the supplied parameters before modifying the contract's state (Checks); only then it makes any changes to the state (Effects);
it may make calls to functions in other contracts *after* all planned state changes have been written to
storage (Interactions). This is a common foolproof way to prevent *re-entrancy attacks*, where an externally called
malicious contract is able to double-spend an allowance, double-withdraw a balance, among other things, by using logic that calls back into the
original contract before it has finalized its transaction.

Note that re-entrancy is not only an effect of Ether transfer but of any
function call on another contract. Furthermore, you also have to take
multi-contract situations into account. A called contract could modify the
state of another contract you depend on.

Gas Limit and Loops
===================

Loops that do not have a fixed number of iterations, for example, loops that depend on storage values, have to be used carefully:
Due to the block gas limit, transactions can only consume a certain amount of gas. Either explicitly or just due to
normal operation, the number of iterations in a loop can grow beyond the block gas limit which can cause the complete
contract to be stalled at a certain point. This may not apply to ``view`` functions that are only executed
to read data from the blockchain. Still, such functions may be called by other contracts as part of on-chain operations
and stall those. Please be explicit about such cases in the documentation of your contracts.

Sending and Receiving Ether
===========================

- Neither contracts nor "external accounts" are currently able to prevent that someone sends them Ether.
  Contracts can react on and reject a regular transfer, but there are ways
  to move Ether without creating a message call. One way is to simply "mine to"
  the contract address and the second way is using ``selfdestruct(x)``.

- If a contract receives Ether (without a function being called),
  either the :ref:`receive Ether <receive-ether-function>`
  or the :ref:`fallback <fallback-function>` function is executed.
  If it does not have a receive nor a fallback function, the Ether will be
  rejected (by throwing an exception). During the execution of one of these
  functions, the contract can only rely on the "gas stipend" it is passed (2300
  gas) being available to it at that time. This stipend is not enough to modify
  storage (do not take this for granted though, the stipend might change with
  future hard forks). To be sure that your contract can receive Ether in that
  way, check the gas requirements of the receive and fallback functions
  (for example in the "details" section in Remix).

- There is a way to forward more gas to the receiving contract using
  ``addr.call{value: x}("")``. This is essentially the same as ``addr.transfer(x)``,
  only that it forwards all remaining gas and opens up the ability for the
  recipient to perform more expensive actions (and it returns a failure code
  instead of automatically propagating the error). This might include calling back
  into the sending contract or other state changes you might not have thought of.
  So it allows for great flexibility for honest users but also for malicious actors.

- Use the most precise units to represent the wei amount as possible, as you lose
  any that is rounded due to a lack of precision.

- If you want to send Ether using ``address.transfer``, there are certain details to be aware of:

  1. If the recipient is a contract, it causes its receive or fallback function
     to be executed which can, in turn, call back the sending contract.
  2. Sending Ether can fail due to the call depth going above 1024. Since the
     caller is in total control of the call depth, they can force the
     transfer to fail; take this possibility into account or use ``send`` and
     make sure to always check its return value. Better yet, write your
     contract using a pattern where the recipient can withdraw Ether instead.
  3. Sending Ether can also fail because the execution of the recipient
     contract requires more than the allotted amount of gas (explicitly by
     using :ref:`require <assert-and-require>`, :ref:`assert <assert-and-require>`,
     :ref:`revert <assert-and-require>` or because the
     operation is too expensive) - it "runs out of gas" (OOG).  If you
     use ``transfer`` or ``send`` with a return value check, this might
     provide a means for the recipient to block progress in the sending
     contract. Again, the best practice here is to use a :ref:`"withdraw"
     pattern instead of a "send" pattern <withdrawal_pattern>`.

Call Stack Depth
================

External function calls can fail any time because they exceed the maximum
call stack size limit of 1024. In such situations, Solidity throws an exception.
Malicious actors might be able to force the call stack to a high value
before they interact with your contract. Note that, since `Tangerine Whistle <https://eips.ethereum.org/EIPS/eip-608>`_ hardfork, the `63/64 rule <https://eips.ethereum.org/EIPS/eip-150>`_ makes call stack depth attack impractical. Also note that the call stack and the expression stack are unrelated, even though both have a size limit of 1024 stack slots.

Note that ``.send()`` does **not** throw an exception if the call stack is
depleted but rather returns ``false`` in that case. The low-level functions
``.call()``, ``.delegatecall()`` and ``.staticcall()`` behave in the same way.

Authorized Proxies
==================

If your contract can act as a proxy, i.e. if it can call arbitrary contracts
with user-supplied data, then the user can essentially assume the identity
of the proxy contract. Even if you have other protective measures in place,
it is best to build your contract system such that the proxy does not have
any permissions (not even for itself). If needed, you can accomplish that
using a second proxy:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.0;
    contract ProxyWithMoreFunctionality {
        PermissionlessProxy proxy;

        function callOther(address addr, bytes memory payload) public
                returns (bool, bytes memory) {
            return proxy.callOther(addr, payload);
        }
        // Other functions and other functionality
    }

    // This is the full contract, it has no other functionality and
    // requires no privileges to work.
    contract PermissionlessProxy {
        function callOther(address addr, bytes memory payload) public
                returns (bool, bytes memory) {
            return addr.call(payload);
        }
    }

tx.origin
=========

Never use tx.origin for authorization. Let's say you have a wallet contract like this:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract TxUserWallet {
        address owner;

        constructor() {
            owner = msg.sender;
        }

        function transferTo(address payable dest, uint amount) public {
            // THE BUG IS RIGHT HERE, you must use msg.sender instead of tx.origin
            require(tx.origin == owner);
            dest.transfer(amount);
        }
    }

Now someone tricks you into sending Ether to the address of this attack wallet:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    interface TxUserWallet {
        function transferTo(address payable dest, uint amount) external;
    }

    contract TxAttackWallet {
        address payable owner;

        constructor() {
            owner = payable(msg.sender);
        }

        receive() external payable {
            TxUserWallet(msg.sender).transferTo(owner, msg.sender.balance);
        }
    }

If your wallet had checked ``msg.sender`` for authorization, it would get the address of the attack wallet, instead of the owner address. But by checking ``tx.origin``, it gets the original address that kicked off the transaction, which is still the owner address. The attack wallet instantly drains all your funds.

.. _underflow-overflow:

Two's Complement / Underflows / Overflows
=========================================

As in many programming languages, Solidity's integer types are not actually integers.
They resemble integers when the values are small, but cannot represent arbitrarily large numbers.

The following code causes an overflow because the result of the addition is too large
to be stored in the type ``uint8``:

.. code-block:: solidity

  uint8 x = 255;
  uint8 y = 1;
  return x + y;

Solidity has two modes in which it deals with these overflows: Checked and Unchecked or "wrapping" mode.

The default checked mode will detect overflows and cause a failing assertion. You can disable this check
using ``unchecked { ... }``, causing the overflow to be silently ignored. The above code would return
``0`` if wrapped in ``unchecked { ... }``.

Even in checked mode, do not assume you are protected from overflow bugs.
In this mode, overflows will always revert. If it is not possible to avoid the
overflow, this can lead to a smart contract being stuck in a certain state.

In general, read about the limits of two's complement representation, which even has some
more special edge cases for signed numbers.

Try to use ``require`` to limit the size of inputs to a reasonable range and use the
:ref:`SMT checker<smt_checker>` to find potential overflows.

.. _clearing-mappings:

Clearing Mappings
=================

The Solidity type ``mapping`` (see :ref:`mapping-types`) is a storage-only
key-value data structure that does not keep track of the keys that were
assigned a non-zero value.  Because of that, cleaning a mapping without extra
information about the written keys is not possible.
If a ``mapping`` is used as the base type of a dynamic storage array, deleting
or popping the array will have no effect over the ``mapping`` elements.  The
same happens, for example, if a ``mapping`` is used as the type of a member
field of a ``struct`` that is the base type of a dynamic storage array.  The
``mapping`` is also ignored in assignments of structs or arrays containing a
``mapping``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Map {
        mapping (uint => uint)[] array;

        function allocate(uint newMaps) public {
            for (uint i = 0; i < newMaps; i++)
                array.push();
        }

        function writeMap(uint map, uint key, uint value) public {
            array[map][key] = value;
        }

        function readMap(uint map, uint key) public view returns (uint) {
            return array[map][key];
        }

        function eraseMaps() public {
            delete array;
        }
    }

Consider the example above and the following sequence of calls: ``allocate(10)``,
``writeMap(4, 128, 256)``.
At this point, calling ``readMap(4, 128)`` returns 256.
If we call ``eraseMaps``, the length of state variable ``array`` is zeroed, but
since its ``mapping`` elements cannot be zeroed, their information stays alive
in the contract's storage.
After deleting ``array``, calling ``allocate(5)`` allows us to access
``array[4]`` again, and calling ``readMap(4, 128)`` returns 256 even without
another call to ``writeMap``.

If your ``mapping`` information must be deleted, consider using a library similar to
`iterable mapping <https://github.com/ethereum/dapp-bin/blob/master/library/iterable_mapping.sol>`_,
allowing you to traverse the keys and delete their values in the appropriate ``mapping``.

Minor Details
=============

- Types that do not occupy the full 32 bytes might contain "dirty higher order bits".
  This is especially important if you access ``msg.data`` - it poses a malleability risk:
  You can craft transactions that call a function ``f(uint8 x)`` with a raw byte argument
  of ``0xff000001`` and with ``0x00000001``. Both are fed to the contract and both will
  look like the number ``1`` as far as ``x`` is concerned, but ``msg.data`` will
  be different, so if you use ``keccak256(msg.data)`` for anything, you will get different results.

***************
Recommendations
***************

Take Warnings Seriously
=======================

If the compiler warns you about something, you should change it.
Even if you do not think that this particular warning has security
implications, there might be another issue buried beneath it.
Any compiler warning we issue can be silenced by slight changes to the
code.

Always use the latest version of the compiler to be notified about all recently
introduced warnings.

Messages of type ``info`` issued by the compiler are not dangerous, and simply
represent extra suggestions and optional information that the compiler thinks
might be useful to the user.

Restrict the Amount of Ether
============================

Restrict the amount of Ether (or other tokens) that can be stored in a smart
contract. If your source code, the compiler or the platform has a bug, these
funds may be lost. If you want to limit your loss, limit the amount of Ether.

Keep it Small and Modular
=========================

Keep your contracts small and easily understandable. Single out unrelated
functionality in other contracts or into libraries. General recommendations
about source code quality of course apply: Limit the amount of local variables,
the length of functions and so on. Document your functions so that others
can see what your intention was and whether it is different than what the code does.

Use the Checks-Effects-Interactions Pattern
===========================================

Most functions will first perform some checks (who called the function,
are the arguments in range, did they send enough Ether, does the person
have tokens, etc.). These checks should be done first.

As the second step, if all checks passed, effects to the state variables
of the current contract should be made. Interaction with other contracts
should be the very last step in any function.

Early contracts delayed some effects and waited for external function
calls to return in a non-error state. This is often a serious mistake
because of the re-entrancy problem explained above.

Note that, also, calls to known contracts might in turn cause calls to
unknown contracts, so it is probably better to just always apply this pattern.

Include a Fail-Safe Mode
========================

While making your system fully decentralised will remove any intermediary,
it might be a good idea, especially for new code, to include some kind
of fail-safe mechanism:

You can add a function in your smart contract that performs some
self-checks like "Has any Ether leaked?",
"Is the sum of the tokens equal to the balance of the contract?" or similar things.
Keep in mind that you cannot use too much gas for that, so help through off-chain
computations might be needed there.

If the self-check fails, the contract automatically switches into some kind
of "failsafe" mode, which, for example, disables most of the features, hands over
control to a fixed and trusted third party or just converts the contract into
a simple "give me back my money" contract.

Ask for Peer Review
===================

The more people examine a piece of code, the more issues are found.
Asking people to review your code also helps as a cross-check to find out whether your code
is easy to understand - a very important criterion for good smart contracts.
