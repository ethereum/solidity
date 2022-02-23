.. index:: ! contract

.. _contracts:

##########
Contrats
##########

Les contrats en Solidity sont similaires à des classes dans les langages orientés objets. Ils contiennent des données persistentes dans des variables et des fonctions peuvent les modifier. Appeler la fonction d'un autre contrat (une autre instance) executera un appel de fonction auprès de l'EVM et changera alors le contexte, rendant inaccessibles ces variables.
A contract and its functions need to be called for anything to happen.
There is no "cron" concept in Ethereum to call a function at a particular event automatically.

.. include:: contracts/creating-contracts.rst

.. include:: contracts/visibility-and-getters.rst

.. include:: contracts/function-modifiers.rst

.. include:: contracts/constant-state-variables.rst
.. include:: contracts/functions.rst

.. include:: contracts/events.rst
.. include:: contracts/errors.rst

.. include:: contracts/inheritance.rst

.. include:: contracts/abstract-contracts.rst
.. include:: contracts/interfaces.rst

.. include:: contracts/libraries.rst

.. include:: contracts/using-for.rst