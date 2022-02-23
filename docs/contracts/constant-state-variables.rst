.. index:: ! constant

<<<<<<< HEAD
************************
Variables d'état constantes
************************
=======
.. _constants:

**************************************
Constant and Immutable State Variables
**************************************
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

Les variables d'état peuvent être déclarées comme ``constantes`` ou ``immutable``. Dans les deux cas, ces variables ne peuvent être modifiées après la construction du contrat.
Dans ce cas, elles doivent être assignées à partir d'une expression constante au moment de la compilation.
Pour les variables ``constant``, la valeur doit être connue à la compilation.
Pour les variables ``immutable``, les variables peuvent être assognées jusqu'à la construction.

<<<<<<< HEAD
Le compilateur ne réserve pas d'emplacement de stockage pour ces variables, et chaque occurrence est remplacée par l'expression constante correspondante.

Tous les types de constantes ne sont pas implémentés pour le moment. Les seuls types pris en charge sont
`chaines de caractères <strings>`_ (uniquement pour les constantes) et `types valeur <value-types>`_.
=======
It is also possible to define ``constant`` variables at the file level.

The compiler does not reserve a storage slot for these variables, and every occurrence is
replaced by the respective value.

Compared to regular state variables, the gas costs of constant and immutable variables
are much lower. For a constant variable, the expression assigned to it is copied to
all the places where it is accessed and also re-evaluated each time. This allows for local
optimizations. Immutable variables are evaluated once at construction time and their value
is copied to all the places in the code where they are accessed. For these values,
32 bytes are reserved, even if they would fit in fewer bytes. Due to this, constant values
can sometimes be cheaper than immutable values.

Not all types for constants and immutables are implemented at this time. The only supported types are
:ref:`strings <strings>` (only for constants) and :ref:`value types <value-types>`.
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.4;

    uint constant X = 32**22 + 8;

    contract C {
        string constant TEXT = "abc";
        bytes32 constant MY_HASH = keccak256("abc");
        uint immutable decimals;
        uint immutable maxBalance;
        address immutable owner = msg.sender;

        constructor(uint _decimals, address _reference) {
            decimals = _decimals;
            // L'assignement à des immutables peut même accéder à l'environnement
            maxBalance = _reference.balance;
        }

        function isBalanceTooHigh(address _other) public view returns (bool) {
            return _other.balance > maxBalance;
        }
    }


Constant
========

<<<<<<< HEAD
Pour les variables ``constant``, doivent être assignées à partir d'une expression constante au moment de la compilation et doit 6etre assignée à la déclaration. Toute expression qui accède au stockage, aux données de la blockchain (par exemple ``now``, ``address(this).balance`` ou ``block.number``) ou
les données d'exécution (``msg.value`` ou ``gasleft()``) ou les appels vers des contrats externes sont interdits. Les expressions qui peuvent avoir un effet secondaire sur l'allocation de mémoire sont autorisées, mais celles qui peuvent avoir un effet secondaire sur d'autres objets mémoire ne le sont pas. Les fonctions intégrées ``keccak256``, ``sha256``, ``ripemd160``, ``ecrecover``, ``addmod`` et ``mulmod`` sont autorisées (même si des contrats externes sont appelés).

La raison pour laquelle on autorise les effets secondaires sur l'allocateur de mémoire est qu'il devrait être possible de construire des objets complexes comme par exemple des tables de consultation.
Cette fonctionnalité n'est pas encore entièrement utilisable.
=======
For ``constant`` variables, the value has to be a constant at compile time and it has to be
assigned where the variable is declared. Any expression
that accesses storage, blockchain data (e.g. ``block.timestamp``, ``address(this).balance`` or
``block.number``) or
execution data (``msg.value`` or ``gasleft()``) or makes calls to external contracts is disallowed. Expressions
that might have a side-effect on memory allocation are allowed, but those that
might have a side-effect on other memory objects are not. The built-in functions
``keccak256``, ``sha256``, ``ripemd160``, ``ecrecover``, ``addmod`` and ``mulmod``
are allowed (even though, with the exception of ``keccak256``, they do call external contracts).

The reason behind allowing side-effects on the memory allocator is that it
should be possible to construct complex objects like e.g. lookup-tables.
This feature is not yet fully usable.
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

Immutable
=========

Variables declared as ``immutable`` are a bit less restricted than those
declared as ``constant``: Immutable variables can be assigned an arbitrary
value in the constructor of the contract or at the point of their declaration.
They can be assigned only once and can, from that point on, be read even during
construction time.

The contract creation code generated by the compiler will modify the
contract's runtime code before it is returned by replacing all references
to immutables by the values assigned to the them. This is important if
you are comparing the
runtime code generated by the compiler with the one actually stored in the
blockchain.

.. note::
  Immutables that are assigned at their declaration are only considered
  initialized once the constructor of the contract is executing.
  This means you cannot initialize immutables inline with a value
  that depends on another immutable. You can do this, however,
  inside the constructor of the contract.

  This is a safeguard against different interpretations about the order
  of state variable initialization and constructor execution, especially
  with regards to inheritance.