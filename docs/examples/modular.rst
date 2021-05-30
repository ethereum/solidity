.. index:: contract;modular, modular contract

*****************
Modular Contracts
*****************

A modular approach to building your contracts helps you reduce the complexity
and improve the readability which will help to identify bugs and vulnerabilities
during development and code review.
If you specify and control the behaviour or each module in isolation, the
interactions you have to consider are only those between the module specifications
and not every other moving part of the contract.
In the example below, the contract uses the ``move`` method
of the ``Balances`` :ref:`library <libraries>` to check that balances sent between
addresses match what you expect. In this way, the ``Balances`` library
provides an isolated component that properly tracks balances of accounts.
It is easy to verify that the ``Balances`` library never produces negative balances or overflows
and the sum of all balances is an invariant across the lifetime of the contract.

Open in `Remix <http://remix.ethereum.org/#code=Ly8gU1BEWC1MaWNlbnNlLUlkZW50aWZpZXI6IEdQTC0zLjANCnByYWdtYSBzb2xpZGl0eSA+PTAuNS4wIDwwLjkuMDsNCg0KbGlicmFyeSBCYWxhbmNlcyB7DQogICAgZnVuY3Rpb24gbW92ZShtYXBwaW5nKGFkZHJlc3MgPT4gdWludDI1Nikgc3RvcmFnZSBiYWxhbmNlcywgYWRkcmVzcyBmcm9tLCBhZGRyZXNzIHRvLCB1aW50IGFtb3VudCkgaW50ZXJuYWwgew0KICAgICAgICByZXF1aXJlKGJhbGFuY2VzW2Zyb21dID49IGFtb3VudCk7DQogICAgICAgIHJlcXVpcmUoYmFsYW5jZXNbdG9dICsgYW1vdW50ID49IGJhbGFuY2VzW3RvXSk7DQogICAgICAgIGJhbGFuY2VzW2Zyb21dIC09IGFtb3VudDsNCiAgICAgICAgYmFsYW5jZXNbdG9dICs9IGFtb3VudDsNCiAgICB9DQp9DQoNCmNvbnRyYWN0IFRva2VuIHsNCiAgICBtYXBwaW5nKGFkZHJlc3MgPT4gdWludDI1NikgYmFsYW5jZXM7DQogICAgdXNpbmcgQmFsYW5jZXMgZm9yICo7DQogICAgbWFwcGluZyhhZGRyZXNzID0+IG1hcHBpbmcgKGFkZHJlc3MgPT4gdWludDI1NikpIGFsbG93ZWQ7DQoNCiAgICBldmVudCBUcmFuc2ZlcihhZGRyZXNzIGZyb20sIGFkZHJlc3MgdG8sIHVpbnQgYW1vdW50KTsNCiAgICBldmVudCBBcHByb3ZhbChhZGRyZXNzIG93bmVyLCBhZGRyZXNzIHNwZW5kZXIsIHVpbnQgYW1vdW50KTsNCg0KICAgIGZ1bmN0aW9uIHRyYW5zZmVyKGFkZHJlc3MgdG8sIHVpbnQgYW1vdW50KSBwdWJsaWMgcmV0dXJucyAoYm9vbCBzdWNjZXNzKSB7DQogICAgICAgIGJhbGFuY2VzLm1vdmUobXNnLnNlbmRlciwgdG8sIGFtb3VudCk7DQogICAgICAgIGVtaXQgVHJhbnNmZXIobXNnLnNlbmRlciwgdG8sIGFtb3VudCk7DQogICAgICAgIHJldHVybiB0cnVlOw0KDQogICAgfQ0KDQogICAgZnVuY3Rpb24gdHJhbnNmZXJGcm9tKGFkZHJlc3MgZnJvbSwgYWRkcmVzcyB0bywgdWludCBhbW91bnQpIHB1YmxpYyByZXR1cm5zIChib29sIHN1Y2Nlc3MpIHsNCiAgICAgICAgcmVxdWlyZShhbGxvd2VkW2Zyb21dW21zZy5zZW5kZXJdID49IGFtb3VudCk7DQogICAgICAgIGFsbG93ZWRbZnJvbV1bbXNnLnNlbmRlcl0gLT0gYW1vdW50Ow0KICAgICAgICBiYWxhbmNlcy5tb3ZlKGZyb20sIHRvLCBhbW91bnQpOw0KICAgICAgICBlbWl0IFRyYW5zZmVyKGZyb20sIHRvLCBhbW91bnQpOw0KICAgICAgICByZXR1cm4gdHJ1ZTsNCiAgICB9DQoNCiAgICBmdW5jdGlvbiBhcHByb3ZlKGFkZHJlc3Mgc3BlbmRlciwgdWludCB0b2tlbnMpIHB1YmxpYyByZXR1cm5zIChib29sIHN1Y2Nlc3MpIHsNCiAgICAgICAgcmVxdWlyZShhbGxvd2VkW21zZy5zZW5kZXJdW3NwZW5kZXJdID09IDAsICIiKTsNCiAgICAgICAgYWxsb3dlZFttc2cuc2VuZGVyXVtzcGVuZGVyXSA9IHRva2VuczsNCiAgICAgICAgZW1pdCBBcHByb3ZhbChtc2cuc2VuZGVyLCBzcGVuZGVyLCB0b2tlbnMpOw0KICAgICAgICByZXR1cm4gdHJ1ZTsNCiAgICB9DQoNCiAgICBmdW5jdGlvbiBiYWxhbmNlT2YoYWRkcmVzcyB0b2tlbk93bmVyKSBwdWJsaWMgdmlldyByZXR1cm5zICh1aW50IGJhbGFuY2UpIHsNCiAgICAgICAgcmV0dXJuIGJhbGFuY2VzW3Rva2VuT3duZXJdOw0KICAgIH0NCn0>`_.

::

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

        function transfer(address to, uint amount) public returns (bool success) {
            balances.move(msg.sender, to, amount);
            emit Transfer(msg.sender, to, amount);
            return true;

        }

        function transferFrom(address from, address to, uint amount) public returns (bool success) {
            require(allowed[from][msg.sender] >= amount);
            allowed[from][msg.sender] -= amount;
            balances.move(from, to, amount);
            emit Transfer(from, to, amount);
            return true;
        }

        function approve(address spender, uint tokens) public returns (bool success) {
            require(allowed[msg.sender][spender] == 0, "");
            allowed[msg.sender][spender] = tokens;
            emit Approval(msg.sender, spender, tokens);
            return true;
        }

        function balanceOf(address tokenOwner) public view returns (uint balance) {
            return balances[tokenOwner];
        }
    }
