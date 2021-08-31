contract C
{
	function f(address payable a, address payable b) public {
		require(a.balance == 0);
		a.transfer(600);
		b.transfer(1000);
		// Fails since a == this is possible.
		assert(a.balance == 600);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (180-204): CHC: Assertion violation happens here.\nCounterexample:\n\na = 8855\nb = 8855\n\nTransaction trace:\nC.constructor()\nC.f(8855, 8855)
// Warning 1236: (101-116): BMC: Insufficient funds happens here.
// Warning 1236: (120-136): BMC: Insufficient funds happens here.
