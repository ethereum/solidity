.. index:: ! value type, ! type;value
.. _value-types:

Value Types
===========

The following types are also called value types because variables of these
types will always be passed by value, i.e. they are always copied when they
are used as function arguments or in assignments.

.. index:: ! bool, ! true, ! false

Booleans
--------

``bool``: The possible values are constants ``true`` and ``false``.

Operators:

*  ``!`` (logical negation)
*  ``&&`` (logical conjunction, "and")
*  ``||`` (logical disjunction, "or")
*  ``==`` (equality)
*  ``!=`` (inequality)

The operators ``||`` and ``&&`` apply the common short-circuiting rules. This means that in the expression ``f(x) || g(y)``, if ``f(x)`` evaluates to ``true``, ``g(y)`` will not be evaluated even if it may have side-effects.

.. index:: ! uint, ! int, ! integer
.. _integers:

Entiers
-------

``int`` / ``uint``: Entiers signés et non-signés de différentes tailles. Les mots-clé ``uint8`` à ``uint256`` par pas de ``8`` (entier non signé de 8 à 256 bits) et ``int8`` à ``int256``. ``uint`` et ``int`` sont des alias de ``uint256`` et ``int256``, respectivement.

Opérateurs:

* Comparaisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (retournent un ``bool``)
* Opérateurs binaires: ``&``, ``|``, ``^`` (ou exclusif binaire), ``~`` (négation binaire)
* Opérateurs de décalage: ``<<`` (décalage vers la gauche), ``>>`` (décalage vers la droite)
* Opérateurs arithmétiques: ``+``, ``-``, l' opérateur unaire ``-``, ``*``, ``/``, ``%`` (modulo), ``**`` (exponentiation)

For an integer type ``X``, you can use ``type(X).min`` and ``type(X).max`` to
access the minimum and maximum value representable by the type.

.. warning::

  Integers in Solidity are restricted to a certain range. For example, with ``uint32``, this is ``0`` up to ``2**32 - 1``.
  There are two modes in which arithmetic is performed on these types: The "wrapping" or "unchecked" mode and the "checked" mode.
  By default, arithmetic is always "checked", which mean that if the result of an operation falls outside the value range
  of the type, the call is reverted through a :ref:`failing assertion<assert-and-require>`. You can switch to "unchecked" mode
  using ``unchecked { ... }``. More details can be found in the section about :ref:`unchecked <unchecked>`.

Comparaisons
^^^^^^^^^^^^

La valeur d'une comparaison est celle obtenue en comparant la valeur entière.

Opérations binaires
^^^^^^^^^^^^^^^^^^^

Les opérations binaires sont effectuées sur la représentation du nombre par `complément à deux<https://fr.wikipedia.org/wiki/Compl%C3%A9ment_%C3%A0_deux>`.
Cela signifie que, par exemple, ``~int256(0) == int256(-1)``.

Décalages
^^^^^^^^^

The result of a shift operation has the type of the left operand, truncating the result to match the type.
The right operand must be of unsigned type, trying to shift by a signed type will produce a compilation error.

Shifts can be "simulated" using multiplication by powers of two in the following way. Note that the truncation
to the type of the left operand is always performed at the end, but not mentioned explicitly.

- ``x << y`` is equivalent to the mathematical expression ``x * 2**y``.
- ``x >> y`` is equivalent to the mathematical expression ``x / 2**y``, rounded towards negative infinity.

Décaler d'un nombre négatif de bits déclenche une exception.


.. warning::
    Avant la version ``0.5.0.0``, un décalage vers la droite ``x >> y`` pour un ``x`` négatif était équivalent à ``x / 2**y``, c'est-à-dire que les décalages vers la droite étaient arrondis vers zéro plutôt que vers l'infini négatif.

.. note::
    Overflow checks are never performed for shift operations as they are done for arithmetic operations.
    Instead, the result is always truncated.

Addition, Soustraction et Multiplication
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

L'addition, la soustraction et la multiplication ont la sémantique habituelle., with two different
modes in regard to over- and underflow:

By default, all arithmetic is checked for under- or overflow, but this can be disabled
using the :ref:`unchecked block<unchecked>`, resulting in wrapping arithmetic. More details
can be found in that section.

The expression ``-x`` is equivalent to ``(T(0) - x)`` where
``T`` is the type of ``x``. It can only be applied to signed types.
The value of ``-x`` can be
positive if ``x`` is negative. There is another caveat also resulting
from two's complement representation:
L'expression ``x`` équivaut à ``(T(0) - x)`` où ``T`` est le type de ``x``. Cela signifie que ``-x`` ne sera pas négatif si le type de ``x`` est un type entier non signé. De plus, ``x`` peut être positif si ``x`` est négatif. Il y a une autre mise en garde qui découle également de la représentation en compléments de deux::

    int x = -2**255;
    assert(-x == x);

Cela signifie que même si un nombre est négatif, vous ne pouvez pas supposer que sa négation sera positive.

