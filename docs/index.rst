Solidity
========

.. image:: logo.svg
    :width: 120px
    :alt: Solidity logo
    :align: center

Solidity is a contract-oriented, high-level language for implementing smart contracts.
It was influenced by C++, Python and JavaScript
and is designed to target the Ethereum Virtual Machine (EVM).

Solidity is statically typed, supports inheritance, libraries and complex
user-defined types among other features.

As you will see, it is possible to create contracts for voting,
crowdfunding, blind auctions, multi-signature wallets and more.

.. note::
    The best way to try out Solidity right now is using
    `Remix <https://remix.ethereum.org/>`_
    (it can take a while to load, please be patient). Remix is a web browser
    based IDE that allows you to write Solidity smart contracts, then deploy
    and run the smart contracts.

.. warning::
    Since software is written by humans, it can have bugs. Thus, also
    smart contracts should be created following well-known best-practices in
    software development. This includes code review, testing, audits and correctness proofs.
    Also note that users are sometimes more confident in code than its authors.
    Finally, blockchains have their own things to watch out for, so please take
    a look at the section :ref:`security_considerations`.

Translations
------------

This documentation is translated into several languages by community volunteers
with varying degrees of completeness and up-to-dateness. The English version stands as a reference.

* `Simplified Chinese <http://solidity-cn.readthedocs.io>`_ (in progress)
* `Spanish <https://solidity-es.readthedocs.io>`_
* `Russian <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (rather outdated)
* `Korean <http://solidity-kr.readthedocs.io>`_ (in progress)


Useful links
------------

General
~~~~~~~

* `Ethereum <https://ethereum.org>`_

* `Changelog <https://github.com/ethereum/solidity/blob/develop/Changelog.md>`_

* `Source Code <https://github.com/ethereum/solidity/>`_

* `Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_

* `Language Users Chat <https://gitter.im/ethereum/solidity/>`_

* `Compiler Developers Chat <https://gitter.im/ethereum/solidity-dev/>`_

Available Solidity Integrations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Generic:

    * `Remix <https://remix.ethereum.org/>`_
        Browser-based IDE with integrated compiler and Solidity runtime environment without server-side components.

    * `Solium <https://github.com/duaraghav8/Solium/>`_
        Linter to identify and fix style and security issues in Solidity.

    * `Solhint <https://github.com/protofire/solhint>`_
        Solidity linter that provides security, style guide and best practice rules for smart contract validation.

* Atom:

    * `Etheratom <https://github.com/0mkara/etheratom>`_
        Plugin for the Atom editor that features syntax highlighting, compilation and a runtime environment (Backend node & VM compatible).

    * `Atom Solidity Linter <https://atom.io/packages/linter-solidity>`_
        Plugin for the Atom editor that provides Solidity linting.

    * `Atom Solium Linter <https://atom.io/packages/linter-solium>`_
        Configurable Solidty linter for Atom using Solium as a base.

* Eclipse:

   * `YAKINDU Solidity Tools <https://yakindu.github.io/solidity-ide/>`_
        Eclipse based IDE. Features context sensitive code completion and help, code navigation, syntax coloring, built in compiler, quick fixes and templates.

* Emacs:

    * `Emacs Solidity <https://github.com/ethereum/emacs-solidity/>`_
        Plugin for the Emacs editor providing syntax highlighting and compilation error reporting.

* IntelliJ:

    * `IntelliJ IDEA plugin <https://plugins.jetbrains.com/plugin/9475-intellij-solidity>`_
        Solidity plugin for IntelliJ IDEA (and all other JetBrains IDEs)

* Sublime:

    * `Package for SublimeText — Solidity language syntax <https://packagecontrol.io/packages/Ethereum/>`_
        Solidity syntax highlighting for SublimeText editor.

