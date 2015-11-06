---
layout: docs
title: Layout of a Solidity Source File
link_title: Source File Layout
permalink: /docs/layout-of-source-files/
---

Source files can contain an arbitrary number of contract definitions and include directives.

### Importing other Source Files

Other source files can be referenced using `import "filename";` and the symbols
defined there will also be available in the current source file.

This system of importing other files is not completely fleshed out yet, so please expect changes.

The [browser-based compiler](https://chriseth.github.io/browser-solidity)
has quite advanced support for multiple files and can even import files
directly from github, by using e.g.
```import "https://github.com/ethereum/dapp-bin/library/iterable_mapping.sol";```

If you want to use multiple source files with the (commandline compiler)[../commandline-compiler/] solc,
you have to specify all files you will use as arguments to solc,
the compiler will not yet search your filesystem on its own.

### Comments

Single-line comments (`//`) and multi-line comments (`/*...*/`) are possible.

There are special types of comments called [natspec](../natspec/) comments
(documentation yet to be written). These are introduced by 
triple-slash comments (`///`) or using double asterists (`/** ... */`).
Right in front of function declarations or statements,
you can use doxygen-style tags inside them to document functions, annotate conditions for formal
verification and provide a **confirmation text** that is shown to users if they want to
invoke a function.
