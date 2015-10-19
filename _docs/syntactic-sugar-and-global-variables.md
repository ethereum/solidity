---
layout: docs
title: Syntactic Sugar and Globally Available Variables
permalink: /docs/syntactic-sugar-and-global-variables/
---
# Syntactic Sugar and Globally Available Variables

## Ether and Time Units

A literal number can take a suffix of `wei`, `finney`, `szabo` or `ether` to convert between the subdenominations of ether, where Ether currency numbers without a postfix are assumed to be "wei", e.g. `2 ether == 2000 finney` evaluates to `true`.

Furthermore, suffixes of `seconds`, `minutes`, `hours`, `days`, `weeks` and `years` can be used to convert between units of time where seconds are the base unit and units are converted naively (i.e. a year is always exactly 365 days, etc.).

## Special Variables and Functions

There are special variables and functions which always exist in the global
namespace and are mainly used to provide information about the blockchain.

### Block and Transaction Properties

 - `block.coinbase` (`address`): current block miner's address
 - `block.difficulty` (`uint`): current block difficulty
 - `block.gaslimit` (`uint`): current block gaslimit
 - `block.number` (`uint`): current block number
 - `block.blockhash` (`function(uint) returns (bytes32)`): hash of the given block
 - `block.timestamp` (`uint`): current block timestamp
 - `msg.data` (`bytes`): complete calldata
 - `msg.gas` (`uint`): remaining gas
 - `msg.sender` (`address`): sender of the message (current call)
 - `msg.sig` (`bytes4`): first four bytes of the calldata (i.e. function identifier)
 - `msg.value` (`uint`): number of wei sent with the message
 - `now` (`uint`): current block timestamp (alias for `block.timestamp`)
 - `tx.gasprice` (`uint`): gas price of the transaction
 - `tx.origin` (`address`): sender of the transaction (full call chain)

### Cryptographic Functions

 - `sha3(...) returns (bytes32)`: compute the Ethereum-SHA-3 hash of the (tightly packed) arguments
 - `sha256(...) returns (bytes32)`: compute the SHA-256 hash of the (tightly packed) arguments
 - `ripemd160(...) returns (bytes20)`: compute RIPEMD-160 hash of the (tightly packed) arguments
 - `ecrecover(bytes32, byte, bytes32, bytes32) returns (address)`: recover public key from elliptic curve signature - arguments are (data, v, r, s)

In the above, "tightly packed" means that the arguments are concatenated without padding, i.e.
`sha3("ab", "c") == sha3("abc") == sha3(0x616263) == sha3(6382179) = sha3(97, 98, 99)`. If padding is needed, explicit type conversions can be used.

It might be that you run into Out-of-Gas for `sha256`, `ripemd160` or `ecrecover` on a *private blockchain*. The reason for this is that those are implemented as so-called precompiled contracts and these contracts only really exist after they received the first message (although their contract code is hardcoded). Messages to non-existing contracts are more expensive and thus the execution runs into an Out-of-Gas error. A workaround for this problem is to first send e.g. 1 Wei to each of the contracts before you use them in your actual contracts. This is not an issue on the official or test net.

### Contract Related

 - `this` (current contract's type): the current contract, explicitly convertible to `address`
 - `suicide(address)`: suicide the current contract, sending its funds to the given address

Furthermore, all functions of the current contract are callable directly including the current function.

