****************************
Unités et variables globales
****************************

.. index:: wei, finney, szabo, gwei, ether

Unités d'Ether
==============

Un nombre littéral peut prendre un suffixe de "wei", ``gwei`` ou "ether" pour spécifier une sous-dénomination d'éther, où les nombres d'éther sans postfix sont supposés être en Wei.

.. code-block:: solidity
    :force:

    assert(1 wei == 1);
    assert(1 gwei == 1e9);
    assert(1 ether == 1e18);

Le seul effet du suffixe de sous-dénomination est une multiplication par une puissance de dix..

.. note::
    The denominations ``finney`` and ``szabo`` have been removed in version 0.7.0.

.. index:: time, seconds, minutes, hours, days, weeks, years

Unités de temps
===============

Des suffixes comme ``seconds``, ``minutes``, ``hours``, ``days`` et ``weeks`` peuvent être utilisés après les nombres littéraux pour spécifier les unités de temps où les secondes sont l'unité de base et les unités sont considérées naïvement de la façon suivante :

* ``1 == 1 seconds``
* ``1 minutes == 60 seconds``
* ``1 hours == 60 minutes``
* ``1 days == 24 hours``
* ``1 weeks == 7 days``

Faites attention si vous effectuez des calculs calendaires en utilisant ces unités, parce que chaque année n'est pas égale à 365 jours ni même chaque jour n'a 24 heures à cause des `secondes bissextiles <https://en.wikipedia.org/wiki/Leap_second>`_.
Les secondes intercalaires étant imprévisibles, une bibliothèque de calendrier exacte doit être mise à jour par un oracle externe.

.. note::
    Le suffixe ``years`` a été supprimé dans la version 0.5.0 pour les raisons ci-dessus.

Ces suffixes ne peuvent pas être appliqués aux variables. Si vous voulez interpréter un paramètre de fonctions en jours, par exemple, vous pouvez le faire de la manière suivante:

.. code-block:: solidity

    function f(uint start, uint daysAfter) public {
        if (block.timestamp >= start + daysAfter * 1 days) {
          // ...
        }
    }

Variables spéciales et fonctions
================================

Il y a des variables et des fonctions spéciales qui existent toujours dans l'espace de nommage global et qui sont principalement utilisées pour fournir des informations sur la chaîne de blocs ou sont des fonctions utilitaires générales.

.. index:: abi, block, coinbase, difficulty, encode, number, block;number, timestamp, block;timestamp, msg, data, gas, sender, value, gas price, origin



Propriétés du bloc et des transactions
--------------------------------------

- ``blockhash(uint blockNumber) returns (bytes32)``: hash du numéro de bloc passé - mnarche seulement pour les 256 plus récents, excluant le bloc courant
- ``block.basefee`` (``uint``): current block's base fee (`EIP-3198 <https://eips.ethereum.org/EIPS/eip-3198>`_ and `EIP-1559 <https://eips.ethereum.org/EIPS/eip-1559>`_)
- ``block.chainid`` (``uint``): current chain id
- ``block.coinbase`` (``address payable``): addresse du mineur du bloc courant
- ``block.difficulty`` (``uint``): difficulté du bloc courant
- ``block.gaslimit`` (``uint``): limite de gas actuelle
- ``block.number`` (``uint``): numéro du bloc courant
- ``block.timestamp`` (``uint``): timestamp du bloc en temps unix (secondes)
- ``gasleft() returns (uint256)``: gas restant
- ``msg.data`` (``bytes calldata``): calldata complet
- ``msg.sender`` (``address payable``): expéditeur du message (appel courant)
- ``msg.sig`` (``bytes4``): 4 premiers octets calldata (i.e. identifiant de function)
- ``msg.value`` (``uint``): nombre de wei envoyés avec le message
- ``tx.gasprice`` (``uint``): prix de la transaction en gas
- ``tx.origin`` (``address payable``): expéditeur de la transaction (appel global complet)

.. note::
    Les valeurs de tous les membres de ``msg``, y compris ``msg.sender``et ``msg.value`` peuvent changer pour chaque appel de fonction **external**.
    Ceci inclut les appels aux fonctions de librairies.

.. note::
    When contracts are evaluated off-chain rather than in context of a transaction included in a
    block, you should not assume that ``block.*`` and ``tx.*`` refer to values from any specific
    block or transaction. These values are provided by the EVM implementation that executes the
    contract and can be arbitrary.

