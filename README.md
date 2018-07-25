# The Solidity Contract-Oriented Programming Language
[![Join the chat at https://gitter.im/ethereum/solidity](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ethereum/solidity?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Build Status](https://travis-ci.org/ethereum/solidity.svg?branch=develop)](https://travis-ci.org/ethereum/solidity)
Solidity is a statically typed, contract-oriented & high-level language for implementing smart contracts on Ethereum Platform.

## Table of Contents

- [Background](#background)
- [Build & install](#build)
- [Example](#example)
- [Documentaion](#documentation)
- [Developement](#development)
- [Maintainers](#maintainers)
- [License](#license)

## Background
Solidity is a statically-typed programming language which was initially proposed in August 2014 by [Gavin Wood](https://en.wikipedia.org/wiki/Gavin_Wood) and later developed by the Ethereum project's Solidity team, led by Christian Reitwiessner. It is designed for developing smart contracts that run on the EVM. With Solidity, developers are able to write applications that implement self-enforcing business logic embodied in smart contracts, leaving a non-repudiable and authoritative record of transactions. Writing smart contracts in Solidity is considered as an easy (ostensibly for those who already have programming skills). It is designed around the ECMAScript syntax to make it familiar for existing web developers.

Solidity contained a number of important differences compare to other EVM-targeting languages like:

1. Complex member variables for contracts including arbitrarily hierarchical        mappings and structs were supported. Contracts support inheritance,              including multiple inheritance with C3 linearization.
2. An application binary interface (ABI) facilitating multiple type-safe functions within a single contract was also introduced (and later supported by Serpent).
3. A documentation system for specifying a user-centric description of the ramifications of a method-call was also included in the proposal, known as "Natural Language Specification".

## Build & install
Quick instructions to build and install Solidity compiler can be found in [Solidity documentation](https://solidity.readthedocs.io/en/latest/installing-solidity.html#building-from-source)


## Example
Soidity code can be run in [Solidity Browser](https://remix.ethereum.org/) also. List of Solidity contract example can be found below:

1. [Voting](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting)       
2. [Blind Auction](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#blind-auction)
3. [Safe remote purchase](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#safe-remote-purchase)
4. [Micropayment Channel](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#micropayment-channel)

## Documentaion
The Solidity documentation can be found at [Solidity docs](https://solidity.readthedocs.io/en/v0.4.24/introduction-to-smart-contracts.html)

## Development
 Solidity is still in development phase. The changelog for this project can be found [here](https://github.com/ethereum/solidity/blob/develop/Changelog.md). Any contributions to Solidity in terms of bug-fixes and addition of new features can be made by following steps outlined at [Developers Guide](https://solidity.readthedocs.io/en/latest/contributing.html).

## Maintainers
[@chriseth](chriseth)
[@axic](https://github.com/axic)

## License
Solidity is licensed under [GNU General Public License v3.0](https://github.com/ethereum/solidity/blob/develop/LICENSE.txt)

