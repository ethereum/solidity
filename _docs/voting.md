---
layout: docs
title: Voting
permalink: /docs/voting/
---

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

At the end of the voting time, `winningProposal()`
will return the proposal with the largest number
of votes.

{% include open_link gist="618560d3f740204d46a5" %}
{% highlight JavaScript %}
/// @title Voting with delegation.
contract Ballot
{
    // This declares a new complex type which will
    // be used for variables later.
    // It will represent a single voter.
    struct Voter
    {
        uint weight; // weight is accumulated by delegation
        bool voted;  // if true, that person already voted
        address delegate; // person delegated to
        uint vote;   // index of the voted proposal
    }
    // This is a type for a single proposal.
    struct Proposal
    {
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
    function Ballot(bytes32[] proposalNames)
    {
        chairperson = msg.sender;
        voters[chairperson].weight = 1;
        // For each of the provided proposal names,
        // create a new proposal object and add it
        // to the end of the array.
        for (uint i = 0; i < proposalNames.length; i++)
            // `Proposal({...})` creates a temporary
            // Proposal object and `proposal.push(...)`
            // appends it to the end of `proposals`.
            proposals.push(Proposal({
                name: proposalNames[i],
                voteCount: 0
            }));
    }

    // Give `voter` the right to vote on this ballot.
    // May only be called by `chairperson`.
    function giveRightToVote(address voter)
    {
        if (msg.sender != chairperson || voters[voter].voted)
            // `throw` terminates and reverts all changes to
            // the state and to Ether balances. It is often
            // a good idea to use this if functions are
            // called incorrectly. But watch out, this
            // will also consume all provided gas.
            throw;
        voters[voter].weight = 1;
    }

    /// Delegate your vote to the voter `to`.
    function delegate(address to)
    {
        // assigns reference
        Voter sender = voters[msg.sender];
        if (sender.voted)
            throw;
        // Forward the delegation as long as
        // `to` also delegated.
        while (voters[to].delegate != address(0) &&
               voters[to].delegate != msg.sender)
            to = voters[to].delegate;
        // We found a loop in the delegation, not allowed.
        if (to == msg.sender)
            throw;
        // Since `sender` is a reference, this
        // modifies `voters[msg.sender].voted`
        sender.voted = true;
        sender.delegate = to;
        Voter delegate = voters[to];
        if (delegate.voted)
            // If the delegate already voted,
            // directly add to the number of votes 
            proposals[delegate.vote].voteCount += sender.weight;
        else
            // If the delegate did not vote yet,
            // add to her weight.
            delegate.weight += sender.weight;
    }

    /// Give your vote (including votes delegated to you)
    /// to proposal `proposals[proposal].name`.
    function vote(uint proposal)
    {
        Voter sender = voters[msg.sender];
        if (sender.voted) throw;
        sender.voted = true;
        sender.vote = proposal;
        // If `proposal` is out of the range of the array,
        // this will throw automatically and revert all
        // changes.
        proposals[proposal].voteCount += sender.weight;
    }

    /// @dev Computes the winning proposal taking all
    /// previous votes into account.
    function winningProposal() constant
            returns (uint winningProposal)
    {
        uint winningVoteCount = 0;
        for (uint p = 0; p < proposals.length; p++)
        {
            if (proposals[p].voteCount > winningVoteCount)
            {
                winningVoteCount = proposals[p].voteCount;
                winningProposal = p;
            }
        }
    }
}
{% endhighlight %}


## Possible Improvements

Currently, many transactions are needed to assign the rights
to vote to all participants. Can you think of a better way?
