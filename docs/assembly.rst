.. _inline-assembly:

###################
Assembleur en ligne
###################

.. index:: ! assembly, ! asm, ! evmasm


Vous pouvez entrelacer les instructions en Solidity avec de l'assembleur en ligne, dans un langage proche de celui de la machine virtuelle. Cela vous donne un contrôle plus fin, en particulier lorsque vous améliorez le langage en écrivant des bibliothèques.

Le langage utilisé pour l'assembleur en ligne en Solidity s'appelle :ref:`Yul <yul>`
et est documenté dans sa propre section. Cette section montre comment le code assembleur en ligne peut s'interfacer au code Solidity l'entourant.


.. warning::
    L'assembleur en ligne est un moyen d'accéder à la machine virtuelle Ethereum en bas niveau. Ceci permet de contourner plusieurs normes de sécurité importantes et contrôles de Solidity. Vous ne devriez l'utiliser que pour les tâches qui en ont besoin, et seulement si vous êtes sûr de pourquoi/comment l'utiliser.


Le bloc de code d'assembleur en ligne est indiqué par ``assembly { ... }``, où le code entre les accolades est écrit en langage :ref:`Yul <yul>`.

Le bloc de code assembleur en ligne peut accéder aux variables locales de Solidity comme expliqué ci-dessous.

Différents bloc de coe assembleur ne partagent pas le même espace de noms, c'est à dire qu'il n'est pas possible d'appeler une fonction Yul où d'accéder à une variable Yul variable definie dans un autre bloc.

Exemple
-------

L'exemple suivant fournit le code de bibliothèque pour accéder au code d'un autre contrat et le charger dans une variable ``bytes``. Ce n'est pas possible de base avec Solidity et l'idée est que les bibliothèques assembleur seront utilisées pour améliorer le langage Solidity.

.. code::


    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.7.0;

    library GetCode {
        function at(address _addr) public view returns (bytes memory o_code) {
            assembly {
                // récupère la taille du code, a besoin d'assembleur
                let size := extcodesize(_addr)
                // allouer le tableau de bytes de sortie - ceci serait fait en Solidity via o_code = new bytes(size)
                o_code := mload(0x40)
                // nouvelle "fin de mémoire" en incluant le padding
                mstore(0x40, add(o_code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // stocke la taille en mémoire
                mstore(o_code, size)
                // récupère le code lui-même, nécessite de l'assembleur
                extcodecopy(_addr, add(o_code, 0x20), 0, size)
            }
        }
    }

L'assembleur en ligne est également utile dans les cas où l'optimiseur ne parvient pas à produire un code efficace, par exemple :

.. code::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.7.0;

    library VectorSum {
        // Cette fonction est moins efficace car l'optimiseur ne parvient
        // pas à supprimer les contrôles de limites dans l'accès aux tableaux.
        function sumSolidity(uint[] memory _data) public pure returns (uint o_sum) {
            for (uint i = 0; i < _data.length; ++i)
                o_sum += _data[i];
        }

        // Nous savons que nous n'accédons au tableau que dans ses
        // limites, ce qui nous permet d'éviter la vérification. 0x20
        // doit être ajouté à un tableau car le premier emplacement
        // contient la longueur du tableau.
        function sumAsm(uint[] memory _data) public pure returns (uint o_sum) {
            for (uint i = 0; i < _data.length; ++i) {
                assembly {
                    o_sum := add(o_sum, mload(add(add(_data, 0x20), mul(i, 0x20))))
                }
            }
        }

        // Même chose que ci-dessus, mais exécute le code entier en assembleur en ligne.
        function sumPureAsm(uint[] memory _data) public pure returns (uint o_sum) {
            assembly {
               // Charge la taille (premiers 32 bytes)
               let len := mload(_data)

               // Saute le champ de taille.
               //
               // Garde une variable temporaire pour pouvoir l'incrémenter.
               //
               // NOTE: incrémenter _data resulterait en une
               // variable _data inutilisable après ce bloc d'assembleur
               let data := add(_data, 0x20)

               // Itère jusqu'à la limite.
               for
                   { let end := add(data, mul(len, 0x20)) }
                   lt(data, end)
                   { data := add(data, 0x20) }
               {
                   o_sum := add(o_sum, mload(data))
               }
            }
        }
    }



Access to External Variables, Functions and Libraries
-----------------------------------------------------

You can access Solidity variables and other identifiers by using their name.

Local variables of value type are directly usable in inline assembly.

Local variables that refer to memory or calldata evaluate to the
address of the variable in memory, resp. calldata, not the value itself.

For local storage variables or state variables, a single Yul identifier
is not sufficient, since they do not necessarily occupy a single full storage slot.
Therefore, their "address" is composed of a slot and a byte-offset
inside that slot. To retrieve the slot pointed to by the variable ``x``, you
use ``x_slot``, and to retrieve the byte-offset you use ``x_offset``.

Local Solidity variables are available for assignments, for example:

.. code::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.7.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                // We ignore the storage slot offset, we know it is zero
                // in this special case.
                r := mul(x, sload(b_slot))
            }
        }
    }

