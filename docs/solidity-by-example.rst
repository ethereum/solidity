######################
Solidity par l'Exemple
######################

.. index:: voting, ballot

.. _voting:

****
Vote
****

Le contrat suivant est assez complexe, mais il présente de nombreuses caractéristiques de Solidity. Il implémente un contrat de vote. Bien entendu, le principal problème du vote électronique est de savoir comment attribuer les droits de vote aux bonnes personnes et éviter les manipulations. Nous ne résoudrons pas tous les problèmes ici, mais nous montrerons au moins comment le vote délégué peut être effectué de manière à ce que le dépouillement soit à la fois **automatique et totalement transparent**.

L'idée est de créer un contrat par bulletin de vote, en donnant un nom court à chaque option.
Ensuite, le créateur du contrat qui agit à titre de président donnera le droit de vote à chaque adresse individuellement.

Les personnes derrière les adresses peuvent alors choisir de voter elles-mêmes ou de déléguer leur vote à une personne en qui elles ont confiance.

A la fin du temps de vote, la ``winningProposal()`` (proposition gagnante) retournera la proposition avec le plus grand nombre de votes.

::

    pragma solidity >=0.4.22 <0.6.0;

    /// @title Vote par délegation.
    contract Ballot {
        // Ceci déclare un type complexe, représentant
        // un votant, qui sera utilisé
        // pour les variables plus tard.
        struct Voter {
            uint weight; // weight (poids), qui s'accumule avec les délégations
            bool voted;  // si true, cette personne a déjà voté
            address delegate; // Cette personne a délégué son vote à
            uint vote;   // index la la proposition choisie
        }

        // Type pour une proposition.
        struct Proposal {
            bytes32 name;   // nom court (jusqu'à 32 octets)
            uint voteCount; // nombre de votes cumulés
        }

        address public chairperson;

        // Ceci déclare une variable d'état qui stocke
        // un élément de structure 'Voters' pour  chaque votant.
        mapping(address => Voter) public voters;

        // Un tableau dynamique de structs `Proposal`.
        Proposal[] public proposals;

        /// Créé un nouveau bulletin pour choisir l'un des `proposalNames`.
        constructor(bytes32[] memory proposalNames) public {
            chairperson = msg.sender;
            voters[chairperson].weight = 1;

            // Pour chacun des noms proposés,
            // crée un nouvel objet proposal
            // à la fin du tableau.
            for (uint i = 0; i < proposalNames.length; i++) {
                // `Proposal({...})` créé un objet temporaire
                // Proposal et `proposals.push(...)`
                // l'ajoute à la fin du tableau `proposals`.
                proposals.push(Proposal({
                    name: proposalNames[i],
                    voteCount: 0
                }));
            }
        }

        // Donne à un `voter` un droit de vote pour ce scrutin.
        // Peut seulement être appelé par `chairperson`.
        function giveRightToVote(address voter) public {
            // Si le premier argument passé à `require` s'évalue
            // à `false`, l'exécution s'arrete et tous les changements
            // à l'état et aux soldes sont annulés.
            // Cette opération consommait tout le gas dans
            // d'anciennes versions de l'EVM, plus maintenant.
            // Il est souvent une bonne idée d'appeler `require` 
            // pour vérifier si les appels de fonctions
            // s'effectuent correctement.
            // Comme second argument, vous pouvez fournir une
            // phrase explicative de ce qui est allé de travers.
            require(
                msg.sender == chairperson,
                "Only chairperson can give right to vote."
            );
            require(
                !voters[voter].voted,
                "The voter already voted."
            );
            require(voters[voter].weight == 0);
            voters[voter].weight = 1;
        }

        /// Delegue son vote au votant `to`.
        function delegate(address to) public {
            // assigne les références
            Voter storage sender = voters[msg.sender];
            require(!sender.voted, "You already voted.");

            require(to != msg.sender, "Self-delegation is disallowed.");

            // Relaie la délégation tant que `to`
            // est également en délégation de vote.
            // En général, ce type de boucles est très dangereux,
            // puisque s'il tourne trop longtemps, l'opération
            // pourrait demander plus de gas qu'il n'est possible
            // d'en avoir dans un bloc.
            // Dans ce cas, la délégation ne se ferait pas,
            // mais dans d'autres circonstances, ces boucles
            // peuvent complètement paraliser un contrat.
            while (voters[to].delegate != address(0)) {
                to = voters[to].delegate;

                // On a trouvé une boucle dans la chaine
                // de délégations => interdit.
                require(to != msg.sender, "Found loop in delegation.");
            }

            // Comme `sender` est une référence, ceci
            // modifie `voters[msg.sender].voted`
            sender.voted = true;
            sender.delegate = to;
            Voter storage delegate_ = voters[to];
            if (delegate_.voted) {
                // Si le délégué a déjà voté,
                // on ajoute directement le vote aux autres
                proposals[delegate_.vote].voteCount += sender.weight;
            } else {
                // Sinon, on l'ajoute au poids de son vote.
                delegate_.weight += sender.weight;
            }
        }

        /// Voter (incluant les procurations par délégation)
        /// pour la proposition `proposals[proposal].name`.
        function vote(uint proposal) public {
            Voter storage sender = voters[msg.sender];
            require(!sender.voted, "Already voted.");
            sender.voted = true;
            sender.vote = proposal;

            // Si `proposal` n'est pas un index valide,
            // une erreur sera levée et l'exécution annulée
            proposals[proposal].voteCount += sender.weight;
        }

        /// @dev Calcule la proposition gagnante
        /// en prenant tous les votes précédents en compte.
        function winningProposal() public view
                returns (uint winningProposal_)
        {
            uint winningVoteCount = 0;
            for (uint p = 0; p < proposals.length; p++) {
                if (proposals[p].voteCount > winningVoteCount) {
                    winningVoteCount = proposals[p].voteCount;
                    winningProposal_ = p;
                }
            }
        }

        // Appelle la fonction winningProposal() pour avoir
        // l'index du gagnant dans le tableau de propositions
        // et retourne le nom de la proposition gagnante.
        function winnerName() public view
                returns (bytes32 winnerName_)
        {
            winnerName_ = proposals[winningProposal()].name;
        }
    }


