.. index:: ! functions

.. _functions:

*********
Fonctions
*********

Functions can be defined inside and outside of contracts.

Functions outside of a contract, also called "free functions", always have implicit ``internal``
:ref:`visibility<visibility-and-getters>`. Their code is included in all contracts
that call them, similar to internal library functions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.1 <0.9.0;

    function sum(uint[] memory _arr) pure returns (uint s) {
        for (uint i = 0; i < _arr.length; i++)
            s += _arr[i];
    }

    contract ArrayExample {
        bool found;
        function f(uint[] memory _arr) public {
            // This calls the free function internally.
            // The compiler will add its code to the contract.
            uint s = sum(_arr);
            require(s >= 10);
            found = true;
        }
    }

.. note::
    Functions defined outside a contract are still always executed
    in the context of a contract. They still have access to the variable ``this``,
    can call other contracts, send them Ether and destroy the contract that called them,
    among other things. The main difference to functions defined inside a contract
    is that free functions do not have direct access to storage variables and functions
    not in their scope.

.. _function-parameters-return-variables:

Function Parameters and Return Variables
========================================

Functions take typed parameters as input and may, unlike in many other
languages, also return an arbitrary number of values as output.

Function Parameters
-------------------

Function parameters are declared the same way as variables, and the name of
unused parameters can be omitted.

For example, if you want your contract to accept one kind of external call
with two integers, you would use something like the following:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        uint sum;
        function taker(uint _a, uint _b) public {
            sum = _a + _b;
        }
    }

Function parameters can be used as any other local variable and they can also be assigned to.

.. note::

  An :ref:`external function<external-function-calls>` cannot accept a
  multi-dimensional array as an input
  parameter. This functionality is possible if you enable the ABI coder v2
  by adding ``pragma abicoder v2;`` to your source file.

  An :ref:`internal function<external-function-calls>` can accept a
  multi-dimensional array without enabling the feature.

.. index:: return array, return string, array, string, array of strings, dynamic array, variably sized array, return struct, struct

Return Variables
----------------

Function return variables are declared with the same syntax after the
``returns`` keyword.

For example, suppose you want to return two results: the sum and the product of
two integers passed as function parameters, then you use something like:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint o_sum, uint o_product)
        {
            o_sum = _a + _b;
            o_product = _a * _b;
        }
    }

The names of return variables can be omitted.
Return variables can be used as any other local variable and they
are initialized with their :ref:`default value <default-value>` and have that
value until they are (re-)assigned.

You can either explicitly assign to return variables and
then leave the function as above,
or you can provide return values
(either a single or :ref:`multiple ones<multi-return>`) directly with the ``return``
statement:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract Simple {
        function arithmetic(uint _a, uint _b)
            public
            pure
            returns (uint o_sum, uint o_product)
        {
            return (_a + _b, _a * _b);
        }
    }

If you use an early ``return`` to leave a function that has return variables,
you must provide return values together with the return statement.

.. note::
    You cannot return some types from non-internal functions, notably
    multi-dimensional dynamic arrays and structs. If you enable the
    ABI coder v2 by adding ``pragma abicoder v2;``
    to your source file then more types are available, but
    ``mapping`` types are still limited to inside a single contract and you
    cannot transfer them.

.. _multi-return:

Returning Multiple Values
-------------------------

When a function has multiple return types, the statement ``return (v0, v1, ..., vn)`` can be used to return multiple values.
The number of components must be the same as the number of return variables
and their types have to match, potentially after an :ref:`implicit conversion <types-conversion-elementary-types>`.

.. _state-mutability:

State Mutability
================

.. index:: ! view function, function;view

.. _view-functions:

Fonctions View
==============

Les fonctions peuvent être déclarées ``view``, auquel cas elles promettent de ne pas modifier l'état.

