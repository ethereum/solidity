library L {
	function l(address payable a) public {
		a.transfer(1);
	}
}

contract C {
	using L for address payable;
	uint x;
	function f(address payable a) public payable {
		require(msg.value > 1);
		uint b1 = address(this).balance;
		a.l();
		uint b2 = address(this).balance;
		assert(b1 == b2); // should fail
		assert(x == 0); // should fail because of `delegatecall`
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4588: (238-243): Assertion checker does not yet implement this type of function call.
// Warning 6328: (282-298): CHC: Assertion violation happens here.
// Warning 6328: (317-331): CHC: Assertion violation happens here.
// Warning 1236: (54-67): BMC: Insufficient funds happens here.