Améliorations possibles
=======================

À l'heure actuelle, de nombreuses opérations sont nécessaires pour attribuer les droits de vote à tous les participants. Pouvez-vous trouver un meilleur moyen ?
.. index:: auction;blind, auction;open, blind auction, open auction

********************
Enchères à l'aveugle
********************

Dans cette section, nous allons montrer à quel point il est facile de créer un contrat d'enchères à l'aveugle sur Ethereum. Nous commencerons par une enchère ouverte où tout le monde pourra voir les offres qui sont faites, puis nous prolongerons ce contrat dans une enchère aveugle où il n'est pas possible de voir l'offre réelle avant la fin de la période de soumission.

.. _simple_auction:

Enchère ouverte simple
======================

L'idée générale du contrat d'enchère simple suivant est que chacun peut envoyer ses offres pendant une période d'enchère. Les ordres incluent l'envoi d'argent / éther afin de lier les soumissionnaires à leur offre. Si l'enchère est la plus haute, l'enchérisseur qui avait fait l'offre la plus élevée auparavant récupère son argent. Après la fin de la période de soumission, le contrat doit être appelé manuellement pour que le bénéficiaire reçoive son argent - les contrats ne peuvent pas s'activer eux-mêmes.

::

    pragma solidity >=0.4.22 <0.6.0;

    contract SimpleAuction {
        // Paramètres de l'enchère
        // temps unix absolus (secondes depuis 01-01-1970)
        // ou des durées en secondes.
        address payable public beneficiary;
        uint public auctionEndTime;

        // État actuel de l'enchère.
        address public highestBidder;
        uint public highestBid;

        // Remboursements autorisés d'enchères précédentes
        mapping(address => uint) pendingReturns;

        // Mis à true à la fin, interdit tout changement.
        // Par defaut à `false`, comme un grand.
        bool ended;

        // Évènements déclenchés aux changements.
        event HighestBidIncreased(address bidder, uint amount);
        event AuctionEnded(address winner, uint amount);

        // Ce ui suit est appelé commentaire natspec,
        // reconaissable à ses 3 slashes.
        // Ce message sera affiché quand l'utilisateur
        // devra confirmer une transaction.

        /// Créée une enchère simple de `_biddingTime`
        /// secondes au profit de l'addresse
        /// beneficaire address `_beneficiary`.
        constructor(
            uint _biddingTime,
            address payable _beneficiary
        ) public {
            beneficiary = _beneficiary;
            auctionEndTime = now + _biddingTime;
        }

        /// Faire une offre avec la valeur envoyée
        /// avec cette transaction.
        /// La valeur ne sera remboursée que si 
        // l'enchère est perdue.
        function bid() public payable {
            // Aucun argument n'est nécessaire, toute
            // l'information fait déjà partie
            // de la transaction. Le mot-clé payable
            // est requis pour autoriser la fonction
            // à recevoir de l'Ether.

            // Annule l'appel si l'enchère est termminée
            require(
                now <= auctionEndTime,
                "Auction already ended."
            );

            // Rembourse si l'enchère est trop basse
            require(
                msg.value > highestBid,
                "There already is a higher bid."
            );

            if (highestBid != 0) {
                // Renvoyer l'argent avec un simple
                // highestBidder.send(highestBid) est un risque de sécurité
                // car ça pourrait déclencher un appel à un contrat.
                // Il est toujours plus sûr de laisser les utilisateurs
                // retirer leur argent eux-mêmes.
                pendingReturns[highestBidder] += highestBid;
            }
            highestBidder = msg.sender;
            highestBid = msg.value;
            emit HighestBidIncreased(msg.sender, msg.value);
        }

        /// Retirer l'argent d'une enchère dépassée
        function withdraw() public returns (bool) {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // Il est important de mettre cette valeur à zéro car l'utilisateur
                // pourrait rappeler cette fonction avant le retour de `send`.
                pendingReturns[msg.sender] = 0;

                if (!msg.sender.send(amount)) {
                    // Pas besoin d'avorter avec un throw ici, juste restaurer le montant
                    pendingReturns[msg.sender] = amount;
                    return false;
                }
            }
            return true;
        }

        /// Met fin à l'enchère et envoie
        /// le montant de l'enchère la plus haute au bénéficiaire.
        function auctionEnd() public {
            // C'est une bonne pratique de structurer les fonctions qui
            // intéragissent avec d'autres contrats (appellent des
            // fonctions ou envoient de l'Ether) en trois phases:
            // 1. Vérifier les conditions
            // 2. éffectuer les actions (potentiellement changeant les conditions)
            // 3. interagir avec les autres contrats
            // Si ces phases sont mélangées, l'autre contrat pourrait rappeler
            // le contrat courant et modifier l'état ou causer des effets
            // (paiements en Ether par ex) qui se produiraient plusieurs fois.
            // Si des fonctions appelées en interne effectuent des appels 
            // à des contrats externes, elles doivent aussi êtres considérées
            // comme concernées par cette norme.

            // 1. Conditions
            require(now >= auctionEndTime, "Auction not yet ended.");
            require(!ended, "auctionEnd has already been called.");

            // 2. Éffets
            ended = true;
            emit AuctionEnded(highestBidder, highestBid);

            // 3. Interaction
            beneficiary.transfer(highestBid);
        }
    }