.. note::
    Ne vous basez pas sur ``block.timestamp`` ou ``blockhash`` comme source de hasard, à moins de savoir ce que vous faites.

    L'horodatage et le hashage du bloc peuvent tous deux être influencés dans une certaine mesure par les mineurs.
    Les mauvais acteurs de la communauté minière peuvent par exemple exécuter une fonction de casino sur un hash choisi et simplement réessayer un hash différent s'ils n'ont pas reçu d'argent.

    L'horodatage du bloc courant doit être strictement supérieur à celui du dernier bloc, mais la seule garantie est qu'il se situera entre les horodatages de deux blocs consécutifs dans la chaîne canonique.

.. note::
    Les hashs de blocs ne sont pas disponibles pour tous les blocs pour des raisons d'évolutivité/place.
    Vous ne pouvez accéder qu'aux hachages des 256 blocs les plus récents, toutes les autres valeurs seront nulles.

.. note::
    La fonction ``blockhash`` était auparavant connue sous le nom ``block.blockhash``. Elle a été dépréciée dans la version 0.4.22 et supprimée dans la version 0.5.0.

.. note::
    La fonction ``gasleft`` était auparavant connue sous le nom de ``msg.gas``. Elle a été dépréciée dans la version 0.4.21 et supprimée dans la version 0.5.0.

.. note::
    In version 0.7.0, the alias ``now`` (for ``block.timestamp``) was removed.

.. index:: abi, encoding, packed

Fonctions d'encodage et de décodage de l'ABI
--------------------------------------------

- ``abi.decode(bytes memory encodedData, (...)) returns (...)``: ABI-decodes the given data, while the types are given in parentheses as second argument. Example: ``(uint a, uint[2] memory b, bytes memory c) = abi.decode(data, (uint, uint[2], bytes))``
- ``abi.encode(...) returns (bytes memory)``: ABI-encodes the given arguments
- ``abi.encodePacked(...) returns (bytes memory)``: Performs :ref:`packed encoding <abi_packed_mode>` of the given arguments. Note that packed encoding can be ambiguous!
- ``abi.encodeWithSelector(bytes4 selector, ...) returns (bytes memory)``: ABI-encodes the given arguments starting from the second and prepends the given four-byte selector
- ``abi.encodeWithSignature(string memory signature, ...) returns (bytes memory)``: Equivalent to ``abi.encodeWithSelector(bytes4(keccak256(bytes(signature))), ...)``
- ``abi.encodeCall(function functionPointer, (...)) returns (bytes memory)``: ABI-encodes a call to ``functionPointer`` with the arguments found in the tuple. Performs a full type-check, ensuring the types match the function signature. Result equals ``abi.encodeWithSelector(functionPointer.selector, (...))``

.. note::
    Ces fonctions d'encodage peuvent être utilisées pour créer des données pour des appels de fonctions externes sans réellement appeler une fonction externe. De plus, ``keccak256(abi.encododePacked(a, b))`` est un moyen de calculer le hash des données structurées (bien qu'il soit possible de créer une ``collision de hachage`` en utilisant différents types d'entrées).

See the documentation about the :ref:`ABI <ABI>` and the
:ref:`tightly packed encoding <abi_packed_mode>` for details about the encoding.

.. index:: bytes members

Members of bytes
----------------

- ``bytes.concat(...) returns (bytes memory)``: :ref:`Concatenates variable number of bytes and bytes1, ..., bytes32 arguments to one byte array<bytes-concat>`

.. index:: string members

Members of string
-----------------

- ``string.concat(...) returns (string memory)``: :ref:`Concatenates variable number of string arguments to one string array<string-concat>`


.. index:: assert, revert, require

Gestion des erreurs
-------------------

Voir la section dédiée sur :ref:`assert and require<assert-and-require>` pour plus de détails sur la gestion des erreurs et quand utiliser quelle fonction.

``assert(bool condition)``:
    entraîne une ``Panic error`` et donc la réversion du changement d'état si la condition n'est pas remplie - à utiliser pour les erreurs internes.
``require(bool condition)``:
    ``revert`` si la condition n'est pas remplie - à utiliser en cas d'erreurs dans les entrées ou les composants externes.

``require(bool condition, string memory message)``:
    ``revert`` si la condition n'est pas remplie - à utiliser en cas d'erreurs dans les entrées ou les composants externes. Fournit également un message d'erreur.
``revert()``:
    annuler l'exécution et annuler les changements d'état
``revert(string memory reason)``:
    annuler l'exécution et annuler les changements d'état, fournissant une phrase explicative

.. index:: keccak256, ripemd160, sha256, ecrecover, addmod, mulmod, cryptography,

.. _mathematical-and-cryptographic-functions:

Fonctions mathématiques et cryptographiques
-------------------------------------------

``addmod(uint x, uint y, uint k) returns (uint)``:
    calcule ``(x + y) % k`` où l'addition est effectuée avec une précision arbitraire et n'overflow pas à ``2**256``. ``assert`` que ``k != 0`` à partir de la version 0.5.0.

