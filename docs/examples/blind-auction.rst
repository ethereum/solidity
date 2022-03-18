.. index:: auction;blind, auction;open, blind auction, open auction

****************
盲拍（秘密竞价）
****************

在本节中，我们将展示如何轻松地在以太坊上创建一个盲拍的合约。
我们将从一个公开拍卖开始，每个人都可以看到出价，
然后将此合约扩展到盲拍合约， 在竞标期结束之前无法看到实际出价。

.. _simple_auction:

简单的公开拍卖
===================

下面这个简单的拍卖合约的总体思路是，每个人都可以在竞标期间发送他们的竞标。
竞标已经包括发送资金/以太币，以便将竞标者与他们的竞标绑定。
如果最高出价被提高，之前的最高出价者就会拿回他们的钱。
竞价期结束后，受益人需要手动调用合约，才能收到他们的钱 - 合约不能自己激活接收。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;
    contract SimpleAuction {
        // 拍卖的参数。
        // 时间是 unix 的绝对时间戳（自1970-01-01以来的秒数）
        // 或以秒为单位的时间段。
        address payable public beneficiary;
        uint public auctionEndTime;

        // 拍卖的当前状态。
        address public highestBidder;
        uint public highestBid;

        // 允许取回以前的竞标。
        mapping(address => uint) pendingReturns;

        // 拍卖结束后设为 `true`，将禁止所有的变更
        // 默认初始化为 `false`。
        bool ended;

        // 变化时将会发出的事件。
        event HighestBidIncreased(address bidder, uint amount);
        event AuctionEnded(address winner, uint amount);

        // 描述失败的错误信息。

        // 三斜线的注释是所谓的 natspec 注释。
        // 当用户被要求确认一个交易或显示一个错误时，它们将被显示。

        /// 竞拍已经结束。
        error AuctionAlreadyEnded();
        /// 已经有一个更高的或相等的出价。
        error BidNotHighEnough(uint highestBid);
        /// 竞拍还没有结束。
        error AuctionNotYetEnded();
        /// 函数 auctionEnd 已经被调用。
        error AuctionEndAlreadyCalled();

        /// 以受益者地址 `beneficiaryAddress` 创建一个简单的拍卖，
        /// 拍卖时长为 `_biddingTime`。
        constructor(
            uint biddingTime,
            address payable beneficiaryAddress
        ) {
            beneficiary = beneficiaryAddress;
            auctionEndTime = block.timestamp + biddingTime;
        }

        /// 对拍卖进行出价，具体的出价随交易一起发送。
        /// 如果没有在拍卖中胜出，则返还出价。
        function bid() external payable {
            // 参数不是必要的。因为所有的信息已经包含在了交易中。
            // 关键字 `payable` 是函数能够接收以太币的必要条件。

            // 如果拍卖已结束，撤销函数的调用。
            if (block.timestamp > auctionEndTime)
                revert AuctionAlreadyEnded();

            // 如果出价不高，就把钱送回去
            //（revert语句将恢复这个函数执行中的所有变化，
            // 包括它已经收到钱）。
            if (msg.value <= highestBid)
                revert BidNotHighEnough(highestBid);

            if (highestBid != 0) {
                // 简单地使用 highestBidder.send(highestBid)
                // 返还出价时，是有安全风险的，
                // 因为它可能执行一个不受信任的合约。
                // 让接收方自己取钱总是比较安全的。
                pendingReturns[highestBidder] += highestBid;
            }
            highestBidder = msg.sender;
            highestBid = msg.value;
            emit HighestBidIncreased(msg.sender, msg.value);
        }

        /// 撤回出价过高的竞标。
        function withdraw() external returns (bool) {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // 将其设置为0是很重要的，
                // 因为接收者可以在 `send` 返回之前再次调用这个函数
                // 作为接收调用的一部分。
                pendingReturns[msg.sender] = 0;

                // msg.sender 不属于 `address payable` 类型，
                // 必须使用 `payable(msg.sender)` 明确转换，
                // 以便使用成员函数 `send()`。
                if (!payable(msg.sender).send(amount)) {
                    // 这里不需抛出异常，只需重置未付款
                    pendingReturns[msg.sender] = amount;
                    return false;
                }
            }
            return true;
        }

        /// 结束拍卖，并把最高的出价发送给受益人。
        function auctionEnd() external {
            // 对于可与其他合约交互的函数（意味着它会调用其他函数或发送以太币），
            // 一个好的指导方针是将其结构分为三个阶段：
            // 1. 检查条件
            // 2. 执行动作 (可能会改变条件)
            // 3. 与其他合约交互
            // 如果这些阶段相混合，其他的合约可能会回调当前合约并修改状态，
            // 或者导致某些效果（比如支付以太币）多次生效。
            // 如果合约内调用的函数包含了与外部合约的交互，
            // 则它也会被认为是与外部合约有交互的。

            // 1. 条件
            if (block.timestamp < auctionEndTime)
                revert AuctionNotYetEnded();
            if (ended)
                revert AuctionEndAlreadyCalled();

            // 2. 影响
            ended = true;
            emit AuctionEnded(highestBidder, highestBid);

            // 3. 交互
            beneficiary.transfer(highestBid);
        }
    }

盲拍（秘密竞拍）
================

之前的公开拍卖接下来将被扩展为盲目拍卖。
盲拍的好处是，在竞价期即将结束时没有时间压力。
在一个透明的计算平台上创建一个盲拍可能听起来是一个矛盾，但加密技术可以实现它。

