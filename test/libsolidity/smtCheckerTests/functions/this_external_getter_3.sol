pragma experimental SMTChecker;

contract C {
	mapping (uint => mapping (uint => uint)) public map;

	function f() public view {
		uint y = this.map(2, 3);
		assert(y == map[2][3]); // This fails as false positive because of lack of support for external getters.
	}
}
// ----
// Warning 6328: (158-180): CHC: Assertion violation happens here.
