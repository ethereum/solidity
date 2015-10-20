---
layout: docs
title: Structure of a Contract
permalink: /docs/structure-of-a-contract/
---

In Solidity, contracts are what classes are in object-oriented languages.
They can contain **state variables**, **functions**, and **events**.

* State variables are values which are permanently stored in contract storage.
* Functions are are the executable units of code within a contract.
* Events are convenience interfaces with the EVM logging facilities.

Functions can also declare local variables which are defined inside of a
function and whos content is cleared as soon as control flow returns from the
function.
