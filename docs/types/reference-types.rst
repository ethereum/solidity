.. index:: ! type;reference, ! reference type, storage, memory, location, array, struct

.. _reference-types:

Types Référence
===============

Les valeurs du type référence peuvent être modifiées par plusieurs noms différents.
Comparez ceci avec les catégories de valeurs où vous obtenez une copie indépendante chaque fois qu'une variable de valeur est utilisée.
Pour cette raison, les types référence doivent être traités avec plus d'attention que les types valeur.
Actuellement, les types référence comprennent les structures, les tableaux et les mappages.
Si vous utilisez un type référence, vous devez toujours indiquer explicitement la zone de données où le type est enregistré :
``memory`` (dont la durée de vie est limitée à un appel de fonction), ``storage`` (l'emplacement où les variables d'état sont stockées) ou ``calldata`` (emplacement de données spécial qui contient les arguments de fonction, disponible uniquement pour les paramètres d'appel de fonction externe).

Une affectation ou une conversion de type qui modifie l'emplacement des données entraîne toujours une opération de copie automatique, alors que les affectations à l'intérieur du même emplacement de données ne copient que dans certains cas selon le type de stockage.

.. _data-location:

Emplacement des données
-----------------------

Chaque type référence, c'est-à-dire *arrays* (tableaux) et *structs*, comporte une annotation supplémentaire, la ``localisation des données``, indiquant où elles sont stockées. Il y a trois emplacements de données :
``Memory``, ``Storage`` et ``Calldata``. Calldata est une zone non modifiable, non persistante où les arguments de fonction sont stockés, et se comporte principalement comme memory.

.. note::
    If you can, try to use ``calldata`` as data location because it will avoid copies and
    also makes sure that the data cannot be modified. Arrays and structs with ``calldata``
    data location can also be returned from functions, but it is not possible to
    allocate such types.

.. note::
    Prior to version 0.6.9 data location for reference-type arguments was limited to
    ``calldata`` in external functions, ``memory`` in public functions and either
    ``memory`` or ``storage`` in internal and private ones.
    Now ``memory`` and ``calldata`` are allowed in all functions regardless of their visibility.

.. note::
    Avant la version 0.5.0, l'emplacement des données pouvait être omis, et était par défaut à des emplacements différents selon le type de variable, le type de fonction, etc.

.. _data-location-assignment:

Data location and assignment behaviour
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

La localisation des données n'est sont pas seulement pertinente pour la persistance des données, mais aussi pour la sémantique des affectations :

* Les affectations entre le stockage et la mémoire (ou à partir des données de la calldata) créent toujours une copie indépendante.
* Les affectations de mémoire à mémoire ne créent que des références. Cela signifie que les modifications d'une variable mémoire sont également visibles dans toutes les autres variables mémoire qui se réfèrent aux mêmes données.
* Les affectations du stockage à une variable de stockage local n'affectent également qu'une référence.
* En revanche, toutes les autres affectations au stockage sont toujours copiées. Les affectations à des variables d'état ou à des membres de variables locales de type structure de stockage, même si la variable locale elle-même n'est qu'une référence, constituent des exemples dans ce cas.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    contract C {
        // The data location of x is storage.
        // This is the only place where the
        // data location can be omitted.
        uint[] x;

        // The data location of memoryArray is memory.
        function f(uint[] memory memoryArray) public {
            x = memoryArray; // works, copies the whole array to storage
            uint[] storage y = x; // works, assigns a pointer, data location of y is storage
            y[7]; // fine, returns the 8th element
            y.pop(); // fine, modifies x through y
            delete x; // fine, clears the array, also modifies y
            // The following does not work; it would need to create a new temporary /
            // unnamed array in storage, but storage is "statically" allocated:
            // y = memoryArray;
            // This does not work either, since it would "reset" the pointer, but there
            // is no sensible location it could point to.
            // delete y;
            g(x); // calls g, handing over a reference to x
            h(x); // calls h and creates an independent, temporary copy in memory
        }

        function g(uint[] storage) internal pure {}
        function h(uint[] memory) public pure {}
    }

.. index:: ! array

.. _arrays:

Tableaux
--------

Les tableaux peuvent avoir une taille fixe à la compilation ou peuvent être dynamiques.

The type of an array of fixed size ``k`` and element type ``T`` is written as ``T[k]``,
and an array of dynamic size as ``T[]``.

Un tableau de taille fixe ``k`` et de type d'élément ``T`` est écrit ``T[k]``, un tableau de taille dynamique ``T[]``.

