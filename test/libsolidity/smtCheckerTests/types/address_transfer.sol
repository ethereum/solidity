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
// Warning 8656: (98-113): CHC: Insufficient funds happens here.
// Warning 6328: (162-186): CHC: Assertion violation happens here.
