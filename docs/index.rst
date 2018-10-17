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
* `French <http://solidity-fr.readthedocs.io>`_ (in progress)

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
   resources.rst
   using-the-compiler.rst
   metadata.rst
   abi-spec.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
   frequently-asked-questions.rst