Par exemple, un tableau de 5 tableaux dynamiques de ``uint`` est ``uint[][5]`` (notez que la notation est inversée par rapport à certains autres langages). Pour accéder au deuxième uint du troisième tableau dynamique, vous utilisez ``x[2][1]`` (les indexs commencent à zéro et
l'accès fonctionne dans le sens inverse de la déclaration, c'est-à-dire que ``x[2]`` supprime un niveau dans le type de déclaration à partir de la droite).

Indices are zero-based, and access is in the opposite direction of the
declaration.

Il y a peu de restrictions concernant l'élément contenu, il peut aussi être un autre tableau, un mappage ou une structure. Les restrictions générales
s'appliquent, cependant, en ce sens que les mappages ne peuvent être utilisés que dans le ``storage`` et que les fonctions visibles au public nécessitent des paramètres qui sont des types reconnus par l':ref:`ABI types <ABI>`.

Il est possible de marquer les tableaux ``public`` et de demander à Solidity de créer un :ref:`getter <visibility-and-getters>`.
L'index numérique deviendra un paramètre obligatoire pour le getter.

For example, if you have a variable ``uint[][5] memory x``, you access the
seventh ``uint`` in the third dynamic array using ``x[2][6]``, and to access the
third dynamic array, use ``x[2]``. Again,
if you have an array ``T[5] a`` for a type ``T`` that can also be an array,
then ``a[2]`` always has type ``T``.


Accessing an array past its end causes a failing assertion. Methods ``.push()`` and ``.push(value)`` can be used
to append a new element at the end of the array, where ``.push()`` appends a zero-initialized element and returns
a reference to it.

.. index:: ! string, ! bytes

.. _strings:

.. _bytes:

``bytes`` and ``string`` as Arrays
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Les variables de type ``bytes`` et ``string`` sont des tableaux spéciaux. Un ``byte`` est semblable à un ``byte[]``, mais il est condensé en calldata et en mémoire. ``string`` est égal à ``bytes``, mais ne permet pas l'accès à la longueur ou à l'index.

Solidity does not have string manipulation functions, but there are
third-party string libraries. You can also compare two strings by their keccak256-hash using
``keccak256(abi.encodePacked(s1)) == keccak256(abi.encodePacked(s2))`` and
concatenate two strings using ``string.concat(s1, s2)``.

Il faut donc généralement préférer les ``bytes`` aux ``bytes[]`` car c'est moins cher à l'usage,
since using ``bytes1[]`` in ``memory`` adds 31 padding bytes between the elements. Note that in ``storage``, the
padding is absent due to tight packing, see :ref:`bytes and string <bytes-and-string>`. 
En règle générale, utilisez ``bytes`` pour les données en octets bruts de longueur arbitraire et ``string`` pour les données de chaîne de caractères de longueur arbitraire (UTF-8).
Si vous pouvez limiter la longueur à un certain nombre d'octets, utilisez toujours un des ``bytes1`` à ``bytes32``, car ils sont beaucoup moins chers également.

.. note::
    Si vous voulez accéder à la représentation en octets d'une chaîne de caractères ``s``, utilisez ``bytes(s).length`` / ``bytes(s)[7] ='x';``. Gardez à l'esprit que vous accédez aux octets de bas niveau de la représentation UTF-8, et non aux caractères individuels !

.. index:: ! bytes-concat, ! string-concat

.. _bytes-concat:
.. _string-concat:

The functions ``bytes.concat`` and ``string.concat``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can concatenate an arbitrary number of ``string`` values using ``string.concat``.
The function returns a single ``string memory`` array that contains the contents of the arguments without padding.
If you want to use parameters of other types that are not implicitly convertible to ``string``, you need to convert them to ``string`` first.

Analogously, the ``bytes.concat`` function can concatenate an arbitrary number of ``bytes`` or ``bytes1 ... bytes32`` values.
The function returns a single ``bytes memory`` array that contains the contents of the arguments without padding.
If you want to use string parameters or other types that are not implicitly convertible to ``bytes``, you need to convert them to ``bytes`` or ``bytes1``/.../``bytes32`` first.


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.12;

    contract C {
        string s = "Storage";
        function f(bytes calldata bc, string memory sm, bytes16 b) public view {
            string memory concat_string = string.concat(s, string(bc), "Literal", sm);
            assert((bytes(s).length + bc.length + 7 + bytes(sm).length) == bytes(concat_string).length);

            bytes memory concat_bytes = bytes.concat(bytes(s), bc, bc[:2], "Literal", bytes(sm), b);
            assert((bytes(s).length + bc.length + 2 + 7 + bytes(sm).length + b.length) == concat_bytes.length);
        }
    }

