library L {
	function l(address payable a) internal {
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
		assert(b1 == b2 - 1); // should hold but we don't keep track of balances with msg.value yet
		assert(x == 0); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (284-300): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 21238\nb1 = 8856\nb2 = 8855\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(21238){ value: 11799 }\n    L.l(21238){ value: 11799 } -- internal call
// Warning 3944: (332-338): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\nx = 0\na = 38\nb1 = 1\nb2 = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(38){ value: 21240 }\n    L.l(38){ value: 21240 } -- internal call
// Warning 6328: (319-339): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 21238\nb1 = 40\nb2 = 39\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.f(21238){ value: 8857 }\n    L.l(21238){ value: 8857 } -- internal call
// Warning 1236: (56-69): BMC: Insufficient funds happens here.
// Warning 1236: (56-69): BMC: Insufficient funds happens here.
