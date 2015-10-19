---
layout: docs
title: Layout of a Solidity Source File
permalink: /docs/layout-of-source-files/
---

A Solidity source file can contain an arbitrary number of contracts.
**Other source files** can be referenced using `import "filename";` and the symbols
defined there will also be available in the current source file. Note that
the browser-based compiler does not support multiple files and if you are using
the [commandline compiler](#using-the-commandline-compiler), you have to explicitly specify all files you will use
as arguments, the compiler will not search your filesystem on its own.

**Comments** Single-line comments (`//`) and multi-line comments (`/*...*/`) are possible, while
triple-slash comments (`///`) right in front of function declarations introduce
[NatSpec](Ethereum-Natural-Specification-Format) (which are not covered here).