If you call ``string.concat`` or ``bytes.concat`` without arguments they return an empty array.

.. index:: ! array;allocating, new

Allouer des tableaux en mémoire
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Vous pouvez utiliser le mot-clé ``new`` pour créer des tableaux dont la longueur dépend de la durée d'exécution en mémoire.
Contrairement aux tableaux de stockage, il n'est **pas** possible de redimensionner les tableaux de mémoire (par exemple en les assignant au membre ``.length``). Vous devez soit calculer la taille requise à l'avance, soit créer un nouveau tableau de mémoire et copier chaque élément.

As all variables in Solidity, the elements of newly allocated arrays are always initialized
with the :ref:`default value<default-value>`.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f(uint len) public pure {
            uint[] memory a = new uint[](7);
            bytes memory b = new bytes(len);
            assert(a.length == 7);
            assert(b.length == len);
            a[6] = 8;
        }
    }

.. index:: ! array;literals, !inline;arrays

Tableaux littéraux / Inline Arrays
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

An array literal is a comma-separated list of one or more expressions, enclosed
in square brackets (``[...]``). For example ``[1, a, f(3)]``. The type of the
array literal is determined as follows:

It is always a statically-sized memory array whose length is the
number of expressions.

The base type of the array is the type of the first expression on the list such that all
other expressions can be implicitly converted to it. It is a type error
if this is not possible.

It is not enough that there is a type all the elements can be converted to. One of the elements
has to be of that type.

Le type d'un tableau littéral est un tableau mémoire de taille fixe dont le type de base est le type commun des éléments donnés. Le type de ``[1, 2, 3]`` est ``uint8[3] memory```, car le type de chacune de ces constantes est ``uint8``.
Pour cette raison, il est nécessaire de convertir le premier élément de l'exemple ci-dessus en ``uint``. 

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            g([uint(1), 2, 3]);
        }
        function g(uint[3] memory) public pure {
            // ...
        }
    }

The array literal ``[1, -1]`` is invalid because the type of the first expression
is ``uint8`` while the type of the second is ``int8`` and they cannot be implicitly
converted to each other. To make it work, you can use ``[int8(1), -1]``, for example.

Since fixed-size memory arrays of different type cannot be converted into each other
(even if the base types can), you always have to specify a common base type explicitly
if you want to use two-dimensional array literals:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure returns (uint24[2][4] memory) {
            uint24[2][4] memory x = [[uint24(0x1), 1], [0xffffff, 2], [uint24(0xff), 3], [uint24(0xffff), 4]];
            // The following does not work, because some of the inner arrays are not of the right type.
            // uint[2][4] memory x = [[0x1, 1], [0xffffff, 2], [0xff, 3], [0xffff, 4]];
            return x;
        }
    }

Fixed size memory arrays cannot be assigned to dynamically-sized
memory arrays, i.e. the following is not possible:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

    // Ceci ne compile pas.
    contract C {
        function f() public {
            // La ligne suivant provoque une erreur car uint[3] memory
            // ne peut pas être convertit en uint[] memory.
            uint[] memory x = [uint(1), 3, 4];
        }
    }

Il est prévu de supprimer cette restriction à l'avenir, mais crée actuellement certaines complications en raison de la façon dont les tableaux sont transmis dans l'ABI.

If you want to initialize dynamically-sized arrays, you have to assign the
individual elements:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    contract C {
        function f() public pure {
            uint[] memory x = new uint[](3);
            x[0] = 1;
            x[1] = 3;
            x[2] = 4;
        }
    }

.. index:: ! array;length, length, push, pop, !array;push, !array;pop

.. _array-members:

Array Members
^^^^^^^^^^^^^