.. warning::
    If you access variables of a type that spans less than 256 bits
    (for example ``uint64``, ``address``, ``bytes16`` or ``byte``),
    you cannot make any assumptions about bits not part of the
    encoding of the type. Especially, do not assume them to be zero.
    To be safe, always clear the data properly before you use it
    in a context where this is important:
    ``uint32 x = f(); assembly { x := and(x, 0xffffffff) /* now use x */ }``
    To clean signed types, you can use the ``signextend`` opcode:
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``


Since Solidity 0.6.0 the name of a inline assembly variable may not end in ``_offset`` or ``_slot``
and it may not shadow any declaration visible in the scope of the inline assembly block
(including variable, contract and function declarations). Similarly, if the name of a declared
variable contains a dot ``.``, the prefix up to the ``.`` may not conflict with any
declaration visible in the scope of the inline assembly block.


Assignments are possible to assembly-local variables and to function-local
variables. Take care that when you assign to variables that point to
memory or storage, you will only change the pointer and not the data.

You can assign to the ``_slot`` part of a local storage variable pointer.
For these (structs, arrays or mappings), the ``_offset`` part is always zero.
It is not possible to assign to the ``_slot`` or ``_offset`` part of a state variable,
though.



Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. Function calls, loops, ifs and switches are converted by simple
rewriting rules and after that, the only thing the assembler does for you is re-arranging
functional-style opcodes, counting stack height for
variable access and removing stack slots for assembly-local variables when the end
of their block is reached.

Conventions in Solidity
-----------------------

In contrast to EVM assembly, Solidity has types which are narrower than 256 bits,
e.g. ``uint24``. For efficiency, most arithmetic operations ignore the fact that
types can be shorter than 256
bits, and the higher-order bits are cleaned when necessary,
i.e., shortly before they are written to memory or before comparisons are performed.
This means that if you access such a variable
from within inline assembly, you might have to manually clean the higher-order bits
first.

Solidity manages memory in the following way. There is a "free memory pointer"
at position ``0x40`` in memory. If you want to allocate memory, use the memory
starting from where this pointer points at and update it.
There is no guarantee that the memory has not been used before and thus
you cannot assume that its contents are zero bytes.
There is no built-in mechanism to release or free allocated memory.
Here is an assembly snippet you can use for allocating memory that follows the process outlined above::

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

The first 64 bytes of memory can be used as "scratch space" for short-term
allocation. The 32 bytes after the free memory pointer (i.e., starting at ``0x60``)
are meant to be zero permanently and is used as the initial value for
empty dynamic memory arrays.
This means that the allocatable memory starts at ``0x80``, which is the initial value
of the free memory pointer.

Elements in memory arrays in Solidity always occupy multiples of 32 bytes (this is
even true for ``byte[]``, but not for ``bytes`` and ``string``). Multi-dimensional memory
arrays are pointers to memory arrays. The length of a dynamic array is stored at the
first slot of the array and followed by the array elements.

.. warning::
    Statically-sized memory arrays do not have a length field, but it might be added later
    to allow better convertibility between statically- and dynamically-sized arrays, so
    do not rely on this.
