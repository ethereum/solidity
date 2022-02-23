.. index:: ! type;conversion, ! cast

.. _types-conversion-elementary-types:

Conversions entre les types élémentaires
========================================

Conversions implicites
----------------------

An implicit type conversion is automatically applied by the compiler in some cases
during assignments, when passing arguments to functions and when applying operators.
En général, une conversion implicite entre les types valeur est possible si elle a un sens sémantique et qu'aucune information n'est perdue.

Par exemple, ``uint8`` est convertible en ``uint16`` et ``int128`` en ``int256``, mais ``uint8`` n'est pas convertible en ``uint256`` (car ``uint256`` ne peut contenir, par exemple, ``-1``)

Si un opérateur est appliqué à différents types, le compilateur essaie de convertir implicitement l'un des opérandes au type de l'autre (c'est la même chose pour les assignations).
This means that operations are always performed in the type of one of the operands.

For more details about which implicit conversions are possible,
please consult the sections about the types themselves.

In the example below, ``y`` and ``z``, the operands of the addition,
do not have the same type, but ``uint8`` can
be implicitly converted to ``uint16`` and not vice-versa. Because of that,
``y`` is converted to the type of ``z`` before the addition is performed
in the ``uint16`` type. The resulting type of the expression ``y + z`` is ``uint16``.
Because it is assigned to a variable of type ``uint32`` another implicit conversion
is performed after the addition.

.. code-block:: solidity

    uint8 y;
    uint16 z;
    uint32 x = y + z;


Conversions explicites
----------------------

Si le compilateur ne permet pas la conversion implicite mais que vous savez ce que vous faites, une conversion de type explicite est parfois possible. Notez que cela peut vous donner un comportement inattendu et vous permet de contourner certaines fonctions de sécurité du compilateur, donc assurez-vous de tester que le résultat est ce que vous voulez !

Prenons l'exemple suivant où l'on convertit un ``int8`` négatif en un ``uint`` :

.. code-block:: solidity

    int  y = -3;
    uint x = uint(y);

A la fin de cet extrait de code, ``x`` aura la valeur ``0xfffffff...fd`` (64 caractères hexadécimaux), qui est -3 dans la représentation en 256 bits du complément à deux.

Si un entier est explicitement converti en un type plus petit, les bits d'ordre supérieur sont coupés:

.. code-block:: solidity

    uint32 a = 0x12345678;
    uint16 b = uint16(a); // b sera désormais 0x5678

Si un entier est explicitement converti en un type plus grand, il est rembourré par la gauche (c'est-à-dire à l'extrémité supérieure de l'ordre).
Le résultat de la conversion sera comparé à l'entier original:

.. code-block:: solidity

    uint16 a = 0x1234;
    uint32 b = uint32(a); // b will be 0x00001234 now
    assert(a == b);

Les types à taille fixe se comportent différemment lors des conversions. Ils peuvent être considérés comme des séquences d'octets individuels et la conversion à un type plus petit coupera la séquence:

.. code-block:: solidity

    bytes2 a = 0x1234;
    bytes1 b = bytes1(a); // b sera désormais 0x12

Si un type à taille fixe est explicitement converti en un type plus grand, il est rembourré à droite. L'accès à l'octet par un index fixe donnera la même valeur avant et après la conversion (si l'index est toujours dans la plage):

.. code-block:: solidity

    bytes2 a = 0x1234;
    bytes4 b = bytes4(a); // b sera désormais 0x12340000
    assert(a[0] == b[0]);
    assert(a[1] == b[1]);

Puisque les entiers et les tableaux d'octets de taille fixe se comportent différemment lorsqu'ils sont tronqués ou rembourrés, les conversions explicites entre entiers et tableaux d'octets de taille fixe ne sont autorisées que si les deux ont la même taille. Si vous voulez convertir entre des entiers et des tableaux d'octets de taille fixe de tailles différentes, vous devez utiliser des conversions intermédiaires qui font la troncature et le remplissage désirés.
règles explicites:

.. code-block:: solidity

    bytes2 a = 0x1234;
    uint32 b = uint16(a); // b sera désormais 0x00001234
    uint32 c = uint32(bytes4(a)); // c sera désormais 0x12340000
    uint8 d = uint8(uint16(a)); // d sera désormais 0x34
    uint8 e = uint8(bytes1(a)); // d sera désormais 0x12

``bytes`` arrays and ``bytes`` calldata slices can be converted explicitly to fixed bytes types (``bytes1``/.../``bytes32``).
In case the array is longer than the target fixed bytes type, truncation at the end will happen.
If the array is shorter than the target type, it will be padded with zeros at the end.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.5;

    contract C {
        bytes s = "abcdefgh";
        function f(bytes calldata c, bytes memory m) public view returns (bytes16, bytes3) {
            require(c.length == 16, "");
            bytes16 b = bytes16(m);  // if length of m is greater than 16, truncation will happen
            b = bytes16(s);  // padded on the right, so result is "abcdefgh\0\0\0\0\0\0\0\0"
            bytes3 b1 = bytes3(s); // truncated, b1 equals to "abc"
            b = bytes16(c[:8]);  // also padded with zeros
            return (b, b1);
        }
    }

.. _types-conversion-literals:

Conversions entre les types littéraux et élémentaires
=====================================================

Types nombres entiers
---------------------

Les nombres décimaux et hexadécimaux peuvent être implicitement convertis en n'importe quel type entier suffisamment grand pour le représenter sans troncature::

    uint8 a = 12; // Bon
    uint32 b = 1234; // Bon
    uint16 c = 0x123456; // échoue, car devrait tronquer en 0x3456

.. note::
    Prior to version 0.8.0, any decimal or hexadecimal number literals could be explicitly
    converted to an integer type. From 0.8.0, such explicit conversions are as strict as implicit
    conversions, i.e., they are only allowed if the literal fits in the resulting range.

Tableaux d'octets de taille fixe
--------------------------------

Les nombres décimaux ne peuvent pas être implicitement convertis en tableaux d'octets de taille fixe. Les nombres hexadécimaux peuvent être littéraux, mais seulement si le nombre de chiffres hexadécimaux correspond exactement à la taille du type de ``bytes``. Par exception, les nombres décimaux et hexadécimaux ayant une valeur de zéro peuvent être convertis en n'importe quel type à taille fixe:

.. code-block:: solidity

    bytes2 a = 54321; // pas autorisé
    bytes2 b = 0x12; // pas autorisé
    bytes2 c = 0x123; // pas autorisé
    bytes2 d = 0x1234; // bon
    bytes2 e = 0x0012; // bon
    bytes4 f = 0; // bon
    bytes4 g = 0x0; // bon

Les littéraux de chaînes de caractères et les littéraux de chaînes hexadécimales peuvent être implicitement convertis en tableaux d'octets de taille fixe, si leur nombre de caractères correspond à la taille du type ``bytes``:

.. code-block:: solidity

    bytes2 a = hex"1234"; // bon
    bytes2 b = "xy"; // bon
    bytes2 c = hex"12"; // pas autorisé
    bytes2 d = hex"123"; // pas autorisé
    bytes2 e = "x"; // pas autorisé
    bytes2 f = "xyz"; // débile

Adresses
--------

Comme décrit dans :ref:`address_literals`, les chaines de caractères hexadécimaux de la bonne taille qui passent le test de somme de contrôle sont de type ``address``. Aucun autre littéral ne peut être implicitement converti au type ``address``.

Les conversions explicites de ``bytes20`` ou de tout type entier en ``address`` aboutissent en une ``address payable```.