**length**:
    Les tableaux ont un membre ``length`` qui contient leur nombre d'éléments.
     La longueur des tableaux memory est fixe (mais dynamique, c'est-à-dire qu'elle peut dépendre des paramètres d'exécution) une fois qu'ils sont créés.
**push()**:
     Les tableaux de stockage dynamique et les ``bytes`` (et non ``string``) ont une fonction membre appelée ``push`` que vous pouvez utiliser pour ajouter un élément à la fin du tableau. L'élément sera mis à zéro à l'initialisation. La fonction renvoie la nouvelle longueur.
**push(x)**:
     Dynamic storage arrays and ``bytes`` (not ``string``) have a member function
     called ``push(x)`` that you can use to append a given element at the end of the array.
     The function returns nothing.
**pop()**:
     Les tableaux de stockage dynamique et les ``bytes`` (et non ``string``) ont une fonction membre appelée ``pop()`` que vous pouvez utiliser pour supprimer un élément à la fin du tableau. Ceci appelle aussi implicitement :ref:``delete`` sur l'élément supprimé.

.. note::
    L'augmentation de la longueur d'un tableau en storage a des coûts en gas constants parce qu'on suppose que le stockage est nul, alors que la diminution de la longueur a au moins un coût linéaire (mais dans la plupart des cas pire que linéaire), parce qu'elle inclut explicitement l'élimination des éléments supprimés comme si on appelait :ref:``delete``.

.. note::
    To use arrays of arrays in external (instead of public) functions, you need to
    activate ABI coder v2.

.. note::
    Dans les versions EVM antérieures à Byzantium, il n'était pas possible d'accéder au retour de tableaux dynamique à partir des appels de fonctions. Si vous appelez des fonctions qui retournent des tableaux dynamiques, assurez-vous d'utiliser un EVM qui est configuré en mode Byzantium.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    contract ArrayContract {
        uint[2**20] m_aLotOfIntegers;
        // Notez que ce qui suit n'est pas une paire de tableaux dynamiques
        // mais un tableau tableau dynamique de paires (c'est-à-dire de
        // tableaux de taille fixe de longueur deux).
        // Pour cette raison, T[] est toujours un tableau dynamique
        // de T, même si T lui-même est un tableau.
        // L'emplacement des données pour toutes les variables d'état
        // est storage.
        bool[2][] m_pairsOfFlags;

        // newPairs est stocké en memory - seule possibilité
        // pour les arguments de fonction publique
        function setAllFlagPairs(bool[2][] memory newPairs) public {
            // l'assignation d' un tableau en storage implique la copie
            // de  ``newPairs`` et remplace l'array ``m_pairsOfFlags``.
            m_pairsOfFlags = newPairs;
        }

        struct StructType {
            uint[] contents;
            uint moreInfo;
        }
        StructType s;

        function f(uint[] memory c) public {
            // stocke un pointeur sur ``s`` dans ``g``
            StructType storage g = s;
            // change aussi ``s.moreInfo``.
            g.moreInfo = 2;
            // assigne une copie car ``g.contents`` n'est
            // pas une variable locale mais un membre
            // d'une variable locale
            g.contents = c;
        }

        function setFlagPair(uint index, bool flagA, bool flagB) public {
            // accès à un index inexistant, déclenche une exception
            m_pairsOfFlags[index][0] = flagA;
            m_pairsOfFlags[index][1] = flagB;
        }

        function changeFlagArraySize(uint newSize) public {
            // using push and pop is the only way to change the
            // length of an array
            if (newSize < m_pairsOfFlags.length) {
                while (m_pairsOfFlags.length > newSize)
                    m_pairsOfFlags.pop();
            } else if (newSize > m_pairsOfFlags.length) {
                while (m_pairsOfFlags.length < newSize)
                    m_pairsOfFlags.push();
            }
        }

        function clear() public {
            // these clear the arrays completely
            delete m_pairsOfFlags;
            delete m_aLotOfIntegers;
            // identical effect here
            m_pairsOfFlags = new bool[2][](0);
        }

        bytes m_byteData;

        function byteArrays(bytes memory data) public {
            // le tableau de byte ("bytes") sont différents car stockés sans
            // padding mais peuvent être traités comme des ``uint8[]``
            m_byteData = data;
            for (uint i = 0; i < 7; i++)
                m_byteData.push();
            m_byteData[3] = 0x08;
            delete m_byteData[2];
        }

        function addFlag(bool[2] memory flag) public returns (uint) {
            m_pairsOfFlags.push(flag);
            return m_pairsOfFlags.length;
        }

        function createMemoryArray(uint size) public pure returns (bytes memory) {
            // Un tableau dynamique est créé via `new`:
            uint[2][] memory arrayOfPairs = new uint[2][](size);

            // Les tableaux littéraux sont toujours de taille statique
            // et en cas d' utilisation de littéraux uniquement, au moins
            // un type doit être spécifié.
            arrayOfPairs[0] = [uint(1), 2];

            // Créée un tableau dynamique de bytes:
            bytes memory b = new bytes(200);
            for (uint i = 0; i < b.length; i++)
                b[i] = bytes1(uint8(i));
            return b;
        }
    }

.. index:: ! array;slice

.. _array-slices:

Array Slices
------------


Array slices are a view on a contiguous portion of an array.
They are written as ``x[start:end]``, where ``start`` and
``end`` are expressions resulting in a uint256 type (or
implicitly convertible to it). The first element of the
slice is ``x[start]`` and the last element is ``x[end - 1]``.

