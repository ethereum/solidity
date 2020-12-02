pragma experimental SMTChecker;

contract C {
	mapping (bytes16 => uint) public m;

	function f() public view {
		uint y = this.m("foo");
		assert(y == m["foo"]); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (180-194): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
