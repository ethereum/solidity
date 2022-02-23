#####################################
Expressions et structures de contrôle
#####################################

.. index:: ! parameter, parameter;input, parameter;output, function parameter, parameter;function, return variable, variable;return, return


.. index:: if, else, while, do/while, for, break, continue, return, switch, goto

Structures de controle
======================

La plupart des structures de contrôle connues des langages à accolades sont disponibles dans Solidity :

Nous disposons de: ``if``, ``else``, ``while``, ``do``, ``for``, ``break``, ``continue``, ``return``, avec la syntaxe famillière du C ou du JavaScript.

Solidity also supports exception handling in the form of ``try``/``catch``-statements,
but only for :ref:`external function calls <external-function-calls>` and
contract creation calls. Errors can be created using the :ref:`revert statement <revert-statement>`.

Les parenthèses ne peuvent *pas* être omises pour les conditions, mais les accolades peuvent être omises autour des déclaration en une opération.

Notez qu'il n'y a pas de conversion de types non booléens vers types booléens comme en C et JavaScript, donc ``if (1) {...}`` n'est pas valable en Solidity.

.. index:: ! function;call, function;internal, function;external

.. _function-calls:

Appels de fonction
==================

.. _internal-function-calls:

Appels de fonction internes
---------------------------

Les fonctions du contrat en cours peuvent être appelées directement (``internal``), également de manière récursive, comme le montre cet exemple absurde:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    // This will report a warning
    contract C {
        function g(uint a) public pure returns (uint ret) { return a + f(); }
        function f() internal pure returns (uint ret) { return g(7) + f(); }
    }

Ces appels de fonction sont traduits en simples sauts ( ``JUMP``) à l'intérieur de l'EVM. Cela a pour effet que la mémoire actuelle n'est pas effacée, c'est-à-dire qu'il est très efficace de passer des références de mémoire aux fonctions appelées en interne. Seules les fonctions du même contrat peuvent être appelées en interne.

Vous devriez toujours éviter une récursivité excessive, car chaque appel de fonction interne utilise au moins un emplacement de pile et il y a au maximum un peu moins de 1024 emplacements disponibles.

.. _external-function-calls:

Appels de fonction externes
---------------------------

Les fonctions peuvent aussi être appellées avec la syntaxe ``this.g(8);`` et ``c.g(2);`` (où ``c`` est une instance de contrat et ``g`` est une fonction appartenant à ``c``).
Appeler la fonction ``g`` d'une manière ou d'une autre résulte en un appel "externe", utilisant ``call`` au lieu de sauts dans le code du contrat.
Veuillez noter que les appels de fonction sur ``this`` ne peuvent pas être utilisés dans le constructeur, car le contrat actuel n'a pas encore été créé.

Les fonctions d'autres contrats doivent être appelées en externe. Pour un appel externe, tous les arguments de fonction doivent être copiés en mémoire.

.. note::
    A function call from one contract to another does not create its own transaction,
    it is a message call as part of the overall transaction.

Lors de l'appel de fonctions d'autres contrats, le montant de Wei envoyé avec l'appel et le gas peut être spécifié avec les options spéciales ``.value()`` et ``.gas()``: ``{value: 10, gas: 10000}``.
Note that it is discouraged to specify gas values explicitly, since the gas costs
of opcodes can change in the future. Any Wei you send to the contract is added
to the total balance of that contract:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.9.0;

    contract InfoFeed {
        function info() public payable returns (uint ret) { return 42; }
    }

    contract Consumer {
        InfoFeed feed;
        function setFeed(InfoFeed addr) public { feed = addr; }
        function callFeed() public { feed.info{value: 10, gas: 800}(); }
    }

Vous devez utiliser le modificateur ``payable`` avec la fonction ``info`` pour pouvoir appeler ``.value()`` .

.. warning::
  Veillez à ce que ``feed.info.value(10).gas(800)`` ne définisse que localement la ``value`` et la quantité de ``gas`` envoyés avec l'appel de fonction, et que les parenthèses à la fin sont bien présentes pour effectuer l'appel. So
  ``feed.info{value: 10, gas: 800}`` does not call the function and
  the ``value`` and ``gas`` settings are lost, only
  ``feed.info{value: 10, gas: 800}()`` performs the function call.