在 **竞标期间**，竞标者实际上并没有发送他们的出价，
而只是发送一个哈希版本的出价。 由于目前几乎不可能找到两个（足够长的）值，
其哈希值是相等的，因此竞标者可通过该方式提交报价。 在竞标结束后，
竞标者必须公开他们的出价：他们发送未加密的值，
合约检查出价的哈希值是否与竞标期间提供的值相同。

另一个挑战是如何使拍卖同时做到 **绑定和秘密** :
唯一能阻止竞标者在赢得拍卖后不付款的方式是，让他们将钱和竞标一起发出。
但由于资金转移在 以太坊 中不能被隐藏，因此任何人都可以看到转移的资金。

下面的合约通过接受任何大于最高出价的值来解决这个问题。
当然，因为这只能在揭示阶段进行检查，有些出价可能是 **无效** 的，
甚至，这是故意的(与高出价一起，它甚至提供了一个明确的标志来标识无效的出价):
竞标者可以通过设置几个或高或低的无效出价来迷惑竞争对手。

.. code-block:: solidity
    :force:

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

        // 允许取回以前的竞标。
        mapping(address => uint) pendingReturns;

        event AuctionEnded(address winner, uint highestBid);

        // 描述失败的错误信息。

        /// 该函数被过早调用。
        /// 在 `time` 时间再试一次。
        error TooEarly(uint time);
        /// 该函数被过晚调用。
        /// 它不能在 `time` 时间之后被调用。
        error TooLate(uint time);
        /// 函数 auctionEnd 已经被调用。
        error AuctionEndAlreadyCalled();

        // 使用 修饰符（modifier） 可以更便捷的校验函数的入参。
        // `onlyBefore` 会被用于后面的 `bid` 函数：
        // 新的函数体是由 modifier 本身的函数体，其中`_`被旧的函数体所取代。
        modifier onlyBefore(uint time) {
            if (block.timestamp >= time) revert TooLate(time);
            _;
        }
        modifier onlyAfter(uint time) {
            if (block.timestamp <= time) revert TooEarly(time);
            _;
        }

        constructor(
            uint biddingTime,
            uint revealTime,
            address payable beneficiaryAddress
        ) {
            beneficiary = beneficiaryAddress;
            biddingEnd = block.timestamp + biddingTime;
            revealEnd = biddingEnd + revealTime;
        }

        /// 可以通过 `_blindedBid` = keccak256(value, fake, secret)
        /// 设置一个盲拍。
        /// 只有在出价披露阶段被正确披露，已发送的以太币才会被退还。
        /// 如果与出价一起发送的以太币至少为 "value" 且 "fake" 不为真，则出价有效。
        /// 将 "fake" 设置为 true ，
        /// 然后发送满足订金金额但又不与出价相同的金额是隐藏实际出价的方法。
        /// 同一个地址可以放置多个出价。
        function bid(bytes32 blindedBid)
            external
            payable
            onlyBefore(biddingEnd)
        {
            bids[msg.sender].push(Bid({
                blindedBid: blindedBid,
                deposit: msg.value
            }));
        }

        /// 披露你的盲拍出价。
        /// 对于所有正确披露的无效出价以及除最高出价以外的所有出价，您都将获得退款。
        function reveal(
            uint[] calldata values,
            bool[] calldata fakes,
            bytes32[] calldata secrets
        )
            external
            onlyAfter(biddingEnd)
            onlyBefore(revealEnd)
        {
            uint length = bids[msg.sender].length;
            require(values.length == length);
            require(fakes.length == length);
            require(secrets.length == length);

            uint refund;
            for (uint i = 0; i < length; i++) {
                Bid storage bidToCheck = bids[msg.sender][i];
                (uint value, bool fake, bytes32 secret) =
                        (values[i], fakes[i], secrets[i]);
                if (bidToCheck.blindedBid != keccak256(abi.encodePacked(value, fake, secret))) {
                    // 出价未能正确披露。
                    // 不返还订金。
                    continue;
                }
                refund += bidToCheck.deposit;
                if (!fake && bidToCheck.deposit >= value) {
                    if (placeBid(msg.sender, value))
                        refund -= value;
                }
                // 使发送者不可能再次认领同一笔订金。
                bidToCheck.blindedBid = bytes32(0);
            }
            payable(msg.sender).transfer(refund);
        }

        /// 撤回出价过高的竞标。
        function withdraw() external {
            uint amount = pendingReturns[msg.sender];
            if (amount > 0) {
                // 这里很重要，首先要设零值。
                // 因为，作为接收调用的一部分，
                // 接收者可以在 `transfer` 返回之前重新调用该函数。
                //（可查看上面关于 条件 -> 影响 -> 交互 的标注）
                pendingReturns[msg.sender] = 0;

                payable(msg.sender).transfer(amount);
            }
        }

        /// 结束拍卖，并把最高的出价发送给受益人。
        function auctionEnd()
            external
            onlyAfter(revealEnd)
        {
            if (ended) revert AuctionEndAlreadyCalled();
            emit AuctionEnded(highestBidder, highestBid);
            ended = true;
            beneficiary.transfer(highestBid);
        }

        // 这是一个 "internal" 函数，
        // 意味着它只能在本合约（或继承合约）内被调用。
        function placeBid(address bidder, uint value) internal
                returns (bool success)
        {
            if (value <= highestBid) {
                return false;
            }
            if (highestBidder != address(0)) {
                // 返还之前的最高出价
                pendingReturns[highestBidder] += highestBid;
            }
            highestBid = value;
            highestBidder = bidder;
            return true;
        }
    }
