contract C {
	struct S {
		uint x;
		bool b;
	}

	S public s;

	constructor() {
		s.x = 1;
		s.b = false;
	}

	function f() public {
		uint x;
		bool b;
		(x,b) = this.s();
		assert(x == s.x); // this should hold
		assert(b == s.b); // this should hold
		assert(b == true); // this should fail
		s.x = 42;
		(uint y, bool c) = this.s();
		assert(c == b); // this should hold
		assert(y == x); // this should fail

	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (255-272): CHC: Assertion violation happens here.
// Warning 6328: (377-391): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
