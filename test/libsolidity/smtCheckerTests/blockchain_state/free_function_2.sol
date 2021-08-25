function l(address payable a) {
	a.transfer(1);
}

contract C {
	uint x;
	function f(address payable a) public payable {
		require(msg.value > 1);
		uint b1 = address(this).balance;
		l(a);
		uint b2 = address(this).balance;
		assert(b1 == b2); // should fail
		assert(b1 == b2 - 1); // should hold but we don't keep track of balances with msg.value yet
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (227-243): CHC: Assertion violation happens here.
// Warning 3944: (275-281): CHC: Underflow (resulting value less than 0) happens here.
// Warning 6328: (262-282): CHC: Assertion violation happens here.
// Warning 1236: (33-46): BMC: Insufficient funds happens here.