Enchère aveugle
===============

L'enchère ouverte précédente est étendue en une enchère aveugle dans ce qui suit. L'avantage d'une enchère aveugle est qu'il n'y a pas de pression temporelle vers la fin de la période de soumission. La création d'une enchère aveugle sur une plate-forme informatique transparente peut sembler une contradiction, mais la cryptographie vient à la rescousse.

Pendant la **période de soumission**, un soumissionnaire n'envoie pas son offre, mais seulement une version hachée de celle-ci. Puisqu'il est actuellement considéré comme pratiquement impossible de trouver deux valeurs (suffisamment longues) dont les valeurs de hachage sont égales, le soumissionnaire s'engage à l'offre par cela. Après la fin de la période de soumission, les soumissionnaires doivent révéler leurs offres : Ils envoient leurs valeurs en clair et le contrat vérifie que la valeur de hachage est la même que celle fournie pendant la période de soumission.

Un autre défi est de savoir comment rendre l'enchère contraignante et aveugle en même temps : La seule façon d'éviter que l'enchérisseur n'envoie pas l'argent après avoir gagné l'enchère est de le lui faire envoyer avec l'enchère. Puisque les transferts de valeur ne peuvent pas être aveuglés dans Ethereum, tout le monde peut voir la valeur.

Le contrat suivant résout ce problème en acceptant toute valeur supérieure à l'offre la plus élevée. Comme cela ne peut bien sûr être vérifié que pendant la phase de révélation, certaines offres peuvent être invalides, et c'est fait exprès (il fournit même un marqueur explicite pour placer des offres invalides avec des transferts de grande valeur) : Les soumissionnaires peuvent brouiller la concurrence en plaçant plusieurs offres invalides hautes ou basses.


