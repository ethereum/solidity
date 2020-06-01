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
=======
.. note::
    La meilleure façon d'essayer Solidity à ce jour est d'utiliser `Remix <https://remix.ethereum.org/>`_ https://remix.ethereum.org/
    (le chargement peut prendre un certain temps, merci d'être patient). Remix est un IDE basé sur un navigateur Web qui vous permet d'écrire des contrats intelligents Solidity, puis de déployer et exécuter les contrats intelligents.

.. warning::
    Puisque le logiciel est écrit par des humains, il peut contenir des bugs. Ainsi, des contrats intelligents devraient également être créés selon les meilleures pratiques bien connues en matière de développement de logiciels. Cela comprend l'examen du code, les essais, les vérifications et les preuves d'exactitude.
    Notez également que les utilisateurs ont parfois plus confiance dans le code que ses auteurs.
    Enfin, les blockchains ont leurs propres choses à surveiller, alors jetez un coup d'oeil à la section :ref:`security_considerations`.


Traductions
-----------

Cette documentation est traduite en plusieurs langues par des bénévoles de la communauté avec divers degrés d'exhaustivité et d'actualité. La version anglaise sert de référence.

* `Anglais <http://solidity.readthedocs.io>`_
* `Chinois simplifié <http://solidity-cn.readthedocs.io>`_ (en cours)
* `Espagnol <https://solidity-es.readthedocs.io>`_
* `Russe <https://github.com/ethereum/wiki/wiki/%5BRussian%5D-%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity>`_ (plutôt périmé)
* `Coréen <http://solidity-kr.readthedocs.io>`_ (en cours)


Liens utiles
------------

Général
~~~~~~~

* `Ethereum <https://ethereum.org>`_

* `Changelog <https://github.com/ethereum/solidity/blob/develop/Changelog.md>`_

* `Code Source <https://github.com/ethereum/solidity/>`_

* `Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_

* `Language Users Chat <https://gitter.im/ethereum/solidity/>`_

* `Compiler Developers Chat <https://gitter.im/ethereum/solidity-dev/>`_

Intégrations de Solidity disponibles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Génériques:

    * `Remix <https://remix.ethereum.org/>`_
        IDE basé sur navigateur avec compilateur intégré et environnement d'exécution Solidity sans composants côté serveur.

    * `Solium <https://github.com/duaraghav8/Solium/>`_
        Linter pour identifier et résoudre les problèmes de style et de sécurité dans Solidity.

    * `Solhint <https://github.com/protofire/solhint>`_
        Solidity linter qui fournit la sécurité, le guide de style et les règles de bonnes pratiques pour la validation intelligente des contrats.

* Atom:

    * `Etheratom <https://github.com/0mkara/etheratom>`_
        Plugin pour l'éditeur Atom qui comprend une coloration syntaxique, une compilation et un environnement d'exécution (Backend node & VM compatible).

    * `Atom Solidity Linter <https://atom.io/packages/linter-solidity>`_
        Plugin pour l'éditeur Atom qui fournit un linter Solidity.

    * `Atom Solium Linter <https://atom.io/packages/linter-solium>`_
        Linter Solidty configurable pour Atom utilisant Solium comme base.

* Eclipse:

   * `YAKINDU Solidity Tools <https://yakindu.github.io/solidity-ide/>`_
        IDE basé sur Eclipse. Caractéristiques : aide et complétion de code contextuelle, navigation dans le code, coloration syntaxique, compilateur intégré, corrections rapides et modèles.

* Emacs:

    * `Emacs Solidity <https://github.com/ethereum/emacs-solidity/>`_
        Plugin pour l'éditeur Emacs fournissant la coloration syntaxique et le reporting des erreurs de compilation.

* IntelliJ:

    * `IntelliJ IDEA plugin <https://plugins.jetbrains.com/plugin/9475-intellij-solidity>`_
        Solidity plugin pour IntelliJ IDEA (et tous les autres IDE JetBrains)

* Sublime:

    * `Package for SublimeText — Solidity language syntax <https://packagecontrol.io/packages/Ethereum/>`_
        Coloration syntaxique pour l'éditeur SublimeText.

* Vim:

    * `Vim Solidity <https://github.com/tomlion/vim-solidity/>`_
        Plugin apportant la coloration syntaxique pour l'éditeur Vim.

    * `Vim Syntastic <https://github.com/scrooloose/syntastic>`_
        Plugin pour l'éditeur Vim fournissant des checks de compilation.

* Visual Studio Code:

    * `Visual Studio Code extension <http://juan.blanco.ws/solidity-contracts-in-visual-studio-code/>`_
        Solidity plugin pour Microsoft Visual Studio Code qui inclus la coloration syntaxique et un compilateur Solidity.

Discontinued:

* `Mix IDE <https://github.com/ethereum/mix/>`_
    Qt IDE pour designer, debugger et tester les smarts contracts Solidity.

* `Ethereum Studio <https://live.ether.camp/>`_
    Web IDE spécialisé qui apporte un environnement Ethereum complet.

* `Visual Studio Extension <https://visualstudiogallery.msdn.microsoft.com/96221853-33c4-4531-bdd5-d2ea5acc4799/>`_
    Solidity plugin pour Microsoft Visual Studio qui inclut le compilateur Solidity.

Outils Solidity
~~~~~~~~~~~~~~~

* `Dapp <https://dapp.tools/dapp/>`_
    Outil de création, gestionnaire de paquets et assistant de déploiement pour Solidity.

* `Solidity REPL <https://github.com/raineorshine/solidity-repl>`_
    Essayez Solidity instantanément avec une console Solidity en ligne de commande.

* `solgraph <https://github.com/raineorshine/solgraph>`_
    Visualisez le flux de contrôle de Solidity et mettez en évidence les vulnérabilités potentielles en matière de sécurité.

* `Doxity <https://github.com/DigixGlobal/doxity>`_
    Générateur de documentation pour Solidity.

* `evmdis <https://github.com/Arachnid/evmdis>`_
    Désassembleur EVM qui effectue une analyse statique sur le bytecode pour fournir un niveau d'abstraction plus élevé que les opérations EVM brutes.

* `ABI to solidity interface converter <https://gist.github.com/chriseth/8f533d133fa0c15b0d6eaf3ec502c82b>`_
    Un script pour générer des interfaces de contrat à partir de l'ABI d'un smart contract.

* `Securify <https://securify.ch/>`_
    Analyseur statique en ligne entièrement automatisé pour les smart contracts, fournissant un rapport de sécurité basé sur les modèles de vulnérabilité.

* `Sūrya <https://github.com/ConsenSys/surya/>`_
    Outil utilitaire pour les systèmes de smart contracts, offrant un certain nombre de résultats visuels et d'informations sur la structure des contrats. Prend également en charge l'interrogation du graphe d'appel de fonction.

* `EVM Lab <https://github.com/ethereum/evmlab/>`_
    Riche ensemble d'outils pour interagir avec l'EVM. Comprend une VM, une API Etherchain et un traceur avec affichage du coût du gaz.

.. note::
  Des informations telles que les noms de variables, les commentaires et le formatage du code source sont perdus dans le processus de compilation et il n'est pas possible de récupérer complètement le code source original. Décompiler les contrats intelligents pour afficher le code source original pourrait ne pas être possible, ou le résultat final pourrait être utile.

Parsers et grammaires de Solidity de tierce parties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* `solidity-parser <https://github.com/ConsenSys/solidity-parser>`_
    Parser Solidity pour JavaScript

* `Solidity Grammar for ANTLR 4 <https://github.com/federicobond/solidity-antlr4>`_
    Vérification de grammaire Solidity pour le générateur de parsers ANTLR 4

Documentation du langage
------------------------
>>>>>>> master

Dans les pages suivantes, nous verrons d'abord un :ref:`smart contract simple <simple-smart-contract>` écrit en Solidity suivi par les bases des :ref:`blockchains <blockchain-basics>` et la :ref:`Machine virtuelle <the-ethereum-virtual-machine>`.

<<<<<<< HEAD
If you are new to the concept of smart contracts we recommend you start with
:ref:`an example smart contract <simple-smart-contract>` written
in Solidity. When you are ready for more detail, we recommend you read the
:doc:`"Solidity by Example" <solidity-by-example>` and
"Language Description" sections to learn the core concepts of the language.

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
`Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_, or
our `gitter channel <https://gitter.im/ethereum/solidity/>`_.

Ideas for improving Solidity or this documentation are always welcome,
read our :doc:`contributors guide <contributing>` for more details.

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
=======
La section suivante expliquera plusieurs *caractéristiques* de Solidity en donnant des exemples de contrats utiles :ref:`contrats d'exemple<voting>`.
Rappelez-vous que vous pouvez toujours essayer les contrats `dans votre navigateur <https://remix.ethereum.org>`_ !

La quatrième et plus vaste section couvrira en profondeur tous les aspects de Solidity.

Si vous avez encore des questions, vous pouvez essayer de chercher ou de poser des questions sur le site Web de
`Ethereum Stackexchange <https://ethereum.stackexchange.com/>`_ ou venez sur notre `gitter channel <https://gitter.im/ethereum/solidity/>`_.
Les idées pour améliorer Solidity ou cette documentation sont toujours les bienvenues !

Traduit de l'anglais par `Kevin Azoulay <https://www.linkedin.com/in/kevin-azoulay/>`_.
>>>>>>> master

Sommaire
========

:ref:`Keyword Index <genindex>`, :ref:`Search Page <search>`

.. toctree::
   :maxdepth: 2
   :caption: Basics

   introduction-to-smart-contracts.rst
   installing-solidity.rst
   solidity-by-example.rst

.. toctree::
   :maxdepth: 2
   :caption: Language Description

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
   :caption: Additional Material

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
