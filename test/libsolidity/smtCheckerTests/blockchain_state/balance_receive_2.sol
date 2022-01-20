contract C {
	uint x;
	bool once;
	constructor() payable {
		x = address(this).balance;
	}
	function f() public payable {
		require(!once);
		once = true;
		require(msg.value > 0);
		assert(address(this).balance > x); // should hold
		assert(address(this).balance > x + 10); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4984: (266-272): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 6328: (235-273): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\nonce\n
