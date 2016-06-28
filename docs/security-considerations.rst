#######################
Security Considerations
#######################

While it is usually quite easy to build software that works as expected,
it is much harder to check that nobody can use it in a way that was **not** anticipated.

In Solidity, this is even more important because you can use smart contracts
to handle tokens or even more valuable things and every execution of a smart
contract happens in public as is mostly open source.

Of course you always have to consider how much is at stake:
You can compare a smart contract with a web service that is open to the
public (and thus also to malicous actors) and perhaps even open source.
If you only store your grocery list on that web service, you might not have
take too much care, but if you manage your bank account using that web service,
you should be more careful.

This section will list some pitfalls and general security recommendations but
can of course never be complete. Also keep in mind that even if your
smart contract code is bug-free, the compiler or the platform itself might
have a bug.

As always with open source documentation, please help us extend this section
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

  // THIS CONTRACT CONTAINS A BUG - DO NOT USE
  contract Fund {
    /// Mapping of ether shares of the contract.
    mapping(address => uint) shares;
    /// Withdraw your share.
    function withdraw() {
      if (msg.sender.send(shares[msg.sender]))
        shares[msg.sender] = 0;
    }
  }

The problem is not too serious here because of the limited gas as part
of ``send``, but it still exposes a weakness: Ether transfer always
includes code execution, so the recipient could be a contract that calls
back into ``withdraw``. This would enable it to get a multiple refund and
basically retrieve all the Ether in the contract.

To avoid re-entrancy, you can use the Checks-Effects-Interactions pattern as
outlined further below:

::

  contract Fund {
    /// Mapping of ether shares of the contract.
    mapping(address => uint) shares;
    /// Withdraw your share.
    function withdraw() {
      var share = shares[msg.sender];
      shares[msg.sender] = 0;
      if (!msg.sender.send(share))
        throw;
    }
  }


Note that re-entrancy is not only an effect of Ether transfer but of any
function call on another contract. Furthermore, you also have to take
multi-contract situations into account. A called contract could modify the
state of another contract you depend on.

Gas Limit and Loops
===================

Loops that do not have a fixed number of iterations, e.g. loops that depends on storage values, have to be used carefully:
Due to the block gas limit, transactions can only consume a certain amount of gas. Either explicitly or just due to
normal operation, the number of iterations in a loop can grow beyond the block gas limit, which can cause the complete
contract to be stalled at a certain point. This does not apply at full extent to ``constant`` functions that are only executed
to read data from the blockchain. Still, such functions may be called by other contracts as part of on-chain operations
and stall those. Please be explicit about such cases in the documentation of your contracts.

Sending and Receiving Ether
===========================

- If a contract receives Ether (without a function being called), the fallback function is executed. The contract can only rely
  on the "gas stipend" (2300 gas) being available to it at that time. This stipend is not enough to access storage in any way.
  To be sure that your contract can receive Ether in that way, check the gas requirements of the fallback function
  (for example in the "details" section in browser-solidity).

- If you want to send ether using ``address.send``, there are certain details to be aware of:

  1. If the recipient is a contract, it causes its fallback function to be executed which can in turn call back into the sending contract
  2. Sending Ether can fail due to the call depth going above 1024. Since the caller is in total control of the call
     depth, they can force the transfer to fail, so make sure to always check the return value of ``send``. Better yet,
     write your contract using a pattern where the recipient can withdraw Ether instead.
  3. Sending Ether can also fail because the recipient runs out of gas (either explicitly by using ``throw`` or
     because the operation is just too expensive). If the return value of ``send`` is checked, this might provide a
     means for the recipient to block progress in the sending contract. Again, the best practise here is to use
     a "withdraw" pattern instead of a "send" pattern.

Callstack Depth
===============

External function calls can fail any time because they exceed the maximum
call stack of 1024. In such situations, Solidity throws an exception.
Malicious actors might be able to force the call stack to a high value
before they interact with your contract. 

Minor Details
=============

- In ``for (var i = 0; i < arrayName.length; i++) { ... }``, the type of ``i`` will be ``uint8``, because this is the smallest type that is required to hold the value ``0``. If the array has more than 255 elements, the loop will not terminate.
- The ``constant`` keyword is currently not enforced by the compiler.
  Furthermore, it is not enforced by the EVM, so a contract function that "claims"
  to be constant might still cause changes to the state.
- Types that do not occupy the full 32 bytes might contain "dirty higher order bits".
  This is especially important if you access ``msg.data`` - it poses a malleability risk.

***************
Recommendations
***************

Restrict the Amount of Ether
============================

Restrict the amount of Ether (or other tokens) that can be stored in a smart
contract. If your source code, the compiler or the platform has a bug, these
funds might be gone. If you want to limit your loss, limit the amount of Ether.

Keep it Small and Modular
=========================

Keep your contracts small and easily understandable. Single out unrelated
functionality in other contracts or into libraries. General recommendations
about source code quality of course apply: Limit the amount of local variables,
the length of functions and so on. Document your functions so that others
can see what your intention was and whether it is different than what the code does.

Program in Checks-Effects-Interactions-way
===========================================

Most functions will first perform some checks (who called the function,
are the arguments in range, did they send enough Ether, does the person
have tokens, ...). These checks should be done first.

As the second step, if all checks passed, effects to the state variables
of the current contract should be made. Interaction with other contracts
should be the very last step in any function.

Early contracts delayed some effects and waited for external function
calls to return in a non-error state. This is often a serious mistake,
because of the re-entrancy problem explained above.

Note that also calls to known contracts might in turn cause calls to
unknown contracts, so it is probably better to just always apply this pattern. 

Include a Failsafe-Mode
=======================

While making your system fully decentralised will remove any intermediary,
it might be a good idea, especially for new code, to include some kind
of fail-safe-mechanism:

You can add a function in your smart contract that performs some
self-checks like "Has any Ether leaked?",
"Is the sum of the tokens equal to the balance of the contract?" or simila things.
Keep in mind that you cannot use too much gas for that, so help though off-chain
computations might be needed there.

If the self-check fails, the contract automatically switches into some kind
of "failsafe" mode, which e.g. disables most of the features, hands over
control to a fixed and trusted third party or just converts the contract into
a simple "give me back my money"-contract.


*******************
Formal Verification
*******************

Using formal verification, it is possible to perform an automated mathematical
proof that your source code fulfills a certain formal specification.
The specification is still formal (just as the source code), but usually much
simpler. There is a prototype in Solidity that performs formal verification and
it will be better documented soon.

Note that formal verification itself can only help you understand the
difference between what you did (the specification) and how you did it
(the actual implementation). You still need to check whether the specification
is what you wanted and that you did not miss any unintended effects of it.