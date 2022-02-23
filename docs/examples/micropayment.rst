************************
Canaux de micro-paiement
************************

Dans cette section, nous allons apprendre comment construire une implémentation simple d'un canal de paiement. Il utilise des signatures cryptographiques pour effectuer des transferts répétés d'Ether entre les mêmes parties en toute sécurité, instantanément et sans frais de transaction. Pour ce faire, nous devons comprendre comment signer et vérifier les signatures, et configurer le canal de paiement.

Création et vérification des signatures
=======================================

Imaginez qu'Alice veuille envoyer une quantité d'Ether à Bob, c'est-à-dire qu'Alice est l'expéditeur et Bob est le destinataire.

Alice n'a qu'à envoyer des messages cryptographiquement signés hors chaîne (par exemple par e-mail) à Bob et cela sera très similaire à la rédaction de chèques.

Les signatures sont utilisées pour autoriser les transactions et sont un outil généraliste à la disposition des contrats intelligents. Alice construira un simple contrat intelligent qui lui permettra de transmettre des Ether, mais d'une manière inhabituelle, au lieu d'appeler une fonction elle-même pour initier un paiement, elle laissera Bob le faire, et donc payer les frais de transaction.

Le contrat fonctionnera comme suit :

    1. Alice déploie le contrat ``ReceiverPays`` en y attachant suffisamment d'éther pour couvrir les paiements qui seront effectués.
    2. Alice autorise un paiement en signant un message avec sa clé privée.
    3. Alice envoie le message signé cryptographiquement à Bob. Le message n'a pas besoin d'être gardé secret
       (vous le comprendrez plus tard), et le mécanisme pour l'envoyer n'a pas d'importance.
    4. Bob réclame leur paiement en présentant le message signé au contrat intelligent, il vérifie l'authenticité du message et libère ensuite les fonds.

<<<<<<< HEAD
Création de la signature
------------------------

Alice n'a pas besoin d'interagir avec le réseau Ethereum pour
signer la transaction, le processus est complètement hors ligne.
Dans ce tutoriel, nous allons signer les messages dans le navigateur
en utilisant `web3.js <https://github.com/ethereum/web3.js>`_ et `MetaMask <https://metamask.io>`_.
En particulier, nous utiliserons la méthode standard décrite dans `EIP-762 <https://github.com/ethereum/EIPs/pull/712>`_,
car elle offre un certain nombre d'autres avantages en matière de sécurité.
=======
    1. Alice deploys the ``ReceiverPays`` contract, attaching enough Ether to cover the payments that will be made.
    2. Alice authorises a payment by signing a message with her private key.
    3. Alice sends the cryptographically signed message to Bob. The message does not need to be kept secret
       (explained later), and the mechanism for sending it does not matter.
    4. Bob claims his payment by presenting the signed message to the smart contract, it verifies the
       authenticity of the message and then releases the funds.

Creating the signature
----------------------

Alice does not need to interact with the Ethereum network
to sign the transaction, the process is completely offline.
In this tutorial, we will sign messages in the browser
using `web3.js <https://github.com/ethereum/web3.js>`_ and
`MetaMask <https://metamask.io>`_, using the method described in `EIP-712 <https://github.com/ethereum/EIPs/pull/712>`_,
as it provides a number of other security benefits.
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

.. code-block:: javascript

    /// Hasher d'abord simplifie un peu les choses
    var hash = web3.sha3("message to sign");
    web3.personal.sign(hash, web3.eth.defaultAccount, function () {...});

.. note::
<<<<<<< HEAD
  Notez que ``web3.personal.sign`` préfixe les données signées de la longueur du message.
  Mais comme nous avons hashé en premier, le message sera toujours exactement 32 octets de long,
  et donc ce préfixe de longueur est toujours le même, ce qui facilite tout.

Que signer
----------

Dans le cas d'un contrat qui effectue des paiements, le message signé doit inclure :

     1. Adresse du destinataire
     2. le montant à transférer
     3. Protection contre les attaques de rediffusion

