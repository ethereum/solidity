Solidity
========

.. image:: logo.svg
    :width: 120px
    :alt: Solidity logo
    :align: center

Solidity is an object-oriented, high-level language for implementing smart
contracts. Smart contracts are programs which govern the behaviour of accounts
within the Ethereum state.

Solidity was influenced by C++, Python and JavaScript and is designed to target
the Ethereum Virtual Machine (EVM).

Solidity is statically typed, supports inheritance, libraries and complex
user-defined types among other features.

With Solidity you can create contracts for uses such as voting, crowdfunding, blind auctions,
and multi-signature wallets.

When deploying contracts, you should use the latest released version of Solidity. This is because breaking changes as well as new features and bug fixes are introduced regularly. We currently use a 0.x version number `to indicate this fast pace of change <https://semver.org/#spec-item-4>`_.

.. warning::

  Solidity recently released the 0.5.x version that introduced a lot of breaking changes. Make sure you read :doc:`the full list <050-breaking-changes>`.

Language Documentation
----------------------

If you are new to the concept of smart contracts we recommend you start with
:ref:`an example smart contract <simple-smart-contract>` written
in Solidity. When you are ready for more detail, we recommend you read the
:doc:`"Solidity by Example" <solidity-by-example>` and :doc:`"Solidity in Depth" <solidity-in-depth>` sections to learn the core concepts of the language.

For further reading, try :ref:`the basics of blockchains <blockchain-basics>`
and details of the :ref:`Ethereum Virtual Machine <the-ethereum-virtual-machine>`.

.. hint::
  You can always try out code examples in your browser with the
  `Remix IDE <https://remix.ethereum.org>`_. Remix is a web browser based IDE
  that allows you to write Solidity smart contracts, then deploy and run the
  smart contracts. It can take a while to load, so please be patient.

.. warning::
    As humans write software, it can have bugs. You should follow established
    software development best-practices when writing your smart contracts, this
    includes code review, testing, audits, and correctness proofs. Smart contract
    users are sometimes more confident with code than their authors, and
    blockchains and smart contracts have their own unique issues to
    watch out for, so before working on production code, make sure you read the
    :ref:`security_considerations` section.

If you have any questions, you can try searching for answers or asking on the
`Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_, or our `gitter channel <https://gitter.im/ethereum/solidity/>`_.

Ideas for improving Solidity or this documentation are always welcome, read our :doc:`contributors guide <contributing>` for more details.

.. _translations:

Translations
------------

Community volunteers help translate this documentation into several languages.
They have varying degrees of completeness and up-to-dateness. The English
version stands as a reference.

* `French <http://solidity-fr.readthedocs.io>`_ (in progress)
* `Japanese <https://solidity-jp.readthedocs.io>`_
* `Korean <http://solidity-kr.readthedocs.io>`_ (in progress)
* `Russian <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (rather outdated)
* `Simplified Chinese <http://solidity-cn.readthedocs.io>`_ (in progress)
* `Spanish <https://solidity-es.readthedocs.io>`_
* `Turkish <https://github.com/denizozzgur/Solidity_TR/blob/master/README.md>`_ (partial)

Contents
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst
   solidity-in-depth.rst
   natspec-format.rst
   security-considerations.rst
   resources.rst
   using-the-compiler.rst
   metadata.rst
   abi-spec.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   lll.rst
