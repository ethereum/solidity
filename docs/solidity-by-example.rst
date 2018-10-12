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

**********************
Achat distant sécurisé
**********************

::

    pragma solidity >=0.4.22 <0.6.0;

    contract Purchase {
        uint public value;
        address payable public seller;
        address payable public buyer;
        enum State { Created, Locked, Inactive }
        State public state;

        // Vérifie que `msg.value` est un nombre pair.
        // La division tronquerait un nombre impair.
        // On multiplie pour vérifier que ce n'était pas un impair.
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

        /// Annule l'achat et rembourse l'ether du dépot.
        /// Peut seulement être appelé par le vendeur
        /// avant le verrouillage du contrat
        function abort()
            public
            onlySeller
            inState(State.Created)
        {
            emit Aborted();
            state = State.Inactive;
            seller.transfer(address(this).balance);
        }

        /// Confirme l'achat en tant qu'acheteur.
        /// La transaction doit inclure `2 * value` ether.
        /// L'Ether sera bloqué jusqu'à ce que confirmReceived
        /// soit appelé.
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

        /// Confirmer que vous (l'acheteur) avez reçu l'objet,
        /// ce qui débloquera l'Ether bloqué.
        function confirmReceived()
            public
            onlyBuyer
            inState(State.Locked)
        {
            emit ItemReceived();
            // Il est important de changer l'état d'abord car sinon
            // les contrats appelés avec `send` ci-dessous
            // pourraient rappeler la fonction.
            state = State.Inactive;

            // NOTE: Ce schéma autorise les deux acteurs à bloquer
            // la transaction par une exception "our of gas" ( pas
            // assez de gas). Un fonction de retrait distincte devrait
            // être utilisée.

            buyer.transfer(value);
            seller.transfer(address(this).balance);
        }
    }

************************
Canaux de micro-paiement
************************

Dans cette section, nous allons apprendre comment construire une implémentation simple d'un canal de paiement. Il utilise des signatures cryptographiques pour effectuer des transferts répétés d'Ether entre les mêmes parties en toute sécurité, instantanément et sans frais de transaction. Pour ce faire, nous devons comprendre comment signer et vérifier les signatures, et configurer le canal de paiement.

Création et vérification des signatures
=======================================

Imaginez qu'Alice veuille envoyer une quantité d'Ether à Bob, c'est-à-dire qu'Alice est l'expéditeur et Bob est le destinataire. Alice n'a qu'à envoyer des messages cryptographiquement signés hors chaîne (par exemple par e-mail) à Bob et cela sera très similaire à la rédaction de chèques.

Les signatures sont utilisées pour autoriser les transactions et sont un outil généraliste à la disposition des contrats intelligents. Alice construira un simple contrat intelligent qui lui permettra de transmettre des Ether, mais d'une manière inhabituelle, au lieu d'appeler une fonction elle-même pour initier un paiement, elle laissera Bob le faire, et donc payer les frais de transaction. Le contrat fonctionnera comme suit :

    1. Alice déploie le contrat ``ReceiverPays`` en y attachant suffisamment d'éther pour couvrir les paiements qui seront effectués.
    2. Alice autorise un paiement en signant un message avec sa clé privée.
    3. Alice envoie le message signé cryptographiquement à Bob. Le message n'a pas besoin d'être gardé secret
       (vous le comprendrez plus tard), et le mécanisme pour l'envoyer n'a pas d'importance.
    4. Bob réclame leur paiement en présentant le message signé au contrat intelligent, il vérifie l'authenticité du message et libère ensuite les fonds.

Création de la signature
------------------------

Alice n'a pas besoin d'interagir avec le réseau Ethereum pour
signer la transaction, le processus est complètement hors ligne.
Dans ce tutoriel, nous allons signer les messages dans le navigateur
en utilisant ``web3.js`` et ``MetaMask``.
En particulier, nous utiliserons la méthode standard décrite dans `EIP-762 <https://github.com/ethereum/EIPs/pull/712>`_,
car elle offre un certain nombre d'autres avantages en matière de sécurité.

::

    /// Hasher d'abord simplifie un peu les choses
    var hash = web3.sha3("message to sign");
    web3.personal.sign(hash, web3.eth.defaultAccount, function () {...});


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

::

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

Récupérer le signataire du message en Solidity
----------------------------------------------

En général, les signatures ECDSA se composent de deux paramètres, ``r`` et ``s``.
Les signatures dans Ethereum incluent un troisième paramètre appelé ``v``, qui peut être utilisé
pour récupérer la clé privée du compte qui a été utilisée pour signer le message,
l'expéditeur de la transaction. Solidity fournit une fonction intégrée
`ecrecover <mathematical-and-cryptographic-functions>`_qui accepte un message avec les paramètres ``r```, ``s`` et ``v`` et renvoie l'adresse qui a été utilisée pour signer le message.

Récupérer le signataire du message dans la solidité
---------------------------------------------------

En général, les signatures ECDSA se composent de deux paramètres, ``r`` et ``s``.
Les signatures dans Ethereum incluent un troisième paramètre appelé "v", qui peut être utilisé
pour récupérer la clé privée du compte qui a été utilisée pour signer le message,
l'expéditeur de la transaction. La solidité offre une fonction intégrée
Récupérer <fonctions-mathématiques et cryptographiques>`_
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

Calcul du hachage des messages
------------------------------

Le smart contract doit savoir exactement quels paramètres ont été signés,
et doit donc recréer le message à partir des paramètres et utiliser cette fonction
pour la vérification des signatures. Les fonctions ``prefixed`` et
``recoverSigner`` s'occupent de cela et leur utilisation peut se trouver
dans la fonction ``claimPayment``.


Le contrat complet
------------------

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

        /// détruit le contrat et réclame son solde.
        function kill() public {
            require(msg.sender == owner);
            selfdestruct(msg.sender);
        }

        /// methodes de signature.
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

        /// construit un hash préfixé pour mimer le comportement de eth_sign.
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