.. note::
  Si la cible EVM du compilateur est Byzantium ou plus récent (par défaut), l'opcode ``STATICCALL`` est utilisé pour les fonctions ``view`` qui imposent à l'état de rester non modifié lors de l'exécution EVM. Pour les librairies, on utilise les fonctions ``view`` et ``DELEGATECALL`` parce qu'il n'y a pas de ``DELEGATECALL`` et ``STATICCALL`` combinés.
  Cela signifie que les fonctions ``view`` de librairies n'ont pas de contrôles d'exécution qui empêchent les modifications d'état. Cela ne devrait pas avoir d'impact négatif sur la sécurité car le code de librairies est généralement connu au moment de la compilation et le vérificateur statique effectue les vérifications au moment de la compilation.

Les déclarations suivantes sont considérées comme une modification de l'état :

#. Ecrire dans les variables d'état.
#. :ref:`Emettre des événements <events>`.
#. :ref:`Création d'autres contrats <creating-contracts>`.
#. Utiliser ``selfdestruct``.
#. Envoyer des Ethers par des appels.
#. Appeler une fonction qui n'est pas marquée ``view`` ou ``pure``.
#. Utilisation d'appels bas niveau.
#. Utilisation d'assembleur inline qui contient certains opcodes.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public view returns (uint) {
            return a * (b + 42) + block.timestamp;
        }
    }

.. note::
  ``constant`` sur les fonctions était un alias de ``view``, mais cela a été abandonné dans la version 0.5.0.

.. note::
  Les méthodes Getter sont automatiquement marquées ``view``.

.. note::
  Avant la version 0.5.0, le compilateur n'utilisait pas l'opcode ``STATICCALL``.
  pour les fonctions ``view``.
  Cela permettait de modifier l'état des fonctions ``view`` grâce à l'utilisation de
  conversions de type explicites non valides.
  En utilisant ``STATICCALL`` pour les fonctions ``view``, les modifications de la fonction
  sont évités au niveau de l'EVM.

.. index:: ! pure function, function;pure

.. _pure-functions:

Fonctions Pure
==============

Les fonctions peuvent être déclarées ``pures``, auquel cas elles promettent de ne pas lire ou modifier l'état.
In particular, it should be possible to evaluate a ``pure`` function at compile-time given
only its inputs and ``msg.data``, but without any knowledge of the current blockchain state.
This means that reading from ``immutable`` variables can be a non-pure operation.

.. note::
  Si la cible EVM du compilateur est Byzantium ou plus récente (par défaut), on utilise l'opcode ``STATICCALL``, ce qui ne garantit pas que l'état ne soit pas lu, mais au moins qu'il ne soit pas modifié.

En plus de la liste des modificateurs d'état expliqués ci-dessus, sont considérés comme des lectures de l'état :

#. Lecture des variables d'état.
#. Accéder à ``address(this).balance`` ou ``<address>.balance``.
#. Accéder à l'un des membres de ``block``, ``tx``, ``msg`` (à l'exception de ``msg.sig`` et ``msg.data``).
#. Appeler une fonction qui n'est pas marquée ``pure``.
#. Utilisation d'assembleur inline qui contient certains opcodes.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

.. note::
  Avant la version 0.5.0, le compilateur n'utilisait pas l'opcode ``STATICCALL`` pour les fonctions ``pure``.
  Cela permettait de modifier l'état des fonctions ``pures`` en utilisant des conversions de type explicites invalides.
  En utilisant ``STATICCALL`` pour des fonctions ``pures``, les modifications de l'état sont empêchées au niveau de l'EVM.

.. avertissement::
  Il n'est pas possible d'empêcher les fonctions de lire l'état au niveau de l'EVM, il est seulement possible de les empêcher d'écrire dans l'état (c'est-à-dire que seul "view" peut être exécuté au niveau de l'EVM, ``pure`` ne peut pas).

.. avertissement::
  Avant la version 0.4.17, le compilateur n'appliquait pas le fait que ``pure`` ne lisait pas l'état.
  Il s'agit d'un contrôle de type à la compilation, qui peut être contourné en effectuant des conversions explicites invalides entre les types de contrats, parce que le compilateur peut vérifier que le type de contrat ne fait pas d'opérations de changement d'état, mais il ne peut pas vérifier que le contrat qui sera appelé à l'exécution est effectivement de ce type.

