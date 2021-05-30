.. index:: purchase, remote purchase, escrow

********************
Safe Remote Purchase
********************

Purchasing goods remotely currently requires multiple parties that need to trust each other.
The simplest configuration involves a seller and a buyer. The buyer would like to receive
an item from the seller and the seller would like to get money (or an equivalent)
in return. The problematic part is the shipment here: There is no way to determine for
sure that the item arrived at the buyer.

There are multiple ways to solve this problem, but all fall short in one or the other way.
In the following example, both parties have to put twice the value of the item into the
contract as escrow. As soon as this happened, the money will stay locked inside
the contract until the buyer confirms that they received the item. After that,
the buyer is returned the value (half of their deposit) and the seller gets three
times the value (their deposit plus the value). The idea behind
this is that both parties have an incentive to resolve the situation or otherwise
their money is locked forever.

This contract of course does not solve the problem, but gives an overview of how
you can use state machine-like constructs inside a contract.

Open in `Remix <http://remix.ethereum.org/#code=Ly8gU1BEWC1MaWNlbnNlLUlkZW50aWZpZXI6IEdQTC0zLjAKcHJhZ21hIHNvbGlkaXR5IF4wLjguNDsKY29udHJhY3QgUHVyY2hhc2UgewogICAgdWludCBwdWJsaWMgdmFsdWU7CiAgICBhZGRyZXNzIHBheWFibGUgcHVibGljIHNlbGxlcjsKICAgIGFkZHJlc3MgcGF5YWJsZSBwdWJsaWMgYnV5ZXI7CgogICAgZW51bSBTdGF0ZSB7IENyZWF0ZWQsIExvY2tlZCwgUmVsZWFzZSwgSW5hY3RpdmUgfQogICAgLy8gVGhlIHN0YXRlIHZhcmlhYmxlIGhhcyBhIGRlZmF1bHQgdmFsdWUgb2YgdGhlIGZpcnN0IG1lbWJlciwgYFN0YXRlLmNyZWF0ZWRgCiAgICBTdGF0ZSBwdWJsaWMgc3RhdGU7CgogICAgbW9kaWZpZXIgY29uZGl0aW9uKGJvb2wgX2NvbmRpdGlvbikgewogICAgICAgIHJlcXVpcmUoX2NvbmRpdGlvbik7CiAgICAgICAgXzsKICAgIH0KCiAgICAvLy8gT25seSB0aGUgYnV5ZXIgY2FuIGNhbGwgdGhpcyBmdW5jdGlvbi4KICAgIGVycm9yIE9ubHlCdXllcigpOwogICAgLy8vIE9ubHkgdGhlIHNlbGxlciBjYW4gY2FsbCB0aGlzIGZ1bmN0aW9uLgogICAgZXJyb3IgT25seVNlbGxlcigpOwogICAgLy8vIFRoZSBmdW5jdGlvbiBjYW5ub3QgYmUgY2FsbGVkIGF0IHRoZSBjdXJyZW50IHN0YXRlLgogICAgZXJyb3IgSW52YWxpZFN0YXRlKCk7CiAgICAvLy8gVGhlIHByb3ZpZGVkIHZhbHVlIGhhcyB0byBiZSBldmVuLgogICAgZXJyb3IgVmFsdWVOb3RFdmVuKCk7CgogICAgbW9kaWZpZXIgb25seUJ1eWVyKCkgewogICAgICAgIGlmIChtc2cuc2VuZGVyICE9IGJ1eWVyKQogICAgICAgICAgICByZXZlcnQgT25seUJ1eWVyKCk7CiAgICAgICAgXzsKICAgIH0KCiAgICBtb2RpZmllciBvbmx5U2VsbGVyKCkgewogICAgICAgIGlmIChtc2cuc2VuZGVyICE9IHNlbGxlcikKICAgICAgICAgICAgcmV2ZXJ0IE9ubHlTZWxsZXIoKTsKICAgICAgICBfOwogICAgfQoKICAgIG1vZGlmaWVyIGluU3RhdGUoU3RhdGUgX3N0YXRlKSB7CiAgICAgICAgaWYgKHN0YXRlICE9IF9zdGF0ZSkKICAgICAgICAgICAgcmV2ZXJ0IEludmFsaWRTdGF0ZSgpOwogICAgICAgIF87CiAgICB9CgogICAgZXZlbnQgQWJvcnRlZCgpOwogICAgZXZlbnQgUHVyY2hhc2VDb25maXJtZWQoKTsKICAgIGV2ZW50IEl0ZW1SZWNlaXZlZCgpOwogICAgZXZlbnQgU2VsbGVyUmVmdW5kZWQoKTsKCiAgICAvLyBFbnN1cmUgdGhhdCBgbXNnLnZhbHVlYCBpcyBhbiBldmVuIG51bWJlci4KICAgIC8vIERpdmlzaW9uIHdpbGwgdHJ1bmNhdGUgaWYgaXQgaXMgYW4gb2RkIG51bWJlci4KICAgIC8vIENoZWNrIHZpYSBtdWx0aXBsaWNhdGlvbiB0aGF0IGl0IHdhc24ndCBhbiBvZGQgbnVtYmVyLgogICAgY29uc3RydWN0b3IoKSBwYXlhYmxlIHsKICAgICAgICBzZWxsZXIgPSBwYXlhYmxlKG1zZy5zZW5kZXIpOwogICAgICAgIHZhbHVlID0gbXNnLnZhbHVlIC8gMjsKICAgICAgICBpZiAoKDIgKiB2YWx1ZSkgIT0gbXNnLnZhbHVlKQogICAgICAgICAgICByZXZlcnQgVmFsdWVOb3RFdmVuKCk7CiAgICB9CgogICAgLy8vIEFib3J0IHRoZSBwdXJjaGFzZSBhbmQgcmVjbGFpbSB0aGUgZXRoZXIuCiAgICAvLy8gQ2FuIG9ubHkgYmUgY2FsbGVkIGJ5IHRoZSBzZWxsZXIgYmVmb3JlCiAgICAvLy8gdGhlIGNvbnRyYWN0IGlzIGxvY2tlZC4KICAgIGZ1bmN0aW9uIGFib3J0KCkKICAgICAgICBwdWJsaWMKICAgICAgICBvbmx5U2VsbGVyCiAgICAgICAgaW5TdGF0ZShTdGF0ZS5DcmVhdGVkKQogICAgewogICAgICAgIGVtaXQgQWJvcnRlZCgpOwogICAgICAgIHN0YXRlID0gU3RhdGUuSW5hY3RpdmU7CiAgICAgICAgLy8gV2UgdXNlIHRyYW5zZmVyIGhlcmUgZGlyZWN0bHkuIEl0IGlzCiAgICAgICAgLy8gcmVlbnRyYW5jeS1zYWZlLCBiZWNhdXNlIGl0IGlzIHRoZQogICAgICAgIC8vIGxhc3QgY2FsbCBpbiB0aGlzIGZ1bmN0aW9uIGFuZCB3ZQogICAgICAgIC8vIGFscmVhZHkgY2hhbmdlZCB0aGUgc3RhdGUuCiAgICAgICAgc2VsbGVyLnRyYW5zZmVyKGFkZHJlc3ModGhpcykuYmFsYW5jZSk7CiAgICB9CgogICAgLy8vIENvbmZpcm0gdGhlIHB1cmNoYXNlIGFzIGJ1eWVyLgogICAgLy8vIFRyYW5zYWN0aW9uIGhhcyB0byBpbmNsdWRlIGAyICogdmFsdWVgIGV0aGVyLgogICAgLy8vIFRoZSBldGhlciB3aWxsIGJlIGxvY2tlZCB1bnRpbCBjb25maXJtUmVjZWl2ZWQKICAgIC8vLyBpcyBjYWxsZWQuCiAgICBmdW5jdGlvbiBjb25maXJtUHVyY2hhc2UoKQogICAgICAgIHB1YmxpYwogICAgICAgIGluU3RhdGUoU3RhdGUuQ3JlYXRlZCkKICAgICAgICBjb25kaXRpb24obXNnLnZhbHVlID09ICgyICogdmFsdWUpKQogICAgICAgIHBheWFibGUKICAgIHsKICAgICAgICBlbWl0IFB1cmNoYXNlQ29uZmlybWVkKCk7CiAgICAgICAgYnV5ZXIgPSBwYXlhYmxlKG1zZy5zZW5kZXIpOwogICAgICAgIHN0YXRlID0gU3RhdGUuTG9ja2VkOwogICAgfQoKICAgIC8vLyBDb25maXJtIHRoYXQgeW91ICh0aGUgYnV5ZXIpIHJlY2VpdmVkIHRoZSBpdGVtLgogICAgLy8vIFRoaXMgd2lsbCByZWxlYXNlIHRoZSBsb2NrZWQgZXRoZXIuCiAgICBmdW5jdGlvbiBjb25maXJtUmVjZWl2ZWQoKQogICAgICAgIHB1YmxpYwogICAgICAgIG9ubHlCdXllcgogICAgICAgIGluU3RhdGUoU3RhdGUuTG9ja2VkKQogICAgewogICAgICAgIGVtaXQgSXRlbVJlY2VpdmVkKCk7CiAgICAgICAgLy8gSXQgaXMgaW1wb3J0YW50IHRvIGNoYW5nZSB0aGUgc3RhdGUgZmlyc3QgYmVjYXVzZQogICAgICAgIC8vIG90aGVyd2lzZSwgdGhlIGNvbnRyYWN0cyBjYWxsZWQgdXNpbmcgYHNlbmRgIGJlbG93CiAgICAgICAgLy8gY2FuIGNhbGwgaW4gYWdhaW4gaGVyZS4KICAgICAgICBzdGF0ZSA9IFN0YXRlLlJlbGVhc2U7CgogICAgICAgIGJ1eWVyLnRyYW5zZmVyKHZhbHVlKTsKICAgIH0KCiAgICAvLy8gVGhpcyBmdW5jdGlvbiByZWZ1bmRzIHRoZSBzZWxsZXIsIGkuZS4KICAgIC8vLyBwYXlzIGJhY2sgdGhlIGxvY2tlZCBmdW5kcyBvZiB0aGUgc2VsbGVyLgogICAgZnVuY3Rpb24gcmVmdW5kU2VsbGVyKCkKICAgICAgICBwdWJsaWMKICAgICAgICBvbmx5U2VsbGVyCiAgICAgICAgaW5TdGF0ZShTdGF0ZS5SZWxlYXNlKQogICAgewogICAgICAgIGVtaXQgU2VsbGVyUmVmdW5kZWQoKTsKICAgICAgICAvLyBJdCBpcyBpbXBvcnRhbnQgdG8gY2hhbmdlIHRoZSBzdGF0ZSBmaXJzdCBiZWNhdXNlCiAgICAgICAgLy8gb3RoZXJ3aXNlLCB0aGUgY29udHJhY3RzIGNhbGxlZCB1c2luZyBgc2VuZGAgYmVsb3cKICAgICAgICAvLyBjYW4gY2FsbCBpbiBhZ2FpbiBoZXJlLgogICAgICAgIHN0YXRlID0gU3RhdGUuSW5hY3RpdmU7CgogICAgICAgIHNlbGxlci50cmFuc2ZlcigzICogdmFsdWUpOwogICAgfQp9>`_.

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;
    contract Purchase {
        uint public value;
        address payable public seller;
        address payable public buyer;

        enum State { Created, Locked, Release, Inactive }
        // The state variable has a default value of the first member, `State.created`
        State public state;

        modifier condition(bool _condition) {
            require(_condition);
            _;
        }

        /// Only the buyer can call this function.
        error OnlyBuyer();
        /// Only the seller can call this function.
        error OnlySeller();
        /// The function cannot be called at the current state.
        error InvalidState();
        /// The provided value has to be even.
        error ValueNotEven();

        modifier onlyBuyer() {
            if (msg.sender != buyer)
                revert OnlyBuyer();
            _;
        }

        modifier onlySeller() {
            if (msg.sender != seller)
                revert OnlySeller();
            _;
        }

        modifier inState(State _state) {
            if (state != _state)
                revert InvalidState();
            _;
        }

        event Aborted();
        event PurchaseConfirmed();
        event ItemReceived();
        event SellerRefunded();

        // Ensure that `msg.value` is an even number.
        // Division will truncate if it is an odd number.
        // Check via multiplication that it wasn't an odd number.
        constructor() payable {
            seller = payable(msg.sender);
            value = msg.value / 2;
            if ((2 * value) != msg.value)
                revert ValueNotEven();
        }

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
            // We use transfer here directly. It is
            // reentrancy-safe, because it is the
            // last call in this function and we
            // already changed the state.
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
            buyer = payable(msg.sender);
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
            state = State.Release;

            buyer.transfer(value);
        }

        /// This function refunds the seller, i.e.
        /// pays back the locked funds of the seller.
        function refundSeller()
            public
            onlySeller
            inState(State.Release)
        {
            emit SellerRefunded();
            // It is important to change the state first because
            // otherwise, the contracts called using `send` below
            // can call in again here.
            state = State.Inactive;

            seller.transfer(3 * value);
        }
    }
