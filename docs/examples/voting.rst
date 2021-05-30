.. index:: voting, ballot

.. _voting:

******
Voting
******

The following contract is quite complex, but showcases
a lot of Solidity's features. It implements a voting
contract. Of course, the main problems of electronic
voting is how to assign voting rights to the correct
persons and how to prevent manipulation. We will not
solve all problems here, but at least we will show
how delegated voting can be done so that vote counting
is **automatic and completely transparent** at the
same time.

The idea is to create one contract per ballot,
providing a short name for each option.
Then the creator of the contract who serves as
chairperson will give the right to vote to each
address individually.

The persons behind the addresses can then choose
to either vote themselves or to delegate their
vote to a person they trust.

At the end of the voting time, ``winningProposal()``
will return the proposal with the largest number
of votes.

Open in `Remix <http://remix.ethereum.org/#code=Ly8gU1BEWC1MaWNlbnNlLUlkZW50aWZpZXI6IEdQTC0zLjAKcHJhZ21hIHNvbGlkaXR5ID49MC43LjAgPDAuOS4wOwovLy8gQHRpdGxlIFZvdGluZyB3aXRoIGRlbGVnYXRpb24uCmNvbnRyYWN0IEJhbGxvdCB7CiAgICAvLyBUaGlzIGRlY2xhcmVzIGEgbmV3IGNvbXBsZXggdHlwZSB3aGljaCB3aWxsCiAgICAvLyBiZSB1c2VkIGZvciB2YXJpYWJsZXMgbGF0ZXIuCiAgICAvLyBJdCB3aWxsIHJlcHJlc2VudCBhIHNpbmdsZSB2b3Rlci4KICAgIHN0cnVjdCBWb3RlciB7CiAgICAgICAgdWludCB3ZWlnaHQ7IC8vIHdlaWdodCBpcyBhY2N1bXVsYXRlZCBieSBkZWxlZ2F0aW9uCiAgICAgICAgYm9vbCB2b3RlZDsgIC8vIGlmIHRydWUsIHRoYXQgcGVyc29uIGFscmVhZHkgdm90ZWQKICAgICAgICBhZGRyZXNzIGRlbGVnYXRlOyAvLyBwZXJzb24gZGVsZWdhdGVkIHRvCiAgICAgICAgdWludCB2b3RlOyAgIC8vIGluZGV4IG9mIHRoZSB2b3RlZCBwcm9wb3NhbAogICAgfQoKICAgIC8vIFRoaXMgaXMgYSB0eXBlIGZvciBhIHNpbmdsZSBwcm9wb3NhbC4KICAgIHN0cnVjdCBQcm9wb3NhbCB7CiAgICAgICAgYnl0ZXMzMiBuYW1lOyAgIC8vIHNob3J0IG5hbWUgKHVwIHRvIDMyIGJ5dGVzKQogICAgICAgIHVpbnQgdm90ZUNvdW50OyAvLyBudW1iZXIgb2YgYWNjdW11bGF0ZWQgdm90ZXMKICAgIH0KCiAgICBhZGRyZXNzIHB1YmxpYyBjaGFpcnBlcnNvbjsKCiAgICAvLyBUaGlzIGRlY2xhcmVzIGEgc3RhdGUgdmFyaWFibGUgdGhhdAogICAgLy8gc3RvcmVzIGEgYFZvdGVyYCBzdHJ1Y3QgZm9yIGVhY2ggcG9zc2libGUgYWRkcmVzcy4KICAgIG1hcHBpbmcoYWRkcmVzcyA9PiBWb3RlcikgcHVibGljIHZvdGVyczsKCiAgICAvLyBBIGR5bmFtaWNhbGx5LXNpemVkIGFycmF5IG9mIGBQcm9wb3NhbGAgc3RydWN0cy4KICAgIFByb3Bvc2FsW10gcHVibGljIHByb3Bvc2FsczsKCiAgICAvLy8gQ3JlYXRlIGEgbmV3IGJhbGxvdCB0byBjaG9vc2Ugb25lIG9mIGBwcm9wb3NhbE5hbWVzYC4KICAgIGNvbnN0cnVjdG9yKGJ5dGVzMzJbXSBtZW1vcnkgcHJvcG9zYWxOYW1lcykgewogICAgICAgIGNoYWlycGVyc29uID0gbXNnLnNlbmRlcjsKICAgICAgICB2b3RlcnNbY2hhaXJwZXJzb25dLndlaWdodCA9IDE7CgogICAgICAgIC8vIEZvciBlYWNoIG9mIHRoZSBwcm92aWRlZCBwcm9wb3NhbCBuYW1lcywKICAgICAgICAvLyBjcmVhdGUgYSBuZXcgcHJvcG9zYWwgb2JqZWN0IGFuZCBhZGQgaXQKICAgICAgICAvLyB0byB0aGUgZW5kIG9mIHRoZSBhcnJheS4KICAgICAgICBmb3IgKHVpbnQgaSA9IDA7IGkgPCBwcm9wb3NhbE5hbWVzLmxlbmd0aDsgaSsrKSB7CiAgICAgICAgICAgIC8vIGBQcm9wb3NhbCh7Li4ufSlgIGNyZWF0ZXMgYSB0ZW1wb3JhcnkKICAgICAgICAgICAgLy8gUHJvcG9zYWwgb2JqZWN0IGFuZCBgcHJvcG9zYWxzLnB1c2goLi4uKWAKICAgICAgICAgICAgLy8gYXBwZW5kcyBpdCB0byB0aGUgZW5kIG9mIGBwcm9wb3NhbHNgLgogICAgICAgICAgICBwcm9wb3NhbHMucHVzaChQcm9wb3NhbCh7CiAgICAgICAgICAgICAgICBuYW1lOiBwcm9wb3NhbE5hbWVzW2ldLAogICAgICAgICAgICAgICAgdm90ZUNvdW50OiAwCiAgICAgICAgICAgIH0pKTsKICAgICAgICB9CiAgICB9CgogICAgLy8gR2l2ZSBgdm90ZXJgIHRoZSByaWdodCB0byB2b3RlIG9uIHRoaXMgYmFsbG90LgogICAgLy8gTWF5IG9ubHkgYmUgY2FsbGVkIGJ5IGBjaGFpcnBlcnNvbmAuCiAgICBmdW5jdGlvbiBnaXZlUmlnaHRUb1ZvdGUoYWRkcmVzcyB2b3RlcikgcHVibGljIHsKICAgICAgICAvLyBJZiB0aGUgZmlyc3QgYXJndW1lbnQgb2YgYHJlcXVpcmVgIGV2YWx1YXRlcwogICAgICAgIC8vIHRvIGBmYWxzZWAsIGV4ZWN1dGlvbiB0ZXJtaW5hdGVzIGFuZCBhbGwKICAgICAgICAvLyBjaGFuZ2VzIHRvIHRoZSBzdGF0ZSBhbmQgdG8gRXRoZXIgYmFsYW5jZXMKICAgICAgICAvLyBhcmUgcmV2ZXJ0ZWQuCiAgICAgICAgLy8gVGhpcyB1c2VkIHRvIGNvbnN1bWUgYWxsIGdhcyBpbiBvbGQgRVZNIHZlcnNpb25zLCBidXQKICAgICAgICAvLyBub3QgYW55bW9yZS4KICAgICAgICAvLyBJdCBpcyBvZnRlbiBhIGdvb2QgaWRlYSB0byB1c2UgYHJlcXVpcmVgIHRvIGNoZWNrIGlmCiAgICAgICAgLy8gZnVuY3Rpb25zIGFyZSBjYWxsZWQgY29ycmVjdGx5LgogICAgICAgIC8vIEFzIGEgc2Vjb25kIGFyZ3VtZW50LCB5b3UgY2FuIGFsc28gcHJvdmlkZSBhbgogICAgICAgIC8vIGV4cGxhbmF0aW9uIGFib3V0IHdoYXQgd2VudCB3cm9uZy4KICAgICAgICByZXF1aXJlKAogICAgICAgICAgICBtc2cuc2VuZGVyID09IGNoYWlycGVyc29uLAogICAgICAgICAgICAiT25seSBjaGFpcnBlcnNvbiBjYW4gZ2l2ZSByaWdodCB0byB2b3RlLiIKICAgICAgICApOwogICAgICAgIHJlcXVpcmUoCiAgICAgICAgICAgICF2b3RlcnNbdm90ZXJdLnZvdGVkLAogICAgICAgICAgICAiVGhlIHZvdGVyIGFscmVhZHkgdm90ZWQuIgogICAgICAgICk7CiAgICAgICAgcmVxdWlyZSh2b3RlcnNbdm90ZXJdLndlaWdodCA9PSAwKTsKICAgICAgICB2b3RlcnNbdm90ZXJdLndlaWdodCA9IDE7CiAgICB9CgogICAgLy8vIERlbGVnYXRlIHlvdXIgdm90ZSB0byB0aGUgdm90ZXIgYHRvYC4KICAgIGZ1bmN0aW9uIGRlbGVnYXRlKGFkZHJlc3MgdG8pIHB1YmxpYyB7CiAgICAgICAgLy8gYXNzaWducyByZWZlcmVuY2UKICAgICAgICBWb3RlciBzdG9yYWdlIHNlbmRlciA9IHZvdGVyc1ttc2cuc2VuZGVyXTsKICAgICAgICByZXF1aXJlKCFzZW5kZXIudm90ZWQsICJZb3UgYWxyZWFkeSB2b3RlZC4iKTsKCiAgICAgICAgcmVxdWlyZSh0byAhPSBtc2cuc2VuZGVyLCAiU2VsZi1kZWxlZ2F0aW9uIGlzIGRpc2FsbG93ZWQuIik7CgogICAgICAgIC8vIEZvcndhcmQgdGhlIGRlbGVnYXRpb24gYXMgbG9uZyBhcwogICAgICAgIC8vIGB0b2AgYWxzbyBkZWxlZ2F0ZWQuCiAgICAgICAgLy8gSW4gZ2VuZXJhbCwgc3VjaCBsb29wcyBhcmUgdmVyeSBkYW5nZXJvdXMsCiAgICAgICAgLy8gYmVjYXVzZSBpZiB0aGV5IHJ1biB0b28gbG9uZywgdGhleSBtaWdodAogICAgICAgIC8vIG5lZWQgbW9yZSBnYXMgdGhhbiBpcyBhdmFpbGFibGUgaW4gYSBibG9jay4KICAgICAgICAvLyBJbiB0aGlzIGNhc2UsIHRoZSBkZWxlZ2F0aW9uIHdpbGwgbm90IGJlIGV4ZWN1dGVkLAogICAgICAgIC8vIGJ1dCBpbiBvdGhlciBzaXR1YXRpb25zLCBzdWNoIGxvb3BzIG1pZ2h0CiAgICAgICAgLy8gY2F1c2UgYSBjb250cmFjdCB0byBnZXQgInN0dWNrIiBjb21wbGV0ZWx5LgogICAgICAgIHdoaWxlICh2b3RlcnNbdG9dLmRlbGVnYXRlICE9IGFkZHJlc3MoMCkpIHsKICAgICAgICAgICAgdG8gPSB2b3RlcnNbdG9dLmRlbGVnYXRlOwoKICAgICAgICAgICAgLy8gV2UgZm91bmQgYSBsb29wIGluIHRoZSBkZWxlZ2F0aW9uLCBub3QgYWxsb3dlZC4KICAgICAgICAgICAgcmVxdWlyZSh0byAhPSBtc2cuc2VuZGVyLCAiRm91bmQgbG9vcCBpbiBkZWxlZ2F0aW9uLiIpOwogICAgICAgIH0KCiAgICAgICAgLy8gU2luY2UgYHNlbmRlcmAgaXMgYSByZWZlcmVuY2UsIHRoaXMKICAgICAgICAvLyBtb2RpZmllcyBgdm90ZXJzW21zZy5zZW5kZXJdLnZvdGVkYAogICAgICAgIHNlbmRlci52b3RlZCA9IHRydWU7CiAgICAgICAgc2VuZGVyLmRlbGVnYXRlID0gdG87CiAgICAgICAgVm90ZXIgc3RvcmFnZSBkZWxlZ2F0ZV8gPSB2b3RlcnNbdG9dOwogICAgICAgIGlmIChkZWxlZ2F0ZV8udm90ZWQpIHsKICAgICAgICAgICAgLy8gSWYgdGhlIGRlbGVnYXRlIGFscmVhZHkgdm90ZWQsCiAgICAgICAgICAgIC8vIGRpcmVjdGx5IGFkZCB0byB0aGUgbnVtYmVyIG9mIHZvdGVzCiAgICAgICAgICAgIHByb3Bvc2Fsc1tkZWxlZ2F0ZV8udm90ZV0udm90ZUNvdW50ICs9IHNlbmRlci53ZWlnaHQ7CiAgICAgICAgfSBlbHNlIHsKICAgICAgICAgICAgLy8gSWYgdGhlIGRlbGVnYXRlIGRpZCBub3Qgdm90ZSB5ZXQsCiAgICAgICAgICAgIC8vIGFkZCB0byBoZXIgd2VpZ2h0LgogICAgICAgICAgICBkZWxlZ2F0ZV8ud2VpZ2h0ICs9IHNlbmRlci53ZWlnaHQ7CiAgICAgICAgfQogICAgfQoKICAgIC8vLyBHaXZlIHlvdXIgdm90ZSAoaW5jbHVkaW5nIHZvdGVzIGRlbGVnYXRlZCB0byB5b3UpCiAgICAvLy8gdG8gcHJvcG9zYWwgYHByb3Bvc2Fsc1twcm9wb3NhbF0ubmFtZWAuCiAgICBmdW5jdGlvbiB2b3RlKHVpbnQgcHJvcG9zYWwpIHB1YmxpYyB7CiAgICAgICAgVm90ZXIgc3RvcmFnZSBzZW5kZXIgPSB2b3RlcnNbbXNnLnNlbmRlcl07CiAgICAgICAgcmVxdWlyZShzZW5kZXIud2VpZ2h0ICE9IDAsICJIYXMgbm8gcmlnaHQgdG8gdm90ZSIpOwogICAgICAgIHJlcXVpcmUoIXNlbmRlci52b3RlZCwgIkFscmVhZHkgdm90ZWQuIik7CiAgICAgICAgc2VuZGVyLnZvdGVkID0gdHJ1ZTsKICAgICAgICBzZW5kZXIudm90ZSA9IHByb3Bvc2FsOwoKICAgICAgICAvLyBJZiBgcHJvcG9zYWxgIGlzIG91dCBvZiB0aGUgcmFuZ2Ugb2YgdGhlIGFycmF5LAogICAgICAgIC8vIHRoaXMgd2lsbCB0aHJvdyBhdXRvbWF0aWNhbGx5IGFuZCByZXZlcnQgYWxsCiAgICAgICAgLy8gY2hhbmdlcy4KICAgICAgICBwcm9wb3NhbHNbcHJvcG9zYWxdLnZvdGVDb3VudCArPSBzZW5kZXIud2VpZ2h0OwogICAgfQoKICAgIC8vLyBAZGV2IENvbXB1dGVzIHRoZSB3aW5uaW5nIHByb3Bvc2FsIHRha2luZyBhbGwKICAgIC8vLyBwcmV2aW91cyB2b3RlcyBpbnRvIGFjY291bnQuCiAgICBmdW5jdGlvbiB3aW5uaW5nUHJvcG9zYWwoKSBwdWJsaWMgdmlldwogICAgICAgICAgICByZXR1cm5zICh1aW50IHdpbm5pbmdQcm9wb3NhbF8pCiAgICB7CiAgICAgICAgdWludCB3aW5uaW5nVm90ZUNvdW50ID0gMDsKICAgICAgICBmb3IgKHVpbnQgcCA9IDA7IHAgPCBwcm9wb3NhbHMubGVuZ3RoOyBwKyspIHsKICAgICAgICAgICAgaWYgKHByb3Bvc2Fsc1twXS52b3RlQ291bnQgPiB3aW5uaW5nVm90ZUNvdW50KSB7CiAgICAgICAgICAgICAgICB3aW5uaW5nVm90ZUNvdW50ID0gcHJvcG9zYWxzW3BdLnZvdGVDb3VudDsKICAgICAgICAgICAgICAgIHdpbm5pbmdQcm9wb3NhbF8gPSBwOwogICAgICAgICAgICB9CiAgICAgICAgfQogICAgfQoKICAgIC8vIENhbGxzIHdpbm5pbmdQcm9wb3NhbCgpIGZ1bmN0aW9uIHRvIGdldCB0aGUgaW5kZXgKICAgIC8vIG9mIHRoZSB3aW5uZXIgY29udGFpbmVkIGluIHRoZSBwcm9wb3NhbHMgYXJyYXkgYW5kIHRoZW4KICAgIC8vIHJldHVybnMgdGhlIG5hbWUgb2YgdGhlIHdpbm5lcgogICAgZnVuY3Rpb24gd2lubmVyTmFtZSgpIHB1YmxpYyB2aWV3CiAgICAgICAgICAgIHJldHVybnMgKGJ5dGVzMzIgd2lubmVyTmFtZV8pCiAgICB7CiAgICAgICAgd2lubmVyTmFtZV8gPSBwcm9wb3NhbHNbd2lubmluZ1Byb3Bvc2FsKCldLm5hbWU7CiAgICB9Cn0>`_.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    /// @title Voting with delegation.
    contract Ballot {
        // This declares a new complex type which will
        // be used for variables later.
        // It will represent a single voter.
        struct Voter {
            uint weight; // weight is accumulated by delegation
            bool voted;  // if true, that person already voted
            address delegate; // person delegated to
            uint vote;   // index of the voted proposal
        }

        // This is a type for a single proposal.
        struct Proposal {
            bytes32 name;   // short name (up to 32 bytes)
            uint voteCount; // number of accumulated votes
        }

        address public chairperson;

        // This declares a state variable that
        // stores a `Voter` struct for each possible address.
        mapping(address => Voter) public voters;

        // A dynamically-sized array of `Proposal` structs.
        Proposal[] public proposals;

        /// Create a new ballot to choose one of `proposalNames`.
        constructor(bytes32[] memory proposalNames) {
            chairperson = msg.sender;
            voters[chairperson].weight = 1;

            // For each of the provided proposal names,
            // create a new proposal object and add it
            // to the end of the array.
            for (uint i = 0; i < proposalNames.length; i++) {
                // `Proposal({...})` creates a temporary
                // Proposal object and `proposals.push(...)`
                // appends it to the end of `proposals`.
                proposals.push(Proposal({
                    name: proposalNames[i],
                    voteCount: 0
                }));
            }
        }

        // Give `voter` the right to vote on this ballot.
        // May only be called by `chairperson`.
        function giveRightToVote(address voter) public {
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

        /// Delegate your vote to the voter `to`.
        function delegate(address to) public {
            // assigns reference
            Voter storage sender = voters[msg.sender];
            require(!sender.voted, "You already voted.");

            require(to != msg.sender, "Self-delegation is disallowed.");

            // Forward the delegation as long as
            // `to` also delegated.
            // In general, such loops are very dangerous,
            // because if they run too long, they might
            // need more gas than is available in a block.
            // In this case, the delegation will not be executed,
            // but in other situations, such loops might
            // cause a contract to get "stuck" completely.
            while (voters[to].delegate != address(0)) {
                to = voters[to].delegate;

                // We found a loop in the delegation, not allowed.
                require(to != msg.sender, "Found loop in delegation.");
            }

            // Since `sender` is a reference, this
            // modifies `voters[msg.sender].voted`
            sender.voted = true;
            sender.delegate = to;
            Voter storage delegate_ = voters[to];
            if (delegate_.voted) {
                // If the delegate already voted,
                // directly add to the number of votes
                proposals[delegate_.vote].voteCount += sender.weight;
            } else {
                // If the delegate did not vote yet,
                // add to her weight.
                delegate_.weight += sender.weight;
            }
        }

        /// Give your vote (including votes delegated to you)
        /// to proposal `proposals[proposal].name`.
        function vote(uint proposal) public {
            Voter storage sender = voters[msg.sender];
            require(sender.weight != 0, "Has no right to vote");
            require(!sender.voted, "Already voted.");
            sender.voted = true;
            sender.vote = proposal;

            // If `proposal` is out of the range of the array,
            // this will throw automatically and revert all
            // changes.
            proposals[proposal].voteCount += sender.weight;
        }

        /// @dev Computes the winning proposal taking all
        /// previous votes into account.
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

        // Calls winningProposal() function to get the index
        // of the winner contained in the proposals array and then
        // returns the name of the winner
        function winnerName() public view
                returns (bytes32 winnerName_)
        {
            winnerName_ = proposals[winningProposal()].name;
        }
    }


Possible Improvements
=====================

Currently, many transactions are needed to assign the rights
to vote to all participants. Can you think of a better way?
