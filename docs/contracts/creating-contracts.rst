.. index:: ! contract;creation, constructor

******************
Créer des contrats
******************

Les contrats peuvent être créés "en dehors" via des transactions Ethereum ou depuis des contrat en Solidity.

Les EDIs, comme `Remix <https://remix.ethereum.org/>`_, facilite la tâche via des éléments visuels.

Créer des contrats via du code se fait le plus simplement en utilisant l'API Javascript `web3.js <https://github.com/ethereum/web3.js>`_.
Elle possède une fonction appelée `web3.eth.Contract <https://web3js.readthedocs.io/en/1.0/web3-eth-contract.html#new-contract>`_ qui facilite cette création

Quand un contrat est créé, son constructeur (une fonction déclarée via le mot-clé ``constructor``) est executé, de manière unique.

Un constructeur est optionnel. Aussi, un seul constructeur est autorisé, ce qui signifie que l'overloading n'est pas supporté.

Après que le constructeur ai été exécuté, le code final du contrat est déployé sur la Blockchain. Ce code inclut toutes les fonctions publiques et externes, et toutes les fonctions qui sont atteignables par des appels de fonctions. Le code déployé n'inclut pas le constructeur ou les fonctions internes uniquement appelées depuis le constructeur.

.. index:: constructor;arguments

En interne, les arguments du constructeur sont passés :ref:`ABI encodés <ABI>>` après le code du contrat lui-même, mais vous n'avez pas à vous en soucier si vous utilisez ``web3.js``.

Si un contrat veut créer un autre contrat, le code source (et le binaire) du contrat créé doit être connu du créateur.
Cela signifie que les dépendances cycliques de création sont impossibles.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.22 <0.9.0;


    contract OwnedToken {
        // TokenCreator est un type de contrat défini ci-dessous.
        // Il est possible de le référencer tant qu'il n'est pas utilisé
        // pour créer un nouveau contrat.
        TokenCreator creator;
        address owner;
        bytes32 name;

        // This is the constructor which registers the
        // creator and the assigned name.
        constructor(bytes32 _name) {
            // State variables are accessed via their name
            // and not via e.g. `this.owner`. Functions can
            // be accessed directly or through `this.f`,
            // but the latter provides an external view
            // to the function. Especially in the constructor,
            // you should not access functions externally,
            // because the function does not exist yet.
            // See the next section for details.
            owner = msg.sender;
            // Nous effectuons une conversion de type explicite de `address`.
            // vers `TokenCreator` et supposons que le type du
            // contrat appelant est TokenCreator,
            // Il n'y a pas vraiment moyen de vérifier ça.
            creator = TokenCreator(msg.sender);
            name = _name;
        }

        function changeName(bytes32 newName) public {
            // Seul le créateur peut modifier le nom --
            // la comparaison est possible puisque les contrats
            // sont explicitement convertibles en adresses.
            if (msg.sender == address(creator))
                name = newName;
        }

        function transfer(address newOwner) public {
            // Seul le propriétaire actuel peut transférer le token.
            if (msg.sender != owner) return;

            // Nous voulons aussi demander au créateur si le transfert
            // est valide. Notez que ceci appelle une fonction de la fonction
            // contrat défini ci-dessous. Si l'appel échoue (p. ex.
            // en raison d'un manque de gas), l'exécution échoue également ici.
            if (creator.isTokenTransferOK(owner, newOwner))
                owner = newOwner;
        }
    }

    contract TokenCreator {
        function createToken(bytes32 name)
           public
           returns (OwnedToken tokenAddress)
        {
            // Créer un nouveau contrat Token et renvoyer son adresse.
            // Du côté JavaScript, le type de retour est simplement
            // `address`, car c'est le type le plus proche disponible dans
            // l'ABI.
            return new OwnedToken(name);
        }

        function changeName(OwnedToken tokenAddress, bytes32 name) public {
            // Encore une fois, le type externe de `tokenAddress' est
            // simplement `adresse`.
            tokenAddress.changeName(name);
        }

        function isTokenTransferOK(address currentOwner, address newOwner)
            public
            pure
            returns (bool ok)
        {
            // Vérifier une condition arbitraire.
            return keccak256(abi.encodePacked(currentOwner, newOwner))[0] == 0x7f;
        }
    }
