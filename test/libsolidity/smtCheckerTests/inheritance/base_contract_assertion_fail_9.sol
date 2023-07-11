contract A {
	uint x;
	function f() public virtual {
		v();
		assert(x == 0); // should fail when C is the most derived contract
		assert(x == 2); // should fail when A is the most derived contract
	}
	function v() internal virtual {
		x = 0;
	}
}

contract B is A {
	function f() public virtual override {
		super.f();
	}
}

contract C is B {
	function g() public {
		x = 1;
		f();
	}
	function v() internal override {
		x = 2;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (62-76): CHC: Assertion violation happens here.
// Warning 6328: (131-145): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
