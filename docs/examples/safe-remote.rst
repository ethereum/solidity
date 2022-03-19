.. index:: purchase, remote purchase, escrow

********************
安全的远程购买
********************

目前，远程购买商品需要多方相互信任。最简单的关系涉及一个卖家和一个买家。
买方希望从卖方那里收到一件物品，卖方希望得到金钱（或等价物）作为回报。
这里面有问题的部分是的运输。没有办法确定物品是否到达买方手中。

有多种方法来解决这个问题，但都有这样或那样的不足之处。
在下面的例子中，双方都要把两倍价值于物品的资金放入合约中作为托管。
只要发生这种情况，钱就会一直锁在合同里面，直到买方确认收到物品。
之后，买方会得到退回的资金（他们押金的一半），卖方得到三倍的资金（他们的押金加上物品的价值）。
这背后的想法是，双方都有动力去解决这个问题，否则他们的钱就会被永远锁定。

这个合约当然不能解决问题，但它概述了如何在合约内使用类似状态机的构造。


.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;
    contract Purchase {
        uint public value;
        address payable public seller;
        address payable public buyer;

        enum State { Created, Locked, Release, Inactive }
        // 状态变量的默认值是第一个成员，`State.created`。
        State public state;

        modifier condition(bool condition_) {
            require(condition_);
            _;
        }

        /// 只有买方可以调用这个函数。
        error OnlyBuyer();
        /// 只有卖方可以调用这个函数。
        error OnlySeller();
        /// 在当前状态下不能调用该函数。
        error InvalidState();
        /// 提供的值必须是偶数。
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

        modifier inState(State state_) {
            if (state != state_)
                revert InvalidState();
            _;
        }

        event Aborted();
        event PurchaseConfirmed();
        event ItemReceived();
        event SellerRefunded();

        // 确保 `msg.value` 是一个偶数。
        // 如果是奇数，除法会截断。
        // 通过乘法检查它不是一个奇数。
        constructor() payable {
            seller = payable(msg.sender);
            value = msg.value / 2;
            if ((2 * value) != msg.value)
                revert ValueNotEven();
        }

        /// 终止购买并收回 ether。
        /// 只能由卖方在合同锁定前能调用。
        function abort()
            external
            onlySeller
            inState(State.Created)
        {
            emit Aborted();
            state = State.Inactive;
            // 我们在这里直接使用 `transfer`。
            // 它可以安全地重入。
            // 因为它是这个函数中的最后一次调用，
            // 而且我们已经改变了状态。
            seller.transfer(address(this).balance);
        }

        /// 买方确认购买。
        /// 交易必须包括 `2 * value` ether。
        /// Ether 将被锁住，直到调用 confirmReceived。
        function confirmPurchase()
            external
            inState(State.Created)
            condition(msg.value == (2 * value))
            payable
        {
            emit PurchaseConfirmed();
            buyer = payable(msg.sender);
            state = State.Locked;
        }

        /// 确认您（买方）已经收到了该物品。
        /// 这将释放锁定的 ether。
        function confirmReceived()
            external
            onlyBuyer
            inState(State.Locked)
        {
            emit ItemReceived();
            // 首先改变状态是很重要的，否则的话，
            // 下面使用 `send` 调用的合约可以在这里再次调用。
            state = State.Release;

            buyer.transfer(value);
        }

        /// 该功能为卖家退款，
        /// 即退还卖家锁定的资金。
        function refundSeller()
            external
            onlySeller
            inState(State.Release)
        {
            emit SellerRefunded();
            // 首先改变状态是很重要的，否则的话，
            // 下面使用 `send` 调用的合约可以在这里再次调用。
            state = State.Inactive;

            seller.transfer(3 * value);
        }
    }
