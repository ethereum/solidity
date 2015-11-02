---
layout: docs
title: Blockchain Basics
permalink: /docs/blockchain-basics/
---

Blockchains as a concept are not too hard to understand for programmers. The reason is that
most of the complications (mining, hashing, elliptic-curve cryptography, peer-to-peer networks, ...)
are just there to provide a certain set of features and promises. Once you accept these
features as given, you do not have to worry about the underlying technology - or do you have
to know how Amazon's AWS works internally in order to use it?

### Transactions

A blockchain is a globally shared, transactional database.
This means that everyone can read entries in the database just by participating in the network.
If you want to change something in the database, you have to create a so-called transaction
which has to be accepted by all others.
The word transaction implies that the change you want to make (assume you want to change
two values at the same time) is either not done at all or completely applied. Furthermore,
while your transaction is applied to the database, no other transaction can alter it.

As an example, imagine a table that lists the balances of all accounts in an
electronic currency. If a transfer from one account to another is requested,
the transactional nature of the database ensures that if the amount is
subtracted from one account, it is always added to the other account. If due
to whatever reason, adding the amount to the target account is not possible,
the source account is also not modified.

Furthermore, a transaction is always cryptographically signed by the sender (creator).
This makes it straightforward to guard access to specific modifications of the
database. In the example of the electronic currency, a simple check ensures that
only the person holding the keys to the account can transfer money from it.

### Blocks

One major obstacle to overcome is what in bitcoin terms is called "double-spend attack":
What happens if two transactions exist in the network that both want to empty an account,
a so-called conflict?

The abstract answer to this is that you do not have to care. An order of the transactions
will be selected for you, the transactions will be bundled into what is called a "block"
and then they will be executed and distributed among all participating nodes.
If two transactions contradict each other, the one that ends up being second will
be rejected and not become part of the block.

These blocks form a linear sequence in time and that is where the word "blockchain"
derives from. Blocks are added to the chain in rather regular intervals - for
Ethereum this is roughly every 17 seconds.

As part of the "order selection mechanism" (which is called "mining") it may happen that
blocks are reverted from time to time, but only at the "tip" of the chain. The more
blocks are reverted the less likely it is. So it might be that your transactions
are reverted and even removed from the blockchain, but the longer you wait, the less
likely it will be.
