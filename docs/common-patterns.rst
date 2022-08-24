###############
通用模式
###############

.. index:: withdrawal

.. _withdrawal_pattern:

*************************
从合约中提款
*************************

在某个操作之后发送资金的推荐方式是使用取回（withdrawal）模式。
尽管在某个操作之后，最直接地发送以太币方法是一个 ``transfer`` 调用，
但这并不推荐,因为这会引入一个潜在的安全风险。
您可能需要参考 :ref:`security_considerations` 来获取更多信息。

下面是一个合约中实际提款模式的例子，其目标是向合约发送最多的钱，
以成为 “首富”，其灵感来自于 `King of the Ether <https://www.kingoftheether.com/>`_。

在下面的合约中，如果您不再是最富有的人，您将收到取代您成为“最富有”的人发送到合约的资金。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract WithdrawalContract {
        address public richest;
        uint public mostSent;

        mapping (address => uint) pendingWithdrawals;

        /// 发送的以太数量不高于目前的最高量。
        error NotEnoughEther();

        constructor() payable {
            richest = msg.sender;
            mostSent = msg.value;
        }

        function becomeRichest() public payable {
            if (msg.value <= mostSent) revert NotEnoughEther();
            pendingWithdrawals[richest] += msg.value;
            richest = msg.sender;
            mostSent = msg.value;
        }

        function withdraw() public {
            uint amount = pendingWithdrawals[msg.sender];
            // 记得在发送前将待处理的退款归零，
            // 以防止重入攻击
            pendingWithdrawals[msg.sender] = 0;
            payable(msg.sender).transfer(amount);
        }
    }

下面是一个相反的直接使用发送模式的例子：

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract SendContract {
        address payable public richest;
        uint public mostSent;

        /// 发送的以太数量不高于目前的最高量。
        error NotEnoughEther();

        constructor() payable {
            richest = payable(msg.sender);
            mostSent = msg.value;
        }

        function becomeRichest() public payable {
            if (msg.value <= mostSent) revert NotEnoughEther();
            // 这一行会导致问题（详见下文）
            richest.transfer(msg.value);
            richest = payable(msg.sender);
            mostSent = msg.value;
        }
    }

请注意，在这个例子中，攻击者可以通过使 ``richest`` 成为一个有 receive 或 fallback 函数的合约的地址
而使合约陷入无法使用的状态（例如，通过使用 ``revert()`` 或只是消耗超过转给他们的2300 gas 津贴）。
这样，每当调用 ``transfer`` 向 “中毒” 的合约交付资金时，它就会失败，
因此 ``becomeRichest`` 也会失败，合约会永远被卡住。

相反，如果您使用第一个例子中的 “取回（withdraw）”模式，
那么攻击者只能使他/她自己的“取回”失败，并不会导致整个合约无法运作。

.. index:: access;restricting

******************
限制访问
******************

限制访问是合约的一个常见模式。
请注意，您永远无法限制任何人类或机器阅读您的交易内容或您的合约状态。
您可以通过使用加密来增加一点难度，
但如果您想让您的合约读取这些数据，那么其他人也将可以做到。

您可以限制 **其他合约** 对您的合约状态的读取权限。
这实际上是默认的，除非您声明您的状态变量为 ``public``。

此外，您可以限制谁可以对您的合约的状态进行修改或调用您的合约的功能，
这就是本节的内容。

.. index:: function;modifier

使用 **函数修饰符** 使这些限制变得非常明确。

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract AccessRestriction {
        // 这些将在构造阶段被赋值
        // 其中，`msg.sender` 是
        // 创建这个合约的账户。
        address public owner = msg.sender;
        uint public creationTime = block.timestamp;

        // 现在列出了该合约可能产生的错误，
        // 并在特别注释中作了文字解释。

        /// 调用者未被授权进行此操作。
        error Unauthorized();

        /// 函数调用过早。
        error TooEarly();

        /// 函数调用时没有发送足够的以太。
        error NotEnoughEther();

        // 修饰器可以用来更改
        // 一个函数的函数体。
        // 如果使用这个修饰器，
        // 它会预置一个检查，仅允许
        // 来自特定地址的
        // 函数调用。
        modifier onlyBy(address _account)
        {
            if (msg.sender != _account)
                revert Unauthorized();
            // 不要忘记写 “_;”！
            // 它会被实际使用这个修饰器的
            // 函数体所替代。
            _;
        }

        /// 使 `_newOwner` 成为这个合约的新所有者。
        function changeOwner(address _newOwner)
            public
            onlyBy(owner)
        {
            owner = _newOwner;
        }

        modifier onlyAfter(uint _time) {
            if (block.timestamp < _time)
                revert TooEarly();
            _;
        }

        /// 抹掉所有者信息。
        /// 仅允许在合约创建成功 6 周以后
        /// 的时间被调用。
        function disown()
            public
            onlyBy(owner)
            onlyAfter(creationTime + 6 weeks)
        {
            delete owner;
        }

        // 这个修饰器要求对函数调用
        // 绑定一定的费用。
        // 如果调用方发送了过多的费用，
        // 他/她会得到退款，但需要先执行函数体。
        // 这在 0.4.0 版本以前的 Solidity 中很危险，
        // 因为很可能会跳过 `_;` 之后的代码。
        modifier costs(uint _amount) {
            if (msg.value < _amount)
                revert NotEnoughEther();

            _;
            if (msg.value > _amount)
                payable(msg.sender).transfer(msg.value - _amount);
        }

        function forceOwnerChange(address _newOwner)
            public
            payable
            costs(200 ether)
        {
            owner = _newOwner;
            // 这只是示例条件
            if (uint160(owner) & 0 == 1)
                // 这无法在 0.4.0 版本之前的
                // Solidity 上进行退还。
                return;
            // 退还多付的费用
        }
    }

