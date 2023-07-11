contract A {
	int public x;
	function f() public virtual {
		x = 1;
	}
}

contract B is A {
	function f() public virtual override {
		super.f();
		x += 100;
	}
}

contract C is B {
	function f() public virtual override {
		super.f();
		x += 10;
	}
}

contract D is B {
}

contract E is C,D {
	function f() public override(C,B) {
		super.f();
		assert(x == 111); // should hold
		assert(x == 13); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (379-394): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
