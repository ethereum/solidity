.. index:: auction;blind, auction;open, blind auction, open auction

*************
Blind Auction
*************

In this section, we will show how easy it is to create a completely blind
auction contract on Ethereum.  We will start with an open auction where
everyone can see the bids that are made and then extend this contract into a
blind auction where it is not possible to see the actual bid until the bidding
period ends.

.. _simple_auction:

Simple Open Auction
===================

The general idea of the following simple auction contract is that everyone can
send their bids during a bidding period. The bids already include sending money
/ Ether in order to bind the bidders to their bid. If the highest bid is
raised, the previous highest bidder gets their money back.  After the end of
the bidding period, the contract has to be called manually for the beneficiary
to receive their money - contracts cannot activate themselves.

Open in `Remix <http://remix.ethereum.org/?code=Ly8gU1BEWC1MaWNlbnNlLUlkZW50aWZpZXI6IEdQTC0zLjAKcHJhZ21hIHNvbGlkaXR5IF4wLjguNDsKY29udHJhY3QgU2ltcGxlQXVjdGlvbiB7CiAgICAvLyBQYXJhbWV0ZXJzIG9mIHRoZSBhdWN0aW9uLiBUaW1lcyBhcmUgZWl0aGVyCiAgICAvLyBhYnNvbHV0ZSB1bml4IHRpbWVzdGFtcHMgKHNlY29uZHMgc2luY2UgMTk3MC0wMS0wMSkKICAgIC8vIG9yIHRpbWUgcGVyaW9kcyBpbiBzZWNvbmRzLgogICAgYWRkcmVzcyBwYXlhYmxlIHB1YmxpYyBiZW5lZmljaWFyeTsKICAgIHVpbnQgcHVibGljIGF1Y3Rpb25FbmRUaW1lOwoKICAgIC8vIEN1cnJlbnQgc3RhdGUgb2YgdGhlIGF1Y3Rpb24uCiAgICBhZGRyZXNzIHB1YmxpYyBoaWdoZXN0QmlkZGVyOwogICAgdWludCBwdWJsaWMgaGlnaGVzdEJpZDsKCiAgICAvLyBBbGxvd2VkIHdpdGhkcmF3YWxzIG9mIHByZXZpb3VzIGJpZHMKICAgIG1hcHBpbmcoYWRkcmVzcyA9PiB1aW50KSBwZW5kaW5nUmV0dXJuczsKCiAgICAvLyBTZXQgdG8gdHJ1ZSBhdCB0aGUgZW5kLCBkaXNhbGxvd3MgYW55IGNoYW5nZS4KICAgIC8vIEJ5IGRlZmF1bHQgaW5pdGlhbGl6ZWQgdG8gYGZhbHNlYC4KICAgIGJvb2wgZW5kZWQ7CgogICAgLy8gRXZlbnRzIHRoYXQgd2lsbCBiZSBlbWl0dGVkIG9uIGNoYW5nZXMuCiAgICBldmVudCBIaWdoZXN0QmlkSW5jcmVhc2VkKGFkZHJlc3MgYmlkZGVyLCB1aW50IGFtb3VudCk7CiAgICBldmVudCBBdWN0aW9uRW5kZWQoYWRkcmVzcyB3aW5uZXIsIHVpbnQgYW1vdW50KTsKCiAgICAvLyBFcnJvcnMgdGhhdCBkZXNjcmliZSBmYWlsdXJlcy4KCiAgICAvLyBUaGUgdHJpcGxlLXNsYXNoIGNvbW1lbnRzIGFyZSBzby1jYWxsZWQgbmF0c3BlYwogICAgLy8gY29tbWVudHMuIFRoZXkgd2lsbCBiZSBzaG93biB3aGVuIHRoZSB1c2VyCiAgICAvLyBpcyBhc2tlZCB0byBjb25maXJtIGEgdHJhbnNhY3Rpb24gb3IKICAgIC8vIHdoZW4gYW4gZXJyb3IgaXMgZGlzcGxheWVkLgoKICAgIC8vLyBUaGUgYXVjdGlvbiBoYXMgYWxyZWFkeSBlbmRlZC4KICAgIGVycm9yIEF1Y3Rpb25BbHJlYWR5RW5kZWQoKTsKICAgIC8vLyBUaGVyZSBpcyBhbHJlYWR5IGEgaGlnaGVyIG9yIGVxdWFsIGJpZC4KICAgIGVycm9yIEJpZE5vdEhpZ2hFbm91Z2godWludCBoaWdoZXN0QmlkKTsKICAgIC8vLyBUaGUgYXVjdGlvbiBoYXMgbm90IGVuZGVkIHlldC4KICAgIGVycm9yIEF1Y3Rpb25Ob3RZZXRFbmRlZCgpOwogICAgLy8vIFRoZSBmdW5jdGlvbiBhdWN0aW9uRW5kIGhhcyBhbHJlYWR5IGJlZW4gY2FsbGVkLgogICAgZXJyb3IgQXVjdGlvbkVuZEFscmVhZHlDYWxsZWQoKTsKCiAgICAvLy8gQ3JlYXRlIGEgc2ltcGxlIGF1Y3Rpb24gd2l0aCBgX2JpZGRpbmdUaW1lYAogICAgLy8vIHNlY29uZHMgYmlkZGluZyB0aW1lIG9uIGJlaGFsZiBvZiB0aGUKICAgIC8vLyBiZW5lZmljaWFyeSBhZGRyZXNzIGBfYmVuZWZpY2lhcnlgLgogICAgY29uc3RydWN0b3IoCiAgICAgICAgdWludCBfYmlkZGluZ1RpbWUsCiAgICAgICAgYWRkcmVzcyBwYXlhYmxlIF9iZW5lZmljaWFyeQogICAgKSB7CiAgICAgICAgYmVuZWZpY2lhcnkgPSBfYmVuZWZpY2lhcnk7CiAgICAgICAgYXVjdGlvbkVuZFRpbWUgPSBibG9jay50aW1lc3RhbXAgKyBfYmlkZGluZ1RpbWU7CiAgICB9CgogICAgLy8vIEJpZCBvbiB0aGUgYXVjdGlvbiB3aXRoIHRoZSB2YWx1ZSBzZW50CiAgICAvLy8gdG9nZXRoZXIgd2l0aCB0aGlzIHRyYW5zYWN0aW9uLgogICAgLy8vIFRoZSB2YWx1ZSB3aWxsIG9ubHkgYmUgcmVmdW5kZWQgaWYgdGhlCiAgICAvLy8gYXVjdGlvbiBpcyBub3Qgd29uLgogICAgZnVuY3Rpb24gYmlkKCkgcHVibGljIHBheWFibGUgewogICAgICAgIC8vIE5vIGFyZ3VtZW50cyBhcmUgbmVjZXNzYXJ5LCBhbGwKICAgICAgICAvLyBpbmZvcm1hdGlvbiBpcyBhbHJlYWR5IHBhcnQgb2YKICAgICAgICAvLyB0aGUgdHJhbnNhY3Rpb24uIFRoZSBrZXl3b3JkIHBheWFibGUKICAgICAgICAvLyBpcyByZXF1aXJlZCBmb3IgdGhlIGZ1bmN0aW9uIHRvCiAgICAgICAgLy8gYmUgYWJsZSB0byByZWNlaXZlIEV0aGVyLgoKICAgICAgICAvLyBSZXZlcnQgdGhlIGNhbGwgaWYgdGhlIGJpZGRpbmcKICAgICAgICAvLyBwZXJpb2QgaXMgb3Zlci4KICAgICAgICBpZiAoYmxvY2sudGltZXN0YW1wID4gYXVjdGlvbkVuZFRpbWUpCiAgICAgICAgICAgIHJldmVydCBBdWN0aW9uQWxyZWFkeUVuZGVkKCk7CgogICAgICAgIC8vIElmIHRoZSBiaWQgaXMgbm90IGhpZ2hlciwgc2VuZCB0aGUKICAgICAgICAvLyBtb25leSBiYWNrICh0aGUgcmV2ZXJ0IHN0YXRlbWVudAogICAgICAgIC8vIHdpbGwgcmV2ZXJ0IGFsbCBjaGFuZ2VzIGluIHRoaXMKICAgICAgICAvLyBmdW5jdGlvbiBleGVjdXRpb24gaW5jbHVkaW5nCiAgICAgICAgLy8gaXQgaGF2aW5nIHJlY2VpdmVkIHRoZSBtb25leSkuCiAgICAgICAgaWYgKG1zZy52YWx1ZSA8PSBoaWdoZXN0QmlkKQogICAgICAgICAgICByZXZlcnQgQmlkTm90SGlnaEVub3VnaChoaWdoZXN0QmlkKTsKCiAgICAgICAgaWYgKGhpZ2hlc3RCaWQgIT0gMCkgewogICAgICAgICAgICAvLyBTZW5kaW5nIGJhY2sgdGhlIG1vbmV5IGJ5IHNpbXBseSB1c2luZwogICAgICAgICAgICAvLyBoaWdoZXN0QmlkZGVyLnNlbmQoaGlnaGVzdEJpZCkgaXMgYSBzZWN1cml0eSByaXNrCiAgICAgICAgICAgIC8vIGJlY2F1c2UgaXQgY291bGQgZXhlY3V0ZSBhbiB1bnRydXN0ZWQgY29udHJhY3QuCiAgICAgICAgICAgIC8vIEl0IGlzIGFsd2F5cyBzYWZlciB0byBsZXQgdGhlIHJlY2lwaWVudHMKICAgICAgICAgICAgLy8gd2l0aGRyYXcgdGhlaXIgbW9uZXkgdGhlbXNlbHZlcy4KICAgICAgICAgICAgcGVuZGluZ1JldHVybnNbaGlnaGVzdEJpZGRlcl0gKz0gaGlnaGVzdEJpZDsKICAgICAgICB9CiAgICAgICAgaGlnaGVzdEJpZGRlciA9IG1zZy5zZW5kZXI7CiAgICAgICAgaGlnaGVzdEJpZCA9IG1zZy52YWx1ZTsKICAgICAgICBlbWl0IEhpZ2hlc3RCaWRJbmNyZWFzZWQobXNnLnNlbmRlciwgbXNnLnZhbHVlKTsKICAgIH0KCiAgICAvLy8gV2l0aGRyYXcgYSBiaWQgdGhhdCB3YXMgb3ZlcmJpZC4KICAgIGZ1bmN0aW9uIHdpdGhkcmF3KCkgcHVibGljIHJldHVybnMgKGJvb2wpIHsKICAgICAgICB1aW50IGFtb3VudCA9IHBlbmRpbmdSZXR1cm5zW21zZy5zZW5kZXJdOwogICAgICAgIGlmIChhbW91bnQgPiAwKSB7CiAgICAgICAgICAgIC8vIEl0IGlzIGltcG9ydGFudCB0byBzZXQgdGhpcyB0byB6ZXJvIGJlY2F1c2UgdGhlIHJlY2lwaWVudAogICAgICAgICAgICAvLyBjYW4gY2FsbCB0aGlzIGZ1bmN0aW9uIGFnYWluIGFzIHBhcnQgb2YgdGhlIHJlY2VpdmluZyBjYWxsCiAgICAgICAgICAgIC8vIGJlZm9yZSBgc2VuZGAgcmV0dXJucy4KICAgICAgICAgICAgcGVuZGluZ1JldHVybnNbbXNnLnNlbmRlcl0gPSAwOwoKICAgICAgICAgICAgaWYgKCFwYXlhYmxlKG1zZy5zZW5kZXIpLnNlbmQoYW1vdW50KSkgewogICAgICAgICAgICAgICAgLy8gTm8gbmVlZCB0byBjYWxsIHRocm93IGhlcmUsIGp1c3QgcmVzZXQgdGhlIGFtb3VudCBvd2luZwogICAgICAgICAgICAgICAgcGVuZGluZ1JldHVybnNbbXNnLnNlbmRlcl0gPSBhbW91bnQ7CiAgICAgICAgICAgICAgICByZXR1cm4gZmFsc2U7CiAgICAgICAgICAgIH0KICAgICAgICB9CiAgICAgICAgcmV0dXJuIHRydWU7CiAgICB9CgogICAgLy8vIEVuZCB0aGUgYXVjdGlvbiBhbmQgc2VuZCB0aGUgaGlnaGVzdCBiaWQKICAgIC8vLyB0byB0aGUgYmVuZWZpY2lhcnkuCiAgICBmdW5jdGlvbiBhdWN0aW9uRW5kKCkgcHVibGljIHsKICAgICAgICAvLyBJdCBpcyBhIGdvb2QgZ3VpZGVsaW5lIHRvIHN0cnVjdHVyZSBmdW5jdGlvbnMgdGhhdCBpbnRlcmFjdAogICAgICAgIC8vIHdpdGggb3RoZXIgY29udHJhY3RzIChpLmUuIHRoZXkgY2FsbCBmdW5jdGlvbnMgb3Igc2VuZCBFdGhlcikKICAgICAgICAvLyBpbnRvIHRocmVlIHBoYXNlczoKICAgICAgICAvLyAxLiBjaGVja2luZyBjb25kaXRpb25zCiAgICAgICAgLy8gMi4gcGVyZm9ybWluZyBhY3Rpb25zIChwb3RlbnRpYWxseSBjaGFuZ2luZyBjb25kaXRpb25zKQogICAgICAgIC8vIDMuIGludGVyYWN0aW5nIHdpdGggb3RoZXIgY29udHJhY3RzCiAgICAgICAgLy8gSWYgdGhlc2UgcGhhc2VzIGFyZSBtaXhlZCB1cCwgdGhlIG90aGVyIGNvbnRyYWN0IGNvdWxkIGNhbGwKICAgICAgICAvLyBiYWNrIGludG8gdGhlIGN1cnJlbnQgY29udHJhY3QgYW5kIG1vZGlmeSB0aGUgc3RhdGUgb3IgY2F1c2UKICAgICAgICAvLyBlZmZlY3RzIChldGhlciBwYXlvdXQpIHRvIGJlIHBlcmZvcm1lZCBtdWx0aXBsZSB0aW1lcy4KICAgICAgICAvLyBJZiBmdW5jdGlvbnMgY2FsbGVkIGludGVybmFsbHkgaW5jbHVkZSBpbnRlcmFjdGlvbiB3aXRoIGV4dGVybmFsCiAgICAgICAgLy8gY29udHJhY3RzLCB0aGV5IGFsc28gaGF2ZSB0byBiZSBjb25zaWRlcmVkIGludGVyYWN0aW9uIHdpdGgKICAgICAgICAvLyBleHRlcm5hbCBjb250cmFjdHMuCgogICAgICAgIC8vIDEuIENvbmRpdGlvbnMKICAgICAgICBpZiAoYmxvY2sudGltZXN0YW1wIDwgYXVjdGlvbkVuZFRpbWUpCiAgICAgICAgICAgIHJldmVydCBBdWN0aW9uTm90WWV0RW5kZWQoKTsKICAgICAgICBpZiAoZW5kZWQpCiAgICAgICAgICAgIHJldmVydCBBdWN0aW9uRW5kQWxyZWFkeUNhbGxlZCgpOwoKICAgICAgICAvLyAyLiBFZmZlY3RzCiAgICAgICAgZW5kZWQgPSB0cnVlOwogICAgICAgIGVtaXQgQXVjdGlvbkVuZGVkKGhpZ2hlc3RCaWRkZXIsIGhpZ2hlc3RCaWQpOwoKICAgICAgICAvLyAzLiBJbnRlcmFjdGlvbgogICAgICAgIGJlbmVmaWNpYXJ5LnRyYW5zZmVyKGhpZ2hlc3RCaWQpOwogICAgfQp9>`_

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;
    contract SimpleAuction {
        // Parameters of the auction. Times are either
        // absolute unix timestamps (seconds since 1970-01-01)
        // or time periods in seconds.
        address payable public beneficiary;
        uint public auctionEndTime;

        // Current state of the auction.
        address public highestBidder;
        uint public highestBid;

        // Allowed withdrawals of previous bids
        mapping(address => uint) pendingReturns;

        // Set to true at the end, disallows any change.
        // By default initialized to `false`.
        bool ended;

        // Events that will be emitted on changes.
        event HighestBidIncreased(address bidder, uint amount);
        event AuctionEnded(address winner, uint amount);

        // Errors that describe failures.

        // The triple-slash comments are so-called natspec
        // comments. They will be shown when the user
        // is asked to confirm a transaction or
        // when an error is displayed.

        /// The auction has already ended.
        error AuctionAlreadyEnded();
        /// There is already a higher or equal bid.
        error BidNotHighEnough(uint highestBid);
        /// The auction has not ended yet.
        error AuctionNotYetEnded();
        /// The function auctionEnd has already been called.
        error AuctionEndAlreadyCalled();

        /// Create a simple auction with `_biddingTime`
        /// seconds bidding time on behalf of the
        /// beneficiary address `_beneficiary`.
        constructor(
            uint _biddingTime,
            address payable _beneficiary
        ) {
            beneficiary = _beneficiary;
            auctionEndTime = block.timestamp + _biddingTime;
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
            if (block.timestamp > auctionEndTime)
                revert AuctionAlreadyEnded();

            // If the bid is not higher, send the
            // money back (the revert statement
            // will revert all changes in this
            // function execution including
            // it having received the money).
            if (msg.value <= highestBid)
                revert BidNotHighEnough(highestBid);

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

                if (!payable(msg.sender).send(amount)) {
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
            if (block.timestamp < auctionEndTime)
                revert AuctionNotYetEnded();
            if (ended)
                revert AuctionEndAlreadyCalled();

            // 2. Effects
            ended = true;
            emit AuctionEnded(highestBidder, highestBid);

            // 3. Interaction
            beneficiary.transfer(highestBid);
        }
    }

Blind Auction
=============

The previous open auction is extended to a blind auction in the following. The
advantage of a blind auction is that there is no time pressure towards the end
of the bidding period. Creating a blind auction on a transparent computing
platform might sound like a contradiction, but cryptography comes to the
rescue.

During the **bidding period**, a bidder does not actually send their bid, but
only a hashed version of it.  Since it is currently considered practically
impossible to find two (sufficiently long) values whose hash values are equal,
the bidder commits to the bid by that.  After the end of the bidding period,
the bidders have to reveal their bids: They send their values unencrypted and
the contract checks that the hash value is the same as the one provided during
the bidding period.

Another challenge is how to make the auction **binding and blind** at the same
time: The only way to prevent the bidder from just not sending the money after
they won the auction is to make them send it together with the bid. Since value
transfers cannot be blinded in Ethereum, anyone can see the value.

The following contract solves this problem by accepting any value that is
larger than the highest bid. Since this can of course only be checked during
the reveal phase, some bids might be **invalid**, and this is on purpose (it
even provides an explicit flag to place invalid bids with high value
transfers): Bidders can confuse competition by placing several high or low
invalid bids.

Open in `Remix <http://remix.ethereum.org/?code=Ly8gU1BEWC1MaWNlbnNlLUlkZW50aWZpZXI6IEdQTC0zLjANCnByYWdtYSBzb2xpZGl0eSBeMC44LjQ7DQpjb250cmFjdCBCbGluZEF1Y3Rpb24gew0KICAgIHN0cnVjdCBCaWQgew0KICAgICAgICBieXRlczMyIGJsaW5kZWRCaWQ7DQogICAgICAgIHVpbnQgZGVwb3NpdDsNCiAgICB9DQoNCiAgICBhZGRyZXNzIHBheWFibGUgcHVibGljIGJlbmVmaWNpYXJ5Ow0KICAgIHVpbnQgcHVibGljIGJpZGRpbmdFbmQ7DQogICAgdWludCBwdWJsaWMgcmV2ZWFsRW5kOw0KICAgIGJvb2wgcHVibGljIGVuZGVkOw0KDQogICAgbWFwcGluZyhhZGRyZXNzID0+IEJpZFtdKSBwdWJsaWMgYmlkczsNCg0KICAgIGFkZHJlc3MgcHVibGljIGhpZ2hlc3RCaWRkZXI7DQogICAgdWludCBwdWJsaWMgaGlnaGVzdEJpZDsNCg0KICAgIC8vIEFsbG93ZWQgd2l0aGRyYXdhbHMgb2YgcHJldmlvdXMgYmlkcw0KICAgIG1hcHBpbmcoYWRkcmVzcyA9PiB1aW50KSBwZW5kaW5nUmV0dXJuczsNCg0KICAgIGV2ZW50IEF1Y3Rpb25FbmRlZChhZGRyZXNzIHdpbm5lciwgdWludCBoaWdoZXN0QmlkKTsNCg0KICAgIC8vIEVycm9ycyB0aGF0IGRlc2NyaWJlIGZhaWx1cmVzLg0KDQogICAgLy8vIFRoZSBmdW5jdGlvbiBoYXMgYmVlbiBjYWxsZWQgdG9vIGVhcmx5Lg0KICAgIC8vLyBUcnkgYWdhaW4gYXQgYHRpbWVgLg0KICAgIGVycm9yIFRvb0Vhcmx5KHVpbnQgdGltZSk7DQogICAgLy8vIFRoZSBmdW5jdGlvbiBoYXMgYmVlbiBjYWxsZWQgdG9vIGxhdGUuDQogICAgLy8vIEl0IGNhbm5vdCBiZSBjYWxsZWQgYWZ0ZXIgYHRpbWVgLg0KICAgIGVycm9yIFRvb0xhdGUodWludCB0aW1lKTsNCiAgICAvLy8gVGhlIGZ1bmN0aW9uIGF1Y3Rpb25FbmQgaGFzIGFscmVhZHkgYmVlbiBjYWxsZWQuDQogICAgZXJyb3IgQXVjdGlvbkVuZEFscmVhZHlDYWxsZWQoKTsNCg0KICAgIC8vIE1vZGlmaWVycyBhcmUgYSBjb252ZW5pZW50IHdheSB0byB2YWxpZGF0ZSBpbnB1dHMgdG8NCiAgICAvLyBmdW5jdGlvbnMuIGBvbmx5QmVmb3JlYCBpcyBhcHBsaWVkIHRvIGBiaWRgIGJlbG93Og0KICAgIC8vIFRoZSBuZXcgZnVuY3Rpb24gYm9keSBpcyB0aGUgbW9kaWZpZXIncyBib2R5IHdoZXJlDQogICAgLy8gYF9gIGlzIHJlcGxhY2VkIGJ5IHRoZSBvbGQgZnVuY3Rpb24gYm9keS4NCiAgICBtb2RpZmllciBvbmx5QmVmb3JlKHVpbnQgX3RpbWUpIHsNCiAgICAgICAgaWYgKGJsb2NrLnRpbWVzdGFtcCA+PSBfdGltZSkgcmV2ZXJ0IFRvb0xhdGUoX3RpbWUpOw0KICAgICAgICBfOw0KICAgIH0NCiAgICBtb2RpZmllciBvbmx5QWZ0ZXIodWludCBfdGltZSkgew0KICAgICAgICBpZiAoYmxvY2sudGltZXN0YW1wIDw9IF90aW1lKSByZXZlcnQgVG9vRWFybHkoX3RpbWUpOw0KICAgICAgICBfOw0KICAgIH0NCg0KICAgIGNvbnN0cnVjdG9yKA0KICAgICAgICB1aW50IF9iaWRkaW5nVGltZSwNCiAgICAgICAgdWludCBfcmV2ZWFsVGltZSwNCiAgICAgICAgYWRkcmVzcyBwYXlhYmxlIF9iZW5lZmljaWFyeQ0KICAgICkgew0KICAgICAgICBiZW5lZmljaWFyeSA9IF9iZW5lZmljaWFyeTsNCiAgICAgICAgYmlkZGluZ0VuZCA9IGJsb2NrLnRpbWVzdGFtcCArIF9iaWRkaW5nVGltZTsNCiAgICAgICAgcmV2ZWFsRW5kID0gYmlkZGluZ0VuZCArIF9yZXZlYWxUaW1lOw0KICAgIH0NCg0KICAgIC8vLyBQbGFjZSBhIGJsaW5kZWQgYmlkIHdpdGggYF9ibGluZGVkQmlkYCA9DQogICAgLy8vIGtlY2NhazI1NihhYmkuZW5jb2RlUGFja2VkKHZhbHVlLCBmYWtlLCBzZWNyZXQpKS4NCiAgICAvLy8gVGhlIHNlbnQgZXRoZXIgaXMgb25seSByZWZ1bmRlZCBpZiB0aGUgYmlkIGlzIGNvcnJlY3RseQ0KICAgIC8vLyByZXZlYWxlZCBpbiB0aGUgcmV2ZWFsaW5nIHBoYXNlLiBUaGUgYmlkIGlzIHZhbGlkIGlmIHRoZQ0KICAgIC8vLyBldGhlciBzZW50IHRvZ2V0aGVyIHdpdGggdGhlIGJpZCBpcyBhdCBsZWFzdCAidmFsdWUiIGFuZA0KICAgIC8vLyAiZmFrZSIgaXMgbm90IHRydWUuIFNldHRpbmcgImZha2UiIHRvIHRydWUgYW5kIHNlbmRpbmcNCiAgICAvLy8gbm90IHRoZSBleGFjdCBhbW91bnQgYXJlIHdheXMgdG8gaGlkZSB0aGUgcmVhbCBiaWQgYnV0DQogICAgLy8vIHN0aWxsIG1ha2UgdGhlIHJlcXVpcmVkIGRlcG9zaXQuIFRoZSBzYW1lIGFkZHJlc3MgY2FuDQogICAgLy8vIHBsYWNlIG11bHRpcGxlIGJpZHMuDQogICAgZnVuY3Rpb24gYmlkKGJ5dGVzMzIgX2JsaW5kZWRCaWQpDQogICAgICAgIHB1YmxpYw0KICAgICAgICBwYXlhYmxlDQogICAgICAgIG9ubHlCZWZvcmUoYmlkZGluZ0VuZCkNCiAgICB7DQogICAgICAgIGJpZHNbbXNnLnNlbmRlcl0ucHVzaChCaWQoew0KICAgICAgICAgICAgYmxpbmRlZEJpZDogX2JsaW5kZWRCaWQsDQogICAgICAgICAgICBkZXBvc2l0OiBtc2cudmFsdWUNCiAgICAgICAgfSkpOw0KICAgIH0NCg0KICAgIC8vLyBSZXZlYWwgeW91ciBibGluZGVkIGJpZHMuIFlvdSB3aWxsIGdldCBhIHJlZnVuZCBmb3IgYWxsDQogICAgLy8vIGNvcnJlY3RseSBibGluZGVkIGludmFsaWQgYmlkcyBhbmQgZm9yIGFsbCBiaWRzIGV4Y2VwdCBmb3INCiAgICAvLy8gdGhlIHRvdGFsbHkgaGlnaGVzdC4NCiAgICBmdW5jdGlvbiByZXZlYWwoDQogICAgICAgIHVpbnRbXSBtZW1vcnkgX3ZhbHVlcywNCiAgICAgICAgYm9vbFtdIG1lbW9yeSBfZmFrZSwNCiAgICAgICAgYnl0ZXMzMltdIG1lbW9yeSBfc2VjcmV0DQogICAgKQ0KICAgICAgICBwdWJsaWMNCiAgICAgICAgb25seUFmdGVyKGJpZGRpbmdFbmQpDQogICAgICAgIG9ubHlCZWZvcmUocmV2ZWFsRW5kKQ0KICAgIHsNCiAgICAgICAgdWludCBsZW5ndGggPSBiaWRzW21zZy5zZW5kZXJdLmxlbmd0aDsNCiAgICAgICAgcmVxdWlyZShfdmFsdWVzLmxlbmd0aCA9PSBsZW5ndGgpOw0KICAgICAgICByZXF1aXJlKF9mYWtlLmxlbmd0aCA9PSBsZW5ndGgpOw0KICAgICAgICByZXF1aXJlKF9zZWNyZXQubGVuZ3RoID09IGxlbmd0aCk7DQoNCiAgICAgICAgdWludCByZWZ1bmQ7DQogICAgICAgIGZvciAodWludCBpID0gMDsgaSA8IGxlbmd0aDsgaSsrKSB7DQogICAgICAgICAgICBCaWQgc3RvcmFnZSBiaWRUb0NoZWNrID0gYmlkc1ttc2cuc2VuZGVyXVtpXTsNCiAgICAgICAgICAgICh1aW50IHZhbHVlLCBib29sIGZha2UsIGJ5dGVzMzIgc2VjcmV0KSA9DQogICAgICAgICAgICAgICAgICAgIChfdmFsdWVzW2ldLCBfZmFrZVtpXSwgX3NlY3JldFtpXSk7DQogICAgICAgICAgICBpZiAoYmlkVG9DaGVjay5ibGluZGVkQmlkICE9IGtlY2NhazI1NihhYmkuZW5jb2RlUGFja2VkKHZhbHVlLCBmYWtlLCBzZWNyZXQpKSkgew0KICAgICAgICAgICAgICAgIC8vIEJpZCB3YXMgbm90IGFjdHVhbGx5IHJldmVhbGVkLg0KICAgICAgICAgICAgICAgIC8vIERvIG5vdCByZWZ1bmQgZGVwb3NpdC4NCiAgICAgICAgICAgICAgICBjb250aW51ZTsNCiAgICAgICAgICAgIH0NCiAgICAgICAgICAgIHJlZnVuZCArPSBiaWRUb0NoZWNrLmRlcG9zaXQ7DQogICAgICAgICAgICBpZiAoIWZha2UgJiYgYmlkVG9DaGVjay5kZXBvc2l0ID49IHZhbHVlKSB7DQogICAgICAgICAgICAgICAgaWYgKHBsYWNlQmlkKG1zZy5zZW5kZXIsIHZhbHVlKSkNCiAgICAgICAgICAgICAgICAgICAgcmVmdW5kIC09IHZhbHVlOw0KICAgICAgICAgICAgfQ0KICAgICAgICAgICAgLy8gTWFrZSBpdCBpbXBvc3NpYmxlIGZvciB0aGUgc2VuZGVyIHRvIHJlLWNsYWltDQogICAgICAgICAgICAvLyB0aGUgc2FtZSBkZXBvc2l0Lg0KICAgICAgICAgICAgYmlkVG9DaGVjay5ibGluZGVkQmlkID0gYnl0ZXMzMigwKTsNCiAgICAgICAgfQ0KICAgICAgICBwYXlhYmxlKG1zZy5zZW5kZXIpLnRyYW5zZmVyKHJlZnVuZCk7DQogICAgfQ0KDQogICAgLy8vIFdpdGhkcmF3IGEgYmlkIHRoYXQgd2FzIG92ZXJiaWQuDQogICAgZnVuY3Rpb24gd2l0aGRyYXcoKSBwdWJsaWMgew0KICAgICAgICB1aW50IGFtb3VudCA9IHBlbmRpbmdSZXR1cm5zW21zZy5zZW5kZXJdOw0KICAgICAgICBpZiAoYW1vdW50ID4gMCkgew0KICAgICAgICAgICAgLy8gSXQgaXMgaW1wb3J0YW50IHRvIHNldCB0aGlzIHRvIHplcm8gYmVjYXVzZSB0aGUgcmVjaXBpZW50DQogICAgICAgICAgICAvLyBjYW4gY2FsbCB0aGlzIGZ1bmN0aW9uIGFnYWluIGFzIHBhcnQgb2YgdGhlIHJlY2VpdmluZyBjYWxsDQogICAgICAgICAgICAvLyBiZWZvcmUgYHRyYW5zZmVyYCByZXR1cm5zIChzZWUgdGhlIHJlbWFyayBhYm92ZSBhYm91dA0KICAgICAgICAgICAgLy8gY29uZGl0aW9ucyAtPiBlZmZlY3RzIC0+IGludGVyYWN0aW9uKS4NCiAgICAgICAgICAgIHBlbmRpbmdSZXR1cm5zW21zZy5zZW5kZXJdID0gMDsNCg0KICAgICAgICAgICAgcGF5YWJsZShtc2cuc2VuZGVyKS50cmFuc2ZlcihhbW91bnQpOw0KICAgICAgICB9DQogICAgfQ0KDQogICAgLy8vIEVuZCB0aGUgYXVjdGlvbiBhbmQgc2VuZCB0aGUgaGlnaGVzdCBiaWQNCiAgICAvLy8gdG8gdGhlIGJlbmVmaWNpYXJ5Lg0KICAgIGZ1bmN0aW9uIGF1Y3Rpb25FbmQoKQ0KICAgICAgICBwdWJsaWMNCiAgICAgICAgb25seUFmdGVyKHJldmVhbEVuZCkNCiAgICB7DQogICAgICAgIGlmIChlbmRlZCkgcmV2ZXJ0IEF1Y3Rpb25FbmRBbHJlYWR5Q2FsbGVkKCk7DQogICAgICAgIGVtaXQgQXVjdGlvbkVuZGVkKGhpZ2hlc3RCaWRkZXIsIGhpZ2hlc3RCaWQpOw0KICAgICAgICBlbmRlZCA9IHRydWU7DQogICAgICAgIGJlbmVmaWNpYXJ5LnRyYW5zZmVyKGhpZ2hlc3RCaWQpOw0KICAgIH0NCg0KICAgIC8vIFRoaXMgaXMgYW4gImludGVybmFsIiBmdW5jdGlvbiB3aGljaCBtZWFucyB0aGF0IGl0DQogICAgLy8gY2FuIG9ubHkgYmUgY2FsbGVkIGZyb20gdGhlIGNvbnRyYWN0IGl0c2VsZiAob3IgZnJvbQ0KICAgIC8vIGRlcml2ZWQgY29udHJhY3RzKS4NCiAgICBmdW5jdGlvbiBwbGFjZUJpZChhZGRyZXNzIGJpZGRlciwgdWludCB2YWx1ZSkgaW50ZXJuYWwNCiAgICAgICAgICAgIHJldHVybnMgKGJvb2wgc3VjY2VzcykNCiAgICB7DQogICAgICAgIGlmICh2YWx1ZSA8PSBoaWdoZXN0QmlkKSB7DQogICAgICAgICAgICByZXR1cm4gZmFsc2U7DQogICAgICAgIH0NCiAgICAgICAgaWYgKGhpZ2hlc3RCaWRkZXIgIT0gYWRkcmVzcygwKSkgew0KICAgICAgICAgICAgLy8gUmVmdW5kIHRoZSBwcmV2aW91c2x5IGhpZ2hlc3QgYmlkZGVyLg0KICAgICAgICAgICAgcGVuZGluZ1JldHVybnNbaGlnaGVzdEJpZGRlcl0gKz0gaGlnaGVzdEJpZDsNCiAgICAgICAgfQ0KICAgICAgICBoaWdoZXN0QmlkID0gdmFsdWU7DQogICAgICAgIGhpZ2hlc3RCaWRkZXIgPSBiaWRkZXI7DQogICAgICAgIHJldHVybiB0cnVlOw0KICAgIH0NCn0>`_

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;
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

        // Allowed withdrawals of previous bids
        mapping(address => uint) pendingReturns;

        event AuctionEnded(address winner, uint highestBid);

        // Errors that describe failures.

        /// The function has been called too early.
        /// Try again at `time`.
        error TooEarly(uint time);
        /// The function has been called too late.
        /// It cannot be called after `time`.
        error TooLate(uint time);
        /// The function auctionEnd has already been called.
        error AuctionEndAlreadyCalled();

        // Modifiers are a convenient way to validate inputs to
        // functions. `onlyBefore` is applied to `bid` below:
        // The new function body is the modifier's body where
        // `_` is replaced by the old function body.
        modifier onlyBefore(uint _time) {
            if (block.timestamp >= _time) revert TooLate(_time);
            _;
        }
        modifier onlyAfter(uint _time) {
            if (block.timestamp <= _time) revert TooEarly(_time);
            _;
        }

        constructor(
            uint _biddingTime,
            uint _revealTime,
            address payable _beneficiary
        ) {
            beneficiary = _beneficiary;
            biddingEnd = block.timestamp + _biddingTime;
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
                    // Bid was not actually revealed.
                    // Do not refund deposit.
                    continue;
                }
                refund += bidToCheck.deposit;
                if (!fake && bidToCheck.deposit >= value) {
                    if (placeBid(msg.sender, value))
                        refund -= value;
                }
                // Make it impossible for the sender to re-claim
                // the same deposit.
                bidToCheck.blindedBid = bytes32(0);
            }
            payable(msg.sender).transfer(refund);
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

                payable(msg.sender).transfer(amount);
            }
        }

        /// End the auction and send the highest bid
        /// to the beneficiary.
        function auctionEnd()
            public
            onlyAfter(revealEnd)
        {
            if (ended) revert AuctionEndAlreadyCalled();
            emit AuctionEnded(highestBidder, highestBid);
            ended = true;
            beneficiary.transfer(highestBid);
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
    }
