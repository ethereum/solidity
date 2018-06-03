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

        /// Place a blinded bid with `_blindedBid` = keccak256(value,
        /// fake, secret).
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
                if (bid.blindedBid != keccak256(value, fake, secret)) {
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
            if (highestBidder != 0) {
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

Creating and verifying signatures
=================================

Signatures are used to authorize transactions,
and they're a general tool that's available to
smart contracts. In this chapter we'll use it to
prove to a smart contract that a certain account
approved a certain message. We'll build a simple
smart contract that lets me transmit ether, but
in a unsual way, instead of calling a function myself
to initiate a payment, I'll let the receiver of
the payment do that, and therefore pay the transaction
fee. All this using cryptographics signatures that I'll
make and send off-chain (e.g. via email). We'll call our
contract as ReceiverPays, it will works a lot like
writing a check.Our contract will works like that:

    1. The owner deploys ReceiverPays, attaching enough ether to cover the payments that will be made.
    2. The owner authorizes a payment by signing a message with their private key.
    3. The owner sends the cryptographically signed message to the designated recipient. The message does not need to be kept secret (you'll understand it later), and the mechanism for sending it doesn't matter.
    4. The recipient claims their payment by presenting the signed message to the smart contract, it verifies the authenticity of the message and then releases the funds.

Creating the signature
----------------------

We don't need the to interact with Ethereum network to
do that. The proccess is completely offline, and the
every major programming language has the libraries to do it.
The signature algorithm Ethereum has the support for is the
`Elliptic Curve Signature Algorithm(EDCSA) <https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm>`_.
In this tutorial, we'll signing messages in the web browser
using web3.js and MetaMask. We can sign messages in too many ways,
but some of them are incompatible. We'll use the new standard
way to do that `EIP-762 <https://github.com/ethereum/EIPs/pull/712>`_,
it provides a number of other security benefits.
Is recommended to stick with the most standard format of signing,
as specified by the `eth_sign JSON-RPC method <https://github.com/ethereum/wiki/wiki/JSON-RPC#eth_sign>`_
In MetaMask, this algorithm is followed best by the web3.js function `web3.personal.sign`,
doing it using:

::

    /// Hashing first makes a few things easier
    var hash = web3.sha3("message to sign");
    web3.personal.sign(hash, web3.eth.defaultAccount, function () {...});


Remind that the prefix includes the length of the message.
Hashing first means the message will always be 32 bytes long,
which means the prefix is always the same, this makes everything easier.

What to Sign
------------

For a contract that fulfills payments, the signed message must include:

    1. The recipient's address
    2. The amount to be transferred
    3. An additional data to protects agains replay attacks

To avoid replay attacks we'll use the same as in Ethereum transactions
themselves, a nonce. And our smart contract will check if that nonce is reused.
There's another type of replay attacks, it occurs when the
owner deploy a ReceiverPays smart contract, make some payments,
and then destroy the contract. Later, he decide to deploy the
RecipientPays smart contract again, but the new contract doesn't
know the nonces used in the previous deployment, so the attacker
can use the old messages again. We can protect agains it including
the contract's address in our message, and only accepting the
messages containing contract's address itself.
You can see this function reading the  *first two lines* in the *claimPayment()* function in the full contract,
it is at the end of this chapter.

Packing arguments
-----------------

Now that we have identified what information to include in the
signed message, we're ready to put the message together, hash it,
and sign it. Solidity's `keccak256/sha3 function <http://solidity.readthedocs.io/en/develop/units-and-global-variables.html#mathematical-and-cryptographic-functions>`_
hashes by first concatenating them in a tightly packed form.
For the hash generated on the client match the one generated in the smart contract,
the arguments must be concatenated in the same way. The 
`ethereumjs-abi <https://github.com/ethereumjs/ethereumjs-abi>`_ library provides
a function called `soliditySHA3` that mimics the behavior
of Solidity's `keccak256` function.
Putting it all together, here's a **JavaScript function** that
creates the proper signature for the `ReceiverPays` example:

::

    /// recipient is the address that should be paid.
    /// amount, in wei, specifies how much ether should be sent.
    /// nonce can be any unique number, do you remind the replay attacks? we are preventing them here
    /// contractAddress do you remind the cross-contract replay attacks?
    function signPayment(recipient, amount, nonce, contractAddress, callback) {
        var hash = "0x" + ethereumjs.ABI.soliditySHA3(
            ["address", "uint256", "uint256", "address"],
            [recipient, amount, nonce, contractAddress]
        ).toString("hex");
        
        web3.personal.sign(hash, web3.eth.defaultAccount, callback);
    }

Recovering the Message Signer in Solidity
-----------------------------------------

In general, ECDSA signatures consist of two parameters, *r* and *s*.
Signatures in Ethereum include a thir parameter *v*, that can be used
to recover which account's private key was used to sign in the message,
the transaction's sender. Solidity provides a built-in function `ecrecover <https://solidity.readthedocs.io/en/develop/units-and-global-variables.html#mathematical-and-cryptographic-functions>`_
that accepts a message along with the *r*, *s* and *v* parameters and
returns the address that was used to sign the message.

Extracting the Signature Parameters
-----------------------------------

Signatures produced by web3.js are the concatenation of *r*, *s* and *v*,
so the 1st step is sppliting those parameters back out. It can be done on the client,
but doing it inside the smart contract means only one signature parameter
needs to be sent rather than three.
Sppliting apart a bytes array into component parts is a little messy.
We'll use the `inline assembly <https://solidity.readthedocs.io/en/develop/assembly.html>`_ to do the job
in the *splitSignature* function, it is the 3rd function in the full contract, this is at the end of this
chapter.

Computing the Message Hash
--------------------------
 
The smart contract needs to know exactly what parameters were signed,
and so it must recreate the message from the parameters and use that
for signature verification. We need the functions *prefixed* and
*recoverSigner* to do it, in function *claimPayment* you can see the
use of that functions.


The full contract
-----------------

::

    pragma solidity ^0.4.20;

    contract ReceiverPays {
        address owner = msg.sender;

        mapping(uint256 => bool) usedNonces;

        function ReceiverPays() public payable {}

        function claimPayment(uint256 amount, uint256 nonce, bytes signature) public {
            require(!usedNonces[nonce]);
            usedNonces[nonce] = true;

            // this recreates the message that was signed on the client
            bytes32 message = prefixed(keccak256(msg.sender, amount, nonce, this));

            require(recoverSigner(message,sig) == owner);

            msg.sender.transfer(amount);
        }

        /// destroy the contract and reclaim the leftover funds.
        function kill() public {
            require(msg.sender == owner);
            selfdestruct(msg.sender);
        }

        /// signature methods.
        function splitSignature(bytes sig)
            internal
            pure
            returns (uint8, bytes32, bytes32)
        {
            require(sig.length == 65);

            bytes32 r;
            bytes32 s;
            uint8 v;

            assembly {
                // first 32 bytes, after the length prefix.
                r := mload(add(sig,32))
                // second 32 bytes.
                s := mload(add(sig,64))
                // final byte (first byte of the next 32 bytes).
                v := byte(0, mload(add(sig, 96)))
            }

            return (v, r, s);
        }

        function recoverSigner(bytes32 message, bytes sig)
            internal
            pure
            returns (address)
        {
            uint8;
            bytes32 r;
            bytes32 s;

            (v, r, s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure return (bytes32) {
            return keccak256("\x19Ethereum Signed Message:\n32", hash);
        }
    }



Writing a Simple Payment Channel
================================

We'll build a simple, but complete, implementation of a payment channel.
Payment channels use cryptographic signatures to make repeated transfers
of ether securely, instantaneously, and without transaction fees. but...

What is a Payment Channel?
--------------------------

Payment channels allow participants to make repeated transfers of ether without
using transactions. This means that the delays and fees associated with transactions
can be avoided. We're going to explore a simple unidirectional payment channel between
two parties. Using it involves three steps:

    1. The sender funds a smart contract with ether. This "opens" the payment channel.
    2. The sender signs messages that specify how much of that ether is owed to the recipient. This step is repeated for each payment.
    3. The recipient "closes" the payment channel, withdrawing their portion of the ether and sending the remainder back to the sender.
    
A note: only steps 1 and 3 require Ethereum transactions, the step 2 means that
the sender a cryptographic signature to the recipient via off chain ways (e.g. email).
This means only two transactions are required to support any number of transfers.

The recipient is guaranteed to receive their funds because the smart contract escrows
the ether and honors a valid signed message. The smart contract also enforces a timeout,
so the sender is guaranteed to eventually recover their funds even if the recipient refuses
to close the channel.
It's up to the participants in a payment channel to decide how long to keep it open.
For a short-lived transaction, such as paying an internet cafe for each minute of network access,
or for a longer relationship, such as paying an employee an hourly wage, a payment could last for months or years.

Opening the Payment Channel
---------------------------

To open the payment channel, the sender deploys the smart contract,
attaching the ether to be escrowed and specifying the intendend recipient
and a maximum duration for the channel to exist. It is the function
*SimplePaymentChannel* in the contract, that is at the end of this chapter.

Making Payments
---------------

The sender makes payments by sending messages to the recipient.
This step is performed entirely outside of the Ethereum network.
Messages are cryptographically signed by the sender and then transmitted directly to the recipient.

Each message includes the following information:

    * The smart contract's address, used to prevent cross-contract replay attacks.
    * The total amount of ether that is owed the recipient so far.
    
A payment channel is closed just once, at the of a series of transfers.
Because of this, only one of the messages sent will be redeemed. This is why
each message specifies a cumulative total amount of ether owed, rather than the
amount of the individual micropayment. The recipient will naturally choose to
redeem the most recent message because that's the one with the highest total.
We don't need anymore the nonce per-message, because the smart contract will
only honor a single message. The address of the smart contract is still used
to prevent a message intended for one payment channel from being used for a different channel.

Here's the modified **javascript code** to cryptographic a message from the previous chapter:

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
    // amount, in wei, specifies how much ether should be sent.
    
    function signPayment(contractAddress, amount, callback) {
        var message = constructPaymentMessage(contractAddress, amount);
        signMessage(message, callback);
    }

Closing the Payment Channel
---------------------------

When the recipient is ready to receive their funds, it's time to
close the payment channel by calling a *close* function on the smart contract.
Closing the channel pays the recipient the ether they're owed and destroys the contract,
sending any remaining ether back to the sender.
To close the channel, the recipient needs to share a message signed by the sender.

The smart contract must verify that the message contains a valid signature from the sender.
The process for doing this verification is the same as the process the recipient uses.
The Solidity functions *isValidSignature* and *recoverSigner* work just like their
JavaScript counterparts in the previous section. The latter is borrowed from the
*ReceiverPays* contract in the previous chapter.

The *close* function can only be called by the payment channel recipient,
who will naturally pass the most recent payment message because that message
carries the highest total owed. If the sender were allowed to call this function,
they could provide a message with a lower amount and cheat the recipient out of what they're owed.

The function verifies the signed message matches the given parameters.
If everything checks out, the recipient is sent their portion of the ether,
and the sender is sent the rest via a *selfdestruct*.
You can see the *close* function in the full contract.

Channel Expiration
-------------------

The recipient can close the payment channel at any time, but if they fail to do so,
the sender needs a way to recover their escrowed funds. An *expiration* time was set
at the time of contract deployment. Once that time is reached, the sender can call
*claimTimeout* to recover their funds. You can see the *claimTimeout* function in the
full contract.

After this function is called, the recipient can no longer receive any ether,
so it's important that the recipient close the channel before the expiration is reached.


The full contract
-----------------

::

    pragma solidity ^0.4.20;

    contract SimplePaymentChannel {
        address public sender;      // The account sending payments.
        address public recipient;   // The account receiving the payments.
        uint256 public expiration;  // Timeout in case the recipient never closes.

        function SimplePaymentChannel(address _recipient, uint256 duration)
            public
            payable
        {
            sender = msg.sender;
            recipient = _recipient;
            expiration = now + duration;
        }

        function isValidSignature(uint256 amount, bytes signature)
        internal
        view
        returns (bool)
        {
            bytes32 message = prefixed(keccak256(this, amount));

            // check that the signature is from the payment sender
            return recoverSigner(message, signature) == sender;
        }

        /// the recipient can close the channel at any time by presenting a
        /// signed amount from the sender. the recipient will be sent that amount,
        /// and the remainder will go back to the sender
        function close(uint256 amount, bytes signature) public {
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
        /// then the ether is realeased back to the sender.
        funtion clainTimeout() public {
            require(now >= expiration);
            selfdestruct(sender);
        }

        /// from here to the end of this contract, all the functions we already wrote, in
        /// the 'creating and verifying signatures' chapter, so if you already know what them
        /// does, you can skip it.

        /// the same functions we wrote in the 'creating and verifying signatures' chapter,
        /// you can go there to find the full explanations

        function splitSignature(bytes sig)
            internal
            pure
            returns (uint8, bytes32, bytes32)
        {
            require(sig.length == 65);

            bytes32 r;
            bytes32 s;
            uint8 v;

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
        
        function recoverSigner(bytes32 message, bytes sig)
            internal
            pure
            returns (address)
        {
            uint8 v;
            bytes32 r;
            bytes32 s;

            (v, r, s) = splitSignature(sig);

            return ecrecover(message, v, r, s);
        }

        /// builds a prefixed hash to mimic the behavior of eth_sign.
        function prefixed(bytes32 hash) internal pure returns (bytes32) {
            return keccak256("\x19Ethereum Signed Message:\n32", hash);
        }
    }



  

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
    3. Verify that the new total does not exceed the amount of ether escrowed.
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
