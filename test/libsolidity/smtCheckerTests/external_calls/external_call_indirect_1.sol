contract A {
	uint x;
	function setX(uint _x) public {
		x = _x;
	}
	function getX() public view returns (uint) {
		return x;
	}
}

contract B {
	A a;
	constructor() {
		a = new A();
		assert(a.getX() == 0); // should hold
	}
	function g() public view {
		assert(a.getX() == 0); // should fail because A.setX() can be called without B
	}
	function getX() public view returns (uint) {
		return a.getX();
	}
}

contract C {
	B b;
	constructor() {
		b = new B();
		assert(b.getX() == 0); // should hold
	}
	function f() public view {
		assert(b.getX() == 0); // should fail because A.setX() can be called without A
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// ----
// Warning 1218: (256-277): CHC: Error trying to invoke SMT solver.
// Warning 6328: (256-277): CHC: Assertion violation might happen here.
// Warning 6328: (533-554): CHC: Assertion violation might happen here.
