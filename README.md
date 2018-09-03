# The Solidity Contract-Oriented Programming Language
[![Join the chat at https://gitter.im/ethereum/solidity](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ethereum/solidity?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Build Status](https://travis-ci.org/ethereum/solidity.svg?branch=develop)](https://travis-ci.org/ethereum/solidity)
Solidity is a statically typed, contract-oriented, high-level language for implementing smart contracts on the Ethereum platform.

## Table of Contents

- [Background](#background)
- [Build and Install](#build-and-install)
- [Example](#example)
- [Documentation](#documentation)
- [Development](#development)
- [Maintainers](#maintainers)
- [License](#license)

## Background

Solidity is a statically-typed curly-braces programming language designed for developing smart contracts
that run on the Ethereum Virtual Machine. Smart contracts are programs that are executed inside a peer-to-peer
network where nobody has special authority over the execution and thus they allow to implement tokens of value,
ownership, voting and other kinds of logics.

## Build and Install

Instructions about how to build and install the Solidity compiler can be found in the [Solidity documentation](https://solidity.readthedocs.io/en/latest/installing-solidity.html#building-from-source)


## Example

A "Hello World" program in Solidity is of even less use than in other languages, but still:

```
contract HelloWorld {
  function f() pure returns (string memory) {
    return "Hello, World!";
  }
}
```

To get started with Solidity, you can use [Remix](https://remix.ethereum.org/), which is an
browser-based IDE. Here are some example contracts:

1. [Voting](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting)
2. [Blind Auction](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#blind-auction)
3. [Safe remote purchase](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#safe-remote-purchase)
4. [Micropayment Channel](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#micropayment-channel)

## Documentation

The Solidity documentation is hosted at [Read the docs](https://solidity.readthedocs.io).

## Development

Solidity is still under development. Contributions are always welcome!
Please follow the
[Developers Guide](https://solidity.readthedocs.io/en/latest/contributing.html)
if you want to help.

## Maintainers
[@axic](https://github.com/axic)
[@chriseth](https://github.com/chriseth)

## License
Solidity is licensed under [GNU General Public License v3.0](https://github.com/ethereum/solidity/blob/develop/LICENSE.txt)

