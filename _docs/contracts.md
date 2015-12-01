---
layout: docs
title: Contracts
permalink: /docs/contracts/
---

Contracts in Solidity what classes are in object oriented languages.
They persistent data in state variables and functions that can modify these variables.
Calling a function on a different contract (instance) will perform an EVM
function call and thus switch the context such that state variables are
inaccessible.
