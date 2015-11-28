---
layout: docs
title: The Ethereum Virtual Machine
link_title: The EVM
permalink: /docs/the-ethereum-virtual-machine/
---

## Overview

The Ethereum Virtual Machine or EVM is the runtime environment
for smart contracts in Ethereum. It is not only sandboxed but
actually completely isolated, which means that code running
inside the EVM has no access to network, filesystem or other processes.
Smart contracts even have limited access to other smart contracts.

## Accounts

There are two kinds of accounts in Ethereum which share the same
address space: **External accounts** that are controlled by
public-private key pairs (i.e. humans) and **contract accounts** which are
controlled by the code stored together with the account.

The address of an external account is determined from
the public key while the address of a contract is
determined at the time the contract is created
(it is derived from the creator address and the number
of transactions sent from that address, the so-called "nonce").

Apart from the fact whether an account stores code or not,
the EVM treats the two types equally, though.

Every account has a persistent key-value store mapping 256 bit words to 256 bit
words called **storage**.

Furthermore, every account has a **balance** in
Ether (in "Wei" to be exact) which can be modified by sending transactions that
include Ether.

## Transactions

A transaction is a message that is sent from one account to another
account (which might be the same or the special zero-account, see below).
It can include binary data (its payload) and Ether.

If the target account contains code, that code is executed and
the payload is provided as input data.

If the target account is the zero-account (the account with the
address `0`), the transaction creates a **new contract**.
As already mentioned, the address of that contract is not
the zero address but an address derived from the sender and
its number of transaction sent (the "nonce"). The payload
of such a contract creation transaction is taken to be
EVM bytecode and executed. The output of this execution is
permanently stored as the code of the contract.
This means that in order to create a contract, you do not
send the actual code of the contract, but in fact code that
returns that code.

## Gas

Upon creation, each transaction is charged with a certain amount of **gas**,
whose purpose is to limit the amount of work that is needed to execute
the transaction and to pay for this execution. While the EVM executes the
transaction, the gas is gradually depleted according to specific rules.

The **gas price** is a value set by the creator of the transaction, who
has to pay `gas_price * gas` up front from the sending account.
If some gas is left after the execution, it is refunded in the same way.

If the gas is used up at any point (i.e. it is negative),
an out-of-gas exception is triggered, which reverts all modifications
made to the state in the current call frame.

## Storage, Memory and the Stack

Each account has a persistent memory area which is called **storage**.
Storage is a key-value store that maps 256 bit words to 256 bit words.
It is not possible to enumerate storage from within a contract
and it is comparatively costly to read and even more so, to modify
storage. A contract can neither read nor write to any storage apart
from its own.

The second memory area is called **memory**, of which a contract obtains
a freshly cleared instance for each message call. Memory can be
addressed at byte level, but read and written to in 32 byte (256 bit)
chunks. Memory is more costly the larger it grows (it scales
quadratically).

The EVM is not a register machine but a stack machine, so all
computations are performed on an area called the **stack**. It has a maximum size of
1024 elements and contains words of 256 bits. Access to the stack is
limited to the top end in the following way:
It is possible to copy one of
the topmost 16 elements to the top of the stack or swap the
topmost element with one of the 16 elements below it.
All other operations take the topmost two (or one, or more, depending on
the operation) elements from the stack and push the result onto the stack.
Of course it is possible to move stack elements to storage or memory,
but it is not possible to just access arbitrary elements deeper in the stack
without first removing the top of the stack.

## Instruction Set

The instruction set of the EVM is kept minimal in order to avoid
incorrect implementations which could cause consensus problems.
All instructions operate on the basic data type, 256 bit words.
The usual arithmetic, bit, logical and comparison operations are present.
Conditional and unconditional jumps are possible. Furthermore,
contracts can access relevant properties of the current block
like its number and timestamp.

## Message Calls

Contracts can call other contracts or send Ether to non-contract
accounts by the means of message calls. Message calls are similar
to transactions, in that they have a source, a target, data payload,
Ether, gas and return data. In fact, every transaction consists of
a top-level message call which in turn can create further message calls.

A contract can decide how much of its remaining **gas** should be sent
with the inner message call and how much it wants to retain.
If an out-of-gas exception happens in the inner call (or any
other exception), this will be signalled by an error value put onto the stack.
In this case, only the gas sent together with the call is used up.
In Solidity, the calling contract causes a manual exception by default in
such situations, so that exceptions "bubble up" the call stack.

As already said, the called contract (which can be the same as the caller)
will receive a freshly cleared instance of memory and has access to the
call payload - which will be provided in a separate area called the **calldata**.
After it finished execution, it can return data which will be stored at
a location in the caller's memory preallocated by the caller.

Calls are **limited** to a depth of 1024, which means that for more complex
operations, loops should be preferred over recursive calls.

## Callcode and Libraries

There exists a special variant of a message call, named **callcode**
which is identical to a message call apart from the fact that
the code at the target address is executed in the context of the calling
contract.

This means that a contract can dynamically load code from a different
address at runtime. Storage, current address and balance still
refer to the calling contract, only the code is taken from the called address.

This makes it possible to implement the "library" feature in Solidity:
Reusable library code that can be applied to a contract's storage in
order to e.g. implement a complex data structure.

## Logs

It is possible to store data in a specially indexed data structure
that maps all they way up to the block level. This feature called **logs**
is used by Solidity in order to implement **events**.
Contracts cannot access log data after it has been created, but they
can be efficiently accessed from outside the blockchain.
Since some part of the log data is stored in bloom filters, it is
possible to search for this data in an efficient and cryptographically
secure way, so network peers that do not download the whole blockchain
("light clients") can still find these logs.

## Create

Contracts can even create other contracts using a special opcode (i.e.
they do not simply call the zero address). The only difference between
these **create calls** and normal message calls is that the payload data is
executed and the result stored as code and the caller / creator
receives the address of the new contract on the stack.

## Selfdestruct

The only possibility that code is removed from the blockchain is
when a contract at that address performs the `SELFDESTRUCT` operation.
The remaining Ether stored at that address is sent to a designated
target and then the storage and code is removed.

Note that even if a contract's code does not contain the `SELFDESTRUCT`
opcode, it can still perform that operation using callcode.
