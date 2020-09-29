pragma experimental SMTChecker;
contract C {
	function f() public pure returns(int) {
		int a;
		((,, a)) = ((((((1, 3, (((((2)))))))))));
		assert(a == 2);
		assert(a == 3);
	}
}
// ----
// Warning 6328: (159-173): CHC: Assertion violation happens here.
