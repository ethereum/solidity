contract C
{
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
// Warning 6328: (162-186): CHC: Assertion violation happens here.\nCounterexample:\n\na = 38\nx = 100\n\nTransaction trace:\nC.constructor()\nC.f(38)
// Warning 1236: (98-113): BMC: Insufficient funds happens here.
