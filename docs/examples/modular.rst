.. index:: contract;modular, modular contract

***********
模块化合约
***********

<<<<<<< HEAD
用模块化的方法来构建您的合约，可以帮助减少复杂性，提高可读性，
这将有助于在开发和代码审查中发现错误和漏洞。
如果您单独指定且控制每个模块的行为，您必须考虑的相互作用只是模块之间的相互作用，
而不是合约的其他每个灵活模块函数。
在下面的例子中，合约使用 ``Balances`` :ref:`库 <libraries>` 的 ``move`` 方法
来检查地址之间发送的余额是否符合您的期望。通过这种方式， ``Balances`` 库提供了一个独立的组件，
可以正确地跟踪账户的余额。
很容易验证 ``Balances`` 库永远不会产生负的余额或溢出，
所有余额的总和在合约的有效期内是一个不变的量。
=======
A modular approach to building your contracts helps you reduce the complexity
and improve the readability which will help to identify bugs and vulnerabilities
during development and code review.
If you specify and control the behaviour of each module in isolation, the
interactions you have to consider are only those between the module specifications
and not every other moving part of the contract.
In the example below, the contract uses the ``move`` method
of the ``Balances`` :ref:`library <libraries>` to check that balances sent between
addresses match what you expect. In this way, the ``Balances`` library
provides an isolated component that properly tracks balances of accounts.
It is easy to verify that the ``Balances`` library never produces negative balances or overflows
and the sum of all balances is an invariant across the lifetime of the contract.
>>>>>>> 07a7930e73f57ce6ed1c6f0b8dd9aad99e5c3692

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    library Balances {
        function move(mapping(address => uint256) storage balances, address from, address to, uint amount) internal {
            require(balances[from] >= amount);
            require(balances[to] + amount >= balances[to]);
            balances[from] -= amount;
            balances[to] += amount;
        }
    }

    contract Token {
        mapping(address => uint256) balances;
        using Balances for *;
        mapping(address => mapping (address => uint256)) allowed;

        event Transfer(address from, address to, uint amount);
        event Approval(address owner, address spender, uint amount);

        function transfer(address to, uint amount) external returns (bool success) {
            balances.move(msg.sender, to, amount);
            emit Transfer(msg.sender, to, amount);
            return true;

        }

        function transferFrom(address from, address to, uint amount) external returns (bool success) {
            require(allowed[from][msg.sender] >= amount);
            allowed[from][msg.sender] -= amount;
            balances.move(from, to, amount);
            emit Transfer(from, to, amount);
            return true;
        }

        function approve(address spender, uint tokens) external returns (bool success) {
            require(allowed[msg.sender][spender] == 0, "");
            allowed[msg.sender][spender] = tokens;
            emit Approval(msg.sender, spender, tokens);
            return true;
        }

        function balanceOf(address tokenOwner) external view returns (uint balance) {
            return balances[tokenOwner];
        }
    }
