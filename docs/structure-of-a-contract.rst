.. index:: contract, state variable, function, event, struct, enum, function;modifier

***********************
Structure of a Contract
***********************

Contracts in Solidity are similar to classes in object-oriented languages.
Each contract can contain declarations of **state variables**, **functions**,
**function modifiers**, **events**, **structs types** and **enum types**.
Furthermore, contracts can inherit from other contracts.

* State variables are values which are permanently stored in contract storage.
* Functions are the executable units of code within a contract.
* Function modifiers can be used to amend the semantics of functions in a declarative way.
* Events are convenience interfaces with the EVM logging facilities.
* Structs are custom defined types that can group several variables.
* Enums can be used to create custom types with a finite set of values.
