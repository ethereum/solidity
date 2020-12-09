pragma experimental SMTChecker;

contract C
{
	function f(uint x, address payable a, address payable b) public {
		require(a != b);
		require(x == 100);
		require(x == a.balance);
		require(a.balance == b.balance);
		a.transfer(600);
		b.transfer(100);
		// Fails since a == this is possible.
		assert(a.balance > b.balance);
	}
}
// ----
// Warning 6328: (295-324): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 100\na = 7720\nb = 7719\n\n\nTransaction trace:\nconstructor()\nf(100, 7720, 7719)
// Warning 1236: (217-232): BMC: Insufficient funds happens here.
// Warning 1236: (236-251): BMC: Insufficient funds happens here.