.. _special-functions:

Special Functions
=================

.. index:: ! receive ether function, function;receive ! receive

.. _receive-ether-function:

Receive Ether Function
----------------------

A contract can have at most one ``receive`` function, declared using
``receive() external payable { ... }``
(without the ``function`` keyword).
This function cannot have arguments, cannot return anything and must have
``external`` visibility and ``payable`` state mutability.
It can be virtual, can override and can have modifiers.

The receive function is executed on a
call to the contract with empty calldata. This is the function that is executed
on plain Ether transfers (e.g. via ``.send()`` or ``.transfer()``). If no such
function exists, but a payable :ref:`fallback function <fallback-function>`
exists, the fallback function will be called on a plain Ether transfer. If
neither a receive Ether nor a payable fallback function is present, the
contract cannot receive Ether through regular transactions and throws an
exception.

In the worst case, the ``receive`` function can only rely on 2300 gas being
available (for example when ``send`` or ``transfer`` is used), leaving little
room to perform other operations except basic logging. The following operations
will consume more gas than the 2300 gas stipend:

- Writing to storage
- Creating a contract
- Calling an external function which consumes a large amount of gas
- Sending Ether

.. warning::
    Contracts that receive Ether directly (without a function call, i.e. using ``send`` or ``transfer``)
    but do not define a receive Ether function or a payable fallback function
    throw an exception, sending back the Ether (this was different
    before Solidity v0.4.0). So if you want your contract to receive Ether,
    you have to implement a receive Ether function (using payable fallback functions for receiving Ether is
    not recommended, since it would not fail on interface confusions).


.. warning::
    A contract without a receive Ether function can receive Ether as a
    recipient of a *coinbase transaction* (aka *miner block reward*)
    or as a destination of a ``selfdestruct``.

    A contract cannot react to such Ether transfers and thus also
    cannot reject them. This is a design choice of the EVM and
    Solidity cannot work around it.

    It also means that ``address(this).balance`` can be higher
    than the sum of some manual accounting implemented in a
    contract (i.e. having a counter updated in the receive Ether function).

Below you can see an example of a Sink contract that uses function ``receive``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // This contract keeps all Ether sent to it with no way
    // to get it back.
    contract Sink {
        event Received(address, uint);
        receive() external payable {
            emit Received(msg.sender, msg.value);
        }
    }

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Fonction de repli
=================

Un contrat peut avoir exactement une fonction dwe repli ``fallback``, declared using either ``fallback () external [payable]``
or ``fallback (bytes calldata _input) external [payable] returns (bytes memory _output)``
(both without the ``function`` keyword).
This function must have ``external`` visibility. A fallback function can be virtual, can override
and can have modifiers.

The fallback function is executed on a call to the contract if none of the other
functions match the given function signature, or if no data was supplied at
all and there is no :ref:`receive Ether function <receive-ether-function>`.
The fallback function always receives data, but in order to also receive Ether
it must be marked ``payable``.

If the version with parameters is used, ``_input`` will contain the full data sent to the contract
(equal to ``msg.data``) and can return data in ``_output``. The returned data will not be
ABI-encoded. Instead it will be returned without modifications (not even padding).

In the worst case, if a payable fallback function is also used in
place of a receive function, it can only rely on 2300 gas being
available (see :ref:`receive Ether function <receive-ether-function>`
for a brief description of the implications of this).

Comme toute fonction, la fonction de fallback peut exécuter des opérations complexes tant que suffisamment de gas lui est transmis.

