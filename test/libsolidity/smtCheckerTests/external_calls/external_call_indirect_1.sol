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
// Warning 6328: (256-277): CHC: Assertion violation happens here.
// Warning 6328: (533-554): CHC: Assertion violation might happen here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
