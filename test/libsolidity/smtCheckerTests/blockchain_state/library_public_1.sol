library L {
	function l(address payable a) public {}
}

contract C {
	using L for address payable;
	uint x;
	function f(address payable a) public payable {
		require(msg.value > 1);
		uint b1 = address(this).balance;
		a.l();
		uint b2 = address(this).balance;
		assert(b1 == b2); // should fail because the called library can transfer with `this`s balance
		assert(x == 0); // should fail because of `delegatecall`
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 4588: (219-224): Assertion checker does not yet implement this type of function call.
// Warning 6328: (263-279): CHC: Assertion violation happens here.
// Warning 6328: (359-373): CHC: Assertion violation happens here.