Due to the fact that the EVM considers a call to a non-existing contract to
always succeed, Solidity uses the ``extcodesize`` opcode to check that
the contract that is about to be called actually exists (it contains code)
and causes an exception if it does not. This check is skipped if the return
data will be decoded after the call and thus the ABI decoder will catch the
case of a non-existing contract.

Note that this check is not performed in case of :ref:`low-level calls <address_related>` which
operate on addresses rather than contract instances.

.. note::
    Be careful when using high-level calls to
    :ref:`precompiled contracts <precompiledContracts>`,
    since the compiler considers them non-existing according to the
    above logic even though they execute code and can return data.

Les appels de fonction provoquent des exceptions si le contrat appelé lui-même lève une exception ou manque de gas.

.. warning::
 Toute interaction avec un autre contrat présente un danger potentiel, surtout si le code source du contrat n'est pas connu à l'avance. Le contrat actuel cède le contrôle au contrat appelé et cela peut potentiellement faire à peu près n'importe quoi. Même si le contrat appelé hérite d'un contrat parent connu, le contrat d'héritage doit seulement avoir une interface correcte. L'exécution du contrat peut cependant être totalement arbitraire et donc représentent un danger. En outre, soyez prêt au cas où il appelle d'autres fonctions de votre contrat ou même de retour dans le contrat d'appel avant le retour du premier appel. Cela signifie que le contrat appelé peut modifier les variables d'état du contrat appelant via ses fonctions. Écrivez vos fonctions de manière à ce que, par exemple, les appels à
 les fonctions externes se produisent après tout changement de variables d'état dans votre contrat, de sorte que votre contrat n'est pas vulnérable à un exploit de réentrée.

.. note::
    Before Solidity 0.6.2, the recommended way to specify the value and gas was to
    use ``f.value(x).gas(g)()``. This was deprecated in Solidity 0.6.2 and is no
    longer possible since Solidity 0.7.0.

Appels nommés et paramètres de fonction anonymes
------------------------------------------------

Les arguments d'appel de fonction peuvent être donnés par leur nom, dans n'importe quel ordre, s'ils sont inclus dans ``{ }`` comme on peut le voir dans l'exemple qui suit. La liste d'arguments doit coïncider par son nom avec la liste des paramètres de la déclaration de fonction, mais peut être dans un ordre arbitraire.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract C {
        mapping(uint => uint) data;

        function f() public {
            set({value: 2, key: 3});
        }

        function set(uint key, uint value) public {
            data[key] = value;
        }

    }

Noms des paramètres de fonction omis
------------------------------------

Les noms des paramètres inutilisés (en particulier les paramètres de retour) peuvent être omis.
Ces paramètres seront toujours présents sur la pile, mais ils sont inaccessibles.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract C {
        // omitted name for parameter
        function func(uint k, uint) public pure returns(uint) {
            return k;
        }
    }


.. index:: ! new, contracts;creating

.. _creating-contracts:

Création de contrats via ``new``
================================

Un contrat peut créer d'autres contrats en utilisant le mot-clé ``new``. Le code complet du contrat en cours de création doit être connu lors de la compilation afin d'éviter les dépendances récursives liées à la création.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract D {
        uint public x;
        constructor(uint a) payable {
            x = a;
        }
    }

    contract C {
        D d = new D(4); // sera exécuté dans le constructor de C

        function createD(uint arg) public {
            D newD = new D(arg);
            newD.x();
        }

        function createAndEndowD(uint arg, uint amount) public payable {
            // Send ether along with the creation
            D newD = new D{value: amount}(arg);
            newD.x();
        }
    }