Une attaque de rediffusion se produit lorsqu'un message signé est réutilisé pour revendiquer l'autorisation pour
une deuxième action.
Pour éviter les attaques par rediffusion, nous utiliserons la même méthode que pour les transactions Ethereum
elles-mêmes, ce qu'on appelle un nonce, qui est le nombre de transactions envoyées par un
compte.
Le contrat intelligent vérifiera si un nonce est utilisé plusieurs fois.

Il existe un autre type d'attaques de redifussion, il se produit lorsque
le propriétaire déploie un smart contract ``ReceiverPays``, effectue certains paiements,
et ensuite détruit le contrat. Plus tard, il décide de déployer
``ReceiverPays`` encore une fois, mais le nouveau contrat ne peut pas
connaître les nonces utilisés dans le déploiement précédent, donc l'attaquant
peut réutiliser les anciens messages.

Alice peut s'en protéger, notamment en incluant
l'adresse du contrat dans le message, et seulement
les messages contenant l'adresse du contrat lui-même seront acceptés.
Cette fonctionnalité se trouve dans les deux premières lignes de la fonction ``claimPayment()`` du contrat complet
à la fin de ce chapitre.

Encoder les arguments
---------------------

Maintenant que nous avons déterminé quelles informations inclure dans le message signé,
nous sommes prêts à assembler le message, à le hacher,
et le signer. Par souci de simplicité, nous ne faisons que concaténer les données.
La bibliothèque
`ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_ fournit
une fonction appelée ``soliditySHA3`` qui imite le comportement
de la fonction ``keccak256`` de Solidity appliquée aux arguments codés
en utilisant ``abi.encododePacked``.
En résumé, voici une fonction JavaScript qui
crée la signature appropriée pour l'exemple ``ReceiverPays`` :
=======
  The ``web3.eth.personal.sign`` prepends the length of the
  message to the signed data. Since we hash first, the message
  will always be exactly 32 bytes long, and thus this length
  prefix is always the same.

What to Sign
------------

For a contract that fulfils payments, the signed message must include:

    1. The recipient's address.
    2. The amount to be transferred.
    3. Protection against replay attacks.

A replay attack is when a signed message is reused to claim
authorization for a second action. To avoid replay attacks
we use the same technique as in Ethereum transactions themselves,
a so-called nonce, which is the number of transactions sent by
an account. The smart contract checks if a nonce is used multiple times.

Another type of replay attack can occur when the owner
deploys a ``ReceiverPays`` smart contract, makes some
payments, and then destroys the contract. Later, they decide
to deploy the ``RecipientPays`` smart contract again, but the
new contract does not know the nonces used in the previous
deployment, so the attacker can use the old messages again.

Alice can protect against this attack by including the
contract's address in the message, and only messages containing
the contract's address itself will be accepted. You can find
an example of this in the first two lines of the ``claimPayment()``
function of the full contract at the end of this section.

Packing arguments
-----------------

Now that we have identified what information to include in the signed message,
we are ready to put the message together, hash it, and sign it. For simplicity,
we concatenate the data. The `ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_
library provides a function called ``soliditySHA3`` that mimics the behaviour of
Solidity's ``keccak256`` function applied to arguments encoded using ``abi.encodePacked``.
Here is a JavaScript function that creates the proper signature for the ``ReceiverPays`` example:
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

.. code-block:: javascript

    // recipient est l'addresse à payer.
    // amount, en wei, spécifie combien d'Ether doivent être envoyés.
    // nonce peut être n'importe quel nombre unique pour prévenir les attques par redifusion
    // contractAddress est utilisé pour éviter les attaque par redifusion de messages inter-contrats
    function signPayment(recipient, amount, nonce, contractAddress, callback) {
        var hash = "0x" + ethereumjs.ABI.soliditySHA3(
            ["address", "uint256", "uint256", "address"],
            [recipient, amount, nonce, contractAddress]
        ).toString("hex");

        web3.personal.sign(hash, web3.eth.defaultAccount, callback);
    }

<<<<<<< HEAD
Récupérer le signataire du message en Solidity
----------------------------------------------

