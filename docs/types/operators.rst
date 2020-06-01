.. index:: assignment, ! delete, lvalue

Opérateurs impliquant des LValues
=================================

Si ``a`` est une LValue (c.-à-d. une variable ou quelque chose qui peut être assigné à), les opérateurs suivants sont disponibles en version raccourcie::

``a += e`` équivaut à ``a = a + e``. Les opérateurs ``-=``, ``*=``, ``/=``, ``%=``, ``|=``, ``&=`` et ``^=`` sont définis de la même manière. ``a++`` et ``a--`` sont équivalents à ``a += 1`` / ``a -= 1`` mais l'expression elle-même a toujours la valeur précédente ``a``. Par contraste, ``--a`` et ``++a`` changent également ``a`` de ``1`` , mais retournent la valeur après le changement.
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

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.7.0;

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
