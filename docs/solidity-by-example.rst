###################
Solidity by Example
###################

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

::

    pragma solidity ^0.4.22;

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
        constructor(bytes32[] proposalNames) public {
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

.. index:: auction;blind, auction;open, blind auction, open auction

*************
Blind Auction
*************

In this section, we will show how easy it is to create a
completely blind auction contract on Ethereum.
We will start with an open auction where everyone
can see the bids that are made and then extend this
contract into a blind auction where it is not
possible to see the actual bid until the bidding
period ends.

.. _simple_auction:

Simple Open Auction
===================

The general idea of the following simple auction contract
is that everyone can send their bids during
a bidding period. The bids already include sending
money / ether in order to bind the bidders to their
bid. If the highest bid is raised, the previously
highest bidder gets her money back.
After the end of the bidding period, the
contract has to be called manually for the
beneficiary to receive his money - contracts cannot
activate themselves.

::

    pragma solidity ^0.4.22;

    contract SimpleAuction {
        // Parameters of the auction. Times are either
        // absolute unix timestamps (seconds since 1970-01-01)
        // or time periods in seconds.
        address public beneficiary;
        uint public auctionEnd;

        // Current state of the auction.
        address public highestBidder;
        uint public highestBid;

        // Allowed withdrawals of previous bids
        mapping(address => uint) pendingReturns;

        // Set to true at the end, disallows any change
        bool ended;

        // Events that will be fired on changes.
        event HighestBidIncreased(address bidder, uint amount);
        event AuctionEnded(address winner, uint amount);

        // The following is a so-called natspec comment,
        // recognizable by the three slashes.
        // It will be shown when the user is asked to
        // confirm a transaction.

        /// Create a simple auction with `_biddingTime`
        /// seconds bidding time on behalf of the
        /// beneficiary address `_beneficiary`.
        constructor(
            uint _biddingTime,
            address _beneficiary
        ) public {
            beneficiary = _beneficiary;
            auctionEnd = now + _biddingTime;
        }

        /// Bid on the auction with the value sent
        /// together with this transaction.
        /// The value will only be refunded if the
        /// auction is not won.
        function bid() public payable {
            // No arguments are necessary, all
            // information is already part of
            // the transaction. The keyword payable
            // is required for the function to
            // be able to receive Ether.

            // Revert the call if the bidding
            // period is over.
            require(
                now <= auctionEnd,
                "Auction already ended."
            );

            // If the bid is not higher, send the
            // money back.
            require(
                msg.value > highestBid,
                "There already is a higher bid."
            );

            if (highestBid != 0) {
                // Sending back the money by simply using
                // highestBidder.send(highestBid) is a security risk
                // because it could execute an untrusted contract.
                // It is always safer to let the recipients
                // withdraw their money themselves.
                pendingReturns[highestBidder] += highestBid;
            }
            highestBidder = msg.sender;
            highestBid = msg.value;
            emit HighestBidIncreased(msg.sender, msg.value);
        }

        /// Withdraw a bid that was overbid.
        function withdraw() public returns (bool) {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // It is important to set this to zero because the recipient
                // can call this function again as part of the receiving call
                // before `send` returns.
                pendingReturns[msg.sender] = 0;

                if (!msg.sender.send(amount)) {
                    // No need to call throw here, just reset the amount owing
                    pendingReturns[msg.sender] = amount;
                    return false;
                }
            }
            return true;
        }

        /// End the auction and send the highest bid
        /// to the beneficiary.
        function auctionEnd() public {
            // It is a good guideline to structure functions that interact
            // with other contracts (i.e. they call functions or send Ether)
            // into three phases:
            // 1. checking conditions
            // 2. performing actions (potentially changing conditions)
            // 3. interacting with other contracts
            // If these phases are mixed up, the other contract could call
            // back into the current contract and modify the state or cause
            // effects (ether payout) to be performed multiple times.
            // If functions called internally include interaction with external
            // contracts, they also have to be considered interaction with
            // external contracts.

            // 1. Conditions
            require(now >= auctionEnd, "Auction not yet ended.");
            require(!ended, "auctionEnd has already been called.");

            // 2. Effects
            ended = true;
            emit AuctionEnded(highestBidder, highestBid);

            // 3. Interaction
            beneficiary.transfer(highestBid);
        }
    }

Blind Auction
=============

The previous open auction is extended to a blind auction
in the following. The advantage of a blind auction is
that there is no time pressure towards the end of
the bidding period. Creating a blind auction on a
transparent computing platform might sound like a
contradiction, but cryptography comes to the rescue.