::

    pragma solidity >0.4.23 <0.6.0;

    contract BlindAuction {
        struct Bid {
            bytes32 blindedBid;
            uint deposit;
        }

        address payable public beneficiary;
        uint public biddingEnd;
        uint public revealEnd;
        bool public ended;

        mapping(address => Bid[]) public bids;

        address public highestBidder;
        uint public highestBid;

        // Remboursements autorisés d'enchères précédentes
        mapping(address => uint) pendingReturns;

        event AuctionEnded(address winner, uint highestBid);

        /// Les Modifiers sont une façon pratique de valider des entrées.
        /// `onlyBefore` est appliqué à `bid` ci-dessous:
        /// Le corps de la fonction sera placé dans le modifier
        /// où `_` est placé.
        modifier onlyBefore(uint _time) { require(now < _time); _; }
        modifier onlyAfter(uint _time) { require(now > _time); _; }

        constructor(
            uint _biddingTime,
            uint _revealTime,
            address payable _beneficiary
        ) public {
            beneficiary = _beneficiary;
            biddingEnd = now + _biddingTime;
            revealEnd = biddingEnd + _revealTime;
        }

        /// Placer une enchère à l'aveugle avec `_blindedBid` =
        /// keccak256(abi.encodePacked(value, fake, secret)).
        ///  L'éther envoyé n'est remboursé que si l'enchère est correctement
        /// révélée dans la phase de révélation. L'offre est valide si
        /// l'éther envoyé avec l'offre est d'au moins "valeur" et
        /// "fake" n'est pas true. Régler "fake" à true et envoyer
        /// envoyer un montant erroné sont des façons de masquer l'enchère
        /// mais font toujours le dépot requis. La même addresse peut placer
        /// plusieurs ordres
        function bid(bytes32 _blindedBid)
            public
            payable
            onlyBefore(biddingEnd)
        {
            bids[msg.sender].push(Bid({
                blindedBid: _blindedBid,
                deposit: msg.value
            }));
        }

        /// Révèle vos ench1eres aveugles. Vous serez remboursé pour toutes
        /// les enchères invalides et toutes les autres exceptée la plus haute
        /// le cas échéant.
        function reveal(
            uint[] memory _values,
            bool[] memory _fake,
            bytes32[] memory _secret
        )
            public
            onlyAfter(biddingEnd)
            onlyBefore(revealEnd)
        {
            uint length = bids[msg.sender].length;
            require(_values.length == length);
            require(_fake.length == length);
            require(_secret.length == length);

            uint refund;
            for (uint i = 0; i < length; i++) {
                Bid storage bidToCheck = bids[msg.sender][i];
                (uint value, bool fake, bytes32 secret) =
                        (_values[i], _fake[i], _secret[i]);
                if (bidToCheck.blindedBid != keccak256(abi.encodePacked(value, fake, secret))) {
                    // L'enchère n'a pas été révélée.
                    // Ne pas rembourser.
                    continue;
                }
                refund += bidToCheck.deposit;
                if (!fake && bidToCheck.deposit >= value) {
                    if (placeBid(msg.sender, value))
                        refund -= value;
                }
                // Rendre impossible un double remboursement
                bidToCheck.blindedBid = bytes32(0);
            }
            msg.sender.transfer(refund);
        }

        // Cette fonction interne ("internal") ne peut être appelée que
        // que depuis l'intérieur du contrat (ou ses contrats dérivés).
        function placeBid(address bidder, uint value) internal
                returns (bool success)
        {
            if (value <= highestBid) {
                return false;
            }
            if (highestBidder != address(0)) {
                // Rembourse la précédent leader.
                pendingReturns[highestBidder] += highestBid;
            }
            highestBid = value;
            highestBidder = bidder;
            return true;
        }

        /// Se faire rembourser une enchère battue.
        function withdraw() public {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // Il est important de mettre cette valeur à zéro car l'utilisateur
                // pourrait rappeler cette fonction avant le retour de `send`.
                // (voir remarque sur conditions -> effets -> interaction).
                pendingReturns[msg.sender] = 0;

                msg.sender.transfer(amount);
            }
        }

        /// Met fin à l'enchère et envoie
        /// le montant de l'enchère la plus haute au bénéficiaire.
        function auctionEnd()
            public
            onlyAfter(revealEnd)
        {
            require(!ended);
            emit AuctionEnded(highestBidder, highestBid);
            ended = true;
            beneficiary.transfer(highestBid);
        }
    }


