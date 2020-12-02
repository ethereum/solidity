pragma experimental SMTChecker;

contract C {
	function f(address payable a) public {
		require(address(this).balance > 1000);
		a.transfer(666);
		assert(address(this).balance > 100);
		// Fails.
		assert(address(this).balance > 500);
	}
}
// ----
// Warning 6328: (199-234): CHC: Assertion violation happens here.\nCounterexample:\n\na = 2437\n\n\nTransaction trace:\nconstructor()\nf(2437)