During the **bidding period**, a bidder does not
actually send her bid, but only a hashed version of it.
Since it is currently considered practically impossible
to find two (sufficiently long) values whose hash
values are equal, the bidder commits to the bid by that.
After the end of the bidding period, the bidders have
to reveal their bids: They send their values
unencrypted and the contract checks that the hash value
is the same as the one provided during the bidding period.

Another challenge is how to make the auction
**binding and blind** at the same time: The only way to
prevent the bidder from just not sending the money
after he won the auction is to make her send it
together with the bid. Since value transfers cannot
be blinded in Ethereum, anyone can see the value.

The following contract solves this problem by
accepting any value that is larger than the highest
bid. Since this can of course only be checked during
the reveal phase, some bids might be **invalid**, and
this is on purpose (it even provides an explicit
flag to place invalid bids with high value transfers):
Bidders can confuse competition by placing several
high or low invalid bids.


::

    pragma solidity >0.4.23 <0.5.0;

    contract BlindAuction {
        struct Bid {
            bytes32 blindedBid;
            uint deposit;
        }

        address public beneficiary;
        uint public biddingEnd;
        uint public revealEnd;
        bool public ended;

        mapping(address => Bid[]) public bids;

        address public highestBidder;
        uint public highestBid;

        // Allowed withdrawals of previous bids
        mapping(address => uint) pendingReturns;

        event AuctionEnded(address winner, uint highestBid);

        /// Modifiers are a convenient way to validate inputs to
        /// functions. `onlyBefore` is applied to `bid` below:
        /// The new function body is the modifier's body where
        /// `_` is replaced by the old function body.
        modifier onlyBefore(uint _time) { require(now < _time); _; }
        modifier onlyAfter(uint _time) { require(now > _time); _; }

        constructor(
            uint _biddingTime,
            uint _revealTime,
            address _beneficiary
        ) public {
            beneficiary = _beneficiary;
            biddingEnd = now + _biddingTime;
            revealEnd = biddingEnd + _revealTime;
        }

        /// Place a blinded bid with `_blindedBid` =
        /// keccak256(abi.encodePacked(value, fake, secret)).
        /// The sent ether is only refunded if the bid is correctly
        /// revealed in the revealing phase. The bid is valid if the
        /// ether sent together with the bid is at least "value" and
        /// "fake" is not true. Setting "fake" to true and sending
        /// not the exact amount are ways to hide the real bid but
        /// still make the required deposit. The same address can
        /// place multiple bids.
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

        /// Reveal your blinded bids. You will get a refund for all
        /// correctly blinded invalid bids and for all bids except for
        /// the totally highest.
        function reveal(
            uint[] _values,
            bool[] _fake,
            bytes32[] _secret
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
                Bid storage bid = bids[msg.sender][i];
                (uint value, bool fake, bytes32 secret) =
                        (_values[i], _fake[i], _secret[i]);
                if (bid.blindedBid != keccak256(abi.encodePacked(value, fake, secret))) {
                    // Bid was not actually revealed.
                    // Do not refund deposit.
                    continue;
                }
                refund += bid.deposit;
                if (!fake && bid.deposit >= value) {
                    if (placeBid(msg.sender, value))
                        refund -= value;
                }
                // Make it impossible for the sender to re-claim
                // the same deposit.
                bid.blindedBid = bytes32(0);
            }
            msg.sender.transfer(refund);
        }

        // This is an "internal" function which means that it
        // can only be called from the contract itself (or from
        // derived contracts).
        function placeBid(address bidder, uint value) internal
                returns (bool success)
        {
            if (value <= highestBid) {
                return false;
            }
            if (highestBidder != address(0)) {
                // Refund the previously highest bidder.
                pendingReturns[highestBidder] += highestBid;
            }
            highestBid = value;
            highestBidder = bidder;
            return true;
        }

        /// Withdraw a bid that was overbid.
        function withdraw() public {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // It is important to set this to zero because the recipient
                // can call this function again as part of the receiving call
                // before `transfer` returns (see the remark above about
                // conditions -> effects -> interaction).
                pendingReturns[msg.sender] = 0;

                msg.sender.transfer(amount);
            }
        }

        /// End the auction and send the highest bid
        /// to the beneficiary.
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

    pragma solidity ^0.4.22;

    contract Purchase {
        uint public value;
        address public seller;
        address public buyer;
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

To be written.