.. index:: purchase, remote purchase, escrow

********************
Safe Remote Purchase
********************

::

    pragma solidity >=0.4.22 <0.6.0;

    contract Purchase {
        uint public value;
        address payable public seller;
        address payable public buyer;
        enum State { Created, Locked, Inactive }
        State public state;

        // Ensure that `msg.value` is an even number.
        // Division will truncate if it is an odd number.
        // Check via multiplication that it wasn't an odd number.
        constructor() public payable {
            seller = msg.sender;
            value = msg.value / 2;
            require((2 * value) == msg.value, "Value has to be even.");
        }

        modifier condition(bool _condition) {
            require(_condition);
            _;
        }

        modifier onlyBuyer() {
            require(
                msg.sender == buyer,
                "Only buyer can call this."
            );
            _;
        }

        modifier onlySeller() {
            require(
                msg.sender == seller,
                "Only seller can call this."
            );
            _;
        }

        modifier inState(State _state) {
            require(
                state == _state,
                "Invalid state."
            );
            _;
        }

        event Aborted();
        event PurchaseConfirmed();
        event ItemReceived();

        /// Abort the purchase and reclaim the ether.
        /// Can only be called by the seller before
        /// the contract is locked.
        function abort()
            public
            onlySeller
            inState(State.Created)
        {
            emit Aborted();
            state = State.Inactive;
            seller.transfer(address(this).balance);
        }

        /// Confirm the purchase as buyer.
        /// Transaction has to include `2 * value` ether.
        /// The ether will be locked until confirmReceived
        /// is called.
        function confirmPurchase()
            public
            inState(State.Created)
            condition(msg.value == (2 * value))
            payable
        {
            emit PurchaseConfirmed();
            buyer = msg.sender;
            state = State.Locked;
        }

        /// Confirm that you (the buyer) received the item.
        /// This will release the locked ether.
        function confirmReceived()
            public
            onlyBuyer
            inState(State.Locked)
        {
            emit ItemReceived();
            // It is important to change the state first because
            // otherwise, the contracts called using `send` below
            // can call in again here.
            state = State.Inactive;

            // NOTE: This actually allows both the buyer and the seller to
            // block the refund - the withdraw pattern should be used.

            buyer.transfer(value);
            seller.transfer(address(this).balance);
        }
    }

********************
Micropayment Channel
********************

In this section we will learn how to build a simple implementation
of a payment channel. It use cryptographics signatures to make
repeated transfers of Ether between the same parties secure, instantaneous, and
without transaction fees. To do it we need to understand how to
sign and verify signatures, and setup the payment channel.

Creating and verifying signatures
=================================

Imagine Alice wants to send a quantity of Ether to Bob, i.e.
Alice is the sender and the Bob is the recipient.
Alice only needs to send cryptographically signed messages off-chain
(e.g. via email) to Bob and it will be very similar to writing checks.

Signatures are used to authorize transactions,
and they are a general tool that is available to
smart contracts. Alice will build a simple
smart contract that lets her transmit Ether, but
in a unusual way, instead of calling a function herself
to initiate a payment, she will let Bob
do that, and therefore pay the transaction fee.
The contract will work as follows:

    1. Alice deploys the ``ReceiverPays`` contract, attaching enough Ether to cover the payments that will be made.
    2. Alice authorizes a payment by signing a message with their private key.
    3. Alice sends the cryptographically signed message to Bob. The message does not need to be kept secret
       (you will understand it later), and the mechanism for sending it does not matter.
    4. Bob claims their payment by presenting the signed message to the smart contract, it verifies the
       authenticity of the message and then releases the funds.

