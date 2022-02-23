.. index:: ! operator

Operators
=========

Arithmetic and bit operators can be applied even if the two operands do not have the same type.
For example, you can compute ``y = x + z``, where ``x`` is a ``uint8`` and ``z`` has
the type ``int32``. In these cases, the following mechanism will be used to determine
the type in which the operation is computed (this is important in case of overflow)
and the type of the operator's result:

1. If the type of the right operand can be implicitly converted to the type of the left
   operand, use the type of the left operand,
2. if the type of the left operand can be implicitly converted to the type of the right
   operand, use the type of the right operand,
3. otherwise, the operation is not allowed.

In case one of the operands is a :ref:`literal number <rational_literals>` it is first converted to its
"mobile type", which is the smallest type that can hold the value
(unsigned types of the same bit-width are considered "smaller" than the signed types).
If both are literal numbers, the operation is computed with arbitrary precision.

The operator's result type is the same as the type the operation is performed in,
except for comparison operators where the result is always ``bool``.

The operators ``**`` (exponentiation), ``<<``  and ``>>`` use the type of the
left operand for the operation and the result.

.. index:: assignment, lvalue, ! compound operators

Compound and Increment/Decrement Operators
------------------------------------------

If ``a`` is an LValue (i.e. a variable or something that can be assigned to), the
following operators are available as shorthands:

``a += e`` is equivalent to ``a = a + e``. The operators ``-=``, ``*=``, ``/=``, ``%=``,
``|=``, ``&=``, ``^=``, ``<<=`` and ``>>=`` are defined accordingly. ``a++`` and ``a--`` are equivalent
to ``a += 1`` / ``a -= 1`` but the expression itself still has the previous value
of ``a``. In contrast, ``--a`` and ``++a`` have the same effect on ``a`` but
return the value after the change.

.. index:: !delete

.. _delete:

delete
------

``delete a`` affecte la valeur initiale du type à ``a``. C'est-à-dire que pour les entiers, il est équivalent à ``a = 0``, mais il peut aussi être utilisé sur les tableaux, où il assigne un tableau dynamique de longueur zéro ou un tableau statique de la même longueur avec tous les éléments initialisés à leur valeur par défaut. ``delete a[x]`` deletes the item at index ``x`` of the array and leaves
all other elements and the length of the array untouched. This especially means that it leaves
a gap in the array. If you plan to remove items, a :ref:`mapping <mapping-types>` is probably a better choice.

Pour les structs, il assigne une structure avec tous les membres réinitialisés. En d'autres termes, la valeur de ``a`` après ``delete a`` est la même que si ``a`` était déclaré sans attribution, avec la réserve suivante :

``delete`` n'a aucun effet sur les mappages (car les clés des mappages peuvent être arbitraires et sont généralement inconnues). Ainsi, si vous supprimez une structure, elle réinitialisera tous les membres qui ne sont pas des ``mappings`` et se propagera récursivement dans les membres à moins qu'ils ne soient des mappings. Toutefois, il est possible de supprimer des clés individuelles et ce à quoi elles correspondent : Si ``a`` est un mappage, alors ``delete a[x]`` supprimera la valeur stockée à ``x``.

Il est important de noter que ``delete a`` se comporte vraiment comme une affectation à ``a``, c'est-à-dire qu'il stocke un nouvel objet dans ``a``.
Cette distinction est visible lorsque ``a`` est une variable par référence : Il ne réinitialisera que ``a`` lui-même, et non la valeur à laquelle il se référait précédemment.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    contract DeleteExample {
        uint data;
        uint[] dataArray;

        function f() public {
            uint x = data;
            delete x; // met x à 0, n' affecte pas data
            delete data; // met data à 0, n'affecte pas x
            uint[] storage y = dataArray;
            delete dataArray; // ceci met dataArray.length à zéro, mais un uint[]
            // est un objet complexe, donc y est affecté est un alias
            // vers l' objet en storage.
            // D' un autre côté: "delete y" est invalid, car l' assignement à
            // une variable locale pointant vers un objet en storage n' est
            // autorisée que depuis un objet en storage.
            assert(y.length == 0);
        }
    }
