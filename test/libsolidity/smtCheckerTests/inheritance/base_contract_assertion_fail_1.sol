pragma experimental SMTChecker;

contract B {
	uint x;
	function f() public view {
		assert(x == 0);
	}
}

contract C is B {
	uint y;
	function g() public {
		x = 1;
		f();
	}
}
// ----
// Warning 6328: (85-99): CHC: Assertion violation happens here.
