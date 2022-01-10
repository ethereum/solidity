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

Quand vous déployez des contrats, vous devriez utiliser la dernière
version stable de Solidity. Ceci est dû au fait des changements brisés
comme des nouvelles fonctionnalités et la fixation de bugs, qui sont régulièrement introduits.
Nous utilisons actuellement une version de numéro 0.x `pour indiquer ce rapide rythme de changements <https://semver.org/#spec-item-4>`_.

.. warning::

  Solidity a récemment publié la version 0.6.x qui introduit beaucoup de changements
  de rupture. Soyez sûr que vous lisez :doc:`la liste entière <060-breaking-changes>`.

Documentation du langage
------------------------

Si vous êtes nouveau dans le concept des smart contracts, nous vous recommandons de commencer avec
:ref:`un exemple de smart contract <simple-smart-contract>` écrit
sous Solidity. Quand vous êtes prêt pour plus de détails, nous vous recommandons de lire
:doc:`"Solidity par Exemple" <solidity-by-example>` et
"Langage Description" sections pour apprendre les concepts de base du langage.

Pour aller plus loin dans la lecture, essayez :ref:`les bases des blockchains <blockchain-basics>`
et les détails de la :ref:`Ethereum Virtual Machine <the-ethereum-virtual-machine>`.

.. hint::
  Rappelez-vous que vous pouvez toujours essayer les contrats `dans votre navigateur <https://remix.ethereum.org>`_ ! Remix est un navigateur Web basé comme un IDE
  qui vous autorise d'écrire des smart contracts en Solidity, pour ensuite déployer et exécuter les
  smart contracts. Cela peu prendre du temps à charger, soyez patient.

.. warning::
    Comme ce sont des humains qui écrivent les logiciels, il peut y avoir des bugs. Vous devriez suivre les
    meilleures pratiques de développement logiciel quand vous écrivez vos smart contracts, cela
    inclut la relecture du code, les tests, les vérifications, et les preuves d'exactitude. Les utilisateurs
    de smart contracts sont parfois plus confiants avec le code que leurs auteurs, puis les
    blockchains et les smart contracts ont leurs propres problèmes uniques à
    surveiller. Donc, avant de travailler avec un code en production, soyez sûr d'avoir lu la section
    :ref:`security_considerations`.

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
