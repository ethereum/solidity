contract A {
	uint x;
	function f() internal view {
		assert(x == 0);
	}
}

contract B is A {
	uint a;
	uint b;
}

contract C is B {
	uint y;
	uint z;
	uint w;
	function g() public {
		x = 1;
		f();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (54-68): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
