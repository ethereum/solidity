function l(address payable a) {
	a.transfer(1);
}

contract C {
	uint x;
	function f(address payable a) public payable {
		require(msg.value > 1);
		uint b1 = address(this).balance;
		require(a != address(this));
		l(a);
		uint b2 = address(this).balance;
		assert(b1 == b2); // should fail
		assert(b1 == b2 + 1); // should hold
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (258-274): CHC: Assertion violation happens here.
// Info 1180: Contract invariant(s) for :C:\n(x <= 0)\n
// Warning 1236: (33-46): BMC: Insufficient funds happens here.