Comme dans l'exemple, il est possible d'envoyer des Ether en créant une instance de ``D`` en utilisant l'option ``.value()``, mais il n'est pas possible de limiter la quantité de gas.
Si la création échoue (à cause d'une rupture de pile, d'un manque de gas ou d'autres problèmes), une exception est levée.

Salted contract creations / create2
-----------------------------------

When creating a contract, the address of the contract is computed from
the address of the creating contract and a counter that is increased with
each contract creation.

If you specify the option ``salt`` (a bytes32 value), then contract creation will
use a different mechanism to come up with the address of the new contract:

It will compute the address from the address of the creating contract,
the given salt value, the (creation) bytecode of the created contract and the constructor
arguments.

In particular, the counter ("nonce") is not used. This allows for more flexibility
in creating contracts: You are able to derive the address of the
new contract before it is created. Furthermore, you can rely on this address
also in case the creating
contracts creates other contracts in the meantime.

The main use-case here is contracts that act as judges for off-chain interactions,
which only need to be created if there is a dispute.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract D {
        uint public x;
        constructor(uint a) {
            x = a;
        }
    }

    contract C {
        function createDSalted(bytes32 salt, uint arg) public {
            // This complicated expression just tells you how the address
            // can be pre-computed. It is just there for illustration.
            // You actually only need ``new D{salt: salt}(arg)``.
            address predictedAddress = address(uint160(uint(keccak256(abi.encodePacked(
                bytes1(0xff),
                address(this),
                salt,
                keccak256(abi.encodePacked(
                    type(D).creationCode,
                    arg
                ))
            )))));

            D d = new D{salt: salt}(arg);
            require(address(d) == predictedAddress);
        }
    }

.. warning::
    There are some peculiarities in relation to salted creation. A contract can be
    re-created at the same address after having been destroyed. Yet, it is possible
    for that newly created contract to have a different deployed bytecode even
    though the creation bytecode has been the same (which is a requirement because
    otherwise the address would change). This is due to the fact that the constructor
    can query external state that might have changed between the two creations
    and incorporate that into the deployed bytecode before it is stored.


Ordre d'évaluation des expressions
==================================

L'ordre d'évaluation des expressions est non spécifié (plus formellement, l'ordre dans lequel les enfants d'un noeud de l'arbre des expressions sont évalués n'est pas spécifié, mais ils sont bien sûr évalués avant le noeud lui-même). La seule garantie est que les instructions sont exécutées dans l'ordre et que les expressions booléennes sont court-circuitées correctement.

.. index:: ! assignment

Assignation
===========

.. index:: ! assignment;destructuring

Déstructuration d'assignations et retour de valeurs multiples
-------------------------------------------------------------

Solidity permet en interne les tuples, c'est-à-dire une liste d'objets de types potentiellement différents dont le nombre est une constante au moment de la compilation. Ces tuples peuvent être utilisés pour retourner plusieurs valeurs en même temps.
Ceux-ci peuvent ensuite être affectés soit à des variables nouvellement déclarées, soit à des variables préexistantes (ou à des LValues en général).

Les tuples ne sont pas des types propres à Solidity, ils ne peuvent être utilisés que pour former des groupes syntaxiques d'expressions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        uint index;

        function f() public pure returns (uint, bool, uint) {
            return (7, true, 2);
        }

        function g() public {
            // Variables declared with type and assigned from the returned tuple,
            // not all elements have to be specified (but the number must match).
            (uint x, , uint y) = f();
            // Common trick to swap values -- does not work for non-value storage types.
            (x, y) = (y, x);
            // Components can be left out (also for variable declarations).
            (index, , ) = f(); // Sets the index to 7
        }
    }

It is not possible to mix variable declarations and non-declaration assignments,
i.e. the following is not valid: ``(x, uint y) = (1, 2);``

.. note::
    Prior to version 0.5.0 it was possible to assign to tuples of smaller size, either
    filling up on the left or on the right side (which ever was empty). This is
    now disallowed, so both sides have to have the same number of components.

.. warning::
    Be careful when assigning to multiple variables at the same time when
    reference types are involved, because it could lead to unexpected
    copying behaviour.

Complications pour les tableaux et les structures
-------------------------------------------------

La sémantique des affectations est un peu plus compliquée pour les types autres que valeurs comme les tableaux et les structs, y compris ``bytes`` et ``string``, voir :ref:`Emplacement des donnés et comportements à l'assignation <data-location-assignment>` pour plus de détails.

In the example below the call to ``g(x)`` has no effect on ``x`` because it creates
an independent copy of the storage value in memory. However, ``h(x)`` successfully modifies ``x``
because only a reference and not a copy is passed.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;

    contract C {
        uint[20] x;

        function f() public {
            g(x);
            h(x);
        }

        function g(uint[20] memory y) internal pure {
            y[2] = 3;
        }

        function h(uint[20] storage y) internal {
            y[3] = 4;
        }
    }

.. index:: ! scoping, declarations, default value

.. _default-value:

Portée et déclarations
======================

