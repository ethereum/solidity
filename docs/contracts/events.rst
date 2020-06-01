.. index:: ! event

.. _events:

******
Événements
******

Les événements Solidity autorisent une abstraction en plus de la fonctionnalité de journalisation de l'EVM.
Les applications peuvent souscrire à et écouter ces événements via l'interface RPC d'un client Ethereum.

Les événements sont des membres héritables des contrats. Lorsque vous les appelez, ils font en sorte que les arguments soient stockés dans le journal des transactions - une structure de données spéciale dans la blockchain. Ces logs sont associés à l'adresse du contrat, sont incorporés dans la blockchain et y restent tant qu'un bloc est accessible (pour toujours à partir des versions Frontier et Homestead, mais cela peut changer avec Serenity). Le journal et ses données d'événement ne sont pas accessibles depuis les contrats (pas même depuis le contrat qui les a créés).

Il est possible de demander une simple vérification de paiement (SPV) pour les logs, de sorte que si une entité externe fournit un contrat avec une telle vérification, elle peut vérifier que le log existe réellement dans la blockchain. Vous devez fournir des en-têtes (headers) de bloc car le contrat ne peut voir que les 256 derniers hashs de blocs.

Vous pouvez ajouter l'attribut ``indexed`` à un maximum de trois paramètres qui les ajoute à une structure de données spéciale appelée :ref:`"topics" <abi_events>` au lieu de la partie data du log. Si vous utilisez des tableaux (y compris les ``string`` et ``bytes``)
comme arguments indexés, leurs hashs Keccak-256 sont stockés comme topic à la place, car un topic ne peut contenir qu'un seul mot (32 octets).

Tous les paramètres sans l'attribut ``indexed`` sont :ref:`ABI-encoded <ABI>` dans la partie données du log.

Les topics vous permettent de rechercher des événements, par exemple lors du filtrage d'une séquence de blocs pour certains événements. Vous pouvez également filtrer les événements par l'adresse du contrat qui les a émis.

Par exemple, le code ci-dessous utilise web3.js ``subscribe("logs")``
`method <https://web3js.readthedocs.io/en/1.0/web3-eth-subscribe.html#subscribe-logs>`_  pour filtrer les logs qui correspondent à un sujet avec une certaine valeur d'adresse :

.. code-block:: javascript

    var options = {
        fromBlock: 0,
        address: web3.eth.defaultAccount,
        topics: ["0x0000000000000000000000000000000000000000000000000000000000000000", null, null]
    };
    web3.eth.subscribe('logs', options, function (error, result) {
        if (!error)
            console.log(result);
    })
        .on("data", function (log) {
            console.log(log);
        })
        .on("changed", function (log) {
    });


Le hash de la signature de l'event est l'un des topics, sauf si vous avez déclaré l'événement avec le spécificateur "anonymous". Cela signifie qu'il n'est pas possible de filtrer des événements anonymes spécifiques par leur nom.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.21 <0.7.0;

    contract ClientReceipt {
        event Deposit(
            address indexed _from,
            bytes32 indexed _id,
            uint _value
        );

        function deposit(bytes32 _id) public payable {
            // Les événements sont émis à l'aide de `emit`, suivi du
            // nom de l'événement et des arguments
            // (le cas échéant) entre parenthèses. Une telle invocation
            // (même profondément imbriquée) peut être détectée à partir de
            // l'API JavaScript en filtrant `Deposit`.
            emit Deposit(msg.sender, _id, msg.value);
        }
    }

L'utilisation dans l'API JavaScript est la suivante :

::

    var abi = /* abi telle que génerée par le compilateur */;
    var ClientReceipt = web3.eth.contract(abi);
    var clientReceipt = ClientReceipt.at("0x1234...ab67" /* adresse */);

    var event = clientReceipt.Deposit();

    // inspecter les eventuels changements
    event.watch(function(error, result){
        // le résultat contient des arguments et topics non indexés
        // passées à l'appel de `Deposit`.
        if (!error)
            console.log(result);
    });


    // Ou passez une fonction pour ecouter dès maintenant
    var event = clientReceipt.Deposit(function(error, result) {
        if (!error)
            console.log(result);
    });

La sortie du code ci-dessus ressemble à (trimmée):

.. code-block:: json

  {
     "returnValues": {
         "_from": "0x1111…FFFFCCCC",
         "_id": "0x50…sd5adb20",
         "_value": "0x420042"
     },
     "raw": {
         "data": "0x7f…91385",
         "topics": ["0xfd4…b4ead7", "0x7f…1a91385"]
     }
  }

.. index:: ! log

Interface bas-niveau des Logs
===========================

Il est également possible d'accéder à l'interface bas niveau du mécanisme de logs via les fonctions ``log0``, ``log1``, ``log2``, ``log3`` et ``log4``.
``logi`` prend le paramètre ``i + 1`` paramètre de type ``bytes32``, où le premier argument sera utilisé pour la partie données du journal et les autres comme sujets. L'appel d'événement ci-dessus peut être effectué de la même manière que

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.10 <0.7.0;

    contract C {
        function f() public payable {
            uint256 _id = 0x420042;
            log3(
                bytes32(msg.value),
                bytes32(0x50cb9fe53daa9737b786ab3646f04d0150dc50ef4e75f59509d83667ad5adb20),
                bytes32(uint256(msg.sender)),
                bytes32(_id)
            );
        }
    }

où le nombre hexadécimal long est égal à ``keccak256("Deposit(address,bytes32,uint256)")``, la signature de l'événement.

Ressources complémentaires pour comprendre les Events
==============================================

- `Javascript documentation <https://github.com/ethereum/wiki/wiki/JavaScript-API#contract-events>`_
- `Example usage of events <https://github.com/debris/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>`_
- `How to access them in js <https://github.com/debris/smart-exchange/blob/master/lib/exchange_transactions.js>`_