.. note::
    If you want to decode the input data, you can check the first four bytes
    for the function selector and then
    you can use ``abi.decode`` together with the array slice syntax to
    decode ABI-encoded data:
    ``(c, d) = abi.decode(_input[4:], (uint256, uint256));``
    Note that this should only be used as a last resort and
    proper functions should be used instead.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    contract Test {
        uint x;
        // This function is called for all messages sent to
        // this contract (there is no other function).
        // Sending Ether to this contract will cause an exception,
        // because the fallback function does not have the `payable`
        // modifier.
        fallback() external { x = 1; }
    }

    contract TestPayable {
        uint x;
        uint y;
        // This function is called for all messages sent to
        // this contract, except plain Ether transfers
        // (there is no other function except the receive function).
        // Any call with non-empty calldata to this contract will execute
        // the fallback function (even if Ether is sent along with the call).
        fallback() external payable { x = 1; y = msg.value; }

        // This function is called for plain Ether transfers, i.e.
        // for every call with empty calldata.
        receive() external payable { x = 2; y = msg.value; }
    }

    contract Caller {
        function callTest(Test test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1.

            // address(test) will not allow to call ``send`` directly, since ``test`` has no payable
            // fallback function.
            // It has to be converted to the ``address payable`` type to even allow calling ``send`` on it.
            address payable testPayable = payable(address(test));

            // If someone sends Ether to that contract,
            // the transfer will fail, i.e. this returns false here.
            return testPayable.send(2 ether);
        }

        function callTestPayable(TestPayable test) public returns (bool) {
            (bool success,) = address(test).call(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1 and test.y becoming 0.
            (success,) = address(test).call{value: 1}(abi.encodeWithSignature("nonExistingFunction()"));
            require(success);
            // results in test.x becoming == 1 and test.y becoming 1.

            // If someone sends Ether to that contract, the receive function in TestPayable will be called.
            // Since that function writes to storage, it takes more gas than is available with a
            // simple ``send`` or ``transfer``. Because of that, we have to use a low-level call.
            (success,) = address(test).call{value: 2 ether}("");
            require(success);
            // results in test.x becoming == 2 and test.y becoming 2 ether.

            return true;
        }
    }

.. index:: ! overload

.. _overload-function:

Surcharge de fonctions
====================

Un contrat peut avoir plusieurs fonctions du même nom, mais avec des types de paramètres différents.
Ce processus est appelé "surcharge" et s'applique également aux fonctions héritées.
L'exemple suivant montre la surcharge de la fonction ``f`` dans le champ d'application du contrat ``A``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint _in) public pure returns (uint out) {
            out = _in;
        }

        function f(uint _in, bool _really) public pure returns (int out) {
            if (_really)
                out = int(_in);
        }
    }

Des fonctions surchargées sont également présentes dans l'interface externe. C'est une erreur si deux fonctions visibles de l'extérieur diffèrent par leur type Solidity (ici `A` et `B`) mais pas par leur type extérieur (ici ``address``).

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    // Ceci ne compile pas
    contract A {
        function f(B _in) public pure returns (B out) {
            out = _in;
        }

        function f(A _in) public pure returns (B out) {
            out = B(address(_in));
        }
    }

    contract B {
    }


Les deux fonctions ``f`` surchargées ci-dessus acceptent des addresses du point de vue de l'ABI, mais ces adresses sont considérées comme différents types en Solidity.

Résolution des surcharges et concordance des arguments
-----------------------------------------

Les fonctions surchargées sont sélectionnées en faisant correspondre les déclarations de fonction dans le scope actuel aux arguments fournis dans l'appel de fonction. La fonction évaluée est choisie si tous les arguments peuvent être implicitement convertis en types attendus. S'il y a plusieurs fonctions correspondantes, la résolution échoue.

.. note::
    Le type des valeurs retournées par la fonction n'est pas pris en compte dans la résolution des surcharges.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract A {
        function f(uint8 _in) public pure returns (uint8 out) {
            out = _in;
        }

        function f(uint256 _in) public pure returns (uint256 out) {
            out = _in;
        }
    }

L'appel de ``f(50)`` créerait une erreur de type puisque ``50`` peut être implicitement converti à la fois en type ``uint8`` et ``uint256``. D'un autre côté, ``f(256)`` se résoudrait à ``f(uint256)`` car ``256`` ne peut pas être implicitement converti en ``uint8``.