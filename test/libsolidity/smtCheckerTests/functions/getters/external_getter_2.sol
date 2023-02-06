contract E {
	uint public e;
	function setE(uint _e) public {
		e = _e;
	}
}

contract D {
	E e;
	constructor(E _e) {
		e = _e;
	}
	function setE(uint x) public {
		e.setE(x);
	}
}

contract C {
	function f() public {
		E e = new E();
		D d = new D(e);
		assert(e.e() == 0); // should hold
		d.setE(42);
		assert(e.e() == 42); // should hold
		assert(e.e() == 2); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (344-362): CHC: Assertion violation happens here.
