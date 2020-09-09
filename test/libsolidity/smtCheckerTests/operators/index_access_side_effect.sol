pragma experimental SMTChecker;

contract C {
	uint[] a;
	function h() internal returns (uint[] storage) {
		if (a[2] == 0)
			a[2] = 3;
		return a;
	}
	function g() public {
		h()[2] = 4;
		assert(h()[2] == 3);
	}
}
// ----
// Warning 6328: (191-210): Assertion violation happens here.
