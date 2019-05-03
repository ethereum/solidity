pragma solidity >=0.0.0;

// This is a contract with a number of syntax errors in
// statements where deleting to semicolon seems to be
// the right thing to do

// Tokens are deleted until the next subsequent semicolon.

contract MetaCoin2 {
	mapping (address => uint balances;

	event Transfer(address indexed _from, address indexed _to, uint256 _value);

	constructor() public {
		balances[tx.origin] = 10000;
	}

	function sendCoin(address receiver, uint amount) public returns(bool sufficient) {
		uint i;
		if (balances[msg.sender])
		    return false;

		balances[msg.sender] -= // missing RHS

		balances[receiver] += amount;
		emit Transfer(msg.sender;  // missing )

		// missing < in while
		while (amount 10) {
			if (amount > 1) break;
				amount += ;  //
			continue;
			if (amount > 2) break;
		}
		return true;
	}

	function getBalanceInEth(address addr) public view returns(uint){
		return ConvertLib.convert(getBalance(addr),2);
	}

	function getBalance(address addr) public view returns(uint) {
		return balances[addr];
	}
}
// ----
// ParserError: (530-538): Expected ';' but got identifier. Skipping to next ';'.
// ParserError: (635-639): Expected ';' but got 'emit'. Skipping to next ';'.
// ParserError: (774-780): Expected ';' but got identifier. Skipping to next ';'.
// ParserError: (841-843): Expected ';' but got 'if'. Skipping to next ';'.
