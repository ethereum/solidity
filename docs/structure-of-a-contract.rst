.. index:: contract, state variable, function, event, struct, enum, function;modifier

.. _contract_structure:

**********************
Structure d'un contrat
**********************

Les contrats Solidity sont similaires à des classes dans des langages orientés objet.
Chaque contrat peut contenir des déclarations de :ref:`structure-state-variables`, :ref:`structure-functions`,
:ref:`structure-function-modifiers`, :ref:`structure-events`, :ref:`structure-errors`, :ref:`structure-struct-types` et :ref:`structure-enum-types`.
De plus, les contrats peuvent hériter d'autres contrats.

Il existe également des types de contrats spéciaux appelés :ref:`libraries<libraries>` et :ref:`interfaces<interfaces>`.

La section sur les :ref:`contrats<contracts<contracts>` contient plus de détails que cette section, qui permet d'avoir une vue d'ensemble rapide.

.. _structure-state-variables :

Variables d'état
================

Les variables d'état sont des variables dont les valeurs sont stockées en permanence dans le storage du contrat.
.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract SimpleStorage {
        uint storedData; // State variable
        // ...
    }

Voir la section :ref:`types` pour les types de variables d'état valides et :ref:`visibility-and-getters` pour les choix possibles de visibilité.

.. _structure-functions:

Fonctions
=========

Les fonctions sont les unités exécutables du code d'un contrat.  Functions are usually
defined inside a contract, but they can also be defined outside of
contracts.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    contract SimpleAuction {
        function bid() public payable { // Function
            // ...
        }
    }

    // Helper function defined outside of a contract
    function helper(uint x) pure returns (uint) {
        return x * 2;
    }

Les :ref:`function-calls` peuvent se faire en interne ou en externe
et ont différents niveaux de :ref:`visibilité<visibility-and-getters>`
pour d'autres contrats. :ref:`Functions<functions>` accept :ref:`parameters and return variables<function-parameters-return-variables>` to pass parameters
and values between them.

.. _structure-function-modifiers:

Modificateurs de fonction
=========================

Les modificateurs de fonction peuvent être utilisés pour modifier la sémantique des fonctions d'une manière déclarative (voir :ref:`modifiers` dans la section contrats).

Overloading, that is, having the same modifier name with different parameters,
is not possible.

Like functions, modifiers can be :ref:`overridden <modifier-overriding>`.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract Purchase {
        address public seller;

        modifier onlySeller() { // Modifier
            require(
                msg.sender == seller,
                "Only seller can call this."
            );
            _;
        }

        function abort() public view onlySeller { // Modifier usage
            // ...
        }
    }

.. _structure-events:

Évènements
==========

Les évènements (``event``) sont une interface d'accès aux fonctionnalités de journalisation (logs) de l'EVM.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.21 <0.9.0;

    contract SimpleAuction {
        event HighestBidIncreased(address bidder, uint amount); // Event

        function bid() public payable {
            // ...
            emit HighestBidIncreased(msg.sender, msg.value); // Triggering event
        }
    }

Voir :ref:`events` dans la section contrats pour plus d'informations sur la façon dont les événements sont déclarés et peuvent être utilisés à partir d'une dapp.

.. _structure-errors:

Errors
======

Errors allow you to define descriptive names and data for failure situations.
Errors can be used in :ref:`revert statements <revert-statement>`.
In comparison to string descriptions, errors are much cheaper and allow you
to encode additional data. You can use NatSpec to describe the error to
the user.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    /// Not enough funds for transfer. Requested `requested`,
    /// but only `available` available.
    error NotEnoughFunds(uint requested, uint available);

    contract Token {
        mapping(address => uint) balances;
        function transfer(address to, uint amount) public {
            uint balance = balances[msg.sender];
            if (balance < amount)
                revert NotEnoughFunds(amount, balance);
            balances[msg.sender] -= amount;
            balances[to] += amount;
            // ...
        }
    }

See :ref:`errors` in the contracts section for more information.

.. _structure-struct-types:

Types Structure
===============

Les structures sont des types personnalisés qui peuvent regrouper plusieurs variables (voir
:ref:`structs` dans la section types).

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Ballot {
        struct Voter { // Struct
            uint weight;
            bool voted;
            address delegate;
            uint vote;
        }
    }

.. _structure-enum-types:

Types Enum
==========

Les Enumérateurs (``enum``) peuvent être utilisés pour créer des types personnalisés avec un ensemble fini de 'valeurs constantes' (voir :ref:`enums` dans la section Types).

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Purchase {
        enum State { Created, Locked, Inactive } // Enum
    }
