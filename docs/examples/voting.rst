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

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
<<<<<<< HEAD
    pragma solidity >=0.4.22 <0.7.0;

    /// @title Vote par délegation.
=======
    pragma solidity >=0.7.0 <0.9.0;
    /// @title Voting with delegation.
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
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

<<<<<<< HEAD
        /// Créé un nouveau bulletin pour choisir l'un des `proposalNames`.
        constructor(bytes32[] memory proposalNames) public {
=======
        /// Create a new ballot to choose one of `proposalNames`.
        constructor(bytes32[] memory proposalNames) {
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
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

<<<<<<< HEAD
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
=======
        // Give `voter` the right to vote on this ballot.
        // May only be called by `chairperson`.
        function giveRightToVote(address voter) external {
            // If the first argument of `require` evaluates
            // to `false`, execution terminates and all
            // changes to the state and to Ether balances
            // are reverted.
            // This used to consume all gas in old EVM versions, but
            // not anymore.
            // It is often a good idea to use `require` to check if
            // functions are called correctly.
            // As a second argument, you can also provide an
            // explanation about what went wrong.
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
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

<<<<<<< HEAD
        /// Delegue son vote au votant `to`.
        function delegate(address to) public {
            // assigne les références
=======
        /// Delegate your vote to the voter `to`.
        function delegate(address to) external {
            // assigns reference
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
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

<<<<<<< HEAD
            // Comme `sender` est une référence, ceci
            // modifie `voters[msg.sender].voted`
=======
            // Since `sender` is a reference, this
            // modifies `voters[msg.sender].voted`
            Voter storage delegate_ = voters[to];

            // Voters cannot delegate to wallets that cannot vote.
            require(delegate_.weight >= 1);
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
            sender.voted = true;
            sender.delegate = to;
            if (delegate_.voted) {
                // Si le délégué a déjà voté,
                // on ajoute directement le vote aux autres
                proposals[delegate_.vote].voteCount += sender.weight;
            } else {
                // Sinon, on l'ajoute au poids de son vote.
                delegate_.weight += sender.weight;
            }
        }

<<<<<<< HEAD
        /// Voter (incluant les procurations par délégation)
        /// pour la proposition `proposals[proposal].name`.
        function vote(uint proposal) public {
=======
        /// Give your vote (including votes delegated to you)
        /// to proposal `proposals[proposal].name`.
        function vote(uint proposal) external {
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
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

<<<<<<< HEAD
        // Appelle la fonction winningProposal() pour avoir
        // l'index du gagnant dans le tableau de propositions
        // et retourne le nom de la proposition gagnante.
        function winnerName() public view
=======
        // Calls winningProposal() function to get the index
        // of the winner contained in the proposals array and then
        // returns the name of the winner
        function winnerName() external view
>>>>>>> 47d77931747aba8e364452537d989b795df7ca04
                returns (bytes32 winnerName_)
        {
            winnerName_ = proposals[winningProposal()].name;
        }
    }


Améliorations possibles
=======================

À l'heure actuelle, de nombreuses opérations sont nécessaires pour attribuer les droits de vote à tous les participants. Pouvez-vous trouver un meilleur moyen ?
