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
can, of course, never be complete. Also, keep in mind that even if your
smart contract code is bug-free, the compiler or the platform itself might
have a bug. A list of some publicly known security-relevant bugs of the compiler
can be found in the
:ref:`list of known bugs<known_bugs>`, which is also machine-readable. Note
that there is a bug bounty program that covers the code generator of the
Solidity compiler.

As always, with open source documentation, please help us extend this section
(especially, some examples would not hurt)!

********
Pitfalls
********

Private Information and Randomness
==================================

Everything you use in a smart contract is publicly visible, even
local variables and state variables marked ``private``.

Using random numbers in smart contracts is quite tricky if you do not want
miners to be able to cheat.

Re-Entrancy
===========

Any interaction from a contract (A) with another contract (B) and any transfer
of Ether hands over control to that contract (B). This makes it possible for B
to call back into A before this interaction is completed. To give an example,
the following code contains a bug (it is just a snippet and not a
complete contract):

::

    pragma solidity >=0.4.0 <0.7.0;

    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract Fund {
        /// Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            if (msg.sender.send(shares[msg.sender]))
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

::

    pragma solidity >=0.4.0 <0.7.0;

    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract Fund {
        /// Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            (bool success,) = msg.sender.call.value(shares[msg.sender])("");
            if (success)
                shares[msg.sender] = 0;
        }
    }

To avoid re-entrancy, you can use the Checks-Effects-Interactions pattern as
outlined further below:

::

    pragma solidity >=0.4.11 <0.7.0;

    contract Fund {
        /// Mapping of ether shares of the contract.
        mapping(address => uint) shares;
        /// Withdraw your share.
        function withdraw() public {
            uint share = shares[msg.sender];
            shares[msg.sender] = 0;
            msg.sender.transfer(share);
        }
    }

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

- If a contract receives Ether (without a function being called), the fallback function is executed.
  If it does not have a fallback function, the Ether will be rejected (by throwing an exception).
  During the execution of the fallback function, the contract can only rely
  on the "gas stipend" it is passed (2300 gas) being available to it at that time. This stipend is not enough to modify storage
  (do not take this for granted though, the stipend might change with future hard forks).
  To be sure that your contract can receive Ether in that way, check the gas requirements of the fallback function
  (for example in the "details" section in Remix).

- There is a way to forward more gas to the receiving contract using
  ``addr.call.value(x)("")``. This is essentially the same as ``addr.transfer(x)``,
  only that it forwards all remaining gas and opens up the ability for the
  recipient to perform more expensive actions (and it returns a failure code
  instead of automatically propagating the error). This might include calling back
  into the sending contract or other state changes you might not have thought of.
  So it allows for great flexibility for honest users but also for malicious actors.

- If you want to send Ether using ``address.transfer``, there are certain details to be aware of:

  1. If the recipient is a contract, it causes its fallback function to be executed which can, in turn, call back the sending contract.
  2. Sending Ether can fail due to the call depth going above 1024. Since the caller is in total control of the call
     depth, they can force the transfer to fail; take this possibility into account or use ``send`` and make sure to always check its return value. Better yet,
     write your contract using a pattern where the recipient can withdraw Ether instead.
  3. Sending Ether can also fail because the execution of the recipient contract
     requires more than the allotted amount of gas (explicitly by using ``require``,
     ``assert``, ``revert``, ``throw`` or
     because the operation is just too expensive) - it "runs out of gas" (OOG).
     If you use ``transfer`` or ``send`` with a return value check, this might provide a
     means for the recipient to block progress in the sending contract. Again, the best practice here is to use
     a :ref:`"withdraw" pattern instead of a "send" pattern <withdrawal_pattern>`.

Callstack Depth
===============

External function calls can fail any time because they exceed the maximum
call stack of 1024. In such situations, Solidity throws an exception.
Malicious actors might be able to force the call stack to a high value
before they interact with your contract.

Note that ``.send()`` does **not** throw an exception if the call stack is
depleted but rather returns ``false`` in that case. The low-level functions
``.call()``, ``.callcode()``, ``.delegatecall()`` and ``.staticcall()`` behave
in the same way.

tx.origin
=========

Never use tx.origin for authorization. Let's say you have a wallet contract like this:

::

    pragma solidity >=0.5.0 <0.7.0;

    // THIS CONTRACT CONTAINS A BUG - DO NOT USE
    contract TxUserWallet {
        address owner;

        constructor() public {
            owner = msg.sender;
        }

        function transferTo(address payable dest, uint amount) public {
            require(tx.origin == owner);
            dest.transfer(amount);
        }
    }