If you have ``int x = type(int).min;``, then ``-x`` does not fit the positive range.
This means that ``unchecked { assert(-x == x); }`` works, and the expression ``-x``
when used in checked mode will result in a failing assertion.

Division
^^^^^^^^

Puisque le type du résultat d'une opération est toujours le type d'un des opérandes, la division sur les entiers donne toujours un entier.
Dans Solidity, la division s'arrondit vers zéro. Cela signifie que ``int256(-5) / int256(2) == int256(-2)``.

Notez qu'en revanche, la division sur les :ref:`littéraux<literals<rational_literals>` donne des valeurs fractionnaires de précision arbitraire.

.. note::
  Division by zero causes a :ref:`Panic error<assert-and-require>`. This check can **not** be disabled through ``unchecked { ... }``.

.. note::
  The expression ``type(int).min / (-1)`` is the only case where division causes an overflow.
  In checked arithmetic mode, this will cause a failing assertion, while in wrapping
  mode, the value will be ``type(int).min``.

Modulo
^^^^^^

L'opération modulo ``a % n`` donne le reste ``r`` après la division de l'opérande ``a`` par l'opérande ``n``, où ``q = int(a / n)`` et ``r = a - (n * q)``. Cela signifie que modulo donne le même signe que son opérande gauche (ou zéro) et ``a % n == -(abs(a) % n)`` est valable pour un ``a`` négatif:

* ``int256(5) % int256(2) == int256(1)``
* ``int256(5) % int256(-2) == int256(1)``
* ``int256(-5) % int256(2) == int256(-1)``
* ``int256(-5) % int256(-2) == int256(-1)``

.. note::
  Modulo with zero causes a :ref:`Panic error<assert-and-require>`. This check can **not** be disabled through ``unchecked { ... }``.

Exponentiation
^^^^^^^^^^^^^^

l'exponentiation n'est disponible que pour les types signés et donne un résultat du même type. Veillez à ce que les types que vous utilisez soient suffisamment grands pour conserver le résultat et vous préparer à un éventuel effet d'enroulage (wrapping/int overflow).

.. note::
  In checked mode, exponentiation only uses the comparatively cheap ``exp`` opcode for small bases.
  For the cases of ``x**3``, the expression ``x*x*x`` might be cheaper.
  In any case, gas cost tests and the use of the optimizer are advisable.

.. note::
  ``0**0`` est défini par l'EVM comme étant ``1``.

.. index:: ! ufixed, ! fixed, ! fixed point number

Nombre à virgule fixe
---------------------

.. warning::
    Les numéros à point fixe ne sont pas encore entièrement pris en charge par Solidity. Ils peuvent être déclarés, mais ne peuvent pas être affectés à ou de.

``fixed`` / ``ufixed``: Nombre à virgule fixe signés et non-signés de taille variable. Les mots-clés ``ufixedMxN`` et ``fixedMxN``, où ``M`` représente le nombre de bits pris par le type et ``N`` représente combien de décimales sont disponibles. ``M`` doit être divisible par 8 et peut aller de 8 à 256 bits. ``N`` doit être compris entre 0 et 80, inclusivement.
``ufixed`` et ``fixed`` sont des alias pour ``ufixed128x18`` et ``fixed128x18``, respectivement.

Opérateurs:

* Comparaisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (évalue à ``bool``)
* Operateurs arithmétiques: ``+``, ``-``, l'opérateur unaire ``-``, ``*``, ``/``, ``%`` (modulo)

.. note::
    La principale différence entre les nombres à virgule flottante (``float``et ``double`` dans de nombreux langages, plus précisément les nombres IEEE 754) et les nombres à virgule fixe est que le nombre de bits utilisés pour l'entier et la partie fractionnaire (la partie après le point décimal) est flexible dans le premier, alors qu'il est strictement défini dans le second. Généralement, en virgule flottante, presque tout l'espace est utilisé pour représenter le nombre, alors que seul un petit nombre de bits définit où se trouve le point décimal.

.. index:: address, balance, send, call, callcode, delegatecall, staticcall, transfer

.. _address:

Adresses
--------

