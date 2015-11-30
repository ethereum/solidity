---
layout: docs
title: Units and Globally Available Variables
link_title: Units and Global Vars
permalink: /docs/units-and-global-variables/
---

## Ether Units

A literal number can take a suffix of `wei`, `finney`, `szabo` or `ether` to convert between the subdenominations of Ether, where Ether currency numbers without a postfix are assumed to be "wei", e.g. `2 ether == 2000 finney` evaluates to `true`.

## Time Units

Suffixes of `seconds`, `minutes`, `hours`, `days`, `weeks` and
`years` after literal numbers can be used to convert between units of time where seconds are the base
unit and units are considered naively in the following way:

 * `1 == 1 second`
 * `1 minutes == 60 seconds`
 * `1 hours == 60 minutes`
 * `1 days == 24 hours`
 * `1 weeks = 7 days`
 * `1 years = 365 days`

Take care if you perform calendar calculations using these units, because
not every year equals 365 days and not even every day has 24 hours
because of [leap seconds](https://en.wikipedia.org/wiki/Leap_second).
Due to the fact that leap seconds cannot be predicted, an exact calendar
library has to be updated by an external oracle.

These suffixes cannot be applied to variables. If you want to
interpret some input variable in e.g. days, you can do it in the following way:

{% highlight JavaScript %}
function f(uint start, uint daysAfter) {
  if (now >= start + daysAfter * 1 days) { ... }
}
{% endhighlight %}

## Special Variables and Functions

There are special variables and functions which always exist in the global
namespace and are mainly used to provide information about the blockchain.

### Block and Transaction Properties

 - `block.coinbase` (`address`): current block miner's address
 - `block.difficulty` (`uint`): current block difficulty
 - `block.gaslimit` (`uint`): current block gaslimit
 - `block.number` (`uint`): current block number
 - `block.blockhash` (`function(uint) returns (bytes32)`): hash of the given block - only for 256 most recent blocks
 - `block.timestamp` (`uint`): current block timestamp
 - `msg.data` (`bytes`): complete calldata
 - `msg.gas` (`uint`): remaining gas
 - `msg.sender` (`address`): sender of the message (current call)
 - `msg.sig` (`bytes4`): first four bytes of the calldata (i.e. function identifier)
 - `msg.value` (`uint`): number of wei sent with the message
 - `now` (`uint`): current block timestamp (alias for `block.timestamp`)
 - `tx.gasprice` (`uint`): gas price of the transaction
 - `tx.origin` (`address`): sender of the transaction (full call chain)

<div class="note info">
The values of all members of <code>msg</code>, including <code>msg.sender</code> and
<code>msg.value</code> can change
for every **external** function call. This includes calls to library functions.
<br/>
If you want to implement access restrictions in library functions using
<code>msg.sender</code>, you have to manually supply the value of
<code>msg.sender</code> as an argument.
</div>

<div class="note info">
The block hashes are not available for all blocks for scalability reasons.
You can only access the hashes of the most recent 256 blocks, all other
values will be zero.
</div>


### Mathematical and Cryptographic Functions

 - `addmod(uint x, uint y, uint k) returns (uint)`: compute `(x + y) % k` where the addition is performed with arbitrary precision and does not wrap around at `2**256`.
 - `mulmod(uint x, uint y, uint k) returns (uint)`: compute `(x * y) % k` where the multiplication is performed with arbitrary precision and does not wrap around at `2**256`.
 - `sha3(...) returns (bytes32)`: compute the Ethereum-SHA-3 hash of the (tightly packed) arguments
 - `sha256(...) returns (bytes32)`: compute the SHA-256 hash of the (tightly packed) arguments
 - `ripemd160(...) returns (bytes20)`: compute RIPEMD-160 hash of the (tightly packed) arguments
 - `ecrecover(bytes32, byte, bytes32, bytes32) returns (address)`: recover public key from elliptic curve signature - arguments are (data, v, r, s)

In the above, "tightly packed" means that the arguments are concatenated without padding.
This means that the following are all identical:

```sha3("ab", "c")
sha3("abc")
sha3(0x616263)
sha3(6382179)
sha3(97, 98, 99)
```

If padding is needed, explicit type conversions can be used: `sha3("\x00\x12")` is the
same as `sha3(uint16(0x12))`.

It might be that you run into Out-of-Gas for `sha256`, `ripemd160` or `ecrecover` on a *private blockchain*. The reason for this is that those are implemented as so-called precompiled contracts and these contracts only really exist after they received the first message (although their contract code is hardcoded). Messages to non-existing contracts are more expensive and thus the execution runs into an Out-of-Gas error. A workaround for this problem is to first send e.g. 1 Wei to each of the contracts before you use them in your actual contracts. This is not an issue on the official or test net.

### Contract Related

 - `this` (current contract's type): the current contract, explicitly convertible to `address`
 - `selfdestruct(address)`: destroy the current contract, sending its funds to the given address

Furthermore, all functions of the current contract are callable directly including the current function.