Une variable qui est déclarée aura une valeur par défaut initiale dont la représentation octale est égale à une suite de zéros.
Les "valeurs par défaut" des variables sont les "états zéro" typiques quel que soit le type. Par exemple, la valeur par défaut d'un ``bool`` est ``false``. La valeur par défaut pour les types ``uint`` ou ``int`` est ``0``. Pour les tableaux de taille statique et les ``bytes1`` à ``bytes32``, chaque élément individuel sera initialisé à la valeur par défaut correspondant à son type. Enfin, pour les tableaux de taille dynamique, les octets et les chaînes de caractères, la valeur par défaut est un tableau ou une chaîne vide.

La portée en Solidity suit les règles de portée très répandues du C99 (et de nombreux autres languages): Les variables sont visibles du point situé juste après leur déclaration jusqu'à la fin du plus petit bloc ``{ }`` qui contient la déclaration. Par exception à cette règle, les variables déclarées dans la partie initialisation d'une boucle ``for`` ne sont visibles que jusqu'à la fin de la boucle for.

Les variables et autres éléments déclarés en dehors d'un bloc de code, par exemple les fonctions, les contrats, les types définis par l'utilisateur, etc. sont visibles avant même leur déclaration. Cela signifie que vous pouvez utiliser les variables d'état avant qu'elles ne soient déclarées et appeler les fonctions de manière récursive.

Par conséquent, les exemples suivants seront compilés sans avertissement, puisque les deux variables ont le même nom mais des portées disjointes.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    contract C {
        function minimalScoping() pure public {
            {
                uint same;
                same = 1;
            }

            {
                uint same;
                same = 3;
            }
        }
    }

À titre d'exemple particulier des règles de détermination de la portée héritées du C99, notons que, dans ce qui suit, la première affectation à ``x`` affectera en fait la variable externe et non la variable interne. Dans tous les cas, vous obtiendrez un avertissement concernant cette double déclaration.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    // This will report a warning
    contract C {
        function f() pure public returns (uint) {
            uint x = 1;
            {
                x = 2; // this will assign to the outer variable
                uint x;
            }
            return x; // x has value 2
        }
    }

.. warning::
    Avant la version 0.5.0, Solidity suivait les mêmes règles de scoping que JavaScript, c'est-à-dire qu'une variable déclarée n'importe où dans une fonction était dans le champ d'application pour l'ensemble de la fonction, peu importe où elle était déclarée. L'exemple suivant montre un extrait de code qui compilait, mais conduit aujourd'hui à une erreur à partir de la version 0.5.0.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    // This will not compile
    contract C {
        function f() pure public returns (uint) {
            x = 2;
            uint x;
            return x;
        }
    }


.. index:: ! safe math, safemath, checked, unchecked
.. _unchecked:

Checked or Unchecked Arithmetic
===============================

An overflow or underflow is the situation where the resulting value of an arithmetic operation,
when executed on an unrestricted integer, falls outside the range of the result type.

Prior to Solidity 0.8.0, arithmetic operations would always wrap in case of
under- or overflow leading to widespread use of libraries that introduce
additional checks.

Since Solidity 0.8.0, all arithmetic operations revert on over- and underflow by default,
thus making the use of these libraries unnecessary.

To obtain the previous behaviour, an ``unchecked`` block can be used:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.0;
    contract C {
        function f(uint a, uint b) pure public returns (uint) {
            // This subtraction will wrap on underflow.
            unchecked { return a - b; }
        }
        function g(uint a, uint b) pure public returns (uint) {
            // This subtraction will revert on underflow.
            return a - b;
        }
    }

The call to ``f(2, 3)`` will return ``2**256-1``, while ``g(2, 3)`` will cause
a failing assertion.

The ``unchecked`` block can be used everywhere inside a block, but not as a replacement
for a block. It also cannot be nested.

The setting only affects the statements that are syntactically inside the block.
Functions called from within an ``unchecked`` block do not inherit the property.

.. note::
    To avoid ambiguity, you cannot use ``_;`` inside an ``unchecked`` block.

The following operators will cause a failing assertion on overflow or underflow
and will wrap without an error if used inside an unchecked block:

``++``, ``--``, ``+``, binary ``-``, unary ``-``, ``*``, ``/``, ``%``, ``**``

``+=``, ``-=``, ``*=``, ``/=``, ``%=``

.. warning::
    It is not possible to disable the check for division by zero
    or modulo by zero using the ``unchecked`` block.