``mulmod(uint x, uint y, uint k) returns (uint)``:
    calcule ``(x * y) % k`` où la multiplication est effectuée avec une précision arbitraire et n'overflow pas à ``2**256``. ``assert`` que ``k != 0`` à partir de la version 0.5.0.

``keccak256(bytes memory) returns (bytes32)``:
    calcule le hash Keccak-256 du paramètre

.. note::
    Il y avait un alias pour ``keccak256`` appelé ``sha3``, qui a été supprimé dans la version 0.5.0. pour éviter la confusion

``sha256(bytes memory) returns (bytes32)``:
    calcule le hash SHA-256 du paramètre

``ripemd160(bytes memory) returns (bytes20)``:
    calcule le hash RIPEMD-160 du paramètre

``ecrecover(bytes32 hash, uint8 v, bytes32 r, bytes32 s) returns (address)``:
    récupérer l'adresse associée à la clé publique à partir de la signature de la courbe elliptique ou retourner zéro sur erreur.
    The function parameters correspond to ECDSA values of the signature:

    * ``r`` = first 32 bytes of signature
    * ``s`` = second 32 bytes of signature
    * ``v`` = final 1 byte of signature

   La fonction ``ecrecover`` renvoie une ``address``, et non une ``address payable``. Voir :ref:`adresse payable<address>` pour la conversion, au cas où vous auriez besoin de transférer des fonds à l'adresse récupérée.

    For further details, read `example usage <https://ethereum.stackexchange.com/questions/1777/workflow-on-signing-a-string-with-private-key-followed-by-signature-verificatio>`_.

.. warning::

    If you use ``ecrecover``, be aware that a valid signature can be turned into a different valid signature without
    requiring knowledge of the corresponding private key. In the Homestead hard fork, this issue was fixed
    for _transaction_ signatures (see `EIP-2 <https://eips.ethereum.org/EIPS/eip-2#specification>`_), but
    the ecrecover function remained unchanged.

    This is usually not a problem unless you require signatures to be unique or
    use them to identify items. OpenZeppelin have a `ECDSA helper library <https://docs.openzeppelin.com/contracts/2.x/api/cryptography#ECDSA>`_ that you can use as a wrapper for ``ecrecover`` without this issue.

.. note::

    Il se peut que vous rencontriez ``out-of-gas`` pour ``sha256``, ``ripemd160`` ou ``erecover`` sur une *blockchain privée*. La raison en est que ces contrats sont mis en œuvre sous la forme de contrats dits précompilés et que ces contrats n'existent réellement qu'après avoir reçu le premier message (bien que leur code contrat soit codé en dur). Les messages à des contrats inexistants sont plus coûteux et l'exécution se heurte donc à une erreur out-of-gas. Une solution de contournement pour ce problème est d'envoyer d'abord, par exemple, 1 Wei à chacun des contrats avant de les utiliser dans vos contrats réels. Le problème n'existe pas sur la cha^ine publique Ethereum ni sur les différents testnets officiels.

.. index:: balance, codehash, send, transfer, call, callcode, delegatecall, staticcall

.. _address_related:

Membres du type address
-----------------------

``<address>.balance`` (``uint256``):
    balance de l':ref:`address` en Wei

``<address>.code`` (``bytes memory``)
    code at the :ref:`address` (can be empty)

``<address>.codehash`` (``bytes32``)
    the codehash of the :ref:`address`

``<address payable>.transfer(uint256 amount)``
    send given amount of Wei to :ref:`address`, reverts on failure, forwards 2300 gas stipend, not adjustable

``<address payable>.send(uint256 amount) returns (bool)``:
    envoie la quantité donnée de Wei à :ref:`adress`, retourne ``false`` en cas d'échec, envoie 2300 gas (non réglable)

``<address>.call(bytes memory) returns (bool, bytes memory)``:
    émett un appel de bas niveau ``CALL`` avec la charge utile donnée, renvoie l'état de réussite et les données de retour, achemine tout le gas disponible ou un montant spécifié

``<address>.delegatecall(bytes memory) returns (bool, bytes memory)``:
    émet un appel de bas niveau ``DELEGATECALL`` avec la charge utile donnée, retourne les données de succès et de retour, achemine tout le gas disponible ou un montant spécifié

``<address>.staticcall(bytes memory) returns (bool, bytes memory)``:
    émettre un appel de bas niveau ``STATICCALL`` avec la charge utile donnée, retourne les conditions de succès et les données de retour, achemine tout le gas disponible ou un montant spécifié

