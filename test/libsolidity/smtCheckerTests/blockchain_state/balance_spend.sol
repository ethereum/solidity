contract C {
	constructor() payable {
		require(msg.value > 100);
	}
	uint c;
	function f(address payable _a, uint _v) public {
		require(_v < 10);
		require(c < 2);
		++c;
		_a.transfer(_v);
	}
	function inv() public view {
		assert(address(this).balance > 80); // should hold
		assert(address(this).balance > 90); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (280-314): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n((!(c <= 1) || !((:var 1).balances[address(this)] <= 90)) && !((:var 1).balances[address(this)] <= 81) && (!(c <= 0) || !((:var 1).balances[address(this)] <= 100)))\n
// Warning 1236: (175-190): BMC: Insufficient funds happens here.