If ``start`` is greater than ``end`` or if ``end`` is greater
than the length of the array, an exception is thrown.

Both ``start`` and ``end`` are optional: ``start`` defaults
to ``0`` and ``end`` defaults to the length of the array.

Array slices do not have any members. They are implicitly
convertible to arrays of their underlying type
and support index access. Index access is not absolute
in the underlying array, but relative to the start of
the slice.

Array slices do not have a type name which means
no variable can have an array slices as type,
they only exist in intermediate expressions.

.. note::
    As of now, array slices are only implemented for calldata arrays.

Array slices are useful to ABI-decode secondary data passed in function parameters:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.5 <0.9.0;
    contract Proxy {
        /// @dev Address of the client contract managed by proxy i.e., this contract
        address client;

        constructor(address _client) {
            client = _client;
        }

        /// Forward call to "setOwner(address)" that is implemented by client
        /// after doing basic validation on the address argument.
        function forward(bytes calldata _payload) external {
            bytes4 sig = bytes4(_payload[:4]);
            // Due to truncating behaviour, bytes4(_payload) performs identically.
            // bytes4 sig = bytes4(_payload);
            if (sig == bytes4(keccak256("setOwner(address)"))) {
                address owner = abi.decode(_payload[4:], (address));
                require(owner != address(0), "Address of owner cannot be zero.");
            }
            (bool status,) = client.delegatecall(_payload);
            require(status, "Forwarded call failed.");
        }
    }



.. index:: ! struct, ! type;struct

.. _structs:

Structs
-------

Solidity permet de définir de nouveaux types sous forme de structs, comme le montre l'exemple suivant :


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    // Defines a new type with two fields.
    // Declaring a struct outside of a contract allows
    // it to be shared by multiple contracts.
    // Here, this is not really needed.
    struct Funder {
        address addr;
        uint amount;
    }

    contract CrowdFunding {
        // Structs can also be defined inside contracts, which makes them
        // visible only there and in derived contracts.
        struct Campaign {
            address payable beneficiary;
            uint fundingGoal;
            uint numFunders;
            uint amount;
            mapping (uint => Funder) funders;
        }

        uint numCampaigns;
        mapping (uint => Campaign) campaigns;

        function newCampaign(address payable beneficiary, uint goal) public returns (uint campaignID) {
            campaignID = numCampaigns++; // campaignID is return variable
            // We cannot use "campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)"
            // because the right hand side creates a memory-struct "Campaign" that contains a mapping.
            Campaign storage c = campaigns[campaignID];
            c.beneficiary = beneficiary;
            c.fundingGoal = goal;
        }

        function contribute(uint campaignID) public payable {
            Campaign storage c = campaigns[campaignID];
            // Creates a new temporary memory struct, initialised with the given values
            // and copies it over to storage.
            // Note that you can also use Funder(msg.sender, msg.value) to initialise.
            c.funders[c.numFunders++] = Funder({addr: msg.sender, amount: msg.value});
            c.amount += msg.value;
        }

        function checkGoalReached(uint campaignID) public returns (bool reached) {
            Campaign storage c = campaigns[campaignID];
            if (c.amount < c.fundingGoal)
                return false;
            uint amount = c.amount;
            c.amount = 0;
            c.beneficiary.transfer(amount);
            return true;
        }
    }

Le contrat ne fournit pas toutes les fonctionnalités d'un contrat de crowdfunding, mais il contient les concepts de base nécessaires pour comprendre les ``struct``.
Les types structs peuvent être utilisés à l'intérieur des ``mapping`` et des ``array`` et peuvent eux-mêmes contenir des mappages et des tableaux.

Il n'est pas possible pour une structure de contenir un membre de son propre type, bien que la structure elle-même puisse être le type de valeur d'un membre de mappage ou peut contenir un tableau de taille dynamique de son type.
Cette restriction est nécessaire, car la taille de la structure doit être finie.

Notez que dans toutes les fonctions, un type structure est affecté à une variable locale avec l'emplacement de données ``storage``.
Ceci ne copie pas la structure mais stocke seulement une référence pour que les affectations aux membres de la variable locale écrivent réellement dans l'état.

Bien sûr, vous pouvez aussi accéder directement aux membres de la structure sans l'affecter à une variable locale, comme dans ``campaigns[campaignID].amount = 0``.

.. note::
    Until Solidity 0.7.0, memory-structs containing members of storage-only types (e.g. mappings)
    were allowed and assignments like ``campaigns[campaignID] = Campaign(beneficiary, goal, 0, 0)``
    in the example above would work and just silently skip those members.