Creating the signature
----------------------

Alice does not need to interact with Ethereum network to
sign the transaction, the process is completely offline.
In this tutorial, we will sign messages in the browser
using ``web3.js`` and ``MetaMask``.
In particular, we will use the standard way described in `EIP-762 <https://github.com/ethereum/EIPs/pull/712>`_,
as it provides a number of other security benefits.

::

    /// Hashing first makes a few things easier
    var hash = web3.sha3("message to sign");
    web3.personal.sign(hash, web3.eth.defaultAccount, function () {...});


Note that the ``web3.personal.sign`` prepends the length of the message to the signed data.
Since we hash first, the message will always be exactly 32 bytes long,
and thus this length prefix is always the same, making everything easier.

What to Sign
------------

For a contract that fulfills payments, the signed message must include:

    1. The recipient's address
    2. The amount to be transferred
    3. Protection against replay attacks

A replay attack is when a signed message is reused to claim authorization for
a second action.
To avoid replay attacks we will use the same as in Ethereum transactions
themselves, a so-called nonce, which is the number of transactions sent by an
account.
The smart contract will check if a nonce is used multiple times.

There is another type of replay attacks, it occurs when the
owner deploys a ``ReceiverPays`` smart contract, performs some payments,
and then destroy the contract. Later, she decides to deploy the
``RecipientPays`` smart contract again, but the new contract does not
know the nonces used in the previous deployment, so the attacker
can use the old messages again.

Alice can protect against it including
the contract's address in the message, and only
messages containing contract's address itself will be accepted.
This functionality can be found in the first two lines of the ``claimPayment()`` function in the full contract
at the end of this chapter.

Packing arguments
-----------------