En général, les signatures ECDSA se composent de deux paramètres, ``r`` et ``s``.
Les signatures dans Ethereum incluent un troisième paramètre appelé "v", qui peut être utilisé
pour récupérer la clé privée du compte qui a été utilisée pour signer le message,
l'expéditeur de la transaction. La solidité offre une fonction intégrée
`ecrecover <fonctions-mathématiques et cryptographiques>`_
qui accepte un message avec les paramètres ``r``, ``s`` et ``v`` et
renvoie l'adresse qui a été utilisée pour signer le message.

Extraire les paramètres de signature
------------------------------------

Les signatures produites par web3.js sont la concaténation de ``r``, ``s`` et ``v``,
donc la première étape est de re-séparer ces paramètres. Cela peut être fait sur le client,
mais le faire à l'intérieur du smart contract signifie qu'un seul paramètre de signature
peut être envoyé au lieu de trois.
Diviser un tableau d'octets en plusieurs parties est un peu compliqué.
Nous utiliserons l'`assembleur en ligne <assembly>`_ pour faire le travail
dans la fonction ``splitSignature`` (la troisième fonction dans le contrat complet
à la fin du présent chapitre).

Calculer le hash du message
---------------------------
=======
Recovering the Message Signer in Solidity
-----------------------------------------

In general, ECDSA signatures consist of two parameters,
``r`` and ``s``. Signatures in Ethereum include a third
parameter called ``v``, that you can use to verify which
account's private key was used to sign the message, and
the transaction's sender. Solidity provides a built-in
function :ref:`ecrecover <mathematical-and-cryptographic-functions>` that
accepts a message along with the ``r``, ``s`` and ``v`` parameters
and returns the address that was used to sign the message.

Extracting the Signature Parameters
-----------------------------------

Signatures produced by web3.js are the concatenation of ``r``,
``s`` and ``v``, so the first step is to split these parameters
apart. You can do this on the client-side, but doing it inside
the smart contract means you only need to send one signature
parameter rather than three. Splitting apart a byte array into
its constituent parts is a mess, so we use
:doc:`inline assembly <assembly>` to do the job in the ``splitSignature``
function (the third function in the full contract at the end of this section).
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

Le smart contract doit savoir exactement quels paramètres ont été signés,
et doit donc recréer le message à partir des paramètres et utiliser cette fonction
pour la vérification des signatures. Les fonctions ``prefixed`` et
``recoverSigner`` s'occupent de cela et leur utilisation peut se trouver
dans la fonction ``claimPayment``.


