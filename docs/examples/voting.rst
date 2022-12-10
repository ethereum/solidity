.. index:: voting, ballot

.. _voting:

********
投票合约
********

下面的合约相当复杂，但展示了Solidity的很多特性。
它实现了一个投票合约。当然，
电子投票的主要问题是如何将投票权分配给正确的人以及如何防止人为操纵。
我们不会在这里解决所有的问题，但至少我们会展示如何进行委托投票，
与此同时，使计票是 **自动且完全透明的。**

我们的想法是为每张选票创建一份合约，
为每个选项提供一个简称。
然后，作为合约的创造者——即主席，
将给予每个地址单独的投票权。

地址后面的人可以选择自己投票，或者委托给他们信任的人来投票。

在投票时间结束时， ``winningProposal()`` 将返回拥有最大票数的提案。

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;
    /// @title 委托投票
    contract Ballot {
        // 这声明了一个新的复杂类型，用于稍后变量。
        // 它用来表示一个选民。
        struct Voter {
            uint weight; // 计票的权重
            bool voted;  // 若为真，代表该人已投票
            address delegate; // 被委托人
            uint vote;   // 投票提案的索引
        }

        // 提案的类型
        struct Proposal {
            bytes32 name;   // 简称（最长32个字节）
            uint voteCount; // 得票数
        }

        address public chairperson;
        // 这声明了一个状态变量，为每个可能的地址存储一个 `Voter`。
        mapping(address => Voter) public voters;

        // 一个 `Proposal` 结构类型的动态数组。
        Proposal[] public proposals;

        /// 为 `proposalNames` 中的每个提案，创建一个新的（投票）表决
        constructor(bytes32[] memory proposalNames) {
            chairperson = msg.sender;
            voters[chairperson].weight = 1;

            // 对于提供的每个提案名称，
            // 创建一个新的 Proposal 对象并把它添加到数组的末尾。
            for (uint i = 0; i < proposalNames.length; i++) {
                // `Proposal({...})` 创建一个临时 Proposal 对象
                // `proposals.push(...)` 将其添加到 `proposals` 的末尾
                proposals.push(Proposal({
                    name: proposalNames[i],
                    voteCount: 0
                }));
            }
        }

        // 给予 `voter` 在这张选票上投票的权利。
        // 只有 `chairperson` 可以调用该函数。
        function giveRightToVote(address voter) external {
            // 若 `require` 的第一个参数的计算结果为 `false`，
            // 则终止执行，撤销所有对状态和以太币余额的改动。
            // 在旧版的 EVM 中这曾经会消耗所有 gas，但现在不会了。
            // 使用 `require` 来检查函数是否被正确地调用，通常是个好主意。
            // 您也可以在 `require` 的第二个参数中提供一个对错误情况的解释。
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

        /// 把您的投票委托给投票者 `to`。
        function delegate(address to) external {
            // 指定引用
            Voter storage sender = voters[msg.sender];
            require(sender.weight != 0, "You have no right to vote");
            require(!sender.voted, "You already voted.");

            require(to != msg.sender, "Self-delegation is disallowed.");

            // 委托是可以传递的，只要被委托者 `to` 也设置了委托。
            // 一般来说，这样的循环委托是非常危险的，因为如果传递的链条太长，
            // 可能需要消耗的gas就会超过一个区块中的可用数量。
            // 这种情况下，委托不会被执行。
            // 但在其他情况下，如果形成闭环，则会导致合约完全被 "卡住"。
            while (voters[to].delegate != address(0)) {
                to = voters[to].delegate;

                // 不允许闭环委托
                require(to != msg.sender, "Found loop in delegation.");
            }

            Voter storage delegate_ = voters[to];

            // 投票者不能将投票权委托给不能投票的账户。
            require(delegate_.weight >= 1);

            // 由于 `sender` 是一个引用，
            // 因此这会修改 `voters[msg.sender]`。
            sender.voted = true;
            sender.delegate = to;

            if (delegate_.voted) {
                // 若被委托者已经投过票了，直接增加得票数。
                proposals[delegate_.vote].voteCount += sender.weight;
            } else {
                // 若被委托者还没投票，增加委托者的权重。
                delegate_.weight += sender.weight;
            }
        }

        /// 把您的票(包括委托给您的票)，
        /// 投给提案 `proposals[proposal].name`。
        function vote(uint proposal) external {
            Voter storage sender = voters[msg.sender];
            require(sender.weight != 0, "Has no right to vote");
            require(!sender.voted, "Already voted.");
            sender.voted = true;
            sender.vote = proposal;

            // 如果 `proposal` 超过了数组的范围，
            // 则会自动抛出异常，并恢复所有的改动。
            proposals[proposal].voteCount += sender.weight;
        }

        /// @dev 结合之前所有投票的情况下，计算出获胜的提案。
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

        // 调用 `winningProposal()` 函数以获取提案数组中获胜者的索引，
        // 并以此返回获胜者的名称。
        function winnerName() external view
                returns (bytes32 winnerName_)
        {
            winnerName_ = proposals[winningProposal()].name;
        }
    }


可能的优化
=====================

当前，为了把投票权分配给所有参与者，需要执行很多交易。
您有没有更好的主意？
