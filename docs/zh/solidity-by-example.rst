#######
Solidity 编程实例
#######

*******
Voting 投票
********

接下来的合约非常复杂，但展示了很多Solidity的特性。它实现了一个投票合约。当然，电子选举的主要问题是如何赋予投票权给准确的人，并防止操纵。我们不能解决所有的问题，但至少我们会展示如何委托投票可以同时做到投票统计是自动和完全透明。

思路是为每张选票创建一个合约，每个投票选项提供一个短名称。合约创建者作为会长将会给每个投票参与人各自的地址投票权。

地址后面的人们可以选择自己投票或者委托信任的代表人替他们投票。在投票结束后，winningProposal()将会返回获得票数最多的提案。

::

    pragma solidity ^0.4.11;

    /// @title Voting with delegation.
    /// @title 授权投票
    contract Ballot
    {
        // 这里声明了复杂类型
        // 将会在被后面的参数使用
        // 代表一个独立的投票人。
        struct Voter
        {
            uint weight; // 累积的权重。
            bool voted;  // 如果为真，则表示该投票人已经投票。
            address delegate; // 委托的投票代表
            uint vote;   // 投票选择的提案索引号
        }

        // 这是一个独立提案的类型
        struct Proposal
        {
            bytes32 name;   // 短名称（32字节）
            uint voteCount; // 累计获得的票数
        }
        address public chairperson;
        //这里声明一个状态变量，保存每个独立地址的`Voter` 结构
        mapping(address => Voter) public voters;
        //一个存储`Proposal`结构的动态数组
        Proposal[] public proposals;

        // 创建一个新的投票用于选出一个提案名`proposalNames`.
        function Ballot(bytes32[] proposalNames)
        {
            chairperson = msg.sender;
            voters[chairperson].weight = 1;
                    
            //对提供的每一个提案名称，创建一个新的提案
            //对象添加到数组末尾
            for (uint i = 0; i < proposalNames.length; i++)
                //`Proposal({...})` 创建了一个临时的提案对象，
                //`proposal.push(...)`添加到了提案数组`proposals`末尾。
                proposals.push(Proposal({
                    name: proposalNames[i],
                    voteCount: 0
                }));
        }

        //给投票人`voter`参加投票的投票权，
        //只能由投票主持人`chairperson`调用。
        function giveRightToVote(address voter)
        {
            if (msg.sender != chairperson || voters[voter].voted)
                //`throw`会终止和撤销所有的状态和以太改变。
            //如果函数调用无效，这通常是一个好的选择。
            //但是需要注意，这会消耗提供的所有gas。
                throw;
            voters[voter].weight = 1;
        }

        // 委托你的投票权到一个投票代表 `to`。
        function delegate(address to)
        {
            // 指定引用
            Voter sender = voters[msg.sender];
            if (sender.voted)
                throw;
                    
            //当投票代表`to`也委托给别人时，寻找到最终的投票代表
            while (voters[to].delegate != address(0) &&
                voters[to].delegate != msg.sender)
                to = voters[to].delegate;
            // 当最终投票代表等于调用者，是不被允许的。
            if (to == msg.sender)
                throw;
            //因为`sender`是一个引用，
            //这里实际修改了`voters[msg.sender].voted`
            sender.voted = true;
            sender.delegate = to;
            Voter delegate = voters[to];
            if (delegate.voted)
                //如果委托的投票代表已经投票了，直接修改票数
                proposals[delegate.vote].voteCount += sender.weight;
            else
                //如果投票代表还没有投票，则修改其投票权重。
                delegate.weight += sender.weight;
        }

        ///投出你的选票（包括委托给你的选票）
        ///给 `proposals[proposal].name`。
        function vote(uint proposal)
        {
            Voter sender = voters[msg.sender];
            if (sender.voted) throw;
            sender.voted = true;
            sender.vote = proposal;
            //如果`proposal`索引超出了给定的提案数组范围
            //将会自动抛出异常，并撤销所有的改变。
            proposals[proposal].voteCount += sender.weight;
        }

        ///@dev 根据当前所有的投票计算出当前的胜出提案
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


可能的改进
=============

现在，指派投票权到所有的投票参加者需要许多的交易。你能想出更好的方法么？

*****
盲拍
*****

这一节，我们将展示在以太上创建一个完整的盲拍合约是多么简单。我们从一个所有人都能看到出价的公开拍卖开始，接着扩展合约成为一个在拍卖结束以前不能看到实际出价的盲拍。

## 简单的公开拍卖

通常简单的公开拍卖合约，是每个人可以在拍卖期间发送他们的竞拍出价。为了实现绑定竞拍人的到他们的拍卖，竞拍包括发送金额/ether。如果产生了新的最高竞拍价，前一个最高价竞拍人将会拿回他的钱。在竞拍阶段结束后，受益人人需要手动调用合约收取他的钱 — — 合约不会激活自己。

::

    contract SimpleAuction {
        // 拍卖的参数。
        // 时间要么为unix绝对时间戳（自1970-01-01以来的秒数），
        // 或者是以秒为单位的出块时间
        address public beneficiary;
        uint public auctionStart;
        uint public biddingTime;

        //当前的拍卖状态
        address public highestBidder;
        uint public highestBid;

        //在结束时设置为true来拒绝任何改变
        bool ended;

        //当改变时将会触发的Event
        event HighestBidIncreased(address bidder, uint amount);
        event AuctionEnded(address winner, uint amount);

        //下面是一个叫做natspec的特殊注释，
        //由3个连续的斜杠标记，当询问用户确认交易事务时将显示。

        ///创建一个简单的合约使用`_biddingTime`表示的竞拍时间，
        /// 地址`_beneficiary`.代表实际的拍卖者
        function SimpleAuction(uint _biddingTime,
                            address _beneficiary) {
            beneficiary = _beneficiary;
            auctionStart = now;
            biddingTime = _biddingTime;
        }

        ///对拍卖的竞拍保证金会随着交易事务一起发送，
        ///只有在竞拍失败的时候才会退回
        function bid() {

            //不需要任何参数，所有的信息已经是交易事务的一部分
            if (now > auctionStart + biddingTime)
                //当竞拍结束时撤销此调用
                throw;
            if (msg.value <= highestBid)
                //如果出价不是最高的，发回竞拍保证金。
                throw;
            if (highestBidder != 0)
                highestBidder.send(highestBid);
            highestBidder = msg.sender;
            highestBid = msg.value;
            HighestBidIncreased(msg.sender, msg.value);
        }

        ///拍卖结束后发送最高的竞价到拍卖人
        function auctionEnd() {
            if (now <= auctionStart + biddingTime)
                throw; 
            //拍卖还没有结束
            if (ended)
                throw; 
            //这个收款函数已经被调用了
            AuctionEnded(highestBidder, highestBid);
            //发送合约拥有所有的钱，因为有一些保证金可能退回失败了。

            beneficiary.send(this.balance);
            ended = true;
        }

        function () {
            //这个函数将会在发送到合约的交易事务包含无效数据
            //或无数据的时执行，这里撤销所有的发送，
            //所以没有人会在使用合约时因为意外而丢钱。
            throw;
        }
    }


Blind Auction 盲拍
==================


接下来扩展前面的公开拍卖成为一个盲拍。盲拍的特点是拍卖结束以前没有时间压力。在一个透明的计算平台上创建盲拍系统听起来可能有些矛盾，但是加密算法能让你脱离困境。 

在拍卖阶段, 竞拍人不需要发送实际的出价，仅仅只需要发送一个它的散列值。因为目前几乎不可能找到两个值（足够长）的散列值相等，竞拍者提交他们的出价散列值。在拍卖结束后，竞拍人重新发送未加密的竞拍出价，合约将检查其散列值是否和拍卖阶段发送的一样。 另一个挑战是如何让拍卖同时实现绑定和致盲 ：防止竞拍人竞拍成功后不付钱的唯一的办法是，在竞拍出价的同时发送保证金。但是在Ethereum上发送保证金是无法致盲，所有人都能看到保证金。下面的合约通过接受任何尽量大的出价来解决这个问题。当然这可以在最后的揭拍阶段进行复核，一些竞拍出价可能是无效的，这样做的目的是（它提供一个显式的标志指出是无效的竞拍，同时包含高额保证金）：竞拍人可以通过放置几个无效的高价和低价竞拍来混淆竞争对手。

::

    contract BlindAuction
    {
        struct Bid
        {
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

        event AuctionEnded(address winner, uint highestBid);

        ///修饰器（Modifier）是一个简便的途径用来验证函数输入的有效性。
        ///`onlyBefore` 应用于下面的 `bid`函数，其旧的函数体替换修饰器主体中 `_`后就是其新的函数体
        modifier onlyBefore(uint _time) { if (now >= _time) throw; _ }
        modifier onlyAfter(uint _time) { if (now <= _time) throw; _ }

        function BlindAuction(uint _biddingTime,
                                uint _revealTime,
                                address _beneficiary)
        {
            beneficiary = _beneficiary;
            auctionStart = now;
            biddingEnd = now + _biddingTime;
            revealEnd = biddingEnd + _revealTime;
        }

        ///放置一个盲拍出价使用`_blindedBid`=sha3(value,fake,secret).
        ///仅仅在竞拍结束正常揭拍后退还发送的以太。当随同发送的以太至少
        ///等于 "value"指定的保证金并且 "fake"不为true的时候才是有效的竞拍
        ///出价。设置 "fake"为true或发送不合适的金额将会掩没真正的竞拍出
        ///价，但是仍然需要抵押保证金。同一个地址可以放置多个竞拍。
        function bid(bytes32 _blindedBid)
            onlyBefore(biddingEnd)
        {
            bids[msg.sender].push(Bid({
                blindedBid: _blindedBid,
                deposit: msg.value
            }));
        }

        ///揭开你的盲拍竞价。你将会拿回除了最高出价外的所有竞拍保证金
        ///以及正常的无效盲拍保证金。
        function reveal(uint[] _values, bool[] _fake,
                        bytes32[] _secret)
            onlyAfter(biddingEnd)
            onlyBefore(revealEnd)
        {
            uint length = bids[msg.sender].length;
            if (_values.length != length || _fake.length != length ||
                        _secret.length != length)
                throw;
            uint refund;
            for (uint i = 0; i < length; i++)
            {
                var bid = bids[msg.sender][i];
                var (value, fake, secret) =
                        (_values[i], _fake[i], _secret[i]);
                if (bid.blindedBid != sha3(value, fake, secret))
                    //出价未被正常揭拍，不能取回保证金。
                    continue;
                refund += bid.deposit;
                if (!fake && bid.deposit >= value)
                    if (placeBid(msg.sender, value))
                        refund -= value;
                //保证发送者绝不可能重复取回保证金
                bid.blindedBid = 0;
            }
            msg.sender.send(refund);
        }

        //这是一个内部 (internal)函数，
        //意味着仅仅只有合约（或者从其继承的合约）可以调用
        function placeBid(address bidder, uint value) internal
                returns (bool success)
        {
            if (value <= highestBid)
                return false;
            if (highestBidder != 0)
                //退还前一个最高竞拍出价
                highestBidder.send(highestBid);
            highestBid = value;
            highestBidder = bidder;
            return true;
        }

        ///竞拍结束后发送最高出价到竞拍人
        function auctionEnd()
            onlyAfter(revealEnd)
        {
            if (ended) throw;
            AuctionEnded(highestBidder, highestBid);
            //发送合约拥有所有的钱，因为有一些保证金退回可能失败了。
            beneficiary.send(this.balance);
            ended = true;
        }

        function () { throw; }
    }

**********************************
Safe Remote Purchase 安全的远程购物
**********************************

::

    contract Purchase
    {
        uint public value;
        address public seller;
        address public buyer;
        enum State { Created, Locked, Inactive }
        State public state;
        function Purchase()
        {
            seller = msg.sender;
            value = msg.value / 2;
            if (2 * value != msg.value) throw;
        }
        modifier require(bool _condition)
        {
            if (!_condition) throw;
            _
        }
        modifier onlyBuyer()
        {
            if (msg.sender != buyer) throw;
            _
        }
        modifier onlySeller()
        {
            if (msg.sender != seller) throw;
            _
        }
        modifier inState(State _state)
        {
            if (state != _state) throw;
            _
        }
        event aborted();
        event purchaseConfirmed();
        event itemReceived();

        ///终止购物并收回以太。仅仅可以在合约未锁定时被卖家调用。
        function abort()
            onlySeller
            inState(State.Created)
        {
            aborted();
            seller.send(this.balance);
            state = State.Inactive;
        }

        ///买家确认购买。交易包含两倍价值的（`2 * value`）以太。
        ///这些以太会一直锁定到收货确认(confirmReceived)被调用。
        function confirmPurchase()
            inState(State.Created)
            require(msg.value == 2 * value)
        {
            purchaseConfirmed();
            buyer = msg.sender;
            state = State.Locked;
        }

        ///确认你（买家）收到了货物，这将释放锁定的以太。
        function confirmReceived()
            onlyBuyer
            inState(State.Locked)
        {
            itemReceived();
            buyer.send(value);//我们有意忽略了返回值。
            seller.send(this.balance);
            state = State.Inactive;
        }
        function() { throw; }
    }


****************
小额支付通道
****************

待补