* Vim:

    * `Vim Solidity <https://github.com/tomlion/vim-solidity/>`_
        Plugin for the Vim editor providing syntax highlighting.

    * `Vim Syntastic <https://github.com/scrooloose/syntastic>`_
        Plugin for the Vim editor providing compile checking.

* Visual Studio Code:

    * `Visual Studio Code extension <http://juan.blanco.ws/solidity-contracts-in-visual-studio-code/>`_
        Solidity plugin for Microsoft Visual Studio Code that includes syntax highlighting and the Solidity compiler.

Discontinued:

* `Mix IDE <https://github.com/ethereum/mix/>`_
    Qt based IDE for designing, debugging and testing solidity smart contracts.

* `Ethereum Studio <https://live.ether.camp/>`_
    Specialized web IDE that also provides shell access to a complete Ethereum environment.

* `Visual Studio Extension <https://visualstudiogallery.msdn.microsoft.com/96221853-33c4-4531-bdd5-d2ea5acc4799/>`_
    Solidity plugin for Microsoft Visual Studio that includes the Solidity compiler.

Solidity Tools
~~~~~~~~~~~~~~

* `Dapp <https://dapp.tools/dapp/>`_
    Build tool, package manager, and deployment assistant for Solidity.

* `Solidity REPL <https://github.com/raineorshine/solidity-repl>`_
    Try Solidity instantly with a command-line Solidity console.

* `solgraph <https://github.com/raineorshine/solgraph>`_
    Visualize Solidity control flow and highlight potential security vulnerabilities.

* `Doxity <https://github.com/DigixGlobal/doxity>`_
    Documentation Generator for Solidity.

* `evmdis <https://github.com/Arachnid/evmdis>`_
    EVM Disassembler that performs static analysis on the bytecode to provide a higher level of abstraction than raw EVM operations.

* `ABI to solidity interface converter <https://gist.github.com/chriseth/8f533d133fa0c15b0d6eaf3ec502c82b>`_
    A script for generating contract interfaces from the ABI of a smart contract.

* `Securify <https://securify.ch/>`_
    Fully automated online static analyzer for smart contracts, providing a security report based on vulnerability patterns.

* `Sūrya <https://github.com/ConsenSys/surya/>`_
    Utility tool for smart contract systems, offering a number of visual outputs and information about the contracts' structure. Also supports querying the function call graph.

* `EVM Lab <https://github.com/ethereum/evmlab/>`_
    Rich tool package to interact with the EVM. Includes a VM, Etherchain API, and a trace-viewer with gas cost display.

.. note::
  Information like variable names, comments, and source code formatting is lost in the compilation process and it is not possible to completely recover the original source code. Decompiling smart contracts to view the original source code might not be possible, or the end result that useful.

Third-Party Solidity Parsers and Grammars
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* `solidity-parser <https://github.com/ConsenSys/solidity-parser>`_
    Solidity parser for JavaScript

* `Solidity Grammar for ANTLR 4 <https://github.com/federicobond/solidity-antlr4>`_
    Solidity grammar for the ANTLR 4 parser generator

Language Documentation
----------------------

On the next pages, we will first see a :ref:`simple smart contract <simple-smart-contract>` written
in Solidity followed by the basics about :ref:`blockchains <blockchain-basics>`
and the :ref:`Ethereum Virtual Machine <the-ethereum-virtual-machine>`.

The next section will explain several *features* of Solidity by giving
useful :ref:`example contracts <voting>`.
Remember that you can always try out the contracts
`in your browser <https://remix.ethereum.org>`_!

The fourth and most extensive section will cover all aspects of Solidity in depth.

If you still have questions, you can try searching or asking on the
`Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_
site, or come to our `gitter channel <https://gitter.im/ethereum/solidity/>`_.
Ideas for improving Solidity or this documentation are always welcome!

Contents
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst
   solidity-in-depth.rst
   security-considerations.rst
   using-the-compiler.rst
   metadata.rst
   abi-spec.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   frequently-asked-questions.rst
