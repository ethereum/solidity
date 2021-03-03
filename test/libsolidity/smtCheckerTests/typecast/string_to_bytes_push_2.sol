pragma experimental SMTChecker;

contract C {
	string x;

	function s() public {
		x = "abc";
		((bytes(((x))))).push("a");
		assert(bytes(x).length == 4); // should hold
		assert(bytes(x).length == 3); // should fail
	}
}
// ----
// Warning 6328: (173-201): CHC: Assertion violation happens here.\nCounterexample:\nx = [97, 98, 99, 97]\n\nTransaction trace:\nC.constructor()\nState: x = []\nC.s()
