.. index:: ! visibility, external, public, private, internal

.. |visibility-caveat| replace:: Making something ``private`` or ``internal`` only prevents other contracts from reading or modifying the information, but it will still be visible to the whole world outside of the blockchain.

.. _visibility-and-getters:

**********************
Visibilité et Getters
**********************


State Variable Visibility
=========================

``public``
    Public state variables differ from internal ones only in that the compiler automatically generates
    :ref:`getter functions<getter-functions>` for them, which allows other contracts to read their values.
    When used within the same contract, the external access (e.g. ``this.x``) invokes the getter
    while internal access (e.g. ``x``) gets the variable value directly from storage.
    Setter functions are not generated so other contracts cannot directly modify their values.

``internal``
    Internal state variables can only be accessed from within the contract they are defined in
    and in derived contracts.
    They cannot be accessed externally.
    This is the default visibility level for state variables.

``private``
    Private state variables are like internal ones but they are not visible in derived contracts.

.. warning::
    |visibility-caveat|

Function Visibility
===================

Puisque Solidity connaît deux types d'appels de fonction (internes qui ne créent pas d'appel EVM réel (également appelés
a "message call") et externes qui le font), il existe quatre types de visibilités pour les fonctions et les variables d'état.

Les fonctions doivent être spécifiées comme étant ``external``, ``public``, ``internal`` ou ``private``.
Pour les variables d'état, ``external`` n'est pas possible.

``external``:
    Les fonctions externes font partie de l'interface du contrat, ce qui signifie qu'elles peuvent être appelées à partir d'autres contrats et via des transactions. Une fonction externe ``f`` ne peut pas être appelée en interne (c'est-à-dire ``f()``ne fonctionne pas, mais ``this.f()`` fonctionne).
    Les fonctions externes sont parfois plus efficaces lorsqu'elles reçoivent de grandes quantités de données.

``public``:
    Les fonctions publiques font partie de l'interface du contrat et peuvent être appelées en interne ou via des messages. Pour les variables d'état publiques, une fonction getter automatique (voir ci-dessous) est générée.

``internal``:
    Ces fonctions et variables d'état ne sont accessibles qu'en interne (c'est-à-dire à partir du contrat en cours ou des contrats qui en découlent), sans utiliser ``this``.

``private``:
    Les fonctions privées et les variables d'état ne sont visibles que pour le contrat dans lequel elles sont définies et non dans les contrats dérivés.

.. note::
     Tout ce qui se trouve à l'intérieur d'un contrat est visible pour tous les observateurs extérieurs à la blockchain. Passer quelque chose en ``private``
    ne fait qu'empêcher les autres contrats d'accéder à l'information et de la modifier, mais elle sera toujours visible pour le monde entier à l'extérieur de la blockchain.

Le spécificateur de visibilité est donné après le type pour les variables d'état et entre la liste des paramètres et la liste des paramètres de retour pour les fonctions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint a) private pure returns (uint b) { return a + 1; }
        function setData(uint a) internal { data = a; }
        uint public data;
    }

Dans l'exemple suivant, ``D``, peut appeler ``c.getData()`` pour retrouver la valeur de ``data`` en mémoire d'état, mais ne peut pas appeler ``f``. Le contrat ``E`` est dérivé du contrat ``C`` et peut donc appeler ``compute``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint private data;

        function f(uint a) private pure returns(uint b) { return a + 1; }
        function setData(uint a) public { data = a; }
        function getData() public view returns(uint) { return data; }
        function compute(uint a, uint b) internal pure returns (uint) { return a + b; }
    }

    // Ceci ne compile pas
    contract D {
        function readData() public {
            C c = new C();
            uint local = c.f(7); // Erreur: le membre `f` n'est pas visible
            c.setData(3);
            local = c.getData();
            local = c.compute(3, 5); // Erreur: le membre `compute` n'est pas visible
        }
    }

    contract E is C {
        function g() public {
            C c = new C();
            uint val = compute(3, 5); // accès à un membre interne (du contrat dérivé au contrat parent)
        }
    }

.. index:: ! getter;function, ! function;getter
.. _getter-functions:

Fonctions Getter
================

Le compilateur crée automatiquement des fonctions getter pour toutes les variables d'état **public**. Pour le contrat donné ci-dessous, le compilateur va générer une fonction appelée ``data`` qui ne prend aucun argument et retourne un ``uint``, la valeur de la variable d'état ``data``. Les variables d'état peuvent être initialisées lorsqu'elles sont déclarées.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        uint public data = 42;
    }

    contract Caller {
        C c = new C();
        function f() public view returns (uint) {
            return c.data();
        }
    }

Les fonctions getter ont une visibilité externe. Si le symbole est accédé en interne (c'est-à-dire sans ``this.``), il est évalué à une variable d'état.  S'il est accédé de l'extérieur (c'est-à-dire avec ``this.``), il évalue à une fonction.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.7.0;

    contract C {
        uint public data;
        function x() public returns (uint) {
            data = 3; // accès interne
            return this.data(); // accès externe
        }
    }

Si vous avez une variable d'état ``public`` de type array, alors vous ne pouvez récupérer que des éléments simples de l'array via la fonction getter générée.
Ce mécanisme permet d'éviter des coûts de gas élevés lors du retour d'un tableau complet.
Vous pouvez utiliser des arguments pour spécifier quel élément individuel retourner, par exemple ``data(0)``. Si vous voulez retourner un tableau entier en un appel, alors vous devez écrire une fonction, par exemple :

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

  contract arrayExample {
    // variable d'état publique
    uint[] public myArray;

    // Fonction getter générée par le compilateur
    /*
    function myArray(uint i) returns (uint) {
        return myArray[i];
    }
    */

    // fonction retournant une array complète
    function getArray() returns (uint[] memory) {
        return myArray;
    }
  }

Maintenant vous pouvez utiliser ``getArray()`` pour récupérer le tableau entier, au lieu de ``myArray(i)``, qui retourne un seul élément par appel.

L'exemple suivant est plus complexe:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract Complex {
        struct Data {
            uint a;
            bytes3 b;
            mapping (uint => uint) map;
            uint[3] c;
            uint[] d;
            bytes e;
        }
        mapping (uint => mapping(bool => Data[])) public data;
    }

It generates a function of the following form. The mapping and arrays (with the
exception of byte arrays) in the struct are omitted because there is no good way
to select individual struct members or provide a key for the mapping:

.. code-block:: solidity

    function data(uint arg1, bool arg2, uint arg3)
        public
        returns (uint a, bytes3 b, bytes memory e)
    {
        a = data[arg1][arg2][arg3].a;
        b = data[arg1][arg2][arg3].b;
        e = data[arg1][arg2][arg3].e;
    }