.. note::
   Bitwise operators do not perform overflow or underflow checks.
   This is particularly visible when using bitwise shifts (``<<``, ``>>``, ``<<=``, ``>>=``) in
   place of integer division and multiplication by a power of 2.
   For example ``type(uint256).max << 3`` does not revert even though ``type(uint256).max * 8`` would.

.. note::
    The second statement in ``int x = type(int).min; -x;`` will result in an overflow
    because the negative range can hold one more value than the positive range.

Explicit type conversions will always truncate and never cause a failing assertion
with the exception of a conversion from an integer to an enum type.

.. index:: ! exception, ! throw, ! assert, ! require, ! revert, ! errors

.. _assert-and-require:

Gestion d'erreurs: Assert, Require, Revert et Exceptions
========================================================

Solidity utilise des exceptions qui restaurent l'état pour gérer les erreurs. Une telle exception annule toutes les modifications apportées à l'état de l'appel en cours (et de tous ses sous-appels) et signale également une erreur à l'appelant.

Lorsque des exceptions se produisent dans un sous-appel, elles "remontent à la surface" automatiquement (c'est-à-dire que les exceptions sont déclenchées en casacade). Les exceptions à cette règle sont ``send`` et les fonctions de bas niveau ``call``, ``delegatecall`` et ``staticcall``, qui retournent ``false`` comme première valeur de retour en cas d'exception au lieu de provoquer une exception qui ne pourra donc pas remonter.

.. warning::
    Les fonctions de bas niveau ``call``, ``delegatecall`` et ``staticcall`` renvoient ``true`` comme première valeur de retour si le compte appelé est inexistant, dû à la conception de l'EVM. L'existence doit être vérifiée avant l'appel si désiré.

Exceptions can contain error data that is passed back to the caller
in the form of :ref:`error instances <errors>`.
The built-in errors ``Error(string)`` and ``Panic(uint256)`` are
used by special functions, as explained below. ``Error`` is used for "regular" error conditions
while ``Panic`` is used for errors that should not be present in bug-free code.

Panic via ``assert`` and Error via ``require``
----------------------------------------------

Les fonctions utilitaires ``assert`` et ``require`` peuvent être utilisées pour vérifier les conditions et lancer une exception si la condition n'est pas remplie.

The ``assert`` function creates an error of type ``Panic(uint256)``.
The same error is created by the compiler in certain situations as listed below.

Assert should only be used to test for internal
errors, and to check invariants. Properly functioning code should
never create a Panic, not even on invalid external input.
If this happens, then there
is a bug in your contract which you should fix. Language analysis
tools can evaluate your contract to identify the conditions and
function calls which will cause a Panic.

A Panic exception is generated in the following situations.
The error code supplied with the error data indicates the kind of panic.

#. 0x00: Used for generic compiler inserted panics.
#. 0x01: If you call ``assert`` with an argument that evaluates to false.
#. 0x11: If an arithmetic operation results in underflow or overflow outside of an ``unchecked { ... }`` block.
#. 0x12; If you divide or modulo by zero (e.g. ``5 / 0`` or ``23 % 0``).
#. 0x21: If you convert a value that is too big or negative into an enum type.
#. 0x22: If you access a storage byte array that is incorrectly encoded.
#. 0x31: If you call ``.pop()`` on an empty array.
#. 0x32: If you access an array, ``bytesN`` or an array slice at an out-of-bounds or negative index (i.e. ``x[i]`` where ``i >= x.length`` or ``i < 0``).
#. 0x41: If you allocate too much memory or create an array that is too large.
#. 0x51: If you call a zero-initialized variable of internal function type.

The ``require`` function either creates an error without any data or
an error of type ``Error(string)``. It
should be used to ensure valid conditions
that cannot be detected until execution time.
This includes conditions on inputs
or return values from calls to external contracts.

.. note::

    It is currently not possible to use custom errors in combination
    with ``require``. Please use ``if (!condition) revert CustomError();`` instead.

An ``Error(string)`` exception (or an exception without data) is generated
by the compiler
in the following situations:

#. Calling ``require(x)`` where ``x`` evaluates to ``false``.
#. If you use ``revert()`` or ``revert("description")``.
#. If you perform an external function call targeting a contract that contains no code.
#. If your contract receives Ether via a public function without
   ``payable`` modifier (including the constructor and the fallback function).
#. If your contract receives Ether via a public getter function.

For the following cases, the error data from the external call
(if provided) is forwarded. This means that it can either cause
an `Error` or a `Panic` (or whatever else was given):

#. If a ``.transfer()`` fails.
#. If you call a function via a message call but it does not finish
   properly (i.e., it runs out of gas, has no matching function, or
   throws an exception itself), except when a low level operation
   ``call``, ``send``, ``delegatecall``, ``callcode`` or ``staticcall``
   is used. The low level operations never throw exceptions but
   indicate failures by returning ``false``.
#. If you create a contract using the ``new`` keyword but the contract
   creation :ref:`does not finish properly<creating-contracts>`.

Vous pouvez facultativement fournir une chaîne de message pour ``require``, mais pas pour ``assert``.

.. note::
    If you do not provide a string argument to ``require``, it will revert
    with empty error data, not even including the error selector.
    

Dans l'exemple suivant, vous pouvez voir comment ``require`` peut être utilisé pour vérifier facilement les conditions sur les entrées et comment ``assert`` peut être utilisé pour vérifier les erreurs internes.

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract Sharer {
        function sendHalf(address payable addr) public payable returns (uint balance) {
            require(msg.value % 2 == 0, "Even value required.");
            uint balanceBeforeTransfer = address(this).balance;
            addr.transfer(msg.value / 2);
            // Since transfer throws an exception on failure and
            // cannot call back here, there should be no way for us to
            // still have half of the money.
            assert(address(this).balance == balanceBeforeTransfer - msg.value / 2);
            return address(this).balance;
        }
    }

En interne, Solidity exécute une opération de ``revert`` (instruction ``0xfd``). Dans les deux cas, cela provoque l'annulation toutes les modifications apportées à l'état de l'EVM dans l'appel courant. La raison du retour en arrière est qu'il n'y a pas de moyen sûr de continuer l'exécution, parce qu'un effet attendu ne s'est pas produit. Parce que nous voulons conserver l'atomicité des transactions, la chose la plus sûre à faire est d'annuler tous les changements et de rendre toute la transaction (ou au moins l'appel) sans effet. 

