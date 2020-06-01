Solidity
========

.. image:: logo.svg
    :width: 120px
    :alt: Solidity logo
    :align: center

Solidity est un langage haut-niveau, orienté objet dédié à l'implémentation de smart contracts. Les smart contracts (littéralement contrats intelligents) sont des programes qui régissent le comportement de comptes dans l'état d'Ethereum.

Solidity a été influencé par C++, Python et JavaScript et est conçu pour cibler
la machine virtuelle Ethereum (EVM).

Solidity est statiquement typé, supporte l'héritage, les librairies et les bibliothèques, ainsi
que les types complexes définis par l'utilisateur parmi d'autres caractéristiques.

Avec Solidity, vous pouvez créer des contrats pour des usages tels que le vote, le crowdfunding, les enchères à l'aveugle,
et portefeuilles multi-signature.

When deploying contracts, you should use the latest released
version of Solidity. This is because breaking changes as well as
new features and bug fixes are introduced regularly. We currently use
a 0.x version number `to indicate this fast pace of change <https://semver.org/#spec-item-4>`_.

.. warning::

  Solidity recently released the 0.6.x version that introduced a lot of breaking
  changes. Make sure you read :doc:`the full list <060-breaking-changes>`.

Documentation du langage
------------------------

If you are new to the concept of smart contracts we recommend you start with
:ref:`an example smart contract <simple-smart-contract>` written
in Solidity. When you are ready for more detail, we recommend you read the
:doc:`"Solidity by Example" <solidity-by-example>` and
"Language Description" sections to learn the core concepts of the language.

For further reading, try :ref:`the basics of blockchains <blockchain-basics>`
and details of the :ref:`Ethereum Virtual Machine <the-ethereum-virtual-machine>`.

.. hint::
  Rappelez-vous que vous pouvez toujours essayer les contrats `dans votre navigateur <https://remix.ethereum.org>`_ ! Remix is a web browser based IDE
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

.. _translations:

Traductions
-----------

Cette documentation est traduite en plusieurs langues par des bénévoles de la communauté avec divers degrés d'exhaustivité et d'actualité. La version anglaise reste la référence.

* `French <http://solidity-fr.readthedocs.io>`_ (en cours)
* `Italian <https://github.com/damianoazzolini/solidity>`_ (en cours)
* `Japanese <https://solidity-jp.readthedocs.io>`_
* `Korean <http://solidity-kr.readthedocs.io>`_ (en cours)
* `Russian <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (plutôt dépassée)
* `Simplified Chinese <http://solidity-cn.readthedocs.io>`_ (en cours)
* `Spanish <https://solidity-es.readthedocs.io>`_
* `Turkish <https://github.com/denizozzgur/Solidity_TR/blob/master/README.md>`_ (partielle)

Sommaire
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2
   :caption: Les bases

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst

.. toctree::
   :maxdepth: 2
   :caption: Description du langage

   layout-of-source-files.rst
   structure-of-a-contract.rst
   types.rst
   units-and-global-variables.rst
   control-structures.rst
   contracts.rst
   assembly.rst
   cheatsheet.rst
   grammar.rst

.. toctree::
   :maxdepth: 2
   :caption: Internals

   internals/layout_in_storage.rst
   internals/layout_in_memory.rst
   internals/layout_in_calldata.rst
   internals/variable_cleanup.rst
   internals/source_mappings.rst
   internals/optimiser.rst
   metadata.rst
   abi-spec.rst

.. toctree::
   :maxdepth: 2
   :caption: Ressources additionelles

   050-breaking-changes.rst
   060-breaking-changes.rst
   natspec-format.rst
   security-considerations.rst
   resources.rst
   using-the-compiler.rst
   yul.rst
   style-guide.rst
   common-patterns.rst
   bugs.rst
   contributing.rst