Now someone tricks you into sending ether to the address of this attack wallet:

::

    pragma solidity >=0.5.0 <0.7.0;

    interface TxUserWallet {
        function transferTo(address payable dest, uint amount) external;
    }

    contract TxAttackWallet {
        address payable owner;

        constructor() public {
            owner = msg.sender;
        }

        function() external {
            TxUserWallet(msg.sender).transferTo(owner, msg.sender.balance);
        }
    }

If your wallet had checked ``msg.sender`` for authorization, it would get the address of the attack wallet, instead of the owner address. But by checking ``tx.origin``, it gets the original address that kicked off the transaction, which is still the owner address. The attack wallet instantly drains all your funds.

.. _underflow-overflow:

Two's Complement / Underflows / Overflows
=========================================

As in many programming languages, Solidity's integer types are not actually integers.
They resemble integers when the values are small, but behave differently if the numbers are larger.
For example, the following is true: ``uint8(255) + uint8(1) == 0``. This situation is called
an *overflow*. It occurs when an operation is performed that requires a fixed size variable
to store a number (or piece of data) that is outside the range of the variable's data type.
An *underflow* is the converse situation: ``uint8(0) - uint8(1) == 255``.

In general, read about the limits of two's complement representation, which even has some
more special edge cases for signed numbers.

Try to use ``require`` to limit the size of inputs to a reasonable range and use the
:ref:`SMT checker<smt_checker>` to find potential overflows, or
use a library like
`SafeMath <https://github.com/OpenZeppelin/openzeppelin-solidity/blob/master/contracts/math/SafeMath.sol>`_
if you want all overflows to cause a revert.

Code such as ``require((balanceOf[_to] + _value) >= balanceOf[_to])`` can also help you check if values are what you expect.

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

If the compiler warns you about something, you should better change it.
Even if you do not think that this particular warning has security
implications, there might be another issue buried beneath it.
Any compiler warning we issue can be silenced by slight changes to the
code.

Always use the latest version of the compiler to be notified about all recently
introduced warnings.

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

.. _formal_verification:

*******************
Formal Verification
*******************

Using formal verification, it is possible to perform an automated mathematical
proof that your source code fulfills a certain formal specification.
The specification is still formal (just as the source code), but usually much
simpler.

Note that formal verification itself can only help you understand the
difference between what you did (the specification) and how you did it
(the actual implementation). You still need to check whether the specification
is what you wanted and that you did not miss any unintended effects of it.

Solidity implements a formal verification approach based on SMT solving.  The
SMTChecker module automatically tries to prove that the code satisfies the
specification given by ``require/assert`` statements. That is, it considers
``require`` statements as assumptions and tries to prove that the conditions
inside ``assert`` statements are always true.  If an assertion failure is
found, a counterexample is given to the user, showing how the assertion can be
violated.

The SMTChecker also checks automatically for arithmetic underflow/overflow,
trivial conditions and unreachable code.
It is currently an experimental feature, therefore in order to use it you need
to enable it via :ref:`a pragma directive<smt_checker>`.

The SMTChecker traverses the Solidity AST creating and collecting program constraints.
When it encounters a verification target, an SMT solver is invoked to determine the outcome.
If a check fails, the SMTChecker provides specific input values that lead to the failure.

For more details on how the SMT encoding works internally, see the paper
`SMT-based Verification of Solidity Smart Contracts <https://github.com/leonardoalt/text/blob/master/solidity_isola_2018/main.pdf>`_.

Abstraction and False Positives
===============================

The SMTChecker implements abstractions in an incomplete and sound way: If a bug
is reported, it might be a false positive introduced by abstractions (due to
erasing knowledge or using a non-precise type). If it determines that a
verification target is safe, it is indeed safe, that is, there are no false
negatives (unless there is a bug in the SMTChecker).

The SMT encoding tries to be as precise as possible, mapping Solidity types
and expressions to their closest `SMT-LIB <http://smtlib.cs.uiowa.edu/>`_
representation, as shown in the table below.

