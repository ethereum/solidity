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

By this mechanism, bitcoin ensures that (in simplified words) the act of subtracting an
amount from one account and adding it to another does not simply stop after the subtracting phase.

Furthermore, a transaction is always cryptographically signed by the sender (creator).
This makes it impossible for anyone except the person holding the keys to an account
(in the case of bitcoin) to spend the money in that account.

### Blocks

One major obstacle to overcome is what in bitcoin terms is called "double-spend attack":
What happens if two transactions exist in the network that both want to empty an account,
a so-called conflict?

The abstract answer to this is that you do not have to care. An order of the transactions
will be selected for you, the transactions will be bundled into what is called a "block"
and then they will be executed and distributed among all participating nodes.
The transaction that ends up being second will be either completely rejected or will
just have no effect.

These blocks form a linear sequence in time and that is where the word "blockchain"
derives from. Blocks are added to the chain in rather regular intervals - for
Ethereum this is roughly every 17 seconds.

As part of the "order selection mechanism" (which is called "mining") it may happen that
blocks are reverted from time to time, but only at the "tip" of the chain. The more
blocks are reverted the less likely it is. So it might be that your transactions
are reverted and even removed from the blockchain, but the longer you wait, the less
likely it will be.
