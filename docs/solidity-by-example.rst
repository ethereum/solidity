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
        function Ballot(bytes32[] proposalNames) {
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
        function giveRightToVote(address voter) {
            if (msg.sender != chairperson || voters[voter].voted) {
                // `throw` terminates and reverts all changes to
                // the state and to Ether balances. It is often
                // a good idea to use this if functions are
                // called incorrectly. But watch out, this
                // will also consume all provided gas.
                throw;
            }
            voters[voter].weight = 1;
        }

        /// Delegate your vote to the voter `to`.
        function delegate(address to) {
            // assigns reference
            Voter sender = voters[msg.sender];
            if (sender.voted)
                throw;

            // Forward the delegation as long as
            // `to` also delegated.
            // In general, such loops are very dangerous,
            // because if they run too long, they might
            // need more gas than is available in a block.
            // In this case, the delegation will not be executed,
            // but in other situations, such loops might
            // cause a contract to get "stuck" completely.
            while (
                voters[to].delegate != address(0) &&
                voters[to].delegate != msg.sender
            ) {
                to = voters[to].delegate;
            }

            // We found a loop in the delegation, not allowed.
            if (to == msg.sender) {
                throw;
            }

            // Since `sender` is a reference, this
            // modifies `voters[msg.sender].voted`
            sender.voted = true;
            sender.delegate = to;
            Voter delegate = voters[to];
            if (delegate.voted) {
                // If the delegate already voted,
                // directly add to the number of votes
                proposals[delegate.vote].voteCount += sender.weight;
            }
            else {
                // If the delegate did not vote yet,
                // add to her weight.
                delegate.weight += sender.weight;
            }
        }

        /// Give your vote (including votes delegated to you)
        /// to proposal `proposals[proposal].name`.
        function vote(uint proposal) {
            Voter sender = voters[msg.sender];
            if (sender.voted)
                throw;
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
            for (uint p = 0; p < proposals.length; p++) {
                if (proposals[p].voteCount > winningVoteCount) {
                    winningVoteCount = proposals[p].voteCount;
                    winningProposal = p;
                }
            }
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

    contract SimpleAuction {
        // Parameters of the auction. Times are either
        // absolute unix timestamps (seconds since 1970-01-01)
        // or time periods in seconds.
        address public beneficiary;
        uint public auctionStart;
        uint public biddingTime;

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
        function SimpleAuction(
            uint _biddingTime,
            address _beneficiary
        ) {
            beneficiary = _beneficiary;
            auctionStart = now;
            biddingTime = _biddingTime;
        }

        /// Bid on the auction with the value sent
        /// together with this transaction.
        /// The value will only be refunded if the
        /// auction is not won.
        function bid() {
            // No arguments are necessary, all
            // information is already part of
            // the transaction.
            if (now > auctionStart + biddingTime) {
                // Revert the call if the bidding
                // period is over.
                throw;
            }
            if (msg.value <= highestBid) {
                // If the bid is not higher, send the
                // money back.
                throw;
            }
            if (highestBidder != 0) {
                // Sending back the money by simply using
                // highestBidder.send(highestBid) is a security risk
                // because it can be prevented by the caller by e.g.
                // raising the call stack to 1023. It is always safer
                // to let the recipient withdraw their money themselves. 
                pendingReturns[highestBidder] += highestBid;
            }
            highestBidder = msg.sender;
            highestBid = msg.value;
            HighestBidIncreased(msg.sender, msg.value);
        }

        /// Withdraw a bid that was overbid.
        function withdraw() {
            var amount = pendingReturns[msg.sender];
            // It is important to set this to zero because the recipient
            // can call this function again as part of the receiving call
            // before `send` returns.
            pendingReturns[msg.sender] = 0;
            if (!msg.sender.send(amount))
                throw; // If anything fails, this will revert the changes above
        }

        /// End the auction and send the highest bid
        /// to the beneficiary.
        function auctionEnd() {
            // It is a good guideline to structure functions that interact
            // with other contracts (i.e. they call functions or send Ether)
            // into three phases:
            // 1. checking conditions
            // 2. performing actions (potentially changing conditions)
            // 3. interacting with other contracts
            // If these phases are mixed up, the other contract could call
            // back into the current contract and modify the state or cause
            // effects (ether payout) to be perfromed multiple times.
            // If functions called internally include interaction with external
            // contracts, they also have to be considered interaction with
            // external contracts.

            // 1. Conditions
            if (now <= auctionStart + biddingTime)
                throw; // auction did not yet end
            if (ended)
                throw; // this function has already been called

            // 2. Effects
            ended = true;
            AuctionEnded(highestBidder, highestBid);

            // 3. Interaction
            if (!beneficiary.send(highestBid))
                throw;
        }

        function () {
            // This function gets executed if a
            // transaction with invalid data is sent to
            // the contract or just ether without data.
            // We revert the send so that no-one
            // accidentally loses money when using the
            // contract.
            throw;
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
accepting any value that is at least as large as
the bid. Since this can of course only be checked during
the reveal phase, some bids might be **invalid**, and
this is on purpose (it even provides an explicit
flag to place invalid bids with high value transfers):
Bidders can confuse competition by placing several
high or low invalid bids.


::

    contract BlindAuction {
        struct Bid {
            bytes32 blindedBid;
            uint deposit;
        }

        address public beneficiary;
        uint public auctionStart;
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
        modifier onlyBefore(uint _time) { if (now >= _time) throw; _ }
        modifier onlyAfter(uint _time) { if (now <= _time) throw; _ }

        function BlindAuction(
            uint _biddingTime,
            uint _revealTime,
            address _beneficiary
        ) {
            beneficiary = _beneficiary;
            auctionStart = now;
            biddingEnd = now + _biddingTime;
            revealEnd = biddingEnd + _revealTime;
        }

        /// Place a blinded bid with `_blindedBid` = sha3(value,
        /// fake, secret).
        /// The sent ether is only refunded if the bid is correctly
        /// revealed in the revealing phase. The bid is valid if the
        /// ether sent together with the bid is at least "value" and
        /// "fake" is not true. Setting "fake" to true and sending
        /// not the exact amount are ways to hide the real bid but
        /// still make the required deposit. The same address can
        /// place multiple bids.
        function bid(bytes32 _blindedBid)
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
            onlyAfter(biddingEnd)
            onlyBefore(revealEnd)
        {
            uint length = bids[msg.sender].length;
            if (
                _values.length != length ||
                _fake.length != length ||
                _secret.length != length
            ) {
                throw;
            }

            uint refund;
            for (uint i = 0; i < length; i++) {
                var bid = bids[msg.sender][i];
                var (value, fake, secret) =
                        (_values[i], _fake[i], _secret[i]);
                if (bid.blindedBid != sha3(value, fake, secret)) {
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
                bid.blindedBid = 0;
            }
            if (!msg.sender.send(refund))
                throw;
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
            if (highestBidder != 0) {
                // Refund the previously highest bidder.
                pendingReturns[highestBidder] += highestBid;
            }
            highestBid = value;
            highestBidder = bidder;
            return true;
        }

        /// Withdraw a bid that was overbid.
        function withdraw() {
            var amount = pendingReturns[msg.sender];
            // It is important to set this to zero because the recipient
            // can call this function again as part of the receiving call
            // before `send` returns (see the remark above about
            // conditions -> effects -> interaction).
            pendingReturns[msg.sender] = 0;
            if (!msg.sender.send(amount))
                throw; // If anything fails, this will revert the changes above
        }

        /// End the auction and send the highest bid
        /// to the beneficiary.
        function auctionEnd()
            onlyAfter(revealEnd)
        {
            if (ended)
                throw;
            AuctionEnded(highestBidder, highestBid);
            ended = true;
            // We send all the money we have, because some
            // of the refunds might have failed.
            if (!beneficiary.send(this.balance))
                throw;
        }

        function () {
            throw;
        }
    }

.. index:: purchase, remote purchase, escrow

********************
Safe Remote Purchase
********************

::

    contract Purchase {
        uint public value;
        address public seller;
        address public buyer;
        enum State { Created, Locked, Inactive }
        State public state;

        function Purchase() {
            seller = msg.sender;
            value = msg.value / 2;
            if (2 * value != msg.value) throw;
        }

        modifier require(bool _condition) {
            if (!_condition) throw;
            _
        }

        modifier onlyBuyer() {
            if (msg.sender != buyer) throw;
            _
        }

        modifier onlySeller() {
            if (msg.sender != seller) throw;
            _
        }

        modifier inState(State _state) {
            if (state != _state) throw;
            _
        }

        event aborted();
        event purchaseConfirmed();
        event itemReceived();

        /// Abort the purchase and reclaim the ether.
        /// Can only be called by the seller before
        /// the contract is locked.
        function abort()
            onlySeller
            inState(State.Created)
        {
            aborted();
            state = State.Inactive;
            if (!seller.send(this.balance))
                throw;
        }

        /// Confirm the purchase as buyer.
        /// Transaction has to include `2 * value` ether.
        /// The ether will be locked until confirmReceived
        /// is called.
        function confirmPurchase()
            inState(State.Created)
            require(msg.value == 2 * value)
        {
            purchaseConfirmed();
            buyer = msg.sender;
            state = State.Locked;
        }

        /// Confirm that you (the buyer) received the item.
        /// This will release the locked ether.
        function confirmReceived()
            onlyBuyer
            inState(State.Locked)
        {
            itemReceived();
            // It is important to change the state first because
            // otherwise, the contracts called using `send` below
            // can call in again here.
            state = State.Inactive;
            // This actually allows both the buyer and the seller to
            // block the refund.
            if (!buyer.send(value) || !seller.send(this.balance))
                throw;
        }

        function() {
            throw;
        }
    }

********************
Micropayment Channel
********************

Right now the big idea for scaling Bitcoin is the Lightning network,
which lets people do most of their transactions off-chain and only
occasionally settle the balances on the actual blockchain. The basic
idea is called a payment channel. Let's say Alice wants to make a lot of
payments to Bob, without paying gas fees for every transaction. She sets
up a contract and deposits some ether. For each payment, she sends Bob a
signed message, saying "I agree to give $X to Bob." At any time, Bob can
post one of Alice's message to the contract, which will check the
signature and send Bob the money.

The trick is, Bob can only do this once. After he does it, the contract
remembers it's done, and refunds the remaining money to Alice. So Alice
can send Bob a series of messages, each with a higher payment. If she's
already sent Bob a message that pays 10 ether, she can pay him another
ether by sending a message that pays 11 ether.

We can also add an expiration date, after which Alice can retrieve any
money she deposited that's not already paid out. Until then, her funds
are locked. Before the deadline, Bob is perfectly safe keeping
everything offline. He just has to check the balance and deadline, and
be sure to post the message with the highest value before the deadline
expires.

There's sample code at a
`project <https://github.com/obscuren/whisper-payment-channel>`__ on
github. This version uses Whisper, which is Ethereum's built-in
messaging system. That's basically working but not enabled by default,
so it's not quite fully usable. But any communications channel can work.
In fact, this sample was made by `EtherAPIs <https://etherapis.io/>`__,
which plans to use similar code to let people send micropayments over
HTTP for API calls.

The actual smart contract code is
`here <https://github.com/obscuren/whisper-payment-channel/blob/master/contract.sol>`__.
Here's the part with the magic, simplified slightly:

::

    function verify(uint channel, address recipient, uint value, 
                    uint8 v, bytes32 r, bytes32 s) constant returns(bool) {
        PaymentChannel ch = channels[channel];
        return ch.valid && 
               ch.validUntil > block.timestamp && 
               ch.owner == ecrecover(sha3(channel, recipient, value), 
                                     v, r, s);
    }

    function claim(uint channel, address recipient, uint value, 
                   uint8 v, bytes32 r, bytes32 s) {
        if (!verify(channel, recipient, value, v, r, s)) return;
        if (msg.sender != recipient) return;

        PaymentChannel ch = channels[channel];
        channels[channel].valid = false;
        uint chval = channels[channel].value;
        uint val = value;
        if (val > chval) val = chval; 
        channels[channel].value -= val;
        if (!recipient.call.value(val)()) throw;;
    }

This contract can handle lots of payment channels, each with an owner.

Alice sends Bob a message with the following values:

-  the id of the channel she's using (since this contract can handle
   lots of channels)
-  the recipient, i.e. Bob's address
-  the value of her payment
-  her signature, consisting of the three numbers v, r, s (a standard
   elliptic curve signature)

The verify function starts by taking a hash of the channel id,
recipient, and value. The sha3 function can take any number of
parameters, and it'll just mash them together and hash it all:

::

    sha3(channel, recipient, value)

To verify the signature we use the ecrecover function, which takes a
hash and the signature (v, r, s), and returns the address that produced
that signature. We just check that to make sure the signature was made
by the channel owner:

::

    ch.owner == ecrecover(sha3(channel, recipient, value), v, r, s);

Make sure the channel is still active and the deadline hasn't passed,
and we're done verifying. The claim function first calls verify, and if
that returns true, sends the money to Bob and sets channel.valid to
false so Bob can't make withdraw any more funds.

If Alice overdraws her funds, it's up to Bob to stop accepting her
payments. In case he screws up, we check for that; if funds are
overdrawn we reduce the payment to what's available in the channel.

Only Bob is allowed to call claim(), and his incentive is to claim the
most money he can, which is exactly what we want to happen. (This hints
at another way to run a `blind
auction <http://www.blunderingcode.com/blind-auctions/>`__: just submit
signatures off-chain to the auctioneer, who submits the largest bid to
the chain!)

**Duplex channels**

Suppose Alice and Bob want to make frequent small payments to each
other. They could use two channels, but that means closing out each
channel when it runs out of funds, even if their net balances haven't
changed much. It'd be better if we had duplex channels, where payments
flow both directions.

One method is for one party to submit the current state (i.e. balances
for both parties), and allow time for the other party to submit a more
recent state. This works for any sort of state channel, but it gets a
little complicated. We have to include a nonce that increments with each
message; what if Alice and Bob send messages to each other at the same
time?

For simple value transfers there's an easier way. Instead of including a
net balance, have messages just add to the total funds sent so far by
the message sender. The contract figures net balances when the channel
closes. This keeps us from having to worry about message ordering. We
can trust both parties to send their most recent receipt, since that
will be the one that pays them the most.

To calculate the net payment to Alice, we take Alice's balance, add
Alice's total receivable, and subtract Bob's total receivable. It's ok
if the receivables exceed the balances, it just means the money's gone
back and forth a lot. As before, we adjust receivables downward if
someone overdraws.

To make this work we remove the immediate ether transfer from the claim
function, and let each party withdraw after both claims are submitted.
If one party doesn't submit a claim before the deadline, we assume they
received no money. An attacker could attempt to spam the network to
prevent the other party from submitting its receipt; to mitigate this
we'll need to make sure the channel stays open for some minimum time
period after the first claim.

**A network of channels**

But Lightning is more than two-party payment channels. It'd be pretty
hard on cash flow if you had to deposit a bunch of money in a payment
channel for everyone you might want to pay a few times. Lightning is
supposed to let you route payments through intermediaries. With a
network of payment channels, you can route your payment anywhere you
want it to go, as long as you can find a path through the network to
your payee.

The Lightning
`paper <https://lightning.network/lightning-network-paper-DRAFT-0.5.pdf>`__
(pdf) is hard to understand in detail if you don't know Bitcoin opcodes.
But the basic
`idea <https://github.com/yoursnetwork/fullnode-pc/blob/master/docs/gentle-lightning.md>`__
is really quite simple and elegant, and easy to implement on Ethereum.

Let's say Alice wants to pay 10 ether to Carol. She doesn't have a
channel to Carol but she does have a channel to Bob, who has a channel
to Carol. So the payment needs to flow from Alice to Bob to Carol.

Carol makes a random number, which we'll call Secret, and hashes it to
make HashedSecret. She gives HashedSecret to Alice.

Alice sends a message to Bob, which is just like the two-party payment
channel message, but adds the HashedSecret. To claim the money, Bob has
to submit this message to the contract along with the matching Secret.
He has to get that secret from Carol.

So he sends a similar message to Carol, with the same payment value
minus his service fee. Service fees don't have to be implemented in the
contract; each node just sends a slightly smaller payment to the next
node.

Carol of course already has the Secret, so she can immediately claim her
funds from Bob. If she does, then Bob will see the Secret on the
blockchain, and be able to claim his funds from Alice.

But instead of doing that, she can just send the Secret to Bob. Now Bob
can retrieve his money from Alice, even if Carol never touches the
blockchain again.

So at this point:

-  Carol is able to claim funds from Bob by submitting his signed
   statement and the matching secret.
-  Bob has the secret too, so he's able to claim his money from Alice
-  Bob sends the secret to Alice so she has verification that Carol got
   the payment

As we make new payments, we do the same as two-party channels, just
updating the total. This means the recipient only has to keep the most
recent secret.

To make all this work, all we have to do is slightly modify our verify
and claim functions:

::

    function verify(uint channel, address recipient, uint value, 
                    bytes32 secret, uint8 v, bytes32 r, bytes32 s) 
             constant returns(bool) {
        PaymentChannel ch = channels[channel];
        if !(ch.valid && ch.validUntil > block.timestamp) return false;
        bytes32 hashedSecret = sha3(secret)
        return ch.owner == ecrecover(sha3(channel, recipient, 
                                          hashedSecret, value), v, r, s);
    }

    function claim(uint channel, address recipient, uint value, 
                   bytes32 secret, uint8 v, bytes32 r, bytes32 s) {
        if( !verify(channel, recipient, value, secret, v, r, s) ) return;

Now the signature is over the sha3 of the channel, recipient,
hashedSecret, and value. And we're passing in the secret, and verifying
that it hashes to what's in the signature.

**Early Shutdown**

Imagine that Alice want to pay Dave, and routes the payment through Bob
and then Carol. So this is payment ABCD. Let's say this is the first
payment in the BC channel, so Bob's total accumulated payment balance to
Carol is just the ABCD amount. But Dave never reveals the secret.

Now Eddie wants to pay Fred, also through Bob and Carol, making payment
EBCF.

To process EBCF, Bob has to add Eddie's payment amount on top of
Carol's, so the total accumulated payment on BC is ABCD + EBCF. But
Carol can redeem that balance with just the secret from Fred.

Bob can use Fred's secret to claim the money from Eddie. But without
Dave's secret, he can't claim the money from Alice, so he eats a loss in
the amount of the ABCD payment.

So Bob has to avoid putting new payments on the BC channel while there's
an unrevealed secret. (It's tempting to think he could issue EBCF with a
total that assumes ABCD didn't exist, but what if the secret's revealed
later?)

This means we should let nodes shut down their channels early, so they
can restart if they stall. Over time, people will settle on reliable
partners.

This also means that channels are completely synchronous, which isn't
ideal for a scalability solution. Fast webservers don't process one
request at a time; they can accept lots of requests and send each
response whenever it's ready. But a Lightning channel has to go through
a complete request-response before it can accept another request. I
think this is also the case with Bitcoin's Lightning. Still, compared to
putting every transaction on chain, we can do pretty well.

Maybe these synchronous channels help avoid centralization. Since each
channel has limited throughput, users are better off routing through
low-traffic channels.

**Routing**

Speaking of routing, it's really easy because we can do it all locally
on the client. All the channels are set up on chain, so the client can
just read them all into memory and use whatever routing algorithm it
likes. Then it can send the complete route in the off-chain message.
This also lets the sender figure out the total transaction fees that
will be charged by all the intermediaries.

To make this easy, we can just use events to log each new channel. The
javascript API can query up to three indexed properties, so we index on
the two endpoint addresses and the expiration. We'll also log the
off-chain contact info for each address; it could be http, email,
whatever. The javascript queries the channels, asks the endpoints how
much funds they have available, and constructs a route.

**The Contract**

So now we can do pretty much does what Lightning does, with a contract
that's two pages long. We'll need client code to handle the routing and
messaging, but the on-chain infrastructure is really simple and works
without any changes to Ethereum. Here's the whole contract.

::

    contract Lightning {

    modifier noeth() { if (msg.value > 0) throw; _ }
    function() noeth {}

    uint finalizationDelay = 10000;

    event LogUser(address indexed user, string contactinfo);
    event LogChannel(address indexed user, address indexed bob, 
                     uint indexed expireblock, uint channelnum);
    event LogClaim(uint indexed channel, bytes32 secret);

    struct Endpoint {
        uint96 balance;
        uint96 receivable;
        bool paid;
        bool closed;
    }

    struct Channel {
        uint expireblock;
        address alice;
        address bob;
        mapping (address => Endpoint) endpoints;
    }

    mapping (uint => Channel) channels;
    uint maxchannel;

    function registerUser(string contactinfo) noeth {
        LogUser(msg.sender, contactinfo);
    }

    function makeChannel(address alice, address bob, uint expireblock) noeth {
        maxchannel += 1;
        channels[maxchannel].alice = alice;
        channels[maxchannel].bob = bob;
        channels[maxchannel].expireblock = expireblock;
        LogChannel(alice, bob, expireblock, maxchannel);
    }

    function deposit(uint channel) {
        Channel ch = channels[channel];
        if (ch.alice != msg.sender && ch.bob != msg.sender) throw;
        ch.endpoints[msg.sender].balance += uint96(msg.value);
    }

    function channelExpired(uint channel) private returns (bool) {
        return channels[channel].expireblock < block.number;
    }

    function channelClosed(uint channel) private returns (bool) {
        Channel ch = channels[channel];
        return channelExpired(channel) || 
               (ch.endpoints[ch.alice].closed && 
                ch.endpoints[ch.bob].closed);
    }

    //Sig must be valid, 
    //signer must be one endpoint and recipient the other
    function verify(uint channel, address recipient, uint value, 
                    bytes32 secret, uint8 v, bytes32 r, bytes32 s) 
             private returns(bool) {
        bytes32 hashedSecret = sha3(secret);
        address signer = ecrecover(sha3(channel, recipient, 
                                        hashedSecret, value), 
                                   v, r, s);
        Channel ch = channels[channel];
        return (signer == ch.alice && recipient == ch.bob) ||
               (signer == ch.bob && recipient == ch.alice);
    }

    function claim(uint channel, address recipient, uint96 value, 
                   bytes32 secret, uint8 v, bytes32 r, bytes32 s) noeth {
        Channel ch = channels[channel];
        Endpoint ep = ch.endpoints[recipient];
        if ( !verify(channel, recipient, value, secret, v, r, s) 
           || channelClosed(channel) 
           || ep.receivable + ep.balance < ep.balance ) return;

        ep.closed = true;
        ep.receivable = value;

        //if this is first claim,
        //make sure other party has sufficient time to submit claim
        if (!channelClosed(channel) && 
            ch.expireblock < block.number + finalizationDelay) {
            ch.expireblock = block.number + finalizationDelay;
        }
        LogClaim(channel, secret);
    }

    function withdraw(uint channel) noeth {
        Channel ch = channels[channel];
        if ( (msg.sender != ch.alice && msg.sender != ch.bob)
           || ch.endpoints[msg.sender].paid
           || !channelClosed(channel) ) return;

        Endpoint alice = ch.endpoints[ch.alice];
        Endpoint bob = ch.endpoints[ch.bob];
        uint alicereceivable = alice.receivable;
        uint bobreceivable = bob.receivable;

        //if anyone overdrew, just take what they have
        if (alicereceivable > bob.balance + bob.receivable) {
            alicereceivable = bob.balance + bob.receivable;
        } 
        if (bobreceivable > alice.balance + alice.receivable) {
            bobreceivable = alice.balance + alice.receivable;
        }

        uint alicenet = alice.balance - bobreceivable + alicereceivable;
        uint bobnet = bob.balance - alicereceivable + bobreceivable;

        //make double sure a bug can't drain from other channels...
        if (alicenet + bobnet > alice.balance + bob.balance) return;

        uint net;
        if (msg.sender == ch.alice) {
            net = alicenet;
        } else {
            net = bobnet;
        }

        ch.endpoints[msg.sender].paid = true;
        if (!msg.sender.call.value(net)()) throw;
    }
    }

**Tokens**

The code above does everything with ether. But it wouldn't be hard to
extend it to use other tokens. Set the token address when creating a
channel, change the deposit and withdraw functions, and you're done.

