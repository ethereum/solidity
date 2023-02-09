abstract contract A {
	bool s;

	function f() public view mod {
		assert(s); // holds for B
		assert(!s); // fails for B
	}
	modifier mod() virtual;
}

contract B is A {
	modifier mod() virtual override {
		bool x = true;
		s = x;
		_;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (94-104): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