In both cases, the caller can react on such failures using ``try``/``catch``, but
the changes in the callee will always be reverted.

.. note::

    Panic exceptions used to use the ``invalid`` opcode before Solidity 0.8.0,
    which consumed all gas available to the call.
    Exceptions that use ``require`` used to consume all gas until before the Metropolis release.

.. _revert-statement:

``revert``
----------

A direct revert can be triggered using the ``revert`` statement and the ``revert`` function.

The ``revert`` statement takes a custom error as direct argument without parentheses:

    revert CustomError(arg1, arg2);

For backwards-compatibility reasons, there is also the ``revert()`` function, which uses parentheses
and accepts a string:

    revert();
    revert("description");

The error data will be passed back to the caller and can be caught there.
Using ``revert()`` causes a revert without any error data while ``revert("description")``
will create an ``Error(string)`` error.

Using a custom error instance will usually be much cheaper than a string description,
because you can use the name of the error to describe it, which is encoded in only
four bytes. A longer description can be supplied via NatSpec which does not incur
any costs.

The following example shows how to use an error string and a custom error instance
together with ``revert`` and the equivalent ``require``:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract VendingMachine {
        address owner;
        error Unauthorized();
        function buy(uint amount) public payable {
            if (amount > msg.value / 2 ether)
                revert("Not enough Ether provided.");
            // Alternative way to do it:
            require(
                amount <= msg.value / 2 ether,
                "Not enough Ether provided."
            );
            // Perform the purchase.
        }
        function withdraw() public {
            if (msg.sender != owner)
                revert Unauthorized();

            payable(msg.sender).transfer(address(this).balance);
        }
    }

The two ways ``if (!condition) revert(...);`` and ``require(condition, ...);`` are
equivalent as long as the arguments to ``revert`` and ``require`` do not have side-effects,
for example if they are just strings.

.. note::
    The ``require`` function is evaluated just as any other function.
    This means that all arguments are evaluated before the function itself is executed.
    In particular, in ``require(condition, f())`` the function ``f`` is executed even if
    ``condition`` is true.

La chaîne fournie sera :ref:`abi-encoded <ABI>` comme si c'était un appel à une fonction ``Error(string)``.
Dans l'exemple ci-dessus, ``revert("Not enough Ether provided.");``` fera en sorte que les données hexadécimales suivantes soient définies comme données de retour d'erreur :

