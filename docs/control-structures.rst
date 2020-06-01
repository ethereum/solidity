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
contract creation calls.

Les parenthèses ne peuvent *pas* être omises pour les conditions, mais les accolades peuvent être omises autour des déclaration en une opération.

Notez qu'il n'y a pas de conversion de types non booléens vers types booléens comme en C et JavaScript, donc ``if (1) {...}`` n'est pas valable en Solidity.

.. index:: ! function;call, function;internal, function;external

.. _function-calls:

Appels de fonction
==================

.. _internal-function-calls:

Appels de fonction internes
---------------------------

Les fonctions du contrat en cours peuvent être appelées directement (``internal``), également de manière récursive, comme le montre cet exemple absurde::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.7.0;

    contract C {
        function g(uint a) public pure returns (uint ret) { return a + f(); }
        function f() internal pure returns (uint ret) { return g(7) + f(); }
    }

Ces appels de fonction sont traduits en simples sauts ( ``JUMP``) à l'intérieur de l'EVM. Cela a pour effet que la mémoire actuelle n'est pas effacée, c'est-à-dire qu'il est très efficace de passer des références de mémoire aux fonctions appelées en interne. Seules les fonctions du même contrat peuvent être appelées en interne.

Vous devriez toujours éviter une récursivité excessive, car chaque appel de fonction interne utilise au moins un emplacement de pile et il y a au maximum un peu moins de 1024 emplacements disponibles.

.. _external-function-calls:

Appels de fonction externes
---------------------------

Les expressions ``this.g(8);`` et ``c.g(2);`` (où ``c`` est une instance de contrat) sont aussi des appels de fonction valides, mais cette fois-ci, la fonction sera appelée ``external``, via un appel de message et non directement via des sauts.
Veuillez noter que les appels de fonction sur ``this`` ne peuvent pas être utilisés dans le constructeur, car le contrat actuel n'a pas encore été créé.

Les fonctions d'autres contrats doivent être appelées en externe. Pour un appel externe, tous les arguments de fonction doivent être copiés en mémoire.

.. note::
    A function call from one contract to another does not create its own transaction,
    it is a message call as part of the overall transaction.

Lors de l'appel de fonctions d'autres contrats, le montant de Wei envoyé avec l'appel et le gas peut être spécifié avec les options spéciales ``.value()`` et ``.gas()``: ``{value: 10, gas: 10000}``.
Note that it is discouraged to specify gas values explicitly, since the gas costs
of opcodes can change in the future. Any Wei you send to the contract is added
to the total balance of that contract:

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.7.0;

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
  Veillez à ce que ``feed.info.value(10).gas(800)`` ne définisse que localement la ``value`` et la quantité de ``gas`` envoyés avec l'appel de fonction, et que les parenthèses à la fin sont bien présentes pour effectuer l'appel. Ainsi, dans cet exemple, la fonction n'est pas appelée.

Les appels de fonction provoquent des exceptions si le contrat appelé n'existe pas (dans le sens où le compte ne contient pas de code) ou si le contrat appelé lui-même lève une exception ou manque de gas.

.. warning::
 Toute interaction avec un autre contrat présente un danger potentiel, surtout si le code source du contrat n'est pas connu à l'avance. Le contrat actuel cède le contrôle au contrat appelé et cela peut potentiellement faire à peu près n'importe quoi. Même si le contrat appelé hérite d'un contrat parent connu, le contrat d'héritage doit seulement avoir une interface correcte. L'exécution du contrat peut cependant être totalement arbitraire et donc représentent un danger. En outre, soyez prêt au cas où il appelle d'autres fonctions de votre contrat ou même de retour dans le contrat d'appel avant le retour du premier appel. Cela signifie que le contrat appelé peut modifier les variables d'état du contrat appelant via ses fonctions. Écrivez vos fonctions de manière à ce que, par exemple, les appels à
 les fonctions externes se produisent après tout changement de variables d'état dans votre contrat, de sorte que votre contrat n'est pas vulnérable à un exploit de réentrée.

.. note::
    Before Solidity 0.6.2, the recommended way to specify the value and gas
    was to use ``f.value(x).gas(g)()``. This is still possible but deprecated
    and will be removed with Solidity 0.7.0.

Appels nommés et paramètres de fonction anonymes
------------------------------------------------