+-----------------------+--------------+-----------------------------+
|Solidity type          |SMT sort      |Theories (quantifier-free)   |
+=======================+==============+=============================+
|Boolean                |Bool          |Bool                         |
+-----------------------+--------------+-----------------------------+
|intN, uintN, address,  |Integer       |LIA, NIA                     |
|bytesN, enum           |              |                             |
+-----------------------+--------------+-----------------------------+
|array, mapping         |Array         |Arrays                       |
+-----------------------+--------------+-----------------------------+
|other types            |Integer       |LIA                          |
+-----------------------+--------------+-----------------------------+

Types that are not yet supported are abstracted by a single 256-bit unsigned integer,
where their unsupported operations are ignored.

Function calls to the same contract (or base contracts) are inlined when
possible, that is, when their implementation is available.
Calls to functions in other contracts are not inlined even if their code is
available, since we cannot guarantee that the actual deployed code is the same.
Complex pure functions are abstracted by an uninterpreted function (UF) over
the arguments.

+-----------------------------------+--------------------------------------+
|Functions                          |SMT behavior                          |
+===================================+======================================+
|``assert``                         |Verification target                   |
+-----------------------------------+--------------------------------------+
|``require``                        |Assumption                            |
+-----------------------------------+--------------------------------------+
|internal                           |Inline function call                  |
+-----------------------------------+--------------------------------------+
|external                           |Inline function call                  |
|                                   |Erase knowledge about state variables |
|                                   |and local storage references          |
+-----------------------------------+--------------------------------------+
|``gasleft``, ``blockhash``,        |Abstracted with UF                    |
|``keccak256``, ``ecrecover``       |                                      |
|``ripemd160``, ``addmod``,         |                                      |
|``mulmod``                         |                                      |
+-----------------------------------+--------------------------------------+
|pure functions without             |Abstracted with UF                    |
|implementation (external or        |                                      |
|complex)                           |                                      |
+-----------------------------------+--------------------------------------+
|external functions without         |Unsupported                           |
|implementation                     |                                      |
+-----------------------------------+--------------------------------------+
|others                             |Currently unsupported                 |
+-----------------------------------+--------------------------------------+

Using abstraction means loss of precise knowledge, but in many cases it does
not mean loss of proving power.

::

   pragma solidity >=0.5.0;
   pragma experimental SMTChecker;

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

In the example above, the SMTChecker is not expressive enough to actually
compute ``ecrecover``, but by modelling the function calls as uninterpreted
functions we know that the return value is the same when called on equivalent
parameters. This is enough to prove that the assertion above is always true.

Abstracting a function call with an UF can be done for functions known to be
deterministic, and can be easily done for pure functions.  It is however
difficult to do this with general external functions, since they might depend
on state variables.

External function calls also imply that any current knowledge that the
SMTChecker might have regarding mutable state variables needs to be erased to
guarantee no false negatives, since the called external function might direct
or indirectly call a function in the analyzed contract that changes state
variables.

Reference Types and Aliasing
=============================

Solidity implements aliasing for reference types with the same :ref:`data
location<data-location>`.
That means one variable may be modified through a reference to the same data
area.
The SMTChecker does not keep track of which references refer to the same data.
This implies that whenever a local reference or state variable of reference
type is assigned, all knowledge regarding variables of the same type and data
location is erased.
If the type is nested, the knowledge removal also includes all the prefix base
types.

::

   pragma solidity >=0.5.0;
   pragma experimental SMTChecker;
   // This will not compile
   contract Aliasing
   {
      uint[] array;
      function f(
         uint[] memory a,
         uint[] memory b,
         uint[][] memory c,
         uint[] storage d
      ) internal view {
         require(array[0] == 42);
         require(a[0] == 2);
         require(c[0][0] == 2);
         require(d[0] == 2);
         b[0] = 1;
         // Erasing knowledge about memory references should not
         // erase knowledge about state variables.
         assert(array[0] == 42);
         // Fails because `a == b` is possible.
         assert(a[0] == 2);
         // Fails because `c[i] == b` is possible.
         assert(c[0][0] == 2);
         assert(d[0] == 2);
         assert(b[0] == 1);
      }
   }

After the assignment to ``b[0]``, we need to clear knowledge about ``a`` since
it has the same type (``uint[]``) and data location (memory).  We also need to
clear knowledge about ``c``, since its base type is also a ``uint[]`` located
in memory. This implies that some ``c[i]`` could refer to the same data as
``b`` or ``a``.

Notice that we do not clear knowledge about ``array`` and ``d`` because they
are located in storage, even though they also have type ``uint[]``.  However,
if ``d`` was assigned, we would need to clear knowledge about ``array`` and
vice-versa.
