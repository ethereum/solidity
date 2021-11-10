contract A {
	uint x;
	address immutable owner;
	constructor() {
		owner = msg.sender;
	}
	function setX(uint _x) public {
		require(msg.sender == owner);
		x = _x;
	}
	function getX() public view returns (uint) {
		return x;
	}
}

contract B {
	A a;
	address immutable owner;
	constructor() {
		owner = msg.sender;
		a = new A();
		assert(a.getX() == 0); // should hold
	}
	function g() public {
		require(msg.sender == owner);
		a.setX(42);
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
		assert(b.getX() == 0); // should hold
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// ----
// Warning 1218: (641-662): CHC: Error trying to invoke SMT solver.
// Warning 6328: (641-662): CHC: Assertion violation might happen here.
