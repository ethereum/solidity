contract C
{
	constructor() payable {}
	function f(address payable a) public {
		uint x = 100;
		require(x == a.balance);
		a.transfer(600);
		// This fails since a == this is possible.
		assert(a.balance == 700);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (188-212): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\nx = 100\n\nTransaction trace:\nC.constructor(){ value: 57 }\nC.f(0)
// Warning 1236: (124-139): BMC: Insufficient funds happens here.
