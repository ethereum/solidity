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

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

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

        // Ce qui suit est appelé commentaire natspec,
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

Un autre défi est de savoir comment rendre l'enchère **contraignante et aveugle** en même temps : La seule façon d'éviter que l'enchérisseur n'envoie pas l'argent après avoir gagné l'enchère est de le lui faire envoyer avec l'enchère. Puisque les transferts de valeur ne peuvent pas être aveuglés dans Ethereum, tout le monde peut voir la valeur.

Le contrat suivant résout ce problème en acceptant toute valeur supérieure à l'offre la plus élevée. Comme cela ne peut bien sûr être vérifié que pendant la phase de révélation, certaines offres peuvent être **invalides**, et c'est fait exprès (il fournit même un marqueur explicite pour placer des offres invalides avec des transferts de grande valeur) : Les soumissionnaires peuvent brouiller la concurrence en plaçant plusieurs offres invalides hautes ou basses.


::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.7.0;

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