pragma experimental SMTChecker;

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
// ----
// Warning 6328: (195-219): CHC: Assertion violation happens here.\nCounterexample:\n\na = 38\n\n\nTransaction trace:\nconstructor()\nf(38)
// Warning 1236: (131-146): BMC: Insufficient funds happens here.