.. code::

    0x08c379a0                                                         // Selecteur de fonction pour Error(string)
    0x0000000000000000000000000000000000000000000000000000000000000020 // Décalage des données
    0x000000000000000000000000000000000000000000000000000000000000001a // Taille de la string
    0x4e6f7420656e6f7567682045746865722070726f76696465642e000000000000 // Données de la string


The provided message can be retrieved by the caller using ``try``/``catch`` as shown below.

.. note::
    There used to be a keyword called ``throw`` with the same semantics as ``revert()`` which
    was deprecated in version 0.4.13 and removed in version 0.5.0.


.. _try-catch:

``try``/``catch``
-----------------

A failure in an external call can be caught using a try/catch statement, as follows:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.1;

    interface DataFeed { function getData(address token) external returns (uint value); }

    contract FeedConsumer {
        DataFeed feed;
        uint errorCount;
        function rate(address token) public returns (uint value, bool success) {
            // Permanently disable the mechanism if there are
            // more than 10 errors.
            require(errorCount < 10);
            try feed.getData(token) returns (uint v) {
                return (v, true);
            } catch Error(string memory /*reason*/) {
                // This is executed in case
                // revert was called inside getData
                // and a reason string was provided.
                errorCount++;
                return (0, false);
            } catch Panic(uint /*errorCode*/) {
                // This is executed in case of a panic,
                // i.e. a serious error like division by zero
                // or overflow. The error code can be used
                // to determine the kind of error.
                errorCount++;
                return (0, false);
            } catch (bytes memory /*lowLevelData*/) {
                // This is executed in case revert() was used.
                errorCount++;
                return (0, false);
            }
        }
    }

The ``try`` keyword has to be followed by an expression representing an external function call
or a contract creation (``new ContractName()``).
Errors inside the expression are not caught (for example if it is a complex expression
that also involves internal function calls), only a revert happening inside the external
call itself. The ``returns`` part (which is optional) that follows declares return variables
matching the types returned by the external call. In case there was no error,
these variables are assigned and the contract's execution continues inside the
first success block. If the end of the success block is reached, execution continues after the ``catch`` blocks.

Solidity supports different kinds of catch blocks depending on the
type of error:

- ``catch Error(string memory reason) { ... }``: This catch clause is executed if the error was caused by ``revert("reasonString")`` or
  ``require(false, "reasonString")`` (or an internal error that causes such an
  exception).

- ``catch Panic(uint errorCode) { ... }``: If the error was caused by a panic, i.e. by a failing ``assert``, division by zero,
  invalid array access, arithmetic overflow and others, this catch clause will be run.

- ``catch (bytes memory lowLevelData) { ... }``: This clause is executed if the error signature
  does not match any other clause, if there was an error while decoding the error
  message, or
  if no error data was provided with the exception.
  The declared variable provides access to the low-level error data in that case.

- ``catch { ... }``: If you are not interested in the error data, you can just use
  ``catch { ... }`` (even as the only catch clause) instead of the previous clause.


It is planned to support other types of error data in the future.
The strings ``Error`` and ``Panic`` are currently parsed as is and are not treated as identifiers.

In order to catch all error cases, you have to have at least the clause
``catch { ...}`` or the clause ``catch (bytes memory lowLevelData) { ... }``.

The variables declared in the ``returns`` and the ``catch`` clause are only
in scope in the block that follows.

.. note::

    If an error happens during the decoding of the return data
    inside a try/catch-statement, this causes an exception in the currently
    executing contract and because of that, it is not caught in the catch clause.
    If there is an error during decoding of ``catch Error(string memory reason)``
    and there is a low-level catch clause, this error is caught there.

.. note::

    If execution reaches a catch-block, then the state-changing effects of
    the external call have been reverted. If execution reaches
    the success block, the effects were not reverted.
    If the effects have been reverted, then execution either continues
    in a catch block or the execution of the try/catch statement itself
    reverts (for example due to decoding failures as noted above or
    due to not providing a low-level catch clause).

.. note::
    The reason behind a failed call can be manifold. Do not assume that
    the error message is coming directly from the called contract:
    The error might have happened deeper down in the call chain and the
    called contract just forwarded it. Also, it could be due to an
    out-of-gas situation and not a deliberate error condition:
    The caller always retains at least 1/64th of the gas in a call and thus
    even if the called contract goes out of gas, the caller still
    has some gas left.
