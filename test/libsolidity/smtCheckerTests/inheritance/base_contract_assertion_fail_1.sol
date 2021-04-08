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
// ====
// SMTEngine: all
// ----
// Warning 6328: (52-66): CHC: Assertion violation happens here.
