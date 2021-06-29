interface Token {
	function balanceOf(address _a) external view returns (uint);
	function transfer(address _to, uint _amt) external;
}

contract TokenCorrect is Token {
	mapping (address => uint) balance;
	constructor(address _a, uint _b) {
		balance[_a] = _b;
	}
	function balanceOf(address _a) public view override returns (uint) {
		return balance[_a];
	}
	function transfer(address _to, uint _amt) public override {
		require(balance[msg.sender] >= _amt);
		balance[msg.sender] -= _amt;
		balance[_to] += _amt;
	}
}

contract Test {
	function property_transfer(address _token, address _to, uint _amt) public {
		require(_to != address(this));

		TokenCorrect t = TokenCorrect(_token);

		uint xPre = t.balanceOf(address(this));
		require(xPre >= _amt);
		uint yPre = t.balanceOf(_to);

		t.transfer(_to, _amt);
		uint xPost = t.balanceOf(address(this));
		uint yPost = t.balanceOf(_to);

		assert(xPost == xPre - _amt);
		assert(yPost == yPre + _amt);
	}

	function test_concrete() public {
		TokenCorrect t = new TokenCorrect(address(this), 1000);

		uint b = t.balanceOf(address(this));
		assert(b == 1000);

		address other = address(0x333);
		require(address(this) != other);

		uint c = t.balanceOf(other);
		assert(c == 0);

		t.transfer(other, 100);

		uint d = t.balanceOf(address(this));
		assert(d == 900);

		uint e = t.balanceOf(other);
		assert(e == 100);
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTContract: Test
// SMTTargets: assert
