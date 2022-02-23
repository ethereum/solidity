.. index:: ! event

.. _events:

******
Événements
******

Les événements Solidity autorisent une abstraction en plus de la fonctionnalité de journalisation de l'EVM.
Les applications peuvent souscrire à et écouter ces événements via l'interface RPC d'un client Ethereum.

Les événements sont des membres héritables des contrats. Lorsque vous les appelez, ils font en sorte que les arguments soient stockés dans le journal des transactions - une structure de données spéciale dans la blockchain. Ces logs sont associés à l'adresse du contrat, sont incorporés dans la blockchain et y restent tant qu'un bloc est accessible (pour toujours à partir des versions Frontier et Homestead, mais cela peut changer avec Serenity). Le journal et ses données d'événement ne sont pas accessibles depuis les contrats (pas même depuis le contrat qui les a créés).

Il est possible de demander une simple vérification de paiement (SPV) pour les logs, de sorte que si une entité externe fournit un contrat avec une telle vérification, elle peut vérifier que le log existe réellement dans la blockchain. Vous devez fournir des en-têtes (headers) de bloc car le contrat ne peut voir que les 256 derniers hashs de blocs.

Vous pouvez ajouter l'attribut ``indexed`` à un maximum de trois paramètres qui les ajoute à une structure de données spéciale appelée :ref:`"topics" <abi_events>` au lieu de la partie data du log.
Si vous utilisez des types "référence" (tableaux, y compris les ``string`` et ``bytes``)
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


Le hash de la signature de l'event est l'un des topics, sauf si vous avez déclaré l'événement avec le spécificateur ``anonymous``.
Cela signifie qu'il n'est pas possible de filtrer des événements anonymes spécifiques par leur nom, you can
only filter by the contract address. The advantage of anonymous events
is that they are cheaper to deploy and call. It also allows you to declare
four indexed arguments rather than three.

.. note::
    Since the transaction log only stores the event data and not the type,
    you have to know the type of the event, including which parameter is
    indexed and if the event is anonymous in order to correctly interpret
    the data.
    In particular, it is possible to "fake" the signature of another event
    using an anonymous event.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.21 <0.9.0;

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

.. code-block:: javascript

    var abi = /* abi telle que génerée par le compilateur */;
    var ClientReceipt = web3.eth.contract(abi);
    var clientReceipt = ClientReceipt.at("0x1234...ab67" /* adresse */);

    var depositEvent = clientReceipt.Deposit();

    // inspecter les eventuels changements
    depositEvent.watch(function(error, result){
        // le résultat contient des arguments et topics non indexés
        // passées à l'appel de `Deposit`.
        if (!error)
            console.log(result);
    });


    // Ou passez une fonction pour ecouter dès maintenant
    var depositEvent = clientReceipt.Deposit(function(error, result) {
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

Additional Resources for Understanding Events
==============================================

- `Javascript documentation <https://github.com/ethereum/web3.js/blob/1.x/docs/web3-eth-contract.rst#events>`_
- `Example usage of events <https://github.com/ethchange/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>`_
- `How to access them in js <https://github.com/ethchange/smart-exchange/blob/master/lib/exchange_transactions.js>`_