Pour plus d'informations, voir la section sur :ref:`adress`.

.. warning::
    You should avoid using ``.call()`` whenever possible when executing another contract function as it bypasses type checking,
    function existence check, and argument packing.

.. warning::
    Il y a certains dangers à utiliser l'option ``send`` : Le transfert échoue si la profondeur de la pile d'appels est à 1024 (cela peut toujours être forcé par l'appelant) et il échoue également si le destinataire manque de gas. Donc, afin d'effectuer des transferts d'éther en toute sécurité, vérifiez toujours la valeur de retour de ``send``, utilisez  ``transfer`` ou mieux encore :
    Utilisez un modèle où le bénéficiaire retire l'argent.

.. warning::
    Due to the fact that the EVM considers a call to a non-existing contract to always succeed,
    Solidity includes an extra check using the ``extcodesize`` opcode when performing external calls.
    This ensures that the contract that is about to be called either actually exists (it contains code)
    or an exception is raised.

    The low-level calls which operate on addresses rather than contract instances (i.e. ``.call()``,
    ``.delegatecall()``, ``.staticcall()``, ``.send()`` and ``.transfer()``) **do not** include this
    check, which makes them cheaper in terms of gas but also less safe.

.. note::
   Avant la version 0.5.0, Solidity permettait aux membres d'adresses d'être accessibles par une instance de contrat, par exemple ``this.balance``.
   Ceci est maintenant interdit et une conversion explicite en adresse doit être faite : ``address(this).balance``.

.. note::
   Si l'accès aux variables d'état s'effectue via un appel de délégation de bas niveau, le plan de stockage des deux contrats doit être alignée pour que le contrat appelé puisse accéder correctement aux variables de stockage du contrat appelant par leur nom.
    Ce n'est bien sûr pas le cas si les pointeurs de stockage sont passés comme arguments de fonction comme dans le cas des fonctions de librairies (bibliothèques) de haut niveau.

.. note::
    Avant la version 0.5.0, ``.call``, ``.delegatecall`` et ``staticcall`` ne renvoyaient que la condition de succès et non les données de retour.

.. note::
    Avant la version 0.5.0, il y avait un membre appelé ``callcode`` avec une sémantique similaire mais légèrement différente de celle de ``delegatecall``.


.. index:: this, selfdestruct

Contract Related
----------------

``this`` (type du contrat courant):
     le contrat en cours, explicitement convertible en :ref:`address`.

``selfdestruct(address payable destinataire_des_fonds)`` :
     détruire le contrat en cours, en envoyant ses fonds à l'adresse :ref:`address` indiquée
    
Note that ``selfdestruct`` has some peculiarities inherited from the EVM:

    - the receiving contract's receive function is not executed.
    - the contract is only really destroyed at the end of the transaction and ``revert`` s might "undo" the destruction.

En outre, toutes les fonctions du contrat en cours peuvent être appelées directement, y compris la fonction en cours.

.. note::
     Avant la version 0.5.0, il existait une fonction appelée ``suicide`` avec la même sémantique que ``selfdestruct``.

.. index:: type, creationCode, runtimeCode

.. _meta-type:

Type Information
----------------

The expression ``type(X)`` can be used to retrieve information about the type
``X``. Currently, there is limited support for this feature (``X`` can be either
a contract or an integer type) but it might be expanded in the future.

The following properties are available for a contract type ``C``:

``type(C).name``
    The name of the contract.

``type(C).creationCode``
    Memory byte array that contains the creation bytecode of the contract.
    This can be used in inline assembly to build custom creation routines,
    especially by using the ``create2`` opcode.
    This property can **not** be accessed in the contract itself or any
    derived contract. It causes the bytecode to be included in the bytecode
    of the call site and thus circular references like that are not possible.

``type(C).runtimeCode``
    Memory byte array that contains the runtime bytecode of the contract.
    This is the code that is usually deployed by the constructor of ``C``.
    If ``C`` has a constructor that uses inline assembly, this might be
    different from the actually deployed bytecode. Also note that libraries
    modify their runtime bytecode at time of deployment to guard against
    regular calls.
    The same restrictions as with ``.creationCode`` also apply for this
    property.

In addition to the properties above, the following properties are available
for an interface type ``I``:

``type(I).interfaceId``:
    A ``bytes4`` value containing the `EIP-165 <https://eips.ethereum.org/EIPS/eip-165>`_
    interface identifier of the given interface ``I``. This identifier is defined as the ``XOR`` of all
    function selectors defined within the interface itself - excluding all inherited functions.

The following properties are available for an integer type ``T``:

``type(T).min``
    The smallest value representable by type ``T``.

``type(T).max``
    The largest value representable by type ``T``.
