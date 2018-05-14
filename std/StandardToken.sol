pragma solidity ^0.4.22;

import "./Token.sol";

contract StandardToken is Token {
	uint256 supply;
	mapping (address => uint256) balance;
	mapping (address =>
		mapping (address => uint256)) m_allowance;

	constructor(address _initialOwner, uint256 _supply) public {
		supply = _supply;
		balance[_initialOwner] = _supply;
	}

	function balanceOf(address _account) view public returns (uint) {
		return balance[_account];
	}

	function totalSupply() view public returns (uint) {
		return supply;
	}

	function transfer(address _to, uint256 _value) public returns (bool success) {
		return doTransfer(msg.sender, _to, _value);
	}

	function transferFrom(address _from, address _to, uint256 _value) public returns (bool) {
		if (m_allowance[_from][msg.sender] >= _value) {
			if (doTransfer(_from, _to, _value)) {
				m_allowance[_from][msg.sender] -= _value;
			}
			return true;
		} else {
			return false;
		}
	}

	function doTransfer(address _from, address _to, uint _value) internal returns (bool success) {
		if (balance[_from] >= _value && balance[_to] + _value >= balance[_to]) {
			balance[_from] -= _value;
			balance[_to] += _value;
			emit Transfer(_from, _to, _value);
			return true;
		} else {
			return false;
		}
	}

	function approve(address _spender, uint256 _value) public returns (bool success) {
		m_allowance[msg.sender][_spender] = _value;
		emit Approval(msg.sender, _spender, _value);
		return true;
	}

	function allowance(address _owner, address _spender) view public returns (uint256) {
		return m_allowance[_owner][_spender];
	}
}
