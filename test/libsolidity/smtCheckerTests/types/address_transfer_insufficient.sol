pragma experimental SMTChecker;

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
// ----
// Warning 6328: (213-237): CHC: Assertion violation happens here.\nCounterexample:\n\na = 7719\nb = 7719\n\n\nTransaction trace:\nconstructor()\nf(7719, 7719)
// Warning 1236: (134-149): BMC: Insufficient funds happens here.
// Warning 1236: (153-169): BMC: Insufficient funds happens here.