Now that we have identified what information to include in the
signed message, we are ready to put the message together, hash it,
and sign it. For simplicity, we just concatenate the data.
The
`ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_ library provides
a function called ``soliditySHA3`` that mimics the behavior
of Solidity's ``keccak256`` function applied to arguments encoded
using ``abi.encodePacked``.
Putting it all together, here is a JavaScript function that
creates the proper signature for the ``ReceiverPays`` example:

::

    // recipient is the address that should be paid.
    // amount, in wei, specifies how much ether should be sent.
    // nonce can be any unique number to prevent replay attacks
    // contractAddress is used to prevent cross-contract replay attacks
    function signPayment(recipient, amount, nonce, contractAddress, callback) {
        var hash = "0x" + ethereumjs.ABI.soliditySHA3(
            ["address", "uint256", "uint256", "address"],
            [recipient, amount, nonce, contractAddress]
        ).toString("hex");

        web3.personal.sign(hash, web3.eth.defaultAccount, callback);
    }

Recovering the Message Signer in Solidity
-----------------------------------------

In general, ECDSA signatures consist of two parameters, ``r`` and ``s``.
Signatures in Ethereum include a third parameter called ``v``, that can be used
to recover which account's private key was used to sign in the message,
the transaction's sender. Solidity provides a built-in function
`ecrecover <mathematical-and-cryptographic-functions>`_
that accepts a message along with the ``r``, ``s`` and ``v`` parameters and
returns the address that was used to sign the message.

Extracting the Signature Parameters
-----------------------------------

Signatures produced by web3.js are the concatenation of ``r``, ``s`` and ``v``,
so the first step is splitting those parameters back out. It can be done on the client,
but doing it inside the smart contract means only one signature parameter
needs to be sent rather than three.
Splitting apart a byte array into component parts is a little messy.
We will use `inline assembly <assembly>`_ to do the job
in the ``splitSignature`` function (the third function in the full contract
at the end of this chapter).

Computing the Message Hash
--------------------------

The smart contract needs to know exactly what parameters were signed,
and so it must recreate the message from the parameters and use that
for signature verification. The functions ``prefixed`` and
``recoverSigner`` do this and their use can be found in the
``claimPayment`` function.


The full contract
-----------------

::

    pragma solidity >=0.4.24 <0.6.0;

    contract ReceiverPays {
        address owner = msg.sender;

        mapping(uint256 => bool) usedNonces;

        constructor() public payable {}

        function claimPayment(uint256 amount, uint256 nonce, bytes memory signature) public {
            require(!usedNonces[nonce]);
            usedNonces[nonce] = true;

            // this recreates the message that was signed on the client
            bytes32 message = prefixed(keccak256(abi.encodePacked(msg.sender, amount, nonce, this)));

            require(recoverSigner(message, signature) == owner);

            msg.sender.transfer(amount);
        }

        /// destroy the contract and reclaim the leftover funds.
        function kill() public {
            require(msg.sender == owner);
            selfdestruct(msg.sender);
        }

        /// signature methods.
        function splitSignature(bytes memory sig)
            internal
            pure
            returns (uint8 v, bytes32 r, bytes32 s)
        {
            require(sig.length == 65);

            assembly {
                // first 32 bytes, after the length prefix.
                r := mload(add(sig, 32))
                // second 32 bytes.
                s := mload(add(sig, 64))
                // final byte (first byte of the next 32 bytes).
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

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


Writing a Simple Payment Channel
================================

Alice will now build a simple but complete implementation of a payment channel.
Payment channels use cryptographic signatures to make repeated transfers
of Ether securely, instantaneously, and without transaction fees.

What is a Payment Channel?
--------------------------

Payment channels allow participants to make repeated transfers of Ether without
using transactions. This means that the delays and fees associated with transactions
can be avoided. We are going to explore a simple unidirectional payment channel between
two parties (Alice and Bob). Using it involves three steps:

    1. Alice funds a smart contract with Ether. This "opens" the payment channel.
    2. Alice signs messages that specify how much of that Ether is owed to the recipient. This step is repeated for each payment.
    3. Bob "closes" the payment channel, withdrawing their portion of the Ether and sending the remainder back to the sender.

Not ethat only steps 1 and 3 require Ethereum transactions, step 2 means that
the sender transmits a cryptographically signed message to the recipient via off chain ways (e.g. email).
This means only two transactions are required to support any number of transfers.

Bob is guaranteed to receive their funds because the smart contract escrows
the Ether and honors a valid signed message. The smart contract also enforces a timeout,
so Alice is guaranteed to eventually recover their funds even if the recipient refuses
to close the channel.
It is up to the participants in a payment channel to decide how long to keep it open.
For a short-lived transaction, such as paying an internet cafe for each minute of network access,
or for a longer relationship, such as paying an employee an hourly wage, a payment could last for months or years.

Opening the Payment Channel
---------------------------

To open the payment channel, Alice deploys the smart contract,
attaching the Ether to be escrowed and specifying the intendend recipient
and a maximum duration for the channel to exist. It is the function
``SimplePaymentChannel`` in the contract, that is at the end of this chapter.

Making Payments
---------------

Alice makes payments by sending signed messages to Bob.
This step is performed entirely outside of the Ethereum network.
Messages are cryptographically signed by the sender and then transmitted directly to the recipient.

Each message includes the following information:

    * The smart contract's address, used to prevent cross-contract replay attacks.
    * The total amount of Ether that is owed the recipient so far.

A payment channel is closed just once, at the end of a series of transfers.
Because of this, only one of the messages sent will be redeemed. This is why
each message specifies a cumulative total amount of Ether owed, rather than the
amount of the individual micropayment. The recipient will naturally choose to
redeem the most recent message because that is the one with the highest total.
The nonce per-message is not needed anymore, because the smart contract will
only honor a single message. The address of the smart contract is still used
to prevent a message intended for one payment channel from being used for a different channel.

Here is the modified javascript code to cryptographically sign a message from the previous chapter:

::

    function constructPaymentMessage(contractAddress, amount) {
        return ethereumjs.ABI.soliditySHA3(
            ["address", "uint256"],
            [contractAddress, amount]
        );
    }

    function signMessage(message, callback) {
        web3.personal.sign(
            "0x" + message.toString("hex"),
            web3.eth.defaultAccount,
            callback
        );
    }

    // contractAddress is used to prevent cross-contract replay attacks.
    // amount, in wei, specifies how much Ether should be sent.

    function signPayment(contractAddress, amount, callback) {
        var message = constructPaymentMessage(contractAddress, amount);
        signMessage(message, callback);
    }


Closing the Payment Channel
---------------------------

When Bob is ready to receive their funds, it is time to
close the payment channel by calling a ``close`` function on the smart contract.
Closing the channel pays the recipient the Ether they are owed and destroys the contract,
sending any remaining Ether back to Alice.
To close the channel, Bob needs to provide a message signed by Alice.

The smart contract must verify that the message contains a valid signature from the sender.
The process for doing this verification is the same as the process the recipient uses.
The Solidity functions ``isValidSignature`` and ``recoverSigner`` work just like their
JavaScript counterparts in the previous section. The latter is borrowed from the
``ReceiverPays`` contract in the previous chapter.

The ``close`` function can only be called by the payment channel recipient,
who will naturally pass the most recent payment message because that message
carries the highest total owed. If the sender were allowed to call this function,
they could provide a message with a lower amount and cheat the recipient out of what they are owed.

The function verifies the signed message matches the given parameters.
If everything checks out, the recipient is sent their portion of the Ether,
and the sender is sent the rest via a ``selfdestruct``.
You can see the ``close`` function in the full contract.

Channel Expiration
-------------------

Bob can close the payment channel at any time, but if they fail to do so,
Alice needs a way to recover their escrowed funds. An *expiration* time was set
at the time of contract deployment. Once that time is reached, Alice can call
``claimTimeout`` to recover their funds. You can see the ``claimTimeout`` function in the
full contract.

After this function is called, Bob can no longer receive any Ether,
so it is important that Bob closes the channel before the expiration is reached.


The full contract
-----------------

::

    pragma solidity >=0.4.24 <0.6.0;

    contract SimplePaymentChannel {
        address payable public sender;      // The account sending payments.
        address payable public recipient;   // The account receiving the payments.
        uint256 public expiration;  // Timeout in case the recipient never closes.

        constructor (address payable _recipient, uint256 duration)
            public
            payable
        {
            sender = msg.sender;
            recipient = _recipient;
            expiration = now + duration;
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

        /// the recipient can close the channel at any time by presenting a
        /// signed amount from the sender. the recipient will be sent that amount,
        /// and the remainder will go back to the sender
        function close(uint256 amount, bytes memory signature) public {
            require(msg.sender == recipient);
            require(isValidSignature(amount, signature));

            recipient.transfer(amount);
            selfdestruct(sender);
        }

        /// the sender can extend the expiration at any time
        function extend(uint256 newExpiration) public {
            require(msg.sender == sender);
            require(newExpiration > expiration);

            expiration = newExpiration;
        }

        /// if the timeout is reached without the recipient closing the channel,
        /// then the Ether is released back to the sender.
        function claimTimeout() public {
            require(now >= expiration);
            selfdestruct(sender);
        }

        /// All functions below this are just taken from the chapter
        /// 'creating and verifying signatures' chapter.

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

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256(abi.encodePacked("\x19Ethereum Signed Message:\n32", hash));
        }
    }


Note: The function ``splitSignature`` is very simple and does not use all security checks.
A real implementation should use a more rigorously tested library, such as
openzepplin's `version <https://github.com/OpenZeppelin/openzeppelin-solidity/blob/master/contracts/ECRecovery.sol>`_ of this code.



Verifying Payments
------------------

Unlike in our previous chapter, messages in a payment channel aren't
redeemed right away. The recipient keeps track of the latest message and
redeems it when it's time to close the payment channel. This means it's
critical that the recipient perform their own verification of each message.
Otherwise there is no guarantee that the recipient will be able to get paid
in the end.

The recipient should verify each message using the following process:

    1. Verify that the contact address in the message matches the payment channel.
    2. Verify that the new total is the expected amount.
    3. Verify that the new total does not exceed the amount of Ether escrowed.
    4. Verify that the signature is valid and comes from the payment channel sender.

We'll use the `ethereumjs-util <https://github.com/ethereumjs/ethereumjs-util>`_
library to write this verifications. The final step can be done a number of ways,
but if it's being done in **JavaScript**.
The following code borrows the `constructMessage` function from the signing **JavaScript code**
above:

::

    // this mimics the prefixing behavior of the eth_sign JSON-RPC method.
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