Le contrat complet
------------------

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract ReceiverPays {
        address owner = msg.sender;

        mapping(uint256 => bool) usedNonces;

        constructor() payable {}

        function claimPayment(uint256 amount, uint256 nonce, bytes memory signature) external {
            require(!usedNonces[nonce]);
            usedNonces[nonce] = true;

            // Cette ligne recrée le message signé par le client
            bytes32 message = prefixed(keccak256(abi.encodePacked(msg.sender, amount, nonce, this)));

            require(recoverSigner(message, signature) == owner);

            payable(msg.sender).transfer(amount);
        }

<<<<<<< HEAD
        /// détruit le contrat et réclame son solde.
        function shutdown() public {
=======
        /// destroy the contract and reclaim the leftover funds.
        function shutdown() external {
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
            require(msg.sender == owner);
            selfdestruct(payable(msg.sender));
        }

        /// methodes de signature.
        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // premiers 32 octets, après le préfixe
                r := mload(add(sig, 32))
                // 32 octets suivants
                s := mload(add(sig, 64))
                // Octet final (premier du prochain lot de 32)
                v := byte(0, mload(add(sig, 96)))
            }

            return (v, r, s);
        }

        function recoverSigner(bytes32 message, bytes memory sig)
            internal
            pure
            returns (address)
        {
            (uint8 v, bytes32 r, bytes32 s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// construit un hash préfixé pour mimer le comportement de eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


Écrire un canal de paiement simple
==================================

Alice va maintenant construire une implémentation simple mais complète d'un canal de paiement.
Les canaux de paiement utilisent des signatures cryptographiques pour effectuer des virements répétés
d'Ether en toute sécurité, instantanément et sans frais de transaction.

Qu'est-ce qu'un canal de paiement ?
-----------------------------------

Les canaux de paiement permettent aux participants d'effectuer des transferts répétés d'Ether sans
utiliser de transactions. Cela signifie que les délais et frais associés aux transactions
peuvent être évités. Nous allons explorer un canal de paiement unidirectionnel simple entre
deux parties (Alice et Bob). Son utilisation implique trois étapes :

<<<<<<< HEAD
     1. Alice déploie un smart contract avec de l'Ether. Cela "ouvre" (``opens``) le canal de paiement.
     2. Alice signe des messages qui précisent combien d'éther est dû au destinataire. Cette étape est répétée pour chaque paiement.
     3. Bob "ferme" (``closes``) le canal de paiement, retirant leur part d'Ether et renvoyant le reste à l'expéditeur.

.. note::
  Non seulement les étapes 1 et 3 exigent des transactions Ethereum, mais l'étape 2 signifie que
  l'expéditeur transmet un message signé cryptographiquement au destinataire par des moyens hors chaîne (par exemple, par courrier électronique).
  Cela signifie que seulement deux transactions sont nécessaires pour traiter un nombre quelconque de transferts.

Bob est assuré de recevoir ses fonds parce que le contrat bloque les fonds en
Ether et respecte des ordres valides et signés. Le smart contract impose également un délai d'attente,
Alice est donc assurée de recouvrer ses fonds même si le bénéficiaire refuse
de fermer le canal.
C'est aux participants d'un canal de paiement de décider combien de temps il doit rester ouvert.
Pour une transaction de courte durée, comme payer un cybercafé pour chaque minute d'accès au réseau,
ou dans le cas d'une relation de plus longue durée, comme le versement d'un salaire horaire à un employé, un paiement pourrait durer des mois ou des années.

Ouverture du canal de paiement
------------------------------
=======
    1. Alice funds a smart contract with Ether. This "opens" the payment channel.
    2. Alice signs messages that specify how much of that Ether is owed to the recipient. This step is repeated for each payment.
    3. Bob "closes" the payment channel, withdrawing his portion of the Ether and sending the remainder back to the sender.

.. note::
  Only steps 1 and 3 require Ethereum transactions, step 2 means that the sender
  transmits a cryptographically signed message to the recipient via off chain
  methods (e.g. email). This means only two transactions are required to support
  any number of transfers.

Bob is guaranteed to receive his funds because the smart contract escrows the
Ether and honours a valid signed message. The smart contract also enforces a
timeout, so Alice is guaranteed to eventually recover her funds even if the
recipient refuses to close the channel. It is up to the participants in a payment
channel to decide how long to keep it open. For a short-lived transaction,
such as paying an internet café for each minute of network access, the payment
channel may be kept open for a limited duration. On the other hand, for a
recurring payment, such as paying an employee an hourly wage, the payment channel
may be kept open for several months or years.

Opening the Payment Channel
---------------------------
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04

Pour ouvrir le canal de paiement, Alice déploie le contrat,
y attachant de l'Ether en dépot et spécifiant le destinataire prévu,
ainsi qu'une durée de vie maximale du canal. C'est la fonction
``SimplePaymentChannel`` dans le contrat.

Effectuer des paiements
-----------------------

Alice effectue des paiements en envoyant des messages signés à Bob.
Cette étape est entièrement réalisée en dehors du réseau Ethereum.
Les messages sont signés cryptographiquement par l'expéditeur puis transmis directement au destinataire.

Chaque message contient les informations suivantes :

     * L'adresse du contrat, utilisé pour empêcher les attaques de redifussion par contrats croisés.
     * Le montant total d'Ether qui est dû au bénéficiaire jusqu'alors.

Un canal de paiement est fermé une seule fois, à la fin d'une série de virements.
De ce fait, un seul des messages envoyés sera échangé. C'est pourquoi
chaque message spécifie un montant total cumulatif d'éther dû, plutôt que le montant total
d'un micropaiement individuel. Le destinataire réclamera naturellement
le message le plus récent parce que c'est celui dont le total est le plus élevé.
Le nonce par message n'est plus nécessaire, car le smart contract ne va
honorer qu'un seul message. L'adresse du contrat intelligent est toujours utilisée
pour éviter qu'un message destiné à un canal de paiement ne soit utilisé pour un autre canal.

Voici le code javascript modifié pour signer cryptographiquement un message du chapitre précédent :

.. code-block:: javascript

    function constructPaymentMessage(contractAddress, amount) {
        return abi.soliditySHA3(
            ["address", "uint256"],
            [contractAddress, amount]
        );
    }

    function signMessage(message, callback) {
        web3.eth.personal.sign(
            "0x" + message.toString("hex"),
            web3.eth.defaultAccount,
            callback
        );
    }

    // contractAddress détectera la rediffusion de messages à d'autres contrats.
    // amount, en wei, précise combien d'Ether doivent être envoyés.

    function signPayment(contractAddress, amount, callback) {
        var message = constructPaymentMessage(contractAddress, amount);
        signMessage(message, callback);
    }


Fermeture du canal de paiement
------------------------------

Lorsque Bob est prêt à recevoir leurs ses, il est temps de
fermer le canal de paiement en appelant une fonction ``close`` sur le smart contract.
La fermeture du canal paie au destinataire l'Ether qui lui est dû et détruit le contrat,
en renvoyant tout Ether restant à Alice.
Pour fermer le canal, Bob doit fournir un message signé par Alice.

Le contrat doit vérifier que le message contient une signature valide de l'expéditeur.
Le processus de vérification est le même que celui utilisé par le destinataire.
Les fonctions Solidity ``isValidSignature`` et ``recoverSigner`` fonctionnent de la même manière que leurs fonctions
JavaScript dans la section précédente. Ce dernier est emprunté au
Le contrat ``ReceiverPays`` du chapitre précédent.

La fonction ``close`` ne peut être appelée que par le destinataire du canal de paiement,
qui enverra naturellement le message de paiement le plus récent car c'est celui qui comporte
le plus haut total dû. Si l'expéditeur était autorisé à appeler cette fonction,
il pourrait fournir un message avec un montant inférieur et escroquer le destinataire de ce qui lui est dû.

La fonction vérifie que le message signé correspond aux paramètres donnés.
Si tout se passe bien, le destinataire reçoit sa part d'Ether,
et l'expéditeur reçoit le reste par ``selfdestruct`` (autodestruction) du contrat.
Vous pouvez voir la fonction ``close`` dans le contrat complet.

Expiration du canal
-------------------

Bob peut fermer le canal de paiement à tout moment, mais s'il ne le fait pas,
Alice a besoin d'un moyen de récupérer les fonds bloqués. Une durée d'*expiration* a été définie
au moment du déploiement du contrat. Une fois cette heure atteinte, Alice peut appeler
pour récupérer leurs fonds. Vous pouvez voir la fonction ``claimTimeout`` dans le
contrat déployé.

Après l'appel de cette fonction, Bob ne peut plus recevoir d'Ether.
Il est donc important que Bob ferme le canal avant que l'expiration ne soit atteinte.


Le contrat complet
-----------------

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    contract SimplePaymentChannel {
        address payable public sender;      // Le compte emvoyant les paiements.
        address payable public recipient;   // Le compte destinataire des paiements.
        uint256 public expiration;  // Expitration si le destinataire ne clot pas le canal.


        constructor (address payable recipientAddress, uint256 duration)
            payable
        {
            sender = payable(msg.sender);
            recipient = recipientAddress;
            expiration = block.timestamp + duration;
        }

        /// Le destinataire peut clore le canal à tout moment en présentant le dernier montant
        /// signé par l'expéditeur des fonds. Le destinataire se verra verser ce montant,
        /// et le reste sera rendu à l'emetteur des fonds.
        function close(uint256 amount, bytes memory signature) external {
            require(msg.sender == recipient);
            require(isValidSignature(amount, signature));

            recipient.transfer(amount);
            selfdestruct(sender);
        }

        /// L'emetteur peut modifier la date d'expiration à tout moment
        function extend(uint256 newExpiration) external {
            require(msg.sender == sender);
            require(newExpiration > expiration);

            expiration = newExpiration;
        }

        /// Si l'expiration est atteinte avant cloture par le destinataire,
        /// l'Ether est renvoyé à l'emetteur
        function claimTimeout() external {
            require(now >= expiration);
            selfdestruct(sender);
        }

        function isValidSignature(uint256 amount, bytes memory signature)
            internal
            view
            returns (bool)
        {
            bytes32 message = prefixed(keccak256(abi.encodePacked(this, amount)));

            // check that the signature is from the payment sender
            return recoverSigner(message, signature) == sender;
        }

        /// Toutes les fonctions ci-dessous sont tirées
        /// du chapitre 'créer et vérifier les signatures'.

        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // first 32 bytes, after the length prefix
                r := mload(add(sig, 32))
                // second 32 bytes
                s := mload(add(sig, 64))
                // final byte (first byte of the next 32 bytes)
                v := byte(0, mload(add(sig, 96)))
            }

            return (v, r, s);
        }

        function recoverSigner(bytes32 message, bytes memory sig)
            internal
            pure
            returns (address)
        {
            (uint8 v, bytes32 r, bytes32 s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// construit un hash préfixé pour mimer le comportement de eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


.. note::
    La fonction ``splitSignature`` est très simple et n'utilise pas tous les contrôles de sécurité.
    Une implémentation réelle devrait utiliser une bibliothèque plus rigoureusement testée de ce code, tel que le fait openzepplin avec `version  <https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/utils/cryptography/ECDSA.sol>`_ of this code.

Vérification des paiements
--------------------------

Contrairement à notre chapitre précédent, les messages dans un canal de paiement ne sont pas
appliqués tout de suite. Le destinataire conserve la trace du dernier message et
le fait parvenir au réseau quand il est temps de fermer le canal de paiement. Cela signifie qu'il
est essentiel que le destinataire effectue sa propre vérification de chaque message.
Sinon, il n'y a aucune garantie que le destinataire sera en mesure d'être payé à la fin.

Le destinataire doit vérifier chaque message à l'aide du processus suivant :

     1. Vérifiez que l'adresse du contact dans le message correspond au canal de paiement.
     2. Vérifiez que le nouveau total est le montant prévu.
     3. Vérifier que le nouveau total ne dépasse pas la quantité d'éther déposée.
     4. Vérifiez que la signature est valide et provient de l'expéditeur du canal de paiement.

Nous utiliserons la librairie `ethereumjs-util <https://github.com/ethereumjs/ethereumjs-util>`_
pour écrire ces vérifications. L'étape finale peut se faire de plusieurs façons,
ici en JavaScript,
Le code suivant emprunte la fonction `constructPaymentMessage` du **code JavaScript** de signature
ci-dessous :

.. code-block:: javascript

    // Cette ligne mine le fonctionnement de la méthode JSON-RPC de eth_sign.
    function prefixed(hash) {
        return ethereumjs.ABI.soliditySHA3(
            ["string", "bytes32"],
            ["\x19Ethereum Signed Message:\n32", hash]
        );
    }

    function recoverSigner(message, signature) {
        var split = ethereumjs.Util.fromRpcSig(signature);
        var publicKey = ethereumjs.Util.ecrecover(message, split.v, split.r, split.s);
        var signer = ethereumjs.Util.pubToAddress(publicKey).toString("hex");
        return signer;
    }

    function isValidSignature(contractAddress, amount, signature, expectedSigner) {
        var message = prefixed(constructPaymentMessage(contractAddress, amount));
        var signer = recoverSigner(message, signature);
        return signer.toLowerCase() ==
            ethereumjs.Util.stripHexPrefix(expectedSigner).toLowerCase();
    }
