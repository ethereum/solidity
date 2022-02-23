.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

***********
Héritage
***********

Solidity supporte l'héritage multiple en copiant du code, incluant le polymorphisme.

Polymorphism means that a function call (internal and external)
always executes the function of the same name (and parameter types)
in the most derived contract in the inheritance hierarchy.
This has to be explicitly enabled on each function in the
hierarchy using the ``virtual`` and ``override`` keywords.
See :ref:`Function Overriding <function-overriding>` for more details.

It is possible to call functions further up in the inheritance
hierarchy internally by explicitly specifying the contract
using ``ContractName.functionName()`` or using ``super.functionName()``
if you want to call the function one level higher up in
the flattened inheritance hierarchy (see below).


Lorsqu'un contrat hérite d'autres contrats, un seul contrat
est créé dans la blockchain et le code de tous les contrats de base
est copié dans le contrat créé. This means that all internal calls
to functions of base contracts also just use internal function calls
(``super.f(..)`` will use JUMP and not a message call).

State variable shadowing is considered as an error.  A derived contract can
only declare a state variable ``x``, if there is no visible state variable
with the same name in any of its bases.

Le système général d'héritage est très similaire à celui de `Python <https://docs.python.org/3/tutorial/classes.html#inheritance>`_,
surtout en ce qui concerne l'héritage multiple, mais il y a aussi quelques :ref:`differences <multi-inheritance>`.

Les détails sont donnés dans l'exemple suivant.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;


    contract Owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }


    // Utilisez `is` pour dériver d'un autre contrat. Les contrats dérivés peuvent accéder à tous les membres non privés, y compris les fonctions internes et les variables d'état. Il n'est cependant pas possible d'y accéder de l'extérieur via `this`.
    contract Destructible is Owned {
        // The keyword `virtual` means that the function can change
        // its behaviour in derived classes ("overriding").
        function destroy() virtual public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }


    // Ces contrats abstraits ne sont fournis que pour faire connaître l'interface au compilateur. Notez la fonction sans corps. Si un contrat n'implémente pas toutes les fonctions, il ne peut être utilisé que comme interface.
    abstract contract Config {
        function lookup(uint id) public virtual returns (address adr);
    }


    abstract contract NameReg {
        function register(bytes32 name) public virtual;
        function unregister() public virtual;
    }


    // L'héritage multiple est possible. Notez que `owned` est aussi une classe de base de `mortal`, pourtant il n'y a qu'une seule instance de `owned` (comme pour l'héritage virtuel en C++).
    contract Named is Owned, Destructible {
        constructor(bytes32 name) {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // Les fonctions peuvent être remplacées par une autre fonction ayant le même nom et le même nombre/type d'entrées.  Si la fonction de surcharge a différents types de paramètres de sortie, cela provoque une erreur.
        // Les appels de fonction locaux et les appels de fonction basés sur la messagerie tiennent compte de ces dérogations.
        // If you want the function to override, you need to use the
        // `override` keyword. You need to specify the `virtual` keyword again
        // if you want this function to be overridden again.
        function destroy() public virtual override {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
                NameReg(config.lookup(1)).unregister();
                // It is still possible to call a specific
                // overridden function.
                Destructible.destroy();
            }
        }
    }


    // Si un constructeur prend un argument, il doit être fourni dans l'en-tête (ou dans le constructeur du contrat dérivé (voir ci-dessous)).
    contract PriceFeed is Owned, Destructible, Named("GoldFeed") {
        function updateInfo(uint newInfo) public {
            if (msg.sender == owner) info = newInfo;
        }

        // Here, we only specify `override` and not `virtual`.
        // This means that contracts deriving from `PriceFeed`
        // cannot change the behaviour of `destroy` anymore.
        function destroy() public override(Destructible, Named) { Named.destroy(); }
        function get() public view returns(uint r) { return info; }

        uint info;
    }

