pragma experimental SMTChecker;
contract D {
	function f() public pure {
		assert(1000000000000000000 wei == 1 ether);
		assert(100000000000000000 wei == 1 ether);
		assert(1000000000 wei == 1 gwei);
		assert(100000000 wei == 1 gwei);
		assert(1000000000 gwei == 1 ether);
		assert(100000000 gwei == 1 ether);
	}
}
// ----
// Warning 6328: (121-162): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()
// Warning 6328: (202-233): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()
// Warning 6328: (275-308): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nD.constructor()\nD.f()