Les arguments d'appel de fonction peuvent être donnés par leur nom, dans n'importe quel ordre, s'ils sont inclus dans ``{ }`` comme on peut le voir dans l'exemple qui suit. La liste d'arguments doit coïncider par son nom avec la liste des paramètres de la déclaration de fonction, mais peut être dans un ordre arbitraire.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.7.0;

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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.7.0;

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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.7.0;

    contract D {
        uint public x;
        constructor(uint a) public payable {
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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.2 <0.7.0;

    contract D {
        uint public x;
        constructor(uint a) public {
            x = a;
        }
    }

    contract C {
        function createDSalted(bytes32 salt, uint arg) public {
            /// This complicated expression just tells you how the address
            /// can be pre-computed. It is just there for illustration.
            /// You actually only need ``new D{salt: salt}(arg)``.
            address predictedAddress = address(uint(keccak256(abi.encodePacked(
                byte(0xff),
                address(this),
                salt,
                keccak256(abi.encodePacked(
                    type(D).creationCode,
                    arg
                ))
            ))));

            D d = new D{salt: salt}(arg);
            require(address(d) == predictedAddress);
        }
    }

.. warning::
    There are some peculiarities in relation to salted creation. A contract can be
    re-created at the same address after having been destroyed. Yet, it is possible
    for that newly created contract to have a different deployed bytecode even
    though the creation bytecode has been the same (which is a requirement because
    otherwise the address would change). This is due to the fact that the compiler
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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.7.0;

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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;
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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;
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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;
    // This will not compile
    contract C {
        function f() pure public returns (uint) {
            x = 2;
            uint x;
            return x;
        }
    }

.. index:: ! exception, ! throw, ! assert, ! require, ! revert, ! errors

.. _assert-and-require:

Gestion d'erreurs: Assert, Require, Revert et Exceptions
========================================================

Solidity utilise des exceptions qui restaurent l'état pour gérer les erreurs. Une telle exception annule toutes les modifications apportées à l'état de l'appel en cours (et de tous ses sous-appels) et signale également une erreur à l'appelant.

Lorsque des exceptions se produisent dans un sous-appel, elles "remontent à la surface" automatiquement (c'est-à-dire que les exceptions sont déclenchées en casacade). Les exceptions à cette règle sont ``send`` et les fonctions de bas niveau ``call``, ``delegatecall`` et ``staticcall``, qui retournent ``false`` comme première valeur de retour en cas d'exception au lieu de lancer une chaine d'exceptions.

.. warning::
    Les fonctions de bas niveau ``call``, ``delegatecall`` et ``staticcall`` renvoient ``true`` comme première valeur de retour si le compte appelé est inexistant, dû à la conception de l'EVM. L'existence doit être vérifiée avant l'appel si désiré.

Exceptions can be caught with the ``try``/``catch`` statement.

``assert`` and ``require``
--------------------------

Les fonctions utilitaires ``assert`` et ``require`` peuvent être utilisées pour vérifier les conditions et lancer une exception si la condition n'est pas remplie.

La fonction ``require`` doit être utilisée pour s'assurer que les conditions valides, telles que les entrées ou les variables d'état du contrat, sont remplies, ou pour valider les valeurs de retour des appels aux contrats externes.
S'ils sont utilisés correctement, les outils d'analyse peuvent évaluer votre contrat afin d'identifier les conditions et les appels de fonction qui parviendront à un échec d'``assert``. Un code fonctionnant correctement ne devrait jamais échouer un ``assert`` ; si cela se produit, il y a un bogue dans votre contrat que vous devriez corriger.

Une exception de type ``assert`` est générée dans les situations suivantes:

#. Si vous accédez à un tableau avec un index trop grand ou négatif (par ex. ``x[i]`` où ``i >= x.length`` ou ``i < 0``).
#. Si vous accédez à une variable de longueur fixe ``bytesN`` à un indice trop grand ou négatif.
#. Si vous divisez ou modulez par zéro (par ex. ``5 / 0`` ou ``23 % 0``).
#. Si vous décalez d'un montant négatif.
#. Si vous convertissez une valeur trop grande ou négative en un type enum.
#. Si vous appelez une variable initialisée nulle de type fonction interne.
#. Si vous appelez ``assert`` avec un argument qui s'évalue à ``false``.

The ``require`` function should be used to ensure valid conditions
that cannot be detected until execution time.
This includes conditions on inputs
or return values from calls to external contracts.

Une exception de type ``require`` est générée dans les situations suivantes:

#. Appeler ``require`` avec un argument qui s'évalue à ``false``.
#. Si vous appelez une fonction via un appel de message mais qu'elle ne se termine pas correctement (c'est-à-dire qu'elle n'a plus de gas, qu'elle n'a pas de fonction correspondante ou qu'elle lance une exception elle-même), sauf lorsqu'une opération de bas niveau ``call``, ``send``, ``staticcall``, ``delegatecall`` ou ``callcode`` est utilisée. Les opérations de bas niveau ne lancent jamais d'exceptions mais indiquent les échecs en retournant ``false``.
#. Si vous créez un contrat en utilisant le mot-clé ``new`` mais que la création du contrat ne se termine pas correctement (voir ci-dessus pour la définition de "ne pas terminer correctement").
#. Si vous effectuez un appel de fonction externe ciblant un contrat qui ne contient aucun code.
#. Si votre contrat reçoit des Ether via une fonction publique sans modificateur ``payable`` (y compris le constructeur et la fonction par defaut).
#. Si votre contrat reçoit des Ether via une fonction de getter public.
#. Si un ``.transfer()`` échoue.

Vous pouvez facultativement fournir une chaîne de message pour ``require``, mais pas pour ``assert``.

Dans l'exemple suivant, vous pouvez voir comment ``require`` peut être utilisé pour vérifier facilement les conditions sur les entrées et comment ``assert`` peut être utilisé pour vérifier les erreurs internes.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

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

En interne, Solidity exécute une opération de retour en arrière (instruction ``0xfd``) pour une exception de type ``require`` et exécute une opération invalide (instruction ``0xfe``) pour lancer une exception de type ``assert``. Dans les deux cas, cela provoque lánnulation toutes les modifications apportées à l'état de l'EVM dans l'appel courant. La raison du retour en arrière est qu'il n'y a pas de moyen sûr de continuer l'exécution, parce qu'un effet attendu ne s'est pas produit. Parce que nous voulons conserver l'atomicité des transactions, la chose la plus sûre à faire est d'annuler tous les changements et de faire toute la transaction (ou au moins l'appel) sans effet. 

In both cases, the caller can react on such failures using ``try``/``catch``
(in the failing ``assert``-style exception only if enough gas is left), but
the changes in the caller will always be reverted.

.. note::
    Les exceptions de type ``assert`` consomment tout le gas disponible pour l'appel, alors que les exceptions de type ``require`` ne consommeront pas de gaz à partir du lancement de Metropolis.

``revert``
----------

The ``revert`` function is another way to trigger exceptions from within other code blocks to flag an error and
revert the current call. The function takes an optional string
message containing details about the error that is passed back to the caller.

L'exemple suivant montre comment une chaîne d'erreurs peut être utilisée avec ``revert`` et ``require`` :

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

    contract VendingMachine {
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
    }

The two syntax options are equivalent, it's developer preference which to use.

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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.6.0;

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
            } catch (bytes memory /*lowLevelData*/) {
                // This is executed in case revert() was used
                // or there was a failing assertion, division
                // by zero, etc. inside getData.
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

Currently, Solidity supports different kinds of catch blocks depending on the
type of error. If the error was caused by ``revert("reasonString")`` or
``require(false, "reasonString")`` (or an internal error that causes such an
exception), then the catch clause
of the type ``catch Error(string memory reason)`` will be executed.

It is planned to support other types of error data in the future.
The string ``Error`` is currently parsed as is and is not treated as an identifier.

The clause ``catch (bytes memory lowLevelData)`` is executed if the error signature
does not match any other clause, there was an error during decoding of the error
message, if there was a failing assertion in the external
call (for example due to a division by zero or a failing ``assert()``) or
if no error data was provided with the exception.
The declared variable provides access to the low-level error data in that case.

If you are not interested in the error data, you can just use
``catch { ... }`` (even as the only catch clause).

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
    The caller always retains 63/64th of the gas in a call and thus
    even if the called contract goes out of gas, the caller still
    has some gas left.