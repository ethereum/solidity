**************************************
Units and Globally Available Variables
**************************************

.. index:: wei, finney, szabo, ether

Ether Units
===========

A literal number can take a suffix of :code:`wei`, :code:`finney`, :code:`szabo` or :code:`ether` to convert between the subdenominations of Ether, where Ether currency numbers without a postfix are assumed to be "wei", e.g. :code:`2 ether == 2000 finney` evaluates to :code:`true`.

.. index:: time, seconds, minutes, hours, days, weeks, years

Time Units
==========

Suffixes of :code:`seconds`, :code:`minutes`, :code:`hours`, :code:`days`, :code:`weeks` and
`years` after literal numbers can be used to convert between units of time where seconds are the base
unit and units are considered naively in the following way:

 * :code:`1 == 1 second`
 * :code:`1 minutes == 60 seconds`
 * :code:`1 hours == 60 minutes`
 * :code:`1 days == 24 hours`
 * :code:`1 weeks = 7 days`
 * :code:`1 years = 365 days`

Take care if you perform calendar calculations using these units, because
not every year equals 365 days and not even every day has 24 hours
because of `leap seconds <https://en.wikipedia.org/wiki/Leap_second>`_.
Due to the fact that leap seconds cannot be predicted, an exact calendar
library has to be updated by an external oracle.

These suffixes cannot be applied to variables. If you want to
interpret some input variable in e.g. days, you can do it in the following way::

    function f(uint start, uint daysAfter) {
        if (now >= start + daysAfter * 1 days) { ... }
    }

Special Variables and Functions
===============================

There are special variables and functions which always exist in the global
namespace and are mainly used to provide information about the blockchain.

.. index:: block, coinbase, difficulty, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, now, gas price, origin


Block and Transaction Properties
------------------------------------

 - :code:`block.coinbase` (:code:`address`): current block miner's address
 - :code:`block.difficulty` (:code:`uint`): current block difficulty
 - :code:`block.gaslimit` (:code:`uint`): current block gaslimit
 - :code:`block.number` (:code:`uint`): current block number
 - :code:`block.blockhash` (:code:`function(uint) returns (bytes32)`): hash of the given block - only for 256 most recent blocks
 - :code:`block.timestamp` (:code:`uint`): current block timestamp
 - :code:`msg.data` (:code:`bytes`): complete calldata
 - :code:`msg.gas` (:code:`uint`): remaining gas
 - :code:`msg.sender` (:code:`address`): sender of the message (current call)
 - :code:`msg.sig` (:code:`bytes4`): first four bytes of the calldata (i.e. function identifier)
 - :code:`msg.value` (:code:`uint`): number of wei sent with the message
 - :code:`now` (:code:`uint`): current block timestamp (alias for :code:`block.timestamp`)
 - :code:`tx.gasprice` (:code:`uint`): gas price of the transaction
 - :code:`tx.origin` (:code:`address`): sender of the transaction (full call chain)

.. note::
    The values of all members of :code:`msg`, including :code:`msg.sender` and
    :code:`msg.value` can change for every **external** function call.
    This includes calls to library functions.

    If you want to implement access restrictions in library functions using
    :code:`msg.sender`, you have to manually supply the value of
    :code:`msg.sender` as an argument.

.. note::
    The block hashes are not available for all blocks for scalability reasons.
    You can only access the hashes of the most recent 256 blocks, all other
    values will be zero.

.. index:: sha3, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography, this, super, selfdestruct, balance, send

Mathematical and Cryptographic Functions
----------------------------------------

:code:`addmod(uint x, uint y, uint k) returns (uint)`:
    compute :code:`(x + y) % k` where the addition is performed with arbitrary precision and does not wrap around at :code:`2**256`.
:code:`mulmod(uint x, uint y, uint k) returns (uint)`:
    compute :code:`(x * y) % k` where the multiplication is performed with arbitrary precision and does not wrap around at :code:`2**256`.
:code:`sha3(...) returns (bytes32)`:
    compute the Ethereum-SHA-3 hash of the (tightly packed) arguments
:code:`sha256(...) returns (bytes32)`:
    compute the SHA-256 hash of the (tightly packed) arguments
:code:`ripemd160(...) returns (bytes20)`:
    compute RIPEMD-160 hash of the (tightly packed) arguments
:code:`ecrecover(bytes32 data, uint8 v, bytes32 r, bytes32 s) returns (address)`:
    recover the address associated with the public key from elliptic curve signature

In the above, "tightly packed" means that the arguments are concatenated without padding.
This means that the following are all identical::

    sha3("ab", "c")
    sha3("abc")
    sha3(0x616263)
    sha3(6382179)
    sha3(97, 98, 99)

If padding is needed, explicit type conversions can be used: :code:`sha3("\x00\x12")` is the
same as :code:`sha3(uint16(0x12))`.

It might be that you run into Out-of-Gas for :code:`sha256`, :code:`ripemd160` or :code:`ecrecover` on a *private blockchain*. The reason for this is that those are implemented as so-called precompiled contracts and these contracts only really exist after they received the first message (although their contract code is hardcoded). Messages to non-existing contracts are more expensive and thus the execution runs into an Out-of-Gas error. A workaround for this problem is to first send e.g. 1 Wei to each of the contracts before you use them in your actual contracts. This is not an issue on the official or test net.

.. index:: this, selfdestruct

Contract Related
----------------

:code:`this` (current contract's type):
    the current contract, explicitly convertible to :ref:`address`

:code:`selfdestruct(address)`:
    destroy the current contract, sending its funds to the given :ref:`address`

Furthermore, all functions of the current contract are callable directly including the current function.

