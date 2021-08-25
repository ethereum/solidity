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
// ----
// Warning 4588: (219-224): Assertion checker does not yet implement this type of function call.
// Warning 6328: (263-279): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 0\nb1 = 7720\nb2 = 7719\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0){ value: 5855 }
// Warning 6328: (359-373): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\na = 0\nb1 = 39\nb2 = 38\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(0){ value: 21240 }
// Warning 4588: (219-224): Assertion checker does not yet implement this type of function call.