Notez que ci-dessus, nous appelons ``Destructible.destroy()`` pour "transmettre" la demande de destruction. La façon dont cela est fait est problématique, comme vu dans l'exemple suivant:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }

    contract Destructible is owned {
        function destroy() public virtual {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is Destructible {
        function destroy() public virtual override { /* do cleanup 1 */ Destructible.destroy(); }
    }

    contract Base2 is Destructible {
        function destroy() public virtual override { /* do cleanup 2 */ Destructible.destroy(); }
    }

    contract Final is Base1, Base2 {
        function destroy() public override(Base1, Base2) { Base2.destroy(); }
    }

Un appel à ``Final.destroy()`` appellera ``Base2.destroy`` puisque nous le demandons explicitement dans l'override, mais cet appel évitera
``Base1.destroy``. La solution à ce problème est d'utiliser ``super``:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract owned {
        constructor() { owner = payable(msg.sender); }
        address payable owner;
    }

    contract Destructible is owned {
        function destroy() virtual public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is Destructible {
        function destroy() public virtual override { /* do cleanup 1 */ super.destroy(); }
    }


    contract Base2 is Destructible {
        function destroy() public virtual override { /* do cleanup 2 */ super.destroy(); }
    }

    contract Final is Base1, Base2 {
        function destroy() public override(Base1, Base2) { super.destroy(); }
    }

Si ``Base2`` appelle une fonction de ``super``, elle n'appelle pas simplement cette fonction sur un de ses contrats de base.  Elle appelle plutôt cette fonction sur le prochain contrat de base dans le graph d'héritage final, donc elle appellera ``Base1.destroy()`` (notez que la séquence d'héritage finale est -- en commençant par le contrat le plus dérivé : Final, Base2, Base1, Destructible, owned).
La fonction réelle qui est appelée lors de l'utilisation de super n'est pas connue dans le contexte de la classe où elle est utilisée, bien que son type soit connu. Il en va de même pour la recherche de méthodes virtuelles ordinaires.

.. index:: ! overriding;function

.. _function-overriding:

Function Overriding
===================

Base functions can be overridden by inheriting contracts to change their
behavior if they are marked as ``virtual``. The overriding function must then
use the ``override`` keyword in the function header.
The overriding function may only change the visibility of the overridden function from ``external`` to ``public``.
The mutability may be changed to a more strict one following the order:
``nonpayable`` can be overridden by ``view`` and ``pure``. ``view`` can be overridden by ``pure``.
``payable`` is an exception and cannot be changed to any other mutability.

The following example demonstrates changing mutability and visibility:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base
    {
        function foo() virtual external view {}
    }

    contract Middle is Base {}

    contract Inherited is Middle
    {
        function foo() override public pure {}
    }

For multiple inheritance, the most derived base contracts that define the same
function must be specified explicitly after the ``override`` keyword.
In other words, you have to specify all base contracts that define the same function
and have not yet been overridden by another base contract (on some path through the inheritance graph).
Additionally, if a contract inherits the same function from multiple (unrelated)
bases, it has to explicitly override it:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base1
    {
        function foo() virtual public {}
    }

    contract Base2
    {
        function foo() virtual public {}
    }

    contract Inherited is Base1, Base2
    {
        // Derives from multiple bases defining foo(), so we must explicitly
        // override it
        function foo() public override(Base1, Base2) {}
    }

An explicit override specifier is not required if
the function is defined in a common base contract
or if there is a unique function in a common base contract
that already overrides all other functions.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract A { function f() public pure{} }
    contract B is A {}
    contract C is A {}
    // No explicit override required
    contract D is B, C {}

More formally, it is not required to override a function (directly or
indirectly) inherited from multiple bases if there is a base contract
that is part of all override paths for the signature, and (1) that
base implements the function and no paths from the current contract
to the base mentions a function with that signature or (2) that base
does not implement the function and there is at most one mention of
the function in all paths from the current contract to that base.

In this sense, an override path for a signature is a path through
the inheritance graph that starts at the contract under consideration
and ends at a contract mentioning a function with that signature
that does not override.

If you do not mark a function that overrides as ``virtual``, derived
contracts can no longer change the behaviour of that function.

.. note::

  Functions with the ``private`` visibility cannot be ``virtual``.

.. note::

  Functions without implementation have to be marked ``virtual``
  outside of interfaces. In interfaces, all functions are
  automatically considered ``virtual``.

.. note::

  Starting from Solidity 0.8.8, the ``override`` keyword is not
  required when overriding an interface function, except for the
  case where the function is defined in multiple bases.


Public state variables can override external functions if the
parameter and return types of the function matches the getter function
of the variable:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract A
    {
        function f() external view virtual returns(uint) { return 5; }
    }

    contract B is A
    {
        uint public override f;
    }

.. note::

  While public state variables can override external functions, they themselves cannot
  be overridden.

.. index:: ! overriding;modifier

.. _modifier-overriding:

Modifier Overriding
===================

Function modifiers can override each other. This works in the same way as
:ref:`function overriding <function-overriding>` (except that there is no overloading for modifiers). The
``virtual`` keyword must be used on the overridden modifier
and the ``override`` keyword must be used in the overriding modifier:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base
    {
        modifier foo() virtual {_;}
    }

    contract Inherited is Base
    {
        modifier foo() override {_;}
    }


In case of multiple inheritance, all direct base contracts must be specified
explicitly:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract Base1
    {
        modifier foo() virtual {_;}
    }

    contract Base2
    {
        modifier foo() virtual {_;}
    }

    contract Inherited is Base1, Base2
    {
        modifier foo() override(Base1, Base2) {_;}
    }



.. index:: ! constructor

.. _constructor:

Constructeurs
============

Un constructeur est une fonction optionnelle déclarée avec le mot-clé ``constructeur`` qui est exécuté lors de la création du contrat, et où vous pouvez exécuter le code d'initialisation du contrat.

Avant l'exécution du code constructeur, les variables d'état sont initialisées à leur valeur spécifiée si vous les initialisez en ligne, ou leur :ref:`default value<default-value>` si vous ne le faites pas.

Après l'exécution du constructeur, le code final du contrat est déployé dans la chaîne de blocs. Le déploiement du code coûte du gas supplémentaire linéairement à la longueur du code.
Ce code inclut toutes les fonctions qui font partie de l'interface publique et toutes les fonctions qui sont accessibles à partir de là par des appels de fonctions.
Il n'inclut pas le code constructeur ni les fonctions internes qui ne sont appelées que par le constructeur.

S'il n'y a pas de constructeur, le contrat assumera le constructeur par défaut, ce qui est équivalent à ``constructor() {}``. Par exemple :

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    abstract contract A {
        uint public a;

        constructor(uint _a) {
            a = _a;
        }
    }

    contract B is A(1) {
        constructor() {}
    }

You can use internal parameters in a constructor (for example storage pointers). In this case,
the contract has to be marked :ref:`abstract <abstract-contract>`, because these parameters
cannot be assigned valid values from outside but only through the constructors of derived contracts.

.. attention ::
    Avant 0.4.22, ont été définis comme des fonctions portant le même nom que le contrat.
    Cette syntaxe a été dépréciée et n'est plus autorisée dans la version 0.5.0.

.. warning ::
    Prior to version 0.7.0, you had to specify the visibility of constructors as either
    ``internal`` or ``public``.


.. index:: ! base;constructor

Arguments des Constructeurs de Base
===============================

Les constructeurs de tous les contrats de base seront appelés selon les règles de linéarisation expliquées ci-dessous.
Si les constructeurs de base ont des arguments, les contrats dérivés doivent les spécifier tous. Cela peut se faire de deux façons:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base {
        uint x;
        constructor(uint _x) { x = _x; }
    }

    // Either directly specify in the inheritance list...
    contract Derived1 is Base(7) {
        constructor() {}
    }

    // or through a "modifier" of the derived constructor.
    contract Derived2 is Base {
        constructor(uint _y) Base(_y * _y) {}
    }

One way is directly in the inheritance list (``is Base(7)``).  The other is in
the way a modifier is invoked as part of
the derived constructor (``Base(_y * _y)``). The first way to
do it is more convenient if the constructor argument is a
constant and defines the behaviour of the contract or
describes it. The second way has to be used if the
constructor arguments of the base depend on those of the
derived contract. Arguments have to be given either in the
inheritance list or in modifier-style in the derived constructor.
Specifying arguments in both places is an error.

If a derived contract does not specify the arguments to all of its base
contracts' constructors, it will be abstract.

.. index:: ! inheritance;multiple, ! linearization, ! C3 linearization

.. _multi-inheritance:

Multiple Inheritance and Linearization
======================================

Languages that allow multiple inheritance have to deal with
several problems.  One is the `Diamond Problem <https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem>`_.
Solidity is similar to Python in that it uses "`C3 Linearization <https://en.wikipedia.org/wiki/C3_linearization>`_"
to force a specific order in the directed acyclic graph (DAG) of base classes. This
results in the desirable property of monotonicity but
disallows some inheritance graphs. Especially, the order in
which the base classes are given in the ``is`` directive is
important: You have to list the direct base contracts
in the order from "most base-like" to "most derived".
Note that this order is the reverse of the one used in Python.

Another simplifying way to explain this is that when a function is called that
is defined multiple times in different contracts, the given bases
are searched from right to left (left to right in Python) in a depth-first manner,
stopping at the first match. If a base contract has already been searched, it is skipped.

In the following code, Solidity will give the
error "Linearization of inheritance graph impossible".

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract X {}
    contract A is X {}
    // This will not compile
    contract C is A, X {}

The reason for this is that ``C`` requests ``X`` to override ``A``
(by specifying ``A, X`` in this order), but ``A`` itself
requests to override ``X``, which is a contradiction that
cannot be resolved.

Due to the fact that you have to explicitly override a function
that is inherited from multiple bases without a unique override,
C3 linearization is not too important in practice.

One area where inheritance linearization is especially important and perhaps not as clear is when there are multiple constructors in the inheritance hierarchy. The constructors will always be executed in the linearized order, regardless of the order in which their arguments are provided in the inheriting contract's constructor.  For example:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract Base1 {
        constructor() {}
    }

    contract Base2 {
        constructor() {}
    }

    // Constructors are executed in the following order:
    //  1 - Base1
    //  2 - Base2
    //  3 - Derived1
    contract Derived1 is Base1, Base2 {
        constructor() Base1() Base2() {}
    }

    // Constructors are executed in the following order:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived2
    contract Derived2 is Base2, Base1 {
        constructor() Base2() Base1() {}
    }

    // Constructors are still executed in the following order:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived3
    contract Derived3 is Base2, Base1 {
        constructor() Base1() Base2() {}
    }


Inheriting Different Kinds of Members of the Same Name
======================================================

It is an error when any of the following pairs in a contract have the same name due to inheritance:
  - a function and a modifier
  - a function and an event
  - an event and a modifier

As an exception, a state variable getter can override an external function.