Le type d'adresse se décline en deux versions, qui sont en grande partie identiques :

 - ``address`` : Contient une valeur de 20 octets (taille d'une adresse Ethereum).
 - ``address payable`` : Même chose que "adresse", mais avec les membres additionnels ``transfert`` et ``envoi``.

L'idée derrière cette distinction est que l'``address payable`` est une adresse à laquelle vous pouvez envoyer de l'éther, alors que vous n'êtes pas censés envoyer d'ethers à une simple ``address``, for example because it might be a smart contract
that was not built to accept Ether.

Conversions de type :

Les conversions implicites de ``address payable`` à ``address`` sont autorisées, tandis que les conversions de ``address`` à ``address payable`` ne sont pas possibles.

Explicit conversions to and from ``address`` are allowed for ``uint160``, integer literals,
``bytes20`` and contract types.

Only expressions of type ``address`` and contract-type can be converted to the type ``address
payable`` via the explicit conversion ``payable(...)``. For contract-type, this conversion is only
allowed if the contract can receive Ether, i.e., the contract either has a :ref:`receive
<receive-ether-function>` or a payable fallback function. Note that ``payable(0)`` is valid and is
an exception to this rule.

.. note::
    If you need a variable of type ``address`` and plan to send Ether to it, then
    declare its type as ``address payable`` to make this requirement visible. Also,
    try to make this distinction or conversion as early as possible.

Opérateurs :

* ``<=``, ``<``, ``==``, ``!=``, ``>=`` and ``>``

.. warning::
    Si vous convertissez un type qui utilise une taille d'octet plus grande en ``address``, par exemple ``bytes32``, alors l'adresse est tronquée.
     Pour réduire l'ambiguïté de conversion à partir de la version 0.4.24 du compilateur vous force à rendre la troncature explicite dans la conversion.
     Prenons par exemple l'adresse ``0x1111222222323333434444545555666666777777778888999999AAAABBBBBBCCDDDDEEFEFFFFFFCC``.

     Vous pouvez utiliser ``address(uint160(octets20(b)))``, ce qui donne ``0x1111212222323333434444545555666677778888889999aAaaa``,
     ou vous pouvez utiliser ``address(uint160(uint256(b)))``, ce qui donne ``0x777777888888999999AaAAbBbbCcccddDdeeeEfFFfCcCcCc``.

.. note::
    La distinction entre ``address``et ``address payable`` a été introduite avec la version 0.5.0.
     À partir de cette version également, les contrats ne dérivent pas du type d'adresse, mais peuvent toujours être convertis explicitement en
     adresse " ou à " adresse payable ", s'ils ont une fonction par défaut payable.

.. _members-of-addresses:

Membres de Address
^^^^^^^^^^^^^^^^^^

Pour une liste des membres de address, voir :ref:`address_related`.

* ``balance`` et ``transfer``.

Il est possible d'interroger le solde d'une adresse en utilisant la propriété ``balance``
et d'envoyer des Ether (en unités de wei) à une adresse payable à l'aide de la fonction ``transfert`` :

.. code-block:: solidity
    :force:

    address payable x = payable(0x123);
    address myAddress = address(this);
    if (x.balance < 10 && myAddress.balance >= 10) x.transfer(10);

La fonction ``transfer`` échoue si le solde du contrat en cours n'est pas suffisant ou si le transfert d'Ether est rejeté par le compte destinataire. La fonction ``transfert`` s'inverse en cas d'échec.

.. note::
    Si ``x`` est une adresse de contrat, son code (plus précisément : sa :ref:`fallback-function`, si présente) sera exécutée avec l'appel ``transfer`` (c'est une caractéristique de l'EVM et ne peut être empêché). Si cette exécution échoue ou s'il n'y a plus de gas, le transfert d'Ether sera annulé et le contrat en cours s'arrêtera avec une exception.

* ``send``

``send`` est la contrepartie de bas niveau du ``transfer``. Si l'exécution échoue, le contrat en cours ne s'arrêtera pas avec une exception, mais ``send`` retournera ``false``.

.. warning::
    Il y a certains dangers à utiliser la fonction ``send`` : Le transfert échoue si la profondeur de la stack atteint 1024 (cela peut toujours être forcé par l'appelant) et il échoue également si le destinataire manque de gas. Donc, afin d'effectuer des transferts d'Ether en toute sécurité, vérifiez toujours la valeur de retour de ``send``, utilisez ``transfer`` ou mieux encore  : utilisez un modèle où le destinataire retire l'argent.

* ``call``, ``delegatecall`` et ``staticcall``

Afin de s'interfacer avec des contrats qui ne respectent pas l'ABI, ou d'obtenir un contrôle plus direct sur l'encodage,
les fonctions ``call``, ``delegatecall`` et ``staticcall`` sont disponibles.
Elles prennent tous pour argument un seul ``bytes memory`` comme entrée et retournent la condition de succès (en tant que ``bool``) et les données (``bytes memory``).
Les fonctions ``abi.encoder``, ``abi.encoderPacked``, ``abi.encoderWithSelector`` et ``abi.encoderWithSignature`` peuvent être utilisées pour coder des données structurées.

Exemple:

.. code-block:: solidity

    bytes memory payload = abi.encodeWithSignature("register(string)", "MyName");
    (bool success, bytes memory returnData) = address(nameReg).call(payload);
    require(success);

.. warning::
    Toutes ces fonctions sont des fonctions de bas niveau et doivent être utilisées avec précaution.
     Plus précisément, tout contrat inconnu peut être malveillant et si vous l'appelez, vous transférez le contrôle à ce contrat qui, à son tour, peut revenir dans votre contrat, donc soyez prêt à modifier les variables de votre état.
     quand l'appel revient. La façon habituelle d'interagir avec d'autres contrats est d'appeler une fonction sur un objet ``contract`` (``x.f()``)..

:: note::
    Les versions précédentes de Solidity permettaient à ces fonctions de recevoir des arguments arbitraires et de traiter différemment un premier argument de type ``bytes4``. Ces cas rares ont été supprimés dans la version 0.5.0.

Il est possible de régler le gas fourni avec le modificateur ``.gas``:

.. code-block:: solidity

    namReg.call.gas(1000000)(abi.encodeWithSignature("register(string)", "MyName"));

De même, la valeur en Ether fournie peut également être contrôlée:

.. code-block:: solidity

    nameReg.call.value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

Enfin, ces modificateurs peuvent être combinés. Leur ordre n'a pas d'importance:

.. code-block:: solidity

    nameReg.call.gas(1000000).value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

De la même manière, la fonction ``delegatecall`` peut être utilisée: la différence est que seul le code de l'adresse donnée est utilisé, tous les autres aspects (stockage, balance,...) sont repris du contrat actuel. Le but de ``delegatecall`` est d'utiliser du code de bibliothèque qui est stocké dans un autre contrat. L'utilisateur doit s'assurer que la disposition du stockage dans les deux contrats est adaptée à l'utilisation de ``delegatecall``.

.. note::
    Avant Homestead, il n'existait qu'une variante limitée appelée ``callcode`` qui ne donnait pas accès aux valeurs originales ``msg.sender`` et ``msg.value``. Cette fonction a été supprimée dans la version 0.5.0.

Depuis Byzantium, ``staticcall`` peut aussi être utilisé. C'est fondamentalement la même chose que ``call``, mais reviendra en arrière si la fonction appelée modifie l'état d'une manière ou d'une autre.

Les trois fonctions ``call``, ``delegatecall``et ``staticcall`` sont des fonctions de très bas niveau et ne devraient être utilisées qu'en *dernier recours* car elles brisent la sécurité de type de Solidity.

L'option ``.gas`` est disponible sur les trois méthodes, tandis que l'option ``value`` n'est supportée que pour ``call``.

.. note::
    It is best to avoid relying on hardcoded gas values in your smart contract code,
    regardless of whether state is read from or written to, as this can have many pitfalls.
    Also, access to gas might change in the future.

* ``code`` and ``codehash``

You can query the deployed code for any smart contract. Use ``.code`` to get the EVM bytecode as a
``bytes memory``, which might be empty. Use ``.codehash`` get the Keccak-256 hash of that code
(as a ``bytes32``). Note that ``addr.codehash`` is cheaper than using ``keccak256(addr.code)``.

.. note::
    Tous les contrats pouvant être convertis en type ``address``, il est possible d'interroger le solde du contrat en cours en utilisant ``address(this).balance``.

.. index:: ! contract type, ! type; contract

.. _contract_types:

Types Contrat
-------------

Chaque :ref:`contrat<contracts>` définit son propre type.
Vous pouvez implicitement convertir des contrats en contrats dont ils héritent.
Les contrats peuvent être explicitement convertis de et vers tous les autres types de contrats et le type ``address``.

La conversion explicite vers et depuis le type ``address payable`` n'est possible que si le type de contrat dispose d'une fonction de repli payante.
La conversion est toujours effectuée en utilisant ``address(x)`` et non ``address payable(x)``. Vous trouverez plus d'informations dans la section sur le :ref:`type address<address>`.

.. note::
     Avant la version 0.5.0, les contrats dérivaient directement du type address et il n'y avait aucune distinction entre ``address`` et ``address payable``.

Si vous déclarez une variable locale de type contrat (`MonContrat c`), vous pouvez appeler des fonctions sur ce contrat. Prenez bien soin de l'assigner à un contrat d'un type correspondant.

Vous pouvez également instancier les contrats (ce qui signifie qu'ils sont nouvellement créés). Vous trouverez plus de détails dans la section :ref:`'contrats de création'<contrats de création>`.

La représentation des données d'un contrat est identique à celle du type ``address`` et ce type est également utilisé dans l':ref:`ABI<ABI>`.

Les contrats ne supportent aucun opérateur.

Les membres du type contrat sont les fonctions externes du contrat, y compris les variables d'état publiques.

For a contract ``C`` you can use ``type(C)`` to access
:ref:`type information<meta-type>` about the contract.

.. index:: byte array, bytes32

Tableaux d'octets de taille fixe
--------------------------------

Les types valeur ``bytes1``, ``bytes2``, ``bytes3``, ..., ``bytes32`` contiennent une séquence de 1 à 32 octets.

Opérateurs:

* Comparaisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (retournent un ``bool``)
* Opérateurs binaires: ``&``, ``|``, ``^`` (ou exclusif binaire), ``~`` (négation binaire)
* Opérateurs de décalage: ``<<`` (décalage vers la gauche), ``>>`` (décalage vers la droite)
* Accès par indexage: Si ``x`` estd e type ``bytesI``, alors ``x[k]`` pour ``0 <= k < I`` retourne le ``k`` ème byte (lecture seule).

L'opérateur de décalage travaille avec n'importe quel type d'entier comme opérande droite (mais retourne le type de l'opérande gauche), qui indique le nombre de bits à décaler.
Le décalage d'un montant négatif entraîne une erreur de compilation.

Membres :

*``.length``` donne la longueur fixe du tableau d'octets (lecture seule).

.. note::
    Le type ``byte[]`` est un tableau d'octets, mais en raison des règles de marges, il gaspille 31 octets d'espace pour chaque élément (sauf en storage). Il est préférable d'utiliser le type "bytes" à la place.

.. note::
    Prior to version 0.8.0, ``byte`` used to be an alias for ``bytes1``.

Tableaux dynamiques d'octets
----------------------------

``bytes``:
    Tableau d'octets de taille dynamique, voir :ref:`arrays`. Ce n'est pas un type valeur !
``string``:
    Chaîne codée UTF-8 de taille dynamique, voir :ref:`arrays`. Ce n'est pas un type valeur !
.. index:: address, literal;address

.. _address_literals:

Adresses Littérales
-------------------

Les caractères hexadécimaux qui réussissent un test de somme de contrôle d'adresse ("address checksum"), par exemple ``0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF`` sont de type ``address``.
Les nombres hexadécimaux qui ont entre 39 et 41 chiffres et qui ne passent pas le test de somme de contrôle produisent une erreur. You can prepend (for integer types) or append (for bytesNN types) zeros to remove the error.

.. note::
    Le format de some de contrôle multi-casse est décrit dans `EIP-55 <https://github.com/ethereum/EIPs/blob/master/EIPS/eip-55.md>`_.


.. index:: literal, literal;rational

.. _rational_literals:

Rationels et entiers littéraux
------------------------------

Les nombres entiers littéraux sont formés à partir d'une séquence de nombres compris entre 0 et 9 interprétés en décimal. Par exemple, ``69`` signifie soixante-neuf.
Les littéraux octaux n'existent pas dans Solidity et les zéros précédant un nombre sont invalides.

Les fractions décimales sont formées par un ``.`` avec au moins un chiffre sur un côté. Exemples : ``1.1``, ``.1 `` et ``1.3``.

La notation scientifique est également supportée, où la base peut avoir des fractions, alors que l'exposant ne le peut pas.
The literal ``MeE`` is equivalent to ``M * 10**E``.
Exemples : ``2e10``, ``-2e10``, ``2e-10``, ``2e-10``, ``2.5e1``.

Les soulignements (underscore) peuvent être utilisés pour séparer les chiffres d'un nombre littéral numérique afin d'en faciliter la lecture.
Par exemple, la décimale ``123_000``, l'hexadécimale ``0x2eff_abde``, la notation décimale scientifique ``1_2e345_678`` sont toutes valables.
Les tirets de soulignement ne sont autorisés qu'entre deux chiffres et un seul tiret de soulignement consécutif est autorisé.
Il n'y a pas de signification sémantique supplémentaire ajoutée à un nombre contenant des tirets de soulignement, les tirets de soulignement sont ignorés.

Les expressions littérales numériques conservent une précision arbitraire jusqu'à ce qu'elles soient converties en un type non littéral (c'est-à-dire en les utilisant avec une expression non littérale ou par une conversion explicite).
Cela signifie que les calculs ne débordent pas (overflow) et que les divisions ne tronquent pas les expressions littérales des nombres.

Par exemple, ``(2**800 + 1) - 2**800`` produit la constante ``1`` (de type ``uint8``) bien que les résultats intermédiaires ne rentrent même pas dans la taille d'un mot machine. De plus, ``.5 * 8`` donne l'entier ``4`` (bien que des nombres non entiers aient été utilisés entre les deux).

N'importe quel opérateur qui peut être appliqué aux nombres entiers peut également être appliqué aux expressions littérales des nombres tant que les opérandes sont des nombres entiers. Si l'un des deux est fractionnaire, les opérations sur bits sont interdites et l'exponentiation est interdite si l'exposant est fractionnaire (parce que cela pourrait résulter en un nombre non rationnel).

Shifts and exponentiation with literal numbers as left (or base) operand and integer types
as the right (exponent) operand are always performed
in the ``uint256`` (for non-negative literals) or ``int256`` (for a negative literals) type,
regardless of the type of the right (exponent) operand.

.. warning::
    La dvision d'entiers littéraux tronquait dans les versions de Solidity avant la version 0.4.0, mais elle donne maintenant en un nombre rationnel, c'est-à-dire que ``5 / 2`` n'est pas égal à ``2``, mais à ``2.5``.

.. note::
    Solidity a un type de nombre littéral pour chaque nombre rationnel.
     Les nombres entiers littéraux et les nombres rationnels appartiennent à des types de nombres littéraux.
     De plus, toutes les expressions numériques littérales (c'est-à-dire les expressions qui ne contiennent que des nombres et des opérateurs) appartiennent à des types littéraux de nombres. Ainsi, les expressions littérales ``1 + 2`` et ``2 + 1`` appartiennent toutes deux au même type littéral de nombre pour le nombre rationnel numéro trois.

.. note::
    Les expressions littérales numériques sont converties en caractères non littéraux dès qu'elles sont utilisées avec des expressions non littérales. Indépendamment des types, la valeur de l'expression assignée à ``b`` ci-dessous est évaluée en entier. Comme ``a`` est de type ``uint128``, l'expression ``2,5 + a`` doit cependant avoir un type. Puisqu'il n'y a pas de type commun pour les types ``2.5`` et ``uint128``, le compilateur Solidity n'accepte pas ce code.

.. code-block:: solidity

    uint128 a = 1;
    uint128 b = 2.5 + a + 0.5;

.. index:: literal, literal;string, string
.. _string_literals:

Chaines de caractères littérales
--------------------------------

Les chaînes de caractères littérales sont écrites avec des guillemets simples ou doubles (``"foo"`` ou ``'bar'``). Elles n'impliquent pas de zéro final comme en C ; ``foo`` représente trois octets, pas quatre. Comme pour les entiers littéraux, leur type peut varier, mais ils sont implicitement convertibles en ``bytes1``, ..., ``bytes32``, ou s'ils conviennent, en ``bytes`` et en ``string``.

String literals can only contain printable ASCII characters, which means the characters between and including 0x20 .. 0x7E.

Les chaînes de caractères littérales supportent les caractères d'échappement suivants :

 - ``\<newline>`` (échappe un réel caractère newline)
 - ``\\`` (barre oblique)
 - ``\'`` (guillemet simple)
 - ``\"`` (guillemet double)
 - ``\n`` (newline)
 - ``\r`` (carriage return)
 - ``\t`` (tabulation horizontale)
 - ``\xNN`` (hex escape, see below)
 - ``\uNNNN`` (echapement d'unicode, voir ci-dessous)

``\xNN`` prend une valeur hexadécimale et insère l'octet approprié, tandis que ``\uNNNNN`` prend un codepoint Unicode et insère une séquence UTF-8.

.. note::

    Until version 0.8.0 there were three additional escape sequences: ``\b``, ``\f`` and ``\v``.
    They are commonly available in other languages but rarely needed in practice.
    If you do need them, they can still be inserted via hexadecimal escapes, i.e. ``\x08``, ``\x0c``
    and ``\x0b``, respectively, just as any other ASCII character.

La chaîne de caractères de l'exemple suivant a une longueur de dix octets.
Elle commence par un octet de newline, suivi d'une guillemet double, d'une guillemet simple, d'un caractère barre oblique inversée et ensuite (sans séparateur) de la séquence de caractères ``abcdef``.

.. code-block:: solidity
    :force:

    "\n\"\'\\abc\
    def"

Tout terminateur de ligne unicode qui n'est pas une nouvelle ligne (i.e. LF, VF, FF, CR, NEL, LS, PS) est considéré comme terminant la chaîne littérale. Newline ne termine la chaîne littérale que si elle n'est pas précédée d'un ``\``.

Unicode Literals
----------------

While regular string literals can only contain ASCII, Unicode literals – prefixed with the keyword ``unicode`` – can contain any valid UTF-8 sequence.
They also support the very same escape sequences as regular string literals.

.. code-block:: solidity

    string memory a = unicode"Hello 😃";

.. index:: literal, bytes

Hexadécimaux littéraux
----------------------

Les caractères hexadécimaux sont précédées du mot-clé ``hex`` et sont entourées de guillemets simples ou doubles (``hex"001122FF"``). Leur contenu doit être une chaîne hexadécimale et leur valeur sera la représentation binaire de ces valeurs.

Les littéraux hexadécimaux se comportent comme :ref:`chaînes de caractères littérales<string_literals>` et ont les mêmes restrictions de convertibilité.

.. index:: enum

.. _enums:

Énumérateurs
------------

Les ``enum`` sont une façon de créer un type défini par l'utilisateur en Solidity. Ils sont explicitement convertibles de et vers tous les types d'entiers mais la conversion implicite n'est pas autorisée.
La conversion explicite à partir d'un nombre entier vérifie au moment de l'exécution que la valeur se trouve à l'intérieur de la plage de l'enum et provoque une Panic Error autrement.
Un enum a besoin d'au moins un membre, and its default value when declared is the first member.
Enums cannot have more than 256 members.

La représentation des données est la même que pour les énumérations en C : Les options sont représentées par des valeurs entières non signées à partir de ``0``.

Using ``type(NameOfEnum).min`` and ``type(NameOfEnum).max`` you can get the
smallest and respectively largest value of the given enum.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.8;

    contract test {
        enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
        ActionChoices choice;
        ActionChoices constant defaultChoice = ActionChoices.GoStraight;

        function setGoStraight() public {
            choice = ActionChoices.GoStraight;
        }

        // Since enum types are not part of the ABI, the signature of "getChoice"
        // will automatically be changed to "getChoice() returns (uint8)"
        // for all matters external to Solidity.
        function getChoice() public view returns (ActionChoices) {
            return choice;
        }

        function getDefaultChoice() public pure returns (uint) {
            return uint(defaultChoice);
        }

        function getLargestValue() public pure returns (ActionChoices) {
            return type(ActionChoices).max;
        }

        function getSmallestValue() public pure returns (ActionChoices) {
            return type(ActionChoices).min;
        }
    }

.. note::
    Enums can also be declared on the file level, outside of contract or library definitions.

.. index:: ! user defined value type, custom type

.. _user-defined-value-types:

User Defined Value Types
------------------------

A user defined value type allows creating a zero cost abstraction over an elementary value type.
This is similar to an alias, but with stricter type requirements.

A user defined value type is defined using ``type C is V``, where ``C`` is the name of the newly
introduced type and ``V`` has to be a built-in value type (the "underlying type"). The function
``C.wrap`` is used to convert from the underlying type to the custom type. Similarly, the
function ``C.unwrap`` is used to convert from the custom type to the underlying type.

The type ``C`` does not have any operators or bound member functions. In particular, even the
operator ``==`` is not defined. Explicit and implicit conversions to and from other types are
disallowed.

The data-representation of values of such types are inherited from the underlying type
and the underlying type is also used in the ABI.

The following example illustrates a custom type ``UFixed256x18`` representing a decimal fixed point
type with 18 decimals and a minimal library to do arithmetic operations on the type.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.8;

    // Represent a 18 decimal, 256 bit wide fixed point type using a user defined value type.
    type UFixed256x18 is uint256;

    /// A minimal library to do fixed point operations on UFixed256x18.
    library FixedMath {
        uint constant multiplier = 10**18;

        /// Adds two UFixed256x18 numbers. Reverts on overflow, relying on checked
        /// arithmetic on uint256.
        function add(UFixed256x18 a, UFixed256x18 b) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(UFixed256x18.unwrap(a) + UFixed256x18.unwrap(b));
        }
        /// Multiplies UFixed256x18 and uint256. Reverts on overflow, relying on checked
        /// arithmetic on uint256.
        function mul(UFixed256x18 a, uint256 b) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(UFixed256x18.unwrap(a) * b);
        }
        /// Take the floor of a UFixed256x18 number.
        /// @return the largest integer that does not exceed `a`.
        function floor(UFixed256x18 a) internal pure returns (uint256) {
            return UFixed256x18.unwrap(a) / multiplier;
        }
        /// Turns a uint256 into a UFixed256x18 of the same value.
        /// Reverts if the integer is too large.
        function toUFixed256x18(uint256 a) internal pure returns (UFixed256x18) {
            return UFixed256x18.wrap(a * multiplier);
        }
    }

Notice how ``UFixed256x18.wrap`` and ``FixedMath.toUFixed256x18`` have the same signature but
perform two very different operations: The ``UFixed256x18.wrap`` function returns a ``UFixed256x18``
that has the same data representation as the input, whereas ``toUFixed256x18`` returns a
``UFixed256x18`` that has the same numerical value.

.. index:: ! function type, ! type; function

.. _function_types:

Types Fonction
--------------

Les types fonction sont les types des fonctions. Les variables du type fonction peuvent être passés et retournés pour transférer les fonctions vers et renvoyer les fonctions des appels de fonction.
Les types de fonctions se déclinent en deux versions : les fonctions *internes* ``internal`` et les fonctions *externes* ``external`` :

Les fonctions internes ne peuvent être appelées qu'à l'intérieur du contrat en cours (plus précisément, à l'intérieur de l'unité de code en cours, qui comprend également les fonctions de bibliothèque internes et les fonctions héritées) car elles ne peuvent pas être exécutées en dehors du contexte du contrat actuel. L'appel d'une fonction interne est réalisé en sautant à son label d'entrée, tout comme lors de l'appel interne d'une fonction du contrat en cours.

Les fonctions externes se composent d'une adresse et d'une signature de fonction et peuvent être transférées et renvoyées à partir des appels de fonction externes.

Les types de fonctions sont notés comme suit:

.. code-block:: solidity
    :force:

     fonction (<types de paramètres>) {internal|external} {pure|view|payable][returns (<types de retour>)]

En contraste avec types de paramètres, les types de retour ne peuvent pas être vides - si le type de fonction ne retourne rien, toute la partie ``returns (<types de retour>)``doit être omise.

Par défaut, les fonctions sont de type ``internal``, donc le mot-clé ``internal`` peut être omis. Notez que ceci ne s'applique qu'aux types de fonctions. La visibilité doit être spécifiée explicitement car les fonctions définies dans les contrats n'ont pas de valeur par défaut.

Conversions :

A function type ``A`` is implicitly convertible to a function type ``B`` if and only if
their parameter types are identical, their return types are identical,
their internal/external property is identical and the state mutability of ``A``
is more restrictive than the state mutability of ``B``. In particular:

- ``pure`` functions can be converted to ``view`` and ``non-payable`` functions
- ``view`` functions can be converted to ``non-payable`` functions
- ``payable`` functions can be converted to ``non-payable`` functions

 - Les fonctions ``pure`` peuvent être converties en fonctions ``view`` et ``non-payable``.
 - Les fonctions ``view`` peuvent être converties en fonctions ``non-payable``.
 - les fonctions ``payable`` peuvent être converties en fonctions ``non-payable``.

Aucune autre conversion entre les types de fonction n'est possible.

If a function type variable is not initialised, calling it results
in a :ref:`Panic error<assert-and-require>`. The same happens if you call a function after using ``delete``
on it.

Si une variable de type fonction n'est pas initialisée, l'appel de celle-ci entraîne l'échec d'une assertion. Il en va de même si vous appelez une fonction après avoir utilisé ``delete`` dessus.

Si des fonctions de type ``external`` sont appelées d'en dehors du contexte de Solidity, ils sont traités comme le type ``function``, qui code l'adresse suivie de l'identificateur de fonction ensemble dans un seul type ``bytes24``.

Notez que les fonctions publiques du contrat actuel peuvent être utilisées à la fois comme une fonction interne et comme une fonction externe. Pour utiliser ``f`` comme fonction interne, utilisez simplement ``f``, si vous voulez utiliser sa forme externe, utilisez ``this.f```.

A function of an internal type can be assigned to a variable of an internal function type regardless
of where it is defined.
This includes private, internal and public functions of both contracts and libraries as well as free
functions.
External function types, on the other hand, are only compatible with public and external contract
functions.
Libraries are excluded because they require a ``delegatecall`` and use :ref:`a different ABI
convention for their selectors <library-selectors>`.
Functions declared in interfaces do not have definitions so pointing at them does not make sense either.

Members:

External (or public) functions have the following members:

* ``.address`` returns the address of the contract of the function.
* ``.selector`` returns the :ref:`ABI function selector <abi_function_selector>`

.. note::
  External (or public) functions used to have the additional members
  ``.gas(uint)`` and ``.value(uint)``. These were deprecated in Solidity 0.6.2
  and removed in Solidity 0.7.0. Instead use ``{gas: ...}`` and ``{value: ...}``
  to specify the amount of gas or the amount of wei sent to a function,
  respectively. See :ref:`External Function Calls <external-function-calls>` for
  more information.

Example that shows how to use the members:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.4 <0.9.0;

    contract Example {
        function f() public payable returns (bytes4) {
            assert(this.f.address == address(this));
            return this.f.selector;
        }

        function g() public {
            this.f{gas: 10, value: 800}();
        }
    }

Exemple d'utilisation des fonctions de type ``internal``:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    library ArrayUtils {
      // les fonctions internes peuvent être utilisées dams des fonctions
      // de librairies internes car elles partagent le même contexte
        function map(uint[] memory self, function (uint) pure returns (uint) f)
            internal
            pure
            returns (uint[] memory r)
        {
            r = new uint[](self.length);
            for (uint i = 0; i < self.length; i++) {
                r[i] = f(self[i]);
            }
        }

        function reduce(
            uint[] memory self,
            function (uint, uint) pure returns (uint) f
        )
            internal
            pure
            returns (uint r)
        {
            r = self[0];
            for (uint i = 1; i < self.length; i++) {
                r = f(r, self[i]);
            }
        }

        function range(uint length) internal pure returns (uint[] memory r) {
            r = new uint[](length);
            for (uint i = 0; i < r.length; i++) {
                r[i] = i;
            }
        }
    }


    contract Pyramid {
        using ArrayUtils for *;

        function pyramid(uint l) public pure returns (uint) {
            return ArrayUtils.range(l).map(square).reduce(sum);
        }

        function square(uint x) internal pure returns (uint) {
            return x * x;
        }

        function sum(uint x, uint y) internal pure returns (uint) {
            return x + y;
        }
    }

Exemple d' usage de fonction ``external``:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;


    contract Oracle {
        struct Request {
            bytes data;
            function(uint) external callback;
        }

        Request[] private requests;
        event NewRequest(uint);

        function query(bytes memory data, function(uint) external callback) public {
            requests.push(Request(data, callback));
            emit NewRequest(requests.length - 1);
        }

        function reply(uint requestID, uint response) public {
            // Here goes the check that the reply comes from a trusted source
            requests[requestID].callback(response);
        }
    }


    contract OracleUser {
        Oracle constant private ORACLE_CONST = Oracle(address(0x00000000219ab540356cBB839Cbe05303d7705Fa)); // known contract
        uint private exchangeRate;

        function buySomething() public {
            ORACLE_CONST.query("USD", this.oracleResponse);
        }

        function oracleResponse(uint response) public {
            require(
                msg.sender == address(ORACLE_CONST),
                "Only oracle can call this."
            );
            exchangeRate = response;
        }
    }
    
.. note::
    Les fonctions lambda ou en in-line sont prévues mais pas encore prises en charge.