在下一个例子中，将讨论一种更专业的限制函数调用访问的方式。

.. index:: state machine

*************
状态机
*************

合约通常会像状态机那样运作，这意味着它们有特定的 **阶段**，
使它们有不同的表现或者仅允许特定的不同函数被调用。
一个函数调用通常会结束一个阶段，
并将合约转换到下一个阶段（特别是如果一个合约是以 **交互** 来建模的时候）。
通过达到特定的 **时间** 点来达到某些阶段也是很常见的。

一个典型的例子是盲拍（blind auction）合约，
它起始于“接受盲目出价”，
然后转换到“公示出价”，
最后结束于“确定拍卖结果”。

.. index:: function;modifier

函数修饰器可以用在这种情况下来对状态进行建模，
并确保合约被正常的使用。

示例
=======

在下边的示例中， 修饰器 ``atStage`` 确保了函数仅在特定的阶段才可以被调用。

自动定时过渡是由修饰器 ``timedTransitions`` 处理的，它应该用于所有函数。

.. note::
    **修饰器的顺序非常重要**.
    如果 atStage 和 timedTransitions 要一起使用，
    请确保在 timedTransitions 之后声明 atStage，
    以便新的状态可以 首先被反映到账户中。

最后， 修饰器 ``transitionNext`` 能够用来在函数执行结束时自动转换到下一个阶段。

.. note::
    **修饰器可以被忽略**.
    这只适用于0.4.0版本之前的Solidity：
    由于修饰器是通过简单地替换代码而不是使用函数调用来应用的，
    如果函数本身使用 return，可以跳过 transitionNext 修饰器中的代码。
    如果您想这样做，请确保从这些函数中手动调用 nextStage。
    从0.4.0版本开始，即使函数明确返回，修饰器代码也会运行。

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.4;

    contract StateMachine {
        enum Stages {
            AcceptingBlindedBids,
            RevealBids,
            AnotherStage,
            AreWeDoneYet,
            Finished
        }
        /// 此阶段不能调用该函数。
        error FunctionInvalidAtThisStage();

        // 这是当前阶段。
        Stages public stage = Stages.AcceptingBlindedBids;

        uint public creationTime = block.timestamp;

        modifier atStage(Stages _stage) {
            if (stage != _stage)
                revert FunctionInvalidAtThisStage();
            _;
        }

        function nextStage() internal {
            stage = Stages(uint(stage) + 1);
        }

        // 执行基于时间的阶段转换。
        // 请确保首先声明这个修饰器，
        // 否则新阶段不会被带入账户。
        modifier timedTransitions() {
            if (stage == Stages.AcceptingBlindedBids &&
                        block.timestamp >= creationTime + 10 days)
                nextStage();
            if (stage == Stages.RevealBids &&
                    block.timestamp >= creationTime + 12 days)
                nextStage();
            // 由交易触发的其他阶段转换
            _;
        }

        // 这里的修饰器顺序非常重要！
        function bid()
            public
            payable
            timedTransitions
            atStage(Stages.AcceptingBlindedBids)
        {
            // 我们不会在这里实现实际功能（因为这仅是个代码示例，译者注）
        }

        function reveal()
            public
            timedTransitions
            atStage(Stages.RevealBids)
        {
        }

        // 这个修饰器在函数执行结束之后
        // 使合约进入下一个阶段。
        modifier transitionNext()
        {
            _;
            nextStage();
        }

        function g()
            public
            timedTransitions
            atStage(Stages.AnotherStage)
            transitionNext
        {
        }

        function h()
            public
            timedTransitions
            atStage(Stages.AreWeDoneYet)
            transitionNext
        {
        }

        function i()
            public
            timedTransitions
            atStage(Stages.Finished)
        {
        }
    }
