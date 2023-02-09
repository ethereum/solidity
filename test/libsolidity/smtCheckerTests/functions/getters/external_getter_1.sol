contract D {
	uint public d;
	function g() public {
		++d;
	}
}

contract C {
	function f() public {
		D a = new D();
		assert(a.d() == 0); // should hold
		a.g();
		assert(a.d() == 1); // should hold
		assert(a.d() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (203-221): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
