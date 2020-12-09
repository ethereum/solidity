pragma experimental SMTChecker;

contract C {
	mapping (uint => uint) public map;

	function f() public view {
		uint y = this.map(2);
		assert(y == map[2]); // should hold
		assert(y == 1); // should fail
	}
}
// ----
// Warning 6328: (175-189): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
