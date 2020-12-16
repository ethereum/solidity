pragma experimental SMTChecker;

contract C {
	uint constant x = 2;
	uint constant y = x ** 10;

	function f() public view {
		assert(y == 2 ** 10);
		assert(y == 1024);
		assert(y == 14); // should fail
	}
}
// ----
// Warning 2018: (98-206): Function state mutability can be restricted to pure
// Warning 6328: (172-187): CHC: Assertion violation happens here.\nCounterexample:\nx = 2, y = 1024\n\n\n\nTransaction trace:\nconstructor()\nState: x = 2, y = 1024\nf()
