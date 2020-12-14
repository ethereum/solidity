pragma experimental SMTChecker;

contract C {
	function f() public pure {
		bytes4 x = 0x01020304;
		bytes1 b = 0x02;
		assert(x[0] == b); // fails
		assert(x[1] == b);
		assert(x[2] == b); // fails
		assert(x[3] == b); // fails
	}
}
// ----
// Warning 6328: (120-137): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (171-188): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (201-218): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
